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
#include "iutil/eventh.h"
#include "iutil/comp.h"

#include "isndsys/ss_structs.h"
#include "isndsys/ss_driver.h"



struct iConfigFile;
struct iReporter;

class SndSysDriverOSS : public iSndSysSoftwareDriver, public csRunnable
{
public:
  SCF_DECLARE_IBASE;

  SndSysDriverOSS(iBase *piBase);
  virtual ~SndSysDriverOSS();

  /// Called to initialize the driver
  bool Open (csSndSysRendererSoftware*renderer,
    csSndSysSoundFormat *requested_format);

  /// Called to shutdown the driver
  void Close ();

  /// Start the thread that will drive audio
  bool StartThread();

  /// Stop the background thread
  void StopThread();

  /// The thread runnable procedure
  void Run ();

  // The system driver.
  static iObjectRegistry *object_reg;

  void Report (int severity, const char* msg, ...);


  // The loaded CS reporter
  static csRef<iReporter> reporter;

protected:
  uint8 *oss_buffer;
  csSndSysRendererSoftware *attached_renderer;
  csSndSysSoundFormat playback_format;

  char output_device[128];
  int output_fd;
  uint32 oss_buffer_bytes;

  /// A flag used to shut down the running background thread.
  //   We don't really need to synchronize access to this since a delay in recognizing a change
  //   isn't a big deal.
  volatile bool running;
  csRef<csThread> bgthread;

protected:
  void ClearBuffer();
  void WriteBuffer(size_t bytes);


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
    SCF_DECLARE_EMBEDDED_IBASE(SndSysDriverOSS);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;

};

#endif // #ifndef SNDSYS_SOFTWARE_DRIVER_OSS_H




