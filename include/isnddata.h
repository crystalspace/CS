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

#ifndef __ISNDDATA_H__
#define __ISNDDATA_H__

#include "csutil/scf.h"

struct iSoundStream;

/**
 * The sound format. This keeps information about the frequency, bits and
 * channels of a sound data object.
 */
typedef struct csSoundFormat {
  /// Frequency of the sound (hz)
  int Freq;
  /// number of bits per sample (8 or 16)
  int Bits;
  /// number of channels (1 or 2)
  int Channels;
} csSoundFormat;

/**
 * The sound data is a template used to play sounds. You can create any
 * number of totally independent sound streams from it to play several
 * instances of the sound.
 */
SCF_VERSION (iSoundData, 1, 0, 0);

struct iSoundData : public iBase
{
  /// get size of this sound in samples
  virtual long GetNumSamples() = 0;
  /// get the format of the sound data
  virtual const csSoundFormat *GetFormat() = 0;
  /// create a sound stream for this data
  virtual iSoundStream *CreateStream() = 0;
};

/**
 * The iSoundStream is either an instance of a sound data object or a
 * streamed sound file. There is a one-to-one relation between a sound stream
 * and a sound source.
 */
SCF_VERSION (iSoundStream, 1, 0, 0);

struct iSoundStream : public iBase
{
  /// get the format of the sound data
  virtual const csSoundFormat *GetFormat() = 0;

  /**
   * Get the total number of samples that can be read from this stream or
   * -1 if unknown
   */
  virtual long GetNumSamples() = 0;

  /**
   * Reset this stream to the beginning. This is used when a looped sound
   * reaches the end or if a source plays with the SOUND_RESTART option.
   * If you write your own implementation and don't want it to be able to
   * restart (for example, run-time generated speech), you can implement this
   * as NOP to simply not restart the sound when the above happens. Note that
   * the sound renderer is not forced to use this method to restart the
   * sound, so you shouldn't abuse this method and do anything else here.
   */
  virtual void Restart() = 0;

  /**
   * Read a data buffer from the stream. When calling, NumSamples should be
   * The number of samples you want to read. After calling, it contains the
   * number of samples actually read. If this is less than the number you
   * wanted to read, the stream is finished and can be DecRef'ed.
   * When you don't need the buffer anymore, call DiscardBuffer() to get
   * rid of it.
   */
  virtual void *Read(long &NumSamples) = 0;

  // discard a buffer returned by Read()
  virtual void DiscardBuffer(void *buf) = 0;
};

#endif
