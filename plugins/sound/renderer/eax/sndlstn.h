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

#if !defined(__CSSoundListenerEAX_H__)
#define __CSSoundListenerEAX_H__

#include "cssfxldr/common/snddata.h"
#include "isndrdr.h"
#include "isndlstn.h"

class csSoundListenerEAX  : public iSoundListener
{
public:
	DECLARE_IBASE;
	csSoundListenerEAX(iBase *piBase);
	virtual ~csSoundListenerEAX();
	
	void SetDirection(float fx, float fy, float fz, float tx, float ty, float tz);
	void SetPosition(float x, float y, float z);
	void SetVelocity(float x, float y, float z);
	void SetDistanceFactor(float factor);
	void SetRollOffFactor(float factor);
	void SetDopplerFactor(float factor);
	void SetHeadSize(float size);
	void SetEnvironment(SoundEnvironment env);
	
	void GetDirection(float &fx, float &fy, float &fz, float &tx, float &ty, float &tz);
	void GetPosition(float &x, float &y, float &z);
	void GetVelocity(float &x, float &y, float &z);
	float GetDistanceFactor();
	float GetRollOffFactor();
	float GetDopplerFactor();
	float GetHeadSize();
	SoundEnvironment GetEnvironment();
	
public:
	// Position
	float fPosX, fPosY, fPosZ;
	// Velocity
	float fVelX, fVelY, fVelZ;
	// Direction
	float fDirTopX, fDirTopY, fDirTopZ, fDirFrontX, fDirFrontY, fDirFrontZ;
	// Doppler
	float fDoppler;
	// Distance
	float fDistance;
	// RollOff
	float fRollOff;
	// HeadSize
	float fHeadSize;
	// Environment
	SoundEnvironment Environment;
	
	bool m_bEaxSupported;
	LPKSPROPERTYSET m_pKsPropertiesSet;
	
	int CreateListener(iSoundRender *render);
	int DestroyListener();
	
	LPDIRECTSOUNDBUFFER		m_pDS3DPrimaryBuffer;
	LPDIRECTSOUND3DLISTENER	m_pDS3DListener;
	LPDIRECTSOUND		m_p3DAudioRenderer;
};

#endif // __CSSoundListenerEAX_H__
