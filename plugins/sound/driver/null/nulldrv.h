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

#ifndef __SOUND_DRIVER_NULL_H__
#define __SOUND_DRIVER_NULL_H__

// SoundDriver.H
// csSoundDriverNull class.

#include "cscom/com.h"
#include "isnddrv.h"

extern const CLSID CLSID_NullSoundDriver;

class csSoundDriverNull : public ISoundDriver
{
protected:
  ISystem* m_piSystem;
  void * memory;
  int memorysize;
  float volume;
  int m_nFrequency;
  bool m_b16Bits;
  bool m_bStereo;

public:
	csSoundDriverNull(ISystem* piSystem);

	virtual ~csSoundDriverNull();

  STDMETHODIMP Open(ISoundRender *render, int frequency, bool bit16, bool stereo);
  STDMETHODIMP Close();
  
	STDMETHODIMP SetVolume(float vol);
  STDMETHODIMP GetVolume(float *vol);
	STDMETHODIMP LockMemory(void **mem, int *memsize);
  STDMETHODIMP UnlockMemory();
  STDMETHODIMP IsBackground(bool *back);
	STDMETHODIMP Is16Bits(bool *bit);
	STDMETHODIMP IsStereo(bool *stereo);
	STDMETHODIMP GetFrequency(int *freq);
  STDMETHODIMP IsHandleVoidSound(bool *handle);

  DECLARE_IUNKNOWN()
	DECLARE_INTERFACE_TABLE(csSoundDriverNull)

  /// print to the system's device
  void SysPrintf(int mode, char* str, ...);
};

class csSoundDriverNullFactory : public ISoundDriverFactory
{
    STDMETHODIMP CreateInstance(REFIID riid, ISystem* piSystem, void** ppv);

    /// Lock or unlock from memory.
    STDMETHODIMP LockServer(BOOL bLock);

    DECLARE_IUNKNOWN()
    DECLARE_INTERFACE_TABLE(csSoundDriverNullFactory)
};


#endif	//__NETWORK_DRIVER_NULL_H__

