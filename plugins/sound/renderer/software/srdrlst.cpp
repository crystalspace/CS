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
#include "cscom/com.h"
#include "cssndrdr/software/srdrlst.h"
#include "isystem.h"

IMPLEMENT_UNKNOWN_NODELETE (csSoundListenerSoftware)

BEGIN_INTERFACE_TABLE(csSoundListenerSoftware)
  IMPLEMENTS_INTERFACE(ISoundListener)
END_INTERFACE_TABLE()

csSoundListenerSoftware::csSoundListenerSoftware()
{
  info.fPosX = info.fPosY = info.fPosZ = 0.0;
  info.fDirTopX = info.fDirTopY = info.fDirTopZ = 0.0;
  info.fDirFrontX = info.fDirFrontY = info.fDirFrontZ = 0.0;
  info.fDoppler = 1.0;
  info.fDistance = 1.0;
  info.fRollOff = 1.0;
}

csSoundListenerSoftware::~csSoundListenerSoftware()
{

}

STDMETHODIMP csSoundListenerSoftware::SetPosition(float x, float y, float z)
{
  info.fPosX = x; info.fPosY = y; info.fPosZ = z;

  return S_OK;
}

STDMETHODIMP csSoundListenerSoftware::SetDirection(float fx, float fy, float fz, float tx, float ty, float tz)
{
  info.fDirFrontX = fx; info.fDirFrontY = fy; info.fDirFrontZ = fz;
  info.fDirTopX = tx; info.fDirTopY = ty; info.fDirTopZ = tz;

  return S_OK;
}

STDMETHODIMP csSoundListenerSoftware::SetHeadSize(float size)
{
  info.fHeadSize = size;

  return S_OK;
}

STDMETHODIMP csSoundListenerSoftware::SetVelocity(float x, float y, float z)
{
  info.fVelX = x; info.fVelY = y; info.fVelZ = z;

  return S_OK;
}

STDMETHODIMP csSoundListenerSoftware::SetDopplerFactor(float factor)
{
  info.fDoppler = factor;

  return S_OK;
}

STDMETHODIMP csSoundListenerSoftware::SetDistanceFactor(float factor)
{
  info.fDistance = factor;

  return S_OK;
}

STDMETHODIMP csSoundListenerSoftware::SetRollOffFactor(float factor)
{
  info.fRollOff = factor;

  return S_OK;
}

STDMETHODIMP csSoundListenerSoftware::SetEnvironment(SoundEnvironment env)
{
  info.Environment = env;

  return S_OK;
}

STDMETHODIMP csSoundListenerSoftware::GetInfoListener(csSoundListenerInfo *i)
{
  *i = info;

  return S_OK;
}
