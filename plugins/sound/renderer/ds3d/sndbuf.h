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

#if !defined(__CSSOUNDBUFFERDS3D_H__)
#define __CSSOUNDBUFFERDS3D_H__

#include "isndbuf.h"
#include "isndsrc.h"

struct iSoundRender;
class csSoundData;

class csSoundBufferDS3D : public iSoundBuffer
{
public:
	DECLARE_IBASE;
	csSoundBufferDS3D(iBase *piBase);
	virtual ~csSoundBufferDS3D();
	
	void Stop();
	void Play(SoundBufferPlayMethod playMethod);
	
	void SetVolume(float vol);
	float GetVolume();
	
	void SetFrequencyFactor(float factor);
	float GetFrequencyFactor();
	
	iSoundSource *CreateSource();
	
public:
	float fFrequencyOrigin;
	float fFrequencyFactor;
	float fVolume;
	
	int DestroySoundBuffer();
	int CreateSoundBuffer(iSoundRender *render, csSoundData * sound);
	
	LPDIRECTSOUNDBUFFER		m_pDS3DBuffer2D;
	LPDIRECTSOUND		m_p3DAudioRenderer;
};

#endif // __CSSOUNDBUFFERDS3D_H__
