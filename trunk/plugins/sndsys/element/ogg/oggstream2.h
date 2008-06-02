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

#ifndef SNDSYS_STREAM_OGG_H
#define SNDSYS_STREAM_OGG_H


#include "csplugincommon/sndsys/sndstream.h"
#include "oggdata2.h"

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


#ifdef CS_LITTLE_ENDIAN
  #define OGG_ENDIAN 0
#else
  #define OGG_ENDIAN 1
#endif

/// Implementation of iSndSysStream for Ogg Vorbis audio
class SndSysOggSoundStream : public SndSysBasicStream
{
public:
  SndSysOggSoundStream (csRef<SndSysOggSoundData> pData, OggDataStore *pDataStore, 
    csSndSysSoundFormat *pRenderFormat, int Mode3D);
  virtual ~SndSysOggSoundStream ();


  ////
  // Interface implementation
  ////

  //------------------------
  // iSndSysStream
  //------------------------
public:

  /// Retrieve the textual description of this stream
  virtual const char *GetDescription();

  /**
  * Get length of this stream in rendered frames.
  * May return CS_SNDSYS_STREAM_UNKNOWN_LENGTH if the stream is of unknown 
  * length. For example, sound data being streamed from a remote host may not 
  * have a pre-determinable length.
  */
  virtual size_t GetFrameCount();

  /**
  * NOT AN APPLICATION CALLABLE FUNCTION!   This function advances the stream
  * position based on the provided frame count value which is considered as an
  * elapsed frame count.
  *
  * A Sound Element will usually store the last advance frame internally.
  * When this function is called it will compare the last frame with the
  * frame presented and retrieve more data from the associated iSndSysData
  * container as needed.  It will then update its internal last advance
  * frame value.
  *  
  * This function is not necessarily thread safe and must be called ONLY
  * from the Sound System's main processing thread.
  */
  virtual void AdvancePosition(size_t frame_delta);

  ////
  //  Member variables
  ////
protected:
  /// State information about the sound data used by the ogg vorbis library
  OggVorbis_File m_VorbisFile;

  /// Our data related information. A reference to the underlying raw data
  //   and our position tracking reference are contained here.
  OggStreamData m_StreamData;

  /// Holds our reference to the underlying data element
  csRef<SndSysOggSoundData> m_pSoundData;

  /// Numeric identifier of the current stream within the ogg file
  //
  //  This is tracked because different streams within the same ogg file
  //   may have different audio properties that require us to adjust our
  //   decoding parameters.
  int m_CurrentOggStream;

  /// Format of the sound data in the current ogg stream
  vorbis_info *m_pCurrentOggFormatInfo;
};

#endif // #ifndef SNDSYS_STREAM_OGG_H


