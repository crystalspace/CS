/*
Copyright (C) 2011 by Alin Baciu

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

#ifndef __CS_THOGGAUDIOMEDIA_H__
#define __CS_THOGGAUDIOMEDIA_H__

/**\file
  * Video Player: media stream 
  */

#include <iutil/comp.h>
#include <ivideodecode/media.h>
#include <csutil/scf_implementation.h>

#include <vorbis/codec.h>

#include <iostream>
#include <stdio.h>
using namespace std;

struct iTextureWrapper; 
struct csVPLvideoFormat;


/**
  * Audio stream
  */
class TheoraAudioMedia : public scfImplementation2 < TheoraAudioMedia, iAudioMedia, scfFakeInterface<iMedia> >
{
private:
  iObjectRegistry*      object_reg;
  csRef<iSndSysStream>	_stream;
  float                 length;

  ogg_stream_state  _streamState;
  vorbis_info       _streamInfo;
  vorbis_dsp_state  _dspState;
  vorbis_block      _vorbisBlock;
  vorbis_comment    _streamComments;
  ogg_packet        _oggPacket;
  ogg_page          *_oggPage;
  int               _vorbis_p;
  FILE              *_infile;
  bool              _decodersStarted;
  bool              _audiobuf_ready;
  
public:
  
  // Provide access to the Vorbis specific members
  // Inline because it's faster, although a bit slow
  inline ogg_stream_state*   StreamState()    { return &_streamState; }
  inline vorbis_info*        StreamInfo()     { return &_streamInfo; }
  inline vorbis_dsp_state*   DspState()       { return &_dspState; }
  inline vorbis_block*       VorbisBlock()    { return &_vorbisBlock; }
  inline vorbis_comment*     StreamComments() { return &_streamComments; }
  inline int&                Vorbis_p()       { return _vorbis_p; }

  // An easy way to initialize the stream
  void InitializeStream (ogg_stream_state &state, vorbis_info &info, vorbis_comment &comments, 
    FILE *source);

public:
  TheoraAudioMedia (iBase* parent);
  virtual ~TheoraAudioMedia ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  virtual const char* GetType () const;
  virtual unsigned long GetFrameCount () const;
  virtual float GetLength() const;
  virtual void SetAudioTarget (csRef<iSndSysStream> stream);
  virtual double GetPosition () const;
  virtual void CleanMedia () ;
  virtual bool Update () ;

  virtual void SwapBuffers() ;

  inline void SetLength (float length)  { this->length=length; }
  void Seek (float time, ogg_sync_state *oy,ogg_page *op,ogg_stream_state *thState);
};

/** @} */

#endif // __CS_THOGGAUDIOMEDIA_H__
