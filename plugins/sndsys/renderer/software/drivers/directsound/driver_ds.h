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
  SndSysDriverDirectSound* m_pParent;
  int m_RefCount;

public:
  SndSysDriverRunnable(SndSysDriverDirectSound* pParent) :
  	m_pParent(pParent), m_RefCount (1) 
  {
  }

  virtual ~SndSysDriverRunnable() 
  {
  }

  /// \brief The main running function of this thread
  virtual void Run();

  virtual void IncRef() 
  { 
    ++m_RefCount; 
  }

  /// Decrement reference count.
  virtual void DecRef()
  {
    --m_RefCount;
    if (m_RefCount <= 0)
      delete this;
  }

  /// Get reference count.
  virtual int GetRefCount() 
  {
    return m_RefCount; 
  }
};

struct iConfigFile;
struct iReporter;

class SndSysDriverDirectSound : 
  public scfImplementation2<SndSysDriverDirectSound, iComponent, iSndSysSoftwareDriver>
{
public:
  SndSysDriverDirectSound(iBase *pParent);
  virtual ~SndSysDriverDirectSound();


  ////
  // Interface implementation
  ////

  //------------------------
  // iComponent
  //------------------------
public:
  // iComponent
  virtual bool Initialize (iObjectRegistry *pObjectRegistry);

  //------------------------
  // iSndSysSoftwareDriver
  //------------------------
public:
  /// Called to initialize the driver
  bool Open (csSndSysRendererSoftware *pRenderer, 
    csSndSysSoundFormat *pRequestedFormat);

  /// Called to shutdown the driver
  void Close ();

  /// Start the thread that will drive audio
  bool StartThread();

  /// Stop the background thread
  void StopThread();

  ////
  // Member Functions
  ////
public:
  /// The thread runnable procedure
  void Run ();

protected:
  /// Creates a new DirectSoundBuffer object using the current parameters
  bool CreateBuffer();

  /// Destroys the current DirectSoundBuffer object
  bool DestroyBuffer();

  /// Lock the entire buffer and fill it with silence
  void ClearBuffer();

  /// Send a message to the sound system event recorder as the driver
  void RecordEvent(SndSysEventLevel Severity, const char* msg, ...);

  /// \brief Called to determine whether we have underbuffered based on cursor positions
  ///
  /// \note This test cannot detect conditions where we're underbuffered so far that the play cursor
  ///       has lapped the buffer.
  ///
  /// \return true : An underbuffer condition has occurred   false : No underbuffer condition has occurred 
  bool HasUnderbuffered(int DSWriteCursor, int DSPlayCursor, int RealWriteCursor);

  /// \brief Called to determine whether we have underbuffered based on last buffer fill time
  ///
  /// \note This test detects only conditions where we've underbuffered so far that the play cursor
  ///       has lapped the buffer
  ///
  /// \return true : An underbuffer condition has occurred   false : No underbuffer condition has occurred 
  bool HasUnderbuffered(csTicks CurrentTime, csTicks LastBufferFillTime);

  /// \brief Called on an underbuffer condition to determine whether corrective action should be taken
  ///
  /// \return true : Corrective action should be taken.    false : Corrective action should not be taken.
  bool NeedUnderbufferCorrection();

  /// \brief Expands the underlying directsound buffer, clears the buffer, and restarts playback at the begining.
  ///
  /// \return true : OK   false : Expansion failed, cannot continue playback.
  bool CorrectUnderbuffer();

  ////
  // Member Variables
  ////
protected:
  /// Access interface to the object registry
  iObjectRegistry *m_pObjectRegistry;

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

  /// \brief The byte offset into the directsound buffer where data should
  ///        next be written.
  int m_RealWriteCursor;

  /// The number of underbuffer conditions that must occur before
  //   we take major corrective action
  int m_UnderBuffersAllowed;

  /// \brief The number of underbuffer conditions that have occurred
  ///        since the last corrective action.
  int m_UnderBufferCount;

  /// A flag used to shut down the running background thread.
  // We don't really need to synchronize access to this since a delay in
  // recognizing a change isn't a big deal.
  volatile bool m_bRunning;

  /// Handle to the csThread object that controls execution of our background thread
  csRef<csThread> m_pBackgroundThread;

  /// The event recorder interface, if active
  csRef<iSndSysEventRecorder> m_pEventRecorder;

  /// \brief The windows Event object that will be signaled at pre-defined playback positions
  ///        to request buffer fills.
  HANDLE m_BufferFillNeededEvent;
};

}
CS_PLUGIN_NAMESPACE_END(SndSysDIRECTSOUND)

#endif // #ifndef SNDSYS_SOFTWARE_DRIVER_DIRECTSOUND_H
