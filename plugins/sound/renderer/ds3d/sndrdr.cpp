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
#include "csutil/sysfunc.h"
#include <stdio.h>

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


SCF_IMPLEMENT_IBASE(csSoundRenderDS3D)
  SCF_IMPLEMENTS_INTERFACE(iSoundRender)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundRenderDS3D::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundRenderDS3D::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

//  Pre-Multi Threaded section
csSoundRenderDS3D::csSoundRenderDS3D(iBase *piBase)
{
  SCF_CONSTRUCT_IBASE(piBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);
  Listener = 0;
  AudioRenderer = 0;
  object_reg = 0;
  mutex_Listener = csMutex::Create (true);
  mutex_ActiveSources = csMutex::Create (true);
  mutex_SoundHandles = csMutex::Create (true);
  bRunning=false;
}

//  Pre-Multi Threaded section
bool csSoundRenderDS3D::Initialize(iObjectRegistry *r)
{
  object_reg = r;
  csRef<iEventQueue> q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
    q->RegisterListener (&scfiEventHandler,
    CSMASK_Command | CSMASK_Broadcast | CSMASK_Nothing);
  
  win32Assistant = CS_QUERY_REGISTRY (object_reg, iWin32Assistant);
  if (!win32Assistant)
  {
    MessageBox (0, "No iWin32Assistant!", "DS3D error", MB_OK | MB_ICONSTOP);
    return false;
  }

  LoadFormat.Bits = -1;
  LoadFormat.Freq = -1;
  LoadFormat.Channels = -1;
  
  Config.AddConfig(object_reg, "/config/sound.cfg");
  return true;
}

csSoundRenderDS3D::~csSoundRenderDS3D()
{
  csRef<iEventQueue> q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
    q->RemoveListener (&scfiEventHandler);
  Close();
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiEventHandler);
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

//  Pre-Multi Threaded section
bool csSoundRenderDS3D::Open()
{
  csRef<iReporter> reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (reporter)
    reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
    "crystalspace.sound.ds3d",
    "SoundRender DirectSound3D selected");

  HRESULT r;
  if (!AudioRenderer)
  {
    r = DirectSoundCreate(0, &AudioRenderer, 0);
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
      return false;
    }

    DWORD dwLevel = DSSCL_PRIORITY;
    r = AudioRenderer->SetCooperativeLevel(
      win32Assistant->GetApplicationWindow(), dwLevel);
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
      return false;
    }
  }

  if (!Listener)
  {
    Listener = csPtr<csSoundListenerDS3D>(new csSoundListenerDS3D(this));
    if (!Listener->Initialize(this))
    {
      return false;
    }
  }

  float vol = Config->GetFloat("Sound.Volume",-1);
  if (vol>=0) SetVolume(vol);
  if (reporter)
    reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
    "crystalspace.sound.ds3d",
    "  Volume: %g\n", GetVolume());

  BufferLengthSeconds = Config->GetFloat("Sound.ds3d.StreamingBufferLength", 0.2f);
  BackgroundProcessing = Config->GetBool("Sound.ds3d.BackgroundProcessing", true);
  MuteInBackground = Config->GetBool("Sound.ds3d.MuteInBackground", true);
  LazySourceSync = Config->GetBool("Sound.ds3d.LazySourceSync",true);

  // Set the first time
  LastTime = csGetTicks ();

  // Start the background thread - Make sure this is done AFTER all initialization!
  if (BackgroundProcessing)
  {
    bRunning=true;
    bgThread=csThread::Create (new DS3DRunnable (this));
    bgThread->Start();
  }

  return true;
}

void csSoundRenderDS3D::Close()
{
  if (bRunning)
  {
    bRunning=false;
    bgThread->Wait();
  }
  bgThread = 0;

  mutex_ActiveSources->LockWait();
  while (ActiveSources.Length()>0)
    ((iSoundSource*)ActiveSources.Get(0))->Stop();
  mutex_ActiveSources->Release();


  mutex_SoundHandles->LockWait();
  while (SoundHandles.Length()>0)
  {
    csRef<csSoundHandleDS3D> hdl = SoundHandles.Pop();
    hdl->Unregister();
  }
  mutex_SoundHandles->Release();

  mutex_Listener->LockWait();
  Listener = 0;
  mutex_Listener->Release();

  if (AudioRenderer) AudioRenderer->Release();
  AudioRenderer = 0;
}

void csSoundRenderDS3D::SetVolume(float vol)
{
  if (!Listener) return;
  long dsvol = (long)(DSBVOLUME_MIN + (DSBVOLUME_MAX-DSBVOLUME_MIN)*vol);
  mutex_Listener->LockWait();
  Listener->PrimaryBuffer->SetVolume(dsvol);
  mutex_Listener->Release();
}

float csSoundRenderDS3D::GetVolume()
{
  if (!Listener) return 0;
  long dsvol;
  mutex_Listener->LockWait();
  Listener->PrimaryBuffer->GetVolume(&dsvol);
  mutex_Listener->Release();
  return (float)(dsvol-DSBVOLUME_MIN)/(float)(DSBVOLUME_MAX-DSBVOLUME_MIN);
}

csPtr<iSoundHandle> csSoundRenderDS3D::RegisterSound(iSoundData *snd)
{
  if (!snd) return 0;
  if (!snd->Initialize(&LoadFormat)) return 0;
  csSoundHandleDS3D *hdl = new csSoundHandleDS3D(this, snd,BufferLengthSeconds,!LazySourceSync);
  mutex_SoundHandles->LockWait();
  SoundHandles.Push(hdl);
  mutex_SoundHandles->Release();

  return csPtr<iSoundHandle> (hdl);
}

void csSoundRenderDS3D::UnregisterSound(iSoundHandle *snd)
{
  mutex_SoundHandles->LockWait();
  csRef<csSoundHandleDS3D> hdl = (csSoundHandleDS3D *)snd;
  int n = SoundHandles.Find(hdl);
  if (n != -1)
  {
    SoundHandles.DeleteIndex (n);
    hdl->Unregister();
  }
  mutex_SoundHandles->Release();
}

iSoundListener *csSoundRenderDS3D::GetListener()
{
  return Listener;
}

void csSoundRenderDS3D::Update()
{
  size_t i;
  csTicks ct, et;
  ct = csGetTicks ();

  et=ct-LastTime;
  LastTime = ct;


  mutex_Listener->LockWait();
  Listener->Prepare();
  mutex_Listener->Release();

  mutex_SoundHandles->LockWait();
  mutex_ActiveSources->LockWait();
  for (i=0;i<SoundHandles.Length();i++)
  {
    csSoundHandleDS3D *hdl = (csSoundHandleDS3D*)SoundHandles.Get(i);
    // Elapsed Time is ignored unless the handle itself is keeping track of playback time
    hdl->Update_Time(et);
  }

  for (i=0;i<ActiveSources.Length();i++)
  {
    csSoundSourceDS3D *src = (csSoundSourceDS3D*)ActiveSources.Get(i);
    if (!src->IsPlaying())
    {
      //  Once a source has stopped playing it can be removed.
      ActiveSources.DeleteIndex (i);
      i--;
    }
  }
  mutex_ActiveSources->Release();
  mutex_SoundHandles->Release();
}

void csSoundRenderDS3D::MixingFunction()
{
  // DirectSound takes care of mixing.
}

void csSoundRenderDS3D::SetDirty()
{
  mutex_Listener->LockWait();
  Listener->Dirty = true;
  mutex_Listener->Release();
}

void csSoundRenderDS3D::AddSource(csSoundSourceDS3D *src)
{
  mutex_ActiveSources->LockWait();
  if (ActiveSources.Find (src) == csArrayItemNotFound)
  {
    ActiveSources.Push(src);
  }
  mutex_ActiveSources->Release();
}

void csSoundRenderDS3D::RemoveSource(csSoundSourceDS3D *src)
{
  mutex_ActiveSources->LockWait();
  int n=ActiveSources.Find(src);
  if (n!=-1)
  {
    ActiveSources.DeleteIndex (n);
  }
  mutex_ActiveSources->Release();
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

void csSoundRenderDS3D::ThreadProc ()
{
  /* This should let us sleep a good amount, but not so much that the buffer can empty out
  *  even if the system is under high load and things are running slow.
  */
  int sleeptime = (int)(BufferLengthSeconds*300);

  while (bRunning)
  {
    Update();
    csSleep(sleeptime);
  }
}

bool csSoundRenderDS3D::HandleEvent (iEvent &e)
{
  if (e.Type == csevCommand || e.Type == csevBroadcast)
  {
    switch (e.Command.Code)
    {
    case cscmdPreProcess:
      if (!BackgroundProcessing)
        Update(); // Only perform updates here if there is no background thread
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
