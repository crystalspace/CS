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

#include <stdarg.h>
#include <stdio.h>

#include "cssysdef.h"
#include "isys/system.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "isound/driver.h"
#include "isound/data.h"
#include "isys/plugin.h"
#include "ivaria/reporter.h"

#include "srdrcom.h"
#include "../common/slstn.h"
#include "srdrsrc.h"
#include "sndhdl.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csSoundRenderSoftware)

SCF_EXPORT_CLASS_TABLE (sndsoft)
  SCF_EXPORT_CLASS (csSoundRenderSoftware,
    "crystalspace.sound.render.software",
    "Software Sound Renderer for Crystal Space")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE(csSoundRenderSoftware)
	SCF_IMPLEMENTS_INTERFACE(iSoundRender)
	SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iPlugin)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundRenderSoftware::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSoundRenderSoftware::csSoundRenderSoftware(iBase* piBase) : Listener(NULL)
{
  SCF_CONSTRUCT_IBASE(piBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
  object_reg = NULL;
  SoundDriver = NULL;
  Listener = NULL;
  memory = NULL;
  memorysize = 0;
  ActivateMixing = false;
}

void csSoundRenderSoftware::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (object_reg, iReporter);
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

  // set event callback
  iEventQueue* q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
    q->RegisterListener(&scfiPlugin,
      CSMASK_Command | CSMASK_Broadcast | CSMASK_Nothing);

  // read the config file
  Config.AddConfig(object_reg, "/config/sound.cfg");

  // load the sound driver plug-in
#ifdef CS_SOUND_DRIVER
  char *drv = CS_SOUND_DRIVER;   // "crystalspace.sound.driver.xxx"
#else
  char *drv = "crystalspace.sound.driver.null";
#endif

  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  SoundDriver = CS_LOAD_PLUGIN (plugin_mgr, drv, NULL, iSoundDriver);
  if (!SoundDriver) {	
    Report (CS_REPORTER_SEVERITY_ERROR,
      "csSoundRenderSoftware: Failed to load sound driver: %s", drv);
    return false;
  }

  return true;
}

csSoundRenderSoftware::~csSoundRenderSoftware()
{
  Close();
  if (SoundDriver) SoundDriver->DecRef();
}

bool csSoundRenderSoftware::Open()
{
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Software Sound Renderer selected");

  SoundDriver->Open(this,
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
  iSystem* sys = CS_GET_SYSTEM (object_reg);	//@@@
  sys->GetElapsedTime(et, ct);
  LastTime = ct;

  return true;
}

void csSoundRenderSoftware::Close()
{
  ActivateMixing = false;
  if (SoundDriver) {
    iSoundDriver *d = SoundDriver;
    SoundDriver = NULL;
    d->Close ();
    d->DecRef ();
  }

  if (Listener) {
    Listener->DecRef();
    Listener = NULL;
  }

  while (Sources.Length()>0)
    ((iSoundSource*)Sources.Get(0))->Stop();

  while (SoundHandles.Length()>0) {
    csSoundHandleSoftware *hdl = (csSoundHandleSoftware *)SoundHandles.Pop();
    hdl->Unregister();
    hdl->DecRef();
  }
}

iSoundHandle *csSoundRenderSoftware::RegisterSound(iSoundData *snd) {
  // convert the sound
  if (!snd->Initialize(&LoadFormat)) return NULL;

  // create the sound handle
  csSoundHandleSoftware *hdl = new csSoundHandleSoftware(this, snd);
  SoundHandles.Push(hdl);
  return hdl;
}

void csSoundRenderSoftware::UnregisterSound(iSoundHandle *snd) {
  int n = SoundHandles.Find(snd);
  if (n != -1) {
    csSoundHandleSoftware *hdl = (csSoundHandleSoftware *)snd;
    SoundHandles.Delete(n);
    hdl->Unregister();
    hdl->DecRef();
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

void csSoundRenderSoftware::AddSource(csSoundSourceSoftware *src) {
  Sources.Push(src);
  src->IncRef();
}

void csSoundRenderSoftware::RemoveSource(csSoundSourceSoftware *src) {
  int n=Sources.Find(src);
  if (n!=-1) {
    Sources.Delete(n);
    src->DecRef();
  }
}

void csSoundRenderSoftware::MixingFunction()
{
  long i;

  // look if this function is activated
  if (!ActivateMixing) return;

  // test if we have a sound driver
  if(!SoundDriver) return;

  // if no sources exist, there may be an optimized way to handle this
  if (Sources.Length()==0 && SoundDriver->IsHandleVoidSound()) return;

  // lock sound memory
  SoundDriver->LockMemory(&memory, &memorysize);
  if(!memory || memorysize<1) return;

  // clear the buffer
  if (is16Bits()) memset(memory,0,memorysize);
  else memset(memory,128,memorysize);

  // prepare and play all sources
  for (i=0;i<Sources.Length();i++) {
    csSoundSourceSoftware *src=(csSoundSourceSoftware*)(Sources.Get(i));
    src->Prepare(Volume);
    src->AddToBufferStatic(memory, memorysize);
    if (!src->IsActive()) {
      RemoveSource(src);
      i--;
    }
  }  

  // update sound handles
  long NumSamples = memorysize / (is16Bits()?2:1) / (isStereo()?2:1);
  for (i=0;i<SoundHandles.Length();i++) {
    csSoundHandleSoftware *hdl = (csSoundHandleSoftware*)SoundHandles.Get(i);
    hdl->UpdateCount(NumSamples);
  }

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
