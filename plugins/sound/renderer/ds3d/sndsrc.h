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

#ifndef __CS_SOUNDSOURCEDS3D_H__
#define __CS_SOUNDSOURCEDS3D_H__

#include "isound/source.h"

#include "csgeom/vector3.h"

struct iSoundData;
class csSoundRenderDS3D;
class csSoundHandleDS3D;

class csSoundSourceDS3D : public iSoundSource
{
public:
  SCF_DECLARE_IBASE;

  csSoundSourceDS3D(iBase *scfParent);
  virtual ~csSoundSourceDS3D();
  bool Initialize(csSoundRenderDS3D* srdr, csSoundHandleDS3D* hdl, int Mode3d,
    long NumSamples);

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
  virtual void SetMinimumDistance (float distance);
  virtual void SetMaximumDistance (float distance);
  virtual float GetMinimumDistance ();
  virtual float GetMaximumDistance ();


  bool IsPlaying();
  void Write(void *d, unsigned long NumBytes);
  void WriteMute(unsigned long NumBytes);
  void ClearBuffer();
  csSoundHandleDS3D *GetSoundHandle();

  /** Retrieves the number of bytes free in the buffer.  
   *  This is used to determine how much data should be added to keep the buffer full.
   */
  int32 GetFreeBufferSpace();

  /** Called by a soundhandle to obtain the write cursor position.
   *   Usefull if the soundhandle has internal buffering and needs to syncronize a new source.
   */
  long GetWriteCursor() { return WriteCursor; };

  

  /// Called by the handle to notify the source that the stream has ended
  void NotifyStreamEnd();
  /// Called from the handle instead of Write() or WriteMute() when the stream has ended to allow the source to wait for the playback buffer to end.
  void WatchBufferEnd();
  /// Fills the output buffer with silence.  Called from ClearBuffer().
  void FillBufferWithSilence();

private:
  // Position and velocity of sound object. These are copies of the internal
  // values to assure correct return values when calling Get*() while
  // deferred settings are not yet committed.
  csVector3 Position, Velocity;

  // Minimum and maximum distances of the sound object. Copies of internal values.
  float MinimumDistance,MaximumDistance;

  // sound buffers
  LPDIRECTSOUND3DBUFFER Buffer3D;
  LPDIRECTSOUNDBUFFER Buffer2D;

  // renderer
  csRef<csSoundRenderDS3D> Renderer;

  // frequency of sound data
  unsigned long BaseFrequency;

  /// If this is false the data is streamed and we must continually pull more data from the data source during playback
  bool Static;

  // the sound handle
  csRef<csSoundHandleDS3D> SoundHandle;

  // size of the sound buffer in bytes, size of one sample in bytes
  unsigned long BufferBytes, SampleBytes;

  // true if the sound is looped
  bool Looped;

  /** Current position for writing sound data
   *
   *  The D3D "write cursor" is not related to "new" or "old" data in the circular buffer.
   *  It is instead an indicator of the offset into the buffer that it's safe to write without interfering
   *  with playback.
   */
  long WriteCursor;
  
  /// Last position of the WriteCursor at the time that the stream ended. Only valid for streams.  Only valid when SoundHandle->ActiveStream is false.
  long PlayEnd;
};

#endif // __CS_SOUNDSOURCEDS3D_H__
