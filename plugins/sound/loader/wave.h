/*
    Copyright (C) 1998, 1999 by Nathaniel 'NooTe' Saint Martin
    Copyright (C) 1998, 1999 by Jorrit Tyberghein
    Written by Nathaniel 'NooTe' Saint Martin

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

#if !defined(__CSSOUNDLOADER_WAVE_H__)
#define __CSSOUNDLOADER_WAVE_H__

#include "isnddata.h"

/**
 * Wave class :<br>
 *    It's the 'sound' PCM stored data<br>
 *    16 bits is signed (short)<br>
 *    8 bits is unsigned (unsigned char)<p>
 *
 * Warning : size is the number of samples not the size of data
 *  i.e. if it's mono 8 bit and size = 800, data size is 800
 *       if it's stereo 16 bit and size = 800, data size is 800*2*2 (two channels and 16 bits)
 *
 * Building a sound data happens in two steps:
 * (1) Call Initialize() to fill the object with raw data. You must tell the object
 *     the format of this data.
 * (2) Call Prepare() to convert raw data to useful data.
 * After doing this, you may create instances of this data.
 */

class csSoundDataWave : public iSoundData {
friend class csSoundStreamWave;
public:
  DECLARE_IBASE;

  //
  csSoundDataWave(iBase *iParent);
  //
  virtual ~csSoundDataWave();

  // delete all stored data
  void Cleanup();

  // store the raw data. The sound data will delete this buffer
  void Initialize(int frequency, int Bits, int Channels, long numsmp, unsigned char *data);

  // prepare the sound
  void Prepare(const csSoundFormat *RequestFormat);

  virtual const csSoundFormat *GetFormat();
  virtual iSoundStream *CreateStream();
  virtual unsigned long GetNumSamples();
  virtual iSoundData *Decode();

private:
  void ConvertChannels(int NumChannels);
  void ConvertBits(int Bits);
  void ConvertFreq(int Freq);

  // size of this sound in samples
  unsigned long NumSamples;
  // format descriptor for prepared sound
  csSoundFormat Format;
  // prepared data for mono output
  unsigned char *Data;
};

class csSoundStreamWave : public iSoundStream {
public:
  DECLARE_IBASE;
  
  // constructor
  csSoundStreamWave(csSoundDataWave *Snd);
  // return the sound data object
  iSoundData *GetSoundData();
  // read the sound buffers
  virtual void *Read(unsigned long &NumSamples);
  // discard a buffer
  virtual void DiscardBuffer(void *Buffer);
  // reset the instance to the beginning of the data
  virtual void Reset();
private:
  csSoundDataWave *Data;
  unsigned long Position;
};

#endif // __CSSOUNDLOADER_WAVE_H__
