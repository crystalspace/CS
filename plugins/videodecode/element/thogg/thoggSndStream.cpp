#include <cssysdef.h>
#include "thoggSndStream.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>

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

SndSysTheoraStream::SndSysTheoraStream (csSndSysSoundFormat *pRenderFormat, 
                                            int Mode3D) :
  SndSysBasicStream(pRenderFormat, Mode3D)
{
  memcpy(&this->pRenderFormat,pRenderFormat,sizeof(csSndSysSoundFormat));
 // this->pRenderFormat = pRenderFormat;
  // Allocate an advance buffer
  m_pCyclicBuffer = new SoundCyclicBuffer (
    (m_RenderFormat.Bits/8 * m_RenderFormat.Channels) * 
    (m_RenderFormat.Freq * OGG_BUFFER_LENGTH_MULTIPLIER / 
    OGG_BUFFER_LENGTH_DIVISOR));
  CS_ASSERT(m_pCyclicBuffer!=0);
}

SndSysTheoraStream::~SndSysTheoraStream ()
{
}

const char *SndSysTheoraStream::GetDescription ()
{
  return "TheoraOggStream";
}

size_t SndSysTheoraStream::GetFrameCount() 
{
  return CS_SNDSYS_STREAM_UNKNOWN_LENGTH;
}

void SndSysTheoraStream::AdvancePosition(size_t frame_delta) 
{
  size_t needed_bytes=0;

  if (m_NewPosition != InvalidPosition)
  {
    // Signal a full cyclic buffer flush
    needed_bytes=m_pCyclicBuffer->GetLength();

    // Flush the prepared samples too
    m_PreparedDataBufferUsage=0;
    m_PreparedDataBufferStart=0;

    m_NewPosition = InvalidPosition;
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

  while (needed_bytes > 0)
  {
    if ((m_NewOutputFrequency != m_OutputFrequency))
    {
      int needed_buffer,source_sample_size;

      m_OutputFrequency=m_NewOutputFrequency;


      // Create the pcm sample converter if it's not yet created
      if (m_pPCMConverter == 0)
        m_pPCMConverter = new PCMSampleConverter (
        pRenderFormat.Channels, m_RenderFormat.Bits,
        pRenderFormat.Freq);

      // Calculate the size of one source sample
      source_sample_size=pRenderFormat.Channels * m_RenderFormat.Bits;

      // Calculate the needed buffer size for this conversion
      needed_buffer = (m_pPCMConverter->GetRequiredOutputBufferMultiple (
        m_RenderFormat.Channels,m_RenderFormat.Bits,m_OutputFrequency) * 
        (OGG_DECODE_BUFFER_SIZE + source_sample_size))/1024;
      cout<<"buffer size: "<<needed_buffer<<endl;

      if (m_PreparedDataBufferSize < needed_buffer)
      {
        delete[] m_pPreparedDataBuffer;
        m_pPreparedDataBuffer = new char[needed_buffer];
        m_PreparedDataBufferSize=needed_buffer;
      }
    }

    if ((m_RenderFormat.Freq == m_OutputFrequency) &&
      (m_RenderFormat.Channels == m_RenderFormat.Channels))
    {
      /*CS_ASSERT(bytes_read <= m_PreparedDataBufferSize);
      memcpy(m_pPreparedDataBuffer,ogg_decode_buffer,bytes_read);
      m_PreparedDataBufferUsage=bytes_read;*/
    }
    else
    {
     /* m_PreparedDataBufferUsage = m_pPCMConverter->ConvertBuffer (ogg_decode_buffer,
        bytes_read, m_pPreparedDataBuffer, m_RenderFormat.Channels,
        m_RenderFormat.Bits,m_OutputFrequency);*/
    }

    if (m_PreparedDataBufferUsage > 0)
      needed_bytes -= CopyBufferBytes (needed_bytes);
    break;
  }
}
