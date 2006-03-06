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
#include "iutil/eventh.h"
#include "iutil/comp.h"

#include "isndsys/ss_structs.h"
#include "isndsys/ss_driver.h"
#include "isndsys/ss_eventrecorder.h"

// ALSA sound library header
#include <alsa/asoundlib.h>


struct iConfigFile;
struct iReporter;

class SndSysDriverALSA;

class SndSysDriverRunnable : public csRunnable
{
private:
  SndSysDriverALSA* parent;
  int ref_count;

public:
  SndSysDriverRunnable (SndSysDriverALSA* parent) :
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


class SndSysDriverALSA : public iSndSysSoftwareDriver
{
public:
  SCF_DECLARE_IBASE;

  SndSysDriverALSA(iBase *piBase);
  virtual ~SndSysDriverALSA();

  /// Called to initialize the driver
  bool Open (csSndSysRendererSoftware* pRenderer,
    csSndSysSoundFormat *pRequestedFormat);

  /// Called to shutdown the driver
  void Close ();

  /// Start the thread that will drive audio
  bool StartThread();

  /// Stop the background thread
  void StopThread();

  /// The thread runnable procedure
  void Run ();

  /// Interface to the global object registry
  //    
  static iObjectRegistry *m_pObjectReg;

protected:
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

  /// Stores the size of the ALSA layer sound buffer in bytes
  snd_pcm_uframes_t m_HWBufferBytes;

  /// A flag used to shut down the running background thread.
  // We don't really need to synchronize access to this since a delay in
  // recognizing a change isn't a big deal.
  volatile bool m_Running;
  csRef<csThread> m_pBGThread;

  /// The event recorder interface, if active
  csRef<iSndSysEventRecorder> m_EventRecorder;

  /// Number of bytes in a frame of audio
  size_t m_BytesPerFrame;
protected:
  /// Write silence to the ALSA mmap buffer up to Bytes bytes
  size_t ClearBuffer(size_t Bytes);

  /// Fill the ALSA mmap buffer with data from the renderer up to Bytes bytes
  int FillBuffer(snd_pcm_sframes_t& AvailableFrames, snd_pcm_uframes_t& MappedFrames, snd_pcm_uframes_t& FilledFrames);
  bool SetupHWParams();
  bool SetupSWParams();

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
    SCF_DECLARE_EMBEDDED_IBASE(SndSysDriverALSA);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;

};

#endif // #ifndef SNDSYS_SOFTWARE_DRIVER_ALSA_H




