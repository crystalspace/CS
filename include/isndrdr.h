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

#ifndef __ISOUNDRENDER_H__
#define __ISOUNDRENDER_H__

#include "csutil/scf.h"

/**
 * This is the sound render interface for CS.
 * All sound renders must implement this interface.
 * The standard implementation is ISoundDriver.
 */

scfInterface iSoundListener;
scfInterface iSoundSource;
scfInterface iSoundBuffer;
class csSoundData;

SCF_INTERFACE (iSoundRender, 0, 0, 1) : public iBase
{
public:
  /// Open the sound render
  virtual bool Open () = 0;
  /// Close the sound render
  virtual void Close () = 0;
  /// Update Sound Render
  virtual void Update () = 0;
  /// Set Volume [0, 1]
  virtual void SetVolume (float vol) = 0;
  /// Get Volume [0, 1]
  virtual float GetVolume () = 0;
  /// Play a sound buffer without control (it's play until his end)
  virtual void PlayEphemeral (csSoundData *snd) = 0;
  /// Create a Listener object
  virtual iSoundListener *GetListener () = 0;
  /// Create a Source object
  virtual iSoundSource *CreateSource (csSoundData *snd) = 0;
  /// Create a controled SoundBuffer object
  virtual iSoundBuffer *CreateSoundBuffer (csSoundData *snd) = 0;
  /// Internal use : mixing function (need if your render use a driver)
  virtual void MixingFunction () = 0;
};

#endif //__ISOUNDRENDER_H__
