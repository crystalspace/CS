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
#include "csutil/sysfunc.h"
#include <stdio.h>

#include "csutil/scf.h"
#include "csutil/csuctransform.h"
#include "isound/renderer.h"
#include "iutil/cfgfile.h"
#include "iutil/objreg.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "ivaria/reporter.h"
#include "wodrv.h"

#include "csutil/win32/wintools.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csSoundDriverWaveOut)


SCF_IMPLEMENT_IBASE(csSoundDriverWaveOut)
  SCF_IMPLEMENTS_INTERFACE(iSoundDriver)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundDriverWaveOut::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSoundDriverWaveOut::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

#define WAVEOUT_BUFFER_COUNT 5


csSoundDriverWaveOut::csSoundDriverWaveOut(iBase *piBase)
{
  SCF_CONSTRUCT_IBASE(piBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  scfiEventHandler = 0;

  object_reg = 0;
  SoundRender = 0;
  MemorySize = 0;
  Memory = 0;
  hevent_EmptyBlocksReady = 0;
}

csSoundDriverWaveOut::~csSoundDriverWaveOut()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
    if (q != 0)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csSoundDriverWaveOut::Initialize (iObjectRegistry *r)
{
  object_reg = r;
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Nothing | CSMASK_Broadcast);
  Config.AddConfig(object_reg, "/config/sound.cfg");


  /* Create a critical section to lock access to the block exchange list.
   *  Note.  According to the MSDN documentation, a critical section is the ONLY mutexing
   *  method that may be used here.  This means that in its current state on windows, csMutex
   *  is not an option!
  */
  InitializeCriticalSection(&critsec_EmptyBlocks);

  SECURITY_ATTRIBUTES event_sec;
  event_sec.nLength=sizeof(SECURITY_ATTRIBUTES);
  event_sec.lpSecurityDescriptor=NULL;
  event_sec.bInheritHandle=true;

  hevent_EmptyBlocksReady = CreateEvent(&event_sec,true,false,NULL);

  return true;
}

void csSoundDriverWaveOut::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csReportV (object_reg, severity, "crystalspace.sound.driver.waveout", msg, 
    arg);
  va_end (arg);
}

bool csSoundDriverWaveOut::Open(iSoundRender *render, int frequency,
  bool bit16, bool stereo)
{
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Wave-Out Sound Driver selected.");

  // store pointer to sound renderer
  if (!render) return false;
  SoundRender = render;
  SoundRender->IncRef();

  // store sound format settings
  Frequency = frequency;
  Bit16 = bit16;
  Stereo = stereo;

  // setup wave format
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
  float BufferLength=Config->GetFloat("Sound.WaveOut.BufferLength", (float)0.10);
  Report (CS_REPORTER_SEVERITY_NOTIFY, "  pre-buffering %f seconds of sound",
    BufferLength);
  waveout_device_index=(unsigned int)Config->GetInt("Sound.WaveOut.DeviceIndexOverride",(int)WAVE_MAPPER);

  // Send out NOTIFY messages listing available devices
  EnumerateAvailableDevices();

  // setup misc member variables

  // We'll keep 5 buffer-segments.  Every time one is empty we'll refill it and send it along again
  float f_memsize=(BufferLength * Format.nAvgBytesPerSec) / WAVEOUT_BUFFER_COUNT;

  MemorySize = (int)f_memsize; // Translate to an int
  MemorySize += Format.nBlockAlign; // Add the alignment size - this causes a round-up
  MemorySize -= (MemorySize%Format.nBlockAlign); // Truncate to an even alignment size
  Memory = 0;


  // Allocate the buffers
  int buffer_idx;
  
  AllocatedBlocks = new SoundBlock[WAVEOUT_BUFFER_COUNT];
  for (buffer_idx=0;buffer_idx<WAVEOUT_BUFFER_COUNT;buffer_idx++)
  {
    AllocatedBlocks[buffer_idx].Driver = this;
    AllocatedBlocks[buffer_idx].Prepared = false;
    AllocatedBlocks[buffer_idx].DataHandle = GlobalAlloc(GMEM_FIXED | GMEM_SHARE, sizeof(WAVEHDR) + MemorySize);
    AllocatedBlocks[buffer_idx].Data = (unsigned char *)GlobalLock(AllocatedBlocks[buffer_idx].DataHandle);
    AllocatedBlocks[buffer_idx].WaveHeader = (LPWAVEHDR)(AllocatedBlocks[buffer_idx].Data);

    // Zero out the block
    memset(AllocatedBlocks[buffer_idx].Data,0,sizeof(WAVEHDR) + MemorySize);

    EmptyBlocks.Push(&(AllocatedBlocks[buffer_idx]));
  }

  // Start the background thread
  bgThread.AttachNew (new BackgroundThread (this));
  csbgThread=csThread::Create (bgThread);
  csbgThread->Start();

  // Store old volume and set full volume because the software sound renderer
  // will apply volume internally. If this device does not allow volume
  // changes, all the volume set/get calls will fail, but output will be
  // correct. So don't check for errors here!
//  waveOutGetVolume(WaveOut, &OldVolume);
//  waveOutSetVolume(WaveOut, 0xffffffff);


  return S_OK;
}

void csSoundDriverWaveOut::Close()
{
  int wait_timer=0;
  // Tell the background thread to halt
  bgThread->RequestStop();

  // Signal the event that the background thread may be waiting on
  SetEvent(hevent_EmptyBlocksReady);

  // Wait a bit for it to shut down nicely.
  while (bgThread->IsRunning() && wait_timer++<120000)
    csSleep(0);
  //csbgThread->Wait ();

  // Clear out the EmptyBlocks queue
  EnterCriticalSection(&critsec_EmptyBlocks);

  while (EmptyBlocks.Length())
  {
    SoundBlock *Block=EmptyBlocks.Pop();
    GlobalUnlock(Block->DataHandle);
    GlobalFree(Block->DataHandle);
  }

  LeaveCriticalSection(&critsec_EmptyBlocks);
  DeleteCriticalSection(&critsec_EmptyBlocks);
  CloseHandle(hevent_EmptyBlocksReady);

  delete[] AllocatedBlocks;
  AllocatedBlocks = 0;

 
  if (SoundRender)
  {
    SoundRender->DecRef();
    SoundRender = 0;
  }

  Memory = 0;
}

void csSoundDriverWaveOut::EnumerateAvailableDevices()
{
  unsigned int num_devs,current_dev;
  WAVEOUTCAPS devcaps;

  num_devs=waveOutGetNumDevs();
  Report (CS_REPORTER_SEVERITY_NOTIFY, "%u WaveOut audio output devices available:",num_devs);

  //Sound.WaveOut.DeviceIndexOverride
  for (current_dev=0;current_dev<num_devs;current_dev++)
  {
    if (MMSYSERR_NOERROR == waveOutGetDevCaps(current_dev,&devcaps,sizeof(WAVEOUTCAPS)))
    {
      bool device_valid=true;

      // Check for compatible format
      if (!(devcaps.dwFormats & WAVE_FORMAT_4S16))
        device_valid=false;

      // Override selection if the selection cannot handle 44khz stereo
      if (waveout_device_index == current_dev && !device_valid)
        waveout_device_index=WAVE_MAPPER;

      // If it's still valid, this is the selected device
      if (waveout_device_index == current_dev)
        Report (CS_REPORTER_SEVERITY_NOTIFY, "Device %u : %s (SELECTED)",current_dev,devcaps.szPname);
      else
        Report (CS_REPORTER_SEVERITY_NOTIFY, "Device %u : %s (%s)",current_dev,devcaps.szPname,device_valid?"SUPPORTED":"NOT SUPPORTED");
    }
  }
  if (num_devs>1)
    Report (CS_REPORTER_SEVERITY_NOTIFY, "To override the default device selection, set the Sound.WaveOut.DeviceIndexOverride configuration value to the index of the desired device listed above.");
  
  // Trim out any bogus override values
  if (waveout_device_index >= num_devs)
    waveout_device_index=WAVE_MAPPER;
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

bool csSoundDriverWaveOut::HandleEvent(iEvent &e)
{
  return false;
}

/*
 * Applications should not call any system-defined functions from inside a callback function, 
 * except for EnterCriticalSection, LeaveCriticalSection, midiOutLongMsg, midiOutShortMsg, 
 * OutputDebugString, PostMessage, PostThreadMessage, SetEvent, timeGetSystemTime, timeGetTime, 
 * timeKillEvent, and timeSetEvent. Calling other wave functions will cause deadlock.
*/
void CALLBACK csSoundDriverWaveOut::waveOutProc(HWAVEOUT /*WaveOut*/,
  UINT uMsg, DWORD /*dwInstance*/, DWORD dwParam1, DWORD /*dwParam2*/)
{
  if (uMsg==MM_WOM_DONE) {
    LPWAVEHDR OldHeader = (LPWAVEHDR)dwParam1;
    if (OldHeader->dwUser == 0) return;
    SoundBlock *Block = (SoundBlock *)OldHeader->dwUser;
    Block->Driver->RecycleBlock(Block);
  }
}

void csSoundDriverWaveOut::RecycleBlock(SoundBlock *Block)
{
  // Add to the empty blocks list
  EnterCriticalSection(&critsec_EmptyBlocks);
  EmptyBlocks.Push(Block);
  LeaveCriticalSection(&critsec_EmptyBlocks);

  // Signal the background thread to pick it up and refill it
  SetEvent(hevent_EmptyBlocksReady);
}


const char *csSoundDriverWaveOut::BackgroundThread::GetMMError (MMRESULT r) 
{
  switch (r) 
  {
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

    default:
      {
	static char err[MAXERRORLENGTH * CS_UC_MAX_UTF8_ENCODED];
	if (cswinIsWinNT ())
	{
	  static WCHAR wideErr[MAXERRORLENGTH];
          waveOutGetErrorTextW (r, wideErr, 
	    sizeof (wideErr) / sizeof (WCHAR));
	  csUnicodeTransform::WCtoUTF8 ((utf8_char*)err, 
	    sizeof (err) / sizeof (char), wideErr, (size_t)-1);
	}
	else
	{
	  static char ansiErr[MAXERRORLENGTH];
          waveOutGetErrorTextA (r, ansiErr, 
	    sizeof (ansiErr) / sizeof (char));
	  wchar_t* wideErr = cswinAnsiToWide (ansiErr);
	  csUnicodeTransform::WCtoUTF8 ((utf8_char*)err, 
	    sizeof (err) / sizeof (char), wideErr, (size_t)-1);
	  delete[] wideErr;
	}
	return err;
      }
  }
}

bool csSoundDriverWaveOut::BackgroundThread::CheckError(const char *action, MMRESULT code)
{
  if (code != MMSYSERR_NOERROR) 
  {
    if (code != LastError)
    {
      parent_driver->Report (CS_REPORTER_SEVERITY_ERROR,
	"%s: %.8x %s .", action, code, GetMMError (code));
      LastError = code;
    }
    return false;
  }
  else
  {
    return true;
  }
}

bool csSoundDriverWaveOut::BackgroundThread::FillBlock(SoundBlock *Block)
{
  LPWAVEHDR lpWaveHdr;
  MMRESULT result;

  /* Unprepare the block, in case it was used previously.  Per MSDN an unprepare on a block that has
   *  not been previously prepared is safe - it does nothing and returns zero.
   *
   * However, testing on systems using C-Media sound cards proves that this does not hold for
   *  all cards/drivers.  On these cards, passing an unprepared buffer will return an error
   *  and apparently invalidate the opened device - causing sound to fail to be produced.
   */
  lpWaveHdr = (LPWAVEHDR)Block->Data;
  if (Block->Prepared)
  {
    result = waveOutUnprepareHeader(WaveOut, lpWaveHdr, sizeof(WAVEHDR));
    if (result != 0 && !CheckError("waveOutUnprepareHeader", result))
      return false;

    // Block is no longer prepared
    Block->Prepared=false;
  }

  // Ensure the block pointers are correct
  Block->WaveHeader = (LPWAVEHDR)Block->Data;

  // Parent Memory pointer is used by the renderer to lock memory to write into
  //  It's a very circular process
  parent_driver->Memory = Block->Data + sizeof(WAVEHDR);

  // call the sound renderer mixing function
  parent_driver->SoundRender->MixingFunction();

  // Set up and prepare header.
  lpWaveHdr->lpData = (char *)parent_driver->Memory;
  lpWaveHdr->dwBufferLength = parent_driver->MemorySize;
  lpWaveHdr->dwFlags = 0L;
  lpWaveHdr->dwLoops = 0L;
  lpWaveHdr->dwUser = (DWORD_PTR)Block;

  result = waveOutPrepareHeader(WaveOut, lpWaveHdr, sizeof(WAVEHDR));
  if (!CheckError("waveOutPrepareHeader", result)) {
      return false;
  }


  /* Now the data block can be sent to the output device. The
   * waveOutWrite function returns immediately and waveform
   * data is sent to the output device in the background.
   */
  result = waveOutWrite(WaveOut, lpWaveHdr, sizeof(WAVEHDR));
  if (!CheckError("waveOutWrite", result)) {
    result = waveOutUnprepareHeader(WaveOut, lpWaveHdr, sizeof(WAVEHDR));
    if (!CheckError("waveOutUnprepareHeader", result))
      Block->Prepared=true; // Mark the block as prepared if we couldn't unprepare it
    return false;
  }

  Block->Prepared=true;
  return true;
}

void csSoundDriverWaveOut::BackgroundThread::Run()
{
  MMRESULT result;
  size_t block_idx;
  int shutdown_wait_counter=0;
  running=true;

  // Cheating some.  csThread should be able to abstract this functionality, but it's not there
  //  So we'll go straight to the windows API.
  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);

  // Startup waveOut device
  result = waveOutOpen (&WaveOut, parent_driver->waveout_device_index, 
    &(parent_driver->Format), (DWORD_PTR)&waveOutProc, 0L, CALLBACK_FUNCTION);
  CheckError ("waveOutOpen", result);


  while (!request_stop)
  {
    int block_count;

    // Grab a lock on the available blocks list
    EnterCriticalSection(&(parent_driver->critsec_EmptyBlocks));

    // Reset the event, if a block comes in from here forward, the event will be set again by the time we try to wait
    ResetEvent(parent_driver->hevent_EmptyBlocksReady);

    // Fill all available blocks
    block_count=(int)parent_driver->EmptyBlocks.Length();
    while (block_count>0)
    {
      bool fill_result;
      SoundBlock *CurrentBlock=parent_driver->EmptyBlocks.Pop();

      // Release the lock during the buffer filling.  We don't need the lock here and it's the longest part of this operation
      LeaveCriticalSection(&(parent_driver->critsec_EmptyBlocks));
      fill_result=FillBlock(CurrentBlock);
      // Grab the lock back before continuing
      EnterCriticalSection(&(parent_driver->critsec_EmptyBlocks));

      if (!fill_result)
        parent_driver->EmptyBlocks.Insert(0,CurrentBlock);
      block_count--;
    }

    // Release the lock
    LeaveCriticalSection(&(parent_driver->critsec_EmptyBlocks));
    
    // Give up timeslice
    WaitForSingleObject(parent_driver->hevent_EmptyBlocksReady,INFINITE);
  }

  // Request all blocks to stop and be returned 
  result = waveOutReset(WaveOut);
  CheckError ("waveOutReset", result);

  // Need to wait for queued blocks to all be finished
  while (1)
  {
    // Dont stall forever
    if (shutdown_wait_counter++>100000)
      break; 

    // Grab a lock on the available blocks list
    EnterCriticalSection(&(parent_driver->critsec_EmptyBlocks));

    // If all blocks are free, don't wait anymore
    if (parent_driver->EmptyBlocks.Length() >= WAVEOUT_BUFFER_COUNT)
    {
      // Release the lock
      LeaveCriticalSection(&(parent_driver->critsec_EmptyBlocks));
      break;
    }

    // Release the lock
    LeaveCriticalSection(&(parent_driver->critsec_EmptyBlocks));
    // Give up timeslice
    csSleep(0);
  }

  // Unprepare any Blocks before closing the device
  EnterCriticalSection(&(parent_driver->critsec_EmptyBlocks));
  for (block_idx=0;block_idx<parent_driver->EmptyBlocks.Length();block_idx++)
  {
    SoundBlock *Block=parent_driver->EmptyBlocks[block_idx];
    if (Block->Prepared)
    {
      LPWAVEHDR lpWaveHdr;
      MMRESULT result;

      lpWaveHdr = (LPWAVEHDR)Block->Data;
      result = waveOutUnprepareHeader(WaveOut, lpWaveHdr, sizeof(WAVEHDR));
      if (CheckError("waveOutUnprepareHeader", result))
        Block->Prepared=false;
    }
  }
  LeaveCriticalSection(&(parent_driver->critsec_EmptyBlocks));

  // Close the wave output device
  waveOutClose(WaveOut);

  running=false;
}
