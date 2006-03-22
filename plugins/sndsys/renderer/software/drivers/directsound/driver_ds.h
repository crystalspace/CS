/*
    Copyright (C) 2005 by Andrew Mann

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


#ifndef SNDSYS_SOFTWARE_DRIVER_DIRECTSOUND_H
#define SNDSYS_SOFTWARE_DRIVER_DIRECTSOUND_H

#include "csutil/cfgacc.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

#include "csutil/win32/win32.h"
#include "isndsys/ss_structs.h"
#include "isndsys/ss_driver.h"
#include "isndsys/ss_eventrecorder.h"

#include <dsound.h>


CS_PLUGIN_NAMESPACE_BEGIN(SndSysDIRECTSOUND)
{

class SndSysDriverDirectSound;

class SndSysDriverRunnable : public csRunnable
{
private:
  SndSysDriverDirectSound* parent;
  int ref_count;

public:
  SndSysDriverRunnable (SndSysDriverDirectSound* parent) :
  	parent (parent), ref_count (1) { }
  virtual ~SndSysDriverRunnable () { }

  virtual void Run ();
  virtual void IncRef() { ++ref_count; }
  /// Decrement reference count.
  virtual void DecRef()
  {
    --ref_count;
    if (ref_count <= 0)
      delete this;
  }

  /// Get reference count.
  virtual int GetRefCount() { return ref_count; }
};

struct iConfigFile;
struct iReporter;

class SndSysDriverDirectSound : public iSndSysSoftwareDriver
{
public:
  SCF_DECLARE_IBASE;

  SndSysDriverDirectSound(iBase *piBase);
  virtual ~SndSysDriverDirectSound();

  /// Called to initialize the driver
  bool Open (csSndSysRendererSoftware *renderer, 
    csSndSysSoundFormat *requested_format);

  /// Called to shutdown the driver
  void Close ();

  /// Start the thread that will drive audio
  bool StartThread();

  /// Stop the background thread
  void StopThread();

  /// The thread runnable procedure
  void Run ();

  /// Access interface to the object registry
  iObjectRegistry *m_pObjectRegistry;

protected:
  /// Pointer to the owning SndSysRendererSoftware
  csSndSysRendererSoftware *m_pAttachedRenderer;
  /// Local copy of the audio format parameters
  csSndSysSoundFormat m_PlaybackFormat;

  /// The DirectSound audio device interface
  LPDIRECTSOUND8 m_pDirectSoundDevice;
  /// The DirectSound audio buffer interface
  LPDIRECTSOUNDBUFFER m_pDirectSoundBuffer;
  
  /// Number of bytes in a single frame
  size_t m_BytesPerFrame;
  /// The size of the DirectSound allocated buffer, in bytes
  DWORD m_DirectSoundBufferBytes;
  /// The size of the DirectSound allocated buffer, in frames
  DWORD m_DirectSoundBufferFrames;
  /// The length of the DirectSound allocated buffer, in milliseconds
  csTicks m_BufferLengthms;
  /** The minimum number of empty frames that need to be available
   *    before a write is considered worthwhile.
   */
  DWORD m_DirectSoundBufferMinimumFillFrames;
  /// The number of underbuffer conditions that must occur before
  //   we take major corrective action
  int m_UnderBuffersAllowed;

  /// A flag used to shut down the running background thread.
  // We don't really need to synchronize access to this since a delay in
  // recognizing a change isn't a big deal.
  volatile bool m_bRunning;

  /// Handle to the csThread object that controls execution of our background thread
  csRef<csThread> m_pBackgroundThread;

  /// The event recorder interface, if active
  csRef<iSndSysEventRecorder> m_EventRecorder;

protected:
  /// Creates a new DirectSoundBuffer object using the current parameters
  bool CreateBuffer();

  /// Destroys the current DirectSoundBuffer object
  bool DestroyBuffer();

  /// Lock the entire buffer and fill it with silence
  void ClearBuffer();

  /// Send a message to the sound system event recorder as the driver
  void RecordEvent(SndSysEventLevel Severity, const char* msg, ...);

public:
  ////
  //
  // Interface implementation
  //
  ////

  // iComponent
  virtual bool Initialize (iObjectRegistry *obj_reg);


  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(SndSysDriverDirectSound);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;

};

}
CS_PLUGIN_NAMESPACE_END(SndSysDIRECTSOUND)

#endif // #ifndef SNDSYS_SOFTWARE_DRIVER_DIRECTSOUND_H
