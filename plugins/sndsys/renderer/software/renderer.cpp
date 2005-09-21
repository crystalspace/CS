/*
	Copyright (C) 2004 by Andrew Mann

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

#include "isndsys/ss_structs.h"

#include "csutil/sysfunc.h"
#include "csutil/event.h"
#include "csutil/csendian.h"

#include "iutil/plugin.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "isndsys/ss_driver.h"
#include "isndsys/ss_data.h"
#include "isndsys/ss_stream.h"
#include "isndsys/ss_listener.h"
#include "isndsys/ss_renderer.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "iutil/cmdline.h"
#include "ivaria/reporter.h"

#include "listener.h"
#include "source.h"
#include "renderer.h"


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (SndSysRendererSoftware)


SCF_IMPLEMENT_IBASE(SndSysRendererSoftware)
	SCF_IMPLEMENTS_INTERFACE(iSndSysRenderer)
	SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (SndSysRendererSoftware::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (SndSysRendererSoftware::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

// The system driver.
iObjectRegistry *SndSysRendererSoftware::object_reg=NULL;

// The loaded CS reporter
csRef<iReporter> SndSysRendererSoftware::reporter;


SndSysRendererSoftware::SndSysRendererSoftware(iBase* piBase) :
sample_buffer(NULL), sample_buffer_samples(0), last_intensity_multiplier(0)
{
  SCF_CONSTRUCT_IBASE(piBase);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  scfiEventHandler = 0;
  object_reg = 0;
  Volume=0.5;

  int i;
  for (i=0;i<MAX_CHANNELS;i++)
    memset(&Speakers[i],0,sizeof(st_speaker_properties));

  Speakers[0].RelativePosition.x=-0.1f;
  Speakers[0].RelativePosition.y=0.02f;
  Speakers[0].Direction.x=-1.0f;
  Speakers[1].RelativePosition.x=0.1f;
  Speakers[1].RelativePosition.y=0.02f;
  Speakers[1].Direction.x=1.0f;
}


SndSysRendererSoftware::~SndSysRendererSoftware()
{
  delete[] sample_buffer;

  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

void SndSysRendererSoftware::Report(int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);

  if (!reporter)
    reporter = CS_QUERY_REGISTRY(object_reg, iReporter);

  if (reporter)
    reporter->ReportV (severity, "crystalspace.SndSys.renderer.software", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}


bool SndSysRendererSoftware::HandleEvent (iEvent &e)
{
  if (e.Type == csevCommand || e.Type == csevBroadcast) {
    switch (csCommandEventHelper::GetCode(&e)) {
    case cscmdSystemOpen:
      Open();
      break;
    case cscmdSystemClose:
      Close();
      break;
    }
  }
  return false;
}

bool SndSysRendererSoftware::Initialize (iObjectRegistry *obj_reg)
{
  // copy the system pointer
  object_reg=obj_reg;

  Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Software Renderer Initializing..");


  // read the config file
  Config.AddConfig(object_reg, "/config/sound.cfg");

  // check for optional sound driver from the commandline
  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg, iCommandLineParser));
  const char *drv = cmdline->GetOption ("sounddriver");
  if (!drv)
  {
    // Read the configuration value for the sound driver if one is set
    drv = Config->GetStr ("SndSys.Driver", "crystalspace.SndSys.driver.null");
  }

  Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Configured for driver [%s]", drv);



  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  SoundDriver = CS_LOAD_PLUGIN (plugin_mgr, drv, iSndSysSoftwareDriver);
  if (!SoundDriver)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Sound System: Failed to load driver [%s].", drv);
    return false;
  }

  // set event callback
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener(scfiEventHandler,
      CSMASK_Command | CSMASK_Broadcast | CSMASK_Nothing);

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
bool SndSysRendererSoftware::Open ()
{
  Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Open()");
  CS_ASSERT (Config != 0);

  if (!SoundDriver) return false;

  render_format.Freq = Config->GetInt("SndSys.Frequency", 44100);
  render_format.Bits = Config->GetInt("SndSys.Bits", 16);
  render_format.Channels = Config->GetInt("SndSys.Channels", 2);

  Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Initializing driver with Freq=%d Channels=%d Bits=%d",
    render_format.Freq, render_format.Channels, render_format.Bits);

  if (!SoundDriver->Open(this, &render_format))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Sound System: SoundDriver->Open() failed!");
    return false;
  }

  Listener = new SndSysListenerSoftware();
  

  Volume = Config->GetFloat("SndSys.Volume", 1.0);
  if (Volume>1.0f) Volume = 1.0f;
  if (Volume<0.0f) Volume = 0.0f;

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Sound System:  Volume=%f", Volume);

  return SoundDriver->StartThread();
}

void SndSysRendererSoftware::Close ()
{
  if (!SoundDriver) return;

  SoundDriver->StopThread();
  SoundDriver->Close();
}


void SndSysRendererSoftware::SetVolume (float vol)
{
  Volume=vol;
}

float SndSysRendererSoftware::GetVolume ()
{
  return Volume;
}

csPtr<iSndSysStream> SndSysRendererSoftware::CreateStream(csRef<iSndSysData> data, int mode3d)
{
  SndSysSoundFormat stream_format;
  iSndSysStream *stream;

  // Determine the desired format based on the 3d mode
  stream_format.Bits=render_format.Bits;
  stream_format.Freq=render_format.Freq;

  if (mode3d == SND3D_DISABLE)
    stream_format.Channels=render_format.Channels;
  else
    stream_format.Channels=1; // positional sounds are rendered to mono format
  
  // Tell the data object to create a stream using our desired format
  stream=data->CreateStream(&stream_format, mode3d);

  // This is the reference that will belong to the render thread
  stream->IncRef();

  Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Queueing stream with addr %08x", stream);

  // Queue this source for the background thread to add to its list of existant sources
  stream_add_queue.QueueEntry(stream);

  return stream;
}


csPtr<iSndSysSource> SndSysRendererSoftware::CreateSource(csRef<iSndSysStream> stream)
{
  iSndSysSourceSoftware *source=NULL;
  // Needs to be threadsafe with the background thread
  if (stream->Get3dMode() == SND3D_DISABLE)
  {
    SndSysSourceSoftwareBasic *s =new SndSysSourceSoftwareBasic(stream, this);
    source=s;
  }
  else
  {
    SndSysSourceSoftware3D *s =new SndSysSourceSoftware3D(stream, this);
    source=s;
  }

  // This is the reference that will belong to the render thread
  source->IncRef();

  Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Queueing source with addr %08x", source);
  // Queue this source for the background thread to add to its list of existant sources
  source_add_queue.QueueEntry(source);
 
  return source;
}

/// Remove a stream from the sound renderer's list of streams
bool SndSysRendererSoftware::RemoveStream(csRef<iSndSysStream> stream)
{
  iSndSysStream *streamptr=stream->GetPtr();

  stream_remove_queue.QueueEntry(streamptr);
  streamptr=stream_clear_queue.DequeueEntry(true);

  if (!streamptr)
    return false;

  streamptr->DecRef();

  return true;
}

/// Remove a source from the sound renderer's list of sources
bool SndSysRendererSoftware::RemoveSource(csRef<iSndSysSource> source)
{
  iSndSysSource *sourceptr=source->GetPtr();

  source_remove_queue.QueueEntry(sourceptr);
  sourceptr=source_clear_queue.DequeueEntry(true);

  if (!sourceptr)
    return false;

  sourceptr->DecRef();

  return true;
}



csRef<iSndSysListener> SndSysRendererSoftware::GetListener()
{
  return Listener;
}

uint32 SndSysRendererSoftware::FillDriverBuffer(void *buf1, uint32 buf1_len,
						void *buf2, uint32 buf2_len)
{
  csTicks current_ticks;

  // Update queued listener property changes
  Listener->UpdateQueuedProperties();
 
  // Handle any pending source or stream additions or removals
  ProcessPendingSources();
  ProcessPendingStreams();
  
  // Calculate the elapsed time since the last fill
  current_ticks=csGetTicks();

  // Resize the samplebuffer if needed
  long needed_samples = CalculateMaxSamples (buf1_len+buf2_len);

  if ((sample_buffer==NULL) || (needed_samples > (long)sample_buffer_samples))
  {
    delete[] sample_buffer;
    sample_buffer=new SoundSample[needed_samples];
    //Report(CS_REPORTER_SEVERITY_DEBUG, "Allocated a new sample buffer at %08x of %08x samples", sample_buffer, needed_samples);
    sample_buffer_samples = needed_samples;
  }

  // Clear as much of the buffer as we need
  memset(sample_buffer,0,sizeof(SoundSample) * needed_samples);

  size_t maxidx,currentidx;

  // Advance all the streams based on the elapsed time
  maxidx=streams.GetSize();
  for (currentidx=0;currentidx<maxidx;currentidx++)
  {
    // Search for any streams that have an unregister requested and are paused and mark them for removal in the next cycle
    iSndSysStream *str=streams.Get(currentidx);
    if ((str->GetPauseState() == ISNDSYS_STREAM_PAUSED) && (str->GetAutoUnregisterRequested() == true))
    {
      int currentsource,maxsource;
      maxsource=sources.GetSize();
      for (currentsource=0;currentsource<maxsource;currentsource++)
      {
        if (sources.Get(currentsource)->GetStream() == str)
          source_remove_queue.QueueEntry(sources.Get(currentsource));
      }
      stream_remove_queue.QueueEntry(str);
    }
    // Advance stream
    streams.Get(currentidx)->AdvancePosition(current_ticks);
  }


  // Mix all the sources
  maxidx=sources.GetSize();
  for (currentidx=0;currentidx<maxidx;currentidx++)
  {
    long provided_samples;
    //Report (CS_REPORTER_SEVERITY_DEBUG, 
      //"Sound System: Requesting %d samples from source.", needed_samples);

    // The return from this function is this number of samples actually available 
    provided_samples = sources.Get(currentidx)->MergeIntoBuffer(sample_buffer,
      needed_samples);

    if (provided_samples==0)
    {
      //Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Source failing to keep up, skipping audio.");
    }
    else
    {
      CS_ASSERT (provided_samples <= needed_samples);
      // Reduce the number of samples we request from future sources, as well as the number we provide to the driver
      needed_samples=provided_samples; 
    }
  }


  // Normalize the sample buffer
  NormalizeSampleBuffer(needed_samples);

  // Correct the length of buf1_len and buf2_len
  CalculateMaxBuffers(needed_samples,&buf1_len,&buf2_len);

  // Copy normalized data to the driver
  CopySampleBufferToDriverBuffer(buf1,buf1_len,buf2,buf2_len, needed_samples/2);

  //csTicks end_ticks=csGetTicks();
//  Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Processing time: %u ticks.", end_ticks-current_ticks);


  return needed_samples * (render_format.Bits/8);
}


uint32 SndSysRendererSoftware::CalculateMaxSamples(size_t bytes)
{
  uint32 samples;
  
  // Divide the bytes available by the number of bytes per sample
  samples=bytes / (render_format.Bits/8);

  return samples;
}

void SndSysRendererSoftware::CalculateMaxBuffers(size_t samples, uint32 *buf1_len, uint32 *buf2_len)
{
  uint32 bytes;

  bytes=samples * (render_format.Bits/8);

  if (bytes==(*buf1_len + *buf2_len))
    return; // No adjustment needed

  if (bytes>*buf1_len)
  {
    *buf2_len=bytes-(*buf1_len);
    return;
  }
  *buf2_len=0;
  *buf1_len=bytes;
}


void SndSysRendererSoftware::ProcessPendingSources()
{
  iSndSysSourceSoftware *src;

  while (src=source_add_queue.DequeueEntry())
  {
//    Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Found a queued source to add to the active list.");
    sources.Push(src);
  }

  while (src=(iSndSysSourceSoftware *)source_remove_queue.DequeueEntry())
  {
    if (sources.Delete(src))
      source_clear_queue.QueueEntry(src);
    else
      source_clear_queue.QueueEntry(NULL);
  }
}

void SndSysRendererSoftware::ProcessPendingStreams()
{
  iSndSysStream *stream;
  while (stream=stream_add_queue.DequeueEntry())
    streams.Push(stream);
  while (stream=stream_remove_queue.DequeueEntry())
  {
    if (streams.Delete(stream))
      stream_clear_queue.QueueEntry(stream);
    else
      stream_clear_queue.QueueEntry(NULL);
  }
}


void SndSysRendererSoftware::NormalizeSampleBuffer(size_t used_samples)
{
  size_t sample_idx;
  SoundSample maxintensity=0;
  SoundSample desiredintensity = (SoundSample)(0x7FFF * Volume);
  SoundSample low_threshold;
  uint32 multiplier;

  if (desiredintensity> 0x7FFF) 
    desiredintensity=0x7FFF;
  desiredintensity=desiredintensity<<16;
  
  // First scan, find the max abs sample value
  for (sample_idx=0;sample_idx<used_samples;sample_idx++)
  {
    SoundSample abssamp=sample_buffer[sample_idx];
    if (abssamp<0) abssamp=-abssamp;
    if (abssamp > maxintensity) maxintensity=abssamp;
  }

  //Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Maximum sample intensity is %d", maxintensity);

  // Silence fills the buffer, also dividing by zero is bad
  if (maxintensity==0)
    return;

  if (render_format.Bits==8)
    low_threshold=127;
  else
    low_threshold=32767;
  if (maxintensity<low_threshold)
    maxintensity=low_threshold;

  //Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Maximum sample intensity (clamped) is %d", maxintensity);

  multiplier=desiredintensity / maxintensity;
  

  // Clamp the intensity multiplier to about 3% +/- the last value
  if (last_intensity_multiplier>0)
  {
    uint32 acceptable_range=last_intensity_multiplier/32;

    //Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: last intensity multiplier is %u", last_intensity_multiplier);

    if (multiplier > last_intensity_multiplier + acceptable_range)
      multiplier=last_intensity_multiplier+acceptable_range;
    if (multiplier < last_intensity_multiplier - acceptable_range)
      multiplier=last_intensity_multiplier-acceptable_range;


    // Recalculate the maximum intensity
    maxintensity=desiredintensity/multiplier;
  }
  last_intensity_multiplier=multiplier;

  //Report (CS_REPORTER_SEVERITY_DEBUG, "Sound System: Multiplier is %u", multiplier);

  // Second scan, multiply the values to create 32 bit samples with a max near 0x7FFFFFFF
  for (sample_idx=0;sample_idx<used_samples;sample_idx++)
  {
    // Clamp to max
    if (sample_buffer[sample_idx] > maxintensity)
      sample_buffer[sample_idx]=desiredintensity;
    else if (sample_buffer[sample_idx] < -maxintensity)
      sample_buffer[sample_idx]=-desiredintensity;
    else
      sample_buffer[sample_idx]*=multiplier;
  }
}


void SndSysRendererSoftware::CopySampleBufferToDriverBuffer(void *drvbuf1,size_t drvbuf1_len,void *drvbuf2, size_t drvbuf2_len, uint32 samples_per_channel)
{
  SoundSample *ptr=sample_buffer;

  if (drvbuf1)
    ptr=CopySampleBufferToDriverBuffer(drvbuf1,drvbuf1_len, ptr, samples_per_channel);
  if (drvbuf2)
    ptr=CopySampleBufferToDriverBuffer(drvbuf2,drvbuf2_len, ptr, samples_per_channel);
}

SoundSample *SndSysRendererSoftware::CopySampleBufferToDriverBuffer(void *drvbuf,size_t drvbuf_len, SoundSample *src, uint32 samples_per_channel)
{
  if (render_format.Bits==8)
  {
    // 8 bit destination samples
    unsigned char *dptr=(unsigned char *)drvbuf;
//    while (drvbuf_len-- > 0)
//      *(dptr++)= ((( (*(src++))   >> 24) & 0xFF) + 128);
    drvbuf_len/=2;
    while (drvbuf_len-- > 0)
    {
      *(dptr++)= (((src[0] >> 24) & 0xFF) + 128);
      *(dptr++)= (((src[samples_per_channel] >> 24) & 0xFF) + 128);
      src++;
    }

  }
  else
  {
    // 16 bit destination samples
    short *dptr=(short *)drvbuf;
//    drvbuf_len/=2;
//    while (drvbuf_len-- > 0)
//      *(dptr++)=(((*(src++)) >> 16) & 0xFFFF);
    drvbuf_len/=4;
    while (drvbuf_len-- > 0)
    {
      // 16 bit output should be in little endian
      *(dptr++)=csLittleEndianShort((((src[0]) >> 16) & 0xFFFF));
      *(dptr++)=csLittleEndianShort((((src[samples_per_channel]) >> 16) & 0xFFFF));
      src++;
    }
  }
  return src;
}
