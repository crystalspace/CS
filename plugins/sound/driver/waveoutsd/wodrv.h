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
#include "isound/driver.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

#include <mmsystem.h>

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
    HGLOBAL DataHandle;
    unsigned char *Data;
    LPWAVEHDR WaveHeader;
  };

  // system driver
  iObjectRegistry *object_reg;

  // config file
  csConfigAccess Config;

  // sound renderer
  iSoundRender *SoundRender;

  // sound memory
  void *Memory;
  int MemorySize;

  // sound format
  int Frequency;
  bool Bit16;
  bool Stereo;

  // switch to activate/deactivate the sound proc
  volatile bool ActivateSoundProc;
  // is the sound proc locked?
  volatile bool SoundProcLocked;
  // number of sound blocks to write
  volatile int NumSoundBlocksToWrite;
  // list of blocks to delete
  csArray<SoundBlock*> BlocksToDelete;
  // has playback already started?
  volatile int Playback;
  // when the same error occurs multiple times, we just show the first
  volatile MMRESULT LastError;

  // this function is called when a sound block is returned by wave-out
  static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance,
    DWORD dwParam1, DWORD dwParam2);
  // this function writes a new sound block to wave-out
  void SoundProc(LPWAVEHDR OldHeader);

  bool CheckError(const char *action, MMRESULT code);
  const char *GetMMError(MMRESULT code);

  // wave-out device
  HWAVEOUT WaveOut;
  // old system volume
  DWORD OldVolume;
};

#endif // __CS_WODRV_H__
