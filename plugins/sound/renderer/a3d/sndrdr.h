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

#ifndef __SOUND_RENDER_A3D_H__
#define __SOUND_RENDER_A3D_H__

// SoundRender.H
// csSoundRenderA3D class.

#include "csutil/scf.h"
#include "cssfxldr/common/snddata.h"
#include "isndrdr.h"

class csSoundListenerA3D;

class csSoundRenderA3D : public iSoundRender
{
public:
	DECLARE_IBASE;
	csSoundRenderA3D(iBase *piBase);
	virtual ~csSoundRenderA3D();


	bool Initialize(iSystem *iSys);
	
	bool Open();
	void Close();
	
	void Update();
	
	void SetVolume(float vol);
	float GetVolume();
	
	void PlayEphemeral(csSoundData *snd);
	
	iSoundListener *GetListener();
	iSoundSource *CreateSource(csSoundData *snd);
	iSoundBuffer *CreateSoundBuffer(csSoundData *snd);
	
	void MixingFunction() { }
	
public:
	IA3d4 *m_p3DAudioRenderer;
	iSystem* m_piSystem;
	
	/// print to the system's device
	void SysPrintf(int mode, char* str, ...);
	
	csSoundListenerA3D* m_pListener;
};

#endif	//__SOUND_RENDER_A3D_H__
