/*
    Copyright (C) 2002 by Jorrit Tyberghein, Daniel Duhprey

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

#ifndef __SNDRDR_H__
#define __SNDRDR_H__

#include "isound/renderer.h"
#include "isound/data.h"
#include "iutil/comp.h"
#include "csutil/cfgacc.h"
#include "iutil/eventh.h"

// Bah. The OpenAL 1.0 SDK for Windows has the headers directly
// below the Includes directory, and not in an "AL" subdir.
#ifdef _WIN32_
  #include <al.h>
  #include <alut.h>
#else
  #include <AL/al.h>
  #include <AL/alut.h>
#endif

class csSoundListenerOpenAL;
class csSoundSourceOpenAL;

/// This will render the sound through the openal interface
class csSoundRenderOpenAL : public iSoundRender
{
public:
  SCF_DECLARE_IBASE;
  csSoundRenderOpenAL(iBase *parent);
  virtual ~csSoundRenderOpenAL();

  bool Initialize (iObjectRegistry *object_reg);
  bool Open ();
  void Close ();
  virtual bool HandleEvent (iEvent &e);

  void SetVolume (float vol);
  float GetVolume ();

  csPtr<iSoundHandle> RegisterSound(iSoundData *);
  void UnregisterSound(iSoundHandle *);

  iSoundListener *GetListener () { return listener; }
  void MixingFunction ();

  csConfigAccess &GetConfig () { return config; }

  void AddSource (csSoundSourceOpenAL *src);
  void RemoveSource (csSoundSourceOpenAL *src);

  /* These are set on a per source basis in openal, so this comes in from listener */
  void SetDistanceFactor (float d) { dist = d; }
  void SetRollOffFactor (float r) { roll = r; }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundRenderOpenAL);
    bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;

  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundRenderOpenAL);
    virtual bool HandleEvent (iEvent& e) { return scfParent->HandleEvent(e); }
  } scfiEventHandler;

private:
  csRef<iObjectRegistry> object_reg;
  csRef<iSoundListener> listener;
  csConfigAccess config;
  csSoundFormat format;
  csVector handles;
  csVector sources;

  float dist;
  float roll;

  bool al_open;
};


#endif // __CS_SNDRDR_H__
