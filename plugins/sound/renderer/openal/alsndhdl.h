/*
    Copyright (C) 2002 by Jorrit Tyberghein, Daniel Duhprey

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

#ifndef __CS_SNDHDLOPENAL_H__
#define __CS_SNDHDLOPENAL_H__

#include "csutil/thread.h"
#include "csplugincommon/soundrenderer/shdl.h"

class csSoundRenderOpenAL;
class csSoundHandleOpenAL : public csSoundHandle
{
  csRef<csSoundRenderOpenAL> SoundRender;
 

public:
  // constructor
  csSoundHandleOpenAL(csSoundRenderOpenAL *srdr, iSoundData *snd,float BufferLengthSeconds,bool LocalBuffer);
  // destructor
  ~csSoundHandleOpenAL();

  void Update_Time(csTicks ElapsedTime); 
  void UpdateCount(long NumSamples);

  /// Implimented to perform local buffer fills if needed
  virtual void StartStream(bool Loop);
  /// Returns the play cursor position in the virtual sound buffer
  long GetPlayCursorPosition();

  csPtr<iSoundSource> CreateSource(int m);
  void vUpdate (void *buf, long samples);

  void *local_buffer;
  long buffer_length;
  long NumSamples;
  long buffer_writecursor;

  /// The writecursor needs to be read by new sources to syncronize to the current position and updated by the background thread.
  csRef<csMutex> mutex_WriteCursor;

  
};

#endif // __CS_SNDHDLOPENAL_H__
