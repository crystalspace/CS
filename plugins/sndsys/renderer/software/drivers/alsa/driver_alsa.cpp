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
#include <sys/time.h>

#include "csutil/sysfunc.h"
#include "csutil/event.h"

#include "iutil/plugin.h"
#include "iutil/cfgfile.h"
#include "csutil/cfgacc.h"
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
#include "driver_alsa.h"


CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(SndSysALSA)
{

SCF_IMPLEMENT_FACTORY (SndSysDriverALSA)


SCF_IMPLEMENT_IBASE(SndSysDriverALSA)
SCF_IMPLEMENTS_INTERFACE(iSndSysSoftwareDriver)
SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (SndSysDriverALSA::eiComponent)
SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

// The system driver.
iObjectRegistry *SndSysDriverALSA::m_pObjectReg=0;

SndSysDriverALSA::SndSysDriverALSA(iBase* piBase) :
  m_Running(false)
{
  SCF_CONSTRUCT_IBASE(piBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  m_pPCMDevice = 0;

//  scfiEventHandler = 0;
  m_pObjectReg = 0;
}


SndSysDriverALSA::~SndSysDriverALSA()
{

  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}


bool SndSysDriverALSA::Initialize (iObjectRegistry *pObjectReg)
{
  /// Interface to the Configuration file
  csConfigAccess Config;

  // Store the Object registry interface pointer, we'll need it later
  m_pObjectReg=pObjectReg;

  // Get an interface for event recorder (if present)
  m_EventRecorder = csQueryRegistry<iSndSysEventRecorder> (m_pObjectReg);

  // Critical because you really want to log this.  Trust me.  Really.
  RecordEvent(SSEL_CRITICAL, "ALSA driver for software sound renderer initialized.");

  // Make sure sound.cfg is available
  Config.AddConfig(m_pObjectReg, "/config/sound.cfg");

  // check for optional output device name from the commandline
  csRef<iCommandLineParser> CMDLine (CS_QUERY_REGISTRY (m_pObjectReg,
    iCommandLineParser));
  const char *OutputDeviceName = CMDLine->GetOption ("alsadevice");
  if (!OutputDeviceName)
    OutputDeviceName = Config->GetStr("SndSys.Driver.ALSA.Device", "default");

  // Copy into the used buffer
  strcpy(m_OutputDeviceName, OutputDeviceName);
 
  return true;
}

void SndSysDriverALSA::RecordEvent(SndSysEventLevel Severity, const char* msg, ...)
{
  if (!m_EventRecorder)
    return;

  va_list arg;
  va_start (arg, msg);
  m_EventRecorder->RecordEventV(SSEC_DRIVER, Severity, msg, arg);
  va_end (arg);
}


bool SndSysDriverALSA::SetupHWParams()
{
  int result;
  snd_pcm_hw_params_t *pHWParams;

  // Allocate hardware parameter structure on the stack
  snd_pcm_hw_params_alloca(&pHWParams);

  // Retrieve all current parameters
  result = snd_pcm_hw_params_any(m_pPCMDevice, pHWParams);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to retrieve any sound hardware configuration parameters.  Error [%s]", 
      snd_strerror(result));
    return false;
  }

  // Set the buffer format to interleaved access, MMAP memory pages
  // TODO: In the future we may be able to optimize the renderer by allowing it to leave samples non-interleaved
  result = snd_pcm_hw_params_set_access(m_pPCMDevice, pHWParams, SND_PCM_ACCESS_MMAP_INTERLEAVED);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to set interleaved MMAP write access for sound device.  Error [%s]", 
      snd_strerror(result));
    return false;
  }

  // Set bits per sample and sample format
  snd_pcm_format_t format;
  switch (m_PlaybackFormat.Bits)
  {
    case 8:
      format=SND_PCM_FORMAT_U8;
      break;
    case 16:
      if ((m_PlaybackFormat.Flags & CSSNDSYS_SAMPLE_ENDIAN_MASK) == CSSNDSYS_SAMPLE_BIG_ENDIAN)
        format=SND_PCM_FORMAT_S16_BE;
      else
        format=SND_PCM_FORMAT_S16_LE;
      break;
    default:
      RecordEvent(SSEL_WARNING, "Unhandled output bits %d. Forcing to 16 bit.", m_PlaybackFormat.Bits);
      m_PlaybackFormat.Bits=16;
      if ((m_PlaybackFormat.Flags & CSSNDSYS_SAMPLE_ENDIAN_MASK) == CSSNDSYS_SAMPLE_BIG_ENDIAN)
        format=SND_PCM_FORMAT_S16_BE;
      else
        format=SND_PCM_FORMAT_S16_LE;
      break;
  }
  result = snd_pcm_hw_params_set_format(m_pPCMDevice, pHWParams, format);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to set output format to %d bit (%s).  Error [%s]", 
      m_PlaybackFormat.Bits, 
      ((m_PlaybackFormat.Flags & CSSNDSYS_SAMPLE_ENDIAN_MASK) == CSSNDSYS_SAMPLE_BIG_ENDIAN) ?"Big Endian" : "Little Endian",
      snd_strerror(result));

    return false;
  }

  // Set number of channels
  result = snd_pcm_hw_params_set_channels(m_pPCMDevice, pHWParams, m_PlaybackFormat.Channels);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to set sound device output channels to %d.  Error [%s]", 
      m_PlaybackFormat.Channels, snd_strerror(result));
    return false;
  }


  // Set the output stream sample rate (frequency) or get one as close as possible
  unsigned int freq = m_PlaybackFormat.Freq;
  result = snd_pcm_hw_params_set_rate_near(m_pPCMDevice, pHWParams, &freq, 0);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to set sound device output frequency to %d Hz (or anything near).  Error [%s]", 
      m_PlaybackFormat.Freq, snd_strerror(result));
    return false;
  }
  if (freq != (unsigned int)m_PlaybackFormat.Freq)
  {
    RecordEvent(SSEL_WARNING, "Requested frequency of %d Hz unavailable.  Using %u hz.", 
      m_PlaybackFormat.Freq, freq);
  }

  // Setup the buffer for 1/5th of a second (us)
  unsigned int buffertime=200000; 
  result = snd_pcm_hw_params_set_buffer_time_near(m_pPCMDevice, pHWParams, &buffertime, 0);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to set sound device buffer duration to 1/10s (or anything near).  Error [%s]", 
            snd_strerror(result));
    return false;
  }
  
  result = snd_pcm_hw_params_get_buffer_size(pHWParams, &m_HWBufferBytes);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to retrieve sound device buffer size.  Error [%s]", 
      snd_strerror(result));
    return false;
  }
  // Translate from frames to bytes
  m_HWBufferBytes *= m_PlaybackFormat.Bits/8 * m_PlaybackFormat.Channels;
  RecordEvent(SSEL_DEBUG, "Sound device buffer duration set to %fs (%u bytes)",
    ((float)buffertime) / 1000000.0f, m_HWBufferBytes);

  // Set the period time near 1/10th of a second.  This sounds like it determines the amount of data that
  //  is sent to the underlying hardware in each operation.  The smaller this value the more overhead
  //  but the finer granularity on sample status.  This should be at largest 1/4 the size of the total sample
  //  buffer, since anything larger may result in under buffering.
  unsigned int periodtime=50000;
  result = snd_pcm_hw_params_set_period_time_near(m_pPCMDevice, pHWParams, &periodtime, 0);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to set sound device period time to .01s.  Error [%s]", 
      snd_strerror(result));
    return false;
  }

  // Send the parameters to the device
  result = snd_pcm_hw_params(m_pPCMDevice, pHWParams);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to apply hw sound parameters.  Error [%s]", 
      snd_strerror(result));
    return false;
  }
  return true;
}

bool SndSysDriverALSA::SetupSWParams()
{
  snd_pcm_sw_params_t *pSWParams;
  int result;

  // Allocate sofware parameter structure on the stack
  snd_pcm_sw_params_alloca(&pSWParams);


  // Retrieve all current parameters
  result = snd_pcm_sw_params_current(m_pPCMDevice, pSWParams);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to retrieve current sound software configuration parameters.  Error [%s]", 
      snd_strerror(result));
    return false;
  }

  
  /*
  // start the transfer when the buffer is almost full: 
  // (buffer_size / avail_min) * avail_min 
  err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
  if (err < 0) {
    printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
    return err;
  }
  // allow the transfer when at least period_size samples can be processed
  err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
  if (err < 0) {
    printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
    return err;
  }
  */

  // Align transfers to 1 sample
  result = snd_pcm_sw_params_set_xfer_align(m_pPCMDevice, pSWParams, 1);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to set sound software transfer to 1 sample.  Error [%s]", 
      snd_strerror(result));
    return false;
  }

  // Apply the parameters
  result = snd_pcm_sw_params(m_pPCMDevice, pSWParams);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to set sound software configuration parameters.  Error [%s]", 
      snd_strerror(result));
    return false;
  }
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
bool SndSysDriverALSA::Open (csSndSysRendererSoftware* pRenderer,
			     csSndSysSoundFormat *pRequestedFormat)
{
  int result;

  RecordEvent(SSEL_DEBUG, "ALSA Driver: Open()");
//  CS_ASSERT (Config != 0);

  // Copy the format into local storage
  memcpy(&m_PlaybackFormat, pRequestedFormat, sizeof(csSndSysSoundFormat));

  m_pAttachedRenderer=pRenderer;

  // Open the ALSA PCM device for playback
  if ((result = snd_pcm_open(&m_pPCMDevice, m_OutputDeviceName, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    RecordEvent(SSEL_ERROR, "Failed to open sound output device [%s] Error [%s]", 
      m_OutputDeviceName, snd_strerror(result));
    return false;
  }

  // Setup various parameters of how we interact with ALSA and how ALSA should interact with the device
  if (!SetupHWParams() || !SetupSWParams())
  {
    snd_pcm_close(m_pPCMDevice);
    return false;
  }

  result=snd_pcm_prepare(m_pPCMDevice);
  if (result < 0)
  {
    RecordEvent(SSEL_ERROR, "Failed to prepare sound output device for playback Error [%s]", 
            snd_strerror(result));
    snd_pcm_close(m_pPCMDevice);
    return false;
  }
 
  // Copy any changes back into the caller's buffer
  memcpy(pRequestedFormat, &m_PlaybackFormat, sizeof(csSndSysSoundFormat));
 
  return true;
}

void SndSysDriverALSA::Close ()
{
  if (m_pPCMDevice != 0)
  {
    snd_pcm_close(m_pPCMDevice);
    m_pPCMDevice = 0;
  }
}

bool SndSysDriverALSA::StartThread()
{
  if (m_Running) return false;

  m_Running=true;
  SndSysDriverRunnable* runnable = new SndSysDriverRunnable (this);
  m_pBGThread = csThread::Create(runnable);
  runnable->DecRef ();

  m_pBGThread->Start();
  
  return true;
}


void SndSysDriverALSA::StopThread()
{
  m_Running=false;
  csSleep(100);
}

void SndSysDriverRunnable::Run ()
{
  parent->Run ();
}

void SndSysDriverALSA::Run()
{
  int result;
  snd_pcm_state_t devicestate;

  m_BytesPerFrame = m_PlaybackFormat.Channels * m_PlaybackFormat.Bits/8;

  csTicks last_write, current_ticks, tick_difference;
  csTicks last_error_time=0;

  // Write at about 20 times per second
  tick_difference=50;

  last_write=csGetTicks();


  // Clear the buffer and write one section ahead
  ClearBuffer(tick_difference/1000 * m_PlaybackFormat.Freq * m_BytesPerFrame);
  //WriteBuffer(ALSA_buffer_bytes);
  
  snd_pcm_start(m_pPCMDevice);

  while (m_Running)
  {
    current_ticks=csGetTicks();
    if (last_write + tick_difference <= current_ticks)
    {
      bool UnderRunRecovery=false;

      // Check for underrun
      devicestate = snd_pcm_state(m_pPCMDevice);
      if (devicestate == SND_PCM_STATE_XRUN)
      {
        // Log an underbuffer error - only log once every 15 seconds
        if (last_error_time==0 || current_ticks-last_error_time > 15000)
        {
          RecordEvent(SSEL_WARNING, "Underbuffer detected on output device!");
          last_error_time=current_ticks;
        }

        // Try to recover from the underbuffer by re-preparing the output device
        result=snd_pcm_prepare(m_pPCMDevice);
        if (result<0)
        {
          RecordEvent(SSEL_CRITICAL, "Failed to recover from underbuffer.  Error [%s]", snd_strerror(result));
          break;
        }

        UnderRunRecovery=true;
      }

      snd_pcm_sframes_t AvailableFrames;
      snd_pcm_uframes_t MappedFrames, FilledFrames;
      MappedFrames=0;
      AvailableFrames=0;
      FilledFrames=0;

      // Continue mapping and filling buffers while:
      // 1) There are no errors 
      //   AND
      // 2) All available frames were not mapped
      //   AND
      // 3) All mapped frames were filled
      do 
      {
        result=FillBuffer(AvailableFrames, MappedFrames, FilledFrames);

        // Note that the comparison below fails horribly if you map over max signed int32 frames (~2 billion)
        //  If you are mapping 2 billion frames, something is horribly wrong anyway.
        if (result>=0 && (snd_pcm_sframes_t)MappedFrames<AvailableFrames)
          RecordEvent(SSEL_DEBUG, "Short mapping of %u frames [%d available]", MappedFrames, AvailableFrames);
      } while(result>=0 && (snd_pcm_sframes_t)MappedFrames<AvailableFrames && FilledFrames==MappedFrames);

      // If a mapping error occured, break
      if (result<0)
        break;

      // If an underrun occurred and we are recovering, we need to issue a 'start'
      if (UnderRunRecovery)
        snd_pcm_start(m_pPCMDevice);

      last_write=current_ticks;
    }

    RecordEvent(SSEL_DEBUG, "Finished filling mmap buffers.  Going to sleep.");

    // Sleep for at least a little bit
    csSleep(tick_difference);
  }
  RecordEvent(SSEL_DEBUG, "Main run loop complete.  Shutting down.");

  // Stop playback and drop all queued frames
  snd_pcm_drop(m_pPCMDevice);
}

size_t SndSysDriverALSA::ClearBuffer(size_t Bytes)
{
  if (m_pPCMDevice)
  {
    snd_pcm_sframes_t availableframes;
    snd_pcm_uframes_t mmap_offset, mmap_frames;
    const snd_pcm_channel_area_t *mmap_areas;
    int result;

    availableframes = snd_pcm_avail_update(m_pPCMDevice);
    if (availableframes <= 0)
      return 0;


    // Try to lock the mmap buffer for writing
    mmap_frames=availableframes;
    result = snd_pcm_mmap_begin(m_pPCMDevice, &mmap_areas, &mmap_offset, &mmap_frames);
    if (result < 0)
    {
      RecordEvent(SSEL_CRITICAL, "Error mmaping PCM buffer from sound device! Error [%s]", snd_strerror(result));
      return 0;
    }

    // Fill the buffer with silence
    if (m_PlaybackFormat.Bits==8)
    {
      // For 8 bit samples, fill with 128
      memset(((unsigned char *)mmap_areas[0].addr) + mmap_offset * mmap_areas[0].step/8, 128,  mmap_frames * m_PlaybackFormat.Channels * m_PlaybackFormat.Bits/8);
    }
    else
    {
      // For 16 bit samples, fill with 0
      memset(((unsigned char *)mmap_areas[0].addr) + mmap_offset * mmap_areas[0].step/8, 0,  mmap_frames * m_PlaybackFormat.Channels * m_PlaybackFormat.Bits/8);
    }

    result = snd_pcm_mmap_commit(m_pPCMDevice, mmap_offset, mmap_frames);
    if (result < 0)
    {
      RecordEvent(SSEL_CRITICAL, "Error committing PCM buffer to sound device! Error [%s]", snd_strerror(result));
      return 0;
    }
  }
  return 0;
}

int SndSysDriverALSA::FillBuffer(snd_pcm_sframes_t& AvailableFrames, snd_pcm_uframes_t& MappedFrames, snd_pcm_uframes_t& FilledFrames)
{
  snd_pcm_uframes_t mmap_offset;
  const snd_pcm_channel_area_t *mmap_areas;
  int result;


  AvailableFrames = snd_pcm_avail_update(m_pPCMDevice);
  if (AvailableFrames <= 0)
    return 0;

  RecordEvent(SSEL_DEBUG, "%d available frames in %d bytes", AvailableFrames, AvailableFrames * m_BytesPerFrame);

  // Try to lock the mmap buffer for writing
  MappedFrames=AvailableFrames;
  mmap_offset=0;
  result = snd_pcm_mmap_begin(m_pPCMDevice, &mmap_areas, &mmap_offset, &MappedFrames);
  if (result < 0)
  {
    RecordEvent(SSEL_CRITICAL, "Error mmaping PCM buffer from sound device! Error [%s]", snd_strerror(result));
    return -1;
  }

  RecordEvent(SSEL_DEBUG, "Mapped %u bytes", MappedFrames * m_BytesPerFrame);

  uint32 bytes_used=m_pAttachedRenderer->FillDriverBuffer(((unsigned char *)mmap_areas[0].addr) + mmap_offset * mmap_areas[0].step/8,
                                                          MappedFrames * m_PlaybackFormat.Channels * m_PlaybackFormat.Bits/8, 0, 0);

  RecordEvent(SSEL_DEBUG, "FillDriverBuffer() filled %u bytes", bytes_used);
  FilledFrames = bytes_used / m_BytesPerFrame;

  result = snd_pcm_mmap_commit(m_pPCMDevice, mmap_offset, FilledFrames);
  if (result < 0)
  {
    RecordEvent(SSEL_CRITICAL, "Error committing PCM buffer to sound device! Error [%s]", snd_strerror(result));
    return -1;
  }
  return 0;
}

}
CS_PLUGIN_NAMESPACE_END(SndSysALSA)

