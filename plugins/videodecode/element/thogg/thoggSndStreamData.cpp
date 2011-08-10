
#include "cssysdef.h"

#include "iutil/comp.h"
#include "isndsys/ss_loader.h"

#include "csutil/csendian.h"

#include "thoggSndStreamData.h"


SndSysTheoraSoundData::SndSysTheoraSoundData (iBase *pParent, iDataBuffer* pData) :
SndSysBasicData(pParent)
{
  m_SoundFormat.Bits = 16;
  m_SoundFormat.Channels = 2;
  m_SoundFormat.Freq = 44100;
}

SndSysTheoraSoundData::~SndSysTheoraSoundData ()
{
}

size_t SndSysTheoraSoundData::GetDataSize()
{
  return 0;
}

iSndSysStream *SndSysTheoraSoundData::CreateStream (
  csSndSysSoundFormat *renderformat, int mode3d)
{
  if (!m_bInfoReady)
    Initialize();

  SndSysTheoraStream *stream = new SndSysTheoraStream(renderformat,mode3d);
  /*SndSysWavSoundStream *stream=new SndSysWavSoundStream(this, 
    (char *)m_pPCMData, m_PCMDataLength, renderformat, mode3d);*/

  return stream;//(stream);
}
void SndSysTheoraSoundData::Initialize()
{
  m_bInfoReady = true;
}