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

#include "cscom/com.h"
interface ISystem;

extern const GUID IID_ISoundRender;

/**
 * This is the sound render interface for CS.
 * All sound renders must implement this interface.
 * The standard implementation is ISoundDriver.
 */

interface ISoundListener;
interface ISoundSource;
class csSoundBuffer;

interface ISoundRender : public IUnknown
{
public:
  /// Open the sound render
  STDMETHOD (Open) () PURE;
  /// Close the sound render
  STDMETHOD (Close) () PURE;
  /// Update Sound Render
  STDMETHOD (Update) () PURE;
  /// Set Volume [0, 1]
  STDMETHOD (SetVolume) (float vol) PURE;
  /// Get Volume [0, 1]
  STDMETHOD (GetVolume) (float *vol) PURE;
  /// Play a sound buffer without control (it's play until his end)
  STDMETHOD (PlayEphemeral) (csSoundBuffer *snd) PURE;
  /// Create a Listener object
  STDMETHOD (GetListener) (ISoundListener ** ppv ) PURE;
  /// Create a Source object
  STDMETHOD (CreateSource) (ISoundSource ** ppv, csSoundBuffer *snd) PURE;
  /// Internal use : mixing function (need if your render use a driver)
  STDMETHOD (MixingFunction) () PURE;
};

extern const IID IID_ISoundRenderFactory;

interface ISoundRenderFactory : public IUnknown
{
  ///
  STDMETHOD (CreateInstance) (REFIID riid, ISystem * piSystem, void **ppv) PURE;

  /// Lock or unlock from memory.
  STDMETHOD (LockServer) (COMBOOL bLock) PURE;
};

#endif //__ISOUNDRENDER_H__
