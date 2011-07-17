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

#ifndef __CS_THOGGVIDEOMEDIA_H__
#define __CS_THOGGVIDEOMEDIA_H__

/**\file
* Video Player: media stream 
*/

#include <iutil/comp.h>
#include <ivideodecode/media.h>
#include <csutil/scf_implementation.h>

// theora headers
#include "theora/theoradec.h"

#include <iostream>
using namespace std;

#define QUALIFIED_PLUGIN_NAME "crystalspace.vpl.element.thogg"

struct iTextureWrapper; 
struct csVPLvideoFormat;

/**
  * Video stream
  */
class TheoraVideoMedia : public scfImplementation2< TheoraVideoMedia, iVideoMedia, scfFakeInterface<iMedia> >
{
private:
  iObjectRegistry* object_reg;
  float            length;
  unsigned long    frameCount;

  ogg_int64_t      _videobuf_granulepos;
  ogg_int64_t      _frameToSkip;

  csRef<iTextureHandle> _texture;
  csRefArray<iTextureHandle> _buffers;

  // these will be private and have getters and setters, but for now, it's faster like this
public:
  int activeBuffer;

  ogg_stream_state  _streamState;
  th_info           _streamInfo;
  th_comment        _streamComments;
  th_dec_ctx       *_decodeControl;
  ogg_packet        _oggPacket;
  th_setup_info    *_setupInfo;
  int               _theora_p;
  FILE    *_infile;
  bool    _decodersStarted;
  bool    _videobufReady;
  double  _videobufTime;

  void InitializeStream (ogg_stream_state &state, th_info &info, th_comment &comments, th_setup_info *setupInfo,
    FILE *source, csRef<iTextureManager> texManager);

public:
  TheoraVideoMedia (iBase* parent);
  virtual ~TheoraVideoMedia ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  virtual const char* GetType () const;
  virtual const csVPLvideoFormat* GetFormat() const;
  virtual unsigned long GetFrameCount() const;
  virtual float GetLength() const;
  virtual void SetVideoTarget (csRef<iTextureHandle> &texture);
  virtual double GetPosition () const;
  virtual void CleanMedia () ;
  virtual bool Update () ;

  virtual void SwapBuffers() ;

  inline void SetFrameCount (unsigned long count)  { frameCount=count; }
  inline void SetLength (float length)  { this->length=length; }
  long SeekPage(long targetFrame,bool return_keyframe, ogg_sync_state *oy,unsigned  long fileSize);
};


#endif // __CS_THOGGVIDEOMEDIA_H__
