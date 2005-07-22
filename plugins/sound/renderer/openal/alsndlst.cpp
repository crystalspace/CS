/*
    Copyright (C) 2002 by Jorrit Tyberghein, Daniel Duhprey

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
#include "csutil/scf.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"

#include "alsndrdr.h"
#include "alsndlst.h"

SCF_IMPLEMENT_IBASE(csSoundListenerOpenAL)
	SCF_IMPLEMENTS_INTERFACE(iSoundListener)
SCF_IMPLEMENT_IBASE_END

csSoundListenerOpenAL::csSoundListenerOpenAL(csSoundRenderOpenAL *p) 
{
  SCF_CONSTRUCT_IBASE(p);
  SoundRender = p;
}

csSoundListenerOpenAL::~csSoundListenerOpenAL() 
{
  SCF_DESTRUCT_IBASE();
}

void csSoundListenerOpenAL::SetPosition(const csVector3 &v) {
  csSoundListener::SetPosition (v);
  position[0] = v.x; position[1] = v.y; position[2] = -v.z;
  SoundRender->mutex_OpenAL->LockWait();
  alListenerfv (AL_POSITION, position);
  SoundRender->mutex_OpenAL->Release();
}

void csSoundListenerOpenAL::SetDirection(const csVector3 &f, const csVector3 &t) {
  orientation[0] = f.x; orientation[1] = f.y; orientation[2] = -f.z; 
  orientation[3] = t.x; orientation[4] = t.y; orientation[5] = -t.z; 
  csSoundListener::SetDirection(f, t);
  SoundRender->mutex_OpenAL->LockWait();
  alListenerfv (AL_ORIENTATION, orientation);
  SoundRender->mutex_OpenAL->Release();
}

void csSoundListenerOpenAL::SetHeadSize(float size) {
  csSoundListener::SetHeadSize(size);
}

void csSoundListenerOpenAL::SetVelocity(const csVector3 &v) {
  csSoundListener::SetVelocity(v);
  velocity[0] = v.x; velocity[1] = v.y; velocity[2] = -v.z; 
  SoundRender->mutex_OpenAL->LockWait();
  alListenerfv (AL_VELOCITY, velocity);
  SoundRender->mutex_OpenAL->Release();
}

void csSoundListenerOpenAL::SetDopplerFactor(float factor) {
  csSoundListener::SetDopplerFactor(factor);
}

void csSoundListenerOpenAL::SetDistanceFactor(float factor) {
  csSoundListener::SetDistanceFactor(factor);
  SoundRender->SetDistanceFactor (factor);
}

void csSoundListenerOpenAL::SetRollOffFactor(float factor) {
  csSoundListener::SetRollOffFactor(factor);
  SoundRender->SetRollOffFactor (factor);
}

void csSoundListenerOpenAL::SetEnvironment(csSoundEnvironment env) {
  csSoundListener::SetEnvironment(env);
}

