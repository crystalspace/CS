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
#include "cssndrdr/null/nrdrlst.h"
#include "isystem.h"

IMPLEMENT_IBASE (csSoundListenerNull)
  IMPLEMENTS_INTERFACE (iSoundListener)
IMPLEMENT_IBASE_END

csSoundListenerNull::csSoundListenerNull()
{
  fPosX = fPosY = fPosZ = 0.0;
  fDirTopX = fDirTopY = fDirTopZ = 0.0;
  fDirFrontX = fDirFrontY = fDirFrontZ = 0.0;
  fVelX = fVelY = fVelZ = 0.0;
  fDoppler = 1.0;
  fDistance = 1.0;
  fRollOff = 1.0;
}

csSoundListenerNull::~csSoundListenerNull()
{
}

void csSoundListenerNull::SetPosition(float x, float y, float z)
{
  fPosX = x; fPosY = y; fPosZ = z;
}

void csSoundListenerNull::SetDirection(float fx, float fy, float fz, float tx, float ty, float tz)
{
  fDirFrontX = fx; fDirFrontY = fy; fDirFrontZ = fz;
  fDirTopX = tx; fDirTopY = ty; fDirTopZ = tz;
}

void csSoundListenerNull::SetVelocity(float x, float y, float z)
{
  fVelX = x; fVelY = y; fVelZ = z;
}

void csSoundListenerNull::SetHeadSize(float size)
{
  fHeadSize = size;
}

void csSoundListenerNull::SetDopplerFactor(float factor)
{
  fDoppler = factor;
}

void csSoundListenerNull::SetDistanceFactor(float factor)
{
  fDistance = factor;
}

void csSoundListenerNull::SetRollOffFactor(float factor)
{
  fRollOff = factor;
}

void csSoundListenerNull::SetEnvironment(SoundEnvironment env)
{
  Environment = env;
}

void csSoundListenerNull::GetPosition(float &x, float &y, float &z)
{
  x = fPosX; y = fPosY; z = fPosZ;
}

void csSoundListenerNull::GetDirection(float &fx, float &fy, float &fz, float &tx, float &ty, float &tz)
{
  fx = fDirFrontX; fy = fDirFrontY; fz = fDirFrontZ;
  tx = fDirTopX; ty = fDirTopY; tz = fDirTopZ;
}

float csSoundListenerNull::GetHeadSize()
{
  return fHeadSize;
}

void csSoundListenerNull::GetVelocity(float &x, float &y, float &z)
{
  x = fVelX; y = fVelY; z = fVelZ;
}

float csSoundListenerNull::GetDopplerFactor()
{
  return fDoppler;
}

float csSoundListenerNull::GetDistanceFactor()
{
  return fDistance;
}

float csSoundListenerNull::GetRollOffFactor()
{
  return fRollOff;
}

SoundEnvironment csSoundListenerNull::GetEnvironment()
{
  return Environment;
}
