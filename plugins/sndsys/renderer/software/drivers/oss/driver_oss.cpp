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

#include "cssysdef.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/soundcard.h>

#include "csutil/sysfunc.h"
#include "csutil/event.h"

#include "iutil/plugin.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "iutil/cmdline.h"
#include "ivaria/reporter.h"

#include "../../renderer.h"
#include "isndsys/ss_driver.h"
#include "isndsys/ss_renderer.h"
#include "driver_oss.h"


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (SndSysDriverOSS)


SCF_IMPLEMENT_IBASE(SndSysDriverOSS)
SCF_IMPLEMENTS_INTERFACE(iSndSysSoftwareDriver)
SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (SndSysDriverOSS::eiComponent)
SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

// The system driver.
iObjectRegistry *SndSysDriverOSS::object_reg=NULL;

// The loaded CS reporter
csRef<iReporter> SndSysDriverOSS::reporter;


SndSysDriverOSS::SndSysDriverOSS(iBase* piBase) :
 oss_buffer(NULL), output_fd(-1), running(false)
{
  SCF_CONSTRUCT_IBASE(piBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

//  scfiEventHandler = 0;
  object_reg = 0;
}


SndSysDriverOSS::~SndSysDriverOSS()
{
  if (output_fd>=0)
    close(output_fd);
  output_fd=-1;

  delete[] oss_buffer;

  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

void SndSysDriverOSS::Report(int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);

  if (!reporter)
    reporter = CS_QUERY_REGISTRY(object_reg, iReporter);

  if (reporter)
    reporter->ReportV (severity, "crystalspace.SndSys.driver.software.oss", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}



bool SndSysDriverOSS::Initialize (iObjectRegistry *obj_reg)
{
  // copy the system pointer
  object_reg=obj_reg;

  Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: OSS driver for software sound renderer initialized.");

  // read the config file
//  Config.AddConfig(object_reg, "/config/sound.cfg");


  strcpy(output_device, "/dev/dsp");
 
  return true;
}

//////////////////////////////////////////////////////////////////////////
// 
//  
//
//
//
//
//////////////////////////////////////////////////////////////////////////
bool SndSysDriverOSS::Open (SndSysRendererSoftware *renderer,SndSysSoundFormat *requested_format)
{
  int result, param;

  Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: OSS Driver: Open()");
//  CS_ASSERT (Config != 0);

  attached_renderer=renderer;

  output_fd=open(output_device, O_WRONLY, 0);
  if (output_fd==-1)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Sound System: OSS Driver: Failed to open output device [%s].", output_device);
    return false;
  }

  // Set bits per sample and sample format
  switch (requested_format->Bits)
  {
    case 8:
      param=AFMT_U8;
    break;
    case 16:
      param=AFMT_S16_LE;
    break;
    default:
      Report (CS_REPORTER_SEVERITY_ERROR, "Sound System: OSS Driver: Unhandled output bits %d. Forcing to 16 bit.", requested_format->Bits);
      requested_format->Bits=16;
      param=AFMT_S16_LE;
    break;
  }
  result=ioctl(output_fd, SNDCTL_DSP_SETFMT, &param);
  if (result==-1)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Sound System: OSS Driver: Failed to set output format to %d bit.", requested_format->Bits);
    close(output_fd);
    output_fd=-1;
    return false;
  }

  // Set channels
  param=requested_format->Channels;
  result=ioctl(output_fd, SNDCTL_DSP_CHANNELS, &param);
  if (result==-1)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Sound System: OSS Driver: Failed to set output format to %d channels.", requested_format->Channels);
    close(output_fd);
    output_fd=-1;
    return false;
  }

  // Set sample frequency
  param=requested_format->Freq;
  result=ioctl(output_fd, SNDCTL_DSP_SPEED, &param);
  if (result==-1)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Sound System: OSS Driver: Failed to set output bitrate to %d bits per channel per second.", requested_format->Freq);
    close(output_fd);
    output_fd=-1;
    return false;
  }

  // Copy the final format into local storage
  memcpy(&playback_format, requested_format, sizeof(SndSysSoundFormat));


  // Setup a playback buffer

  // 1/10th of a second
  oss_buffer_bytes=playback_format.Channels * (playback_format.Bits/8) * playback_format.Freq / 10;
  oss_buffer=new uint8[oss_buffer_bytes];

  return true;
}

void SndSysDriverOSS::Close ()
{
  if (output_fd)
  {
    close(output_fd);
    output_fd=-1;
  }
}

bool SndSysDriverOSS::StartThread()
{
  if (running) return false;

  running=true;
  bgthread = csThread::Create(this);

  bgthread->Start();
  
  return true;
}


void SndSysDriverOSS::StopThread()
{
  running=false;
  csSleep(100);
}

void SndSysDriverOSS::Run()
{
  csTicks last_write, current_ticks, tick_difference;

  // Write at about 20 times per second
  tick_difference=1000/20;

  last_write=csGetTicks();
  // Clear the buffer and write one section ahead
  ClearBuffer();
  WriteBuffer(oss_buffer_bytes);
  

  while (running)
  {
    current_ticks=csGetTicks();
    if (last_write + tick_difference <= current_ticks)
    {
      uint32 bytes_used=attached_renderer->FillDriverBuffer(oss_buffer, oss_buffer_bytes, NULL, 0);
      if (bytes_used > 0)
      {
        WriteBuffer(bytes_used);
      }
      last_write=current_ticks;
    }
    csSleep(0);
  }
}

void SndSysDriverOSS::ClearBuffer()
{
  if (oss_buffer)
    memset(oss_buffer, 0, sizeof(uint8) * oss_buffer_bytes);
}

void SndSysDriverOSS::WriteBuffer(size_t bytes)
{
  write(output_fd, oss_buffer, bytes);
}

