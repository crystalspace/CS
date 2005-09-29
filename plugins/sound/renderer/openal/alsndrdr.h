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

#ifndef __CS_SNDRDROPENAL_H__
#define __CS_SNDRDROPENAL_H__

#include "isound/renderer.h"
#include "isound/data.h"
#include "iutil/comp.h"
#include "csutil/cfgacc.h"
#include "iutil/eventh.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "csutil/thread.h"

#if defined(CS_OPENAL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENAL_PATH,al.h)
#include CS_HEADER_GLOBAL(CS_OPENAL_PATH,alut.h)
#else
#include <AL/al.h>
#include <AL/alut.h>
#endif

// If defined, information about buffer generation, queueing, dequeueing and destruction is reported
//#define OPENAL_DEBUG_BUFFERS
// If defined, information is reported when important functions are called
//#define OPENAL_DEBUG_CALLS


class csSoundListenerOpenAL;
class csSoundSourceOpenAL;
class csSoundHandleOpenAL;

/// This will render the sound through the openal interface
class csSoundRenderOpenAL : public iSoundRender
{
public:
  SCF_DECLARE_IBASE;
  csSoundRenderOpenAL(iBase *parent);
  virtual ~csSoundRenderOpenAL();

  bool Initialize (iObjectRegistry *object_reg);
  bool Open ();
  void Close ();
  virtual bool HandleEvent (iEvent &e);

  void SetVolume (float vol);
  float GetVolume ();

  csPtr<iSoundHandle> RegisterSound(iSoundData *);
  void UnregisterSound(iSoundHandle *);

  iSoundListener *GetListener () { return Listener; }
  void MixingFunction ();

  csConfigAccess &GetConfig () { return config; }

  void AddSource (csSoundSourceOpenAL *src);
  void RemoveSource (csSoundSourceOpenAL *src);


  /* These are set on a per source basis in OpenAl, so this comes from
     listener */
  void SetDistanceFactor (float d) { dist = d; }
  void SetRollOffFactor (float r) { roll = r; }


  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundRenderOpenAL);
    bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;

  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundRenderOpenAL);
    virtual bool HandleEvent (iEvent& e) { return scfParent->HandleEvent(e); }
  } scfiEventHandler;

  class OpenALRunnable : public csRunnable
  {
    csSoundRenderOpenAL *sr;
    int count;
    csRef<csMutex> mutex_count;

  public:
    OpenALRunnable (csSoundRenderOpenAL *rend): sr(rend),  count(1)
    { mutex_count=csMutex::Create ();}
    virtual ~OpenALRunnable () {}
    virtual void IncRef ()
    {mutex_count->LockWait(); count++; mutex_count->Release(); }
    virtual void DecRef ()
    {
      mutex_count->LockWait();
      if (--count == 0) { mutex_count->Release(); delete this; }
      else mutex_count->Release();
    }
    virtual void Run () {sr->ThreadProc();}
    virtual int GetRefCount () { return count; }
  };


  friend class OpenALRunnable;
  friend class csSoundListenerOpenAL;
  friend class csSoundSourceOpenAL;
  friend class csSoundHandleOpenAL;

private:
  csRef<iSoundListener> Listener;
  csConfigAccess config;
  csSoundFormat format;

  // Sound distance
  float dist;
  // Rolloff factor 
  float roll;

  /// True if the OpenAL library has been initialized
  bool al_open;

  /**
   * Process that performs the main thread loop of the background processing
   * thread (if there is one)
   */
  void ThreadProc();

  /// Update function
  void Update();


  /*
   * Everything that gets accessed by the background thread needs to be mutexed
   */
  csRef<csMutex> mutex_Listener;
  csRef<csMutex> mutex_ActiveSources;
  csRef<csMutex> mutex_SoundHandles;
  csRef<csMutex> mutex_OpenAL;

  /**
   * Stores the buffer length (in seconds) for streaming audio buffers. 
   *
   * Read from the config file option "Sound.OpenAL.StreamingBufferLength"
   * Default: 1.0 seconds (1.0)
   */
  float BufferLengthSeconds;

  /**
   * If false, each handle will keep an internal sound buffer that is used
   * when a source
   * is added to a stream that's already playing.
   * If true, a new source added to a playing stream will have an initial
   * period of silence
   * roughly equal to BufferLengthSeconds until the buffering catches up.
   *  
   * Read from the config file option "Sound.OpenAL.LazySourceSync"
   * Default: true (yes)
   */
  bool LazySourceSync;

  /**
   * True if a separate thread should be kicked off to handle sound buffer
   * procesing for streams. This is not needed for static sounds, but is
   * required for any decent streaming audio.
   *
   * Read from the config file option "Sound.OpenAL.BackgroundProcessing"
   * Default: true (yes)
   */
  bool BackgroundProcessing;

  /**
   * Stores the last value of csTicks.
   * The elapsed time between updates is calculated using this and passed to
   * the sound handle.
   * If the sound handle is an active streaming handle without any playing
   * sources it updates its
   * internal buffer, position and advances the associated data stream based
   * on this change.
   */
  csTicks LastTime;

  /**
   * Used to stop the background thread if it's running.  We don't care
   * about mutexing this since it's just a bool and syncing is not critical.
   */
  volatile bool bRunning;

  /// Pointer to the background thread
  csRef<csThread> bgThread;

  // Object registry pointer
  iObjectRegistry* object_reg;

  // List of current registered handles
  csRefArray<csSoundHandleOpenAL> SoundHandles;

  // List of active sources
  csRefArray<csSoundSourceOpenAL> ActiveSources;
};


#endif // __CS_SNDRDROPENAL_H__
