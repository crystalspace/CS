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

#ifndef __CS_ISOUND_RENDERER_H__
#define __CS_ISOUND_RENDERER_H__

#include "csutil/scf.h"

struct iSoundListener;
struct iSoundData;
struct iSoundHandle;

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
 * Before you can play a sound, you must first register it and get a sound
 * handle. <p>
 */
struct iSoundRender : public iBase
{
public:
  /// Set Volume (range from 0.0 to 1.0)
  virtual void SetVolume (float vol) = 0;
  /// Get Volume (range from 0.0 to 1.0)
  virtual float GetVolume () = 0;

  /// Register a sound
  virtual csPtr<iSoundHandle> RegisterSound(iSoundData *) = 0;
  /// Unregister a sound
  virtual void UnregisterSound(iSoundHandle *) = 0;

  /// Get the global Listener object
  virtual iSoundListener *GetListener () = 0;

  /// Internal use : mixing function (needed if your renderer uses a driver)
  virtual void MixingFunction () = 0;
};

#endif // __CS_ISOUND_RENDERER_H__
