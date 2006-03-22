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
    np = (size_t)(streamdata->position + offset);
  else if (whence == SEEK_END)
    np = (size_t)(ds->length + offset);
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
  return (long)streamdata->position;
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

SndSysOggSoundData::SndSysOggSoundData (iBase *pParent, iDataBuffer* pDataBuffer) :
  scfImplementationType(this, pParent),
  // Create a new DataStore associated with the passed DataBuffer
  m_DataStore(pDataBuffer), m_pDescription(0)
{
  // Set some default format information
  m_SoundFormat.Bits = 16;
  m_SoundFormat.Channels = 2;

  // There is currently no data ready
  m_DataReady = false;
}

SndSysOggSoundData::~SndSysOggSoundData ()
{
  delete[] m_pDescription;
}

const csSndSysSoundFormat *SndSysOggSoundData::GetFormat()
{
  if (!m_DataReady)
    Initialize();
  return &m_SoundFormat;
}

size_t SndSysOggSoundData::GetFrameCount()
{
  if (!m_DataReady)
    Initialize();
  return m_FrameCount;
}

size_t SndSysOggSoundData::GetDataSize()
{
  return m_DataStore.length;
}

iSndSysStream *SndSysOggSoundData::CreateStream (
  csSndSysSoundFormat *pRenderFormat, int Mode3D)
{
  // Creating a stream from the data is a simple operation for Ogg
  SndSysOggSoundStream *pStream=new SndSysOggSoundStream(this, &m_DataStore, pRenderFormat, Mode3D);

  return (pStream);
}

void SndSysOggSoundData::SetDescription(const char *pDescription)
{
  delete[] m_pDescription;
  m_pDescription=0;

  if (!pDescription)
    return;

  m_pDescription=new char[strlen(pDescription)+1];
  strcpy(m_pDescription, pDescription);
}


void SndSysOggSoundData::Initialize()
{
  ogg_int64_t pcm_count;
  vorbis_info *v_info;
  OggStreamData StreamData;

  // Setup a temporary OggStreamData so that we can open callbacks and get information about the audio
  //  We'll tear this all down as soon as we've stored the information we want.
  StreamData.datastore=&m_DataStore;
  StreamData.position=0;

  // ov_open_callbacks prepares the data for decoding, and allows us to retrieve properties of the ogg vorbis sound stream
  OggVorbis_File f;
  memset (&f, 0, sizeof(OggVorbis_File));
  /*bool ok = */ov_open_callbacks(&StreamData, &f, 0, 0, 
    *(ov_callbacks*)GetCallbacks ()) /*== 0*/;

  // The ogg vorbis functions can deal with 64 bit counts of samples, we only use 32 bit for now
  pcm_count=ov_pcm_total(&f, -1);

  // Retrieve and store the sound format information
  v_info=ov_info(&f,-1);
  m_SoundFormat.Freq=v_info->rate;
  m_SoundFormat.Bits=v_info->bitrate_nominal;
  m_SoundFormat.Channels=v_info->channels;

  m_FrameCount=(long)((pcm_count / v_info->channels) & 0x7FFFFFFF);

  // Cleanup
  ov_clear(&f);

  // No need to call this again
  m_DataReady=true;
}


bool SndSysOggSoundData::IsOgg (iDataBuffer* Buffer)
{
  // This is a static function, no member variables are used
  OggDataStore datastore(Buffer);
  OggStreamData streamdata;

  streamdata.datastore=&datastore;
  streamdata.position=0;

  OggVorbis_File f;
  memset (&f, 0, sizeof(OggVorbis_File));
  bool ok = 
    ov_test_callbacks(&streamdata, &f, 0, 0, *(ov_callbacks*)GetCallbacks ()) == 0;
  ov_clear (&f);
  return ok;
}



