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

#ifndef SNDSYS_RENDERER_H
#define SNDSYS_RENDERER_H

#include "csutil/scf.h"

struct SndSysSoundFormat;
struct iSndSysData;
struct iSndSysStream;
struct iSndSysSource;
struct iSndSysListener;

SCF_VERSION (iSndSysRenderer, 0, 1, 0);


#define ISNDSYS_SOURCE_DISTANCE_INFINITE -1.0f


#define ISNDSYS_SOURCE_STOPPED    0
#define ISNDSYS_SOURCE_PLAYING    1


/**
 * 
 *
 *  TODO:  Should Sound Streams get processing time even if no Sound Sources are attached?
 *
 *
 */
struct iSndSysRenderer : public iBase
{
  /// Set Volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual void SetVolume (float vol) = 0;

  /// Get Volume (range 0.0 = silence 1.0 = as provided 2.0 = twice as loud)
  virtual float GetVolume () = 0;

  /**
   * Uses the provided iSound2Data to create a sound stream with the given
   * 3D rendering mode
   */
  virtual csPtr<iSndSysStream> CreateStream(csRef<iSndSysData> data,
  	int mode3d) = 0;

  /// Creates a source when provided with a Sound Stream
  virtual csPtr<iSndSysSource> CreateSource(csRef<iSndSysStream> stream) = 0;

  /// Remove a stream from the sound renderer's list of streams
  virtual bool RemoveStream(csRef<iSndSysStream> stream) = 0;

  /// Remove a source from the sound renderer's list of sources
  virtual bool RemoveSource(csRef<iSndSysSource> source) = 0;

  /// Get the global Listener object
  virtual csRef<iSndSysListener> GetListener () = 0;

  /// 
};

#endif // #ifndef SNDSYS_RENDERER_H


