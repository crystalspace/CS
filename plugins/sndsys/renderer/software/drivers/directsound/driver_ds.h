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

#include <dsound.h>



struct iConfigFile;
struct iReporter;

class SndSysDriverDirectSound : public iSndSysSoftwareDriver, public csRunnable
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

  // The system driver.
  static iObjectRegistry *object_reg;

  const char *GetDSError(HRESULT hr);

  void Report (int severity, const char* msg, ...);


  // The loaded CS reporter
  static csRef<iReporter> reporter;

  /*
  csSoundFormat render_format;

  // TODO: Move to listener
  struct st_speaker_properties Speakers[MAX_CHANNELS];
  */

protected:
  csSndSysRendererSoftware *attached_renderer;
  csSndSysSoundFormat playback_format;

  LPDIRECTSOUND8 ds_device;
  LPDIRECTSOUNDBUFFER ds_buffer;
  uint32 ds_buffer_bytes;
  uint32 ds_buffer_minimum_fill_bytes;
  uint32 ds_buffer_writecursor;

  csRef<iWin32Assistant> win32Assistant;

  /// A flag used to shut down the running background thread.
  //   We don't really need to synchronize access to this since a delay in recognizing a change
  //   isn't a big deal.
  volatile bool running;
  csRef<csThread> bgthread;

protected:
  // Helper function to determine if the Direct Sound 'write' cursor has passed where we've written
  int GetWriteGap(uint32 real_play_cursor, uint32 real_write_cursor);
  uint32 GetWritableBytes(uint32 real_play_cursor);
  void AdvanceWriteBuffer(uint32 bytes);

  void ClearBuffer();


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

#endif // #ifndef SNDSYS_SOFTWARE_DRIVER_DIRECTSOUND_H
