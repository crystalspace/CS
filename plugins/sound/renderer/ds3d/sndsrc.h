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

#ifndef __CSSOUNDSOURCEDS3D_H__
#define __CSSOUNDSOURCEDS3D_H__

#include "isound/source.h"
struct iSoundData;

class csSoundSourceDS3D : public iSoundSource
{
public:
  DECLARE_IBASE;

  csSoundSourceDS3D(iBase *scfParent);
  virtual ~csSoundSourceDS3D();
  bool Initialize(csSoundRenderDS3D *srdr, iSoundStream *data, int Mode3d);

  virtual void Play (unsigned long playMethod = 0);
  virtual void Stop ();
  virtual void SetVolume (float volume);
  virtual float GetVolume ();
  virtual void SetFrequencyFactor (float factor);
  virtual float GetFrequencyFactor ();
  virtual int GetMode3D();
  virtual void SetMode3D(int m);
  virtual void SetPosition(csVector3 pos);
  virtual csVector3 GetPosition();
  virtual void SetVelocity(csVector3 spd);
  virtual csVector3 GetVelocity();

  bool IsPlaying();
  void Update();
  void Write(void *d, unsigned long NumBytes);
  void WriteMute(unsigned long NumBytes);

private:
  // Position and velocity of sound object. These are copies of the internal
  // values to assure correct return values when calling Get*() while
  // deferred settings are not yet committed.
  csVector3 Position, Velocity;

  // sound buffers
  LPDIRECTSOUND3DBUFFER Buffer3D;
  LPDIRECTSOUNDBUFFER Buffer2D;

  // renderer
  csSoundRenderDS3D *Renderer;

  // frequency of sound data
  unsigned long BaseFrequency;

  // if this is false new samples must be written all the time
  bool Precached;

  // the stream for the sample data. This is NULL if sound is precached.
  iSoundStream *SoundStream;

  // size of the sound buffer in bytes, size of one sample in bytes
  unsigned long BufferBytes, SampleBytes;

  // true if the sound is looped
  bool Looped;

  // if this is set the source must be stopped in the next call to Update().
  bool StopNextUpdate;
};

#endif
