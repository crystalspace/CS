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

#ifndef __ISOUNDDRIVER_H__
#define __ISOUNDDRIVER_H__

#include "cscom/com.h"
interface ISystem;
interface ISoundRender;

extern const GUID IID_ISoundDriver;

/**
 * This is the sound render interface for CS.
 * All sound renders must implement this interface.
 * The standard implementation is ISoundDriver.
 */

interface ISoundDriver : public IUnknown
{
public:

  /// Open the sound render
  STDMETHOD (Open) (ISoundRender *render, int frequency, bool bit16, bool stereo) PURE;
  /// Close the sound render
  STDMETHOD (Close) () PURE;
  /// Set Volume [0, 1]
  STDMETHOD (SetVolume) (float vol) PURE;
  /// Get Volume [0, 1]
  STDMETHOD (GetVolume) (float *vol) PURE;
  /// Lock and Get Sound Memory Buffer
  STDMETHOD (LockMemory) (void **mem, int *memsize) PURE;
  /// Unlock Sound Memory Buffer
  STDMETHOD (UnlockMemory) () PURE;
  /// Is driver need to be updated 'manually' ?
  STDMETHOD (IsBackground) (bool *back) PURE;
  /// Do driver is in 16 bits mode ?
  STDMETHOD (Is16Bits) (bool *bit) PURE;
  /// Do driver is in stereo mode ?
  STDMETHOD (IsStereo) (bool *stereo) PURE;
  /// get current frequency of driver
  STDMETHOD (GetFrequency) (int *freq) PURE;
  /// Is driver have it's own handler for no sound data else soundrender fill memory
  STDMETHOD (IsHandleVoidSound) (bool *handle) PURE;
};

extern const IID IID_ISoundDriverFactory;

interface ISoundDriverFactory : public IUnknown
{
  ///
  STDMETHOD (CreateInstance) (REFIID riid, ISystem * piSystem, void **ppv) PURE;

  /// Lock or unlock from memory.
  STDMETHOD (LockServer) (BOOL bLock) PURE;
};

#endif //__ISOUNDDRIVER_H__
