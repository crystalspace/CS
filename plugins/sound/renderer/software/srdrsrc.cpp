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
#include "cssndrdr/software/srdrbuf.h"
#include "cssndrdr/software/srdrsrc.h"
#include "isystem.h"

IMPLEMENT_FACTORY (csSoundSourceSoftware)

IMPLEMENT_IBASE(csSoundSourceSoftware)
  IMPLEMENTS_INTERFACE(iSoundSource)
IMPLEMENT_IBASE_END;

csSoundSourceSoftware::csSoundSourceSoftware(iBase *piBase)
{
	CONSTRUCT_IBASE(piBase);
  fPosX = fPosY = fPosZ = 0.0;
  fVelX = fVelY = fVelZ = 0.0;
  pSoundBuffer = NULL;
}

csSoundSourceSoftware::~csSoundSourceSoftware()
{
}

iSoundBuffer *csSoundSourceSoftware::GetSoundBuffer()
{
  if(!pSoundBuffer) return NULL;
  return QUERY_INTERFACE(pSoundBuffer, iSoundBuffer);
}

void csSoundSourceSoftware::SetPosition(float x, float y, float z)
{
  fPosX = x; fPosY = y; fPosZ = z;
}

void csSoundSourceSoftware::SetVelocity(float x, float y, float z)
{
  fVelX = x; fVelY = y; fVelZ = z;
}

void csSoundSourceSoftware::GetPosition(float &x, float &y, float &z)
{
  x = fPosX; y = fPosY; z = fPosZ;
}

void csSoundSourceSoftware::GetVelocity(float &x, float &y, float &z)
{
  x = fVelX; y = fVelY; z = fVelZ;
}

