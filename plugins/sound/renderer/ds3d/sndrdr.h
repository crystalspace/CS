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

#ifndef __CS_SNDRDRDS3D_H__
#define __CS_SNDRDRDS3D_H__

#include "isound/data.h"
#include "isound/renderer.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/cfgacc.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/thread.h"
#include "csutil/win32/win32.h"

#include <dsound.h>

class csSoundListenerDS3D;
class csSoundSourceDS3D;
class csSoundHandleDS3D;

/**
 * The Direct Sound 3D renderer plugin class for Crystal Space
 *
 * This driver follows a different philosophy for streamed sound than the
 * software driver. A DirectSound cyclic buffer is created that contains a
 * configurable time-length of sound.
 * On each update message, free space in this buffer is identified and
 * filled.  If the stream is non-looping and has reached the end, this
 * filling process fills the buffer with silence while watching for playback
 * to reach the end.
 *
 * This driver is much less sensative to poor or uneven frame rates on
 * streaming audio data.  In fact, for a trade-off of memory to use as
 * buffer space, there is no limit to how much audio can be buffered 
 * ahead of time.
 *
 *
 *
 * This plugin respects the following configuration options:
 *  Sound.Volume  - A floating point volume indicator (default is no setting
 *     - direct sound sets to an internal default)
 *  Sound.ds3d.StreamingBufferLength - A floating point value specifying
 *     the length of buffers (in seconds) created in Direct Sound to
 *     hold data from streaming sources. (default is 0.2 seconds)
 */
class csSoundRenderDS3D : public iSoundRender
{
public:
  SCF_DECLARE_IBASE;
  csSoundRenderDS3D(iBase *piBase);
  virtual ~csSoundRenderDS3D();
  virtual bool Initialize (iObjectRegistry *object_reg);

  virtual void SetVolume (float vol);
  virtual float GetVolume ();

  virtual csPtr<iSoundHandle> RegisterSound(iSoundData *);
  virtual void UnregisterSound(iSoundHandle *);

  virtual iSoundListener *GetListener ();

  /**
   * Mixing functionality is included in the underlying DirectSound
   * mechanism.  This function is empty.
   */
  virtual void MixingFunction ();
  /// Handles the Update, Open and Close events
  virtual bool HandleEvent (iEvent &e);

  /**
   * Called from the event handler when the plugin is opened, after
   * initialization.  Performs Direct Sound initialization.
   */
  bool Open ();
  /// Called from the event handler prior to destruction to tell us to shut down
  void Close ();
  /**
   * Called from the event handler regularly (once per Crystal Space
   * cycle/frame).  Performs buffer fills, looping checks, termination checks.
   */
  void Update();

  void SetDirty();
  void AddSource(csSoundSourceDS3D *src);
  void RemoveSource(csSoundSourceDS3D *src);

  /**
   * Retrieves DirectSound specific errors based on an HRESULT returned
   * from a DirectSound call.
   */
  const char *GetError(HRESULT result);

  /**
   * Process that performs the main thread loop of the background processing
   * thread (if there is one)
   */
  void ThreadProc();
  
  /// Pointer to the DirectSound interface
  LPDIRECTSOUND AudioRenderer;
 
  csRef<iWin32Assistant> win32Assistant;
  iObjectRegistry* object_reg;
  /// Contains information about how the sound should be heard
  csRef<csSoundListenerDS3D> Listener;
  /// The requested playback format that this renderer should request from sound data objects.
  csSoundFormat LoadFormat;
  /**
   * Array of sound sources.  These are representations of places or things
   * that create sound.
   */
  csRefArray<csSoundSourceDS3D> ActiveSources;
  /**
   * Array of sound handles.  Sound handles wrap sound data and associate
   * renderer specific data and functions.
   */
  csRefArray<csSoundHandleDS3D> SoundHandles;
  csConfigAccess Config;

  /*
   * Everything that gets accessed by the background thread needs to be mutexed
   */
  csRef<csMutex> mutex_Listener;
  csRef<csMutex> mutex_ActiveSources;
  csRef<csMutex> mutex_SoundHandles;

  /**
   * Stores the buffer length (in seconds) for streaming audio buffers.  
   * Read from the config file option "Sound.ds3d.StreamingBufferLength"
   * Default: 3.0 seconds
   */
  float BufferLengthSeconds;

  /**
   * False if sounds should be heard even when the application does not have
   * focus. Default is true.
   */
  bool MuteInBackground;

  /**
   * If false, each handle will keep an internal sound buffer that is used
   * when a source is added to a stream that's already playing.
   * If true, a new source added to a playing stream will have an initial
   * period of silence roughly equal to BufferLengthSeconds until the
   * buffering catches up.
   */
  bool LazySourceSync;
 
  /**
   * Stores the last value of csTicks.
   * The elapsed time between updates is calculated using this and passed to
   * the sound handle. If the sound handle is an active streaming handle
   * without any playing sources it updates its internal buffer, position
   * and advances the associated data stream based on this change.
   */
  csTicks LastTime;

  /**
   * True if a separate thread should be kicked off to handle sound buffer
   * procesing for streams. This is not needed for static sounds
   */
  bool BackgroundProcessing;

  /**
   * Used to stop the background thread if it's running.  We don't care
   * about mutexing this since it's just a bool.
   */
  volatile bool bRunning;

  /// Pointer to the background thread
  csRef<csThread> bgThread;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundRenderDS3D);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;

  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundRenderDS3D);
    virtual bool HandleEvent (iEvent& e) { return scfParent->HandleEvent(e); }
  } scfiEventHandler;

  class DS3DRunnable : public csRunnable
  {
    csSoundRenderDS3D *sr;
    int count;
    csRef<csMutex> mutex_count;

  public:
    DS3DRunnable (csSoundRenderDS3D *rend): sr(rend),  count(1)
    {
      mutex_count=csMutex::Create ();
    }
    virtual ~DS3DRunnable () {}
    virtual void IncRef ()
    {
      mutex_count->LockWait();
      count++;
      mutex_count->Release();
    }
    virtual void DecRef ()
    {
      mutex_count->LockWait();
      if (--count == 0)
      {
        mutex_count->Release();
	delete this;
      }
      else
        mutex_count->Release();
    }
    virtual void Run () {sr->ThreadProc();}
    virtual int GetRefCount () { return count; }
  };
  friend class DS3DRunnable;


};

#endif // __CS_SNDRDRDS3D_H__
