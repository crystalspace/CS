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

#include "sysdef.h"
#include "csutil/scf.h"
#include "cssndrdr/software/srdrlst.h"
#include "isystem.h"

IMPLEMENT_FACTORY (csSoundListenerSoftware);

IMPLEMENT_IBASE(csSoundListenerSoftware)
IMPLEMENTS_INTERFACE(iSoundListener)
IMPLEMENT_IBASE_END

csSoundListenerSoftware::csSoundListenerSoftware(iBase *piBase)
{
	CONSTRUCT_IBASE(piBase);
	fPosX = fPosY = fPosZ = 0.0;
	fDirTopX = fDirTopY = fDirTopZ = 0.0;
	fDirFrontX = fDirFrontY = fDirFrontZ = 0.0;
	fDoppler = 1.0;
	fDistance = 1.0;
	fRollOff = 1.0;
}

csSoundListenerSoftware::~csSoundListenerSoftware()
{
	
}

void csSoundListenerSoftware::SetPosition(float x, float y, float z)
{
	fPosX = x; fPosY = y; fPosZ = z;
}

void csSoundListenerSoftware::SetDirection(float fx, float fy, float fz, float tx, float ty, float tz)
{
	fDirFrontX = fx; fDirFrontY = fy; fDirFrontZ = fz;
	fDirTopX = tx; fDirTopY = ty; fDirTopZ = tz;
}

void csSoundListenerSoftware::SetHeadSize(float size)
{
	fHeadSize = size;
}

void csSoundListenerSoftware::SetVelocity(float x, float y, float z)
{
	fVelX = x; fVelY = y; fVelZ = z;
}

void csSoundListenerSoftware::SetDopplerFactor(float factor)
{
	fDoppler = factor;
}

void csSoundListenerSoftware::SetDistanceFactor(float factor)
{
	fDistance = factor;
}

void csSoundListenerSoftware::SetRollOffFactor(float factor)
{
	fRollOff = factor;
}

void csSoundListenerSoftware::SetEnvironment(SoundEnvironment env)
{
	Environment = env;
}

void csSoundListenerSoftware::GetPosition(float &x, float &y, float &z)
{
	x = fPosX; y = fPosY; z = fPosZ;
}

void csSoundListenerSoftware::GetDirection(float &fx, float &fy, float &fz, float &tx, float &ty, float &tz)
{
	fx = fDirFrontX; fy = fDirFrontY; fz = fDirFrontZ;
	tx = fDirTopX; ty = fDirTopY; tz = fDirTopZ;
}

float csSoundListenerSoftware::GetHeadSize()
{
	return fHeadSize;
}

void csSoundListenerSoftware::GetVelocity(float &x, float &y, float &z)
{
	x = fVelX; y = fVelY; z = fVelZ;
}

float csSoundListenerSoftware::GetDopplerFactor()
{
	return fDoppler;
}

float csSoundListenerSoftware::GetDistanceFactor()
{
	return fDistance;
}

float csSoundListenerSoftware::GetRollOffFactor()
{
	return fRollOff;
}

SoundEnvironment csSoundListenerSoftware::GetEnvironment()
{
	return Environment;
}
