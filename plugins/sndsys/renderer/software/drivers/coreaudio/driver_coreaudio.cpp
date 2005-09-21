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

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (SndSysDriverCoreAudio)

SCF_IMPLEMENT_IBASE(SndSysDriverCoreAudio)
SCF_IMPLEMENTS_INTERFACE(iSndSysSoftwareDriver)
SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (SndSysDriverCoreAudio::eiComponent)
SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

// The system driver.
iObjectRegistry *SndSysDriverCoreAudio::object_reg = 0;

// The loaded CS reporter
csRef<iReporter> SndSysDriverCoreAudio::reporter;


// CoreAudio static IO procedure wrapper
static OSStatus StaticAudioProc(AudioDeviceID inDevice,
				const AudioTimeStamp *inNow,
				const AudioBufferList *inInputData,
				const AudioTimeStamp *inInputTime,
				AudioBufferList *outOutputData,
				const AudioTimeStamp *inOutputTime,
				void *inClientData)
{
  SndSysDriverCoreAudio *p_audio=(SndSysDriverCoreAudio *)inClientData;
  return p_audio->AudioProc(inDevice, inNow, inInputData, inInputTime,
			    outOutputData, inOutputTime);
}

SndSysDriverCoreAudio::SndSysDriverCoreAudio(iBase* piBase) :
  attached_renderer(0), running(false)
{
  SCF_CONSTRUCT_IBASE(piBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  object_reg = 0;
}

SndSysDriverCoreAudio::~SndSysDriverCoreAudio()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

void SndSysDriverCoreAudio::Report(int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);

  if (!reporter)
    reporter = CS_QUERY_REGISTRY(object_reg, iReporter);

  if (reporter)
  {
    reporter->ReportV (severity,
      "crystalspace.sndsys.driver.software.coreaudio", msg, arg);
  }
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

bool SndSysDriverCoreAudio::Initialize (iObjectRegistry* r)
{
  object_reg = r;
  Report (CS_REPORTER_SEVERITY_DEBUG,
    "Sound System: CoreAudio driver for software sound renderer initialized.");
  return true;
}

bool SndSysDriverCoreAudio::Open (SndSysRendererSoftware *renderer,
				  SndSysSoundFormat *requested_format)
{
  uint32 propertysize, buffersize;
  AudioStreamBasicDescription outStreamDesc;
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

  // Set buffer size - attempting to use signed 16 bit samples here using
  // 1/20th of a second buffer size
  propertysize = sizeof(buffersize);
  buffersize = requested_format->Freq * (requested_format->Bits/8) *
    requested_format->Channels / 20;
  status = AudioDeviceSetProperty(outputDeviceID, 0, 0, false,
    kAudioDevicePropertyBufferSize, propertysize, &buffersize);
  if (status)
  {
    Report(CS_REPORTER_SEVERITY_ERROR,
	   "Failed to set buffersize to %d bytes for CoreAudio output device. "
	   "Return of %d.", buffersize, (int)status);
    return false;
  }

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

  // Modify stream information to our desired format
  outStreamDesc.mSampleRate=requested_format->Freq;
  outStreamDesc.mFormatID=kAudioFormatLinearPCM;
  // This is where we set the output to signed integer output (not float),
  // little endian
  outStreamDesc.mFormatFlags =
    kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
  outStreamDesc.mChannelsPerFrame=requested_format->Channels;
  outStreamDesc.mBitsPerChannel=requested_format->Bits;
  // Frames per packet are the number of full sets of sound samples (one for
  // each channel) encoded in a single logical unit of the source data.  In
  // compressed formats there may be many frames encoded in each logical
  // packet.  In uncompressed formats (like linear PCM) this is always 1.
  outStreamDesc.mFramesPerPacket=1;
  // The bytes per frame of a packed data stream will be channels * bits per
  // sample / 8
  outStreamDesc.mBytesPerFrame = 
    requested_format->Channels * requested_format->Bits/8;
  // The bytes per packet is purely a product of bytes per frame and frames per
  // packet since we don't have any padding in a packet here
  outStreamDesc.mBytesPerPacket =
    outStreamDesc.mFramesPerPacket * outStreamDesc.mBytesPerFrame;

  propertysize = sizeof(outStreamDesc);
  status = AudioDeviceSetProperty(outputDeviceID, 0, 0, false,
    kAudioDevicePropertyStreamFormat, propertysize, &outStreamDesc);
  if (status != 0)
  {
    Report(CS_REPORTER_SEVERITY_ERROR,
	   "Failed to set output stream properties to Freq=%d Channels=%d "
	   "Bits=%d .  Return of %d.", requested_format->Freq,
	   requested_format->Channels, requested_format->Bits, (int)status);
    return false;
  }

  // Copy the final format into local storage
  memcpy(&playback_format, requested_format, sizeof(SndSysSoundFormat));

  // Add a callback and begin playback
  status = AudioDeviceAddIOProc(outputDeviceID, StaticAudioProc, this);
  if (status != 0)
  {
    Report(CS_REPORTER_SEVERITY_ERROR,
	   "Failed to add audio device IO proc.  Return of %d.", (int)status);
    return false;
  }

  return true;
}

void SndSysDriverCoreAudio::Close ()
{
  StopThread();
  AudioDeviceRemoveIOProc(outputDeviceID, StaticAudioProc);
}

bool SndSysDriverCoreAudio::StartThread()
{
  OSStatus status;
  // Since the Core Audio API is callback driven, we don't actually start a
  // thread here ourselves.  Instead we start the audio device pulling from the
  // audio procedure.  This runs in its own thread that we don't see.
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

void SndSysDriverCoreAudio::StopThread()
{
  running = false;
  AudioDeviceStop(outputDeviceID, StaticAudioProc);
  csSleep(100);
}


OSStatus SndSysDriverCoreAudio::AudioProc(AudioDeviceID inDevice,
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
  // number of bytes provided
  outOutputData->mBuffers[0].mDataByteSize=attached_renderer->FillDriverBuffer(
    outOutputData->mBuffers[0].mData,
    outOutputData->mBuffers[0].mDataByteSize,0, 0);
  return 0;
}
