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

#include "cssysdef.h"

#include "speexstream.h"

/* Keep these as integer values or the stream may feed portions of a sample to 
 * the higher layer which may cause problems
 * 1 , 1 will decode 1/1 or 1 second of audio in advance */
#define SPEEX_BUFFER_LENGTH_MULTIPLIER  1
#define SPEEX_BUFFER_LENGTH_DIVISOR     20

SndSysSpeexSoundStream::SndSysSpeexSoundStream (csRef<SndSysSpeexSoundData> pData, 
                                                csSndSysSoundFormat *pRenderFormat, 
                                                int Mode3D) : 
SndSysBasicStream(pRenderFormat, Mode3D), m_pSoundData(pData), header(0)
{
  // Allocate an advance buffer
  m_pCyclicBuffer = new SoundCyclicBuffer (
    (m_RenderFormat.Bits/8 * m_RenderFormat.Channels) * 
      (m_RenderFormat.Freq * SPEEX_BUFFER_LENGTH_MULTIPLIER / 
	SPEEX_BUFFER_LENGTH_DIVISOR));
  CS_ASSERT(m_pCyclicBuffer!=0);

  // Initialize speex stream.
  ResetPosition(false);
}

SndSysSpeexSoundStream::~SndSysSpeexSoundStream ()
{
  speex_header_free(header);
  ogg_stream_clear(&os);
}

const char *SndSysSpeexSoundStream::GetDescription()
{
  // Try to retrieve the description from the underlying data component.
  const char *pDesc=m_pSoundData->GetDescription();

  // If the data component has no description, return a default description.
  if (!pDesc)
    return "Speex Stream";
  return pDesc;
}

size_t SndSysSpeexSoundStream::GetFrameCount()
{
  const csSndSysSoundFormat *data_format=m_pSoundData->GetFormat();

  uint64 framecount=m_pSoundData->GetFrameCount();
  framecount*=m_RenderFormat.Freq;
  framecount/=data_format->Freq;

  return framecount;
}

bool SndSysSpeexSoundStream::ResetPosition(bool clear)
{
  // Clear the stream if needed.
  if(clear)
  {
    ogg_stream_clear(&os);
  }

  // Set up the sync state and buffers.
  speex_bits_init(&bits);
  ogg_sync_init(&oy);
  oy.data = m_pSoundData->GetDataStore().data;
  oy.storage = (int)m_pSoundData->GetDataStore().length;
  ogg_sync_wrote(&oy, (long)m_pSoundData->GetDataStore().length);

  // Reset flags and counters.
  stream_init = false;
  newPage = true;
  packet_count = 0;

  return true;
}

void SndSysSpeexSoundStream::AdvancePosition(size_t frame_delta)
{
  if (m_PauseState != CS_SNDSYS_STREAM_UNPAUSED || m_bPlaybackReadComplete || frame_delta==0)
    return;

  // Figure out how many bytes we need to fill for this advancement
  size_t needed_bytes = frame_delta * (m_RenderFormat.Bits/8) * m_RenderFormat.Channels;

  // If we need more space than is available in the whole cyclic buffer, then we already underbuffered, reduce to just 1 cycle full
  if ((size_t)needed_bytes > m_pCyclicBuffer->GetLength())
    needed_bytes=(size_t)(m_pCyclicBuffer->GetLength() & 0x7FFFFFFF);

  // Free space in the cyclic buffer if necessary
  if ((size_t)needed_bytes > m_pCyclicBuffer->GetFreeBytes())
    m_pCyclicBuffer->AdvanceStartValue(needed_bytes - (size_t)(m_pCyclicBuffer->GetFreeBytes() & 0x7FFFFFFF));

  // Fill in leftover decoded data if needed
  if (m_PreparedDataBufferUsage > 0)
    needed_bytes-=CopyBufferBytes(needed_bytes);

  while (needed_bytes > 0)
  {
    if(newPage)
    {
      if(ogg_sync_pageout(&oy, &og) != 1)
      {
        // Mark as complete if not looping.
        if (!m_bLooping)
        {
          m_bPlaybackReadComplete = true;
        }

        // Reset stream.
        ResetPosition();

        return;
      }

      if (!stream_init)
      {
        ogg_stream_init(&os, ogg_page_serialno(&og));
        stream_init = true;
      }

      if (ogg_page_serialno(&og) != os.serialno)
      {
        ogg_stream_reset_serialno(&os, ogg_page_serialno(&og));
      }

      ogg_stream_pagein(&os, &og);
      newPage = false;
    }

    if(ogg_stream_packetout(&os, &op) != 1)
    {
      newPage = true;
      continue;
    }

    // First packets contain header data.
    if(packet_count == 0)
    {
      if(header)
      {
        speex_header_free(header);
      }

      header = speex_packet_to_header((char*)op.packet, op.bytes);
      // const_cast for version compatibility.
      SpeexMode* mode = const_cast<SpeexMode*>(speex_lib_get_mode (header->mode));
      state = speex_decoder_init(mode);
      speex_decoder_ctl(state, SPEEX_SET_SAMPLING_RATE, &header->rate);

      m_OutputFrequency=m_NewOutputFrequency;

      // Create the pcm sample converter if it's not yet created
      if (m_pPCMConverter == 0)
        m_pPCMConverter = new PCMSampleConverter (
        m_RenderFormat.Channels, m_RenderFormat.Bits,
        m_RenderFormat.Freq);

      // Calculate the size of one source sample
      int source_sample_size = m_RenderFormat.Channels * m_RenderFormat.Bits;

      // Calculate the needed buffer size for this conversion
      int needed_buffer = (m_pPCMConverter->GetRequiredOutputBufferMultiple (
        m_RenderFormat.Channels, m_RenderFormat.Bits, m_OutputFrequency) * 
        (4096 + source_sample_size))/1024;

      // Allocate a new buffer.
      if (m_PreparedDataBufferSize < needed_buffer)
      {
        delete[] m_pPreparedDataBuffer;
        m_pPreparedDataBuffer = new char[needed_buffer];
        m_PreparedDataBufferSize=needed_buffer;
      }
    }

    if(packet_count++ < uint(2+header->extra_headers))
    {
      continue;
    }

    // Read and decode.
    speex_bits_read_from(&bits, (char*)op.packet, op.bytes);
    speex_decode_int(state, &bits, (int16*)m_pPreparedDataBuffer);

    // Frame size is in shorts.
    speex_decoder_ctl(state, SPEEX_GET_FRAME_SIZE, &m_PreparedDataBufferUsage);
    m_PreparedDataBufferUsage *= sizeof(short);

    if (m_PreparedDataBufferUsage > 0)
      needed_bytes -= CopyBufferBytes (needed_bytes);
  }
}

