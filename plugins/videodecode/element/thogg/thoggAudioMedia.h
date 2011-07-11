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

public:
  ogg_stream_state  vo;
  vorbis_info       vi;
  vorbis_dsp_state  vd;
  vorbis_block      vb;
  vorbis_comment    vc;
  ogg_packet        op;
  ogg_page         *og;
  int               vorbis_p;

  FILE  *out,
        *infile;

  bool decodersStarted;
  bool audiobuf_ready;

public:
  TheoraAudioMedia (iBase* parent);
  virtual ~TheoraAudioMedia ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  virtual const char* GetType ();
  virtual unsigned long GetFrameCount ();
  virtual float GetLength();
  virtual void SetAudioTarget (csRef<iSndSysStream> stream);
  virtual double GetPosition ();
  virtual void CleanMedia () ;
  virtual int Update () ;
  void SetLength (float length)  { this->length=length; }
  void Seek (float time, ogg_sync_state *oy,ogg_page *op,ogg_stream_state *thState);
};

/** @} */

#endif // __CS_THOGGAUDIOMEDIA_H__
