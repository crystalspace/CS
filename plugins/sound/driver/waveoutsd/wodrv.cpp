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

  System = NULL;
  SoundRender = NULL;
  MemorySize = 0;
  Memory = NULL;
  WaveOut = NULL;
  Config = 0;
}

csSoundDriverWaveOut::~csSoundDriverWaveOut()
{
  if (Config) Config->DecRef();
}

bool csSoundDriverWaveOut::Initialize (iSystem *iSys)
{
  System = iSys;
  Config = System->CreateConfig ("/config/sound.cfg");
  return true;
}

bool csSoundDriverWaveOut::Open(iSoundRender *render, int frequency, bool bit16, bool stereo)
{
  System->Printf (MSG_INITIALIZATION, "Wave-Out Sound Driver selected.\n");

  // store pointer to sound renderer
  if (!render) return false;
  SoundRender = render;
  SoundRender->IncRef();

  // store sound format settings
  Frequency = frequency;
  Bit16 = bit16;
  Stereo = stereo;

  // setup wave format
  WAVEFORMATEX Format;
  Format.wFormatTag = WAVE_FORMAT_PCM;
  if (Stereo) Format.nChannels = 2;
  else Format.nChannels = 1;
  Format.nSamplesPerSec = Frequency;
  if (Bit16) Format.wBitsPerSample = 16;
  else Format.wBitsPerSample = 8;
  Format.nBlockAlign = Format.nChannels * Format.wBitsPerSample / 8;
  Format.nAvgBytesPerSec = Format.nBlockAlign * Format.nSamplesPerSec;
  Format.cbSize = 0;

  // read settings from the config file
  unsigned int RefreshRate=Config->GetInt("Sound.WaveOut", "Refresh", 5);
  System->Printf (MSG_INITIALIZATION, "  updating %d times per second\n",
    RefreshRate);

  // setup misc member variables
  MemorySize = Format.nAvgBytesPerSec/RefreshRate;
  MemorySize -=(MemorySize%Format.nBlockAlign);
  Memory = NULL;
  SoundProcLocked = false;
  NumSoundBlocksToWrite = 0;

  // initialize sound output.
  // @@@ must this be called from within another thread for multi-processor
  // support?
  MMRESULT res = waveOutOpen(&WaveOut, WAVE_MAPPER, &Format,
    (LONG)&waveOutProc, 0L, CALLBACK_FUNCTION);
  if (res != MMSYSERR_NOERROR) System->Printf(MSG_INITIALIZATION,
    "  could not open Wave-Out system (%s).\n", GetMMError(res));

  // Store old volume and set full volume because the software sound renderer
  // will apply volume internally. If this device does not allow volume
  // changes, all the volume set/get calls will fail, but output will be
  // correct. So don't check for errors here!
  waveOutGetVolume(WaveOut, &OldVolume);
  waveOutSetVolume(WaveOut, 0xffffffff);

  // write empty sound blocks to start playback. Three sound blocks should
  // be enough to prevent sound gaps; if not, make this an option in the
  // config file.
  SoundProc(NULL);
  SoundProc(NULL);
  SoundProc(NULL);

  return S_OK;
}

void csSoundDriverWaveOut::Close()
{
  if (WaveOut)
  {
    waveOutSetVolume(WaveOut, OldVolume);
    /* @@@ error checking */
    waveOutReset(WaveOut);
    waveOutClose(WaveOut);
    WaveOut=NULL;
  }

  // wait for SoundProc() to exit
  while (SoundProcLocked);

  if (SoundRender) 
  {
    SoundRender->DecRef();
    SoundRender = NULL;
  }

  Memory = NULL;
}

void csSoundDriverWaveOut::LockMemory(void **mem, int *memsize)
{
  *mem = Memory;
  *memsize = MemorySize;
}

void csSoundDriverWaveOut::UnlockMemory() {}
bool csSoundDriverWaveOut::IsBackground() {return true;}
bool csSoundDriverWaveOut::Is16Bits() {return Bit16;}
bool csSoundDriverWaveOut::IsStereo() {return Stereo;}
bool csSoundDriverWaveOut::IsHandleVoidSound() {return false;}
int csSoundDriverWaveOut::GetFrequency() {return Frequency;}

typedef struct {
  csSoundDriverWaveOut *Driver;
  HGLOBAL DataHandle;
  unsigned char *Data;
} SoundBlock;

void CALLBACK csSoundDriverWaveOut::waveOutProc(HWAVEOUT /*WaveOut*/,
  UINT uMsg, DWORD /*dwInstance*/, DWORD dwParam1, DWORD /*dwParam2*/)
{
  if (uMsg==MM_WOM_DONE) {
    // @@@ special behaviour in case this was called by waveOutReset?

    LPWAVEHDR OldHeader = (LPWAVEHDR)dwParam1;
    if (OldHeader->dwUser==NULL) return;
    SoundBlock *Block = (SoundBlock *)OldHeader->dwUser;
    Block->Driver->SoundProc(OldHeader);
  }
}

// @@@ error checking
void csSoundDriverWaveOut::SoundProc(LPWAVEHDR OldHeader) {
  // get rid of the previous header
  if (OldHeader) {
    // get the previous sound block
    SoundBlock *Block = (SoundBlock *)(OldHeader->dwUser);

    // Free this block up
    MMRESULT res;
    res = waveOutUnprepareHeader(WaveOut, OldHeader, sizeof(WAVEHDR));
    if (res != MMSYSERR_NOERROR) System->Printf(MSG_WARNING,
      "cannot unprepare wave-out header (%s).\n", GetMMError(res));
    
    GlobalUnlock(Block->DataHandle);
    GlobalFree(Block->DataHandle);
    delete Block;
  }

  // if there is already an instance of this function, tell it to write our
  // block as well and return
  NumSoundBlocksToWrite++;
  if (SoundProcLocked) return;

  // lock this function
  SoundProcLocked = true;

  while (NumSoundBlocksToWrite > 0) {
    NumSoundBlocksToWrite--;

    // create a new block
    SoundBlock *Block = new SoundBlock();
    Block->Driver = this;
    Block->DataHandle = GlobalAlloc(GMEM_MOVEABLE |
        GMEM_SHARE, sizeof(WAVEHDR)+MemorySize);
    Block->Data = (unsigned char *)GlobalLock(Block->DataHandle);

    // Set up various pointers
    LPWAVEHDR lpWaveHdr = (LPWAVEHDR)Block->Data;
    Memory = Block->Data + sizeof(WAVEHDR);

    // call the sound renderer mixing function
    SoundRender->MixingFunction();
  
    // Set up and prepare header. 
    lpWaveHdr->lpData = (char *)Memory;
    lpWaveHdr->dwBufferLength = MemorySize;
    lpWaveHdr->dwFlags = 0L;
    lpWaveHdr->dwLoops = 0L;
    lpWaveHdr->dwUser = (DWORD)Block;
    waveOutPrepareHeader(WaveOut, lpWaveHdr, sizeof(WAVEHDR));

    // Now the data block can be sent to the output device. The 
    // waveOutWrite function returns immediately and waveform 
    // data is sent to the output device in the background.
    MMRESULT result = waveOutWrite(WaveOut, lpWaveHdr, sizeof(WAVEHDR)); 
    if (result != MMSYSERR_NOERROR) {
      System->Printf(MSG_WARNING, "cannot write sound block to wave-out "
        "(%s).\n", GetMMError(result));

      result = waveOutUnprepareHeader(WaveOut, lpWaveHdr, sizeof(WAVEHDR));
      if (result != MMSYSERR_NOERROR) System->Printf(MSG_WARNING, "cannot "
        "unprepare sound block (%s).\n", GetMMError(result));
      GlobalUnlock(Block->DataHandle);
      GlobalFree(Block->DataHandle);
      delete Block;
    }
  }
  
  // unlock this function
  SoundProcLocked = false;
}

const char *csSoundDriverWaveOut::GetMMError(MMRESULT r) {
  switch (r) {
  case MMSYSERR_NOERROR:
    return "no MM error";

  case MMSYSERR_ALLOCATED:
    return "resource already allocated by an other program";

  case MMSYSERR_BADDEVICEID:
    return "bad device";

  case MMSYSERR_NODRIVER:
    return "there is no device";

  case MMSYSERR_NOMEM:
    return "unable to allocate memory";

  case WAVERR_BADFORMAT:
    return "unsupported audio format";

  case WAVERR_SYNC:
    return "synchronous device without WAVE_ALLOWSYNC flag";

  case MMSYSERR_INVALHANDLE:
    return "invalid device handle";

  case WAVERR_STILLPLAYING:
    return "still playing this sound block";

  default:
    return "unknown MM error";
  }
}

