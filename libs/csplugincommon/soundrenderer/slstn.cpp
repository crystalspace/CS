/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "csplugincommon/soundrenderer/slstn.h"

SCF_IMPLEMENT_IBASE(csSoundListener);
  SCF_IMPLEMENTS_INTERFACE(iSoundListener);
SCF_IMPLEMENT_IBASE_END

csSoundListener::csSoundListener()
{
  SCF_CONSTRUCT_IBASE(0);
  SetPosition(csVector3(0,0,0));
  SetVelocity(csVector3(0,0,0));
  SetDirection(csVector3(0,0,1), csVector3(0,1,0));
  SetDistanceFactor(1.0);
  SetDopplerFactor(1.0);
  SetHeadSize(1.0);
  SetRollOffFactor(1.0);
  SetEnvironment(ENVIRONMENT_GENERIC);
}

csSoundListener::~csSoundListener()
{
  SCF_DESTRUCT_IBASE();
}

void csSoundListener::SetDirection (const csVector3 &f, const csVector3 &t)
{
  Front = f;
  Top = t;
}

void csSoundListener::SetPosition (const csVector3 &pos)
{
  Position = pos;
}

void csSoundListener::SetVelocity (const csVector3 &v)
{
  Velocity = v;
}

void csSoundListener::SetDistanceFactor (float factor)
{
  DistanceFactor = factor;
}

void csSoundListener::SetRollOffFactor (float factor)
{
  RollOffFactor = factor;
}

void csSoundListener::SetDopplerFactor (float factor)
{
  DopplerFactor = factor;
}

void csSoundListener::SetHeadSize (float size)
{
  HeadSize = size;
}

void csSoundListener::SetEnvironment (csSoundEnvironment env)
{
  Environment = env;
}

void csSoundListener::GetDirection (csVector3 &f, csVector3 &t)
{
  f = Front;
  t = Top;
}

const csVector3 &csSoundListener::GetPosition ()
{
  return Position;
}

const csVector3 &csSoundListener::GetVelocity ()
{
  return Velocity;
}

float csSoundListener::GetDistanceFactor ()
{
  return DistanceFactor;
}

float csSoundListener::GetRollOffFactor ()
{
  return RollOffFactor;
}

float csSoundListener::GetDopplerFactor ()
{
  return DopplerFactor;
}

float csSoundListener::GetHeadSize ()
{
  return HeadSize;
}

csSoundEnvironment csSoundListener::GetEnvironment ()
{
  return Environment;
}
