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

CS_IMPLEMENT_PLATFORM_PLUGIN

SCF_IMPLEMENT_IBASE (csMp3SoundData)
  SCF_IMPLEMENTS_INTERFACE (iSoundData)
SCF_IMPLEMENT_IBASE_END


csMp3SoundData::myCallback::myCallback ()
{
  read  = myread;
  seek  = myseek;
  close = myclose;
  tell  = mytell;
}

size_t csMp3SoundData::myCallback::myread  (void *ptr, size_t size, void *datasource)
{
  csMp3SoundData::datastore *ds = (csMp3SoundData::datastore *)datasource;

  size_t br = MIN (size, ds->length-ds->pos);
  //  printf ("read from %p %ld bytes into %p\n", ds->data + ds->pos, br, ptr);
  memcpy (ptr, ds->data+ds->pos, br);

  ds->pos +=br;
  return br;
}

int csMp3SoundData::myCallback::myseek  (int offset, int whence, void *datasource)
{
  size_t np;
  csMp3SoundData::datastore *ds = (csMp3SoundData::datastore*)datasource;

  if (whence == SEEK_SET)
    np = offset;
  else if (whence == SEEK_CUR)
    np = ds->pos + offset;
  else if (whence == SEEK_END)
    np = ds->length + offset;
  else
    return -1;

  if (np > ds->length)
    return -1;

  ds->pos = np;
  return 0;
}

int csMp3SoundData::myCallback::myclose (void *)
{
  return 0;
}

long csMp3SoundData::myCallback::mytell  (void *datasource)
{
  csMp3SoundData::datastore *ds = (csMp3SoundData::datastore*)datasource;
  return ds->pos;
}

csMp3SoundData::csMp3SoundData (iBase *parent, uint8 *data, size_t len)
{
  SCF_CONSTRUCT_IBASE (parent);

  ds = new datastore (data, len, true);
  mp3_ok = false;
  bytes_left = 0;
  fmt.Bits = 16;
  fmt.Channels = 2;
  pos = NULL;
  mp = new csMPGFrame (ds, (ioCallback*)&cb, AUDIO_FORMAT_SIGNED_8, fmt.Bits, 0);
}

csMp3SoundData::~csMp3SoundData ()
{
  delete mp;
  delete ds;
}

bool csMp3SoundData::Initialize(const csSoundFormat* )
{
  if (!mp3_ok)
  {
    mp3_ok = mp->Read (); // read frame
    if (mp3_ok)
      mp->Decode ();
  }
  return mp3_ok;
}

bool csMp3SoundData::IsMp3 (void *Buffer, size_t len)
{
  myCallback cb;
  datastore ds ((uint8*)Buffer, len, false);
  csMPGFrame *mpframe = new csMPGFrame (&ds, (ioCallback*)&cb, AUDIO_FORMAT_SIGNED_8, 16, 0);
  bool ok = mpframe->Read ();
  //  ok = ok && mpframe->junk < 20; // this doesnt really work :(
  //  printf("ok = %d, junk %d len = %d\n", ok, mpframe->junk, len);
  delete mpframe;
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
  mp->Rewind ();
  bytes_left = 0;
  mp3_ok = true;
}

void *csMp3SoundData::ReadStreamed(long &NumSamples)
{
  if (mp3_ok)
  {
    csPCMBuffer *pcm = mp->GetPCMBuffer ();
    unsigned char *p;

    //    printf ("requesting %d samples, ", NumSamples);

    if (!bytes_left)
    {
      mp3_ok = mp->Read ();
      if (mp3_ok)
      {
	mp->Decode ();
	bytes_left = pcm->pos;
	pos = pcm->buffer;
      }
      else
      {
	NumSamples = 0;
	return NULL;
      }
    }

    p = pos;

    long samples = bytes_left / ((fmt.Bits >> 3) * fmt.Channels);

    //    printf ("returning %d samples\n", samples);

    if (samples > NumSamples)
    {
      pos += NumSamples * ((fmt.Bits >> 3) * fmt.Channels);
      bytes_left -= NumSamples * ((fmt.Bits >> 3) * fmt.Channels);
    }
    else
    {
      NumSamples = samples;
      bytes_left = 0;
      pcm->pos = 0;
    }

    return p;
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
    {
      sd = new csMp3SoundData ((iBase*)this, (uint8*)Buffer, Size);
    }
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

