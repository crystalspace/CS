/*
    Copyright (C) 2004 by Andrew Mann
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef __CS_SNDSYS_RENDERER_H__
#define __CS_SNDSYS_RENDERER_H__

/**\file
 * Sound system: renderer
 */

#include "csutil/scf.h"
#include "isndsys/ss_filter.h"

/**\addtogroup sndsys
 * @{ */

struct csSndSysSoundFormat;
struct iSndSysData;
struct iSndSysStream;
struct iSndSysSource;
struct iSndSysListener;
struct iSndSysRendererCallback;

#ifndef CS_SNDSYS_SOURCE_DISTANCE_INFINITE
#define CS_SNDSYS_SOURCE_DISTANCE_INFINITE -1.0f
#endif

#define CS_SNDSYS_SOURCE_STOPPED    0
#define CS_SNDSYS_SOURCE_PLAYING    1


/**
 * The sound renderer is the core interface for the sound system. It maintains
 * any global state associated with the sound system. It is also the interface
 * through which instances of sound steams, sources and the listener can be
 * retrieved or created.
 *
 * \todo
 *   Should Sound Streams get processing time even if no Sound Sources are 
 *   attached?
 */
struct iSndSysRenderer : public virtual iBase
{
  SCF_INTERFACE(iSndSysRenderer,0,2,1);

  /// Set Volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual void SetVolume (float vol) = 0;

  /// Get Volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual float GetVolume () = 0;

  /**
   * Uses the provided iSound2Data to create a sound stream with the given
   * 3D rendering mode
   */
  virtual csPtr<iSndSysStream> CreateStream(iSndSysData* data,
  	int mode3d) = 0;

  /// Creates a source when provided with a Sound Stream
  virtual csPtr<iSndSysSource> CreateSource(iSndSysStream* stream) = 0;

  /// Remove a stream from the sound renderer's list of streams
  virtual bool RemoveStream(iSndSysStream* stream) = 0;

  /// Remove a source from the sound renderer's list of sources
  virtual bool RemoveSource(iSndSysSource* source) = 0;

  /// Get the global Listener object
  virtual csRef<iSndSysListener> GetListener () = 0;

  /// Register a component to receive notification of renderer events
  virtual bool RegisterCallback(iSndSysRendererCallback *pCallback) = 0;

  /// Unregister a previously registered callback component 
  virtual bool UnregisterCallback(iSndSysRendererCallback *pCallback) = 0;
};


/** Sound System renderer interface for callback notification
 *
 * A component wishing to receive notification of Sound Renderer events
 * should implement this interface, and call iSndSysRenderer::RegisterCallback()
 * to register with the renderer.
 */
struct iSndSysRendererCallback : public virtual iBase
{
  SCF_INTERFACE(iSndSysRendererCallback,0,1,0);

  /// Called whenever a stream is added to the system
  virtual void StreamAddNotification(iSndSysStream *pStream) = 0;

  /// Called whenever a stream is removed from the system
  virtual void StreamRemoveNotification(iSndSysStream *pStream) = 0;

  /// Called whenever a source is added to the system
  virtual void SourceAddNotification(iSndSysSource *pSource) = 0;

  /// Called whenever a source is removed to the system
  virtual void SourceRemoveNotification(iSndSysSource *pSource) = 0;
};


/// Software renderer specific interface extensions
struct iSndSysRendererSoftware : public virtual iBase
{
  SCF_INTERFACE(iSndSysRendererSoftware,0,1,1);

  /// Add an output filter at the specified location.
  //  Output filters can only receive sound data and cannot modify it.  They will receive data
  //   from the same thread that the CS event handler executes in, once per frame.
  //
  //  Valid Locations:  SS_FILTER_LOC_RENDEROUT
  //  
  //  Returns FALSE if the filter could not be added.
  virtual bool AddOutputFilter(SndSysFilterLocation Location, iSndSysSoftwareOutputFilter *pFilter) = 0;

  /// Remove an output filter from the registered list
  //
  //  Valid Locations:  SS_FILTER_LOC_RENDEROUT
  //
  // Returns FALSE if the filter is not in the list at the time of the call.
  virtual bool RemoveOutputFilter(SndSysFilterLocation Location, iSndSysSoftwareOutputFilter *pFilter) = 0;
};


/// OpenAL renderer specific interface extensions
struct iSndSysRendererOpenAL : public virtual iBase
{
  SCF_INTERFACE(iSndSysRendererOpenAL,0,1,1);

  /** Requests the use of the renderers OpenAL context.
   *
   * @note Should only be used internally by the OpenAL renderer.
   * @note Should be matched by a call to ReleaseContext.
   */
  virtual bool LockWait() = 0;

  /** Releases the use of the renderers OpenAL context.
   *
   * @note Should only be used internally by the OpenAL renderer.
   */
  virtual void Release() = 0;
};


/** @} */

#endif // __CS_SNDSYS_RENDERER_H__
