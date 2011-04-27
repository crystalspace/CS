/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_IUTIL_VIRTCLK_H__
#define __CS_IUTIL_VIRTCLK_H__

/**\file
 * Virtual clock interface
 */
#include "csutil/scf_interface.h"

/**
 * A virtual game clock.  Normally, a single instance of iVirtualClock is
 * placed in the shared object registry (iObjectRegistry).
 * 
 * Main creators of instances implementing this interface:
 * - csInitializer::CreateEnvironment()
 * - csInitializer::CreateVirtualClock()
 * 
 * Main ways to get pointers to this interface:
 * - csQueryRegistry()
 */
struct iVirtualClock : public virtual iBase
{
  SCF_INTERFACE(iVirtualClock, 2,0,0);
  /**
   * Advance the engine's virtual-time clock.
   */
  virtual void Advance () = 0;

  /**
   * Suspend the engine's virtual-time clock.<p>
   * Call this function when the client application will fail to call Advance()
   * for an extended period of time.  Suspending the virtual-time clock
   * prevents a temporal anomaly from occurring the next time GetElapsedTicks()
   * is called after the application resumes invoking Advance().
   */
  virtual void Suspend () = 0;

  /**
   * Resume the engine's virtual-time clock.<p>
   * Call this function when the client application begins invoking Advance()
   * again after extended down-time.  This function is the complement of
   * Suspend().
   */
  virtual void Resume () = 0;

  /**
   * Query the time elapsed between the two most recent invocations of
   * Advance().
   */
  virtual csTicks GetElapsedTicks () const = 0;

  /**
   * Returns the absolute time of the last call to Advance().
   * For game logic you should always use this function instead of
   * csGetTicks().
   */
  virtual csTicks GetCurrentTicks () const = 0;
};

#endif // __CS_IUTIL_VIRTCLK_H__
