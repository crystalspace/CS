/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __CS_SNDHDLDS3D_H__
#define __CS_SNDHDLDS3D_H__

#include "csutil/thread.h"
#include "csplugincommon/soundrenderer/shdl.h"

class csSoundRenderDS3D;

class csSoundHandleDS3D : public csSoundHandle
{
public:
  csRef<csSoundRenderDS3D> SoundRender;
  long NumSamples;
  void *buffer;
  long buffer_writecursor;
  long buffer_length;

  /**
   * The writecursor needs to be read by new sources to syncronize to the 
   * current position.
   */
  csRef<csMutex> mutex_WriteCursor;

  /**
   * The constructor for the Direct Sound 3D plugin's Sound Handle also takes 
   * the Buffer Length in seconds for streaming data sources.
   */
  csSoundHandleDS3D(csSoundRenderDS3D* srdr, iSoundData* snd, 
    float BufferLengthSeconds, bool LocalBuffer);
  // destructor
  ~csSoundHandleDS3D();

  void Unregister();
  virtual void vUpdate(void *buf, long NumSamples);

  /// Create a source to eminate this sound from
  virtual csPtr<iSoundSource> CreateSource(int Mode3d);

  /**
   * Overriden because it calls UpdateCount and UpdateCount has altered 
   * functionality
   */
  void Update_Time(csTicks ElapsedTime);
  /** 
   * Overriden.  Uses a source as a timer unless there are no playing sources, 
   * then uses the passed count.
   */
  void UpdateCount(long NumSamples);
  /// Implimented to perform local buffer fills if needed
  virtual void StartStream(bool Loop);
  /// Returns the play cursor position in the virtual sound buffer
  long GetPlayCursorPosition();

};

#endif // __CS_SNDHDLDS3D_H__
