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


#ifndef SNDSYS_SOFTWARE_DRIVER_OSS_H
#define SNDSYS_SOFTWARE_DRIVER_OSS_H

#include "csutil/cfgacc.h"
#include "csutil/threading/thread.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

#include "isndsys/ss_structs.h"
#include "isndsys/ss_driver.h"
#include "isndsys/ss_eventrecorder.h"



CS_PLUGIN_NAMESPACE_BEGIN(SndSysOSS)
{

class SndSysDriverOSS;

class SndSysDriverRunnable : public CS::Threading::Runnable
{
private:
  SndSysDriverOSS* parent;
  
public:
  SndSysDriverRunnable (SndSysDriverOSS* parent) :
  	parent (parent) { }
  virtual ~SndSysDriverRunnable () { }

  virtual void Run ();
};


class SndSysDriverOSS : 
  public scfImplementation2<SndSysDriverOSS, iComponent, iSndSysSoftwareDriver>
{
public:
  SndSysDriverOSS(iBase *pParent);
  virtual ~SndSysDriverOSS();

  ////
  // Interface implementation
  ////

  //------------------------
  // iComponent
  //------------------------
public:
  // iComponent
  virtual bool Initialize (iObjectRegistry *obj_reg);

  //------------------------
  // iSndSysSoftwareDriver
  //------------------------
public:
  /// Called to initialize the driver
  bool Open (csSndSysRendererSoftware*renderer, csSndSysSoundFormat *requested_format);

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
  /// Destroy any existing sound buffer and setup a new one using the current parameters.
  //    Uses m_BufferLengthms and m_PlaybackFormat to determine the size
  bool ResizeBuffer();

  /// Clear the entire sound buffer
  void ClearBuffer();

  /// Write sound data to the output device
  void WriteBuffer(size_t bytes);

  /// Send a message to the sound system event recorder as the driver
  void RecordEvent(SndSysEventLevel Severity, const char* msg, ...);


  ////
  // Member Variables
  ////
protected:
  /// Access interface to the object registry
  iObjectRegistry *m_pObjectRegistry;

  /// Pointer to the buffer we'll fill with sound for delivery to the 
  uint8 *m_pSoundBuffer;

  /// The size of the sound buffer in bytes
  uint32 m_SoundBufferSize;

  /// Pointer to the owning SndSysRendererSoftware
  csSndSysRendererSoftware *m_pAttachedRenderer;

  /// Local copy of the audio format parameters
  csSndSysSoundFormat m_PlaybackFormat;

  /// Name of the output device file (i.e. /dev/dsp)
  char m_OutputDeviceName[128];

  /// The file descriptor of the output device
  int m_OutputFileDescriptor;

  /// The maximum length audio we will send to OSS in advance, in milliseconds
  csTicks m_BufferLengthms;

  /// The number of underbuffer conditions that must occur before
  //   we take major corrective action
  int m_UnderBuffersAllowed;

  /// A flag used to shut down the m_bRunning background thread.
  // We don't really need to synchronize access to this since a delay in
  // recognizing a change isn't a big deal.
  volatile bool m_bRunning;

  /// Handle to the csThread object that controls execution of our background thread
  csRef<CS::Threading::Thread> m_pBackgroundThread;

  /// The event recorder interface, if active
  csRef<iSndSysEventRecorder> m_EventRecorder;
};

}
CS_PLUGIN_NAMESPACE_END(SndSysOSS)

#endif // #ifndef SNDSYS_SOFTWARE_DRIVER_OSS_H




