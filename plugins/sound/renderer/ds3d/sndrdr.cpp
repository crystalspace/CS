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
#include "cssys/sysfunc.h"
#include <stdio.h>
#include <initguid.h>

#include "csutil/scf.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"

#include "sndrdr.h"
#include "sndlstn.h"
#include "sndsrc.h"
#include "sndhdl.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csSoundRenderDS3D);

SCF_EXPORT_CLASS_TABLE (sndrdrds3d)
SCF_EXPORT_CLASS (csSoundRenderDS3D, "crystalspace.sound.render.ds3d",
        "DirectSound 3D Sound Driver for Crystal Space")
SCF_EXPORT_CLASS_TABLE_END;

SCF_IMPLEMENT_IBASE(csSoundRenderDS3D)
  SCF_IMPLEMENTS_INTERFACE(iSoundRender)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundRenderDS3D::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundRenderDS3D::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSoundRenderDS3D::csSoundRenderDS3D(iBase *piBase)
{
  SCF_CONSTRUCT_IBASE(piBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);
  Listener = NULL;
  AudioRenderer = NULL;
  object_reg = NULL;
}

bool csSoundRenderDS3D::Initialize(iObjectRegistry *r)
{
  object_reg = r;
  iEventQueue* q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
  {
    q->RegisterListener(&scfiEventHandler,
      CSMASK_Command | CSMASK_Broadcast | CSMASK_Nothing);
    q->DecRef ();
  }
  LoadFormat.Bits = -1;
  LoadFormat.Freq = -1;
  LoadFormat.Channels = -1;
  Config.AddConfig(object_reg, "/config/sound.cfg");
  return true;
}

csSoundRenderDS3D::~csSoundRenderDS3D()
{
  Close();
}

bool csSoundRenderDS3D::Open()
{
  iReporter* reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (reporter)
    reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
  	"crystalspace.sound.ds3d",
	"SoundRender DirectSound3D selected");
  
  HRESULT r;
  if (!AudioRenderer)
  {
    r = DirectSoundCreate(NULL, &AudioRenderer, NULL);
    if (r != DS_OK)
    {
      if (reporter)
        reporter->Report (CS_REPORTER_SEVERITY_ERROR,
		"crystalspace.sound.ds3d",
		"Error : Cannot Initialize DirectSound3D (%s).", GetError(r));
      else
        csPrintf (
		"Error : Cannot Initialize DirectSound3D (%s).\n", GetError(r));
      Close();
      if (reporter) reporter->DecRef ();
      return false;
    }
  
    DWORD dwLevel = DSSCL_EXCLUSIVE;
    r = AudioRenderer->SetCooperativeLevel(GetForegroundWindow(), dwLevel);
    if (r != DS_OK)
    {
      if (reporter)
        reporter->Report (CS_REPORTER_SEVERITY_ERROR,
		"crystalspace.sound.ds3d",
		"Error : Cannot Set Cooperative Level (%s).", GetError(r));
      else
	csPrintf (
		"Error : Cannot Set Cooperative Level (%s).\n", GetError(r));
      Close();
      if (reporter) reporter->DecRef ();
      return false;
    }
  }

  if (!Listener)
  {
    Listener = new csSoundListenerDS3D(this);
    if (!Listener->Initialize(this))
    {
      if (reporter) reporter->DecRef ();
      return false;
    }
  }

  float vol = Config->GetFloat("Sound.Volume",-1);
  if (vol>=0) SetVolume(vol);
  if (reporter)
    reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
	"crystalspace.sound.ds3d",
    	"  Volume: %g\n", GetVolume());

  csTicks et, ct;
  iVirtualClock* vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  et = vc->GetElapsedTicks ();
  ct = vc->GetCurrentTicks ();
  vc->DecRef ();
  LastTime = ct;
  if (reporter) reporter->DecRef ();
  
  return true;
}

void csSoundRenderDS3D::Close()
{
  while (ActiveSources.Length()>0)
    ((iSoundSource*)ActiveSources.Get(0))->Stop();

  while (SoundHandles.Length()>0)
  {
    csSoundHandleDS3D *hdl = (csSoundHandleDS3D *)SoundHandles.Pop();
    hdl->Unregister();
    hdl->DecRef();
  }

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

iSoundHandle *csSoundRenderDS3D::RegisterSound(iSoundData *snd)
{
  if (!snd) return NULL;
  if (!snd->Initialize(&LoadFormat)) return NULL;
  csSoundHandleDS3D *hdl = new csSoundHandleDS3D(this, snd);
  SoundHandles.Push(hdl);
  return hdl;
}

void csSoundRenderDS3D::UnregisterSound(iSoundHandle *snd)
{
  int n = SoundHandles.Find(snd);
  if (n != -1)
  {
    csSoundHandleDS3D *hdl = (csSoundHandleDS3D *)snd;
    SoundHandles.Delete(n);
    hdl->Unregister();
    hdl->DecRef();
  }
}

iSoundListener *csSoundRenderDS3D::GetListener()
{
  return Listener;
}

void csSoundRenderDS3D::Update()
{
  int i;
  csTicks et, ct;
  iVirtualClock* vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  et = vc->GetElapsedTicks ();
  ct = vc->GetCurrentTicks ();
  vc->DecRef ();
  csTicks ETime = ct - LastTime;
  LastTime = ct;

  Listener->Prepare();

  for (i=0;i<SoundHandles.Length();i++)
  {
    csSoundHandleDS3D *hdl = (csSoundHandleDS3D*)SoundHandles.Get(i);
    hdl->Update_Time(ETime);
  }

  for (i=0;i<ActiveSources.Length();i++)
  {
    csSoundSourceDS3D *src = (csSoundSourceDS3D*)ActiveSources.Get(i);
    if (!src->IsPlaying())
    {
      ActiveSources.Delete(i);
      i--;
    }
  }
}

void csSoundRenderDS3D::MixingFunction()
{
}

void csSoundRenderDS3D::SetDirty()
{
  Listener->Dirty = true;
}

void csSoundRenderDS3D::AddSource(csSoundSourceDS3D *src)
{
  if (ActiveSources.Find(src)==-1)
  {
    src->IncRef();
    ActiveSources.Push(src);
  }
}

void csSoundRenderDS3D::RemoveSource(csSoundSourceDS3D *src)
{
  int n=ActiveSources.Find(src);
  if (n!=-1)
  {
    ActiveSources.Delete(n);
    src->DecRef();
  }
}

const char *csSoundRenderDS3D::GetError(HRESULT r)
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

bool csSoundRenderDS3D::HandleEvent (iEvent &e)
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
