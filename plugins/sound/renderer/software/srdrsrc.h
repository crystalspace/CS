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

#ifndef __CS_SOUNDBUFFERSOFTWARE_H__
#define __CS_SOUNDBUFFERSOFTWARE_H__

#include "csgeom/vector3.h"
#include "csutil/thread.h"

#include "isound/source.h"


class csSoundRenderSoftware;
class csSoundHandleSoftware;

class csSoundSourceSoftware : public iSoundSource
{
public:
  SCF_DECLARE_IBASE;

  csSoundSourceSoftware(csSoundRenderSoftware *srdr,
    csSoundHandleSoftware *hdl, int snd3d);
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

  // add the data from this source to the global buffer (static sound)
  void AddToBufferStatic(void *mem, long size);

  // add the source buffer to the dest buffer, using the settings from this
  // source (volume, format etc.)
  void WriteBuffer(const void *Source, void *Dest, long NumSamples);

  void SetMinimumDistance (float distance);
  void SetMaximumDistance (float distance);
  float GetMinimumDistance ();
  float GetMaximumDistance ();


  // pointer to the sound renderer
  csSoundRenderSoftware *SoundRender;
  // the sound stream for this source
  csSoundHandleSoftware *SoundHandle;
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
  // current position in the sound (only for static sounds)
  long SoundPos;

  // playing method
  unsigned int PlayMethod;
  // calculated l/r volume
  float CalcVolL, CalcVolR;
  // calculated frequency factor
  float CalcFreqFactor;
  // stored sample offset used between steps  0.0<=SampleOffset<1.0
  float SampleOffset;

  /// Closer than the MinimumDistance the sound is heard at full volume. Futher than the maximum distance it is not heard at all.  Between, the sound is attenuated using the rolloff factor.
  float MinimumDistance,MaximumDistance;

protected:
  // restart the sound to the beginning
  void Restart();
  // Controls access to data that may be used by the renderer in another thread
  csRef<csMutex> mutex_RenderLock;

};

#endif //  __CS_SOUNDBUFFERSOFTWARE_H__
