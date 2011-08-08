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
  /*cout<<m_PreparedDataBufferSize<<endl;
  char * m_pWavCurrentPointer = new char[256];
  int available_bytes = 256;
  for(int i=0;i<256;i++)
  {
    m_pWavCurrentPointer[i]=100;
  }
  memcpy(m_pPreparedDataBuffer,m_pWavCurrentPointer,available_bytes);*/

  if (m_bPaused || m_bPlaybackReadComplete || frame_delta==0)
    return;

  size_t needed_bytes=0;

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
    break;
  }
  //cout<<needed_bytes<<endl ;
}
