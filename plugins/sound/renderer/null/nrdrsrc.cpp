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
#include "cssndrdr/null/nrdrbuf.h"
#include "cssndrdr/null/nrdrsrc.h"
#include "isystem.h"

IMPLEMENT_UNKNOWN_NODELETE (csSoundSourceNull)

BEGIN_INTERFACE_TABLE(csSoundSourceNull)
  IMPLEMENTS_INTERFACE(ISoundSource)
END_INTERFACE_TABLE()

csSoundSourceNull::csSoundSourceNull()
{
  fPosX = fPosY = fPosZ = 0.0;
  fVelX = fVelY = fVelZ = 0.0;
  pSoundBuffer = NULL;
}

csSoundSourceNull::~csSoundSourceNull()
{

}

STDMETHODIMP csSoundSourceNull::GetSoundBuffer(ISoundBuffer **ppv)
{
  if(!pSoundBuffer)
  {
    return E_FAIL;
  }

  return pSoundBuffer->QueryInterface (IID_ISoundBuffer, (void**)ppv);
}

STDMETHODIMP csSoundSourceNull::SetPosition(float x, float y, float z)
{
  fPosX = x; fPosY = y; fPosZ = z;

  return S_OK;
}

STDMETHODIMP csSoundSourceNull::SetVelocity(float x, float y, float z)
{
  fVelX = x; fVelY = y; fVelZ = z;

  return S_OK;
}

STDMETHODIMP csSoundSourceNull::GetPosition(float &x, float &y, float &z)
{
  x = fPosX; y = fPosY; z = fPosZ;

  return S_OK;
}

STDMETHODIMP csSoundSourceNull::GetVelocity(float &x, float &y, float &z)
{
  x = fVelX; y = fVelY; z = fVelZ;

  return S_OK;
}
