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

#ifndef __CSSOUNDLISTENERSOFTWARE_H__
#define __CSSOUNDLISTENERSOFTWARE_H__

#include "isound/isndlstn.h"

class csSoundListenerSoftware : public iSoundListener
{
public:
	DECLARE_IBASE;
	csSoundListenerSoftware(iBase *piBase);
	virtual ~csSoundListenerSoftware();
	
	void SetDirection(csVector3 Front, csVector3 Top);
	void SetPosition(csVector3 pos);
	void SetVelocity(csVector3 v);
	void SetDistanceFactor(float factor);
	void SetRollOffFactor(float factor);
	void SetDopplerFactor(float ws);
	void SetHeadSize(float size);
	void SetEnvironment(SoundEnvironment env);
	
	void GetDirection(csVector3 &Front, csVector3 &Top);
	csVector3 GetPosition();
	csVector3 GetVelocity();
	float GetDistanceFactor();
	float GetRollOffFactor();
	float GetDopplerFactor();
	float GetHeadSize();
	SoundEnvironment GetEnvironment();
	
public:
	// Position
	csVector3 Position;
	// Velocity
	csVector3 Velocity;
	// Direction
	csVector3 Front, Top;
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
};

#endif
