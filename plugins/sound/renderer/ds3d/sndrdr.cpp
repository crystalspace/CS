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
#include <stdio.h>
#include <initguid.h>
#include "dsound.h"

#include "csutil/scf.h"
#include "isystem.h"
#include "icfgfile.h"

#include "sndrdr.h"
#include "sndlstn.h"
#include "sndsrc.h"
#include "../common/convmeth.h"

IMPLEMENT_FACTORY(csSoundRenderDS3D);

EXPORT_CLASS_TABLE (sndrdrds3d)
EXPORT_CLASS (csSoundRenderDS3D, "crystalspace.sound.render.ds3d",
        "DirectSound 3D Sound Driver for Crystal Space")
EXPORT_CLASS_TABLE_END;

IMPLEMENT_IBASE(csSoundRenderDS3D)
  IMPLEMENTS_INTERFACE(iSoundRender)
  IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END;

csSoundRenderDS3D::csSoundRenderDS3D(iBase *piBase) {
  CONSTRUCT_IBASE(piBase);
  Listener = NULL;
  AudioRenderer = NULL;
  System = NULL;
  Config = NULL;
}

bool csSoundRenderDS3D::Initialize(iSystem *iSys) {
  System = iSys;
  LoadFormat.Bits = -1;
  LoadFormat.Freq = -1;
  LoadFormat.Channels = -1;
  Config = iSys->CreateConfig("/config/sound.cfg");
  return true;
}

csSoundRenderDS3D::~csSoundRenderDS3D() {
  Close();
  if (Config) Config->DecRef();
}

bool csSoundRenderDS3D::Open()
{
  System->Printf (MSG_INITIALIZATION, "SoundRender DirectSound3D selected\n");
  
  HRESULT r;
  if (!AudioRenderer) {
    r = DirectSoundCreate(NULL, &AudioRenderer, NULL);
    if (r != DS_OK) {
      System->Printf(MSG_FATAL_ERROR, "Error : Cannot Initialize "
        "DirectSound3D (%s).\n", GetError(r));
      Close();
      return false;
    }
  
    DWORD dwLevel = DSSCL_EXCLUSIVE;
    r = AudioRenderer->SetCooperativeLevel(GetForegroundWindow(), dwLevel);
    if (r != DS_OK) {
      System->Printf(MSG_FATAL_ERROR, "Error : Cannot Set "
        "Cooperative Level (%s).\n", GetError(r));
      Close();
      return false;
    }
  }

  if (!Listener) {
    Listener = new csSoundListenerDS3D(this);
    if (!Listener->Initialize(this)) return false;
  }

  float vol = Config->GetFloat("Sound","Volume",-1);
  if (vol>=0) SetVolume(vol);
  System->Printf (MSG_INITIALIZATION, "  Volume: %g\n", GetVolume());
  
  return true;
}

void csSoundRenderDS3D::Close()
{
  if (Listener) Listener->DecRef();
  Listener = NULL;

  if (AudioRenderer) AudioRenderer->Release();
  AudioRenderer = NULL;
}

void csSoundRenderDS3D::SetVolume(float vol)
{
  if (!Listener) return;
  long dsvol = DSBVOLUME_MIN + (DSBVOLUME_MAX-DSBVOLUME_MIN)*vol;
  int a = Listener->PrimaryBuffer->SetVolume(dsvol);
}

float csSoundRenderDS3D::GetVolume()
{
  if (!Listener) return 0;

  long dsvol;
  int a = Listener->PrimaryBuffer->GetVolume(&dsvol);
  return (float)(dsvol-DSBVOLUME_MIN)/(float)(DSBVOLUME_MAX-DSBVOLUME_MIN);
}

IMPLEMENT_SOUNDRENDER_CONVENIENCE_METHODS(csSoundRenderDS3D);

iSoundSource *csSoundRenderDS3D::CreateSource(iSoundStream *snd, int mode3d) {
  if (!snd) return NULL;
  csSoundSourceDS3D *src = new csSoundSourceDS3D(this);
  if (!src->Initialize(this, snd, mode3d)) {
    src->DecRef();
    return NULL;
  } else return src;
}

iSoundListener *csSoundRenderDS3D::GetListener() {
  return Listener;
}

void csSoundRenderDS3D::Update() {
  Listener->Prepare();

  for (int i=0;i<ActiveSources.Length();i++) {
    csSoundSourceDS3D *src = (csSoundSourceDS3D*)ActiveSources.Get(i);
    if (!src->IsPlaying()) {
      ActiveSources.Delete(i);
      i--;
    } else src->Update();
  }
}

const csSoundFormat *csSoundRenderDS3D::GetLoadFormat() {
  return &LoadFormat;
}

void csSoundRenderDS3D::MixingFunction() {
}

void csSoundRenderDS3D::SetDirty() {
  Listener->Dirty = true;
}

void csSoundRenderDS3D::AddSource(csSoundSourceDS3D *src) {
  if (ActiveSources.Find(src)==-1) {
    src->IncRef();
    ActiveSources.Push(src);
  }
}

void csSoundRenderDS3D::RemoveSource(csSoundSourceDS3D *src) {
  int n=ActiveSources.Find(src);
  if (n!=-1) {
    ActiveSources.Delete(n);
    src->DecRef();
  }
}

const char *csSoundRenderDS3D::GetError(HRESULT r) {
  switch (r) {
    case DS_OK: return "success";
    case DSERR_ALLOCATED: return "the resource is already allocated";
    case DSERR_ALREADYINITIALIZED: return "the object is already initialized";
    case DSERR_BADFORMAT: return "the specified wave format is not supported";
    case DSERR_BUFFERLOST: return "the buffer memory has been lost";
    case DSERR_CONTROLUNAVAIL: return "the requested control "
      "(volume, pan, ...) is not available";
    case DSERR_INVALIDCALL: return "this function is not valid for the current "
      "state of this object";
    case DSERR_INVALIDPARAM: return "an invalid parameter was passed to the "
      "returning function";
    case DSERR_NOAGGREGATION: return "the object does not support aggregation";
    case DSERR_NODRIVER: return "no sound driver is available for use";
    case DSERR_OTHERAPPHASPRIO: return "OTHERAPPHASPRIO";
    case DSERR_OUTOFMEMORY: return "out of memory";
    case DSERR_PRIOLEVELNEEDED: return "the application does not have the "
      "required priority level";
    case DSERR_UNINITIALIZED: return "IDirectSound not initialized";
    case DSERR_UNSUPPORTED: return "function is not supported at this time";

    case DSERR_GENERIC: default: return "unknown DirectSound error";
  }
}
