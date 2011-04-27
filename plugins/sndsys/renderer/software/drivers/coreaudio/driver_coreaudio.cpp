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

#include "cssysdef.h"

#include "csutil/event.h"
#include "csutil/sysfunc.h"
#include "iutil/cfgfile.h"
#include "iutil/cmdline.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"

#include "../../renderer.h"
#include "isndsys/ss_driver.h"
#include "isndsys/ss_renderer.h"
#include "driver_coreaudio.h"

#if defined(MAC_OS_X_VERSION_10_5) && (MAC_OS_X_VERSION_MIN_REQUIRED>=MAC_OS_X_VERSION_10_5)
#define CS_AUDIO_CREATE_PROC(DEV,PROC,DATA) AudioDeviceCreateIOProcID(DEV,PROC,DATA,0)
#define CS_AUDIO_DESTROY_PROC(DEV,PROC) AudioDeviceDestroyIOProcID(DEV,PROC)
#else
#define CS_AUDIO_CREATE_PROC(DEV,PROC,DATA) AudioDeviceAddIOProc(DEV,PROC,DATA)
#define CS_AUDIO_DESTROY_PROC(DEV,PROC) AudioDeviceRemoveIOProc(DEV,PROC)
#endif


CS_PLUGIN_NAMESPACE_BEGIN(SndSysCOREAUDIO)
{

SCF_IMPLEMENT_FACTORY (csSndSysDriverCoreAudio)


// CoreAudio static IO procedure wrapper
static OSStatus StaticAudioProc(AudioDeviceID inDevice,
				const AudioTimeStamp *inNow,
				const AudioBufferList *inInputData,
				const AudioTimeStamp *inInputTime,
				AudioBufferList *outOutputData,
				const AudioTimeStamp *inOutputTime,
				void *inClientData)
{
  csSndSysDriverCoreAudio *p_audio=(csSndSysDriverCoreAudio *)inClientData;
  
  return p_audio->AudioProc(inDevice, inNow, inInputData, inInputTime,
			    outOutputData, inOutputTime);

}

csSndSysDriverCoreAudio::csSndSysDriverCoreAudio(iBase* piBase) :
  scfImplementationType (this, piBase), object_reg(0), attached_renderer(0),
  running(false)
{
  convert_buffer = 0;
}

csSndSysDriverCoreAudio::~csSndSysDriverCoreAudio()
{
  cs_free(convert_buffer);
}

void csSndSysDriverCoreAudio::Report(int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csReportV (object_reg, severity,
      "crystalspace.sndsys.driver.software.coreaudio", msg, arg);
  va_end (arg);
}

bool csSndSysDriverCoreAudio::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  Report (CS_REPORTER_SEVERITY_DEBUG,
    "Sound System: CoreAudio driver for software sound renderer initialized.");
  return true;
}

// Takes a 32-bit integer and breaks it into chars
#define SPLIT_TO_CHARS(x)						\
  ((x) >> 24), (((x) >> 16) & 0xff), (((x) >> 8) & 0xff), ((x) & 0xff)

bool csSndSysDriverCoreAudio::Open (csSndSysRendererSoftware *renderer,
				   csSndSysSoundFormat *requested_format)
{
  uint32 propertysize;
  
  //target format that we get from Core Audio
  AudioStreamBasicDescription outStreamDesc;
  
  //description of the data that CS will be providing
  AudioStreamBasicDescription inStreamDesc;
 
  OSStatus status;
  
  Report(CS_REPORTER_SEVERITY_DEBUG, "Sound System: CoreAudio Driver: Open()");

  attached_renderer = renderer;

  // Retrieve the output device ID - this is almost verbatim from the available
  // sample code
  propertysize = sizeof(outputDeviceID);
  status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,
				    (UInt32*)&propertysize, &outputDeviceID);
  if (status) 
  {
    Report(CS_REPORTER_SEVERITY_ERROR,
	   "Failed to obtain default CoreAudio output device.  Return of %d.",
	   (int)status);
    return false;
  }
  if (outputDeviceID == kAudioDeviceUnknown) 
  {
    Report(CS_REPORTER_SEVERITY_ERROR,
	   "Failed to obtain default CoreAudio output device.  "
	   "Resulting ID is kAudioDeviceUnknown.");
    return false;
  }

  // Set buffer size - 1/10th of a second buffer of floats
  propertysize = sizeof(convert_size);
  convert_size = requested_format->Freq / 10; //Number of sound *frames*
	
  status = AudioDeviceSetProperty(outputDeviceID, 0, 0, false,
       		      kAudioDevicePropertyBufferFrameSize, propertysize, &convert_size);
  if (status)
  {
    // Try to find a smaller buffer size which can successfully be used. This is due to the problem of
    // some hardware devices don't having the initially requested buffer size.
    // ToDo: obtain maximal buffer size from the device directly.
    while(status && (convert_size > 0))
    {
      --convert_size;
      status = AudioDeviceSetProperty(outputDeviceID, 0, 0, false,
		  kAudioDevicePropertyBufferFrameSize, propertysize, &convert_size);
    }
    if( status )
    {
      Report(CS_REPORTER_SEVERITY_ERROR, "Failed to set sound buffer");
      return false;
    }
    Report(CS_REPORTER_SEVERITY_WARNING, "Successfully set sound buffer to: %d frames, ",convert_size);
  }

  convert_buffer = cs_malloc(convert_size * requested_format->Channels * requested_format->Bits/8);
  
  // Get stream information
  propertysize = sizeof(outStreamDesc);
  status = AudioDeviceGetProperty(outputDeviceID, 0, false,
    kAudioDevicePropertyStreamFormat, (UInt32*)&propertysize, &outStreamDesc);
  if (status != 0)
  {
    Report(CS_REPORTER_SEVERITY_ERROR,
	   "Failed to retrieve output stream description from CoreAudio "
	   "output device.  Return of %d.", (int)status);
    return false;
  }
  //get another copy to create the input format spec
  status = AudioDeviceGetProperty(outputDeviceID, 0, false,
    kAudioDevicePropertyStreamFormat, (UInt32*)&propertysize, &inStreamDesc);
	   
  // Set up source stream description for an AudioConverter to do its thang
 
  inStreamDesc.mSampleRate = requested_format->Freq;
  inStreamDesc.mFormatID = kAudioFormatLinearPCM;
  inStreamDesc.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked ;
  if ((requested_format->Flags & CSSNDSYS_SAMPLE_ENDIAN_MASK) == CSSNDSYS_SAMPLE_BIG_ENDIAN)
    inStreamDesc.mFormatFlags |= kAudioFormatFlagIsBigEndian;
  inStreamDesc.mChannelsPerFrame = requested_format->Channels;
  inStreamDesc.mBitsPerChannel = requested_format->Bits;
  inStreamDesc.mFramesPerPacket = 1;
  inStreamDesc.mBytesPerFrame = requested_format->Channels * requested_format->Bits / 8;
  // The bytes per packet is purely a product of bytes per frame and frames per
  // packet since we don't have any padding in a packet here
  inStreamDesc.mBytesPerPacket =
    inStreamDesc.mFramesPerPacket * inStreamDesc.mBytesPerFrame;
  
  //Create a converter to do the translation for us
  status = AudioConverterNew(&inStreamDesc, &outStreamDesc, &converter);
  if (status != 0)
  {
    Report(CS_REPORTER_SEVERITY_ERROR,
      "Failed to create converter with error %c%c%c%c",
      SPLIT_TO_CHARS (status));
    //return false;
  }
 
  // Copy the final format into local storage
  memcpy(&playback_format, requested_format, sizeof(csSndSysSoundFormat));

  // Add a callback and begin playback
  status = CS_AUDIO_CREATE_PROC(outputDeviceID, StaticAudioProc, this);
  if (status != 0)
  {
    Report(CS_REPORTER_SEVERITY_ERROR,
	   "Failed to add audio device IO proc.  Return of %d.", (int)status);
    return false;
  }
 
  
  return true;
}

void csSndSysDriverCoreAudio::Close ()
{
  StopThread();
  CS_AUDIO_DESTROY_PROC(outputDeviceID, StaticAudioProc);
  cs_free(convert_buffer);
  convert_buffer = 0;
}

bool csSndSysDriverCoreAudio::StartThread()
{
  OSStatus status;
  // Since the Core Audio API is callback driven, we don't actually start a
  // thread here ourselves.  Instead we start the audio device pulling from the
  // audio procedure.  This runs in its own thread that we don't see.
  Report(CS_REPORTER_SEVERITY_DEBUG, "Getting our thread on!");
  status = AudioDeviceStart(outputDeviceID, StaticAudioProc);
  if (status != 0)
  {
    Report(CS_REPORTER_SEVERITY_ERROR, "Failed to start audio device IO proc. "
	   "Return of %d.", (int)status);
    return false;
  }
  running = true;
  return true;
}

void csSndSysDriverCoreAudio::StopThread()
{
  running = false;
  AudioDeviceStop(outputDeviceID, StaticAudioProc);
  csSleep(100);
}


OSStatus csSndSysDriverCoreAudio::AudioProc(AudioDeviceID inDevice,
					  const AudioTimeStamp *inNow,
					  const AudioBufferList *inInputData,
					  const AudioTimeStamp *inInputTime,
					  AudioBufferList *outOutputData,
					  const AudioTimeStamp *inOutputTime)
{
  if (!running)
  {
    outOutputData->mBuffers[0].mDataByteSize=0;
    return 0;
  }

  // Fill the provided buffer with as many samples as possible and return the
  // number of frames provided
    OSStatus stat = 0;
    size_t framesFilled = attached_renderer->FillDriverBuffer( convert_buffer, convert_size, 0, 0);
    UInt32 bytesWritten = outOutputData->mBuffers[0].mDataByteSize;
    stat = AudioConverterConvertBuffer(converter, framesFilled * playback_format.Bits/8 * playback_format.Channels, convert_buffer, &bytesWritten, outOutputData->mBuffers[0].mData); 
    if(stat){
	  //attached_renderer->RecordEvent(SSEC_DRIVER,SSEL_DEBUG, "Audio convert error %c%c%c%c", SPLIT_TO_CHARS(stat));
	}
	//attached_renderer->RecordEvent(SSEC_DRIVER,SSEL_DEBUG, "CA requested %d bytes, we wrote %d.",outOutputData->mBuffers[0].mDataByteSize,bytesWritten);
  return stat;
}

}
CS_PLUGIN_NAMESPACE_END(SndSysCOREAUDIO)

