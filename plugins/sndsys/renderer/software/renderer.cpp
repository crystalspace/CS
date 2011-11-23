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
#include "csutil/eventnames.h"
#include "csutil/csendian.h"

#include "iutil/plugin.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/systemopenmanager.h"
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




SCF_IMPLEMENT_FACTORY (csSndSysRendererSoftware)


// Run garbage collection at most 2x per second (it's also always run on shutdown)
#define SNDSYS_RENDERER_SOFTWARE_GARBAGECOLLECTION_TICKS 500

//------------------------------------
// Construction/Destruction functions
//------------------------------------

csSndSysRendererSoftware::csSndSysRendererSoftware(iBase* pParent) :
  scfImplementationType(this, pParent),
  m_pObjectRegistry(0), m_pSampleBuffer(0), m_SampleBufferFrames(0),
  m_LastGarbageCollectionTicks(0), m_LastIntensityMultiplier(0)
{
  m_pObjectRegistry = 0;
  m_GlobalVolume=0.5;

#ifdef CS_DEBUG
  m_LastStatusReport=0;
#endif

  m_pSoundCompressor=NULL;

  int i;
  for (i=0;i<MAX_CHANNELS;i++)
    memset(&m_Speakers[i],0,sizeof(st_speaker_properties));

  m_Speakers[0].RelativePosition.x=-0.1f;
  m_Speakers[0].RelativePosition.y=0.02f;
  m_Speakers[0].Direction.x=-1.0f;
  m_Speakers[1].RelativePosition.x=0.1f;
  m_Speakers[1].RelativePosition.y=0.02f;
  m_Speakers[1].Direction.x=1.0f;
}


csSndSysRendererSoftware::~csSndSysRendererSoftware()
{
  RecordEvent(SSEL_DEBUG, "Sound system destructing.");
  if (weakEventHandler)
  {
    csRef<iEventQueue> q = csQueryRegistry<iEventQueue> (m_pObjectRegistry);
    if (q)
      CS::RemoveWeakListener (q, weakEventHandler);
  }
  if (weakOpenEventHandler)
  {
    csRef<iSystemOpenManager> sysOpen (
      csQueryRegistry<iSystemOpenManager> (m_pObjectRegistry));
    if (sysOpen)
      sysOpen->RemoveWeakListener (weakOpenEventHandler);
  }

  delete[] m_pSampleBuffer;
  delete m_pSoundCompressor;
}


//------------------------------------
// Application Interface functions
//------------------------------------

void csSndSysRendererSoftware::SetVolume (float vol)
{
  m_GlobalVolume=vol;
}

float csSndSysRendererSoftware::GetVolume ()
{
  return m_GlobalVolume;
}

csPtr<iSndSysStream> csSndSysRendererSoftware::CreateStream(iSndSysData* data, int mode3d)
{
  csSndSysSoundFormat stream_format;
  iSndSysStream *stream;

  // Determine the desired format based on the 3d mode
  stream_format.Bits=m_PlaybackFormat.Bits;
  stream_format.Freq=m_PlaybackFormat.Freq;

  if (mode3d == CS_SND3D_DISABLE)
    stream_format.Channels=m_PlaybackFormat.Channels;
  else
    stream_format.Channels=1; // positional sounds are rendered to mono format

  // Tell the data object to create a stream using our desired format
  stream=data->CreateStream(&stream_format, mode3d);

  // This is the reference that will belong to the render thread
  stream->IncRef();

  // Notify any registered callback components
  StreamAdded(stream);

  // Queue this stream for the background thread to add to its list of existent streams
  m_StreamAddQueue.QueueEntry(stream);

  // Add this stream to the foreground list of streams that require notification dispatching
  m_DispatchStreams.Push(stream);

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

  // Notify any registered callback components
  SourceAdded((iSndSysSource*)source);

  // Queue this source for the background thread to add to its list of existent sources
  m_SourceAddQueue.QueueEntry(source);

  // Add this source to the foreground list of sources that may have filters
  m_DispatchSources.Push(source);

  return scfQueryInterface<iSndSysSource> (source);
}

/// Remove a stream from the sound renderer's list of streams
bool csSndSysRendererSoftware::RemoveStream(iSndSysStream* stream)
{
  if (!stream)
    return false;

  RecordEvent(SSEL_DEBUG, "Queueing stream [%s] for remove with addr %08x", stream->GetDescription(), stream);

  m_StreamRemoveQueue.QueueEntry(stream);
  return true;
}

/// Remove a source from the sound renderer's list of sources
bool csSndSysRendererSoftware::RemoveSource(iSndSysSource* source)
{
  if (!source)
    return false;

  RecordEvent(SSEL_DEBUG, "Queueing source [%s] for remove with addr %08x", source->GetStream()->GetDescription(), source);

  m_SourceRemoveQueue.QueueEntry(source);
  return true;
}


csRef<iSndSysListener> csSndSysRendererSoftware::GetListener()
{
  return m_pListener;
}

//------------------------------------
// CrystalSpace Component functions
//------------------------------------

bool csSndSysRendererSoftware::HandleEvent (iEvent &e)
{
  if (e.Name == evFrame) 
  {

    if (e.Time >= (m_LastGarbageCollectionTicks+SNDSYS_RENDERER_SOFTWARE_GARBAGECOLLECTION_TICKS))
    {
      GarbageCollection();
      m_LastGarbageCollectionTicks=e.Time;
    }

    // Process any output filters
    ProcessOutputFilters();

    // Process any stream notifications
    ProcessStreamDispatch();
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
  // Keep a pointer to the object registry interface
  m_pObjectRegistry=obj_reg;

  // Use report here since the eventrecorder isn't ready yet
  Report (CS_REPORTER_SEVERITY_DEBUG,
    "Sound System Software Renderer Initializing...");

  // Get an interface for the plugin manager
  csRef<iPluginManager> plugin_mgr (
    csQueryRegistry<iPluginManager> (m_pObjectRegistry));


  // read the config file
  m_Config.AddConfig(m_pObjectRegistry, "/config/sound.cfg");

  // check for optional sound driver from the commandline
  csRef<iCommandLineParser> cmdline (
  	csQueryRegistry<iCommandLineParser> (m_pObjectRegistry));
  const char *drv = cmdline->GetOption ("sounddriver");
  if (!drv)
  {
    // Read the configuration value for the sound driver if one is set
#ifdef CS_SNDSYS_DRIVER
    drv = CS_SNDSYS_DRIVER;   // "crystalspace.sndsys.software.driver.xxx"
#else
    drv = "crystalspace.sndsys.software.driver.null";
#endif
    drv = m_Config->GetStr ("SndSys.Driver", drv);
  }

  // Override for explicit recorder plugin request
  const char *eventrecordername = cmdline->GetOption ("soundeventrecorder");
  if (!eventrecordername)
  {
    // Check config files
    eventrecordername = m_Config->GetStr("SndSys.EventRecorder", eventrecordername);

    // Finally, if an eventlog was specified we will presume 
    //  "crystalspace.sndsys.utility.eventrecorder"
    if (!eventrecordername)
    {
      if ((cmdline->GetOption("soundeventlog") != 0) ||
          (m_Config->GetStr("SndSys.EventLog", 0) != 0))
          eventrecordername = "crystalspace.sndsys.utility.eventrecorder";
    }
  }

  // If we determined an eventrecorder plugin should be loaded, load it
  if (eventrecordername != 0)
  {
    m_pEventRecorder = csQueryRegistryOrLoad<iSndSysEventRecorder> (
    	m_pObjectRegistry, eventrecordername, false);
    if (!m_pEventRecorder)
      Report (CS_REPORTER_SEVERITY_ERROR, "Event Recorder [%s] specified, but unable to load.", eventrecordername);
    // There is no need to bail out, we'll handle this ok
  }

  RecordEvent(SSEL_DEBUG, "Event log started");



  csStringBase DriverFullName;

  // Try to load the specified driver exactly as specified
  DriverFullName=drv;
  RecordEvent(SSEL_DEBUG, "Attempting to load driver plugin [%s]", DriverFullName.GetData());
  m_pSoundDriver = csLoadPlugin<iSndSysSoftwareDriver> (plugin_mgr,
    DriverFullName.GetData());

  // Try to load the driver with "crystalspace.sndsys.software.driver." prepended
  if (!m_pSoundDriver)
  {
    DriverFullName.Format("crystalspace.sndsys.software.driver.%s", drv);
    RecordEvent(SSEL_DEBUG, "Attempting to load driver plugin [%s]", DriverFullName.GetData());
    m_pSoundDriver = csLoadPlugin<iSndSysSoftwareDriver> (plugin_mgr,
      DriverFullName.GetData());
  }

  // If we still failed, report an error
  if (!m_pSoundDriver)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Failed to load driver as [%s] or [%s].", drv, DriverFullName.GetData());
    return false;
  }
  // Success
  RecordEvent(SSEL_DEBUG, "Loaded driver plugin [%s]", DriverFullName.GetData());


  // set event callback
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (m_pObjectRegistry));
  evSystemOpen = csevSystemOpen(m_pObjectRegistry);
  evSystemClose = csevSystemClose(m_pObjectRegistry);
  evFrame = csevFrame(m_pObjectRegistry);
  if (q != 0) {
    csEventID subEvents[] = { evFrame, CS_EVENTLIST_END };
    CS::RegisterWeakListener(q, this, subEvents, weakEventHandler);
  }
  csRef<iSystemOpenManager> sysOpen (
    csQueryRegistry<iSystemOpenManager> (m_pObjectRegistry));
  sysOpen->RegisterWeak (this, weakOpenEventHandler);

  return true;
}

//----------------------------
// Startup/Shutdown functions
//----------------------------

bool csSndSysRendererSoftware::Open ()
{
  RecordEvent(SSEL_DEBUG, "Open() called.");

  CS_ASSERT (m_Config != 0);

  if (!m_pSoundDriver) 
  {
    // The sound driver probably failed to start for some reason
    RecordEvent(SSEL_ERROR, "No sound driver loaded!");
    return false;
  }

  m_PlaybackFormat.Freq = m_Config->GetInt("SndSys.Frequency", 44100);
  m_PlaybackFormat.Bits = m_Config->GetInt("SndSys.Bits", 16);
  m_PlaybackFormat.Channels = m_Config->GetInt("SndSys.Channels", 2);
  m_PlaybackFormat.Flags=0;
  // Renderer data format is native endian
#ifdef CS_LITTLE_ENDIAN
  m_PlaybackFormat.Flags|=CSSNDSYS_SAMPLE_LITTLE_ENDIAN;
#else
  m_PlaybackFormat.Flags|=CSSNDSYS_SAMPLE_BIG_ENDIAN;
#endif


  RecordEvent(SSEL_DEBUG, "Calling SoundDriver->Open() with Freq=%dhz Channels=%d Bits per sample=%d",
               m_PlaybackFormat.Freq, m_PlaybackFormat.Channels, m_PlaybackFormat.Bits);

  if (!m_pSoundDriver->Open(this, &m_PlaybackFormat))
  {
    RecordEvent(SSEL_ERROR, "SoundDriver->Open() failed!");
    return false;
  }

  // The software renderer always works in native endian format.  If the driver changes
  //  the endian format away from the native format, we note that here, but don't pass it down
  //  to the stream and sources.
  if ((m_PlaybackFormat.Flags & CSSNDSYS_SAMPLE_ENDIAN_MASK) == CSSNDSYS_SAMPLE_LITTLE_ENDIAN)
    m_bDriverLittleEndian=true;
  else
    m_bDriverLittleEndian=false;

  // Clear out the flag for byte order and reset to what we'll use in the renderer
  m_PlaybackFormat.Flags&=~(CSSNDSYS_SAMPLE_ENDIAN_MASK);
#ifdef CS_LITTLE_ENDIAN
  m_PlaybackFormat.Flags|=CSSNDSYS_SAMPLE_LITTLE_ENDIAN;
#else
  m_PlaybackFormat.Flags|=CSSNDSYS_SAMPLE_BIG_ENDIAN;
#endif


  // Setup the listener settings
  m_pListener.AttachNew (new SndSysListenerSoftware());

  // TEST - Create the sound normalizer/compressor
  m_pSoundCompressor=new csSoundCompressor(m_PlaybackFormat.Freq * m_PlaybackFormat.Channels / 100);
  m_pSoundCompressor->SetCompressionThreshold(1000);

  m_GlobalVolume = m_Config->GetFloat("SndSys.Volume", 1.0);
  if (m_GlobalVolume>1.0f) m_GlobalVolume = 1.0f;
  if (m_GlobalVolume<0.0f) m_GlobalVolume = 0.0f;

  RecordEvent(SSEL_DEBUG, "Global Volume set to %.2f (0.0 - 1.0)", m_GlobalVolume);

  return m_pSoundDriver->StartThread();
}

void csSndSysRendererSoftware::Close ()
{
  RecordEvent(SSEL_DEBUG, "Close() called.");

  // Halt the background thread
  if (m_pSoundDriver)
  {
    RecordEvent(SSEL_DEBUG, "Halting driver.");
    m_pSoundDriver->StopThread();
    m_pSoundDriver->Close();
  }

  // Clear out all filters
  m_OutputFilterQueue.ClearFilterList();

  // Cleanup any active or pending-active sources
  RemoveAllSources();
  // Cleanup any active or pending-active streams
  RemoveAllStreams();
  // Cleanup any pending-clear sources and streams
  RecordEvent(SSEL_DEBUG, "Garbage collecting.");
  GarbageCollection();
}

//----------------------
//  Filter functions
//----------------------
bool csSndSysRendererSoftware::AddOutputFilter(SndSysFilterLocation Location, iSndSysSoftwareOutputFilter *pFilter)
{
  // Currently we only support the SS_FILTER_LOC_RENDEROUT location through this interface
  if (Location != SS_FILTER_LOC_RENDEROUT)
    return false;

  // Notify the potential filter of our format.  If it doesn't like it, don't add the filter.
  if (!pFilter->FormatNotify(&m_PlaybackFormat))
    return false;

  // Add to the filter list
  m_OutputFilterQueue.AddFilter(pFilter);
  
  return true;
}

bool csSndSysRendererSoftware::RemoveOutputFilter(SndSysFilterLocation Location, iSndSysSoftwareOutputFilter *pFilter)
{
  // Currently we only support the SS_FILTER_LOC_RENDEROUT location through this interface
  if (Location != SS_FILTER_LOC_RENDEROUT)
    return false;

  return m_OutputFilterQueue.RemoveFilter(pFilter);
}

void csSndSysRendererSoftware::ProcessOutputFilters()
{
  m_OutputFilterQueue.DispatchSampleBuffers();

  // Call each source's output filter dispatcher
  size_t MaxIDX=m_DispatchSources.GetSize();
  size_t CurIDX;
  for (CurIDX=0;CurIDX<MaxIDX;CurIDX++)
    m_DispatchSources[CurIDX]->ProcessOutputFilters();
}

void csSndSysRendererSoftware::ProcessStreamDispatch()
{
  size_t MaxIDX=m_DispatchStreams.GetSize();
  size_t CurIDX;
  for (CurIDX=0;CurIDX<MaxIDX;CurIDX++)
    m_DispatchStreams[CurIDX]->ProcessNotifications();
}

//----------------------
// Callback functions
//----------------------

/// Register a component to receive notification of renderer events
bool csSndSysRendererSoftware::RegisterCallback(iSndSysRendererCallback *pCallback)
{
  // Simply add this interface pointer to the list of callbacks
  m_CallbackList.Push(pCallback);
  return true;
}

/// Unregister a previously registered callback component 
bool csSndSysRendererSoftware::UnregisterCallback(iSndSysRendererCallback *pCallback)
{
  // Try to remove the passed interface pointer from the list.  If this fails we return false.
  return m_CallbackList.Delete(pCallback);
}

void csSndSysRendererSoftware::SourceAdded(iSndSysSource *pSource)
{
  size_t IDX, Len;

  if (!pSource)
    return;

  RecordEvent(SSEL_DEBUG, "Queueing source [%s] for add with addr %08x", pSource->GetStream()->GetDescription(), pSource);

  // Call each registered callback with notification
  Len=m_CallbackList.GetSize();
  for (IDX=0;IDX<Len;IDX++)
    m_CallbackList[IDX]->SourceAddNotification(pSource);
}

void csSndSysRendererSoftware::SourceRemoved(iSndSysSource *pSource)
{
  size_t IDX, Len;

  if (!pSource)
    return;

  RecordEvent(SSEL_DEBUG, "Removing source [%s] with refcount=%d", pSource->GetStream()->GetDescription(), pSource->GetRefCount());

  // Call each registered callback with notification
  Len=m_CallbackList.GetSize();
  for (IDX=0;IDX<Len;IDX++)
    m_CallbackList[IDX]->SourceRemoveNotification(pSource);
}

void csSndSysRendererSoftware::StreamAdded(iSndSysStream *pStream)
{
  size_t IDX, Len;

  if (!pStream)
    return;

  RecordEvent(SSEL_DEBUG, "Queueing stream [%s] for add with addr %08x", pStream->GetDescription(), pStream);

  // Call each registered callback with notification
  Len=m_CallbackList.GetSize();
  for (IDX=0;IDX<Len;IDX++)
    m_CallbackList[IDX]->StreamAddNotification(pStream);
}

void csSndSysRendererSoftware::StreamRemoved(iSndSysStream *pStream)
{
  size_t IDX, Len;

  if (!pStream)
    return;

  RecordEvent(SSEL_DEBUG, "Removing stream [%s] with refcount=%d",pStream->GetDescription(), pStream->GetRefCount());

  // Call each registered callback with notification
  Len=m_CallbackList.GetSize();
  for (IDX=0;IDX<Len;IDX++)
    m_CallbackList[IDX]->StreamRemoveNotification(pStream);
}



//----------------------
// 'Maintenance' functions
//----------------------

void csSndSysRendererSoftware::StatusReport()
{
#ifdef CS_DEBUG
  csTicks CurrentTime=csGetTicks();
  if (CurrentTime > m_LastStatusReport + 15000)
  {
    RecordEvent(SSEL_DEBUG,"---RENDERER STATUS REPORT---");
    RecordEvent(SSEL_DEBUG,"Active streams [%d]", m_ActiveStreams.GetSize());
    RecordEvent(SSEL_DEBUG,"Active sources [%d]", m_ActiveSources.GetSize());
    RecordEvent(SSEL_DEBUG,"Source add queue length [%d]", m_SourceAddQueue.Length());
    RecordEvent(SSEL_DEBUG,"Stream add queue length [%d]", m_StreamAddQueue.Length());
    RecordEvent(SSEL_DEBUG,"Source remove queue length [%d]", m_SourceRemoveQueue.Length());
    RecordEvent(SSEL_DEBUG,"Stream remove queue length [%d]", m_StreamRemoveQueue.Length());
    RecordEvent(SSEL_DEBUG,"Current time (csTicks) [%u]", CurrentTime);
    RecordEvent(SSEL_DEBUG,"Last garbage collection time (csTicks) [%u]", m_LastGarbageCollectionTicks);

    m_LastStatusReport=CurrentTime;
  }
#endif // #ifdef CS_DEBUG
}


//      !!WARNING!!  DO NOT CALL THIS FROM THE BACKGROUND THREAD
void csSndSysRendererSoftware::GarbageCollection()
{
  iSndSysSource *sourceptr;
  iSndSysStream *streamptr;

  // Make a pass through the clear queue of streams and cleanup 
  while ((streamptr=m_StreamClearQueue.DequeueEntry(false)) != 0)
  {
    // Notify any callbacks of the removal
    StreamRemoved(streamptr);

    // Remove this stream from the dispatch list as well
    m_DispatchStreams.Delete(streamptr);

    streamptr->DecRef();
  }

  // Make a pass through the clear queue of sources and cleanup 
  while ((sourceptr=m_SourceClearQueue.DequeueEntry(false)) != 0)
  {
    // Notify any callbacks of the removal
    SourceRemoved(sourceptr);

    // Remove this source from the Filter list as well
    m_DispatchSources.Delete(static_cast<iSndSysSourceSoftware *>(sourceptr));

    sourceptr->DecRef();
  }
}

//      !!WARNING!!  DO NOT CALL THIS FROM THE BACKGROUND THREAD
void csSndSysRendererSoftware::RemoveAllSources()
{
  iSndSysSourceSoftware *sourceptr;

  RecordEvent(SSEL_DEBUG, "Clearing all sources.");

  // We will bypass the removal queue since this function is only called when the background thread is
  //  not running
  while (m_ActiveSources.GetSize())
  {
    sourceptr=m_ActiveSources.Get(0);

    // Notify any callbacks of the removal
    SourceRemoved(dynamic_cast<iSndSysSource *>(sourceptr));

    m_ActiveSources.DeleteIndex(0);
    sourceptr->DecRef();
  }
  // Clear out the filter sources list as well
  while (m_DispatchSources.GetSize())
    m_DispatchSources.DeleteIndex(0);

  // Process the addition queue
  while ((sourceptr=m_SourceAddQueue.DequeueEntry(false)) != 0)
  {
    // Notify any callbacks of the removal
    // Although these sources never entered the renderer's background list, notification of
    // addition was sent at creation time
    SourceRemoved(dynamic_cast<iSndSysSource *>(sourceptr));

    sourceptr->DecRef();
  }

  // The clear queue will be processed by GarbageCollection()

}

//      !!WARNING!!  DO NOT CALL THIS FROM THE BACKGROUND THREAD
void csSndSysRendererSoftware::RemoveAllStreams()
{
  iSndSysStream *streamptr;

  RecordEvent(SSEL_DEBUG, "Clearing all streams.");

  // We will bypass the removal queue since this function is only called when the background thread is
  //  not running
  while (m_ActiveStreams.GetSize())
  {
    streamptr=m_ActiveStreams.Get(0);

    // Notify any callbacks of the removal
    StreamRemoved(streamptr);

    m_ActiveStreams.DeleteIndex(0);
    streamptr->DecRef();
  }
  // Clear out the dispatch list as well
  while (m_DispatchStreams.GetSize())
    m_DispatchStreams.DeleteIndex(0);

  // Process the addition queue
  while ((streamptr=m_StreamAddQueue.DequeueEntry(false)) != 0)
  {
    // Notify any callbacks of the removal
    // Although these streams never entered the renderer's background list, notification of
    // addition was sent at creation time
    StreamRemoved(streamptr);

    streamptr->DecRef();
  }

  // The clear queue will be processed by GarbageCollection()
}

void csSndSysRendererSoftware::ProcessPendingSources()
{
  iSndSysSourceSoftware *src;

  // This function is where the sound system thread handles adding and removing
  //  sources from the active set.

  // Empty the queue of source addition requests from the application
  while ((src=m_SourceAddQueue.DequeueEntry()))
  {
    RecordEvent(SSEL_DEBUG, "Found a queued source to add to the active list.");
    m_ActiveSources.Push(src);
  }

  // Empty the queue of source removal requests
  while ((src=static_cast<iSndSysSourceSoftware *> (m_SourceRemoveQueue.DequeueEntry())))
  {
    // We don't know that the application has queued a pointer to a source that's
    //  active.  If the Delete() fails, then the source is not even in our active list.
    if (m_ActiveSources.Delete(src))
    {
      RecordEvent(SSEL_DEBUG, "Processing remove request for source addr [%08x]", src);
      // The source is removed from our active list.  Now queue it back to the foreground thread
      //  so that the reference count can be decreased.
      m_SourceClearQueue.QueueEntry(dynamic_cast<iSndSysSource *>(src));
    }
    else
      RecordEvent(SSEL_WARNING, "Failed remove request for source addr [%08x]. Source not in active list.", src);
  }
}

void csSndSysRendererSoftware::ProcessPendingStreams()
{
  iSndSysStream *stream;

  // This function is where the sound system thread handles adding and removing
  //  streams from the active set.

  // Empty the queue of stream addition requests from the application
  while ((stream=m_StreamAddQueue.DequeueEntry()))
  {
    RecordEvent(SSEL_DEBUG, "Found a queued stream to add to the active list.");
    m_ActiveStreams.Push(stream);
  }

  // Empty the queue of stream removal requests
  while ((stream=m_StreamRemoveQueue.DequeueEntry()))
  {
    // We don't know that the application has queued a pointer to a stream that's
    //  active.  If the Delete() fails, then the stream is not even in our active list.
    if (m_ActiveStreams.Delete(stream))
    {
      RecordEvent(SSEL_DEBUG, "Processing remove request for stream addr [%08x]", stream);
      // The stream is removed from our active list.  Now queue it back to the foreground thread
      //  so that the reference count can be decreased.
      m_StreamClearQueue.QueueEntry(stream);
    }
    else
      RecordEvent(SSEL_WARNING, "Failed remove request for stream addr [%08x]. Stream not in active list.", stream);
  }
}


//------------------------------------
// Sound processing functions
//------------------------------------

void csSndSysRendererSoftware::AdvanceStreams(size_t Frames)
{
  size_t currentidx;

  // Iterate through the list of streams. Each stream will be examined to see if it's
  //  an auto-unregister stream that is not playing (paused).  If so, the stream and
  //  its associated source will be queued for cleanup.
  //
  // This isn't as bad as it seems - it's really not practical to have a lot of streams
  //  playing simultaneously, so these arrays should be fairly short.
  size_t maxidx=m_ActiveStreams.GetSize();
  for (currentidx=0;currentidx<maxidx;currentidx++)
  {
    // Search for any streams that have an unregister requested and are paused and mark them for removal in the next cycle
    iSndSysStream *str=m_ActiveStreams.Get(currentidx);

    // If the stream associated with the source has processed all the data, pause it
    if (str->GetPauseState() == CS_SNDSYS_STREAM_COMPLETED)
    {
      str->Pause();
    }

    if ((str->GetPauseState() == CS_SNDSYS_STREAM_PAUSED)
      && (str->GetAutoUnregisterRequested() == true))
    {
      // This stream is being removed.  Search for any sources associated with it
      //  and remove them as well.
      size_t currentsource, maxsource;
      maxsource = m_ActiveSources.GetSize();
      for (currentsource = 0; currentsource < maxsource; currentsource++)
      {
        csRef<iSndSysSource> currentsourceinterface;
        if ( ( currentsourceinterface = scfQueryInterface<iSndSysSource>(m_ActiveSources.Get(currentsource)) ) && ( currentsourceinterface->GetStream() == str ) )
        {
          // This source is associated with the removed stream
          RecordEvent(SSEL_DEBUG, "Marked source index [%d] for removal due to AutoUnregistered stream.", currentsource);
          // Mark the source for removal
          m_SourceRemoveQueue.QueueEntry (currentsourceinterface);
        }
      }
      // Now that all sources have been removed, queue the stream for cleanup
      //  Note that from this point on the stream MUST NOT be accessed by the
      //  background renderer thread
      m_StreamRemoveQueue.QueueEntry(str);
      continue;
    }

    // This stream is still playing, advance its position
    str->AdvancePosition(Frames);
  }
}

size_t csSndSysRendererSoftware::FillDriverBuffer(void *buf1, size_t buf1_frames,
						  void *buf2, size_t buf2_frames)
{
  // Update queued listener property changes
  m_pListener->UpdateQueuedProperties();

  // Handle any pending source or stream additions or removals
  ProcessPendingSources();
  ProcessPendingStreams();

  // Report some information about the current status of the renderer system
  StatusReport();

  // Resize the samplebuffer if needed
  size_t needed_frames=(buf1_frames+buf2_frames);

  if ((m_pSampleBuffer==0) || (needed_frames > m_SampleBufferFrames))
  {
    RecordEvent(SSEL_DEBUG, "Sample buffer too small. Have [%u frames] Need [%u frames]. Allocating.", m_SampleBufferFrames, needed_frames);
	//asm ("int $3");
    delete[] m_pSampleBuffer;
    m_pSampleBuffer=new csSoundSample[needed_frames * m_PlaybackFormat.Channels];
    m_SampleBufferFrames = needed_frames;
  }

  // Clear as much of the buffer as we need
  memset(m_pSampleBuffer,0,sizeof(csSoundSample) * needed_frames * m_PlaybackFormat.Channels);

  // Advance all active streams based on the number of requested frames
  //  This call also queues completed auto-unregister streams for cleanup
  AdvanceStreams(needed_frames);

  // Mix all the sources
  size_t maxidx=m_ActiveSources.GetSize();
  size_t currentidx;
  for (currentidx=0;currentidx<maxidx;currentidx++)
  {
    size_t provided_frames;
    //Report (CS_REPORTER_SEVERITY_DEBUG,
      //"Requesting %d samples from source.", needed_samples);

    // The return from this function is this number of samples actually available
    provided_frames = m_ActiveSources.Get(currentidx)->MergeIntoBuffer (m_pSampleBuffer, needed_frames);

    if (provided_frames==0)
    {
      RecordEvent(SSEL_DEBUG, "Source index [%d] provided 0 frames.", currentidx);
    }
    else
    {
      CS_ASSERT (provided_frames <= needed_frames);

      RecordEvent(SSEL_DEBUG, "Source index [%d] provided [%d] out of [%d] requested frames.", currentidx,
        provided_frames, needed_frames);

      // BUG: I think if a later source returns less samples than requested than we are throwing away samples from the earlier source
      //      that we will never get back, aren't we?  - A.M.
      //
      // FIX: (maybe) We may need a way to shift the source play cursor backwards - after we find the minimum number of available samples from
      //      all sources, if it's less than initially requested, then we iterate over the sources again, shifting the cursor back.

      // Reduce the number of samples we request from future sources, as well as the number we provide to the driver
      needed_frames=provided_frames;
    }
  }


  // Normalize the sample buffer
  NormalizeSampleBuffer(needed_frames * m_PlaybackFormat.Channels);
//  m_pSoundCompressor->ApplyCompression(m_pSampleBuffer, needed_frames * m_PlaybackFormat.Channels);


  if (m_OutputFilterQueue.GetOutputFilterCount()>0)
    m_OutputFilterQueue.QueueSampleBuffer(m_pSampleBuffer, needed_frames, m_PlaybackFormat.Channels);

  // Copy normalized data to the driver
  CopySampleBufferToDriverBuffer (buf1, buf1_frames * m_PlaybackFormat.Channels * m_PlaybackFormat.Bits/8,
    buf2, buf2_frames * m_PlaybackFormat.Channels * m_PlaybackFormat.Bits/8, needed_frames);

  //csTicks end_ticks=csGetTicks();
//  Report (CS_REPORTER_SEVERITY_DEBUG, "Processing time: %u ticks.", end_ticks-current_ticks);


  return needed_frames;
}

void csSndSysRendererSoftware::NormalizeSampleBuffer(size_t used_samples)
{
  size_t sample_idx;
  csSoundSample maxintensity=0;
  csSoundSample desiredintensity = (csSoundSample)(0x7FFF * m_GlobalVolume);
  csSoundSample low_threshold;
  uint32 multiplier;

  if (desiredintensity> 0x7FFF)
    desiredintensity=0x7FFF;
  desiredintensity=desiredintensity<<16;

  // First scan, find the max abs sample value
  for (sample_idx=0;sample_idx<used_samples;sample_idx++)
  {
    csSoundSample abssamp=m_pSampleBuffer[sample_idx];
    if (abssamp<0) abssamp=-abssamp;
    if (abssamp > maxintensity) maxintensity=abssamp;
  }

  RecordEvent(SSEL_DEBUG, "Maximum sample intensity is %d", maxintensity);

  // Silence fills the buffer, also dividing by zero is bad
  if (maxintensity==0)
    return;

  if (m_PlaybackFormat.Bits==8)
    low_threshold=127;
  else
    low_threshold=32767;
  if (maxintensity<low_threshold)
    maxintensity=low_threshold;

  RecordEvent(SSEL_DEBUG, "Maximum sample intensity (clamped) is %d", maxintensity);

  multiplier=desiredintensity / maxintensity;


  // Clamp the intensity multiplier to about 3% +/- the last value
  if (m_LastIntensityMultiplier>0)
  {
    uint32 acceptable_range=m_LastIntensityMultiplier/32;

    //Report (CS_REPORTER_SEVERITY_DEBUG, "last intensity multiplier is %u", m_LastIntensityMultiplier);

    if (multiplier > m_LastIntensityMultiplier + acceptable_range)
      multiplier=m_LastIntensityMultiplier+acceptable_range;
    if (multiplier < m_LastIntensityMultiplier - acceptable_range)
      multiplier=m_LastIntensityMultiplier-acceptable_range;


    // Recalculate the maximum intensity
    maxintensity=desiredintensity/multiplier;
  }
  m_LastIntensityMultiplier=multiplier;

  //Report (CS_REPORTER_SEVERITY_DEBUG, "Multiplier is %u", multiplier);

  // Second scan, multiply the values to create 32 bit samples with a max near 0x7FFFFFFF
  for (sample_idx=0;sample_idx<used_samples;sample_idx++)
  {
    // Clamp to max
    if (m_pSampleBuffer[sample_idx] > maxintensity)
      m_pSampleBuffer[sample_idx]=desiredintensity;
    else if (m_pSampleBuffer[sample_idx] < -maxintensity)
      m_pSampleBuffer[sample_idx]=-desiredintensity;
    else
      m_pSampleBuffer[sample_idx]*=multiplier;
  }
}


void csSndSysRendererSoftware::CopySampleBufferToDriverBuffer (void *drvbuf1,
							       size_t drvbuf1_len,
							       void *drvbuf2,
							       size_t drvbuf2_len,
							       size_t samples_per_channel)
{
  csSoundSample *ptr=m_pSampleBuffer;

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
  if (m_PlaybackFormat.Bits==8)
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
    if (m_bDriverLittleEndian)
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


//--------------------
// Utility functions
//--------------------

void csSndSysRendererSoftware::RecordEvent(SndSysEventCategory Category, SndSysEventLevel Severity, const char* msg, ...)
{
  if (!m_pEventRecorder)
    return;

  va_list arg;
  va_start (arg, msg);
  m_pEventRecorder->RecordEventV(Category, Severity, msg, arg);
  va_end (arg);
}

void csSndSysRendererSoftware::RecordEvent(SndSysEventLevel Severity, const char* msg, ...)
{
  if (!m_pEventRecorder)
    return;

  va_list arg;
  va_start (arg, msg);
  m_pEventRecorder->RecordEventV(SSEC_RENDERER, Severity, msg, arg);
  va_end (arg);
}


void csSndSysRendererSoftware::Report(int severity, const char* msg, ...)
{
  va_list arg;

  // Anything that goes to the reporter should automatically going to the recorder too
  if (m_pEventRecorder)
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
    m_pEventRecorder->RecordEventV(SSEC_RENDERER, Level, msg, arg);
    va_end (arg);
  }

  // To the reporter (console)
  va_start (arg, msg);
  csReportV (m_pObjectRegistry, severity,
    "crystalspace.sndsys.renderer.software", msg, arg);
  va_end (arg);
}

