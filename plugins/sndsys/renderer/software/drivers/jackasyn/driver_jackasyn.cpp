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
#include "driver_jackasyn.h"
#include <libjackoss.h>



CS_PLUGIN_NAMESPACE_BEGIN(SndSysJACKASYN)
{

SCF_IMPLEMENT_FACTORY (SndSysDriverJackasyn)


// The system driver.
iObjectRegistry *SndSysDriverJackasyn::object_reg=0;

SndSysDriverJackasyn::SndSysDriverJackasyn(iBase* piBase) :
  scfImplementationType (this, piBase),
 oss_buffer(0), output_fd(-1), running(false)
{
  object_reg = 0;
}


SndSysDriverJackasyn::~SndSysDriverJackasyn()
{
  if (output_fd>=0)
    close(output_fd);
  output_fd=-1;

  delete[] oss_buffer;
}

void SndSysDriverJackasyn::Report(int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);

  csRef<iReporter> reporter = csQueryRegistry<iReporter> (object_reg);

  if (reporter)
    reporter->ReportV (severity, "crystalspace.SndSys.driver.software.jackasyn", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}



bool SndSysDriverJackasyn::Initialize (iObjectRegistry *obj_reg)
{
  // copy the system pointer
  object_reg=obj_reg;

  Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Jackasyn driver for software sound renderer initialized.");

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
bool SndSysDriverJackasyn::Open (csSndSysRendererSoftware*renderer,
			     csSndSysSoundFormat *requested_format)
{
  int result, param;

  Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Jackasyn Driver: Open()");
//  CS_ASSERT (Config != 0);

  attached_renderer=renderer;

  output_fd=jackoss_open(output_device, O_WRONLY);
  if (output_fd==-1)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
      "Sound System: Jackasyn Driver: Failed to open output device [%s].", 
      output_device);
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
      Report (CS_REPORTER_SEVERITY_ERROR, "Sound System: Jackasyn Driver: Unhandled output bits %d. Forcing to 16 bit.", requested_format->Bits);
      requested_format->Bits=16;
      param=AFMT_S16_LE;
    break;
  }
  result=jackoss_ioctl(output_fd, SNDCTL_DSP_SETFMT, &param);
  if (result==-1)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Sound System: Jackasyn Driver: Failed to set output format to %d bit.", requested_format->Bits);
    jackoss_close(output_fd);
    output_fd=-1;
    return false;
  }

  // Set channels
  param=requested_format->Channels;
  result=jackoss_ioctl(output_fd, SNDCTL_DSP_CHANNELS, &param);
  if (result==-1)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Sound System: Jackasyn Driver: Failed to set output format to %d channels.", requested_format->Channels);
    jackoss_close(output_fd);
    output_fd=-1;
    return false;
  }

  // Set sample frequency
  param=requested_format->Freq;
  result=jackoss_ioctl(output_fd, SNDCTL_DSP_SPEED, &param);
  if (result==-1)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Sound System: Jackasyn Driver: Failed to set output bitrate to %d bits per channel per second.", requested_format->Freq);
    jackoss_close(output_fd);
    output_fd=-1;
    return false;
  }

  // Copy the final format into local storage
  memcpy(&playback_format, requested_format, sizeof(csSndSysSoundFormat));


  // Setup a playback buffer

  // 1/10th of a second
  oss_buffer_bytes=playback_format.Channels * (playback_format.Bits/8) * playback_format.Freq / 10;
  oss_buffer=new uint8[oss_buffer_bytes];

  return true;
}

void SndSysDriverJackasyn::Close ()
{
  if (output_fd)
  {
    jackoss_close(output_fd);
    output_fd=-1;
  }
}

bool SndSysDriverJackasyn::StartThread()
{
  if (running) return false;

  running=true;
  SndSysDriverRunnable* runnable = new SndSysDriverRunnable (this);
  bgthread.AttachNew (new CS::Threading::Thread (runnable, false));
  runnable->DecRef ();

  bgthread->Start();
  
  return true;
}


void SndSysDriverJackasyn::StopThread()
{
  running=false;
  csSleep(100);
}

void SndSysDriverRunnable::Run ()
{
  parent->Run ();
}

void SndSysDriverJackasyn::Run()
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
      uint32 bytes_used=attached_renderer->FillDriverBuffer(oss_buffer, oss_buffer_bytes, 0, 0);
      if (bytes_used > 0)
      {
        WriteBuffer(bytes_used);
      }
      last_write=current_ticks;
    }
    csSleep(0);
  }
}

void SndSysDriverJackasyn::ClearBuffer()
{
  if (oss_buffer)
    memset(oss_buffer, 0, sizeof(uint8) * oss_buffer_bytes);
}

void SndSysDriverJackasyn::WriteBuffer(size_t bytes)
{
  jackoss_write(output_fd, oss_buffer, bytes);
}

}
CS_PLUGIN_NAMESPACE_END(SndSysJACKASYN)

