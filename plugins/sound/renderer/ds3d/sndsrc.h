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

#if !defined(__CSSOUNDSOURCEDS3D_H__)
#define __CSSOUNDSOURCEDS3D_H__

#include "cssfxldr/common/snddata.h"
#include "isndrdr.h"
#include "isndsrc.h"

class csSoundBufferDS3D;

class csSoundSourceDS3D : public iSoundSource
{
public:
	DECLARE_IBASE;
	csSoundSourceDS3D(iBase *piBase);
	virtual ~csSoundSourceDS3D();
	
	void SetPosition(float x, float y, float z);
	void SetVelocity(float x, float y, float z);
	
	void GetPosition(float &x, float &y, float &z);
	void GetVelocity(float &x, float &y, float &z);
	
	iSoundBuffer *GetSoundBuffer();
	
public:
	/// Position of sound object
	float fPosX, fPosY, fPosZ;
	/// Velocity of sound object
	float fVelX, fVelY, fVelZ;
	
	// SoundBuffer
	csSoundBufferDS3D *pSoundBuffer;
	
	int DestroySource();
	int CreateSource();
	
	LPDIRECTSOUND3DBUFFER	m_pDS3DBuffer3D;
	LPDIRECTSOUNDBUFFER	m_pDS3DBuffer2D;
	LPDIRECTSOUND		m_p3DAudioRenderer;
};

#endif // __CSSOUNDSOURCEDS3D_H__
