/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef __CS_ISOUND_DATA_H__
#define __CS_ISOUND_DATA_H__

#include "csutil/scf_interface.h"

/**
 * The sound format. This keeps information about the frequency, bits and
 * channels of a sound data object.
 */
struct csSoundFormat
{
  /// Frequency of the sound (hz)
  int Freq;
  /// number of bits per sample (8 or 16)
  int Bits;
  /// number of channels (1 or 2)
  int Channels;
};


/**
 * The sound data is a template used to play sounds. It represents a sound
 * file just after it was loaded. To play the sound, you must first prepare
 * it and obtain a sound handle.
 */
struct iSoundData : public virtual iBase
{
  SCF_INTERFACE(iSoundData, 2,0,0);
  /// Prepare the sound for output using the given format.
  virtual bool Initialize(const csSoundFormat *fmt) = 0;
  /// Get the format of the sound data.
  virtual const csSoundFormat *GetFormat() = 0;
  /// Return true if this is a static sound, false if it is streamed.
  virtual bool IsStatic() = 0;

  /// Get size of this sound in samples (static sounds only).
  virtual long GetStaticSampleCount() = 0;
  /// Get a pointer to the data buffer (static sounds only).
  virtual void *GetStaticData() = 0;

  /// Reset the sound to the beginning (streamed sounds only).
  virtual void ResetStreamed() = 0;
  /**
   * Read a data buffer from the sound (streamed sounds only). The NumSamples
   * parameter is modified to a smaller value if not all samples could be
   * read (i.e. the stream is finished). The returned buffer is valid until
   * the next call to Read().
   */
  virtual void *ReadStreamed(long &NumSamples) = 0;
};

#endif // __CS_ISOUND_DATA_H__
