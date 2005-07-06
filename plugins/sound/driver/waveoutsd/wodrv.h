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

#ifndef __CS_WODRV_H__
#define __CS_WODRV_H__

#include "csutil/scf.h"
#include "csutil/array.h"
#include "csutil/cfgacc.h"
#include "csutil/thread.h"
#include "isound/driver.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

#include <mmsystem.h>

/* This driver uses the windows MMSystem wave output functions.  This system has been around forever and it sometimes behaves a bit fickle.
 *
 * This driver kicks off its own thread to attempt to ensure that data is fed to the underlying system in a timely manner.  Additionally, MMSystem
 *  kicks off its own thread in the process space as well.  This means if you use this driver you get 2 extra threads.  Threads are cheap, don't
 *  sweat it.
 *
 * The MMSystem thread performs all the callbacks to the application, but it's not safe to do any work inside this callback - you're extremely limited
 *  in what functions you can safely call.  See the note above the definition of waveOutProc in wodrv.cpp for a list.  The callback receives blocks that
 *  the driver is done with.  All we do here is put those into a queue to be refilled and handed back to the driver.
 *
 * Our own background thread sits in a loop with a sleep(0), waiting for free blocks to become available.  When they are it calls back into the renderer
 *  to have it fill them.  This seems somewhat backwards, having the driver call the renderer, but since the GNU/Linux driver seems to work fine, and this
 *  driver has always done things this way, it's the easiest way to approach this without creating new problems elsewhere.
 *
 * Note that this means the Software Renderer call path invoked from here should be thread safe (real mutexes should lock all the data that may be accessed
 *  by both the call path from this background thread and the main application thread).
 *
 * If you alter or create any code that touches EmptyBlocks, make sure you lock mutex_EmptyBlocks while accessing it, and release it (from all exit paths)
 *  afterward.
 *
 *
 *  This plugin respects the following configuration options:
 *  Sound.WaveOut.BufferLength - A floating point value specifying the length of buffers (in seconds) created to
 *                                     hold data from the renderer (streaming AND static audio). (default is 0.05 seconds)
 *                               Increasing this value will cause delays in sound actions (button clicks are especially noticable)
 *  Sound.WaveOut.DeviceIndexOverride - An integer value 0 - (Number of devices-1) overriding the default selection of output device.
*/
class csSoundDriverWaveOut : public iSoundDriver
{
public:
  SCF_DECLARE_IBASE;

  csSoundDriverWaveOut(iBase *piBase);
  virtual ~csSoundDriverWaveOut();

  void Report (int severity, const char* msg, ...);

  // Implementation of interface for iComponent
  virtual bool Initialize (iObjectRegistry *object_reg);
  virtual bool HandleEvent (iEvent &e);
  virtual bool Open(iSoundRender*, int frequency, bool bit16, bool stereo);
  virtual void Close();
  virtual void LockMemory(void **mem, int *memsize);
  virtual void UnlockMemory();
  virtual bool IsBackground();
  virtual bool Is16Bits();
  virtual bool IsStereo();
  virtual int GetFrequency();
  virtual bool IsHandleVoidSound();

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundDriverWaveOut);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
  struct EventHandler : public iEventHandler
  {
  private:
    csSoundDriverWaveOut* parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler (csSoundDriverWaveOut* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE();
    }
    virtual bool HandleEvent (iEvent& e) { return parent->HandleEvent(e); }
  } * scfiEventHandler;


protected:
  struct SoundBlock 
  {
    csSoundDriverWaveOut *Driver;
    bool Prepared; 
    HGLOBAL DataHandle;
    unsigned char *Data;
    LPWAVEHDR WaveHeader;
  };

  // this function is called when a sound block is returned by wave-out
  static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance,
    DWORD dwParam1, DWORD dwParam2);
  // This function is called by the above callback to put the returned block into the recycle queue
  void RecycleBlock(SoundBlock *Block);

  void EnumerateAvailableDevices();


  class BackgroundThread : public csRunnable
  {
  public:
    BackgroundThread (csSoundDriverWaveOut *driver): count(1), 
      parent_driver(driver), running(false), request_stop(false) {}
    virtual ~BackgroundThread () {}
    virtual void IncRef () 
    {
      count++;
    }
    // If the thread is still running and we're going to delete this object, we are in for a crash
    virtual void DecRef () 
    {
      if (--count == 0) 
      { 
	CS_ASSERT (!running); 
	delete this; 
      } 
    }
    virtual int GetRefCount () { return count; }
    /// For the situations in which this will be used, the lack of a mutex is OK
    virtual void RequestStop () { request_stop=true; }
    /// For the situations in which this will be used, the lack of a mutex is OK
    virtual bool IsRunning() { return running; }
    virtual void Run ();
    virtual bool FillBlock(SoundBlock *Block);
  private:
    // Helper function to check for error result codes from MM function calls
    bool CheckError(const char *action, MMRESULT code);
    // MM error translator
    const char *GetMMError(MMRESULT code);
  public:
    int count;
  private:
    csSoundDriverWaveOut *parent_driver;
    volatile bool running, request_stop;
    // when the same error occurs multiple times, we just show the first
    MMRESULT LastError;
    // wave-out device
    HWAVEOUT WaveOut;
  };
  friend class BackgroundThread;

  // system driver
  iObjectRegistry *object_reg;

  // config file
  csConfigAccess Config;

  // sound renderer
  iSoundRender *SoundRender;

  // sound memory
  void *Memory;
  int MemorySize;

  // Selected waveout device index.
  unsigned int waveout_device_index;

  // Pointer to the array of allocated SoundBlock structures
  SoundBlock *AllocatedBlocks;
  // list of blocks that are available to fill with sound data
  csArray<SoundBlock*> EmptyBlocks;

  // sound format
  WAVEFORMATEX Format;
  int Frequency;
  bool Bit16;
  bool Stereo;

  // old system volume
  DWORD OldVolume;

  /// Critical section wrapping the EmptyBlocks queue which must be accessable by both the background thread and the MMsystem callback.
  CRITICAL_SECTION critsec_EmptyBlocks;
  /// An event that is signalled by the callback when empty blocks are ready to be processed
  HANDLE hevent_EmptyBlocksReady;

  /// CS representation of running background thread
  csRef<csThread> csbgThread;
  /// Background thread object
  csRef<BackgroundThread> bgThread;
};




#endif // __CS_WODRV_H__
