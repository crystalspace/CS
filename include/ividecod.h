/*
    Copyright (C) 2001 by Norman Krämer
  
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

#ifndef _IVIDEO_H_
#define _IVIDEO_H_

/**
 * This is an interface for playing video.
 */

#include "csutil/scf.h"
#include "isystem.h"
#include "iplugin.h"

struct iMaterialHandle;
struct iFile;

/// define some capabilities to request from the decoder

enum csVideoDecoderCap
{
  /**
   * The decoder is able to set the current position to a particular frame.
   */
  CS_POS_BY_FRAME = 1,
  /**
   * The decoder is able to set the current position based on a time index.
   */
  CS_POS_BY_TIME = 2,
  /**
   * The decoder is able to decode a set of consecutive frames at once.
   */
  CS_DECODE_SPAN = 4,
  /**
   * The decoder is able to dynamically change frame output size.
   */
  CS_DYNAMIC_FRAMESIZE = 8
};

SCF_VERSION (iVideoDecoder, 0, 0, 1);

struct iVideoDecoder : public iPlugIn
{
  /**
   * Retrieve the decoder capabilities.
   */
  virtual void GetDecoderCaps (csVideoDecoderCap &caps) = 0;
  /**
   * Next frame to be shown is the frame at frameindex.
   */
  virtual bool GotoFrame (ULong frameindex) = 0;
  /**
   * Next frame to be shown is the frame at timeindex.
   */
  virtual bool GotoTime (ULong timeindex) = 0;
  /**
   * When NextFrame is called either the next frame is read based on frameindex
   * or the next frame at timeindex (bTimeSynced = true).
   */
  virtual bool SetPlayMethod (bool bTimeSynced) = 0;
  /**
   * Set the output rectangle.
   */
  virtual bool SetRect (int x, int y, int w, int h) = 0;
  /**
   * Set the size of the frame to decode. If this does not equal the output rectangle, the frame
   * will be stretched by the renderer.
   */
  virtual bool SetSourceSize (int w, int h) = 0;
  /**
   * Set the blendingmode that is used to combine this frame with the data in the framebuffer
   * Use the usual CS_FX_* values. The default is CS_FX_COPY.
   */
  virtual bool SetFXMode (UInt mode) = 0;
  /** 
   * Call this in your main loop between BeginDraw and EndDraw. This will decode the next frame
   * from the video and draw it to the rectangle set in SetRect ().
   */
  virtual void NextFrame () = 0;
  /**
   * Call this if you want to use the frames as textures on your own polygons.
   */
  virtual iMaterialHandle* NextFrameGetMaterial () = 0;
  /**
   * Load the videodata from the following source.
   */
  virtual bool Load (iFile *pVideoData) = 0;
};
#endif
