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


#ifndef SNDSYS_RENDERER_SOFTWARE_RENDERER_H
#define SNDSYS_RENDERER_SOFTWARE_RENDERER_H

#include "csutil/cfgacc.h"
#include "csutil/csstring.h"
#include "csutil/scf_implementation.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csgeom/vector3.h"

#include "csutil/array.h"

#include "isndsys/ss_structs.h"
#include "isndsys/ss_renderer.h"
#include "isndsys/ss_eventrecorder.h"

#include "csplugincommon/sndsys/queue.h"

#include "filterqueue.h"

#include "compressor.h"


using namespace CS::SndSys;


#define MAX_CHANNELS 18

// see http://www.microsoft.com/whdc/device/audio/multichaud.mspx

#define SPEAKER_FRONT_LEFT                0x00000001
#define SPEAKER_FRONT_RIGHT               0x00000002
#define SPEAKER_FRONT_CENTER              0x00000004
#define SPEAKER_LOW_FREQUENCY             0x00000008
#define SPEAKER_BACK_LEFT                 0x00000010
#define SPEAKER_BACK_RIGHT                0x00000020
#define SPEAKER_FRONT_LEFT_OF_CENTER      0x00000040
#define SPEAKER_FRONT_RIGHT_OF_CENTER     0x00000080
#define SPEAKER_BACK_CENTER               0x00000100
#define SPEAKER_SIDE_LEFT                 0x00000200
#define SPEAKER_SIDE_RIGHT                0x00000400
#define SPEAKER_TOP_CENTER                0x00000800
#define SPEAKER_TOP_FRONT_LEFT            0x00001000
#define SPEAKER_TOP_FRONT_CENTER          0x00002000
#define SPEAKER_TOP_FRONT_RIGHT           0x00004000
#define SPEAKER_TOP_BACK_LEFT             0x00008000
#define SPEAKER_TOP_BACK_CENTER           0x00010000
#define SPEAKER_TOP_BACK_RIGHT            0x00020000


struct st_speaker_properties
{
  csVector3 RelativePosition;
  csVector3 Direction;
  float min_factor,max_factor;
};

struct iSndSysSoftwareDriver;
struct iConfigFile;
class SndSysListenerSoftware;
class SndSysSourceSoftware;
struct iSndSysSourceSoftware;
struct iReporter;

using namespace CS::SndSys;

class csSndSysRendererSoftware : 
  public scfImplementation4<csSndSysRendererSoftware, iComponent, iEventHandler,  iSndSysRenderer, iSndSysRendererSoftware>
{
public:
  csSndSysRendererSoftware(iBase *piBase);
  virtual ~csSndSysRendererSoftware();

  /// Called by the driver thread to request sound data
  virtual size_t FillDriverBuffer(void *buf1, size_t buf1_len,
    void *buf2, size_t buf2_len);

  /// Send a message to the sound system event recorder
  //   This version may be called by other components that have a reference to the renderer
  //   so that they dont need to hold an EventRecorder reference themselves (sources for example)
  void RecordEvent(SndSysEventCategory Category, SndSysEventLevel Severity, const char* msg, ...);


  /// Local copy of the pointer to the object registry interface
  iObjectRegistry *m_pObjectRegistry;

  /// The global listener object which controls how sound is perceived
  csRef<SndSysListenerSoftware> m_pListener;

  /// The sample format used by the software renderer for audio output
  csSndSysSoundFormat m_PlaybackFormat;

  // TODO: Move to listener
  struct st_speaker_properties m_Speakers[MAX_CHANNELS];

protected:
  /// Interface to the Configuration file
  csConfigAccess m_Config;

  /// Interface to the low level sound driver
  csRef<iSndSysSoftwareDriver> m_pSoundDriver;

  /// Global volume setting
  float m_GlobalVolume;

  /** The thread safe queue containing pointers to sources which should be added 
   *    to the active list.
   *
   *  Entries are added to this queue only in the CreateSource() interface function
   *   and we know (really!) that the sources are all of type iSndSysSourceSoftware.
   *  Entries in this queue are removed and processed by ProcessPendingSources()
  */
  Queue<iSndSysSourceSoftware> m_SourceAddQueue;

  /** The thread safe queue containing pointers to sources which should be removed 
   *    from the active list.
   *
   *  Entries are added to this queue in RemoveSource() or AdvanceStreams().
   *  Entries in this queue are removed and processed by ProcessPendingSources()
  */
  Queue<iSndSysSource> m_SourceRemoveQueue;

  /** The thread safe queue containing pointers to sources which have been removed 
   *    from the active list and should be cleaned up.
   *
   *  Entries are added to this queue in ProcessPendingSources() after being removed
   *    from the Remove queue.
   *  Entries in this queue are removed and decref'd by the GarbageCollection()
   *    function.
  */
  Queue<iSndSysSource> m_SourceClearQueue;

  /// The list of active sources.  
  //  Active sources do not necessarily always produce audible output.
  //  
  //  Do not change this to a reference counted array.  Reference counting is not thread safe.
  csArray<iSndSysSourceSoftware *> m_ActiveSources;

  /// This is the list of active sources as held by the foreground thread for the purpose of filter dispatching.
  //
  //  Do not access this member from the background thread.  
  //
  csRefArray<iSndSysSourceSoftware> m_DispatchSources;

  /// This is the list of active streams as held by the foreground thread for the purpose of notification dispatching.
  //
  //  Do not access this member from the background thread.  
  //
  csRefArray<iSndSysStream> m_DispatchStreams;



  /** The thread safe queue containing pointers to streams which should be added 
  *    to the active list.
  *
  *  Entries are added to this queue in the CreateStream() interface function.
  *  Entries in this queue are removed and processed by ProcessPendingStreams()
  */
  Queue<iSndSysStream> m_StreamAddQueue;

  /** The thread safe queue containing pointers to streams which should be removed 
  *    from the active list.
  *
  *  Entries are added to this queue in RemoveStream() or AdvanceStreams().
  *  Entries in this queue are removed and processed by ProcessPendingStreams()
  */
  Queue<iSndSysStream> m_StreamRemoveQueue;

  /** The thread safe queue containing pointers to streams which have been removed 
  *    from the active list and should be cleaned up.
  *
  *  Entries are added to this queue in ProcessPendingStreams() after being removed
  *    from the Remove queue.
  *  Entries in this queue are removed and decref'd by the GarbageCollection()
  *    function.
  */
  Queue<iSndSysStream> m_StreamClearQueue;

  /// The list of active streams.  
  //  Active streams do not necessarily always produce audible output.
  //  
  //  Do not change this to a reference counted array.  Reference counting is not thread safe.
  csArray<iSndSysStream *> m_ActiveStreams;


  /// Pointer to a buffer of sound samples used to mix data prior to sending to the driver
  csSoundSample *m_pSampleBuffer;

  /// Size of the sample buffer in frames of audio
  size_t m_SampleBufferFrames;

  /// ID of the 'Open' event fired on system startup
  csEventID evSystemOpen;
  /// ID of the 'Close' event fired on system shutdown
  csEventID evSystemClose;
  /// ID of the 'Frame' event fired once each frame
  csEventID evFrame;

  /// The last time (in csTicks) that the source/stream garbage collection process was run
  csTicks m_LastGarbageCollectionTicks;

  /// Set to true if the driver expects little endian data
  bool m_bDriverLittleEndian;

  /// TODO: This should probably be combined into a buffer normalization class
  //     along with the Normalize routine
  uint32 m_LastIntensityMultiplier;

  /// Testing normalization interface
  csSoundCompressor *m_pSoundCompressor;

#ifdef CS_DEBUG
  /// The last time status information was reported
  csTicks m_LastStatusReport;
#endif

  /// The event recorder interface, if active
  csRef<iSndSysEventRecorder> m_pEventRecorder;

  /// The output filter queue.
  //  This stores any attached filters as well as the buffers queued for delivery to the filters
  SndSysOutputFilterQueue m_OutputFilterQueue;

  /// Stores the list of components that have registered for callback notification
  csRefArray<iSndSysRendererCallback> m_CallbackList;

  csRef<iEventHandler> weakEventHandler;
  csRef<iEventHandler> weakOpenEventHandler;

protected:
  // Called when the renderer plugin is opened from the HandleEvent function
  bool Open();

  // Called when the renderer plugin is closed from the HandleEvent function
  void Close();

  /// Send a message to the console reporter
  void Report (int severity, const char* msg, ...);

  /// Performs cleanup operations on removed sources and streams
  //      !!WARNING!!  DO NOT CALL THIS FROM THE BACKGROUND THREAD
  void GarbageCollection();

  /// Shutdown procedure.  This removes all sources from the renderer's lists.
  //      !!WARNING!!  DO NOT CALL THIS FROM THE BACKGROUND THREAD
  void RemoveAllSources();

  /// Shutdown procedure.  This removes all streams from the renderer's lists.
  //      !!WARNING!!  DO NOT CALL THIS FROM THE BACKGROUND THREAD
  void RemoveAllStreams();

  /// Send a message to the sound system event recorder as the renderer
  void RecordEvent(SndSysEventLevel Severity, const char* msg, ...);

  /// Report current renderer status via RecordEvent
  //   This function only has a body in debug mode
  void StatusReport();

  /// Called when a source is added to the sound renderer
  void SourceAdded(iSndSysSource *pSource);
  /// Called when a source is removed from the sound renderer
  void SourceRemoved(iSndSysSource *pSource);
  /// Called when a stream is added to the sound renderer
  void StreamAdded(iSndSysStream *pStream);
  /// Called when a stream is removed from the sound renderer
  void StreamRemoved(iSndSysStream *pStream);

  /// Advance active streams in time equal to the provided renderer audio frame count
  //   This function also handles auto-unregister streams which have completed playback
  //   by moving them to a cleanup queue.
  void AdvanceStreams(size_t Frames);

  /// Process the addition and removal queues for sources. 
  //  This should only be called from the background thread. 
  //
  //  This function processes requests (usually from the application running in 
  //   the main thread) to add sources to the system or remove active sources 
  //   from the system.
  void ProcessPendingSources();

  /// Process the addition and removal queues for streams. 
  //  This should only be called from the background thread. 
  //
  //  This function processes requests (usually from the application running in 
  //   the main thread) to add streams to the system or remove active streams 
  //   from the system.
  void ProcessPendingStreams();

  /// Called to dequeue output sound buffers and send them to the output filters
  void ProcessOutputFilters();

  void ClearOutputFilters();

  /// Called to provide processing time from the main application thread for streams
  //    which have pending notification events
  void ProcessStreamDispatch();

  void NormalizeSampleBuffer(size_t used_samples);
  void CopySampleBufferToDriverBuffer(void *drvbuf1,size_t drvbuf1_len,
    void *drvbuf2, size_t drvbuf2_len, size_t samples_per_channel);
  /// This copies to a single buffer, called up to twice
  csSoundSample *CopySampleBufferToDriverBuffer(void *drvbuf, 
    size_t drvbuf_len, csSoundSample *src, size_t samples_per_channel);
  

  ////
  // Interface implementation
  ////

  //------------------------
  // iComponent
  //------------------------
public:
  virtual bool Initialize (iObjectRegistry *obj_reg);

  //------------------------
  // iEventHandler
  //------------------------
public:
  virtual bool HandleEvent (iEvent &e);
  CS_EVENTHANDLER_NAMES("crystalspace.sndsys.renderer")
  CS_EVENTHANDLER_NIL_CONSTRAINTS

  //--------------------------
  // iSndSysRendererSoftware
  //--------------------------

  /// Add an output filter at the specified location.
  //  Output filters can only receive sound data and cannot modify it.  They will receive data
  //   from the same thread that the CS event handler executes in, once per frame.
  //
  //  Valid Locations:  SS_FILTER_LOC_RENDEROUT
  //  
  //  Returns FALSE if the filter could not be added.
  virtual bool AddOutputFilter(SndSysFilterLocation Location, iSndSysSoftwareOutputFilter *pFilter);

  /// Remove an output filter from the registered list
  //
  //  Valid Locations:  SS_FILTER_LOC_RENDEROUT
  //
  // Returns FALSE if the filter is not in the list at the time of the call.
  virtual bool RemoveOutputFilter(SndSysFilterLocation Location, iSndSysSoftwareOutputFilter *pFilter);

  //------------------------
  // iSndSysRenderer
  //------------------------
public:
  /// Set m_GlobalVolume (range from 0.0 to 1.0)
  virtual void SetVolume (float vol);

  /// Get m_GlobalVolume (range from 0.0 to 1.0)
  virtual float GetVolume ();

  /**
  * Uses the provided iSndSysData to create a sound stream with the given
  * 3D rendering mode
  */
  virtual csPtr<iSndSysStream> CreateStream(iSndSysData* data, int mode3d);

  /// Creates a source when provided with a Sound Stream
  virtual csPtr<iSndSysSource> CreateSource(iSndSysStream* stream);

  /// Remove a stream from the sound renderer's list of streams
  virtual bool RemoveStream(iSndSysStream* stream);

  /// Remove a source from the sound renderer's list of sources
  virtual bool RemoveSource(iSndSysSource* source);

  /// Get the global listener object
  virtual csRef<iSndSysListener> GetListener ();

  /// Register a component to receive notification of renderer events
  virtual bool RegisterCallback(iSndSysRendererCallback *pCallback);

  /// Unregister a previously registered callback component 
  virtual bool UnregisterCallback(iSndSysRendererCallback *pCallback);
};

#endif // #ifndef SNDSYS_RENDERER_SOFTWARE_RENDERER_H



