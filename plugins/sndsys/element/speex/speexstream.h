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

#ifndef SNDSYS_STREAM_SPEEX_H
#define SNDSYS_STREAM_SPEEX_H

#include <ogg/ogg.h>
#include <speex/speex.h>
#include <speex/speex_header.h>

#include "csplugincommon/sndsys/sndstream.h"
#include "speexdata.h"

#ifndef CS_HAVE_SPEEX_HEADER_FREE
#define speex_header_free(X)	free(X)
#endif

using namespace CS::SndSys;

class SndSysSpeexSoundStream : public SndSysBasicStream
{
public:
  SndSysSpeexSoundStream(csRef<SndSysSpeexSoundData> pData, 
    csSndSysSoundFormat *pRenderFormat, int Mode3D);

  virtual ~SndSysSpeexSoundStream ();

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
    * Not permitted (yet) operations.
    */
  virtual bool SetPosition (size_t newposition) { return false; }

  /**
    * Not permitted (yet) operations.
    */
  virtual bool SetLoopBoundaries(size_t &startPosition, size_t &endPosition) { return false; }

  /**
   * Reset the stream back to the beginning.
   */
  virtual bool ResetPosition(bool clear = true);

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

  /// Whether this stream always needs to be treated as a stream regardless of size.
  virtual bool AlwaysStream() const { return true; }

private:
  /// Holds our reference to the underlying data element
  csRef<SndSysSpeexSoundData> m_pSoundData;

  /// Ogg data structures for accessing the speex file.
  ogg_sync_state oy;
  ogg_page og;
  ogg_packet op;
  ogg_stream_state os;

  /// Speex data structures.
  void* state;
  SpeexHeader* header;
  SpeexBits bits;

  /// Marks whether a new page is required.
  bool newPage;

  /// True if the stream has finished being initialised.
  bool stream_init;

  /// Count of processed packets.
  uint packet_count;
};

#endif // #ifndef SNDSYS_STREAM_SPEEX_H


