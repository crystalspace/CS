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

#ifndef SNDSYS_STREAM_WAV_H
#define SNDSYS_STREAM_WAV_H


#include "csplugincommon/sndsys/sndstream.h"
#include "wavdata2.h"

CS_PLUGIN_NAMESPACE_BEGIN(SndSysWav)
{

using namespace CS::SndSys;

class SndSysWavSoundStream : public SndSysBasicStream
{
 public:
  /// Construct a stream from the provided data.
  SndSysWavSoundStream(csRef<SndSysWavSoundData> pData, char *pWavData, 
    size_t WavDataLen, csSndSysSoundFormat *pRenderFormat, int Mode3D);
  virtual ~SndSysWavSoundStream();


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
  /// Holds our reference to the underlying data element
  csRef<SndSysWavSoundData> m_pSoundData;

  /// Pointer to the base of the WAV audio data buffer
  char *m_pWavDataBase;

  /// Length in bytes of the audio data contained in the buffer
  size_t m_WavDataLength;

  /// Pointer to the next sample in the audio buffer
  char *m_pWavCurrentPointer;

  /// Number of bytes left in the audio buffer
  size_t m_WavBytesLeft;

  /// Stores wether a conversion is needed on the data
  bool m_bConversionNeeded;
};

}
CS_PLUGIN_NAMESPACE_END(SndSysWav)

#endif // #ifndef SNDSYS_STREAM_WAV_H
