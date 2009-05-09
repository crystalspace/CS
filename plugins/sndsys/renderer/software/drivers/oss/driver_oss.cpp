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

#include <errno.h>
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




CS_PLUGIN_NAMESPACE_BEGIN(SndSysOSS)
{

SCF_IMPLEMENT_FACTORY (SndSysDriverOSS)


SndSysDriverOSS::SndSysDriverOSS(iBase* pParent) :
 scfImplementationType(this, pParent),
 m_pSoundBuffer(0), m_OutputFileDescriptor(-1), m_bRunning(false)
{
  m_pObjectRegistry = 0;
}


SndSysDriverOSS::~SndSysDriverOSS()
{
  if (m_OutputFileDescriptor>=0)
    close(m_OutputFileDescriptor);
  m_OutputFileDescriptor=-1;

  delete[] m_pSoundBuffer;
}

void SndSysDriverOSS::RecordEvent(SndSysEventLevel Severity, const char* msg, ...)
{
  if (!m_EventRecorder)
    return;

  va_list arg;
  va_start (arg, msg);
  m_EventRecorder->RecordEventV(SSEC_DRIVER, Severity, msg, arg);
  va_end (arg);
}

bool SndSysDriverOSS::Initialize (iObjectRegistry *obj_reg)
{
  /// Interface to the Configuration file
  csConfigAccess Config;

  // copy the system pointer
  m_pObjectRegistry=obj_reg;

  // Get an interface for event recorder (if present)
  m_EventRecorder = csQueryRegistry<iSndSysEventRecorder> (m_pObjectRegistry);

  // Critical because you really want to log this.  Trust me.  Really.
  RecordEvent(SSEL_CRITICAL, "OSS sound driver for software sound renderer initialized.");

  // read the config file
  Config.AddConfig(m_pObjectRegistry, "/config/sound.cfg");

  // check for optional output device name from the commandline
  csRef<iCommandLineParser> CMDLine (
    csQueryRegistry<iCommandLineParser> (m_pObjectRegistry));
  const char *OutputDeviceName = CMDLine->GetOption ("ossdevice");
  if (!OutputDeviceName)
    OutputDeviceName = Config->GetStr("SndSys.Driver.OSS.Device", "/dev/dsp");

  strcpy(m_OutputDeviceName, OutputDeviceName);


  // The 'buffer length' here is not a real buffer allocation, but
  //  is instead the maximum amount of data we will send ahead to OSS.
  m_BufferLengthms=0;
  if (CMDLine)
  {
    const char *BufferLengthStr = CMDLine->GetOption("soundbufferms");
    if (BufferLengthStr) m_BufferLengthms=atoi(BufferLengthStr);
  }

  // Check for sound config file option. Default to 20 ms if no option is found.
  if (m_BufferLengthms<=0)
    m_BufferLengthms = Config->GetInt("SndSys.Driver.OSS.SoundBufferms", 20);

  // The number of underbuffer events before the buffer size is automatically increased
  m_UnderBuffersAllowed=5;

  return true;
}


bool SndSysDriverOSS::Open (csSndSysRendererSoftware*renderer,
			     csSndSysSoundFormat *requested_format)
{
  int result, param;

  RecordEvent(SSEL_DEBUG, "OSS Driver: Open()");

  m_pAttachedRenderer=renderer;

  m_OutputFileDescriptor=open(m_OutputDeviceName, O_WRONLY, 0);
  if (m_OutputFileDescriptor==-1)
  {
    RecordEvent(SSEL_ERROR, "Failed to open output device [%s].", m_OutputDeviceName);
    return false;
  }

  // Set bits per sample and sample format
  switch (requested_format->Bits)
  {
    case 8:
      param=AFMT_U8;
    break;
    case 16:
      if ((requested_format->Flags & CSSNDSYS_SAMPLE_ENDIAN_MASK) == CSSNDSYS_SAMPLE_BIG_ENDIAN)
        param=AFMT_S16_BE;
      else
        param=AFMT_S16_LE;
    break;
    default:
      RecordEvent(SSEL_WARNING, "Unhandled output bits %d. Forcing to 16 bit.", requested_format->Bits);
      requested_format->Bits=16;
      if ((requested_format->Flags & CSSNDSYS_SAMPLE_ENDIAN_MASK) == CSSNDSYS_SAMPLE_BIG_ENDIAN)
        param=AFMT_S16_BE;
      else
        param=AFMT_S16_LE;
    break;
  }
  result=ioctl(m_OutputFileDescriptor, SNDCTL_DSP_SETFMT, &param);
  if (result==-1)
  {
    const char *endian_string;

    if ((requested_format->Flags & CSSNDSYS_SAMPLE_ENDIAN_MASK) == CSSNDSYS_SAMPLE_BIG_ENDIAN)
      endian_string="Big Endian";
    else
      endian_string="Little Endian";
    
    RecordEvent(SSEL_ERROR, "Failed to set output format to %d bit (%s).", requested_format->Bits, endian_string);
    close(m_OutputFileDescriptor);
    m_OutputFileDescriptor=-1;
    return false;
  }

  // Set channels
  param=requested_format->Channels;
  result=ioctl(m_OutputFileDescriptor, SNDCTL_DSP_CHANNELS, &param);
  if (result==-1)
  {
    RecordEvent(SSEL_ERROR, "Failed to set output format to %d channels.", requested_format->Channels);
    close(m_OutputFileDescriptor);
    m_OutputFileDescriptor=-1;
    return false;
  }

  // Set sample frequency
  param=requested_format->Freq;
  result=ioctl(m_OutputFileDescriptor, SNDCTL_DSP_SPEED, &param);
  if (result==-1)
  {
    RecordEvent(SSEL_ERROR, "Failed to set output bitrate to %d bits per channel per second.", requested_format->Freq);
    close(m_OutputFileDescriptor);
    m_OutputFileDescriptor=-1;
    return false;
  }

  // Copy the final format into local storage
  memcpy(&m_PlaybackFormat, requested_format, sizeof(csSndSysSoundFormat));

  // Setup our initial sound buffer
  if (!ResizeBuffer())
    return false;

  return true;
}

bool SndSysDriverOSS::ResizeBuffer()
{
  // Setup a buffer which will be used to move sound from the renderer to
  //  OSS.  This is set as the full size of the maximum latency time of the 
  //  OSS output.
  delete[] m_pSoundBuffer;

  m_SoundBufferSize=m_BufferLengthms * m_PlaybackFormat.Freq * m_PlaybackFormat.Bits/8 * m_PlaybackFormat.Channels / 1000;
  m_pSoundBuffer = new uint8[m_SoundBufferSize];

  RecordEvent(SSEL_DEBUG, "Resizing buffer to %d ms (%d frames)", m_BufferLengthms, m_BufferLengthms * m_PlaybackFormat.Freq / 1000);

  if (!m_pSoundBuffer)
    return false;
  return true;
}

void SndSysDriverOSS::Close ()
{
  if (m_OutputFileDescriptor)
  {
    close(m_OutputFileDescriptor);
    m_OutputFileDescriptor=-1;
  }
}

bool SndSysDriverOSS::StartThread()
{
  if (m_bRunning) return false;

  m_bRunning=true;
  SndSysDriverRunnable* runnable = new SndSysDriverRunnable (this);
  m_pBackgroundThread.AttachNew (new CS::Threading::Thread (runnable, false));
  runnable->DecRef ();

  m_pBackgroundThread->Start();
  
  return true;
}


void SndSysDriverOSS::StopThread()
{
  m_bRunning=false;
  csSleep(100);
}

void SndSysDriverRunnable::Run ()
{
  parent->Run ();
}

void SndSysDriverOSS::Run()
{
  csTicks last_write, current_ticks;
  int UnderBufferCount=0;

  // Clear the buffer and write the full buffer size ahead
  ClearBuffer();
  WriteBuffer(m_SoundBufferSize);
  // Mark the current time, OSS provides no timing information, so we
  //  rely entirely on the system clock
  last_write=csGetTicks();
  

  while (m_bRunning)
  {
    // Get the current time
    current_ticks=csGetTicks();
    
    // Determine if a write is worthwhile.  We will fill if at least 1/4 of the
    //  full buffer time has elapsed
    if ((current_ticks - last_write) > m_BufferLengthms/4)
    {
      // The number of frames needed to refill the OSS buffer
      size_t NeededFrames=(current_ticks - last_write) * m_PlaybackFormat.Freq / 1000;

      // Check for underbuffer - if the elapsed time has exceeded the total buffer time
      if ((current_ticks - last_write) >= m_BufferLengthms)
      {
        UnderBufferCount++;
        RecordEvent(SSEL_WARNING, "Underbuffer condition detected. Buffer length [%d ms] elapsed cycle [%d ms] Underbuffer count [%d] allowed [%d]",
          m_BufferLengthms, current_ticks - last_write, UnderBufferCount, m_UnderBuffersAllowed);

        // Maximum allowed underbuffer count exceeded, expand our buffer to compensate
        if (UnderBufferCount > m_UnderBuffersAllowed)
        {
          // Double our buffer length
          m_BufferLengthms*=2;

          if (!ResizeBuffer())
          {
            RecordEvent(SSEL_ERROR, "Failed to resize buffer!  Aborting main loop.");
            break;
          }

          // Reset the count to 0
          UnderBufferCount=0;

          // We'll need to adjust the number of frames so that we fill the entire buffer here
          NeededFrames=m_BufferLengthms * m_PlaybackFormat.Freq / 1000;
        }
      }

      // Fill the buffer with the required number of frames
      m_pAttachedRenderer->FillDriverBuffer(m_pSoundBuffer, NeededFrames, 0, 0);

      // Write the data to OSS
      WriteBuffer(NeededFrames);

      // Update our last write time
      last_write=current_ticks;
    }
    else
    {
      // Otherwise, sleep about 1/4 of the buffer length
      csSleep(m_BufferLengthms/4);
    }
  }
}

void SndSysDriverOSS::ClearBuffer()
{
  if (m_pSoundBuffer)
    memset(m_pSoundBuffer, 0, sizeof(uint8) * m_SoundBufferSize);
}

void SndSysDriverOSS::WriteBuffer(size_t Frames)
{
  const size_t size = Frames * m_PlaybackFormat.Channels * m_PlaybackFormat.Bits/8;
  const size_t res = write(m_OutputFileDescriptor, m_pSoundBuffer, size);
  if (res != size)
    RecordEvent(SSEL_WARNING, "WriteBuffer could only write %zu of %zu bytes"
      " (errno = %d)!", res, size, errno);
}

}
CS_PLUGIN_NAMESPACE_END(SndSysOSS)

