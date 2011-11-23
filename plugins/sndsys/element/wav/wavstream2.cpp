/*
    Copyright (C) 2004 by Andrew Mann
    Based in part on work by Norman Kraemer

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
#include "iutil/comp.h"
#include "wavdata2.h"
#include "wavstream2.h"

CS_PLUGIN_NAMESPACE_BEGIN(SndSysWav)
{

// Keep these as integer values or the stream may feed portions of a sample to the higher layer which may cause problems
// 1 , 1 will decode 1/1 or 1 second of audio in advance
#define WAV_BUFFER_LENGTH_MULTIPLIER  1
#define WAV_BUFFER_LENGTH_DIVISOR     2

/// The size in bytes of the buffer in which is used if PCM conversion is necessary
#define WAV_DECODE_BUFFER_SIZE 4096


static const size_t positionInvalid = (size_t)~0;

SndSysWavSoundStream::SndSysWavSoundStream (csRef<SndSysWavSoundData> pData, 
        char *pWavData, size_t WavDataLen, csSndSysSoundFormat *pRenderFormat, 
        int Mode3D) :
  SndSysBasicStream(pRenderFormat, Mode3D)
{
  // Copy the wav data buffer start and length
  m_pWavDataBase=pWavData;
  m_WavDataLength=WavDataLen;

  // Current position is at the beginning
  m_pWavCurrentPointer=m_pWavDataBase;
  m_WavBytesLeft=m_WavDataLength;

  m_pSoundData=pData;

  // Allocate an advance buffer
  m_pCyclicBuffer = new SoundCyclicBuffer (
    (m_RenderFormat.Bits/8 * m_RenderFormat.Channels) * 
    (m_RenderFormat.Freq * WAV_BUFFER_LENGTH_MULTIPLIER / 
      WAV_BUFFER_LENGTH_DIVISOR));
  CS_ASSERT(m_pCyclicBuffer!=0);
}

SndSysWavSoundStream::~SndSysWavSoundStream ()
{
}

const char *SndSysWavSoundStream::GetDescription()
{
  const char *pDesc=m_pSoundData->GetDescription();
  if (!pDesc)
    return "Wav Stream";
  return pDesc;
}

size_t SndSysWavSoundStream::GetFrameCount()
{
  // The Frame count is the modified by the relative frequencies of
  //  the data and the renderer
  const csSndSysSoundFormat *data_format=m_pSoundData->GetFormat();
 
  uint64 framecount=m_pSoundData->GetFrameCount();
  framecount*=m_RenderFormat.Freq;
  framecount/=data_format->Freq;

  return (size_t)(framecount & 0x7FFFFFFF);
}

void SndSysWavSoundStream::AdvancePosition(size_t frame_delta)
{
  size_t needed_bytes=0;

  //if loop is enabled, end loop frame is different than zero and we are at the loop ending return to the
  //start of the loop
  if(m_bLooping && m_endLoopFrame != 0 && m_MostAdvancedReadPointer+frame_delta >= m_endLoopFrame)
  {
      //first advance the decoding of the exact length we need to reach the endloopframe
      AdvancePosition(m_endLoopFrame-m_MostAdvancedReadPointer-1);
      //remove from frame_delta what we decoded already
      frame_delta -= m_endLoopFrame-m_MostAdvancedReadPointer-1;
      // Flush the prepared samples
      m_PreparedDataBufferUsage=0;
      m_PreparedDataBufferStart=0;

      //Move the wav read position to the requested position 
      //to the start loop position position for the rest of the advancement
      m_pWavCurrentPointer=m_pWavDataBase+m_startLoopFrame;
      m_WavBytesLeft=m_WavDataLength-m_startLoopFrame;
  }

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
  if (m_PauseState != CS_SNDSYS_STREAM_UNPAUSED || m_bPlaybackReadComplete || frame_delta==0)
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
      const csSndSysSoundFormat *data_format=m_pSoundData->GetFormat();

      m_OutputFrequency=m_NewOutputFrequency;

      // Create the pcm sample converter if it's not yet created
      if (m_pPCMConverter == 0)
      {
#ifdef CS_LITTLE_ENDIAN
        m_pPCMConverter = new PCMSampleConverter (
          data_format->Channels,data_format->Bits,data_format->Freq);
#else
        // If we're running on a big endian system and the data is using 
        // 16 bit samples, endian conversion is necessary
        if (data_format->Bits>8)
          m_pPCMConverter = new PCMSampleConverter(
          data_format->Channels,data_format->Bits,data_format->Freq, true);
        else
          m_pPCMConverter = new PCMSampleConverter (
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
      m_pWavCurrentPointer=m_pWavDataBase+m_startLoopFrame;
      m_WavBytesLeft=m_WavDataLength-m_startLoopFrame;
    }
  }
      
}

}
CS_PLUGIN_NAMESPACE_END(SndSysWav)
