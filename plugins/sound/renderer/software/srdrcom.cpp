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
#include "qint.h"
#include "csutil/inifile.h"
#include "srdrcom.h"
#include "srdrlst.h"
#include "srdrsrc.h"
#include "isystem.h"
#include "isnddrv.h"
#include "isndldr.h"
#include "isnddata.h"

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
  Sources = NULL;
  VFS = NULL;
}

bool csSoundRenderSoftware::Initialize (iSystem *iSys)
{
  // copy the system pointer
  if (iSys == NULL) return false;
  System = iSys;

  // get the VFS interface
  VFS=QUERY_PLUGIN(System, iVFS);
  if (!VFS) return false;

  // read the config file
  Config = new csIniFile(VFS,"config/sound.cfg");

  const char *drv = Config->GetStr ("Driver","driver");
  if (drv && strlen (drv) > 0)
    ;
  else
  {    
  // load the sound driver plug-in
#ifdef SOUND_DRIVER
  drv = SOUND_DRIVER;	// "crystalspace.sound.driver.xxx"
#else
  drv = "crystalspace.sound.driver.null";
#endif
  }
  SoundDriver = LOAD_PLUGIN (System, drv, NULL, iSoundDriver);
  if (!SoundDriver) {	
    System->Printf(MSG_FATAL_ERROR, "Error! Cant find sound driver %s.\n", drv);
    exit(0);
  }

  // some other initialization stuff
  Listener = new csSoundListenerSoftware (NULL);
  Sources = new csVector();

  return true;
}

csSoundRenderSoftware::~csSoundRenderSoftware()
{
  if (SoundDriver) SoundDriver->DecRef();
  if (Listener) Listener->DecRef();

  if (Sources) {
    while (Sources->Length()>0) {
      iSoundSource *src=(iSoundSource*)(Sources->Get(0));
      src->DecRef();
    }
    delete Sources;
  }

  delete Config;

  VFS->DecRef();
}

iSoundListener *csSoundRenderSoftware::GetListener()
{
  return Listener;
}

iSoundSource *csSoundRenderSoftware::CreateSource(iSoundData *snd, bool Is3d)
{
  if (snd == NULL) return NULL;
  return new csSoundSourceSoftware(snd, Is3d, this, snd);
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

bool csSoundRenderSoftware::Open()
{
  System->Printf (MSG_INITIALIZATION, "SoundRender Software selected\n");

  SoundDriver->Open(this,
    Config->GetInt("SoundRender.Software", "FREQUENCY", 22050),
    Config->GetYesNo("SoundRender.Software", "16BITS", true),
    Config->GetYesNo("SoundRender.Software", "STEREO", true));

  float v=Config->GetFloat("SoundSystem","VOLUME",-1);
  if (v>1) v=1;
  if (v>=0) SetVolume(v);

  LoadFormat.Freq = getFrequency();
  LoadFormat.Bits = is16Bits()?16:8;
  LoadFormat.Channels = -1;

  return true;
}

void csSoundRenderSoftware::Close()
{
  SoundDriver->Close();
}

void csSoundRenderSoftware::SetVolume(float vol)
{
  SoundDriver->SetVolume(vol);
}

float csSoundRenderSoftware::GetVolume()
{
  return SoundDriver->GetVolume();
}

void csSoundRenderSoftware::PlayEphemeral(iSoundData *snd, bool Loop)
{
  iSoundSource *src=CreateSource(snd,false);
  if (!src) return;
  src->Play(Loop ? SOUND_LOOP : 0);
  src->DecRef();
}

void csSoundRenderSoftware::AddSource(csSoundSourceSoftware *src) {
  Sources->Push(src);
  src->IncRef();
}

void csSoundRenderSoftware::RemoveSource(csSoundSourceSoftware *src) {
  int n=Sources->Find(src);
  if (n!=-1) {
    Sources->Delete(n);
    src->DecRef();
  }
}

void csSoundRenderSoftware::MixingFunction()
{
  // test if we have a sound driver
  if(!SoundDriver) return;
	
  // if no sources exist, there may be an optimized way to handle this
  if (Sources->Length()==0 && SoundDriver->IsHandleVoidSound()) return;

  // lock sound memory
  SoundDriver->LockMemory(&memory, &memorysize);
  if(!memory || memorysize<1) return;

  // clear the buffer @@@ is this always needed?
  if (is16Bits()) memset(memory,0,memorysize);
  else memset(memory,128,memorysize);

  // prepare and play all sources
  long i;
  for (i=0;i<Sources->Length();i++) {
    csSoundSourceSoftware *src=(csSoundSourceSoftware*)(Sources->Get(i));
    // @@@ this divides volume by number of sources. If we don't do this,
    // sound can be distorted because of too high volume. Is there a better
    // solution?
    src->Prepare(Sources->Length());
    src->AddToBuffer(memory, memorysize);
    if (!src->IsActive()) {
      RemoveSource(src);
      i--;
    }
  }  

  SoundDriver->UnlockMemory();
}

void csSoundRenderSoftware::Update()
{
  // Is current sound driver work in a background thread ?
  bool res = SoundDriver->IsBackground();
	
  if(!res) // No !
  {
    // update sound data
    MixingFunction();
  }
}

const csSoundFormat *csSoundRenderSoftware::GetLoadFormat() {
  return &LoadFormat;
}
