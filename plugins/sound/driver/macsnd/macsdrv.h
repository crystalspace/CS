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

#ifndef __SOUND_DRIVER_MAC_H__
#define __SOUND_DRIVER_MAC_H__

// SoundDriver.H
// csSoundDriverMac class.

#include <Sound.h>

#include "cscom/com.h"
#include "isnddrv.h"

extern const CLSID CLSID_MacSoundDriver;

class csSoundDriverMac : public ISoundDriver
{
protected:
  ISystem* m_piSystem;
  ISoundRender *m_piSoundRender;
  void * Memory;
  int MemorySize;
  float volume;
  int m_nFrequency;
  bool m_b16Bits;
  bool m_bStereo;

public:
	csSoundDriverMac(ISystem* piSystem);

	virtual ~csSoundDriverMac();

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
	DECLARE_INTERFACE_TABLE(csSoundDriverMac)

  /// print to the system's device
	void SysPrintf(int mode, char* str, ...);

	void SndDoubleBackProc( SndChannelPtr channel, SndDoubleBufferPtr doubleBuffer );

private:
	SndDoubleBufferHeader	mSoundDBHeader;
	SndDoubleBuffer			mSoundDoubleBuffer;
	SndChannelPtr			mSoundChannel;

	long	mFramesPerBuffer;
	bool	mStopPlayback;
	long	mOutputVolume;

	long	mBuffersFilled;

};

class csSoundDriverMacFactory : public ISoundDriverFactory
{
    STDMETHODIMP CreateInstance(REFIID riid, ISystem* piSystem, void** ppv);

    /// Lock or unlock from memory.
    STDMETHODIMP LockServer(BOOL bLock);

    DECLARE_IUNKNOWN()
    DECLARE_INTERFACE_TABLE(csSoundDriverMacFactory)
};


#endif	//__SOUND_DRIVER_MAC_H__

