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

#ifndef __CS_SOUNDLISTENERDS3D_H__
#define __CS_SOUNDLISTENERDS3D_H__

#include "csplugincommon/soundrenderer/slstn.h"

class csSoundRenderDS3D;

class csSoundListenerDS3D  : public csSoundListener
{
friend class csSoundRenderDS3D;
public:
  SCF_DECLARE_IBASE;
  csSoundListenerDS3D(iBase *piBase);
  virtual ~csSoundListenerDS3D();

  virtual void SetDirection (const csVector3 &Front, const csVector3 &Top);
  virtual void SetPosition (const csVector3 &pos);
  virtual void SetVelocity (const csVector3 &v);
  virtual void SetDistanceFactor (float factor);
  virtual void SetRollOffFactor (float factor);
  virtual void SetDopplerFactor (float factor);
  virtual void SetHeadSize (float size);
  virtual void SetEnvironment (csSoundEnvironment env);

  bool Initialize(csSoundRenderDS3D* srdr);
  void Prepare();

private:
  csRef<csSoundRenderDS3D> Renderer;
  LPDIRECTSOUNDBUFFER PrimaryBuffer;
  LPDIRECTSOUND3DLISTENER Listener;
  bool Dirty;
};

#endif // __CS_SOUNDLISTENERDS3D_H__
