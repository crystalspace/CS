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

#ifndef __CS_VIRTCLK_H__
#define __CS_VIRTCLK_H__

/**\file
 * Implementation of iVirtualClock
 */

#include "csextern.h"
#include "csutil/scf_implementation.h"
#include "iutil/virtclk.h"

/**
 * This is an implementation of a virtual clock. Using this
 * clock you can easily keep track of elapsed and current time
 * in a virtual setting.
 */
class CS_CRYSTALSPACE_EXPORT csVirtualClock : 
  public scfImplementation1<csVirtualClock, iVirtualClock>
{
private:
  /// Virtual clock state flags
  enum 
  { 
    /// The virtual clock is suspended
    flagSuspended = 1,
    /**
     * The virtual clock did not advance yet, means on the first call to
     * Advance(), the start time needs to be taken, and no elapsed time
     * is reported.
     */
    flagFirstShot = 2 
  };
  /// Elapsed time between last two frames
  csTicks elapsedTime;
  /// Virtual time in milliseconds
  csTicks currentVirtualTime;
  /// Absolute time in milliseconds
  csTicks currentRealTime;

  /// Elapsed time between last two frames
  csMicroTicks elapsedTimeM;
  /// Virtual time in milliseconds
  csMicroTicks currentVirtualTimeM;
  /// Absolute time in milliseconds
  csMicroTicks currentRealTimeM;
  uint flags;

  float elapsedSeconds;
  bool elapsedSecondsValid;

public:
  csVirtualClock ();
  virtual ~csVirtualClock ();

  virtual void Advance ();
  virtual void Suspend ();
  virtual void Resume ();
  virtual csTicks GetElapsedTicks () const { return elapsedTime; }
  virtual csTicks GetCurrentTicks () const { return currentVirtualTime; }
  virtual csMicroTicks GetElapsedMicroTicks () const { return elapsedTimeM; }
  virtual float GetElapsedSeconds ();
  virtual csMicroTicks GetCurrentMicroTicks () const { return currentVirtualTimeM; }
};

#endif // __CS_VIRTCLK_H__

