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
}

bool csSoundRenderDS3D::Initialize(iSystem *iSys) {
  System = iSys;
  LoadFormat.Bits = -1;
  LoadFormat.Freq = -1;
  LoadFormat.Channels = -1;
  return true;
}

csSoundRenderDS3D::~csSoundRenderDS3D() {
  Close();
}

bool csSoundRenderDS3D::Open()
{
  System->Printf (MSG_INITIALIZATION, "SoundRender DirectSound3D selected\n");
  
  if (!AudioRenderer) {
    if (FAILED(DirectSoundCreate(NULL, &AudioRenderer, NULL))) {
      System->Printf(MSG_FATAL_ERROR, "Error : Cannot Initialize DirectSound3D !");
      Close();
      return false;
    }
  
    DWORD dwLevel = DSSCL_NORMAL;
    if (FAILED(AudioRenderer->SetCooperativeLevel(GetForegroundWindow(), dwLevel)))
    {
      System->Printf(MSG_FATAL_ERROR, "Error : Cannot Set Cooperative Level!");
      Close();
      return false;
    }
  }

  if (!Listener) {
    Listener = new csSoundListenerDS3D(this);
    if (!Listener->Initialize(this)) return false;
  }
  
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
  Listener->PrimaryBuffer->SetVolume(dsvol);
}

float csSoundRenderDS3D::GetVolume()
{
  if (!Listener) return 0;

  long dsvol;
  Listener->PrimaryBuffer->GetVolume(&dsvol);
  return (float)(dsvol-DSBVOLUME_MIN)/(float)(DSBVOLUME_MAX-DSBVOLUME_MIN);
}

IMPLEMENT_SOUNDRENDER_CONVENIENCE_METHODS(csSoundRenderDS3D);

iSoundSource *csSoundRenderDS3D::CreateSource(iSoundStream *snd, bool is3d) {
  if (!snd) return NULL;
  csSoundSourceDS3D *src = new csSoundSourceDS3D(this);
  if (!src->Initialize(this, snd, is3d)) {
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
