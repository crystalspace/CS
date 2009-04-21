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


#ifndef SNDSYS_SOFTWARE_DRIVER_NULL_H
#define SNDSYS_SOFTWARE_DRIVER_NULL_H

#include "csutil/threading/thread.h"

#include "iutil/eventh.h"
#include "iutil/comp.h"

#include "isndsys/ss_structs.h"
#include "isndsys/ss_driver.h"
#include "isndsys/ss_eventrecorder.h"


CS_PLUGIN_NAMESPACE_BEGIN(SndSysNull)
{

class SndSysDriverNull;

class SndSysDriverRunnable : public CS::Threading::Runnable
{
private:
  SndSysDriverNull* m_pParent;

public:
  SndSysDriverRunnable (SndSysDriverNull* pParent) 
    : m_pParent (pParent)
  { }
  virtual ~SndSysDriverRunnable () { }

  virtual void Run ();
};

// NULL implementation of the iSndSysSoftwareDriver interface
class SndSysDriverNull : 
  public scfImplementation2<SndSysDriverNull, iComponent, iSndSysSoftwareDriver>
{
public:
  SndSysDriverNull(iBase *piBase);
  virtual ~SndSysDriverNull();

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
  /// Send a message to the sound system event recorder as the driver
  void RecordEvent(SndSysEventLevel Severity, const char* msg, ...);

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

  /// A flag used to shut down the running background thread.
  // We don't really need to synchronize access to this since a delay in
  // recognizing a change isn't a big deal.
  volatile bool m_bRunning;

  /// A reference to the CS interface for our background thread
  csRef<CS::Threading::Thread> m_pBGThread;

  /// The event recorder interface, if active
  csRef<iSndSysEventRecorder> m_EventRecorder;

  /// The length of the virtual sound buffer in milliseconds
  csTicks m_BufferLengthms;
};

}
CS_PLUGIN_NAMESPACE_END(SndSysNull)

#endif // #ifndef SNDSYS_SOFTWARE_DRIVER_NULL_H




