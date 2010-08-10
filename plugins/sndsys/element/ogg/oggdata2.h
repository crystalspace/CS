/*
    Copyright (C) 2005 by Andrew Mann
    Based in part on work by Norman Kraemer

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

#ifndef SNDSYS_DATA_OGG_H
#define SNDSYS_DATA_OGG_H

/**
 * iSndSysData implementation for ogg bitdata.
 */

#include "iutil/databuff.h"
#include "csplugincommon/sndsys/snddata.h"


using namespace CS::SndSys;



// This hack works around a build problem with some installations of Ogg/Vorbis
// on Cygwin where it attempts to #include the non-existent <_G_config.h>.  We
// must ensure that _WIN32 is not defined prior to #including the Vorbis
// headers.  This problem is specific to C++ builds; it does not occur with
// plain C builds (which explains why the CS configure check does not require
// this hack).
#ifdef __CYGWIN__
#undef _WIN32
#endif

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

/// This simple structure acts as an interface between an the Vorbis read/seek functions and data stored accessed through an iDataBuffer.
struct OggDataStore
{
  csRef<iDataBuffer> buf;
  unsigned char *data;
  size_t length;

  OggDataStore (iDataBuffer* buf) : buf(buf)
  {
    data = buf->GetUint8();
    length = buf->GetSize();
  }
};

/// A simple structure used by the stream to store both position
//  and a reference to the data
struct OggStreamData
{
  OggDataStore *datastore;
  size_t position;
};

/// The implementation of iSndSysData for Ogg Vorbis audio
class SndSysOggSoundData : public SndSysBasicData
{
public:
  /// A structure describing the list of callbacks used for various ogg functionality
  static const ov_callbacks ogg_callbacks;
  
  /// Construction requires passing an iDataBuffer which references encoded ogg vorbis audio
  SndSysOggSoundData (iBase *pParent, iDataBuffer* pDataBuffer);
  virtual ~SndSysOggSoundData ();

  ////
  // Internal functions
  ////
protected:

  /// This is required to initialize the m_SampleCount and m_SoundFormat member variables. It is called internally.
  //    This is only called the first time that SoundFormat or SampleCount data is requested.
  void Initialize();

public:
  /// Call to determine if the provided data can be decoded as ogg vorbis audio
  static bool IsOgg (iDataBuffer* Buffer);


  ////
  // Interface implementation
  ////

  //------------------------
  // iSndSysData
  //------------------------
public:

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
  virtual iSndSysStream *CreateStream (csSndSysSoundFormat *pRenderFormat, 
    int Mode3D);

   ////
   //  Member variables
   ////
protected:

   /// An accessor structure for the underlying ogg vorbis sound data
  OggDataStore m_DataStore;
};

#endif // #ifndef SNDSYS_DATA_OGG_H


