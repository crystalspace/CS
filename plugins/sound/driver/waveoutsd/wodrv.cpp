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

#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>

#include "sysdef.h"
#include "csutil/scf.h"
#include "cssnddrv/waveoutsd/wodrv.h"
#include "isystem.h"
#include "csutil/inifile.h"
#include "isndlstn.h"
#include "isndsrc.h"

csIniFile* configwodrv;

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

IMPLEMENT_UNKNOWN_NODELETE (csSoundDriverWaveOut)

BEGIN_INTERFACE_TABLE(csSoundDriverWaveOut)
  IMPLEMENTS_INTERFACE(ISoundDriver)
END_INTERFACE_TABLE()

csSoundDriverWaveOut::csSoundDriverWaveOut(iSystem* piSystem)
{
  m_piSystem = piSystem;
  m_piSoundRender = NULL;
  MemorySize = 0;
  Memory = NULL;
  volume = 1.0;
  hwo = NULL;
  hThread = NULL;
  dwThread = 0;
}

csSoundDriverWaveOut::~csSoundDriverWaveOut()
{
}

STDMETHODIMP csSoundDriverWaveOut::Open(ISoundRender *render, int frequency, bool bit16, bool stereo)
{
  SysPrintf (MSG_INITIALIZATION, "\nSoundDriver waveOut selected\n");

  m_piSoundRender = render;
  configwodrv = new csIniFile ("wodrv.cfg");

  MMRESULT res;
  WAVEFORMATEX format;
  memset(&format, 0, sizeof(PCMWAVEFORMAT));
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

  char *callback_func=configwodrv->GetStr("SoundDriver.waveOut", "CALLBACK", "function");
  char *thread_func=configwodrv->GetStr("SoundDriver.waveOut", "THREAD_PRIORITY", "normal");
  bool threading = false;

  if(stricmp(callback_func, "thread")==0)
    threading = true;

  unsigned int refresh=configwodrv->GetInt("SoundDriver.waveOut", "REFRESH", 5);

  if(refresh>format.nAvgBytesPerSec) refresh=format.nAvgBytesPerSec;

  MemorySize = format.nAvgBytesPerSec/refresh;
  MemorySize -=(MemorySize%format.nBlockAlign);
  Memory = NULL;

	// Open the playback device
	if(threading)
  {
    SysPrintf (MSG_INITIALIZATION, "SoundDriver use thread method\n");
    hThread = CreateThread(NULL, 0, &waveOutThreadProc, this, 0, &dwThread);
    if(hThread)
    {
      for(int p=0; p<NUM_ThreadPriority; p++)
        if(stricmp(ThreadPriority[p].name, thread_func) == 0)
        {
          if(SetThreadPriority(hThread, ThreadPriority[p].priority)!=0)
            SysPrintf (MSG_INITIALIZATION, "SoundDriver thread set to %s priority\n", ThreadPriority[p].name);
          break;
        }
      res = waveOutOpen(&hwo, WAVE_MAPPER, &format, (LONG)dwThread, 0L, CALLBACK_THREAD);
    }
    else
      res = MYMMSYSERR_NOCREATETHREAD;
  }
  else
  {
    SysPrintf (MSG_INITIALIZATION, "SoundDriver use function callback method\n");
    res=waveOutOpen(&hwo, WAVE_MAPPER, &format, (LONG)&waveOutProc, 0L, CALLBACK_FUNCTION);
  }
  if(res!=MMSYSERR_NOERROR)
  {
    switch(res)
    {
    case MYMMSYSERR_NOCREATETHREAD:
      SysPrintf (MSG_FATAL_ERROR, "WaveOut error : Cannot create thread !");
      break;

    case MMSYSERR_ALLOCATED:
      SysPrintf (MSG_FATAL_ERROR, "WaveOut error : Ressource already allocated by an other program !");
      break;

    case MMSYSERR_BADDEVICEID:
      SysPrintf (MSG_FATAL_ERROR, "WaveOut error : Bad device !");
      break;

    case MMSYSERR_NODRIVER:
      SysPrintf (MSG_FATAL_ERROR, "WaveOut error : there is no device !");
      break;

    case MMSYSERR_NOMEM:
      SysPrintf (MSG_FATAL_ERROR, "WaveOut error : Unable to allocate memory !");
      break;

    case WAVERR_BADFORMAT:
      SysPrintf (MSG_FATAL_ERROR, "WaveOut error : Unsupported audio format !");
      break;

    default:
      SysPrintf (MSG_FATAL_ERROR, "WaveOut error : Inknown Error !");
      break;
    }
    return E_FAIL;
  }

  waveOutGetVolume(hwo, &old_Volume);
  int old_v2=old_Volume&0xffff;
  SetVolume((float)old_v2/(float)65535.0);
	
  if (MixChunk())
	{
		if (MixChunk())
		{
      SysPrintf (MSG_INITIALIZATION, "SoundDriver initialized to %d Hz %d bits %s\n",
        m_nFrequency, (m_b16Bits)?16:8, (m_bStereo)?"Stereo":"Mono");
      return S_OK;
		}
	}

  SysPrintf (MSG_FATAL_ERROR, "WaveOut error : Error then prepare chunk");
  return E_FAIL;
}

STDMETHODIMP csSoundDriverWaveOut::Close()
{
  if (hwo)
	{
    waveOutSetVolume(hwo, old_Volume);
    waveOutReset(hwo);
		waveOutClose(hwo);
		hwo=NULL;
	}

  Memory = NULL;

  return S_OK;
}

STDMETHODIMP csSoundDriverWaveOut::SetVolume(float vol)
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

  return S_OK;
}

STDMETHODIMP csSoundDriverWaveOut::GetVolume(float *vol)
{
  *vol = volume;

  return S_OK;
}

STDMETHODIMP csSoundDriverWaveOut::LockMemory(void **mem, int *memsize)
{
  *mem = Memory;
  *memsize = MemorySize;

  return S_OK;
}

STDMETHODIMP csSoundDriverWaveOut::UnlockMemory()
{
  return S_OK;
}

STDMETHODIMP csSoundDriverWaveOut::IsBackground(bool *back)
{
  *back = true;

  return S_OK;
}

STDMETHODIMP csSoundDriverWaveOut::Is16Bits(bool *bit)
{
  *bit = m_b16Bits;
  return S_OK;
}

STDMETHODIMP csSoundDriverWaveOut::IsStereo(bool *stereo)
{
  *stereo = m_bStereo;
  return S_OK;
}

STDMETHODIMP csSoundDriverWaveOut::IsHandleVoidSound(bool *handle)
{
  *handle = false;
  return S_OK;
}

STDMETHODIMP csSoundDriverWaveOut::GetFrequency(int *freq)
{
  *freq = m_nFrequency;
  return S_OK;
}

void csSoundDriverWaveOut::SysPrintf(int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;
  
  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);
  
  m_piSystem->Print(mode, buf);
}

static bool are_you_playing = false;

DWORD WINAPI csSoundDriverWaveOut::waveOutThreadProc( LPVOID dwParam)
{
  MSG msg;
  csSoundDriverWaveOut *snd = (csSoundDriverWaveOut *)dwParam;

  while(1)
  {
    WaitMessage();
    if(GetMessage( &msg, NULL, 0, 0 ) )
    {
      if(msg.message==MM_WOM_DONE && !are_you_playing)
      {
        if(snd==NULL || snd->hwo==NULL) continue;
        
        are_you_playing = true;
        
        CHK(SoundBlock * block = new SoundBlock);
        
        // Allocate memory for both the header and the actual sound data itself
        block->driver = snd;
        block->hHeader = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR)+snd->MemorySize);
        block->lpSoundData = (LPBYTE)GlobalLock(block->hHeader);
        
        // Set up various pointers
        LPWAVEHDR lpWaveHdr = (LPWAVEHDR)block->lpSoundData;
        
        snd->Memory = (unsigned char *)&block->lpSoundData[sizeof(WAVEHDR)];
        
        if(snd->Memory == NULL) continue;
        
        snd->m_piSoundRender->MixingFunction();
        
        // Set up and prepare header. 
        lpWaveHdr->lpData = (char *)snd->Memory;
        lpWaveHdr->dwBufferLength = snd->MemorySize;
        lpWaveHdr->dwFlags = 0L;
        lpWaveHdr->dwLoops = 0L;
        lpWaveHdr->dwUser = (DWORD)block;
        waveOutPrepareHeader(snd->hwo, lpWaveHdr, sizeof(WAVEHDR));
        
        // Now the data block can be sent to the output device. The 
        // waveOutWrite function returns immediately and waveform 
        // data is sent to the output device in the background.
        MMRESULT result = waveOutWrite(snd->hwo, lpWaveHdr, sizeof(WAVEHDR));
        
        // Free this block up
        waveOutUnprepareHeader(snd->hwo, lpWaveHdr, sizeof(WAVEHDR));
        GlobalUnlock(block->hHeader);
        GlobalFree(block->hHeader);
        delete block;
        
        are_you_playing = false;
      }
      
      TranslateMessage(&msg); 
      DispatchMessage(&msg);
    }
    else return msg.wParam;
  }
  return 0xFFFFFFFF;
}

void CALLBACK csSoundDriverWaveOut::waveOutProc(HWAVEOUT /*hwo*/, UINT uMsg, DWORD /*dwInstance*/, 
                                                DWORD dwParam1, DWORD /*dwParam2*/)
{
  if (uMsg==MM_WOM_DONE)
  {
    if(are_you_playing) return;

    are_you_playing = true;

    LPWAVEHDR lpWaveHdr = (LPWAVEHDR)dwParam1;
    // void data ?
    if(lpWaveHdr->dwUser==NULL
      || lpWaveHdr->dwUser==0xcdcdcdcd) return;

    SoundBlock * block = (SoundBlock *)lpWaveHdr->dwUser;
    csSoundDriverWaveOut *me = block->driver;

    if(me==NULL) return;

    // Free this block up
    waveOutUnprepareHeader(me->hwo, lpWaveHdr, sizeof(WAVEHDR));
    GlobalUnlock(block->hHeader);
    GlobalFree(block->hHeader);
    delete block;

    // If we're still playing then mix another chunk
		me->MixChunk();
    
    are_you_playing = false;
  } 
}

bool csSoundDriverWaveOut::MixChunk()
{
  if(hwo==NULL) return false;

	CHK(SoundBlock * block = new SoundBlock);

	// Allocate memory for both the header and the actual sound data itself
	block->driver = this;
	block->hHeader = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR)+MemorySize);
	block->lpSoundData = (LPBYTE)GlobalLock(block->hHeader);

	// Set up various pointers
	LPWAVEHDR lpWaveHdr = (LPWAVEHDR)block->lpSoundData;

	Memory = (unsigned char *)&block->lpSoundData[sizeof(WAVEHDR)];

  if(Memory == NULL) return false;

  m_piSoundRender->MixingFunction();

	// Set up and prepare header. 
	lpWaveHdr->lpData = (char *)Memory;
	lpWaveHdr->dwBufferLength = MemorySize;
	lpWaveHdr->dwFlags = 0L;
	lpWaveHdr->dwLoops = 0L;
	lpWaveHdr->dwUser = (DWORD)block;
	waveOutPrepareHeader(hwo, lpWaveHdr, sizeof(WAVEHDR));
 
	// Now the data block can be sent to the output device. The 
	// waveOutWrite function returns immediately and waveform 
	// data is sent to the output device in the background.
	MMRESULT result = waveOutWrite(hwo, lpWaveHdr, sizeof(WAVEHDR)); 
	if (result != 0) 
	{ 
		waveOutUnprepareHeader(hwo, lpWaveHdr, sizeof(WAVEHDR)); 
		GlobalUnlock(block->hHeader);
		GlobalFree(block->hHeader);
    return false;
	}
  return true;
}
