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

#ifndef __ISNDDRV_H__
#define __ISNDDRV_H__

#include "csutil/scf.h"
#include "iplugin.h"

struct iSoundRender;

SCF_VERSION (iSoundDriver, 0, 0, 1);

/**
 * This is the sound render interface for CS.
 * All sound renders must implement this interface.
 * The standard implementation is ISoundDriver.
 */
struct iSoundDriver : public iPlugIn
{
  /// Open the sound render
  virtual bool Open (iSoundRender *render, int frequency, bool bit16, bool stereo) = 0;
  /// Close the sound render
  virtual void Close () = 0;
  /// Lock and Get Sound Memory Buffer
  virtual void LockMemory (void **mem, int *memsize) = 0;
  /// Unlock Sound Memory Buffer
  virtual void UnlockMemory () = 0;
  /// Is driver need to be updated 'manually' ?
  virtual bool IsBackground () = 0;
  /// Do driver is in 16 bits mode ?
  virtual bool Is16Bits () = 0;
  /// Do driver is in stereo mode ?
  virtual bool IsStereo () = 0;
  /// get current frequency of driver
  virtual int GetFrequency () = 0;
  /// Is driver have it's own handler for no sound data else soundrender fill memory
  virtual bool IsHandleVoidSound () = 0;
};

#endif
