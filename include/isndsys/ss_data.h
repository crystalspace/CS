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

#ifndef SNDSYS_DATA_H
#define SNDSYS_DATA_H

#include "csutil/scf.h"


struct iSndSysStream;

SCF_VERSION (iSndSysData, 0, 1, 0);

#define ISNDSYS_DATA_UNKNOWN_SIZE -1

/**
 * The sound data is an interface to the container object controlling raw sound data.
 *  After obtaining an iSound2Data interface (most likely by loading a sound file)
 *  at least one iSound2Stream must be obtained.
 *
 * This interface is implemented at least once per Sound Element.
 *
 *  TODO:  
 *  Open issues:
 *
 */
struct iSndSysData : public iBase
{
  /// Get the format of the sound data.
  virtual const SndSysSoundFormat *GetFormat() = 0;

  /// Get size of this sound in samples.
  virtual long GetSampleCount() = 0;

  /** Return the size of the data stored in bytes.  This is informational only and is not guaranteed to be a number usable for sound calculations.
   *  For example, an audio file compressed with variable rate compression may result in a situation where FILE_SIZE is not equal to
   *   SAMPLE_COUNT * SAMPLE_SIZE since SAMPLE_SIZE may vary throughout the audio data.
   */
  virtual long GetDataSize() = 0;

  /** NOT AN APPLICATION CALLABLE FUNCTION!   This function should be called from the Renderer where a proper renderformat can be provided.
   * Creates a stream associated with this sound data positioned at the beginning of the sound data and initially paused if possible.
   * 
   */
  virtual iSndSysStream *CreateStream(SndSysSoundFormat *renderformat, int mode3d) = 0;

};

#endif // #ifndef SNDSYS_DATA_H

