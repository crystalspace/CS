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
#include "csutil/scf.h"
#include "isys/isystem.h"

#include "dsound.h"

#include "sndrdr.h"
#include "sndlstn.h"

IMPLEMENT_IBASE(csSoundListenerDS3D)
	IMPLEMENTS_INTERFACE(iSoundListener)
IMPLEMENT_IBASE_END;

csSoundListenerDS3D::csSoundListenerDS3D(iBase *piBase) {
  CONSTRUCT_IBASE(piBase);
  PrimaryBuffer = NULL;
  Listener = NULL;
  Renderer = NULL;
}

csSoundListenerDS3D::~csSoundListenerDS3D() {
  if (Renderer) Renderer->DecRef();
  if (Listener) Listener->Release();
  if (PrimaryBuffer) {
    PrimaryBuffer->Stop();
    PrimaryBuffer->Release();
  }
}

bool csSoundListenerDS3D::Initialize(csSoundRenderDS3D *srdr) {
  srdr->IncRef();
  Renderer = srdr;
	
  DSBUFFERDESC dsbd;
  dsbd.dwSize = sizeof(DSBUFFERDESC);
  dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME;
  dsbd.dwReserved = 0;
  dsbd.dwBufferBytes = 0;
  dsbd.lpwfxFormat = NULL;

  HRESULT r;
  r = Renderer->AudioRenderer->CreateSoundBuffer(&dsbd, &PrimaryBuffer, NULL);
  if (r != DS_OK) {
    Renderer->System->Printf(MSG_INITIALIZATION, "DS3D listener: "
      "Cannot create primary sound buffer (%s).\n", Renderer->GetError(r));
    return false;
  }
	
  r = PrimaryBuffer->QueryInterface(IID_IDirectSound3DListener, (void **) &Listener);
  if (r != DS_OK) {
    Renderer->System->Printf(MSG_INITIALIZATION, "DS3D listener: Cannot query listener"
      " interface from primary sound buffer (%s).\n", Renderer->GetError(r));
    return false;
  }

  SetPosition(csVector3(0,0,0));
  SetVelocity(csVector3(0,0,0));
  SetDirection(csVector3(0,0,1), csVector3(0,1,0));
  SetDistanceFactor(1.0);
  SetDopplerFactor(1.0);
  SetDistanceFactor(1.0);
  SetRollOffFactor(1.0);
  SetEnvironment(ENVIRONMENT_GENERIC);
  Prepare();

  return true;
}

void csSoundListenerDS3D::SetPosition(csVector3 v) {
  Dirty = true;
  Position = v;
  Listener->SetPosition( v.x, v.y, v.z, DS3D_DEFERRED);
}

void csSoundListenerDS3D::SetDirection(csVector3 f, csVector3 t) {
  Dirty = true;
  Front = f;
  Top = t;
  Listener->SetOrientation(f.x, f.y, f.z,t.x, t.y, t.z,DS3D_DEFERRED);
}

void csSoundListenerDS3D::SetHeadSize(float size) {
//  Dirty = true;
  HeadSize = size;
// @@@
}

void csSoundListenerDS3D::SetVelocity(csVector3 v) {
  Dirty = true;
  Velocity = v;
  Listener->SetVelocity(v.x, v.y, v.z, DS3D_DEFERRED);
}

void csSoundListenerDS3D::SetDopplerFactor(float factor) {
  Dirty = true;
  Doppler = factor;
  Listener->SetDopplerFactor(Doppler, DS3D_DEFERRED);
}

void csSoundListenerDS3D::SetDistanceFactor(float factor) {
  Dirty = true;
  DistanceFactor = factor;
  Listener->SetDistanceFactor(factor, DS3D_DEFERRED);
}

void csSoundListenerDS3D::SetRollOffFactor(float factor) {
  Dirty = true;
  RollOff = factor;
  Listener->SetRolloffFactor(factor, DS3D_DEFERRED);
}

void csSoundListenerDS3D::SetEnvironment(SoundEnvironment env) {
//  Dirty = true;
  Environment = env;
// @@@
}

csVector3 csSoundListenerDS3D::GetPosition() {
  return Position;
}

void csSoundListenerDS3D::GetDirection(csVector3 &f, csVector3 &t) {
  f = Front;
  t = Top;
}

float csSoundListenerDS3D::GetHeadSize() {
  return HeadSize;
}

csVector3 csSoundListenerDS3D::GetVelocity() {
  return Velocity;
}

float csSoundListenerDS3D::GetDopplerFactor() {
  return Doppler;
}

float csSoundListenerDS3D::GetDistanceFactor() {
  return DistanceFactor;
}

float csSoundListenerDS3D::GetRollOffFactor() {
  return RollOff;
}

SoundEnvironment csSoundListenerDS3D::GetEnvironment() {
  return Environment;
}

void csSoundListenerDS3D::Prepare() {
  if (!Dirty) return;
  Listener->CommitDeferredSettings();
  Dirty = false;
}
