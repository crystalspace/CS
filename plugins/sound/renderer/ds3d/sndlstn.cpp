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
#include "csplugincommon/directx/guids.h"
#include "csutil/scf.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"

#include <dsound.h>

#include "sndrdr.h"
#include "sndlstn.h"

SCF_IMPLEMENT_IBASE(csSoundListenerDS3D)
	SCF_IMPLEMENTS_INTERFACE(iSoundListener)
SCF_IMPLEMENT_IBASE_END;

csSoundListenerDS3D::csSoundListenerDS3D(iBase *piBase) 
{
  SCF_CONSTRUCT_IBASE(piBase);
  PrimaryBuffer = 0;
  Listener = 0;
  Renderer = 0;
}

csSoundListenerDS3D::~csSoundListenerDS3D() 
{
  if (Renderer) Renderer = 0;
  if (Listener) Listener->Release();
  if (PrimaryBuffer)
  {
    PrimaryBuffer->Stop();
    PrimaryBuffer->Release();
  }
  SCF_DESTRUCT_IBASE();
}

bool csSoundListenerDS3D::Initialize(csSoundRenderDS3D* srdr) 
{
  Renderer = srdr;

  DSBUFFERDESC dsbd;
  dsbd.dwSize = sizeof(DSBUFFERDESC);
  dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME;
  dsbd.dwReserved = 0;
  dsbd.dwBufferBytes = 0;
  dsbd.lpwfxFormat = 0;

  csRef<iReporter> reporter =
    CS_QUERY_REGISTRY (Renderer->object_reg, iReporter);

  HRESULT r;
  r = Renderer->AudioRenderer->CreateSoundBuffer(&dsbd, &PrimaryBuffer, 0);
  if (r != DS_OK)
  {
    if (reporter)
      reporter->Report (CS_REPORTER_SEVERITY_WARNING,
      "crystalspace.sound.ds3d", "DS3D listener: "
      "Cannot create primary sound buffer (%s).", Renderer->GetError(r));
    return false;
  }

  r = PrimaryBuffer->QueryInterface(IID_IDirectSound3DListener,
    (void **) &Listener);
  if (r != DS_OK)
  {
    if (reporter)
      reporter->Report (CS_REPORTER_SEVERITY_WARNING,
      "crystalspace.sound.ds3d",
      "DS3D listener: Cannot query listener"
      " interface from primary sound buffer (%s).", Renderer->GetError(r));
    return false;
  }

  Dirty = true;
  Prepare();
  return true;
}

void csSoundListenerDS3D::SetPosition(const csVector3 &v) 
{
  Dirty = true;
  csSoundListener::SetPosition(v);
  Listener->SetPosition( v.x, v.y, v.z, DS3D_DEFERRED);
}

void csSoundListenerDS3D::SetDirection(const csVector3 &f, const csVector3 &t) 
{
  Dirty = true;
  csSoundListener::SetDirection(f, t);
  Listener->SetOrientation(f.x, f.y, f.z,t.x, t.y, t.z,DS3D_DEFERRED);
}

void csSoundListenerDS3D::SetHeadSize(float size) 
{
  //  Dirty = true;
  csSoundListener::SetHeadSize(size);
  // @@@
}

void csSoundListenerDS3D::SetVelocity(const csVector3 &v) 
{
  Dirty = true;
  csSoundListener::SetVelocity(v);
  Listener->SetVelocity(v.x, v.y, v.z, DS3D_DEFERRED);
}

void csSoundListenerDS3D::SetDopplerFactor(float factor) 
{
  Dirty = true;
  csSoundListener::SetDopplerFactor(factor);
  Listener->SetDopplerFactor(factor, DS3D_DEFERRED);
}

void csSoundListenerDS3D::SetDistanceFactor(float factor) 
{
  Dirty = true;
  csSoundListener::SetDistanceFactor(factor);
  Listener->SetDistanceFactor(factor, DS3D_DEFERRED);
}

void csSoundListenerDS3D::SetRollOffFactor(float factor) 
{
  Dirty = true;
  csSoundListener::SetRollOffFactor(factor);
  Listener->SetRolloffFactor(factor, DS3D_DEFERRED);
}

void csSoundListenerDS3D::SetEnvironment(csSoundEnvironment env) 
{
  //  Dirty = true;
  csSoundListener::SetEnvironment(env);
  // @@@
}

void csSoundListenerDS3D::Prepare() 
{
  if (!Dirty) return;
  Listener->CommitDeferredSettings();
  Dirty = false;
}
