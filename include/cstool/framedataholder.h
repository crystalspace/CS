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
  struct FrameData
  {
    uint lastFrame;
    T data;
  };
  csArray<FrameData> data;
  size_t lastData;
  uint nextShrink;
  uint lastFrame, clearReq;
public:
  csFrameDataHolder () : lastData(0), nextShrink(0), lastFrame((uint)~0), clearReq((uint)~0)
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
    if ((clearReq != (uint)~0) && (clearReq != frameNumber))
      data.DeleteAll();
    lastFrame = frameNumber;
    created = false;
    if ((data.Length() == 0) || (data[lastData].lastFrame == frameNumber))
    {
      lastData = (size_t)-1;
      //check the list
      size_t i;
      for(i = 0; i < data.Length (); i++)
      {
	if (data[i].lastFrame != frameNumber)
	{
	  lastData = i;
	  break;
	}
      }
      if (lastData == (size_t)-1)
      {
	data.SetLength ((lastData = data.Length ()) + 1);
	created = true;
	nextShrink = frameNumber + 1;
      }
    }
  
    // Conserve some memory
    if (!created && (frameNumber <= nextShrink))
    {
      data.ShrinkBestFit ();
    }
  
    FrameData& frameData = data[lastData];
    frameData.lastFrame = frameNumber;
    return frameData.data;
  }
  
  /**
   * Remove all allocated instances.
   * \remarks Warning! Don't use if pointers etc. to contained data still in 
   *  use!
   * \remarks Does not clear the allocated data instantly but rather upon the
   *  next frame (ie when the \a frameNumbe parameter passed to 
   *  GetUnusedData() differs from the last).
   */
  void Clear ()
  {
    // Don't clear just yet, rather, clear when we enter the next frame.
    clearReq = lastFrame;
  }
};

#endif // __CS_CSTOOL_FRAMEDATAHOLDER_H__
