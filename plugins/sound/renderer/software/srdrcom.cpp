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
#include "isys/isystem.h"
#include "iutil/icfgfile.h"
#include "isound/isnddrv.h"
#include "isound/isnddata.h"
#include "isys/ievent.h"

#include "srdrcom.h"
#include "srdrlst.h"
#include "srdrsrc.h"
#include "../common/convmeth.h"

IMPLEMENT_FACTORY (csSoundRenderSoftware)

EXPORT_CLASS_TABLE (sndsoft)
  EXPORT_CLASS (csSoundRenderSoftware, "crystalspace.sound.render.software",
    "Software Sound Renderer for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE(csSoundRenderSoftware)
	IMPLEMENTS_INTERFACE(iSoundRender)
	IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END;

csSoundRenderSoftware::csSoundRenderSoftware(iBase* piBase) : Listener(NULL)
{
  CONSTRUCT_IBASE(piBase);
  System = NULL;
  SoundDriver = NULL;
  Listener = NULL;
  memory = NULL;
  memorysize = 0;
  ActivateMixing = false;
}

bool csSoundRenderSoftware::Initialize (iSystem *iSys)
{
  // copy the system pointer
  System = iSys;

  // set event callback
  System->CallOnEvents(this, CSMASK_Command | CSMASK_Broadcast | CSMASK_Nothing);

  // read the config file
  Config.AddConfig(System, "/config/sound.cfg");

  // load the sound driver plug-in
#ifdef SOUND_DRIVER
  char *drv = SOUND_DRIVER;   // "crystalspace.sound.driver.xxx"
#else
  char *drv = "crystalspace.sound.driver.null";
#endif

  SoundDriver = LOAD_PLUGIN (System, drv, NULL, iSoundDriver);
  if (!SoundDriver) {	
    System->Printf(MSG_INITIALIZATION, "csSoundRenderSoftware::Initialize(): "
      "cannot load sound driver: %s\n", drv);
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
  System->Printf (MSG_INITIALIZATION, "Software Sound Renderer selected\n");

  SoundDriver->Open(this,
    Config->GetInt("Sound.Software.Frequency", 22050),
    Config->GetBool("Sound.Software.16Bits", true),
    Config->GetBool("Sound.Software.Stereo", true));

  Volume = Config->GetFloat("Sound.Volume", 1.0);
  if (Volume>1) Volume = 1;
  if (Volume<0) Volume = 0;

  Listener = new csSoundListenerSoftware (NULL);
  ActivateMixing = true;
  LoadFormat.Freq = getFrequency();
  LoadFormat.Bits = is16Bits() ? 16 : 8;
  LoadFormat.Channels = -1;

  System->Printf (MSG_INITIALIZATION, "  Playing %d Hz, %d bits, %s\n",
    getFrequency(), (is16Bits())?16:8, (isStereo())?"Stereo":"Mono");
  System->Printf (MSG_INITIALIZATION, "  Volume: %g\n", Volume);
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
}

IMPLEMENT_SOUNDRENDER_CONVENIENCE_METHODS(csSoundRenderSoftware);

iSoundSource *csSoundRenderSoftware::CreateSource(iSoundStream *Str, int mode3d) {
  if (!Str) return NULL;
  // check for correct input format
  const csSoundFormat *fmt = Str->GetFormat();
  if (fmt->Bits != (is16Bits()?16:8)) {
    System->Printf(MSG_WARNING, "incorrect input format for sound renderer (bits)\n");
    return NULL;
  }
  if (fmt->Freq != getFrequency()) {
    System->Printf(MSG_WARNING, "incorrect input format for sound renderer (frequency)\n");
    return NULL;
  }

  return new csSoundSourceSoftware(this, Str, mode3d);
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

const csSoundFormat *csSoundRenderSoftware::GetLoadFormat() {
  return &LoadFormat;
}

void csSoundRenderSoftware::MixingFunction()
{
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
  long i;
  for (i=0;i<Sources.Length();i++) {
    csSoundSourceSoftware *src=(csSoundSourceSoftware*)(Sources.Get(i));
    src->Prepare(Volume);
    src->AddToBuffer(memory, memorysize);
    if (!src->IsActive()) {
      RemoveSource(src);
      i--;
    }
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
