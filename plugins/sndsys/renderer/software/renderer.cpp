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
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "iutil/cmdline.h"
#include "ivaria/reporter.h"

#include "listener.h"
#include "source.h"
#include "renderer.h"


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csSndSysRendererSoftware)


SCF_IMPLEMENT_IBASE(csSndSysRendererSoftware)
	SCF_IMPLEMENTS_INTERFACE(iSndSysRenderer)
	SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSndSysRendererSoftware::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSndSysRendererSoftware::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

// Run garbage collection at most 2x per second (it's also always run on shutdown)
#define SNDSYS_RENDERER_SOFTWARE_GARBAGECOLLECTION_TICKS 500

csSndSysRendererSoftware::csSndSysRendererSoftware(iBase* piBase) :
  object_reg(0), sample_buffer(0), sample_buffer_samples(0),
  last_intensity_multiplier(0), LastGarbageCollectionTicks(0)
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


csSndSysRendererSoftware::~csSndSysRendererSoftware()
{
  delete[] sample_buffer;

  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

void csSndSysRendererSoftware::RecordEvent(SndSysEventCategory Category, SndSysEventLevel Severity, const char* msg, ...)
{
  if (!EventRecorder)
    return;

  va_list arg;
  va_start (arg, msg);
  EventRecorder->RecordEventV(Category, Severity, msg, arg);
  va_end (arg);
}

void csSndSysRendererSoftware::RecordEvent(SndSysEventLevel Severity, const char* msg, ...)
{
  if (!EventRecorder)
    return;

  va_list arg;
  va_start (arg, msg);
  EventRecorder->RecordEventV(SSEC_RENDERER, Severity, msg, arg);
  va_end (arg);
}


void csSndSysRendererSoftware::Report(int severity, const char* msg, ...)
{
  va_list arg;

  // Anything that goes to the reporter should automatically going to the recorder too
  if (EventRecorder)
  {
    SndSysEventLevel Level;

    switch (severity)
    {
      case CS_REPORTER_SEVERITY_DEBUG:
        Level=SSEL_DEBUG;
        break;
      case CS_REPORTER_SEVERITY_ERROR:
        Level=SSEL_ERROR;
        break;
      case CS_REPORTER_SEVERITY_NOTIFY:
        Level=SSEL_CRITICAL;
        break;
      case CS_REPORTER_SEVERITY_BUG:
        Level=SSEL_BUG;
        break;
      case CS_REPORTER_SEVERITY_WARNING:
        Level=SSEL_WARNING;
        break;
      default:
        Level=SSEL_CRITICAL;
        break;
    }

    va_list arg;
    va_start (arg, msg);
    EventRecorder->RecordEventV(SSEC_RENDERER, Level, msg, arg);
    va_end (arg);
  }

  // To the reporter (console)
  va_start (arg, msg);
  csReportV (object_reg, severity,
    "crystalspace.sndsys.renderer.software", msg, arg);
  va_end (arg);
}

bool csSndSysRendererSoftware::HandleEvent (iEvent &e)
{
  if (e.Name == evFrame) 
  {
    if (e.Time >= (LastGarbageCollectionTicks+SNDSYS_RENDERER_SOFTWARE_GARBAGECOLLECTION_TICKS))
    {
      GarbageCollection();
      LastGarbageCollectionTicks=e.Time;
    }
  }
  else if (e.Name == evSystemOpen) 
  {
    Open();
  } else if (e.Name == evSystemClose) 
  {
    Close();
  }
  return false;
}

bool csSndSysRendererSoftware::Initialize (iObjectRegistry *obj_reg)
{
  // copy the system pointer
  object_reg=obj_reg;

  Report (CS_REPORTER_SEVERITY_DEBUG, "Software Renderer Initializing..");

  // Get an interface for the plugin manager
  csRef<iPluginManager> plugin_mgr (
    CS_QUERY_REGISTRY (object_reg, iPluginManager));


  // read the config file
  Config.AddConfig(object_reg, "/config/sound.cfg");

  // check for optional sound driver from the commandline
  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser));
  const char *drv = cmdline->GetOption ("sounddriver");
  if (!drv)
  {
    // Read the configuration value for the sound driver if one is set
#ifdef CS_SNDSYS_DRIVER
    drv = CS_SNDSYS_DRIVER;   // "crystalspace.sndsys.software.driver.xxx"
#else
    drv = "crystalspace.sndsys.driver.null";
#endif
    drv = Config->GetStr ("SndSys.Driver", drv);
  }

  // Override for explicit recorder plugin request
  const char *eventrecorder = cmdline->GetOption ("soundeventrecorder");
  if (!eventrecorder)
  {
    // Check config files
    eventrecorder = Config->GetStr("SndSys.EventRecorder", eventrecorder);

    // Finally, if an eventlog was specified we will presume 
    //  "crystalspace.sndsys.utility.eventrecorder"
    if (!eventrecorder)
    {
      if ((cmdline->GetOption("soundeventlog") != 0) ||
          (Config->GetStr("SndSys.EventLog", 0) != 0))
          eventrecorder = "crystalspace.sndsys.utility.eventrecorder";
    }
  }

  // If we determined an eventrecorder plugin should be loaded, load it
  if (eventrecorder != 0)
  {
    CS_QUERY_REGISTRY_PLUGIN(EventRecorder, object_reg, eventrecorder, iSndSysEventRecorder);
    if (!EventRecorder)
      Report (CS_REPORTER_SEVERITY_ERROR, "Event Recorder [%s] specified, but unable to load.", eventrecorder);
    // There is no need to bail out, we'll handle this ok
  }

  RecordEvent(SSEL_DEBUG, "Event log started");

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Configured for driver [%s]", drv);



  SoundDriver = CS_LOAD_PLUGIN (plugin_mgr, drv, iSndSysSoftwareDriver);
  if (!SoundDriver)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Failed to load driver [%s].", drv);
    return false;
  }

  // set event callback
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  evSystemOpen = csevSystemOpen(object_reg);
  evSystemClose = csevSystemClose(object_reg);
  evFrame = csevFrame(object_reg);
  if (q != 0) {
    csEventID subEvents[] = { evSystemOpen, evSystemClose, evFrame, CS_EVENTLIST_END };
    q->RegisterListener(scfiEventHandler, subEvents);
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
bool csSndSysRendererSoftware::Open ()
{
  Report (CS_REPORTER_SEVERITY_DEBUG, "Open()");
  CS_ASSERT (Config != 0);

  if (!SoundDriver) return false;

  render_format.Freq = Config->GetInt("SndSys.Frequency", 44100);
  render_format.Bits = Config->GetInt("SndSys.Bits", 16);
  render_format.Channels = Config->GetInt("SndSys.Channels", 2);
  render_format.Flags=0;
  // Renderer data format is native endian
#ifdef CS_LITTLE_ENDIAN
  render_format.Flags|=CSSNDSYS_SAMPLE_LITTLE_ENDIAN;
#else
  render_format.Flags|=CSSNDSYS_SAMPLE_BIG_ENDIAN;
#endif


  Report (CS_REPORTER_SEVERITY_DEBUG,
    "Initializing driver with Freq=%d Channels=%d Bits=%d",
    render_format.Freq, render_format.Channels, render_format.Bits);

  if (!SoundDriver->Open(this, &render_format))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "SoundDriver->Open() failed!");
    return false;
  }

  // The software renderer always works in native endian format.  If the driver changes
  //  the endian format away from the native format, we note that here, but don't pass it down
  //  to the stream and sources.
  if ((render_format.Flags & CSSNDSYS_SAMPLE_ENDIAN_MASK) == CSSNDSYS_SAMPLE_LITTLE_ENDIAN)
    driver_little_endian=true;
  else
    driver_little_endian=false;

  // Clear out the flag for byte order and reset to what we'll use in the renderer
  render_format.Flags&=~(CSSNDSYS_SAMPLE_ENDIAN_MASK);
#ifdef CS_LITTLE_ENDIAN
  render_format.Flags|=CSSNDSYS_SAMPLE_LITTLE_ENDIAN;
#else
  render_format.Flags|=CSSNDSYS_SAMPLE_BIG_ENDIAN;
#endif


  Listener = new SndSysListenerSoftware();


  Volume = Config->GetFloat("SndSys.Volume", 1.0);
  if (Volume>1.0f) Volume = 1.0f;
  if (Volume<0.0f) Volume = 0.0f;

  Report (CS_REPORTER_SEVERITY_NOTIFY, "Volume=%f", Volume);

  return SoundDriver->StartThread();
}

void csSndSysRendererSoftware::Close ()
{
  // Halt the background thread
  if (SoundDriver)
  {
    SoundDriver->StopThread();
    SoundDriver->Close();
  }
  // Cleanup any active or pending-active sources
  RemoveAllSources();
  // Cleanup any active or pending-active streams
  RemoveAllStreams();
  // Cleanup any pending-clear sources and streams
  GarbageCollection();
}


//      !!WARNING!!  DO NOT CALL THIS FROM THE BACKGROUND THREAD
void csSndSysRendererSoftware::GarbageCollection()
{
  iSndSysSource *sourceptr;
  iSndSysStream *streamptr;

  // Make a pass through the clear queue of streams and cleanup 
  while (streamptr=stream_clear_queue.DequeueEntry(false))
    streamptr->DecRef();

  // Make a pass through the clear queue of sources and cleanup 
  while (sourceptr=source_clear_queue.DequeueEntry(false))
    sourceptr->DecRef();
}

//      !!WARNING!!  DO NOT CALL THIS FROM THE BACKGROUND THREAD
void csSndSysRendererSoftware::RemoveAllSources()
{
  iSndSysSourceSoftware *sourceptr;

  // We will bypass the removal queue since this function is only called when the background thread is
  //  not running
  while (sourceptr=sources.Get(0))
  {
    sources.Delete(sourceptr);
    sourceptr->DecRef();
  }
  // Process the addition queue
  while (sourceptr=source_add_queue.DequeueEntry(false))
    sourceptr->DecRef();

  // The clear queue will be processed by GarbageCollection()

}

//      !!WARNING!!  DO NOT CALL THIS FROM THE BACKGROUND THREAD
void csSndSysRendererSoftware::RemoveAllStreams()
{
  iSndSysStream *streamptr;

  // We will bypass the removal queue since this function is only called when the background thread is
  //  not running
  while (streamptr=streams.Get(0))
  {
    streams.Delete(streamptr);
    streamptr->DecRef();
  }
  // Process the addition queue
  while (streamptr=stream_add_queue.DequeueEntry(false))
    streamptr->DecRef();

  // The clear queue will be processed by GarbageCollection()
}

void csSndSysRendererSoftware::SetVolume (float vol)
{
  Volume=vol;
}

float csSndSysRendererSoftware::GetVolume ()
{
  return Volume;
}

csPtr<iSndSysStream> csSndSysRendererSoftware::CreateStream(iSndSysData* data, int mode3d)
{
  csSndSysSoundFormat stream_format;
  iSndSysStream *stream;

  // Determine the desired format based on the 3d mode
  stream_format.Bits=render_format.Bits;
  stream_format.Freq=render_format.Freq;

  if (mode3d == CS_SND3D_DISABLE)
    stream_format.Channels=render_format.Channels;
  else
    stream_format.Channels=1; // positional sounds are rendered to mono format

  // Tell the data object to create a stream using our desired format
  stream=data->CreateStream(&stream_format, mode3d);

  // This is the reference that will belong to the render thread
  stream->IncRef();

  //Report (CS_REPORTER_SEVERITY_DEBUG, "Queueing stream with addr %08x", stream);

  // Queue this source for the background thread to add to its list of existant sources
  stream_add_queue.QueueEntry(stream);

  return stream;
}


csPtr<iSndSysSource> csSndSysRendererSoftware::CreateSource(iSndSysStream* stream)
{
  iSndSysSourceSoftware *source=0;
  // Needs to be threadsafe with the background thread
  if (stream->Get3dMode() == CS_SND3D_DISABLE)
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

  //Report (CS_REPORTER_SEVERITY_DEBUG, "Queueing source with addr %08x", source);
  // Queue this source for the background thread to add to its list of existant sources
  source_add_queue.QueueEntry(source);

  return source;
}

/// Remove a stream from the sound renderer's list of streams
bool csSndSysRendererSoftware::RemoveStream(iSndSysStream* stream)
{
  stream_remove_queue.QueueEntry(stream);
  return true;
}

/// Remove a source from the sound renderer's list of sources
bool csSndSysRendererSoftware::RemoveSource(iSndSysSource* source)
{
  source_remove_queue.QueueEntry(source);
  return true;
}


csRef<iSndSysListener> csSndSysRendererSoftware::GetListener()
{
  return Listener;
}

size_t csSndSysRendererSoftware::FillDriverBuffer(void *buf1, size_t buf1_len,
						  void *buf2, size_t buf2_len)
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
  size_t needed_samples = CalculateMaxSamples (buf1_len+buf2_len);

  if ((sample_buffer==0) || (needed_samples > sample_buffer_samples))
  {
    delete[] sample_buffer;
    sample_buffer=new csSoundSample[needed_samples];
    //Report(CS_REPORTER_SEVERITY_DEBUG, "Allocated a new sample buffer at %08x of %08x samples", sample_buffer, needed_samples);
    sample_buffer_samples = needed_samples;
  }

  // Clear as much of the buffer as we need
  memset(sample_buffer,0,sizeof(csSoundSample) * needed_samples);

  size_t maxidx,currentidx;

  // Advance all the streams based on the elapsed time
  maxidx=streams.GetSize();
  for (currentidx=0;currentidx<maxidx;currentidx++)
  {
    // Search for any streams that have an unregister requested and are paused and mark them for removal in the next cycle
    iSndSysStream *str=streams.Get(currentidx);
    if ((str->GetPauseState() == CS_SNDSYS_STREAM_PAUSED)
      && (str->GetAutoUnregisterRequested() == true))
    {
      size_t currentsource, maxsource;
      maxsource = sources.GetSize();
      for (currentsource = 0; currentsource < maxsource; currentsource++)
      {
        if (sources.Get(currentsource)->GetStream() == str)
          source_remove_queue.QueueEntry (sources.Get(currentsource));
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
    size_t provided_samples;
    //Report (CS_REPORTER_SEVERITY_DEBUG,
      //"Requesting %d samples from source.", needed_samples);

    // The return from this function is this number of samples actually available
    provided_samples = sources.Get(currentidx)->MergeIntoBuffer (sample_buffer,
      needed_samples);

    if (provided_samples==0)
    {
      //Report (CS_REPORTER_SEVERITY_DEBUG, "Source failing to keep up, skipping audio.");
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
  CopySampleBufferToDriverBuffer (buf1,buf1_len,buf2,buf2_len, needed_samples/2);

  //csTicks end_ticks=csGetTicks();
//  Report (CS_REPORTER_SEVERITY_DEBUG, "Processing time: %u ticks.", end_ticks-current_ticks);


  return needed_samples * (render_format.Bits/8);
}


size_t csSndSysRendererSoftware::CalculateMaxSamples(size_t bytes)
{
  size_t samples;

  // Divide the bytes available by the number of bytes per sample
  samples=bytes / (render_format.Bits/8);

  return samples;
}

void csSndSysRendererSoftware::CalculateMaxBuffers(size_t samples,
						   size_t *buf1_len,
						   size_t *buf2_len)
{
  size_t bytes;

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


void csSndSysRendererSoftware::ProcessPendingSources()
{
  iSndSysSourceSoftware *src;

  while ((src=source_add_queue.DequeueEntry()))
  {
//    Report (CS_REPORTER_SEVERITY_DEBUG, "Found a queued source to add to the active list.");
    sources.Push(src);
  }

  while ((src=(iSndSysSourceSoftware *)source_remove_queue.DequeueEntry()))
  {
    if (sources.Delete(src))
      source_clear_queue.QueueEntry(src);
    else
      source_clear_queue.QueueEntry(0);
  }
}

void csSndSysRendererSoftware::ProcessPendingStreams()
{
  iSndSysStream *stream;
  while ((stream=stream_add_queue.DequeueEntry()))
    streams.Push(stream);
  while ((stream=stream_remove_queue.DequeueEntry()))
  {
    if (streams.Delete(stream))
      stream_clear_queue.QueueEntry(stream);
    else
      stream_clear_queue.QueueEntry(0);
  }
}


void csSndSysRendererSoftware::NormalizeSampleBuffer(size_t used_samples)
{
  size_t sample_idx;
  csSoundSample maxintensity=0;
  csSoundSample desiredintensity = (csSoundSample)(0x7FFF * Volume);
  csSoundSample low_threshold;
  uint32 multiplier;

  if (desiredintensity> 0x7FFF)
    desiredintensity=0x7FFF;
  desiredintensity=desiredintensity<<16;

  // First scan, find the max abs sample value
  for (sample_idx=0;sample_idx<used_samples;sample_idx++)
  {
    csSoundSample abssamp=sample_buffer[sample_idx];
    if (abssamp<0) abssamp=-abssamp;
    if (abssamp > maxintensity) maxintensity=abssamp;
  }

  //Report (CS_REPORTER_SEVERITY_DEBUG, "Maximum sample intensity is %d", maxintensity);

  // Silence fills the buffer, also dividing by zero is bad
  if (maxintensity==0)
    return;

  if (render_format.Bits==8)
    low_threshold=127;
  else
    low_threshold=32767;
  if (maxintensity<low_threshold)
    maxintensity=low_threshold;

  //Report (CS_REPORTER_SEVERITY_DEBUG, "Maximum sample intensity (clamped) is %d", maxintensity);

  multiplier=desiredintensity / maxintensity;


  // Clamp the intensity multiplier to about 3% +/- the last value
  if (last_intensity_multiplier>0)
  {
    uint32 acceptable_range=last_intensity_multiplier/32;

    //Report (CS_REPORTER_SEVERITY_DEBUG, "last intensity multiplier is %u", last_intensity_multiplier);

    if (multiplier > last_intensity_multiplier + acceptable_range)
      multiplier=last_intensity_multiplier+acceptable_range;
    if (multiplier < last_intensity_multiplier - acceptable_range)
      multiplier=last_intensity_multiplier-acceptable_range;


    // Recalculate the maximum intensity
    maxintensity=desiredintensity/multiplier;
  }
  last_intensity_multiplier=multiplier;

  //Report (CS_REPORTER_SEVERITY_DEBUG, "Multiplier is %u", multiplier);

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


void csSndSysRendererSoftware::CopySampleBufferToDriverBuffer (void *drvbuf1,
							       size_t drvbuf1_len,
							       void *drvbuf2,
							       size_t drvbuf2_len,
							       size_t samples_per_channel)
{
  csSoundSample *ptr=sample_buffer;

  if (drvbuf1)
    ptr=CopySampleBufferToDriverBuffer(drvbuf1,drvbuf1_len, ptr,
      samples_per_channel);
  if (drvbuf2)
    ptr=CopySampleBufferToDriverBuffer(drvbuf2,drvbuf2_len, ptr,
      samples_per_channel);
}

csSoundSample *csSndSysRendererSoftware::CopySampleBufferToDriverBuffer (
  void *drvbuf, size_t drvbuf_len, csSoundSample *src,
  size_t samples_per_channel)
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
    if (driver_little_endian)
    {
      while (drvbuf_len-- > 0)
      {
        // 16 bit output should be in little endian
        *(dptr++) = csLittleEndian::UInt16 ((((src[0]) >> 16) & 0xFFFF));
        *(dptr++) = csLittleEndian::UInt16 ((((src[samples_per_channel]) >> 16) & 0xFFFF));
        src++;
      }
    }
    else
    {
      while (drvbuf_len-- > 0)
      {
        // 16 bit output should be in big endian
        *(dptr++) = csBigEndian::UInt16 ((((src[0]) >> 16) & 0xFFFF));
        *(dptr++) = csBigEndian::UInt16 ((((src[samples_per_channel]) >> 16) & 0xFFFF));
        src++;
      }
    }
  }
  return src;
}
