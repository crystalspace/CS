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
#include "isys/system.h"
#include "iutil/cfgfile.h"
#include "isys/event.h"

#include "sndrdr.h"
#include "sndlstn.h"
#include "sndsrc.h"
#include "../common/convmeth.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csSoundRenderEAX);

SCF_EXPORT_CLASS_TABLE (sndrdrds3d)
SCF_EXPORT_CLASS (csSoundRenderEAX, "crystalspace.sound.render.ds3d",
        "DirectSound 3D Sound Driver for Crystal Space")
SCF_EXPORT_CLASS_TABLE_END;

SCF_IMPLEMENT_IBASE(csSoundRenderEAX)
  SCF_IMPLEMENTS_INTERFACE(iSoundRender)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iPlugIn)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundRenderEAX::eiPlugIn)
  SCF_IMPLEMENTS_INTERFACE (iPlugIn)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSoundRenderEAX::csSoundRenderEAX(iBase *piBase)
{
  SCF_CONSTRUCT_IBASE(piBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugIn);
  Listener = NULL;
  AudioRenderer = NULL;
  System = NULL;
}

bool csSoundRenderEAX::Initialize(iSystem *iSys)
{
  (System = iSys)->IncRef();
  System->CallOnEvents(&scfiPlugIn,
    CSMASK_Command | CSMASK_Broadcast | CSMASK_Nothing);
  LoadFormat.Bits = -1;
  LoadFormat.Freq = -1;
  LoadFormat.Channels = -1;
  Config.AddConfig(iSys, "/config/sound.cfg");
  return true;
}

csSoundRenderEAX::~csSoundRenderEAX() {
  Close();
}

bool csSoundRenderEAX::Open()
{
  System->Printf(CS_MSG_INITIALIZATION,"SoundRender DirectSound3D selected\n");
  
  HRESULT r;
  if (!AudioRenderer)
  {
    r = DirectSoundCreate(NULL, &AudioRenderer, NULL);
    if (r != DS_OK)
    {
      System->Printf(CS_MSG_FATAL_ERROR, "Error : Cannot Initialize "
        "DirectSound3D (%s).\n", GetError(r));
      Close();
      return false;
    }
  
    DWORD dwLevel = DSSCL_EXCLUSIVE;
    r = AudioRenderer->SetCooperativeLevel(GetForegroundWindow(), dwLevel);
    if (r != DS_OK)
    {
      System->Printf(CS_MSG_FATAL_ERROR, "Error : Cannot Set "
        "Cooperative Level (%s).\n", GetError(r));
      Close();
      return false;
    }
  }

  if (!Listener)
  {
    Listener = new csSoundListenerEAX(this);
    if (!Listener->Initialize(this)) return false;
  }

  float vol = Config->GetFloat("Sound.Volume",-1);
  if (vol>=0) SetVolume(vol);
  System->Printf (CS_MSG_INITIALIZATION, "  Volume: %g\n", GetVolume());
  
  return true;
}

void csSoundRenderEAX::Close()
{
  if (Listener) Listener->DecRef();
  Listener = NULL;

  if (AudioRenderer) AudioRenderer->Release();
  AudioRenderer = NULL;
}

void csSoundRenderEAX::SetVolume(float vol)
{
  if (!Listener) return;
  long dsvol = DSBVOLUME_MIN + (DSBVOLUME_MAX-DSBVOLUME_MIN)*vol;
  int a = Listener->PrimaryBuffer->SetVolume(dsvol);
}

float csSoundRenderEAX::GetVolume()
{
  if (!Listener) return 0;
  long dsvol;
  int a = Listener->PrimaryBuffer->GetVolume(&dsvol);
  return (float)(dsvol-DSBVOLUME_MIN)/(float)(DSBVOLUME_MAX-DSBVOLUME_MIN);
}

IMPLEMENT_SOUNDRENDER_CONVENIENCE_METHODS(csSoundRenderEAX);

iSoundSource *csSoundRenderEAX::CreateSource(iSoundStream *snd, int mode3d)
{
  if (!snd) return NULL;
  csSoundSourceEAX *src = new csSoundSourceEAX(this);
  if (!src->Initialize(this, snd, mode3d))
  {
    src->DecRef();
    return NULL;
  } else return src;
}

iSoundListener *csSoundRenderEAX::GetListener()
{
  return Listener;
}

void csSoundRenderEAX::Update()
{
  Listener->Prepare();

  for (int i=0;i<ActiveSources.Length();i++)
  {
    csSoundSourceEAX *src = (csSoundSourceEAX*)ActiveSources.Get(i);
    if (!src->IsPlaying())
    {
      ActiveSources.Delete(i);
      i--;
    } else src->Update();
  }
}

const csSoundFormat *csSoundRenderEAX::GetLoadFormat()
{
  return &LoadFormat;
}

void csSoundRenderEAX::MixingFunction()
{
}

void csSoundRenderEAX::SetDirty()
{
  Listener->Dirty = true;
}

void csSoundRenderEAX::AddSource(csSoundSourceEAX *src)
{
  if (ActiveSources.Find(src)==-1)
  {
    src->IncRef();
    ActiveSources.Push(src);
  }
}

void csSoundRenderEAX::RemoveSource(csSoundSourceEAX *src)
{
  int n=ActiveSources.Find(src);
  if (n!=-1)
  {
    ActiveSources.Delete(n);
    src->DecRef();
  }
}

const char *csSoundRenderEAX::GetError(HRESULT r)
{
  switch (r)
  {
    case DS_OK: return "success";
    case DSERR_ALLOCATED: return "the resource is already allocated";
    case DSERR_ALREADYINITIALIZED: return "the object is already initialized";
    case DSERR_BADFORMAT: return "the specified wave format is not supported";
    case DSERR_BUFFERLOST: return "the buffer memory has been lost";
    case DSERR_CONTROLUNAVAIL: return "the requested control "
      "(volume, pan, ...) is not available";
    case DSERR_INVALIDCALL:
      return "this function is not valid for the current state of this object";
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

bool csSoundRenderEAX::HandleEvent (iEvent &e)
{
  if (e.Type == csevCommand || e.Type == csevBroadcast)
  {
    switch (e.Command.Code)
    {
    case cscmdPreProcess:
      Update();
      break;
    case cscmdSystemOpen:
      Open();
      break;
    case cscmdSystemClose:
      Close();
      break;
    }
  }
  return false;
}
