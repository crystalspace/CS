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

#ifndef __CS_ISOUND_DRIVER_H__
#define __CS_ISOUND_DRIVER_H__

#include "csutil/scf.h"

struct iSoundRender;

SCF_VERSION (iSoundDriver, 0, 0, 1);

/**
 * This is the interface for the low-level, system-dependent sound driver
 * that is used by the software sound renderer. The sound driver is
 * responsible for playing a single stream of samples.
 */
struct iSoundDriver : public iBase
{
  /// Open the sound render
  virtual bool Open(iSoundRender*, int frequency, bool bit16, bool stereo) = 0;
  /// Close the sound render
  virtual void Close () = 0;
  /// Lock and Get Sound Memory Buffer
  virtual void LockMemory (void **mem, int *memsize) = 0;
  /// Unlock Sound Memory Buffer
  virtual void UnlockMemory () = 0;
  /// Must the driver be updated manually or does it run in background?
  virtual bool IsBackground () = 0;
  /// Is the driver in 16 bits mode ?
  virtual bool Is16Bits () = 0;
  /// Is the driver in stereo mode ?
  virtual bool IsStereo () = 0;
  /// Get current frequency of driver
  virtual int GetFrequency () = 0;
  /**
   * Is the sound driver able to create silence without locking and
   * writing to the sound memory?
   */
  virtual bool IsHandleVoidSound () = 0;
  // @@@ temporary
  virtual bool ThreadAware (){ return false; }
};

#endif // __CS_ISOUND_DRIVER_H__
