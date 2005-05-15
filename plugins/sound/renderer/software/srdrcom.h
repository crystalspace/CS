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

#ifndef __CS_SRDRCOMSOFTWARE_H__
#define __CS_SRDRCOMSOFTWARE_H__

#include "csutil/scf.h"
#include "csutil/array.h"
#include "csutil/cfgacc.h"
#include "csutil/thread.h"
#include "isound/data.h"
#include "isound/renderer.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

struct iSoundDriver;
struct iConfigFile;
class csSoundListener;
class csSoundSourceSoftware;
class csSoundHandleSoftware;

class csSoundRenderSoftware : public iSoundRender
{
  friend class csSoundSourceSoftware;
  class MixerRunnable : public csRunnable
  {
    csSoundRenderSoftware *sr;
    int count;
  public:
    MixerRunnable (csSoundRenderSoftware *rend): sr(rend), count(1){}
    virtual ~MixerRunnable () {}
    virtual void IncRef () {count++;}
    virtual void DecRef () {if (--count == 0) delete this;}
    virtual void Run () {sr->ThreadedMix ();}
    virtual int GetRefCount () { return count; }
  };
  friend class MixerRunnable;

  void ThreadedMix ();

  // thread and mutex
  bool bRunning, owning, downing;
  csRef<csMutex> mixing;
  csRef<csCondition> data;
  csRef<csThread> mixer;

public:
  SCF_DECLARE_IBASE;
  // The system driver.
  iObjectRegistry *object_reg;

  csSoundRenderSoftware(iBase *piBase);
  virtual ~csSoundRenderSoftware();

  // implementation of iComponent
  virtual bool Initialize (iObjectRegistry *object_reg);

  // implementation of iSoundRender
  virtual void SetVolume (float vol);
  virtual float GetVolume ();
  virtual csPtr<iSoundHandle> RegisterSound(iSoundData *);
  virtual void UnregisterSound(iSoundHandle *);
  virtual iSoundListener *GetListener ();
  virtual void MixingFunction ();
  virtual bool HandleEvent (iEvent &e);

  void Update ();
  bool Open ();
  void Close ();

  void Report (int severity, const char* msg, ...);

  // add a sound source
  void AddSource(csSoundSourceSoftware *);
  // remove a sound source
  void RemoveSource(csSoundSourceSoftware *);

  // is 16 bit mode device
  bool is16Bits();
  // is a stereo device
  bool isStereo();
  // return frequency
  int getFrequency();

  // the config file
  csConfigAccess Config;

  // all active sound sources
  csArray<csSoundSourceSoftware*> Sources;

  // all registered sound handles
  csArray<csSoundHandleSoftware*> SoundHandles;

  // the low-level sound driver
  csRef<iSoundDriver> SoundDriver;

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
  csTicks LastTime;

  // sound renderer has been opened
  bool isOpen;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundRenderSoftware);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
  struct EventHandler : public iEventHandler
  {
  private:
    csSoundRenderSoftware* parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler (csSoundRenderSoftware* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE();
    }
    virtual bool HandleEvent (iEvent& e) { return parent->HandleEvent(e); }
  } * scfiEventHandler;
};

#endif // __CS_SRDRCOMSOFTWARE_H__
