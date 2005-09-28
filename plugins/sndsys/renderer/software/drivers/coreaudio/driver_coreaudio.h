/*
    Copyright (C) 2005 by Andrew Mann
    Based in part on work by Matt Reda <mreda@mac.com>

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

/*
 *  A.M. 
 * 
 *  I don't own a mac, so this driver is based off the old Crystal Space
 *  CoreAudio driver by Matt Reda (2001) and documentation found on the
 *  web.
 *
 *  To keep things simple, this driver takes audio formatted just like the
 *  Linux OSS and Windows DirectSound drivers take audio - 16 bit signed
 *  little endian.  Supporting 8 bit wouldn't be hard, but there's
 *  probably no reason to do that.  Supporting Big Endian and 24/32 bit
 *  samples would require a little bit of change to the software renderer,
 *  but not a huge amount.  The renderer itself already accumulates and
 *  normalizes samples using a 32 bit buffer size, and it has to perform
 *  all its work in native system endianess, so there's actually an extra
 *  conversion back to little endian on current Mac systems.
 *
 *  The other drivers are specifically threaded - the driver itself is a
 *  csRunnable and kicks off its own background thread to push data to the
 *  OS.  Since CoreAudio uses callbacks (ones which thankfully dont seem
 *  nearly as limited as the winmm callbacks in terms of supported
 *  operations), we use the callbacks as our 'background thread' here.
 */

#ifndef SNDSYS_SOFTWARE_DRIVER_COREAUDIO_H
#define SNDSYS_SOFTWARE_DRIVER_COREAUDIO_H

#include <CoreAudio/CoreAudio.h>

#include "csutil/cfgacc.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "isndsys/ss_structs.h"
#include "isndsys/ss_driver.h"

struct iReporter;

class csSndSysDriverCoreAudio : public iSndSysSoftwareDriver
{
public:
  SCF_DECLARE_IBASE;

  csSndSysDriverCoreAudio(iBase *piBase);
  virtual ~csSndSysDriverCoreAudio();

  /// Called to initialize the driver.
  bool Open (csSndSysRendererSoftware*, csSndSysSoundFormat* requested_format);

  /// Called to shutdown the driver.
  void Close ();

  /// Start the thread that will drive audio.
  bool StartThread();

  /// Stop the background thread.
  void StopThread();

  /// Report a diagnostic.
  void Report (int severity, const char* msg, ...);

  /// The CoreAudio callback really ends up here.
  OSStatus AudioProc(AudioDeviceID inDevice,
		     const AudioTimeStamp* inNow,
		     const AudioBufferList* inInputData,
                     const AudioTimeStamp* inInputTime,
		     AudioBufferList* outOutputData,
                     const AudioTimeStamp* inOutputTime);

protected:
  iObjectRegistry* object_reg;
  csSndSysRendererSoftware *attached_renderer;
  csSndSysSoundFormat playback_format;
  bool running;

  // The output device
  AudioDeviceID outputDeviceID;

public:
  /// Initialize the module.
  virtual bool Initialize (iObjectRegistry*);

  // iComponent implementation.
  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSndSysDriverCoreAudio);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // #ifndef SNDSYS_SOFTWARE_DRIVER_COREAUDIO_H
