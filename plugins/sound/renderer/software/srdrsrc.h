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

#ifndef __CSSOUNDBUFFERSOFTWARE_H__
#define __CSSOUNDBUFFERSOFTWARE_H__

#include "isndsrc.h"

class csSoundRenderSoftware;
class csSoundHandleSoftware;

class csSoundSourceSoftware : public iSoundSource
{
public:
  DECLARE_IBASE;
	
  csSoundSourceSoftware(csSoundRenderSoftware *srdr, iSoundStream *str, int snd3d);
  virtual ~csSoundSourceSoftware();
	
  virtual void Stop();
  virtual void Play(unsigned long playMethod);

  virtual void SetVolume(float vol);
  virtual float GetVolume();
  virtual void SetFrequencyFactor(float vol);
  virtual float GetFrequencyFactor();

  virtual void SetMode3D(int m);
  virtual int GetMode3D();
  virtual void SetPosition(csVector3 pos);
  virtual csVector3 GetPosition();
  virtual void SetVelocity(csVector3 spd);
  virtual csVector3 GetVelocity();

  // returns whether this source is currently being played.
  bool IsActive();

  // calculate internal values
  void Prepare(float BaseVolume);

  // add the data from this source to the global buffer
  void AddToBuffer(void *mem, long size);

protected:
  // pointer to the sound renderer
  csSoundRenderSoftware *SoundRender;
  // the sound stream for this source
  iSoundStream *SoundStream;
  // frequency factor - a factor of 1 plays the sound in its original frequency
  float FrequencyFactor;
  // volume
  float Volume;
  // 3d mode
  int Mode3d;
  // position
  csVector3 Position;
  // velocity
  csVector3 Velocity;
  // is this buffer currently being played
  bool Active;

  // playing method
  unsigned int PlayMethod;
  // calculated l/r volume
  float CalcVolL, CalcVolR;
  // calculated frequency factor
  float CalcFreqFactor;
};

#endif
