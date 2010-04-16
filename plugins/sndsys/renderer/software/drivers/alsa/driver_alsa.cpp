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




CS_PLUGIN_NAMESPACE_BEGIN(SndSysALSA)
{

SCF_IMPLEMENT_FACTORY (SndSysDriverALSA)


SndSysDriverALSA::SndSysDriverALSA(iBase* pParent) :
  scfImplementationType(this, pParent),
  m_pObjectReg(0), m_pPCMDevice(0), m_bRunning(false), m_UnderBufferCount(0)
{
}


SndSysDriverALSA::~SndSysDriverALSA()
{
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
  csRef<iCommandLineParser> CMDLine (
    csQueryRegistry<iCommandLineParser> (m_pObjectReg));
  const char *OutputDeviceName = CMDLine->GetOption ("alsadevice");
  if (!OutputDeviceName)
    OutputDeviceName = Config->GetStr("SndSys.Driver.ALSA.Device", "default");

  // Copy into the used buffer
  strcpy(m_OutputDeviceName, OutputDeviceName);


  m_BufferLengthms=0;
  if (CMDLine)
  {
    const char *BufferLengthStr = CMDLine->GetOption("soundbufferms");
    if (BufferLengthStr) m_BufferLengthms=atoi(BufferLengthStr);
  }

  // Check for sound config file option. Default to 20 ms if no option is found.
  if (m_BufferLengthms<=0)
    m_BufferLengthms = Config->GetInt("SndSys.Driver.ALSA.SoundBufferms", 20);

  // The number of underbuffer events before the buffer size is automatically increased
  m_UnderBuffersAllowed=5;

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
  // Ignore GCC warning(s) on systems with alsa pre 1.0.16
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

  // The number of frames we want in the buffer
  snd_pcm_uframes_t WantedFrames = (m_BufferLengthms * freq / 1000);

  // Setup the buffer for the requested time length (us)
  unsigned int buffertime=m_BufferLengthms * 1000; 
  result = snd_pcm_hw_params_set_buffer_time_near(m_pPCMDevice, pHWParams, &buffertime, 0);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to set sound device buffer duration to %d ms (or anything near).  Error [%s]", 
            m_BufferLengthms, snd_strerror(result));
    return false;
  }
  
  result = snd_pcm_hw_params_get_buffer_size(pHWParams, &m_HWBufferFrames);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to retrieve sound device buffer size.  Error [%s]", 
      snd_strerror(result));
    return false;
  }
  RecordEvent(SSEL_DEBUG, "Sound device buffer duration set to %d ms (%u frames).  Requested %d ms.",
    buffertime / 1000, m_HWBufferFrames, m_BufferLengthms);

  // Calculate how many frames we wont use in the ALSA buffer so
  //   that we have an acceptably low latency
  m_HWUnusableFrames=0;
  if (m_HWBufferFrames > WantedFrames)
  {
    m_HWUnusableFrames=m_HWBufferFrames - WantedFrames;
    RecordEvent(SSEL_DEBUG, "Received excess frame space in requested buffer. Not using %d frames.",
      m_HWUnusableFrames);
  }

  // Set the period time to about 1/4 of the buffer.  This sounds like it determines the amount of data that
  //  is sent to the underlying hardware in each operation.  The smaller this value the more overhead
  //  but the finer granularity on sample status.  This should be at largest 1/4 the size of the total sample
  //  buffer, since anything larger may result in under buffering.
  unsigned int periodtime=m_BufferLengthms * 1000 / 4;
  result = snd_pcm_hw_params_set_period_time_near(m_pPCMDevice, pHWParams, &periodtime, 0);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to set sound device period time to %d ms.  Error [%s]", 
      m_BufferLengthms, snd_strerror(result));
    return false;
  }

  // The effective period time may require us to reduce our unused frame count
  if (periodtime > (m_BufferLengthms * 1000 / 4))
  {
    // Calculate our actual buffer time to 4x the period time
    m_BufferLengthms = 4 * periodtime / 1000;

    // If that ends up being the same size as our buffer, or larger, then just use the whole buffer
    if (m_HWBufferFrames <= (m_BufferLengthms * freq / 1000))
      m_HWUnusableFrames=0;
    else
      m_HWUnusableFrames = m_HWBufferFrames - (m_BufferLengthms * freq / 1000);

    // Log it
    RecordEvent(SSEL_DEBUG, "Received larger than desired period time of %d ms. Updated unused frames to %d.",
      periodtime / 1000, m_HWUnusableFrames);
  }

  // Send the parameters to the device
  result = snd_pcm_hw_params(m_pPCMDevice, pHWParams);
  if (result < 0) 
  {
    RecordEvent(SSEL_ERROR, "Failed to apply hw sound parameters.  Error [%s]", 
      snd_strerror(result));
    return false;
  }

  // Log a message indicating the latency we end up with
  RecordEvent(SSEL_DEBUG, "Final maximum driver latency of %d ms (%d frames).",
    m_BufferLengthms, (m_HWBufferFrames - m_HWUnusableFrames));

  // Store the actual frequency back into our format data
  m_PlaybackFormat.Freq = freq;

  // Set the minimum number of frames it's worthwhile to fill to 1/4 of the buffer size
  m_SoundBufferMinimumFillFrames=(m_HWBufferFrames - m_HWUnusableFrames)/4;

  return true;
}

bool SndSysDriverALSA::SetupSWParams()
{
  snd_pcm_sw_params_t *pSWParams;
  int result;

  // Allocate sofware parameter structure on the stack
  snd_pcm_sw_params_alloca(&pSWParams);


  // Retrieve all current parameters
  // Ignore GCC warning(s) on systems with alsa pre 1.0.16
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
    Close();
    return false;
  }

  result=snd_pcm_prepare(m_pPCMDevice);
  if (result < 0)
  {
    RecordEvent(SSEL_ERROR, "Failed to prepare sound output device for playback Error [%s]", 
            snd_strerror(result));
    Close();
    return false;
  }
 
  // Copy any changes back into the caller's buffer
  memcpy(pRequestedFormat, &m_PlaybackFormat, sizeof(csSndSysSoundFormat));

  // Store the bytes per frame for later output calculations
  m_BytesPerFrame = m_PlaybackFormat.Channels * m_PlaybackFormat.Bits/8;

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
  if (m_bRunning) return false;

  m_bRunning=true;
  SndSysDriverRunnable* runnable = new SndSysDriverRunnable (this);
  m_pBGThread.AttachNew (new CS::Threading::Thread (runnable, false));  
  runnable->DecRef ();

  m_pBGThread->Start();
  
  return true;
}


void SndSysDriverALSA::StopThread()
{
  m_bRunning=false;
  csSleep(100);
}

void SndSysDriverRunnable::Run ()
{
  m_pParent->Run ();
}

void SndSysDriverALSA::Run()
{
  int result;
  

  // Fill the buffer initially.  This will pull silence from the renderer
  result=FillBuffer(m_HWBufferFrames - m_HWUnusableFrames);
  if (result<0)
  {
    RecordEvent(SSEL_CRITICAL, "Failed to fill initial sound buffer.  Error [%s].  Aborting runloop.", snd_strerror(result));

    // Stop playback and drop all queued frames
    snd_pcm_drop(m_pPCMDevice);

    m_bRunning=false;
    return;
  }

  // Start the sound output device
  result=snd_pcm_start(m_pPCMDevice);
  if (result < 0)
  {
    RecordEvent(SSEL_CRITICAL, "Failed to start sound device. Error [%s]. Aborting runloop.", snd_strerror(result));

    // Stop playback and drop all queued frames
    snd_pcm_drop(m_pPCMDevice);
    return;
  }

  // The main loop of the background thread
  while (m_bRunning)
  {
    // Retrieve the number of available frames, and only update if there's enough
    //  to make it worthwhile
    snd_pcm_sframes_t AvailableFrames = snd_pcm_avail_update(m_pPCMDevice);

    if (HasUnderbuffered())
    {
      // Handle/recover from the underbuffer condition
      if (!HandleUnderbuffer())
      {
        // Failed to handle underbuffer condition
        RecordEvent(SSEL_CRITICAL, "Failed to recover from underbuffer condition! Aborting runloop.");
        break;
      }
      continue;
    }

    // An error code was returned
    if (AvailableFrames<0)
    {
      if (HandleALSAError(AvailableFrames))
        continue;

      // Unhandled error
      RecordEvent(SSEL_CRITICAL, "Aborting runloop.");
      break;
    }

    if ((AvailableFrames - m_HWUnusableFrames) >= m_SoundBufferMinimumFillFrames)
    {
      // Log our fill
      RecordEvent(SSEL_DEBUG, "Filling [%d] available frames for [%d] total bytes", 
        (AvailableFrames - m_HWUnusableFrames), (AvailableFrames - m_HWUnusableFrames) * m_BytesPerFrame);

      // Continue mapping and filling buffers while:
      // 1) There are no errors 
      //   AND
      // 2) There are at least enough available frames to be worth filling
      do
      {
        result=FillBuffer(AvailableFrames - m_HWUnusableFrames);

        // If no error occurred, re-read the number of available frames
        if (result>0)
          AvailableFrames = snd_pcm_avail_update(m_pPCMDevice);
      } while(result>0 && ((AvailableFrames - m_HWUnusableFrames) >= m_SoundBufferMinimumFillFrames));

      // If a mapping error occurred, break
      if (result<0)
        break;
    }
    else
    {
      RecordEvent(SSEL_DEBUG, "%d of %d required minimum frames ready for fill. Going to sleep for %d ms.",
        (AvailableFrames - m_HWUnusableFrames), m_SoundBufferMinimumFillFrames, m_BufferLengthms/4);

      // Sleep for a little bit
      csSleep(m_BufferLengthms/4);
    }

  }
  RecordEvent(SSEL_DEBUG, "Main run loop complete.  Shutting down.");

  // Stop playback and drop all queued frames
  snd_pcm_drop(m_pPCMDevice);
}


snd_pcm_sframes_t SndSysDriverALSA::FillBuffer(snd_pcm_sframes_t AvailableFrames)
{
  snd_pcm_uframes_t MappedFrames, FilledFrames;
  snd_pcm_uframes_t mmap_offset;
  const snd_pcm_channel_area_t *mmap_areas;
  int result;

  // Initialize these values
  MappedFrames=0;
  FilledFrames=0;

  // Try to lock all available frames in the mmap buffer for writing
  MappedFrames=AvailableFrames;

  do
  {
    mmap_offset=0;
    result = snd_pcm_mmap_begin(m_pPCMDevice, &mmap_areas, &mmap_offset, &MappedFrames);
    if (result >= 0)
      break;
    RecordEvent(SSEL_WARNING, "Error mmaping PCM buffer from sound device.");
  } while (HandleALSAError(result));

  // Log the number of frames we did map
  RecordEvent(SSEL_DEBUG, "Mapped %u frames", MappedFrames);

  // Call the renderer to fill in the frame data
  //  This is somewhat of a hack.  I think technically the mmaped areas can have
  //  a different stride than we presume here.  In reality it probably never happens.
  FilledFrames = m_pAttachedRenderer->FillDriverBuffer(((unsigned char *)mmap_areas[0].addr) + mmap_offset * mmap_areas[0].step/8,
                                                          MappedFrames , 0, 0);

  // Log the number of frames the renderer filled (should be all of them)
  RecordEvent(SSEL_DEBUG, "FillDriverBuffer() filled %u frames", FilledFrames);

  do
  {
    // Commit the written frames to the ALSA layer
    result = snd_pcm_mmap_commit(m_pPCMDevice, mmap_offset, FilledFrames);
    if (result >= 0)
      break;
    RecordEvent(SSEL_WARNING, "Error committing PCM buffer to sound device.");
  } while (HandleALSAError(result));

  return FilledFrames;
}

bool SndSysDriverALSA::HasUnderbuffered()
{
  snd_pcm_state_t devicestate = snd_pcm_state(m_pPCMDevice);
  if (devicestate == SND_PCM_STATE_XRUN)
  {
    // Log an underbuffer error
    RecordEvent(SSEL_WARNING, "Underbuffer detected on output device!");
    return true;
  }
  return false;
}

bool SndSysDriverALSA::HandleUnderbuffer()
{
  // Try corrective action if warranted
  if (NeedUnderbufferCorrection() && !CorrectUnderbuffer())
    return false;

  // Refill the buffer.
  int result=FillBuffer(m_HWBufferFrames - m_HWUnusableFrames);
  if (result<0)
  {
    RecordEvent(SSEL_CRITICAL, "Failed to refill sound buffer. Aborting runloop.");
    return false;
  }

  // Try to restart the sound device and handle errors as they come up
  do
  {
    // Restart the sound output
    result=snd_pcm_start(m_pPCMDevice);
    if (result>=0)
      break;
    RecordEvent(SSEL_ERROR, "Failed to restart sound device.");
  } while (HandleALSAError(result));

  // Done
  return true;
}

bool SndSysDriverALSA::NeedUnderbufferCorrection()
{
  // Add one to our under buffer counter
  m_UnderBufferCount++;

  RecordEvent(SSEL_WARNING, "Underbuffer condition detected. Buffer length [%u] Count [%d].",
              m_BufferLengthms, m_UnderBufferCount); 

  // If we haven't exceeded the allowed threshold, continue
  if (m_UnderBufferCount <= m_UnderBuffersAllowed)
    return false;

  return true;
}


bool SndSysDriverALSA::CorrectUnderbuffer()
{
  if (m_BufferLengthms >= 1000)
  {
    RecordEvent(SSEL_ERROR, "Sound buffer is already at [%d ms] (>= 1s). Will not grow further.", 
      m_BufferLengthms);
    return true;
  }

  // Reset the count
  m_UnderBufferCount=0;
  
  
  RecordEvent(SSEL_ERROR, "Corrective underbuffering protection doubling buffer size. Current [%d ms]  New [%d ms].", 
    m_BufferLengthms, m_BufferLengthms*2);

  // We have exceeded the threshold, time to enlarge our underlying buffer
  m_BufferLengthms *= 2;

  // If our buffer is already too big, we can just reduce the unusable frame count
  //  such that the effective buffer size is doubled

  // We need the same amount of frames as we're already using in the buffer
  //  in order to double it.
  snd_pcm_sframes_t NeededFrameGrowth=m_HWBufferFrames - m_HWUnusableFrames;
  if (m_HWUnusableFrames >= NeededFrameGrowth)
  {
    RecordEvent(SSEL_DEBUG, "System provided buffer is already sufficiently large."
                            " Reducing unusable frames from [%d] to [%d].",
                            m_HWUnusableFrames, m_HWUnusableFrames-NeededFrameGrowth);
    m_HWUnusableFrames-=NeededFrameGrowth;
  }
  else
  {
      // Otherwise, fall back and enlarge the real buffer
    if (!SetupHWParams())
    {
      RecordEvent(SSEL_ERROR, "Failed to enlarge sound buffer to [%d ms].", m_BufferLengthms);
      return false;
    }
  }

  return true;
}

bool SndSysDriverALSA::HandleALSAError(int ErrorNumber)
{
  int result;

  switch((-ErrorNumber))
  {
    case EBADFD:
      RecordEvent(SSEL_WARNING, "Error [%s] (%d). This error sometimes occurs during extremely high load (such as running under valgrind)."
                                , snd_strerror(ErrorNumber), ErrorNumber);
     // This appears to be documented in bugs 1716, 1900 and 1975 filed against ALSA in early 2005
     //  It may just be poorly documented 'expected' functionality.  To recover, we need to re-prepare
     //  the device.
      result=snd_pcm_prepare(m_pPCMDevice);
      if (result < 0)
      {
        RecordEvent(SSEL_ERROR, "Failed to prepare sound output device for playback Error [%s (%d)]", 
                    snd_strerror(result), result);
        snd_pcm_close(m_pPCMDevice);
        return false;
      }
      return true;
    break;
    default:
      RecordEvent(SSEL_CRITICAL, "Unhandled ALSA error [%s (%d)].", snd_strerror(ErrorNumber), ErrorNumber);
    return false;
  }
}


}
CS_PLUGIN_NAMESPACE_END(SndSysALSA)

