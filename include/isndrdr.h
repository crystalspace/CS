/*
    Copyright (C) 1998, 1999 by Nathaniel 'NooTe' Saint Martin
    Copyright (C) 1998, 1999 by Jorrit Tyberghein
    Written by Nathaniel 'NooTe' Saint Martin

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

#ifndef __ISNDRDR_H__
#define __ISNDRDR_H__

#include "iplugin.h"
#include "isndsrc.h"

struct iSoundListener;
struct iSoundData;
struct iSoundStream;
struct csSoundFormat;

SCF_VERSION (iSoundRender, 1, 0, 0);

/**
 * The sound renderer is used to play previously loaded sounds or music.
 * Loading itself is NOT done through this interface. <p>
 *
 * Sounds may be played as non-3d directly with PlaySound(). If you want
 * more control (for example, stop the sound at any time) or if you want 3d
 * sound you have to create a sound source. Sources can be 3d or non-3d,
 * where non-3d sources simply ignore the position and velocity control
 * methods of iSoundSource. <p>
 *
 * The renderer can play sound data and sound stream objects. You should
 * read the docs for them to understand the template-instance relation
 * between them. This basically means that you can create any number of
 * sound sources from a sound data object, but only one source from a sound
 * stream.
 */
struct iSoundRender : public iPlugIn
{
public:
  /// Open the sound render
  virtual bool Open () = 0;
  /// Close the sound render
  virtual void Close () = 0;

  /// Set Volume [0, 1]
  virtual void SetVolume (float vol) = 0;
  /// Get Volume [0, 1]
  virtual float GetVolume () = 0;

  /**
   * Play a sound without further control. This is a convenience function
   * to create and play a new sound stream.
   */
  virtual void PlaySound(iSoundData *Data, bool Loop = false) = 0;
  /**
   * Play a sound without further control. Note: No other part of your
   * program should read data from the sound stream while it is playing.
   */
  virtual void PlaySound(iSoundStream *Sound, bool Loop = false) = 0;

  /**
   * Create a sound source. This is a convenience function
   * to create a new sound stream and attach it to a new sound source. Mode3D
   * can be one of SOUND_HEAD, SOUND_RELATIVE or SOUND_3D.
   */
  virtual iSoundSource *CreateSource(iSoundData *Sound, int Mode3D) = 0;
  /**
   * Create a sound source and attach the given sound stream. Note: No other
   * part of your program should read data from the sound stream as long as
   * the sound source exists. Mode3D can be one of SOUND_HEAD, SOUND_RELATIVE
   * or SOUND_3D.
   */
  virtual iSoundSource *CreateSource(iSoundStream *Sound, int Mode3D) = 0;

  /// Get the global Listener object
  virtual iSoundListener *GetListener () = 0;

  /// return the sound format to load a sound
  virtual const csSoundFormat *GetLoadFormat() = 0;

  /// update the renderer (must be called regularly).
  virtual void Update() = 0;
  /// Internal use : mixing function (needed if your renderer uses a driver)
  virtual void MixingFunction () = 0;
};

#endif
