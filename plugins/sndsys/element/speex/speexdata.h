/*
    Copyright (C) 2008 by Mike Gist, Andrew Mann

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

#ifndef SNDSYS_DATA_SPEEX_H
#define SNDSYS_DATA_SPEEX_H

#include "csplugincommon/sndsys/snddata.h"

using namespace CS::SndSys;

struct SpeexDataStore
{
  csRef<iDataBuffer> buf;
  unsigned char *data;
  size_t length;

  SpeexDataStore (iDataBuffer* buf) : buf(buf)
  {
    data = buf->GetUint8();
    length = buf->GetSize();
  }
};

/// The implementation of iSndSysData for Speex audio
class SndSysSpeexSoundData : public SndSysBasicData
{
public:
  SndSysSpeexSoundData (iBase *pParent, iDataBuffer* pDataBuffer);
  virtual ~SndSysSpeexSoundData ();

protected:

  /// This is required to initialize the m_SampleCount and m_SoundFormat member variables. It is called internally.
  //  This is only called the first time that SoundFormat or SampleCount data is requested.
  void Initialize();

  /// An accessor structure for the underlying speex sound data
  SpeexDataStore m_DataStore;

public:
  /// Call to determine if the provided data can be decoded as speex audio
  static bool IsSpeex (iDataBuffer* Buffer);

  SpeexDataStore& GetDataStore() { return m_DataStore; }

  /**
   * Return the size of the data stored in bytes.  This is informational only
   * and is not guaranteed to be a number usable for sound calculations.
   * For example, an audio file compressed with variable rate compression may
   * result in a situation where FILE_SIZE is not equal to
   * SAMPLE_COUNT * SAMPLE_SIZE since SAMPLE_SIZE may vary throughout the
   * audio data.
   */
  virtual size_t GetDataSize();

  /**
   * Creates a stream associated with this sound data positioned at the
   * begining of the sound data and initially paused if possible.
   */
  virtual iSndSysStream *CreateStream (csSndSysSoundFormat *pRenderFormat, int Mode3D);
};

#endif // SNDSYS_DATA_SPEEX_H


