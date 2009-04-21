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


#ifndef SNDSYS_SOFTWARE_DRIVER_ALSA_H
#define SNDSYS_SOFTWARE_DRIVER_ALSA_H

#include "csutil/cfgacc.h"
#include "csutil/threading/thread.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

#include "isndsys/ss_structs.h"
#include "isndsys/ss_driver.h"
#include "isndsys/ss_eventrecorder.h"

// ALSA sound library header
#include <alsa/asoundlib.h>


struct iConfigFile;
struct iReporter;

CS_PLUGIN_NAMESPACE_BEGIN(SndSysALSA)
{

class SndSysDriverALSA;

class SndSysDriverRunnable : public CS::Threading::Runnable
{
private:
  SndSysDriverALSA* m_pParent;

public:
  SndSysDriverRunnable (SndSysDriverALSA* pParent) :
  	m_pParent (pParent) { }
  virtual ~SndSysDriverRunnable () { }

  virtual void Run ();
};

// ALSA implementation of the iSndSysSoftwareDriver interface
class SndSysDriverALSA : 
  public scfImplementation2<SndSysDriverALSA, iComponent, iSndSysSoftwareDriver>
{
public:
  SndSysDriverALSA(iBase *piBase);
  virtual ~SndSysDriverALSA();

  ////
  // Interface implementation
  ////

  //------------------------
  // iComponent
  //------------------------
public:
  virtual bool Initialize (iObjectRegistry *obj_reg);

  //------------------------
  // iSndSysSoftwareDriver
  //------------------------
public:

  /// Called to initialize the driver
  bool Open (csSndSysRendererSoftware* pRenderer,
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
  /// \brief Fill the ALSA mmap buffer with data from the renderer up to Bytes bytes
  /// 
  /// \return <0: An error occurred.  >=0 : Number of frames placed in the buffer
  snd_pcm_sframes_t FillBuffer(snd_pcm_sframes_t AvailableFrames);

  /// Setup ALSA 'hwparams'
  bool SetupHWParams();

  /// Setup ALSA 'swparams'
  bool SetupSWParams();

  /// Send a message to the sound system event recorder as the driver
  void RecordEvent(SndSysEventLevel Severity, const char* msg, ...);

  /// Returns true if a underbuffer condition has been detected
  bool HasUnderbuffered();

  /// Called if HasUnderbuffered() returns true to recover and possibly
  ///   take corrective action.
  bool HandleUnderbuffer();

  /// Returns true if the number of underbuffer conditions has exceeded
  ///  the configured allowed maximum before corrective action
  bool NeedUnderbufferCorrection();

  /// Take corrective action based on repeated underbuffer events.
  ///
  /// \return false: An uncorrectable error occurred
  ///         true: The situation has been handled (possibly without
  ///               any action taken)
  bool CorrectUnderbuffer();

  /// Called to provide special-case handling for ALSA returned
  ///  error codes.
  bool HandleALSAError(int ErrorNumber);

  ////
  // Member Variables
  ////
protected:
  /// Interface to the global object registry
  iObjectRegistry *m_pObjectReg;

  /// The renderer that's using this sound driver
  csSndSysRendererSoftware *m_pAttachedRenderer;

  /// The playback audio format, of course
  csSndSysSoundFormat m_PlaybackFormat;

  /// The name of the device which will be/is opened for playback
  //    This has the format hw:#,#  or plughw:#,#
  char m_OutputDeviceName[32];

  /// A pointer/handle to the output device
  //    I don't think the layout of this structure is meant to be dependable
  snd_pcm_t *m_pPCMDevice;

  /// The length of the ALSA sound buffer in milliseconds
  //   This must be set before calling SetupHWParams()
  csTicks m_BufferLengthms;

  /// Stores the size of the ALSA layer sound buffer in audio frames
  //   This is overwritten during the call to SetupHWParams()
  snd_pcm_uframes_t m_HWBufferFrames;

  /// The number of frames of the HWBuffer that we will not use
  //  Since ALSA in some cases seems to allocate excessive buffers,
  //    we will compensate by refusing to fill most of the buffer.
  snd_pcm_sframes_t m_HWUnusableFrames;

  /// A flag used to shut down the running background thread.
  // We don't really need to synchronize access to this since a delay in
  // recognizing a change isn't a big deal.
  volatile bool m_bRunning;

  /// A reference to the CS interface for our background thread
  csRef<CS::Threading::Thread> m_pBGThread;

  /// The event recorder interface, if active
  csRef<iSndSysEventRecorder> m_EventRecorder;

  /// Number of bytes in a frame of audio
  size_t m_BytesPerFrame;

  /// The number of underbuffer conditions that have occured since
  ///  the last corrective action.
  int m_UnderBufferCount;

  /// The number of underbuffer conditions that must occur before
  //   we take major corrective action
  int m_UnderBuffersAllowed;

  /** The minimum number of empty frames that need to be available
  *    before a write is considered worthwhile.
  */
  snd_pcm_sframes_t m_SoundBufferMinimumFillFrames;

};

}
CS_PLUGIN_NAMESPACE_END(SndSysALSA)

#endif // #ifndef SNDSYS_SOFTWARE_DRIVER_ALSA_H




