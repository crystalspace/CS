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

#ifndef __CS_SNDLSTNOPENAL_H__
#define __CS_SNDLSTNOPENAL_H__

#include "csplugincommon/soundrenderer/slstn.h"

class csSoundRenderOpenAL;

class csSoundListenerOpenAL : public csSoundListener
{
public:
  SCF_DECLARE_IBASE;
  csSoundListenerOpenAL(csSoundRenderOpenAL *p);
  virtual ~csSoundListenerOpenAL();

  void SetDirection (const csVector3 &Front, const csVector3 &Top);
  void SetPosition (const csVector3 &pos);
  void SetVelocity (const csVector3 &v);
  void SetDistanceFactor (float factor);
  void SetRollOffFactor (float factor);
  void SetDopplerFactor (float factor);
  void SetHeadSize (float size);
  void SetEnvironment (csSoundEnvironment env);

  void Prepare();

private:
  csRef<csSoundRenderOpenAL> SoundRender;
  ALfloat position[3];
  ALfloat velocity[3];
  ALfloat orientation[6];
};

#endif // __CS_SNDLSTNOPENAL_H__
