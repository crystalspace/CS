/*
  Copyright (C) 2006 by Andrew Mann

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef SNDSYS_RENDERER_SOFTWARE_FILTERQUEUE_H
#define SNDSYS_RENDERER_SOFTWARE_FILTERQUEUE_H

#include "csutil/refarr.h"

#include "isndsys/ss_structs.h"
#include "isndsys/ss_filter.h"

#include "csplugincommon/sndsys/queue.h"

using namespace CS::SndSys;

/// 
class SndSysOutputFilterQueue
{
public:
  SndSysOutputFilterQueue();
  ~SndSysOutputFilterQueue();

  /// A simple wrapper class to handle buffers of sound frames traversing from the background thread.
  //    
  class SampleBuffer
  {
  public:

    /// Primary constructor copies a provided buffer
    SampleBuffer(csSoundSample *pSamples, size_t FrameCount, size_t SamplesPerFrame) :
        m_FrameCount(FrameCount), m_SamplesPerFrame(SamplesPerFrame), m_CurrentOffset(0)
    {
      m_pSamples = new csSoundSample[FrameCount * SamplesPerFrame];
      memcpy(m_pSamples, pSamples, sizeof(csSoundSample) * FrameCount * SamplesPerFrame);
    }

    /// Secondary constructor creates an empty buffer.  Use AddSamples() to add samples to the buffer.
    SampleBuffer(size_t FrameCount, size_t SamplesPerFrame) :
      m_FrameCount(FrameCount), m_SamplesPerFrame(SamplesPerFrame), m_CurrentOffset(0)
    {
      m_pSamples = new csSoundSample[FrameCount * SamplesPerFrame];
    }

    ~SampleBuffer()
    {
      delete[] m_pSamples;
    }

    bool AddSamples(csSoundSample *pSamples, size_t SampleCount)
    {
      // Don't overflow the buffer
      if (m_CurrentOffset + SampleCount > (m_FrameCount * m_SamplesPerFrame))
        return false;
      memcpy(&m_pSamples[m_CurrentOffset], pSamples, SampleCount * sizeof(csSoundSample));
      m_CurrentOffset+=SampleCount;
      return true;
    }
  public:
    csSoundSample *m_pSamples;
    size_t m_FrameCount;
    size_t m_SamplesPerFrame;
    size_t m_CurrentOffset;
  };

public:
  /// Get the count of attached output filters
  size_t GetOutputFilterCount() { return m_OutputFilterCount; }

  /// Call from the main thread to dispatch all queued samples
  void DispatchSampleBuffers();

  /// Call from the background thread to copy and queue samples for dispatching
  bool QueueSampleBuffer(csSoundSample *pSampleBuffer, size_t FrameCount, size_t SamplesPerFrame);

  /// Call from the background thread to copy and queue samples for dispatching
  bool QueueSampleBuffer(SampleBuffer *pSampleBuffer);

  /// Add a filter to the list
  bool AddFilter(iSndSysSoftwareOutputFilter *pFilter);

  /// Remove a filter to the list
  bool RemoveFilter(iSndSysSoftwareOutputFilter *pFilter);

  /// Clear the queued sample buffers
  void ClearSampleBuffers();

  /// Remove all filters from the list
  void ClearFilterList();

protected:
  /// Queue of sample buffers to be passed to the output filter
  Queue<SampleBuffer> m_OutputSampleBufferQueue;

  /// The number of attached filters
  size_t m_OutputFilterCount;

  /// The list of attached filters
  csRefArray<iSndSysSoftwareOutputFilter> m_OutputFilterList;
};













#endif // #ifndef SNDSYS_RENDERER_SOFTWARE_FILTERQUEUE_H

