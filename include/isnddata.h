
#ifndef ISNDDATA_H
#define ISNDDATA_H

#include "csutil/scf.h"

struct iSoundStream;

/**
 * The sound format. This keeps information about the frequency and bits of a
 * sound data object and whether the object is prepared for mono and/or
 * stereo use.
 */
typedef struct csSoundFormat {
  // Frequency of the sound (hz)
  int Freq;
  // 8 or 16 bits
  int Bits;
  // number of channels (1 or 2)
  int Channels;
} csSoundFormat;

/**
 * The sound data. This represents any form of audio data, for example
 * a wave file loaded from your hard disk. The sound data is not nessecarily
 * stored as samples. Normally, you should not need to call any of the member
 * functions.
 */
SCF_VERSION (iSoundData, 0, 0, 1);

struct iSoundData : public iBase
{
  /// Create a stream to read the data.
  virtual iSoundStream *CreateStream() = 0;
  /// return the format of this sound
  virtual const csSoundFormat *GetFormat() = 0;
  /// get size of this sound in samples
  virtual unsigned long GetNumSamples() = 0;
  /**
   * Return a sound data object that is a decoded (raw wave) version of this
   * one.
   */
  virtual iSoundData *Decode() = 0;
};

/**
 * The sound stream. This represents a source of samples and is intended for
 * internal use.
 */
SCF_VERSION (iSoundStream, 0, 0, 1);

struct iSoundStream : public iBase
{
  /// Return the sound data of which this is an instance
  virtual iSoundData *GetSoundData() = 0;

  /**
   * Get a buffer with data. NumSample should be the requested number of
   * samples.
   * After calling, the NumSamples is set to the number of samples you
   * really got. If you got less samples than you requested, the stream is
   * finished and you can DecRef() it.
   * Note: A 'sample' includes 8/16 bits and all channels.
   * When you don't need the buffer anymore, you should use DiscardBuffer().
   */
  virtual void *Read(unsigned long &NumSamples) = 0;

  /// discard a buffer creates by Read().
  virtual void DiscardBuffer(void *Buffer) = 0;

  /// reset this instance to the beginning of the sound data
  virtual void Reset() = 0;
};

#endif
