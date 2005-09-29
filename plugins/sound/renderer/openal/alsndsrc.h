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

#ifndef __CS_SNDSRCOPENAL_H__
#define __CS_SNDSRCOPENAL_H__

#include "csgeom/vector3.h"

#include "isound/source.h"

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
  bool IsPlaying();
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
  void SetMinimumDistance (float distance);
  void SetMaximumDistance (float distance);
  float GetMinimumDistance ();
  float GetMaximumDistance ();

  ALuint GetID () { return source; }

  csSoundHandleOpenAL *GetSoundHandle() { return SoundHandle; }

private:

  void Report(int severity, const char* msg, ...);

  // renderer
  csRef<csSoundRenderOpenAL> SoundRender;

  // the sound handle
  csRef<csSoundHandleOpenAL> SoundHandle;

  /**
   * If this is false the data is streamed and we must continually pull more
   * data from the data source during playback
   */
  bool Static;

  // OpenAL related values
  ALenum format;
  ALuint source;
  int frequency;

  /**
   * OpenAL cannot play when there are no buffers to be played, so we use
   * this as our official playing indicator.
   * 
   * When the Write() member function is called, if the source is not really
   * playing we should start playback and set this to false;
   */
  bool SourcePlaying; 

  // Position and velocity of sound object. These are copies of the internal
  // values to assure correct return values when calling Get*() while
  // deferred settings are not yet committed.
  ALfloat position[3];
  ALfloat velocity[3];

  int mode;

  /**
   * Called from the handle to notify the source that the underlying stream
   * has reached the end.
   *
   * This function is called when the handler's advanced pointer reaches the
   * end of it's stream data if was not played in a looping mode.  This does
   * not mean that the source should immediately stop since the source will
   * usually have more data buffered to be played.
   * Rather, it tells the source that the source's Write() member function
   * has been called for the final time, and the source should begin
   * checking for the actual end of playback when WatchBufferEnd() is called.
   *
   * The OpenAL plugin does not need to perform any processing in this call.
   */
  void NotifyStreamEnd();

  /**
   * Called from the handle instead of Write() while the source is still in
   * a playing state but the handle's data stream has ended.
   *
   * This function should perform any checks necessary to determine wether
   * the underlying sound system has finished playing all buffered
   * data in the stream.  It should stop the source (such that IsPlaying()
   * will return false) when this happens.
   *
   * OpenAL sources automatically stop when they run out of data.  For this
   * reason IsPlaying() cannot directly return the status of the
   * OpenAL source.  However, we can check the status of the OpenAL source
   * in this function since we know all remaining data has been
   * buffered.
   */
  void WatchBufferEnd();

  /**
   * Called from the handle to send a block of data to the sound system for
   * this source.
   *
   * OpenAL uses a rather abstract notion of buffers for the purpose of data
   * passing.  Buffers have no set size.  Buffers are
   * generated, data is added, and the buffer is queued.  Some time after the
   * buffer is played it is released to a state
   */
  void Write(void *Data, unsigned long NumBytes);

  friend class csSoundRenderOpenAL;
  friend class csSoundHandleOpenAL;
};

#endif // __CS_SNDSRCOPENAL_H__
