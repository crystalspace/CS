/*
	Copyright (C) 1998, 1999 by Nathaniel 'NooTe' Saint Martin
	Copyright (C) 1998, 1999 by Jorrit Tyberghein
	Written by Nathaniel 'NooTe' Saint Martin

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
#include <stdio.h>
#include <mmsystem.h>

#include "csutil/scf.h"
#include "isystem.h"
#include "isndrdr.h"
#include "icfgfile.h"
#include "wodrv.h"

#define MYMMSYSERR_NOCREATETHREAD 12

typedef struct
{
	csSoundDriverWaveOut *driver;
	HGLOBAL hHeader;
  LPBYTE lpSoundData;
} SoundBlock;

#define NUM_ThreadPriority 3
struct
{
  int  priority;
  char *name;
} ThreadPriority[NUM_ThreadPriority]=
{
  { THREAD_PRIORITY_LOWEST, "lowest"},
  { THREAD_PRIORITY_NORMAL, "normal"},
  { THREAD_PRIORITY_HIGHEST, "highest"}
};

static bool are_you_playing = false;

IMPLEMENT_FACTORY (csSoundDriverWaveOut)

EXPORT_CLASS_TABLE (sndwaveout)
  EXPORT_CLASS (csSoundDriverWaveOut, "crystalspace.sound.driver.waveout",
    "Software Sound Driver for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE(csSoundDriverWaveOut)
	IMPLEMENTS_INTERFACE(iPlugIn)
	IMPLEMENTS_INTERFACE(iSoundDriver)
IMPLEMENT_IBASE_END

csSoundDriverWaveOut::csSoundDriverWaveOut(iBase *piBase)
{
  CONSTRUCT_IBASE(piBase);

  m_piSystem = NULL;
  m_piSoundRender = NULL;
  MemorySize = 0;
  Memory = NULL;
  volume = 1.0;
  hwo = NULL;
  hThread = NULL;
  dwThread = 0;
  Config = 0;
}

csSoundDriverWaveOut::~csSoundDriverWaveOut()
{
  if (Config) Config->DecRef();
}

bool csSoundDriverWaveOut::Initialize (iSystem *iSys)
{
  m_piSystem = iSys;
  return true;
}

bool csSoundDriverWaveOut::Open(iSoundRender *render, int frequency, bool bit16, bool stereo)
{
  m_piSystem->Printf (MSG_INITIALIZATION, "Wave-Out Sound Driver selected\n");

  if (!render) return false;
  m_piSoundRender = render;
  m_piSoundRender->IncRef();

  Config = m_piSystem->CreateConfig ("/config/sound.cfg");

  MMRESULT res;
  WAVEFORMATEX format;
  memset(&format, 0, sizeof(WAVEFORMATEX));
  format.wFormatTag = WAVE_FORMAT_PCM;
  if(stereo)
    format.nChannels = 2;
  else
    format.nChannels = 1;
  format.nSamplesPerSec = frequency;
  if(bit16)
    format.wBitsPerSample = 16;
  else
    format.wBitsPerSample = 8;
  format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
  format.nAvgBytesPerSec = format.nBlockAlign * format.nSamplesPerSec;
  format.cbSize = 0;

  m_nFrequency = frequency;
  m_b16Bits = bit16;
  m_bStereo = stereo;

  const char *callback_func=Config->GetStr
    ("SoundDriver.WaveOut", "CALLBACK", "function");
  const char *thread_func=Config->GetStr
    ("SoundDriver.WaveOut", "THREAD_PRIORITY", "normal");
  bool threading = false;

  if(stricmp(callback_func, "thread")==0)
    threading = true;

  unsigned int refresh=Config->GetInt("SoundDriver.WaveOut", "REFRESH", 5);

//  if(refresh>format.nAvgBytesPerSec) refresh=format.nAvgBytesPerSec;

  MemorySize = format.nAvgBytesPerSec/refresh;
  MemorySize -=(MemorySize%format.nBlockAlign);
  Memory = NULL;

  // Open the playback device
  if(threading)
  {
    m_piSystem->Printf (MSG_INITIALIZATION, "  uses thread method\n");
    hThread = CreateThread(NULL, 0, &waveOutThreadProc, this, 0, &dwThread);
    if(hThread)
    {
      for(int p=0; p<NUM_ThreadPriority; p++)
        if(stricmp(ThreadPriority[p].name, thread_func) == 0)
        {
          if(SetThreadPriority(hThread, ThreadPriority[p].priority)!=0)
            m_piSystem->Printf (MSG_INITIALIZATION, "  thread set to %s priority\n", ThreadPriority[p].name);
          break;
        }
      res = waveOutOpen(&hwo, WAVE_MAPPER, &format, (LONG)dwThread, 0L, CALLBACK_THREAD);
    }
    else
      res = MYMMSYSERR_NOCREATETHREAD;
  }
  else
  {
    m_piSystem->Printf (MSG_INITIALIZATION, "  uses function callback method\n");
    res=waveOutOpen(&hwo, WAVE_MAPPER, &format, (LONG)&waveOutProc, 0L, CALLBACK_FUNCTION);
  }
  if(res!=MMSYSERR_NOERROR)
  {
    switch(res)
    {
      case MYMMSYSERR_NOCREATETHREAD:
        m_piSystem->Printf (MSG_FATAL_ERROR, "WaveOut error : Cannot create thread !");
        break;

      case MMSYSERR_ALLOCATED:
        m_piSystem->Printf (MSG_FATAL_ERROR, "WaveOut error : Ressource already allocated by an other program !");
        break;

      case MMSYSERR_BADDEVICEID:
        m_piSystem->Printf (MSG_FATAL_ERROR, "WaveOut error : Bad device !");
        break;

      case MMSYSERR_NODRIVER:
        m_piSystem->Printf (MSG_FATAL_ERROR, "WaveOut error : there is no device !");
        break;

      case MMSYSERR_NOMEM:
        m_piSystem->Printf (MSG_FATAL_ERROR, "WaveOut error : Unable to allocate memory !");
        break;

      case WAVERR_BADFORMAT:
        m_piSystem->Printf (MSG_FATAL_ERROR, "WaveOut error : Unsupported audio format !");
        break;

      default:
        m_piSystem->Printf (MSG_FATAL_ERROR, "WaveOut error : Inknown Error !");
        break;
    }
    return E_FAIL;
  }

  waveOutGetVolume(hwo, &old_Volume);
  int old_v2=old_Volume&0xffff;
  SetVolume((float)old_v2/(float)65535.0);
	
  SoundProc(NULL);
  m_piSystem->Printf (MSG_INITIALIZATION, "  playing %d Hz, %d bits, %s\n",
    m_nFrequency, (m_b16Bits)?16:8, (m_bStereo)?"Stereo":"Mono");
  return S_OK;
}

void csSoundDriverWaveOut::Close()
{
  if (hwo)
  {
    waveOutSetVolume(hwo, old_Volume);
    waveOutReset(hwo);
    waveOutClose(hwo);
    hwo=NULL;
  }

  // wait for SoundProc() to exit
  while (are_you_playing);

  if( m_piSoundRender ) 
  {
    m_piSoundRender->DecRef();
    m_piSoundRender = NULL;
  }

  Memory = NULL;
}

void csSoundDriverWaveOut::SetVolume(float vol)
{
  if(vol<0)
    vol = 0;
  else if (vol>1)
    vol = 1;

  if(hwo)
  {
    unsigned long av=(unsigned long)(vol*65535.0);
    av=(av<<16)|av;
    waveOutSetVolume(hwo, av);
  }

  volume = vol;
}

float csSoundDriverWaveOut::GetVolume()
{
  return volume;
}

void csSoundDriverWaveOut::LockMemory(void **mem, int *memsize)
{
  *mem = Memory;
  *memsize = MemorySize;
}

void csSoundDriverWaveOut::UnlockMemory()
{
}

bool csSoundDriverWaveOut::IsBackground()
{
  return true;
}

bool csSoundDriverWaveOut::Is16Bits()
{
  return m_b16Bits;
}

bool csSoundDriverWaveOut::IsStereo()
{
  return m_bStereo;
}

bool csSoundDriverWaveOut::IsHandleVoidSound()
{
  return false;
}

int csSoundDriverWaveOut::GetFrequency()
{
  return m_nFrequency;
}

DWORD WINAPI csSoundDriverWaveOut::waveOutThreadProc( LPVOID dwParam)
{
  csSoundDriverWaveOut *snd = (csSoundDriverWaveOut *)dwParam;
  MSG msg;

  while(1) {
    WaitMessage();
    if(GetMessage( &msg, NULL, 0, 0 ) ) {
      if(msg.message==MM_WOM_DONE) {
        if(snd==NULL || snd->hwo==NULL) continue;
        LPWAVEHDR OldHeader=(LPWAVEHDR)(msg.lParam);
        snd->SoundProc(OldHeader);
      }
      TranslateMessage(&msg); 
      DispatchMessage(&msg);
    } else return msg.wParam;
  }
  return 0xFFFFFFFF;
}

void CALLBACK csSoundDriverWaveOut::waveOutProc(HWAVEOUT /*hwo*/, UINT uMsg, DWORD /*dwInstance*/, 
                                                DWORD dwParam1, DWORD /*dwParam2*/)
{
  if (uMsg==MM_WOM_DONE) {
    LPWAVEHDR OldHeader = (LPWAVEHDR)dwParam1;
    // void data ?
    if(OldHeader->dwUser==NULL || OldHeader->dwUser==0xcdcdcdcd ) return;
    SoundBlock *block = (SoundBlock *)OldHeader->dwUser;
    csSoundDriverWaveOut *Driver = block->driver;
    Driver->SoundProc(OldHeader);
  }
}

void csSoundDriverWaveOut::SoundProc(LPWAVEHDR OldHeader) {
  // wait for the previous call to exit
  while (are_you_playing);
  // lock this function
  are_you_playing = true;

  SoundBlock *Block;

  if (OldHeader) {
    // get the previous sound block
    Block = (SoundBlock *)(OldHeader->dwUser);

    // Free this block up
    waveOutUnprepareHeader(hwo, OldHeader, sizeof(WAVEHDR));
    GlobalUnlock(Block->hHeader);
    GlobalFree(Block->hHeader);
    delete Block;
  }

  // create a new block
  Block = new SoundBlock();
  Block->driver = this;
  Block->hHeader = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR)+MemorySize);
  Block->lpSoundData = (LPBYTE)GlobalLock(Block->hHeader);

  // Set up various pointers
  LPWAVEHDR lpWaveHdr = (LPWAVEHDR)Block->lpSoundData;
  Memory = (unsigned char *)&Block->lpSoundData[sizeof(WAVEHDR)];
  if(Memory == NULL) return;

  // call the sound renderer mixing function
  m_piSoundRender->MixingFunction();
  
  // Set up and prepare header. 
  lpWaveHdr->lpData = (char *)Memory;
  lpWaveHdr->dwBufferLength = MemorySize;
  lpWaveHdr->dwFlags = 0L;
  lpWaveHdr->dwLoops = 0L;
  lpWaveHdr->dwUser = (DWORD)Block;
  waveOutPrepareHeader(hwo, lpWaveHdr, sizeof(WAVEHDR));

  // Now the data block can be sent to the output device. The 
  // waveOutWrite function returns immediately and waveform 
  // data is sent to the output device in the background.
  MMRESULT result = waveOutWrite(hwo, lpWaveHdr, sizeof(WAVEHDR)); 
  if (result != 0) { 
    waveOutUnprepareHeader(hwo, lpWaveHdr, sizeof(WAVEHDR)); 
    GlobalUnlock(Block->hHeader);
    GlobalFree(Block->hHeader);
    delete Block;
  }
  
  // unlock this function
  are_you_playing = false;
}
