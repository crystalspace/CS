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
#include <stdarg.h>
#include <stdio.h>

#include "csutil/sysfunc.h"
#include "iutil/plugin.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "isound/driver.h"
#include "isound/data.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "iutil/cmdline.h"
#include "ivaria/reporter.h"

#include "srdrcom.h"
#include "csplugincommon/soundrenderer/slstn.h"
#include "srdrsrc.h"
#include "sndhdl.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csSoundRenderSoftware)


SCF_IMPLEMENT_IBASE(csSoundRenderSoftware)
	SCF_IMPLEMENTS_INTERFACE(iSoundRender)
	SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundRenderSoftware::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSoundRenderSoftware::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

csSoundRenderSoftware::csSoundRenderSoftware(iBase* piBase) : Listener(0)
{
  SCF_CONSTRUCT_IBASE(piBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  scfiEventHandler = 0;
  object_reg = 0;
  Listener = 0;
  memory = 0;
  memorysize = 0;
  ActivateMixing = false;
  owning = downing = false;
  data = csCondition::Create ();
  mixing = csMutex::Create (true);
}

void csSoundRenderSoftware::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.sound.software", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool csSoundRenderSoftware::Initialize (iObjectRegistry *r)
{
  // copy the system pointer
  object_reg = r;

  // read the config file
  Config.AddConfig(object_reg, "/config/sound.cfg");

  // check for optional sound driver fro the commandline
  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (r, iCommandLineParser));
  const char *drv = cmdline->GetOption ("sounddriver");
  if (!drv)
  {
    // load the sound driver plug-in
#ifdef CS_SOUND_DRIVER
    drv = CS_SOUND_DRIVER;   // "crystalspace.sound.driver.xxx"
#else
    drv = "crystalspace.sound.driver.null";
#endif
    drv = Config->GetStr ("Sound.Driver", drv);
  }

  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  SoundDriver = CS_LOAD_PLUGIN (plugin_mgr, drv, iSoundDriver);
  if (!SoundDriver)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "csSoundRenderSoftware: Failed to load sound driver: %s", drv);
    return false;
  }

  // set event callback
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener(scfiEventHandler,
      CSMASK_Command | CSMASK_Broadcast | CSMASK_Nothing);

  return true;
}

csSoundRenderSoftware::~csSoundRenderSoftware()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
    if (q != 0)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }
  Close();
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csSoundRenderSoftware::Open()
{
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Software Sound Renderer selected");
  CS_ASSERT (Config != 0);

  if (!SoundDriver) return false;

  /* Grab the mixing mutex while we start the sound driver.
   *  The windows driver calls back to the mixer to mix audio.  The mixer is 
   *  not ready to be called yet.
   */
  mixing->LockWait ();

  SoundDriver->Open (this,
    Config->GetInt("Sound.Software.Frequency", 22050),
    Config->GetBool("Sound.Software.16Bits", true),
    Config->GetBool("Sound.Software.Stereo", true));

  Volume = Config->GetFloat("Sound.Volume", 1.0);
  if (Volume>1) Volume = 1;
  if (Volume<0) Volume = 0;

  Listener = new csSoundListener ();
  ActivateMixing = true;
  LoadFormat.Freq = getFrequency();
  LoadFormat.Bits = is16Bits() ? 16 : 8;
  LoadFormat.Channels = -1;

  Report (CS_REPORTER_SEVERITY_NOTIFY, "  Playing %d Hz, %d bits, %s",
    getFrequency(), (is16Bits())?16:8, (isStereo())?"Stereo":"Mono");
  Report (CS_REPORTER_SEVERITY_NOTIFY, "  Volume: %g", Volume);

  csTicks et, ct;
  csRef<iVirtualClock> vc (CS_QUERY_REGISTRY (object_reg, iVirtualClock));

  et = vc->GetElapsedTicks ();
  ct = vc->GetCurrentTicks ();
  LastTime = ct;

  // The mixer is now ready to go
  mixing->Release ();

  if (SoundDriver->ThreadAware ())
  {
    mixing->LockWait ();
    bRunning = true;
    mixer = csThread::Create (new MixerRunnable (this));
    mixer->Start ();
    mixing->Release ();
  }
  return true;
}

void csSoundRenderSoftware::Close()
{
  bRunning = false;
  data->Signal (true);
  mixing->LockWait ();
  owning = true;
  downing = true;
  ActivateMixing = false;

  // Must release the lock while we close the driver
  mixing->Release ();

  if (SoundDriver)
  {
    SoundDriver->Close ();
    SoundDriver = 0;
  }

  mixing->LockWait ();
  if (Listener)
  {
    Listener->DecRef();
    Listener = 0;
  }

  while (Sources.Length()>0)
    Sources.Get(0)->Stop();

  while (SoundHandles.Length()>0)
  {
    csSoundHandleSoftware *hdl = SoundHandles.Pop();
    hdl->Unregister();
    hdl->DecRef();
  }
  owning = false;
  downing = false;
  mixing->Release ();
}

csPtr<iSoundHandle> csSoundRenderSoftware::RegisterSound(iSoundData *snd)
{
  // convert the sound
  if (!snd->Initialize(&LoadFormat)) return 0;

  // create the sound handle
  csSoundHandleSoftware *hdl = new csSoundHandleSoftware(this, snd);
  SoundHandles.Push(hdl);
  hdl->IncRef ();	// Prevent smart pointer release.
  return csPtr<iSoundHandle> (hdl);
}

void csSoundRenderSoftware::UnregisterSound(iSoundHandle *snd)
{
  size_t n = SoundHandles.Find((csSoundHandleSoftware*)snd);
  if (n != csArrayItemNotFound)
  {
    if (owning || mixing->LockWait ()) // dont remove while we mix
    {
      csSoundHandleSoftware *hdl = (csSoundHandleSoftware *)snd;
      SoundHandles.DeleteIndex (n);
      hdl->Unregister();
      hdl->DecRef();
      
      if (!owning) mixing->Release ();
    }
  }
}

iSoundListener *csSoundRenderSoftware::GetListener()
{
  return Listener;
}

bool csSoundRenderSoftware::is16Bits()
{
  return SoundDriver->Is16Bits();
}

bool csSoundRenderSoftware::isStereo()
{
  return SoundDriver->IsStereo();
}

int csSoundRenderSoftware::getFrequency()
{
  return SoundDriver->GetFrequency();
}

void csSoundRenderSoftware::SetVolume(float vol)
{
  Volume = vol;
}

float csSoundRenderSoftware::GetVolume()
{
  return Volume;
}

void csSoundRenderSoftware::AddSource(csSoundSourceSoftware *src)
{
  csScopedMutexLock lock(mixing);
  Sources.Push(src);
  src->IncRef();
  data->Signal (true);
}

void csSoundRenderSoftware::RemoveSource(csSoundSourceSoftware *src)
{
  if (downing || mixing->LockWait ()) // dont remove while we mix
  {
    if (!downing) owning = true;
    size_t n = Sources.Find(src);
    if (n != csArrayItemNotFound)
    {
      Sources.DeleteIndex (n);
      src->DecRef();
    }
    if (!downing) 
    {
      owning = false;
      mixing->Release ();
    }
  }
}

void csSoundRenderSoftware::MixingFunction()
{
  size_t i;

  // look if this function is activated
  if (!ActivateMixing) return;

  // test if we have a sound driver
  if(!SoundDriver) return;

  // Get a mixing lock.  Mutex is recursive.
  mixing->LockWait ();

  // if no sources exist, there may be an optimized way to handle this
  if (Sources.Length()==0 && SoundDriver->IsHandleVoidSound())
  {
    mixing->Release();
    return;
  }

  // lock sound memory
  SoundDriver->LockMemory(&memory, &memorysize);
  if(!memory || memorysize<1) 
  {
    mixing->Release();
    return;
  }

  // clear the buffer
  if (is16Bits()) memset(memory,0,memorysize);
  else memset(memory,128,memorysize);

  // prepare and play all sources
  for (i=0;i<Sources.Length();i++)
  {
    csSoundSourceSoftware *src=Sources.Get(i);
    src->Prepare(Volume);
    src->AddToBufferStatic(memory, memorysize);
    if (!src->IsActive())
    {
      Sources.DeleteIndex (i);
      src->DecRef ();
      i--;
    }
  }

  /* Do not update the sound handles. Right now the source advances
   *  the sound data stream.  The default handle UpdateCount() function
   *  will also advance the stream causing the sound to skip and play
   *  twice as fast if the stream is started.
   */
/*
  // update sound handles
   long NumSamples = memorysize / (is16Bits()?2:1) / (isStereo()?2:1);
   for (i=0;i<SoundHandles.Length();i++) {
    csSoundHandleSoftware *hdl = SoundHandles.Get(i);
    hdl->UpdateCount(NumSamples);
  }
*/
  mixing->Release();

  // UnlockMemory blocks in the OSS driver for linux.  It should not be called with a lock held.
  SoundDriver->UnlockMemory();
}

bool csSoundRenderSoftware::HandleEvent (iEvent &e)
{
  if (e.Type == csevCommand || e.Type == csevBroadcast) {
    switch (e.Command.Code) {
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

void csSoundRenderSoftware::Update()
{
  // update sound if the sound driver doesn't do it
  if(!SoundDriver->IsBackground()) MixingFunction();
}

void csSoundRenderSoftware::ThreadedMix ()
{
  while (bRunning)
  {
    mixing->LockWait ();
    data->Wait (mixing);
    bool bLocked = true;
    while (bRunning && Sources.Length() != 0)
    {
      // Release any lock we hold here before calling the MixingFunction, since it will obtain the lock as needed itself
      if (bLocked) mixing->Release ();
      MixingFunction ();
      bLocked = false;
    }
    if (bLocked) mixing->Release ();
  }
}
