/*
    Copyright (C) 2011 by Alin Baciu

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

#ifndef __CS_VPL_CODEC_H__
#define __CS_VPL_CODEC_H__

/**\file
 * Video Player: data container. Each videodecode plugin must have one.
 * This is what reads video and audio data from a file
 */

#include "csutil/scf.h"

/**\addtogroup vpl
 * @{ */

struct csVPLvideoFormat;
struct vidFrameData;

/**
 * This is an interface for the codec.
 */
struct iVPLCodec : public virtual iBase
{
  SCF_INTERFACE(iVPLCodec,0,1,0);

  /// Get the format of the video data.
  virtual const csVPLvideoFormat *GetFormat()=0;
  virtual void SetFormat(csVPLvideoFormat *vplFormat)=0;

  /// Get size of this video in frames.
  virtual size_t GetFrameCount() = 0;

  /**
   * Return the size of the data stored in bytes.
   */
  virtual size_t GetDataSize() = 0;

  /// Set an optional description to be associated with this sound data
  //   A filename isn't a bad idea!
  virtual void SetDescription(const char *pDescription) = 0;

  /// Retrieve the description associated with this sound data
  //   This may return 0 if no description is set.
  virtual const char *GetDescription() = 0;

  virtual void getNextFrame(vidFrameData &data) = 0;
  virtual void update() = 0;
};

/** @} */

#endif // __CS_VPL_CODEC_H__
