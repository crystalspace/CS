#include <cssysdef.h>
#include "thoggAudioMedia.h"
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
  return 0;
}

void SndSysTheoraStream::AdvancePosition(size_t frame_delta) 
{

}
