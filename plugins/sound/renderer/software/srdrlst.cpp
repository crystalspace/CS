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

#include "cssysdef.h"
#include "srdrlst.h"

IMPLEMENT_FACTORY (csSoundListenerSoftware);

IMPLEMENT_IBASE(csSoundListenerSoftware)
IMPLEMENTS_INTERFACE(iSoundListener)
IMPLEMENT_IBASE_END

csSoundListenerSoftware::csSoundListenerSoftware(iBase *piBase)
{
  CONSTRUCT_IBASE(piBase);
  Position = csVector3(0,0,0);
  Velocity = csVector3(0,0,0);
  Front = csVector3(0,0,1);
  Top = csVector3(0,1,0);
  fDoppler = 1.0;
  fDistance = 1.0;
  fRollOff = 1.0;
  fHeadSize = 1.0;
  Environment = ENVIRONMENT_GENERIC;
}

csSoundListenerSoftware::~csSoundListenerSoftware()
{
}

void csSoundListenerSoftware::SetPosition(csVector3 pos)
{
  Position = pos;
}

void csSoundListenerSoftware::SetDirection(csVector3 f, csVector3 t)
{
  Front=f;
  Top=t;
}

void csSoundListenerSoftware::SetHeadSize(float size)
{
  fHeadSize = size;
}

void csSoundListenerSoftware::SetVelocity(csVector3 v)
{
  Velocity=v;
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

csVector3 csSoundListenerSoftware::GetPosition()
{
  return Position;
}

void csSoundListenerSoftware::GetDirection(csVector3 &f, csVector3 &t)
{
  f = Front;
  t = Top;
}

float csSoundListenerSoftware::GetHeadSize()
{
  return fHeadSize;
}

csVector3 csSoundListenerSoftware::GetVelocity()
{
  return Velocity;
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
