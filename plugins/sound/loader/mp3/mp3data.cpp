/*
    Copyright (C) 2001 by Norman Krämer
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.
  
    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "mp3data.h"
#include "isound/loader.h"
#include "iutil/comp.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csMp3SoundData)
  SCF_IMPLEMENTS_INTERFACE (iSoundData)
SCF_IMPLEMENT_IBASE_END

csMp3SoundData::csMp3SoundData (iBase *parent, uint8 *data, size_t len)
{
  SCF_CONSTRUCT_IBASE (parent);

  ds = new datastore (data, len, true);
  mp3_ok = false;
  this->len = 8192;
  bytes_left = 0;
  buf = (uint8*)malloc (this->len*2);
  pos = buf;
  fmt.Bits = 16;
  fmt.Channels = 2;
  InitMP3 (&mp);
}

csMp3SoundData::~csMp3SoundData ()
{
  ExitMP3 (&mp);
  delete [] buf;
  delete ds;
}

bool csMp3SoundData::Initialize(const csSoundFormat* )
{
  if (!mp3_ok)
  {
    // try to convert the first few frames
    mp3_ok = decodeMP3(&mp, (char*)ds->data, ds->length, (char*)buf, len, &bytes_left) == MP3_OK;
  }
  return mp3_ok;
}

bool csMp3SoundData::IsMp3 (void *Buffer, size_t len)
{
  char buf[8192];
  int pos;
  mpstr m;
  InitMP3 (&m);
  bool ok = decodeMP3(&m, (char*)Buffer, len, (char*)buf, 8192, &pos) == MP3_OK;
  ExitMP3 (&m);
  return ok;
}

const csSoundFormat* csMp3SoundData::GetFormat()
{
  return &fmt;
}

bool csMp3SoundData::IsStatic()
{
  return false;
}

long csMp3SoundData::GetStaticSampleCount()
{
  return -1;
}

void *csMp3SoundData::GetStaticData()
{
  return NULL;
}

void csMp3SoundData::ResetStreamed()
{
  ExitMP3 (&mp);
  InitMP3 (&mp);
  bytes_left = 0;
  pos = buf;
  mp3_ok = decodeMP3(&mp, (char*)ds->data, ds->length, (char*)buf, len, &bytes_left) == MP3_OK;
}

void *csMp3SoundData::ReadStreamed(long &NumSamples)
{
  if (mp3_ok)
  {
    uint8 *p;
    if (!bytes_left)
    {
      mp3_ok = decodeMP3(&mp, NULL, 0, (char*)buf, len, &bytes_left) == MP3_OK;
      p = pos = buf;
    }
    else
      p = pos;

    if (mp3_ok)
    {
      long samples = bytes_left / ((fmt.Bits >> 3) * fmt.Channels);
      if (samples > NumSamples)
      {
	pos += NumSamples * ((fmt.Bits >> 3) * fmt.Channels);
	bytes_left -= NumSamples * ((fmt.Bits >> 3) * fmt.Channels);
      }
      else
      {
	NumSamples = samples;
	bytes_left = 0;
      }

      return p;
    }
  }

  NumSamples = 0;
  return NULL;
}


// mp3 loader

class csMp3Loader : public iSoundLoader
{
public:
  SCF_DECLARE_IBASE;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csMp3Loader);
    virtual bool Initialize (iObjectRegistry *){return true;}
  }scfiComponent;

  csMp3Loader (iBase *parent)
  {
    SCF_CONSTRUCT_IBASE (parent);
    SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  }

  virtual iSoundData *LoadSound (void *Buffer, unsigned long Size) const
  {
    csMp3SoundData *sd=NULL;
    if (csMp3SoundData::IsMp3 (Buffer, Size))
      sd = new csMp3SoundData ((iBase*)this, (uint8*)Buffer, Size);
    return sd;
  }
};

SCF_IMPLEMENT_IBASE (csMp3Loader)
  SCF_IMPLEMENTS_INTERFACE (iSoundLoader)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMp3Loader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csMp3Loader);

SCF_EXPORT_CLASS_TABLE (sndmp3)
  SCF_EXPORT_CLASS (csMp3Loader, "crystalspace.sound.loader.mp3", "Mp3 Sound Loader")
SCF_EXPORT_CLASS_TABLE_END

