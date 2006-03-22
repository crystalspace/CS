/*
    Copyright (C) 2004 by Andrew Mann
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
#include "wavdata2.h"
#include "wavstream2.h"

// Keep these as integer values or the stream may feed portions of a sample to the higher layer which may cause problems
// 1 , 1 will decode 1/1 or 1 second of audio in advance
#define WAV_BUFFER_LENGTH_MULTIPLIER  1
#define WAV_BUFFER_LENGTH_DIVISOR     5

/// The size in bytes of the buffer in which is used if PCM conversion is necessary
#define WAV_DECODE_BUFFER_SIZE 4096


static const size_t positionInvalid = (size_t)~0;

SndSysWavSoundStream::SndSysWavSoundStream (csRef<SndSysWavSoundData> data, 
        char *WavData, size_t WavDataLen, csSndSysSoundFormat *renderformat, 
        int mode3d) :
  scfImplementationType(this)
{
  // Copy render format information
  memcpy(&m_RenderFormat,renderformat,sizeof(csSndSysSoundFormat));

  // Copy the wav data buffer start and length
  m_pWavDataBase=WavData;
  m_WavDataLength=WavDataLen;

  // Current position is at the beginning
  m_pWavCurrentPointer=m_pWavDataBase;
  m_WavBytesLeft=m_WavDataLength;


  m_SoundData=data;

  // Allocate an advance buffer
  m_pCyclicBuffer = new CS::Sound::SoundCyclicBuffer (
    (m_RenderFormat.Bits/8 * m_RenderFormat.Channels) * 
    (m_RenderFormat.Freq * WAV_BUFFER_LENGTH_MULTIPLIER / 
      WAV_BUFFER_LENGTH_DIVISOR));
  CS_ASSERT(m_pCyclicBuffer!=0);

  // Start the most advanced read pointer at offset 0
  m_MostAdvancedReadPointer=0;

  // Set stream to m_bPaused, at the beginning, and not m_bLooping
  m_bPaused=true;
  m_bLooping=false;

  // Set the render sample size
  m_RenderFrameSize=(renderformat->Bits/8) * renderformat->Channels;


  // The prepared data buffer will be initialized on first use
  m_pPreparedDataBuffer=0;
  m_PreparedDataBufferSize=0;
  m_PreparedDataBufferStart=0;

  // No extra data left in the decode buffer since last call
  m_PreparedDataBufferUsage=0;

  // Playback rate is initially 100% (normal)
  m_PlaybackPercent=100;

  /* Forces the output buffer to be resized and the converter to be created on 
   * the first pass */
  m_OutputFrequency=0;

  // Output frequency is initially the same as the render frequency
  m_NewOutputFrequency=renderformat->Freq;

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

  // Playback isn't complete
  m_bPlaybackReadComplete=false;
}

SndSysWavSoundStream::~SndSysWavSoundStream ()
{
  delete m_pCyclicBuffer;
  delete m_pPCMConverter;
  delete[] m_pPreparedDataBuffer;
}

const char *SndSysWavSoundStream::GetDescription()
{
  const char *pDesc=m_SoundData->GetDescription();
  if (!pDesc)
    return "Wav Stream";
  return pDesc;
}


const csSndSysSoundFormat *SndSysWavSoundStream::GetRenderedFormat()
{
  return &m_RenderFormat;
}

int SndSysWavSoundStream::Get3dMode()
{
  return m_3DMode;
}


size_t SndSysWavSoundStream::GetFrameCount()
{
  // The Frame count is the modified by the relative frequencies of
  //  the data and the renderer
  const csSndSysSoundFormat *data_format=m_SoundData->GetFormat();
 
  uint64 framecount=m_SoundData->GetFrameCount();
  framecount*=m_RenderFormat.Freq;
  framecount/=data_format->Freq;

  return (size_t)(framecount & 0x7FFFFFFF);
}


size_t SndSysWavSoundStream::GetPosition()
{
  return m_MostAdvancedReadPointer;
}

bool SndSysWavSoundStream::ResetPosition()
{
  m_NewPosition=0;
//  m_pCyclicBuffer->Clear();
//  m_MostAdvancedReadPointer=0;
  m_bPlaybackReadComplete=false;
  return true;
}

bool SndSysWavSoundStream::SetPosition (size_t newposition)
{
  m_NewPosition=newposition;
//  m_pCyclicBuffer->Clear(newposition);
  m_bPlaybackReadComplete=false;
  return true;
}

bool SndSysWavSoundStream::Pause()
{
  m_bPaused=true;
  return true;
}


bool SndSysWavSoundStream::Unpause()
{
  m_bPaused=false;
  return true;
}

int SndSysWavSoundStream::GetPauseState()
{
  if (m_bPaused)
    return CS_SNDSYS_STREAM_PAUSED;
  return CS_SNDSYS_STREAM_UNPAUSED;
}

bool SndSysWavSoundStream::SetLoopState(int loopstate)
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
int SndSysWavSoundStream::GetLoopState()
{
  if (m_bLooping)
    return CS_SNDSYS_STREAM_LOOP;
  return CS_SNDSYS_STREAM_DONTLOOP;
}

void SndSysWavSoundStream::SetPlayRatePercent(int percent)
{
  m_PlaybackPercent=percent;
  m_NewOutputFrequency = (m_RenderFormat.Freq * 100) / m_PlaybackPercent;
}

int SndSysWavSoundStream::GetPlayRatePercent()
{
  return m_PlaybackPercent;
}


/**
 * If AutoUnregister is set, when the stream is m_bPaused it, and all sources 
 * attached to it are removed from the sound engine.
 */
void SndSysWavSoundStream::SetAutoUnregister(bool autounreg)
{
  m_bAutoUnregisterRequested=autounreg;
}

/**
 * If AutoUnregister is set, when the stream is m_bPaused it, and all sources 
 * attached to it are removed from the sound engine. 
 */
bool SndSysWavSoundStream::GetAutoUnregister()
{
  return m_bAutoUnregisterRequested;
}

/** 
 * Used by the sound renderer to determine if this stream needs to be 
 * unregistered.
 */
bool SndSysWavSoundStream::GetAutoUnregisterRequested()
{
  return m_bAutoUnregisterReady;
}


void SndSysWavSoundStream::AdvancePosition(size_t frame_delta)
{
  size_t needed_bytes=0;

  // If a new position has been requested via the SetPosition function,
  //  handle that first.
  if (m_NewPosition != positionInvalid)
  {
    // Signal a full cyclic buffer flush
    needed_bytes=m_pCyclicBuffer->GetLength();

    // Flush the prepared samples too
    m_PreparedDataBufferUsage=0;
    m_PreparedDataBufferStart=0;

    // Move the wav read position to the requested position
    if (m_NewPosition >= m_WavDataLength)
      m_NewPosition=m_WavDataLength-1;
    m_pWavCurrentPointer=m_pWavDataBase+m_NewPosition;
    m_WavBytesLeft=m_WavDataLength-m_NewPosition;

    m_NewPosition = positionInvalid;
    m_bPlaybackReadComplete=false;
  }
  if (m_bPaused || m_bPlaybackReadComplete || frame_delta==0)
    return;


  


  // Figure out how many bytes we need to fill for this advancement
  if (needed_bytes==0)
    needed_bytes=frame_delta  * (m_RenderFormat.Bits/8) * m_RenderFormat.Channels;

  // If we need more space than is available in the whole cyclic buffer, then we already underbuffered, reduce to just 1 cycle full
  if ((size_t)needed_bytes > m_pCyclicBuffer->GetLength())
    needed_bytes=(size_t)(m_pCyclicBuffer->GetLength() & 0x7FFFFFFF);

  // Free space in the cyclic buffer if necessary
  if ((size_t)needed_bytes > m_pCyclicBuffer->GetFreeBytes())
    m_pCyclicBuffer->AdvanceStartValue(needed_bytes - (size_t)(m_pCyclicBuffer->GetFreeBytes() & 0x7FFFFFFF));

  // Fill in leftover decoded data if needed
  if (m_PreparedDataBufferUsage > 0)
    needed_bytes-=CopyBufferBytes(needed_bytes);


  while (needed_bytes>0)
  {
    size_t available_bytes=WAV_DECODE_BUFFER_SIZE;

//    if (available_bytes > WAV_DECODE_BUFFER_SIZE)
//      available_bytes=WAV_DECODE_BUFFER_SIZE;

    if (available_bytes > m_WavBytesLeft)
      available_bytes=m_WavBytesLeft;


    // If the output frequency has changed, a new converter is necessary
    if (m_NewOutputFrequency != m_OutputFrequency)
    {
      int needed_buffer,source_sample_size;
      const csSndSysSoundFormat *data_format=m_SoundData->GetFormat();

      m_OutputFrequency=m_NewOutputFrequency;

      // Create the pcm sample converter if it's not yet created
      if (m_pPCMConverter == 0)
      {
#ifdef CS_LITTLE_ENDIAN
        m_pPCMConverter = new CS::Sound::PCMSampleConverter (
	  data_format->Channels,data_format->Bits,data_format->Freq);
#else
        // If we're running on a big endian system and the data is using 
	// 16 bit samples, endian conversion is necessary
        if (data_format->Bits>8)
          m_pPCMConverter = new CS::Sound::PCMSampleConverter(
	    data_format->Channels,data_format->Bits,data_format->Freq, true);
        else
          m_pPCMConverter = new CS::Sound::PCMSampleConverter (
	    data_format->Channels,data_format->Bits,data_format->Freq);
#endif
      }

      // Calculate the size of one source sample
      source_sample_size=data_format->Channels * data_format->Bits;

      // Calculate the needed buffer size for this conversion
      needed_buffer=(m_pPCMConverter->GetRequiredOutputBufferMultiple (
	m_RenderFormat.Channels,m_RenderFormat.Bits,m_OutputFrequency) * 
	(WAV_DECODE_BUFFER_SIZE + source_sample_size))/1024;

      // Allocate a new buffer if needed - this will only happen if the source rate changes
      if (m_PreparedDataBufferSize < needed_buffer)
      {
        delete[] m_pPreparedDataBuffer;
        m_pPreparedDataBuffer = new char[needed_buffer];
        m_PreparedDataBufferSize=needed_buffer;
      }

      
      if ((data_format->Bits == m_RenderFormat.Bits) &&
          (data_format->Channels == m_RenderFormat.Channels) &&
          (data_format->Freq == m_RenderFormat.Freq))
      {
          m_bConversionNeeded=false;
      }
      else
        m_bConversionNeeded=true;

#ifdef CS_BIG_ENDIAN
      // WAV data is always stored in little endian format.  If we're running 
      // on a big endian system and we're reading 16 bit samples, then we need 
      // a converter
      if (data_format->Bits >8)
        m_bConversionNeeded=true;
#endif

    }

    // If no conversion is necessary 
    if (!m_bConversionNeeded)
    {
      memcpy(m_pPreparedDataBuffer,m_pWavCurrentPointer,available_bytes);
      m_PreparedDataBufferUsage=available_bytes;
    }
    else
      m_PreparedDataBufferUsage = m_pPCMConverter->ConvertBuffer (
	m_pWavCurrentPointer,available_bytes,m_pPreparedDataBuffer,m_RenderFormat.Channels,
      m_RenderFormat.Bits,m_OutputFrequency);

    // Decrease the available bytes and move the buffer pointer ahead
    m_pWavCurrentPointer+=available_bytes;
    m_WavBytesLeft-=available_bytes;

    // Copy the data available into the destination buffer as requested
    if (m_PreparedDataBufferUsage > 0)
      needed_bytes-=CopyBufferBytes(needed_bytes);

    // At the end of the stream, do we loop or stop?
    if (m_WavBytesLeft<=0)
    {
      if (!m_bLooping)
      {
        m_bPlaybackReadComplete=true;
        m_pWavCurrentPointer=m_pWavDataBase;
        m_WavBytesLeft=m_WavDataLength;
        break;
      }

      // Loop by resetting the position to the beginning and continuing
      m_pWavCurrentPointer=m_pWavDataBase;
      m_WavBytesLeft=m_WavDataLength;
    }
  }
      
}


size_t SndSysWavSoundStream::CopyBufferBytes (size_t max_dest_bytes)
{
  if (max_dest_bytes >= m_PreparedDataBufferUsage)
  {
    max_dest_bytes=m_PreparedDataBufferUsage;
    m_pCyclicBuffer->AddBytes(&(m_pPreparedDataBuffer[m_PreparedDataBufferStart]),max_dest_bytes);
    m_PreparedDataBufferUsage=0;
    m_PreparedDataBufferStart=0;
    return max_dest_bytes;
  }


  m_pCyclicBuffer->AddBytes(&(m_pPreparedDataBuffer[m_PreparedDataBufferStart]),max_dest_bytes);
  m_PreparedDataBufferUsage-=max_dest_bytes;
  m_PreparedDataBufferStart+=max_dest_bytes;
  return max_dest_bytes;
}




void SndSysWavSoundStream::GetDataPointers (size_t* position_marker,
					    size_t max_requested_length,
					    void **buffer1,
					    size_t *buffer1_length,
					    void **buffer2,
					    size_t *buffer2_length)
{
  m_pCyclicBuffer->GetDataPointersFromPosition (position_marker,
    max_requested_length, (uint8 **)buffer1, buffer1_length, 
    (uint8 **)buffer2, buffer2_length);

  /* If read is finished and we've underbuffered here, then we can mark the 
   * stream as m_bPaused so no further advancement takes place */
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

void SndSysWavSoundStream::InitializeSourcePositionMarker (
  size_t* position_marker)
{
  *position_marker=m_MostAdvancedReadPointer;
}
