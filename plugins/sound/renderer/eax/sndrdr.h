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

#ifndef __SOUND_RENDER_EAX_H__
#define __SOUND_RENDER_EAX_H__

// SoundRender.H
// csSoundRenderEAX class.

#include "cscom/com.h"
#include "cssfxldr/common/snddata.h"
#include "isndrdr.h"

class csSoundListenerEAX;

extern const CLSID CLSID_EAXSoundRender;

class csSoundRenderEAX : public ISoundRender
{
public:
	csSoundRenderEAX(ISystem* piSystem);

	virtual ~csSoundRenderEAX();

  STDMETHODIMP Open();
  STDMETHODIMP Close();
  
  STDMETHODIMP Update();

  STDMETHODIMP SetVolume(float vol);
  STDMETHODIMP GetVolume(float *vol);

  STDMETHODIMP PlayEphemeral(csSoundData *snd);

  STDMETHODIMP GetListener(ISoundListener** ppv );
  STDMETHODIMP CreateSource(ISoundSource** ppv, csSoundData *snd);
  STDMETHODIMP CreateSoundBuffer(ISoundBuffer ** ppv, csSoundData *snd);

  STDMETHODIMP MixingFunction() {return S_OK;}

	DECLARE_IUNKNOWN()
	DECLARE_INTERFACE_TABLE(csSoundRenderEAX)

public:
	LPDIRECTSOUND		m_p3DAudioRenderer;
  ISystem* m_piSystem;

  /// print to the system's device
  void SysPrintf(int mode, char* str, ...);

  csSoundListenerEAX* m_pListener;
};

class csSoundRenderEAXFactory : public ISoundRenderFactory
{
    STDMETHODIMP CreateInstance(REFIID riid, ISystem* piSystem, void** ppv);

    /// Lock or unlock from memory.
    STDMETHODIMP LockServer(COMBOOL bLock);

    DECLARE_IUNKNOWN()
    DECLARE_INTERFACE_TABLE(csSoundRenderEAXFactory)
};


#endif	//__SOUND_RENDER_DS3D_H__

