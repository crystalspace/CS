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

#ifndef __CSSOUNDLISTENEREAX_H__
#define __CSSOUNDLISTENEREAX_H__

#include "isound/isndlstn.h"

class csSoundRenderEAX;

class csSoundListenerEAX  : public iSoundListener
{
friend class csSoundRenderEAX;
public:
  DECLARE_IBASE;
  csSoundListenerEAX(iBase *piBase);
  virtual ~csSoundListenerEAX();
	
  virtual void SetDirection (csVector3 Front, csVector3 Top);
  virtual void SetPosition (csVector3 pos);
  virtual void SetVelocity (csVector3 v);
  virtual void SetDistanceFactor (float factor);
  virtual void SetRollOffFactor (float factor);
  virtual void SetDopplerFactor (float factor);
  virtual void SetHeadSize (float size);
  virtual void SetEnvironment (SoundEnvironment env);
  virtual void GetDirection (csVector3 &Front, csVector3 &Top);
  virtual csVector3 GetPosition ();
  virtual csVector3 GetVelocity ();
  virtual float GetDistanceFactor ();
  virtual float GetRollOffFactor ();
  virtual float GetDopplerFactor ();
  virtual float GetHeadSize ();
  virtual SoundEnvironment GetEnvironment ();

  bool Initialize(csSoundRenderEAX *srdr);
  void Prepare();

private:
  csSoundRenderEAX *Renderer;
  LPDIRECTSOUNDBUFFER PrimaryBuffer;
  LPDIRECTSOUND3DLISTENER Listener;

	// For Eax support
	LPKSPROPERTYSET EaxKsPropertiesSet;

  // we have to store these values outside the ds3d object. This assures that
  // correct values are returned in the Get*() functions for deferred setup,
  // which may not have applied the changes yet.
  bool Dirty;
  csVector3 Position;
  csVector3 Velocity;
  csVector3 Front, Top;
  float DistanceFactor, RollOff, Doppler, HeadSize;
  SoundEnvironment Environment;
};

#endif
