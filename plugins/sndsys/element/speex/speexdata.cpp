/*
    Copyright (C) 2008 by Mike Gist

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

#include "cssysdef.h"

#include <speex/speex_header.h>

#include "speexdata.h"
#include "speexstream.h"

SndSysSpeexSoundData::SndSysSpeexSoundData (iBase *pParent, iDataBuffer* pDataBuffer) :
  SndSysBasicData(pParent), m_DataStore(pDataBuffer)
{
  m_SoundFormat.Bits = 16;
  m_SoundFormat.Channels = 2;
}

SndSysSpeexSoundData::~SndSysSpeexSoundData ()
{
}

size_t SndSysSpeexSoundData::GetDataSize()
{
  return m_DataStore.length;
}

iSndSysStream *SndSysSpeexSoundData::CreateStream(csSndSysSoundFormat *pRenderFormat, int Mode3D)
{
  if (!m_bInfoReady)
    Initialize();

  SndSysSpeexSoundStream *pStream = new SndSysSpeexSoundStream(this, pRenderFormat, Mode3D);

  return (pStream);
}


void SndSysSpeexSoundData::Initialize()
{
  ogg_sync_state oy;
  ogg_page og;
  ogg_packet op;
  ogg_stream_state os;

  // Process enough to get the header.
  ogg_sync_init(&oy);
  oy.data = m_DataStore.data;
  oy.storage = (int)m_DataStore.length;
  ogg_sync_wrote(&oy, (long)m_DataStore.length);
  ogg_sync_pageout(&oy, &og);
  ogg_stream_init(&os, ogg_page_serialno(&og));
  ogg_stream_pagein(&os, &og);
  ogg_stream_packetout(&os, &op);

  SpeexHeader* header = speex_packet_to_header((char*)op.packet, op.bytes);
  m_SoundFormat.Channels = header->nb_channels;
  m_SoundFormat.Freq = header->rate;

  // Is there a better way to do this?
  // Get number of packets (-1 as the second contains misc info).
  int count = -1;
  while(ogg_sync_pageout(&oy, &og)==1)
  {
    ogg_stream_pagein(&os, &og);
    count += ogg_page_packets(&og);
  }

  m_FrameCount = header->frames_per_packet * count;

  // Free memory.
  speex_header_free(header);
  ogg_stream_clear(&os);

  // No need to call this again.
  m_bInfoReady=true;
}

bool SndSysSpeexSoundData::IsSpeex(iDataBuffer* Buffer)
{
  SpeexDataStore datastore(Buffer);

  // Check capture pattern.
  if(memcmp(datastore.data, "OggS", 4))
    return false;

  // Find size of header.
  int headerbytes = datastore.data[26] + 27;

  // Check data for speex type.
  const char* str = "Speex   ";
  if(memcmp((void*)&datastore.data[headerbytes], (void*)str, 8))
    return false;

  return true;
}
