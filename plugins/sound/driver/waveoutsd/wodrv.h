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
#include "csutil/cfgacc.h"
#include "isound/driver.h"
#include "isys/plugin.h"
#include "isys/system.h"

class csSoundDriverWaveOut : public iSoundDriver
{
public:
  SCF_DECLARE_IBASE;
	
  csSoundDriverWaveOut(iBase *piBase);
  virtual ~csSoundDriverWaveOut();

  // Implementation of interface for iPlugin
  virtual bool Initialize (iSystem *iSys);
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

  const char *GetMMError(MMRESULT code);
	
  struct eiPlugIn : public iPlugIn
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundDriverWaveOut);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent& e) { return scfParent->HandleEvent(e); }
  } scfiPlugIn;

protected:
  // system driver
  iSystem *System;

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
  csVector BlocksToDelete;

  // this function is called when a sound block is returned by wave-out
  static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance,
    DWORD dwParam1, DWORD dwParam2);
  // this function writes a new sound block to wave-out
  void SoundProc(LPWAVEHDR OldHeader);

  // wave-out device
  HWAVEOUT WaveOut;
  // old system volume
  DWORD OldVolume;
};

#endif // __CS_WODRV_H__
