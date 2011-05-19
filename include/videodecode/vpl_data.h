/*
    Copyright (C) 2004 by Andrew Mann

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

#ifndef __CS_VPL_DATA_H__
#define __CS_VPL_DATA_H__

/**\file
 * Video Player: data container 
 */

#include "csutil/scf.h"

/**\addtogroup vpl
 * @{ */

struct csVPLvideoFormat;
struct iSndSysStream;
struct vidFrameData;

#define CS_VPL_DATA_UNKNOWN_SIZE -1

/**
 * The sound data is an interface to the container object controlling raw
 * sound data.
 * After obtaining an iSndSysData interface (most likely by loading a sound
 * file) at least one iSndSysStream must be obtained.
 *
 * This interface is implemented at least once per Sound Element.
 */
struct iVPLData : public virtual iBase
{
  SCF_INTERFACE(iVPLData,0,2,0);

  /// Get the format of the sound data.
  virtual const csVPLvideoFormat *GetFormat()=0;
  virtual void SetFormat(csVPLvideoFormat *vplFormat)=0;

  /// Get size of this sound in frames.
  virtual size_t GetFrameCount() = 0;

  /**
   * Return the size of the data stored in bytes.  This is informational only
   * and is not guaranteed to be a number usable for sound calculations.
   * For example, an audio file compressed with variable rate compression may
   * result in a situation where FILE_SIZE is not equal to
   * FRAME_COUNT * FRAME_SIZE since FRAME_SIZE may vary throughout the
   * audio data.
   */
  virtual size_t GetDataSize() = 0;

  /// Set an optional description to be associated with this sound data
  //   A filename isn't a bad idea!
  virtual void SetDescription(const char *pDescription) = 0;

  /// Retrieve the description associated with this sound data
  //   This may return 0 if no description is set.
  virtual const char *GetDescription() = 0;

  virtual void getNextFrame(vidFrameData &data) = 0;
};

/** @} */

#endif // __CS_VPL_DATA_H__
