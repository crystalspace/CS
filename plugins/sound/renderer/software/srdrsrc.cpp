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
#include "cssndrdr/software/srdrsrc.h"
#include "isystem.h"

IMPLEMENT_UNKNOWN_NODELETE (csSoundSourceSoftware)

BEGIN_INTERFACE_TABLE(csSoundSourceSoftware)
  IMPLEMENTS_INTERFACE(ISoundSource)
END_INTERFACE_TABLE()

csSoundSourceSoftware::csSoundSourceSoftware()
{
  info.fPosX = info.fPosY = info.fPosZ = 0.0;
  info.fVelX = info.fVelY = info.fVelZ = 0.0;
}

csSoundSourceSoftware::~csSoundSourceSoftware()
{

}

STDMETHODIMP csSoundSourceSoftware::PlaySource(bool inloop)
{
  setStarted(true);
  inLoop(inloop);

  return S_OK;
}

STDMETHODIMP csSoundSourceSoftware::StopSource()
{
  setStarted(false);

  return S_OK;
}

STDMETHODIMP csSoundSourceSoftware::SetPosition(float x, float y, float z)
{
  info.fPosX = x; info.fPosY = y; info.fPosZ = z;

  return S_OK;
}

STDMETHODIMP csSoundSourceSoftware::SetVelocity(float x, float y, float z)
{
  info.fVelX = x; info.fVelY = y; info.fVelZ = z;

  return S_OK;
}

STDMETHODIMP csSoundSourceSoftware::GetInfoSource(csSoundSourceInfo *i)
{
  *i=info;

  return S_OK;
}
