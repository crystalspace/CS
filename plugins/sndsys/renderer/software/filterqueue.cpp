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

#include "cssysdef.h"


#include "filterqueue.h"



SndSysOutputFilterQueue::SndSysOutputFilterQueue() : 
  m_OutputFilterCount(0) 
{

}

SndSysOutputFilterQueue::~SndSysOutputFilterQueue() 
{
  ClearSampleBuffers();
}

/// Call from the main thread to dispatch all queued samples
void SndSysOutputFilterQueue::DispatchSampleBuffers()
{
  SampleBuffer *pBuffer;

  // Grab each SampleBuffer and pass it to the filters for processing
  while ((pBuffer=m_OutputSampleBufferQueue.DequeueEntry()) != 0)
  {
    size_t MaxIDX=m_OutputFilterList.GetSize();
    size_t FilterIDX;
    for (FilterIDX=0;FilterIDX<MaxIDX;FilterIDX++)
      m_OutputFilterList[FilterIDX]->DeliverData(pBuffer->m_pSamples, pBuffer->m_FrameCount);

    delete pBuffer;
  }
}

/// Call from the main thread to dispatch all queued samples
void SndSysOutputFilterQueue::ClearSampleBuffers()
{
  SampleBuffer *pBuffer;

  // Grab each SampleBuffer and delete it
  while ((pBuffer=m_OutputSampleBufferQueue.DequeueEntry()) != 0)
    delete pBuffer;
}


/// Call from the background thread to copy and queue samples for dispatching
bool SndSysOutputFilterQueue::QueueSampleBuffer(csSoundSample *pSampleBuffer, size_t FrameCount, size_t SamplesPerFrame)
{
  SampleBuffer *pSB=new SampleBuffer(pSampleBuffer, FrameCount, SamplesPerFrame);
  if (!pSB)
    return false;
  if (!QueueSampleBuffer(pSB))
  {
    delete pSB;
    return false;
  }
  return true;
}

bool SndSysOutputFilterQueue::QueueSampleBuffer(SampleBuffer *pSampleBuffer)
{
  return (m_OutputSampleBufferQueue.QueueEntry(pSampleBuffer) == QUEUE_SUCCESS);
}


bool SndSysOutputFilterQueue::AddFilter(iSndSysSoftwareOutputFilter *pFilter)
{
  m_OutputFilterList.Push(pFilter);
  m_OutputFilterCount=m_OutputFilterList.GetSize();

  return true;
}

bool SndSysOutputFilterQueue::RemoveFilter(iSndSysSoftwareOutputFilter *pFilter)
{
  if (m_OutputFilterList.Delete(pFilter))
  {
    m_OutputFilterCount=m_OutputFilterList.GetSize();
    return true;
  }
  return false;
}

void SndSysOutputFilterQueue::ClearFilterList()
{
  m_OutputFilterList.DeleteAll();
}





