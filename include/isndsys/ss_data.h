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

#ifndef __CS_SNDSYS_DATA_H__
#define __CS_SNDSYS_DATA_H__

/**\file
 * Sound system: data container
 */

#include "csutil/scf.h"

/**\addtogroup sndsys
 * @{ */

struct csSndSysSoundFormat;
struct iSndSysStream;

#define CS_SNDSYS_DATA_UNKNOWN_SIZE -1

/**
 * The sound data is an interface to the container object controlling raw
 * sound data.
 * After obtaining an iSndSysData interface (most likely by loading a sound
 * file) at least one iSndSysStream must be obtained.
 *
 * This interface is implemented at least once per Sound Element.
 */
struct iSndSysData : public virtual iBase
{
  SCF_INTERFACE(iSndSysData,0,2,0);

  /// Get the format of the sound data.
  virtual const csSndSysSoundFormat *GetFormat() = 0;

  /**
   * Get the count of frames of this sound data. The count of frames is
   * equal to the actual duration of the sound, times its frequency. So, if
   * you want to know the duration of the sound data, you should divide the
   * return value of this method by the frequency of the sound data.
   */
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

  /**
   * This function should be called from the Renderer where a proper 
   * renderformat can be provided.
   * Creates a stream associated with this sound data positioned at the
   * beginning of the sound data and initially paused if possible.
   *
   * \remarks Not intended to be called by an application.
   */
  virtual iSndSysStream *CreateStream (csSndSysSoundFormat *renderformat,
  	int mode3d) = 0;

  /// Set an optional description to be associated with this sound data
  //   A filename isn't a bad idea!
  virtual void SetDescription(const char *pDescription) = 0;

  /// Retrieve the description associated with this sound data
  //   This may return 0 if no description is set.
  virtual const char *GetDescription() = 0;
};

/** @} */

#endif // __CS_SNDSYS_DATA_H__
