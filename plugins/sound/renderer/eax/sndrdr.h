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

#ifndef __CS_SNDRDR_H__
#define __CS_SNDRDR_H__

#include "isound/data.h"
#include "isound/renderer.h"
#include "isys/plugin.h"
#include "csutil/csvector.h"
#include "csutil/cfgacc.h"

class csSoundListenerEAX;
class csSoundSourceEAX;
struct iConfigFile;

class csSoundRenderEAX : public iSoundRender
{
public:
  SCF_DECLARE_IBASE;
  csSoundRenderEAX(iBase *piBase);
  virtual ~csSoundRenderEAX();
  virtual bool Initialize (iSystem *iSys);

  virtual void SetVolume (float vol);
  virtual float GetVolume ();

  virtual void PlaySound (iSoundData *snd, bool Loop);
  virtual void PlaySound (iSoundStream *snd, bool Loop);
  virtual iSoundSource *CreateSource (iSoundData *snd, int Mode3d);
  virtual iSoundSource *CreateSource (iSoundStream *snd, int Mode3d);

  virtual iSoundListener *GetListener ();
  virtual const csSoundFormat *GetLoadFormat();
  virtual void MixingFunction ();
  virtual bool HandleEvent (iEvent &e);

  bool Open ();
  void Close ();
  void Update();

  void SetDirty();
  void AddSource(csSoundSourceEAX *src);
  void RemoveSource(csSoundSourceEAX *src);

  const char *GetError(HRESULT result);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundRenderEAX);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent& e) { return scfParent->HandleEvent(e); }
  } scfiPlugin;

public:
  LPDIRECTSOUND AudioRenderer;
  iSystem *System;
  csSoundListenerEAX *Listener;
  csSoundFormat LoadFormat;
  csVector ActiveSources;
  csConfigAccess Config;
};

#endif// __CS_SNDRDR_H__
