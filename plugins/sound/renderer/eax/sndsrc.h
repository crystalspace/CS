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

#ifndef __CSSOUNDSOURCEEAX_H__
#define __CSSOUNDSOURCEEAX_H__

#include "isound/source.h"
#include <eax.h>

struct iSoundData;
class csSoundRenderEAX;
class csSoundHandleEAX;

class csSoundSourceEAX : public iSoundSource
{
public:
  SCF_DECLARE_IBASE;

  csSoundSourceEAX(iBase *scfParent);
  virtual ~csSoundSourceEAX();
  bool Initialize(csSoundRenderEAX *srdr, csSoundHandleEAX *hdl, int Mode3d, long NumSamples);

  void Report (int severity, const char* msg, ...);

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
  void Write(void *d, unsigned long NumBytes);
  void WriteMute(unsigned long NumBytes);
  void ClearBuffer();
  csSoundHandleEAX *GetSoundHandle();

private:
  // Position and velocity of sound object. These are copies of the internal
  // values to assure correct return values when calling Get*() while
  // deferred settings are not yet committed.
  csVector3 Position, Velocity;

  // sound buffers
  LPDIRECTSOUND3DBUFFER Buffer3D;
  LPDIRECTSOUNDBUFFER Buffer2D;

  // eax property-set
  LPKSPROPERTYSET EaxKsPropertiesSet;

  // renderer
  csSoundRenderEAX *Renderer;

  // frequency of sound data
  unsigned long BaseFrequency;

  // if this is false new samples must be written all the time
  bool Static;

  // the sound handle
  csSoundHandleEAX *SoundHandle;

  // size of the sound buffer in bytes, size of one sample in bytes
  unsigned long BufferBytes, SampleBytes;

  // true if the sound is looped
  bool Looped;

  // Current position for writing sound data (DS3D's own write cursor
  // doesn't work correctly)
  long WriteCursor;
};

#endif
