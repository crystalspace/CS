/*
    Copyright (C) 2005 by Andrew Mann
    Based in part on work by Norman Kraemer

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
#include "iutil/comp.h"
#include "isndsys/ss_loader.h"
#include "oggstream2.h"
#include "oggdata2.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (SndSysOggSoundData)
  SCF_IMPLEMENTS_INTERFACE (iSndSysData)
SCF_IMPLEMENT_IBASE_END

// As of 2004/10/05 the specs to which these functions (should) adhere is located at
// http://www.xiph.org/ogg/vorbis/doc/vorbisfile/callbacks.html

static size_t cs_ogg_read (void *ptr, size_t size, size_t nmemb, void *datasource)
{
  OggStreamData *streamdata = (OggStreamData *)datasource;
  OggDataStore *ds = streamdata->datastore;

  // size_t is unsigned, be careful with subtraction.  A 0 return indicates end of stream.
  if (ds->length <= (size_t)streamdata->position)
    return 0;
  size_t br = MIN (size*nmemb, ds->length - streamdata->position);

  memcpy (ptr, ds->data+streamdata->position, br);

  streamdata->position += br;
  return br;
}

static int cs_ogg_seek (void *datasource, ogg_int64_t offset, int whence)
{
  size_t np;
  OggStreamData *streamdata = (OggStreamData *)datasource;
  OggDataStore *ds = streamdata->datastore;

  if (whence == SEEK_SET)
    np = offset;
  else if (whence == SEEK_CUR)
    np = streamdata->position + offset;
  else if (whence == SEEK_END)
    np = ds->length + offset;
  else
    return -1;

  // Since np is unsigned, a negative value will wrap around to a huge positive value, and 
  //  inevitably be well beyond the length.
  if (np > ds->length)
    return -1;

  streamdata->position = np;
  return 0;
}

static int cs_ogg_close (void *)
{
  return 0;
}

static long cs_ogg_tell (void *datasource)
{
  OggStreamData *streamdata = (OggStreamData *)datasource;
  return streamdata->position;
}

cs_ov_callbacks::cs_ov_callbacks ()
{
  read_func = cs_ogg_read;
  seek_func = cs_ogg_seek;
  close_func = cs_ogg_close;
  tell_func = cs_ogg_tell;
}


cs_ov_callbacks ogg_callbacks;

cs_ov_callbacks *GetCallbacks()
{
  return &ogg_callbacks;
}

SndSysOggSoundData::SndSysOggSoundData (iBase *parent, uint8 *data, size_t len)
{
  SCF_CONSTRUCT_IBASE (parent);
#ifdef CS_LITTLE_ENDIAN
  endian = 0;
#else
  endian = 1;
#endif

  ds = new OggDataStore (data, len, true);
  fmt.Bits = 16;
  fmt.Channels = 2;
  data_ready = false;
}

SndSysOggSoundData::~SndSysOggSoundData ()
{
  delete ds;
  SCF_DESTRUCT_IBASE();
}

const csSndSysSoundFormat *SndSysOggSoundData::GetFormat()
{
  if (!data_ready)
    Initialize();
  return &fmt;
}

long SndSysOggSoundData::GetSampleCount()
{
  if (!data_ready)
    Initialize();
  return sample_count;
}

long SndSysOggSoundData::GetDataSize()
{
  return (long)(ds->length & 0x7FFFFFFF);
}

iSndSysStream *SndSysOggSoundData::CreateStream (
  csSndSysSoundFormat *renderformat, int mode3d)
{
  SndSysOggSoundStream *stream=new SndSysOggSoundStream(this, ds, renderformat, mode3d);

  return (stream);
}

void SndSysOggSoundData::Initialize()
{
  ogg_int64_t pcm_count;
  vorbis_info *v_info;
  OggStreamData *streamdata = new OggStreamData;

  streamdata->datastore=ds;
  streamdata->position=0;

  OggVorbis_File f;
  memset (&f, 0, sizeof(OggVorbis_File));
  /*bool ok = */ov_open_callbacks(streamdata, &f, 0, 0, 
    *(ov_callbacks*)GetCallbacks ()) /*== 0*/;

  pcm_count=ov_pcm_total(&f, -1);
  sample_count=(long)(pcm_count & 0x7FFFFFFF);


  v_info=ov_info(&f,-1);

  fmt.Freq=v_info->rate;
  fmt.Bits=v_info->bitrate_nominal;
  fmt.Channels=v_info->channels;

  ov_clear(&f);
  delete streamdata;
 
  data_ready=true;
}

bool SndSysOggSoundData::IsOgg (void *Buffer, size_t len)
{
  OggDataStore *dd = new OggDataStore ((uint8*)Buffer, len, false);
  OggStreamData *streamdata = new OggStreamData;

  streamdata->datastore=dd;
  streamdata->position=0;

  OggVorbis_File f;
  memset (&f, 0, sizeof(OggVorbis_File));
  bool ok = 
    ov_test_callbacks(streamdata, &f, 0, 0, *(ov_callbacks*)GetCallbacks ()) == 0;
  ov_clear (&f);
  delete dd;
  return ok;
}



