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

#ifndef __SOUND_RENDER_SOFTWARE_H__
#define __SOUND_RENDER_SOFTWARE_H__

// SoundRender.H
// csSoundRenderSoftware class.

#include "csutil/scf.h"
#include "csutil/csvector.h"
#include "csutil/cfgacc.h"
#include "isound/data.h"
#include "isound/renderer.h"

struct iSoundDriver;
struct iConfigFile;
class csSoundListener;
class csSoundSourceSoftware;

class csSoundRenderSoftware : public iSoundRender
{
friend class csSoundSourceSoftware;
public:
  DECLARE_IBASE;
  // the system driver
  iSystem *System;

  csSoundRenderSoftware(iBase *piBase);
  virtual ~csSoundRenderSoftware();
	
  // implementation of iPlugin
  virtual bool Initialize (iSystem *iSys);

  // implementation of iSoundRender
  virtual void SetVolume (float vol);
  virtual float GetVolume ();
  virtual iSoundHandle *RegisterSound(iSoundData *);
  virtual void UnregisterSound(iSoundHandle *);
  virtual iSoundListener *GetListener ();
  virtual void MixingFunction ();
  virtual bool HandleEvent (iEvent &e);

  void Update();
  bool Open ();
  void Close ();

  // add a sound source
  void AddSource(csSoundSourceSoftware *src);
  // remove a sound source
  void RemoveSource(csSoundSourceSoftware *src);

  // is 16 bit mode device
  bool is16Bits();
  // is a stereo device
  bool isStereo();
  // return frequency
  int getFrequency();

  // the config file
  csConfigAccess Config;

  // all active sound sources
  csVector Sources;

  // all registered sound handles
  csVector SoundHandles;

  // the low-level sound driver
  iSoundDriver *SoundDriver;

  // memory of the sound driver
  void *memory;
  int memorysize;
	
  // the global listener object
  csSoundListener *Listener;

  // is the mixing function acitvated?
  bool ActivateMixing;

  // the format used to load sounds
  csSoundFormat LoadFormat;

  // global volume setting
  float Volume;

  // previous time the sound handles were updated
  cs_time LastTime;
};

#endif

