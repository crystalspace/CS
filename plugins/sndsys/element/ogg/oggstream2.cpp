/*
    Copyright (C) 2005 by Andrew Mann
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
#include "isndsys/ss_stream.h"
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

SndSysOggSoundStream::SndSysOggSoundStream (csRef<SndSysOggSoundData> pData, 
					    OggDataStore *pDataStore, csSndSysSoundFormat *pRenderFormat, 
              int Mode3D) :
  SndSysBasicStream(pRenderFormat, Mode3D)
{
  // Initialize the stream data
  m_StreamData.datastore=pDataStore;
  m_StreamData.position=0;

  m_pSoundData=pData;

  // Allocate an advance buffer
  m_pCyclicBuffer = new SoundCyclicBuffer (
    (m_RenderFormat.Bits/8 * m_RenderFormat.Channels) * 
      (m_RenderFormat.Freq * OGG_BUFFER_LENGTH_MULTIPLIER / 
	OGG_BUFFER_LENGTH_DIVISOR));
  CS_ASSERT(m_pCyclicBuffer!=0);

  // Initialize ogg file
  memset(&m_VorbisFile,0,sizeof(OggVorbis_File));
  ov_open_callbacks (&m_StreamData,&m_VorbisFile,0,0,
    SndSysOggSoundData::ogg_callbacks);

  // Set to not a valid stream
  m_CurrentOggStream=-1;
}

SndSysOggSoundStream::~SndSysOggSoundStream ()
{
  //Closes the bitstream and cleans up loose ends.
  ov_clear(&m_VorbisFile);
}

const char *SndSysOggSoundStream::GetDescription()
{
  // Try to retrieve the description from the underlying data component.
  const char *pDesc=m_pSoundData->GetDescription();

  // If the data component has no description, return a default description.
  if (!pDesc)
    return "Ogg Stream";
  return pDesc;
}

size_t SndSysOggSoundStream::GetFrameCount()
{
  const csSndSysSoundFormat *data_format=m_pSoundData->GetFormat();

  uint64 framecount=m_pSoundData->GetFrameCount();
  framecount*=m_RenderFormat.Freq;
  framecount/=data_format->Freq;

  return framecount;
}

void SndSysOggSoundStream::AdvancePosition(size_t frame_delta)
{
  size_t needed_bytes=0;
  //if loop is enabled, end loop frame is different than zero and we are at the loop ending return to the
  //start of the loop
  if(m_bLooping && m_endLoopFrame != 0 && m_MostAdvancedReadPointer+frame_delta >= m_endLoopFrame)
  {
      //first advance the decoding of the exact bound we need to reach the endloopframe
      AdvancePosition(m_endLoopFrame-m_MostAdvancedReadPointer-1);
      //remove from frame_delta what we decoded already
      frame_delta -= m_endLoopFrame-m_MostAdvancedReadPointer-1;
      // Flush the prepared samples
      m_PreparedDataBufferUsage=0;
      m_PreparedDataBufferStart=0;

      // Seek the ogg stream to the start loop position position for the rest of the advancement
      ov_pcm_seek(&m_VorbisFile,m_startLoopFrame);
  }

  if (m_NewPosition != InvalidPosition)
  {
    // Signal a full cyclic buffer flush
    needed_bytes=m_pCyclicBuffer->GetLength();

    // Flush the prepared samples too
    m_PreparedDataBufferUsage=0;
    m_PreparedDataBufferStart=0;

    // Seek the ogg stream to the requested position
    ov_pcm_seek(&m_VorbisFile,m_NewPosition);

    m_NewPosition = InvalidPosition;
    m_bPlaybackReadComplete=false;
  }
  if (m_PauseState != CS_SNDSYS_STREAM_UNPAUSED || m_bPlaybackReadComplete || frame_delta==0)
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

        // Loop by resetting the position to the start loop position and continuing
        ov_pcm_seek(&m_VorbisFile,m_startLoopFrame);
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
        m_pPCMConverter = new PCMSampleConverter (
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

