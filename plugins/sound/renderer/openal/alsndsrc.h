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

#ifndef __SNDSRC_H__
#define __SNDSRC_H__

#include "isound/source.h"
#include <AL/al.h>

struct iSoundData;
class csSoundRenderOpenAL;
class csSoundHandleOpenAL;

class csSoundSourceOpenAL : public iSoundSource
{
public:
  SCF_DECLARE_IBASE;

  csSoundSourceOpenAL(csSoundRenderOpenAL *rdr, csSoundHandleOpenAL *hdl);
  virtual ~csSoundSourceOpenAL();

  void Play (unsigned long playMethod = 0);
  void Stop ();
  void SetVolume (float volume);
  float GetVolume ();
  void SetFrequencyFactor (float factor);
  float GetFrequencyFactor ();
  int GetMode3D() { return mode; }
  void SetMode3D(int m);
  void SetPosition(csVector3 pos);
  csVector3 GetPosition() 
  { return csVector3 (position[0], position[1], position[2]); }
  void SetVelocity(csVector3 spd);
  csVector3 GetVelocity() 
  { return csVector3 (velocity[0], velocity[1], velocity[2]); }

  ALuint GetID () { return source; }

private:
  // renderer
  csSoundRenderOpenAL *parent;

  // the sound handle
  csRef<csSoundHandleOpenAL> handle;

  ALuint source;

  // Position and velocity of sound object. These are copies of the internal
  // values to assure correct return values when calling Get*() while
  // deferred settings are not yet committed.
  ALfloat position[3];
  ALfloat velocity[3];

  int mode;
};

#endif
