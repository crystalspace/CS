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

#ifndef __SOUND_RENDER_DS3D_H__
#define __SOUND_RENDER_DS3D_H__

// SoundRender.H
// csSoundRenderDS3D class.

#include "isnddata.h"
#include "isndrdr.h"
#include "csutil/csvector.h"

class csSoundListenerDS3D;
class csSoundSourceDS3D;
struct iConfigFileNew;

class csSoundRenderDS3D : public iSoundRender
{
public:
  DECLARE_IBASE;
  csSoundRenderDS3D(iBase *piBase);
  virtual ~csSoundRenderDS3D();
  virtual bool Initialize (iSystem *iSys);

  virtual bool Open ();
  virtual void Close ();

  virtual void SetVolume (float vol);
  virtual float GetVolume ();

  virtual void PlaySound (iSoundData *snd, bool Loop);
  virtual void PlaySound (iSoundStream *snd, bool Loop);
  virtual iSoundSource *CreateSource (iSoundData *snd, int Mode3d);
  virtual iSoundSource *CreateSource (iSoundStream *snd, int Mode3d);

  virtual iSoundListener *GetListener ();
  virtual const csSoundFormat *GetLoadFormat();
  virtual void Update();
  virtual void MixingFunction ();

  void SetDirty();
  void AddSource(csSoundSourceDS3D *src);
  void RemoveSource(csSoundSourceDS3D *src);

  const char *GetError(HRESULT result);

public:
  LPDIRECTSOUND AudioRenderer;
  iSystem *System;
  csSoundListenerDS3D *Listener;
  csSoundFormat LoadFormat;
  csVector ActiveSources;
  iConfigFileNew *Config;
};

#endif	//__SOUND_RENDER_DS3D_H__
