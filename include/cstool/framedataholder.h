/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_CSTOOL_FRAMEDATAHOLDER_H__
#define __CS_CSTOOL_FRAMEDATAHOLDER_H__

/**\file
 * Helper template to retrieve an instance of some type that has not yet been
 * used in a frame.
 */

#include "csgeom/math.h"
#include "csutil/array.h"

/**
 * Helper template to retrieve an instance of some type that has not yet been
 * used in a frame. Retrieval in subsequent frames will reuse already created 
 * instances, if appropriate (that is, the associated frame number differs from
 * the provide current frame number).
 */
template <class T>
class csFrameDataHolder
{
  /// Wrapper around data to hold.
  struct FrameData
  {
    /// Number of frame this data was last used
    uint lastFrame;
    /// The actual data
    T data;
  };
  /// The "cache" for the data
  csArray<FrameData> data;
  /// Index last checked for an available instance.
  size_t lastData;
  /// Frame number at which a shrink should be attempted next.
  uint nextShrink;
  /// Last frame number
  uint lastFrame;
  /// Frame number at which a "clear" was requested.
  uint clearReq;

  /// Small helper called at the start of a new frame
  void NewFrame()
  {
    if (clearReq != (uint)~0)
    {
      data.DeleteAll();
      clearReq = (uint)~0;
    }
    // Try to shrink the cache to a size that reflects what's needed
    if (lastFrame > nextShrink)
    {
      /* lastData is reset every frame, so it gives a good indicator
       * on how much has been used the last frame */
      data.Truncate (csMin(lastData+1, data.GetSize ()));
      data.ShrinkBestFit ();
      nextShrink = (uint)~0;
    }
    else if (lastData+1 < data.GetSize())
    {
      // Last frame we used less cache than available.
      // "Schedule" a shrink in the future.
      nextShrink = lastFrame + 5;
    }
    lastData = 0;
  }
public:
  csFrameDataHolder () : lastData(0), nextShrink((uint)~0),
  	lastFrame((uint)~0), clearReq((uint)~0)
  { }
  ~csFrameDataHolder () { }

  /**
   * Retrieve an instance of the type \a T whose associated frame number
   * differs from \a frameNumber. In \a created, it is returned whether a
   * new instance was created (value is \a true) or an existing one was
   * reused (value is \a false). Can be used to e.g. determine whether some 
   * initialization work can be saved.
   */
  T& GetUnusedData (bool& created, uint frameNumber)
  {
    if (lastFrame != frameNumber)
    {
      NewFrame();
      lastFrame = frameNumber;
    }
    created = false;
    if ((data.GetSize() == 0) || (data[lastData].lastFrame == frameNumber))
    {
      size_t startPoint = lastData;
      //check the list
      if (data.GetSize() > 0)
      {
	do
	{
	  if (data[lastData].lastFrame != frameNumber)
	    break;
	  lastData++;
	  if (lastData >= data.GetSize()) lastData = 0;
	}
	while (lastData != startPoint);
      }
      if (lastData == startPoint)
      {
	data.SetSize ((lastData = data.GetSize ()) + 1);
	created = true;
      }
    }
  
    FrameData& frameData = data[lastData];
    frameData.lastFrame = frameNumber;
    return frameData.data;
  }
  
  /**
   * Remove all allocated instances.
   * \remarks Warning! Don't use if pointers etc. to contained data still in 
   *  use!
   * \remarks By default. does not clear the allocated data instantly but 
   *  rather upon the next frame (ie when the \a frameNumber parameter passed 
   *  to GetUnusedData() differs from the last). To change this behaviour,
   *  set \a instaClear to true.
   */
  void Clear (bool instaClear = false)
  {
    if (instaClear)
      data.DeleteAll();
    else
      // Don't clear just yet, rather, clear when we enter the next frame.
      clearReq = lastFrame;
  }
};

#endif // __CS_CSTOOL_FRAMEDATAHOLDER_H__
