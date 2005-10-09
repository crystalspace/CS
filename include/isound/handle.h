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

#ifndef __CS_ISOUND_HANDLE_H__
#define __CS_ISOUND_HANDLE_H__

#include "csutil/scf_interface.h"
#include "csutil/ref.h"

struct iSoundSource;


/**
 * The sound handle is a sound in the state that is needed by the sound
 * renderer. It can be used to create instances of the sound, the sound
 * sources (the Play() method is only a convenience method to create and
 * play a sound source). <p>
 *
 * Sounds can be either static or streamed, depending on the sound data the
 * handle is created from. There is a basic difference between these two modes
 * how they are used: You can create any number of totally independent sound
 * sources from a static sound, but not from a streamed sound. All sources
 * that are created from the same streamed sound *always* play the same
 * sequence of samples, if they play sound at all. So the Play() and Stop()
 * methods of a streamed sound source only set the source to active or
 * quiet mode, but they don't control the position that is currently played.
 * Instead this position is controlled with the sound handle interface.
 */
struct iSoundHandle : public virtual iBase
{
  SCF_INTERFACE(iSoundHandle, 2,0,0);
  /// is this a static or streamed handle?
  virtual bool IsStatic() = 0;

  /// play an instance of this sound
  /// For Loop == true it returns a iSoundSource you have to Stop ()
  /// if you want to get rid of the looping sound
  /// (also if you want to unload your SoundRenderer)
  /// If Loop is false 0 is returned
  virtual csPtr<iSoundSource> Play(bool Loop = false) = 0;
  /// create a sound source
  virtual csPtr<iSoundSource> CreateSource(int Mode3d) = 0;

  /// Start playing the stream (only for streamed sound)
  virtual void StartStream(bool Loop) = 0;
  /// Stop playing the stream (only for streamed sound)
  virtual void StopStream() = 0;
  /// Reset the stream to the beginning (only for streamed sound)
  virtual void ResetStream() = 0;
};

#endif // __CS_ISOUND_HANDLE_H__
