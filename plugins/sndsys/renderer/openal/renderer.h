/*
	Copyright (C) 2006 by Søren Bøg

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

#ifndef SNDSYS_RENDERER_OPENAL_RENDERER_H
#define SNDSYS_RENDERER_OPENAL_RENDERER_H

#if defined(CS_OPENAL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENAL_PATH,alc.h)
#else
#include <AL/alc.h>
#endif

#include "cssysdef.h"

#include "csutil/cfgacc.h"
#include "csutil/scf_implementation.h"
#include "csutil/threading/thread.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

#include "csutil/array.h"
#include "csutil/hash.h"
#include "csutil/refarr.h"

#include "isndsys/ss_structs.h"
#include "isndsys/ss_renderer.h"
#include "isndsys/ss_eventrecorder.h"

#include "csplugincommon/sndsys/queue.h"

class csSndSysRendererOpenAL : 
  public scfImplementation4<csSndSysRendererOpenAL, iComponent, iEventHandler,  iSndSysRenderer, iSndSysRendererOpenAL>
{
public:
  csSndSysRendererOpenAL(iBase *piBase);
  virtual ~csSndSysRendererOpenAL();

  /*
   * iComponent interface
   */
public:
  virtual bool Initialize (iObjectRegistry *obj_reg);

  /*
   * iEventHandler interface
   */
public:
  virtual bool HandleEvent (iEvent &e);
  CS_EVENTHANDLER_NAMES("crystalspace.sndsys.renderer")
  CS_EVENTHANDLER_NIL_CONSTRAINTS

  /*
   * iSndSysRenderer interface
   */
public:
  /// Set Volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual void SetVolume (float vol);

  /// Get Volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual float GetVolume ();

  /**
   * Uses the provided iSound2Data to create a sound stream with the given
   * 3D rendering mode
   */
  virtual csPtr<iSndSysStream> CreateStream(iSndSysData* data,
  	int mode3d);

  /// Creates a source when provided with a Sound Stream
  virtual csPtr<iSndSysSource> CreateSource(iSndSysStream* stream);

  /// Remove a stream from the sound renderer's list of streams
  virtual bool RemoveStream(iSndSysStream* stream);

  /// Remove a source from the sound renderer's list of sources
  virtual bool RemoveSource(iSndSysSource* source);

  /// Get the global Listener object
  virtual csRef<iSndSysListener> GetListener ();

  /// Register a component to receive notification of renderer events
  virtual bool RegisterCallback(iSndSysRendererCallback *pCallback);

  /// Unregister a previously registered callback component 
  virtual bool UnregisterCallback(iSndSysRendererCallback *pCallback);

  /*
   * iSndSysRendererOpenAL interface
   */
public:
  /** Requests the use of the renderer's OpenAL context.
   *
   * @note Should only be used internally by the OpenAL renderer.
   * @note Should be matched by a call to Release.
   */
  virtual bool LockWait();

  /** Releases the use of the renderer's OpenAL context.
   *
   * @note Should only be used internally by the OpenAL renderer.
   */
  virtual void Release();

  /** \name * OpenAL extension support.
   * @{ */
  bool extAL_EXT_MCFORMATS;
  /** @} */

  /*
   * csSndSysRendererOpenAL implementation
   */
private:
  /// The OpenAL device for this renderer
  ALCdevice *m_Device;
  /// The OpenAL context for this renderer
  ALCcontext *m_Context;
  /// Mutex to lock the OpenAL context
  CS::Threading::RecursiveMutex m_ContextLock;

  /// The unique listener object for this renderer
  csRef<SndSysListenerOpenAL> m_Listener;

  /// Local reference to the object registry.
  iObjectRegistry *m_ObjectRegistry;

  /// Configuration access
  csConfigAccess m_Config;

  /// ID of the 'Open' event fired on system startup
  csEventID evSystemOpen;
  /// ID of the 'Close' event fired on system shutdown
  csEventID evSystemClose;
  /// ID of the 'Frame' event fired once each frame
  csEventID evFrame;

  /// Array of attached streams
  csRefArray<iSndSysStream> m_Streams;
  /// Array of attached sources
  csRefArray<SndSysSourceOpenAL2D> m_Sources;
  /// Array of attached callbacks
  csRefArray<iSndSysRendererCallback> m_Callback;

  // Helper functions
  /// Reporting helper function
  void Report (int severity, const char* msg, ...);

  /// Update everything (renderer, sources, etc.)
  void Update ();

  /// Perform final initialization
  void Open ();

  /// Perform finalization
  void Close ();

  /// Queries available extensions
  void QueryExtensions ();
};

#endif // #ifndef SNDSYS_RENDERER_OPENAL_RENDERER_H
