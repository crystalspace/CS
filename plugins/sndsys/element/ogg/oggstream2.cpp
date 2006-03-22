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
#include "isndsys/ss_stream.h"
#include "csplugincommon/sndsys/convert.h"
#include "csplugincommon/sndsys/cyclicbuf.h"
#include "oggdata2.h"
#include "oggstream2.h"

#include "ivaria/reporter.h"

/* Keep these as integer values or the stream may feed portions of a sample to 
 * the higher layer which may cause problems
 * 1 , 1 will decode 1/1 or 1 second of audio in advance */
#define OGG_BUFFER_LENGTH_MULTIPLIER  1
#define OGG_BUFFER_LENGTH_DIVISOR     5

/**
 * The size in bytes of the buffer in which decoded ogg data is stored before 
 * copying/conversion
 */
#define OGG_DECODE_BUFFER_SIZE 4096

extern cs_ov_callbacks *GetCallbacks();

const size_t positionInvalid = (size_t)~0;

SndSysOggSoundStream::SndSysOggSoundStream (csRef<SndSysOggSoundData> data, 
					    OggDataStore *datastore, csSndSysSoundFormat *renderformat, 
              int mode3d) :
  scfImplementationType(this)
{
  // Copy render format information
  memcpy(&m_RenderFormat,renderformat,sizeof(csSndSysSoundFormat));

  // Initialize the stream data
  m_StreamData.datastore=datastore;
  m_StreamData.position=0;

  m_SoundData=data;

  // Allocate an advance buffer
  m_pCyclicBuffer = new CS::Sound::SoundCyclicBuffer (
    (m_RenderFormat.Bits/8 * m_RenderFormat.Channels) * 
      (m_RenderFormat.Freq * OGG_BUFFER_LENGTH_MULTIPLIER / 
	OGG_BUFFER_LENGTH_DIVISOR));
  CS_ASSERT(m_pCyclicBuffer!=0);

  // Initialize ogg file
  memset(&m_VorbisFile,0,sizeof(OggVorbis_File));
  ov_open_callbacks (&m_StreamData,&m_VorbisFile,0,0,
    *(ov_callbacks*)GetCallbacks());

  // Start the most advanced read pointer at offset 0
  m_MostAdvancedReadPointer=0;

  // Set stream to m_bPaused, at the beginning, and not m_bLooping
  m_bPaused=true;
  m_bLooping=false;

  // A prepared data buffer will be allocated when we know the size needed
  m_pPreparedDataBuffer=0;
  m_PreparedDataBufferSize=0;
  m_PreparedDataBufferStart=0;

  // Set the render sample size
  m_RenderFrameSize=(renderformat->Bits/8) * renderformat->Channels;

  // No extra data left in the decode buffer since last call
  m_PreparedDataBufferUsage=0;

  // Playback rate is initially 100% (normal)
  m_PlaybackPercent=100;

  /* Forces the output buffer to be resized and the converter to be created on 
   * the first pass */
  m_OutputFrequency=0;

  // Output frequency is initially the same as the render frequency
  m_NewOutputFrequency=renderformat->Freq;

  // Set to not a valid stream
  m_CurrentOggStream=-1;

  // Let the pcm converter get created on the first pass
  m_pPCMConverter=0;

  // No new position 
  m_NewPosition = positionInvalid;

  // Store the 3d mode
  m_3DMode=mode3d;

  // Auto m_bAutoUnregisterReady not requested
  m_bAutoUnregisterRequested=false;

  // Not ready to be unregistered
  m_bAutoUnregisterReady=false;

  // Playback not complete
  m_bPlaybackReadComplete=false;
}

SndSysOggSoundStream::~SndSysOggSoundStream ()
{
  delete m_pCyclicBuffer;
  delete m_pPCMConverter;
  delete[] m_pPreparedDataBuffer;
}

const char *SndSysOggSoundStream::GetDescription()
{
  // Try to retrieve the description from the underlying data component.
  const char *pDesc=m_SoundData->GetDescription();

  // If the data component has no description, return a default description.
  if (!pDesc)
    return "Ogg Stream";
  return pDesc;
}


const csSndSysSoundFormat *SndSysOggSoundStream::GetRenderedFormat()
{
  return &m_RenderFormat;
}

int SndSysOggSoundStream::Get3dMode()
{
  return m_3DMode;
}


size_t SndSysOggSoundStream::GetFrameCount()
{
  const csSndSysSoundFormat *data_format=m_SoundData->GetFormat();

  uint64 framecount=m_SoundData->GetFrameCount();
  framecount*=m_RenderFormat.Freq;
  framecount/=data_format->Freq;

  return framecount;
}


size_t SndSysOggSoundStream::GetPosition()
{
  return m_MostAdvancedReadPointer;

}

bool SndSysOggSoundStream::ResetPosition()
{
  m_NewPosition=0;
  return true;
}

bool SndSysOggSoundStream::SetPosition (size_t newposition)
{
  m_NewPosition=newposition;
  return true;
}

bool SndSysOggSoundStream::Pause()
{
  m_bPaused=true;
  return true;
}


bool SndSysOggSoundStream::Unpause()
{
  m_bPaused=false;
  return true;
}

int SndSysOggSoundStream::GetPauseState()
{
  if (m_bPaused)
    return CS_SNDSYS_STREAM_PAUSED;
  return CS_SNDSYS_STREAM_UNPAUSED;
}

bool SndSysOggSoundStream::SetLoopState(int loopstate)
{
  switch (loopstate)
  {
    case CS_SNDSYS_STREAM_DONTLOOP:
      m_bLooping=false;
      break;
    case CS_SNDSYS_STREAM_LOOP:
      m_bLooping=true;
      break;
    default:
      return false; // Looping mode not supported
  }
  return true;
}

/** 
 * Retrieves the loop state of the stream.  Current possible returns are 
 * CS_SNDSYS_STREAM_DONTLOOP and CS_SNDSYS_STREAM_LOOP
 */
int SndSysOggSoundStream::GetLoopState()
{
  if (m_bLooping)
    return CS_SNDSYS_STREAM_LOOP;
  return CS_SNDSYS_STREAM_DONTLOOP;
}

void SndSysOggSoundStream::SetPlayRatePercent(int percent)
{
  m_PlaybackPercent=percent;
  m_NewOutputFrequency = (m_RenderFormat.Freq * 100) / m_PlaybackPercent;
}

int SndSysOggSoundStream::GetPlayRatePercent()
{
  return m_PlaybackPercent;
}

/** 
 * If AutoUnregister is set, when the stream is paused it, and all sources 
 * attached to it are removed from the sound engine
 */
void SndSysOggSoundStream::SetAutoUnregister(bool autounreg)
{
  m_bAutoUnregisterRequested=autounreg;
}

/** 
 * If AutoUnregister is set, when the stream is m_bPaused it, and all sources 
 * attached to it are removed from the sound engine
 */
bool SndSysOggSoundStream::GetAutoUnregister()
{
  return m_bAutoUnregisterRequested;
}

/// Used by the sound renderer to determine if this stream needs to be unregistered
bool SndSysOggSoundStream::GetAutoUnregisterRequested()
{
  return m_bAutoUnregisterReady;
}


void SndSysOggSoundStream::AdvancePosition(size_t frame_delta)
{
  size_t needed_bytes=0;
  if (m_NewPosition != positionInvalid)
  {
    // Signal a full cyclic buffer flush
    needed_bytes=m_pCyclicBuffer->GetLength();

    // Flush the prepared samples too
    m_PreparedDataBufferUsage=0;
    m_PreparedDataBufferStart=0;

    // Seek the ogg stream to the requested position
    ov_raw_seek(&m_VorbisFile,m_NewPosition);

    m_NewPosition = positionInvalid;
    m_bPlaybackReadComplete=false;
  }
  if (m_bPaused || m_bPlaybackReadComplete || frame_delta==0)
    return;

 
  long bytes_read=0;
  

  // Figure out how many bytes we need to fill for this advancement
  if (needed_bytes==0)
    needed_bytes=frame_delta * (m_RenderFormat.Bits/8) * m_RenderFormat.Channels ;

  /* If we need more space than is available in the whole cyclic buffer, 
   * then we already underbuffered, reduce to just 1 cycle full
   */
  if ((size_t)needed_bytes > m_pCyclicBuffer->GetLength())
    needed_bytes=(long)(m_pCyclicBuffer->GetLength() & 0x7FFFFFFF);

  // Free space in the cyclic buffer if necessary
  if (needed_bytes > m_pCyclicBuffer->GetFreeBytes())
    m_pCyclicBuffer->AdvanceStartValue (needed_bytes - 
      m_pCyclicBuffer->GetFreeBytes());

  // Fill in leftover decoded data if needed
  if (m_PreparedDataBufferUsage > 0)
    needed_bytes -= CopyBufferBytes (needed_bytes);

  while (needed_bytes > 0)
  {
    int last_ogg_stream=m_CurrentOggStream;
    char ogg_decode_buffer[OGG_DECODE_BUFFER_SIZE];

    bytes_read=0;

    while (bytes_read==0)
    {
      bytes_read = ov_read (&m_VorbisFile, ogg_decode_buffer,
	OGG_DECODE_BUFFER_SIZE, OGG_ENDIAN,
	(m_RenderFormat.Bits==8)?1:2, (m_RenderFormat.Bits==8)?0:1,
	&m_CurrentOggStream);
   
      // Assert on error
      CS_ASSERT(bytes_read >=0);

      if (bytes_read <= 0)
      {
        if (!m_bLooping)
        {
          // Seek back to the beginning for a restart.  Pause on the next call
          m_bPlaybackReadComplete=true;
          ov_raw_seek(&m_VorbisFile,0);
          return;
        }

        // Loop by resetting the position to the beginning and continuing
        ov_raw_seek(&m_VorbisFile,0);
      }
    }


    

    // If streams changed, the format may have changed as well
    if ((m_NewOutputFrequency != m_OutputFrequency) 
      || (last_ogg_stream != m_CurrentOggStream))
    {
      int needed_buffer,source_sample_size;

      m_OutputFrequency=m_NewOutputFrequency;

      m_pCurrentOggFormatInfo=ov_info(&m_VorbisFile,m_CurrentOggStream);

      // Create the pcm sample converter if it's not yet created
      if (m_pPCMConverter == 0)
        m_pPCMConverter = new CS::Sound::PCMSampleConverter (
	  m_pCurrentOggFormatInfo->channels, m_RenderFormat.Bits,
	  m_pCurrentOggFormatInfo->rate);

      // Calculate the size of one source sample
      source_sample_size=m_pCurrentOggFormatInfo->channels * m_RenderFormat.Bits;

      // Calculate the needed buffer size for this conversion
      needed_buffer = (m_pPCMConverter->GetRequiredOutputBufferMultiple (
	m_RenderFormat.Channels,m_RenderFormat.Bits,m_OutputFrequency) * 
	  (OGG_DECODE_BUFFER_SIZE + source_sample_size))/1024;

      // Allocate a new buffer if needed - this will only happen if the source rate changes
      if (m_PreparedDataBufferSize < needed_buffer)
      {
        delete[] m_pPreparedDataBuffer;
        m_pPreparedDataBuffer = new char[needed_buffer];
        m_PreparedDataBufferSize=needed_buffer;
      }
    }

    // If no conversion is necessary 
    if ((m_pCurrentOggFormatInfo->rate == m_OutputFrequency) &&
        (m_pCurrentOggFormatInfo->channels == m_RenderFormat.Channels))
    {
      CS_ASSERT(bytes_read <= m_PreparedDataBufferSize);
      memcpy(m_pPreparedDataBuffer,ogg_decode_buffer,bytes_read);
      m_PreparedDataBufferUsage=bytes_read;
    }
    else
    {
      m_PreparedDataBufferUsage = m_pPCMConverter->ConvertBuffer (ogg_decode_buffer,
	bytes_read, m_pPreparedDataBuffer, m_RenderFormat.Channels,
	m_RenderFormat.Bits,m_OutputFrequency);
    }

    if (m_PreparedDataBufferUsage > 0)
      needed_bytes -= CopyBufferBytes (needed_bytes);

  }
      
}


size_t SndSysOggSoundStream::CopyBufferBytes(size_t max_dest_bytes)
{
  // If the entire prepared buffer can fit into the cyclic buffer, copy it
  //  there and reset the prepared buffer so we can use to it from the
  //  beginning again.
  if (max_dest_bytes >= m_PreparedDataBufferUsage)
  {
    max_dest_bytes=m_PreparedDataBufferUsage;
    m_pCyclicBuffer->AddBytes (&(m_pPreparedDataBuffer[m_PreparedDataBufferStart]),
      max_dest_bytes);
    m_PreparedDataBufferUsage=0;
    m_PreparedDataBufferStart=0;
    return max_dest_bytes;
  }

  // Otherwise copy what will fit and update the position and remaining byte
  //  indicators appropriately.
  m_pCyclicBuffer->AddBytes (&(m_pPreparedDataBuffer[m_PreparedDataBufferStart]),
    max_dest_bytes);
  m_PreparedDataBufferUsage-=max_dest_bytes;
  m_PreparedDataBufferStart+=max_dest_bytes;
  return max_dest_bytes;
}




void SndSysOggSoundStream::GetDataPointers (size_t* position_marker, 
					    size_t max_requested_length,
					    void **buffer1, 
					    size_t *buffer1_length,
					    void **buffer2,
					    size_t *buffer2_length)
{
  m_pCyclicBuffer->GetDataPointersFromPosition (position_marker,
    max_requested_length, (uint8 **)buffer1, buffer1_length, 
    (uint8 **)buffer2,buffer2_length);

  /* If read is finished and we've underbuffered here, then we can mark the 
   * stream as paused so no further advancement takes place */
  if ((!m_bPaused) && (m_bPlaybackReadComplete) 
      && ((*buffer1_length + *buffer2_length) < max_requested_length))
  {
    m_bPaused=true;
    if (m_bAutoUnregisterRequested)
      m_bAutoUnregisterReady=true;
    m_bPlaybackReadComplete=false;
  }

  if (*position_marker > m_MostAdvancedReadPointer)
    m_MostAdvancedReadPointer=*position_marker;
}



void SndSysOggSoundStream::InitializeSourcePositionMarker (
  size_t *position_marker)
{
  *position_marker=m_MostAdvancedReadPointer;
}
