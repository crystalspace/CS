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
#include "oggdata.h"
#include "isound/loader.h"
#include "iutil/comp.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csOggSoundData)
  SCF_IMPLEMENTS_INTERFACE (iSoundData)
SCF_IMPLEMENT_IBASE_END

static size_t cs_ogg_read (void *ptr, size_t size, size_t nmemb, void *datasource)
{
  csOggSoundData::datastore *ds = (csOggSoundData::datastore*)datasource;
  size_t br = MIN (size*nmemb, ds->length-ds->pos);

  memcpy (ptr, ds->data+ds->pos, br);

  ds->pos +=br;
  return br;
}

static int cs_ogg_seek (void *datasource, int64_t offset, int whence)
{
  size_t np;
  csOggSoundData::datastore *ds = (csOggSoundData::datastore*)datasource;

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

static int cs_ogg_close (void *)
{
  return 0;
}

static long cs_ogg_tell (void *datasource)
{
  csOggSoundData::datastore *ds = (csOggSoundData::datastore*)datasource;
  return ds->pos;
}

csOggSoundData::cs_ov_callbacks::cs_ov_callbacks ()
{
  read_func = cs_ogg_read;
  seek_func = cs_ogg_seek;
  close_func = cs_ogg_close;
  tell_func = cs_ogg_tell;
}

csOggSoundData::cs_ov_callbacks csOggSoundData::callbacks;

csOggSoundData::csOggSoundData (iBase *parent, uint8 *data, size_t len)
{
  SCF_CONSTRUCT_IBASE (parent);
#ifdef CS_LITTLE_ENDIAN
  endian = 0;
#else
  endian = 1;
#endif

  ds = new datastore (data, len, true);
  ogg_ok = false;
  buf = NULL;
  len = 0;
  current_section = 0;
  fmt.Bits = 16;
  fmt.Channels = 2;
}

csOggSoundData::~csOggSoundData ()
{
  ov_clear (&vf);
  delete [] buf;
  delete ds;
}

bool csOggSoundData::Initialize(const csSoundFormat *fmt)
{
  if (!ogg_ok)
  {
    ogg_ok = ov_open_callbacks(ds, &vf, NULL, 0, *(ov_callbacks*)&callbacks) == 0;
    if (fmt->Channels != -1)
      this->fmt.Channels = fmt->Channels;
    if (fmt->Freq != -1)
      this->fmt.Freq = fmt->Freq;

  }
  return ogg_ok;
}

bool csOggSoundData::IsOgg (void *Buffer, size_t len)
{
  datastore *dd = new datastore ((uint8*)Buffer, len, false);
  OggVorbis_File f;
  bool ok = ov_open_callbacks(dd, &f, NULL, 0, *(ov_callbacks*)&callbacks) == 0;
  ov_clear (&f);
  delete dd;
  return ok;
}

const csSoundFormat* csOggSoundData::GetFormat()
{
  return &fmt;
}

bool csOggSoundData::IsStatic()
{
  return false;
}

long csOggSoundData::GetStaticSampleCount()
{
  return -1;
}

void *csOggSoundData::GetStaticData()
{
  return NULL;
}

void csOggSoundData::ResetStreamed()
{
  ogg_ok = ov_raw_seek (&vf, 0) == 0;
}

void *csOggSoundData::ReadStreamed(long &NumSamples)
{
  if (ogg_ok)
  {
    long buffersize = NumSamples * (fmt.Bits >> 3) * fmt.Channels;

    if (buffersize > len)
    {
      buf = (uint8*) realloc (buf, buffersize);
      len = buffersize;
    }

    long bytes_read = ov_read (&vf, (char*)buf, buffersize, endian,
			       fmt.Bits>>3, 1, &current_section);

    NumSamples = bytes_read / ((fmt.Bits >> 3) * fmt.Channels);
    return buf;
  }

  NumSamples = 0;
  return NULL;
}


// ogg loader

class csOggLoader : public iSoundLoader
{
public:
  SCF_DECLARE_IBASE;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csOggLoader);
    virtual bool Initialize (iObjectRegistry *){return true;}
  }scfiComponent;

  csOggLoader (iBase *parent)
  {
    SCF_CONSTRUCT_IBASE (parent);
    SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  }

  virtual iSoundData *LoadSound (void *Buffer, unsigned long Size) const
  {
    csOggSoundData *sd=NULL;
    if (csOggSoundData::IsOgg (Buffer, Size))
      sd = new csOggSoundData ((iBase*)this, (uint8*)Buffer, Size);
    return sd;
  }
};

SCF_IMPLEMENT_IBASE (csOggLoader)
  SCF_IMPLEMENTS_INTERFACE (iSoundLoader)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csOggLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csOggLoader);

SCF_EXPORT_CLASS_TABLE (sndogg)
  SCF_EXPORT_CLASS (csOggLoader, "crystalspace.sound.loader.ogg", "Ogg Vorbis Sound Loader")
SCF_EXPORT_CLASS_TABLE_END

