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
public:
  csFrameDataHolder ()
  {
    lastData = 0;
    nextShrink = 0;
  }
  ~csFrameDataHolder ()
  {
  }

  T& GetUnusedData (bool& created, uint frameNumber)
  {
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
	data.GetExtend (lastData = data.Length ());
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
  
  void Clear ()
  {
    data.DeleteAll();
  }
};

#endif // __CS_CSTOOL_FRAMEDATAHOLDER_H__
