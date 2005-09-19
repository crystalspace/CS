/*
    Copyright (C) 2005 by Andrew Mann
    Based in part on work by Matt Reda

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

/*  A.M. 
 * 
 *  I don't own a mac, so this driver is based off the old Crystal Space CoreAudio driver 
 *  by Matt Reda (2001) and documentation found on the web.  
 *
 *  To keep things simple, this driver takes audio formatted just like the Linux OSS and Windows DirectSound drivers
 *  take audio - 16 bit signed little endian.  Supporting 8 bit wouldn't be hard, but there's probably no reason to do
 *  that.  Supporting Big Endian and 24/32 bit samples would require a little bit of change to the software renderer,
 *  but not a huge amount.  The renderer itself already accumulates and normalizes samples using a 32 bit buffer size,
 *  and it has to perform all its work in native system endianess, so there's actually an extra conversion back to 
 *  little endian on current Mac systems.
 *
 *  The other drivers are specifically threaded - the driver itself is a csRunnable and kicks off its own background thread
 *  to push data to the OS.  Since CoreAudio uses callbacks (ones which thankfully dont seem nearly as limited as the winmm
 *  callbacks in terms of supported operations), we use the callbacks as our 'background thread' here.
 *
 *  
 *
 *
*/

#ifndef SNDSYS_SOFTWARE_DRIVER_COREAUDIO_H
#define SNDSYS_SOFTWARE_DRIVER_COREAUDIO_H

#include <CoreAudio/CoreAudio.h>

#include "csutil/cfgacc.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

#include "ss_structs.h"
#include "ss_driver.h"



struct iConfigFile;
struct iReporter;

class SndSysDriverCoreAudio : public iSndSysSoftwareDriver
{
public:
  SCF_DECLARE_IBASE;

  SndSysDriverCoreAudio(iBase *piBase);
  virtual ~SndSysDriverCoreAudio();

  /// Called to initialize the driver
  bool Open (SndSysRendererSoftware *renderer,SndSysSoundFormat *requested_format);

  /// Called to shutdown the driver
  void Close ();

  /// Start the thread that will drive audio
  bool StartThread();

  /// Stop the background thread
  void StopThread();

  // The system driver.
  static iObjectRegistry *object_reg;

  void Report (int severity, const char* msg, ...);


  /// The CoreAudio callback really ends up here
  OSStatus AudioProc(AudioDeviceID inDevice, const AudioTimeStamp *inNow, const AudioBufferList *inInputData,
                     const AudioTimeStamp *inInputTime, AudioBufferList *outOutputData,
                     const AudioTimeStamp *inOutputTime);


  // The loaded CS reporter
  static csRef<iReporter> reporter;

protected:
  SndSysRendererSoftware *attached_renderer;
  SndSysSoundFormat playback_format;

  // The output device
  AudioDeviceID outputDeviceID;

protected:


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
    SCF_DECLARE_EMBEDDED_IBASE(SndSysDriverCoreAudio);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;

};

#endif // #ifndef SNDSYS_SOFTWARE_DRIVER_COREAUDIO_H




