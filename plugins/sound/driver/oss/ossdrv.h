/*
	Copyright (C) 1998, 1999 by Nathaniel 'NooTe' Saint Martin
	Copyright (C) 1998, 1999 by Jorrit Tyberghein
	Written by Nathaniel 'NooTe' Saint Martin
	Linux sound driver by Gert Steenssens <gsteenss@eps.agfa.be>

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

#ifndef __SOUND_DRIVER_OSS_H__
#define __SOUND_DRIVER_OSS_H__

// SoundDriver.H
// csSoundDriverOSS class.

#include "csutil/scf.h"
#include "isound/driver.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/time.h>


#define SOUND_DEVICE "/dev/dsp"

extern bool AudioDeviceBlocked();

/// audio device
class AudioDevice
{
public:

  AudioDevice();

  /// opens device with specified params
  bool Open(int& frequency, bool& bit16, bool& stereo, int& fragments, int& block_size );

  /// close device
  void Close();
  void Play( unsigned char *snddata, int len );

private:

  int audio;

};

class csSoundDriverOSS : public iSoundDriver
{
protected:
  iSystem* m_piSystem;
  void * memory;
  int memorysize;
  int m_nFrequency;
  bool m_b16Bits;
  bool m_bStereo;
  int fragments;
  int block_size;
  int block;
  unsigned char *soundbuffer;

public:
  DECLARE_IBASE;
  csSoundDriverOSS(iBase *iParent);
  virtual ~csSoundDriverOSS();

  bool Initialize(iSystem *iSys);
  
  bool Open(iSoundRender *render, int frequency, bool bit16, bool stereo);
  void Close();
  
  void LockMemory(void **mem, int *memsize);
  void UnlockMemory();
  bool IsBackground();
  bool Is16Bits();
  bool IsStereo();
  int GetFrequency();
  bool IsHandleVoidSound();
  
private:
  // used to setup timer when background=true (not currently used)
  bool SetupTimer( int nTimesPerSecond );
  
  AudioDevice device;
  struct sigaction oldact;
  struct itimerval otime;
  bool bTimerInstalled, bSignalInstalled;
 public:
  iSoundRender *m_piSoundRender;
};

#endif	//__SOUND_DRIVER_OSS_H__

