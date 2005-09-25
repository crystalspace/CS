/*
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_IUTIL_TIMER_H__
#define __CS_IUTIL_TIMER_H__

/**\file
 * Timer event interfaces
 */

#include "csutil/scf.h"


SCF_VERSION (iTimerEvent, 0, 0, 1);

/**
 * A timer event.
 */
struct iTimerEvent : public iBase
{
  /**
   * Perform the event. If this returns true the event is re-scheduled.
   * Otherwise the event will be removed from the event timer.
   */
  virtual bool Perform (iTimerEvent* ev) = 0;
};

SCF_VERSION (iEventTimer, 0, 0, 1);

/**
 * A timer. You can add operations to it and they
 * will be performed at the dedicated time.
 */
struct iEventTimer : public iBase
{
  /// Add a timer event to be scheduled later.
  virtual void AddTimerEvent (iTimerEvent* ev, csTicks delay) = 0;

  /// Remove a timer event.
  virtual void RemoveTimerEvent (iTimerEvent* ev) = 0;

  /// Clear all timer events.
  virtual void RemoveAllTimerEvents () = 0;

  /// Query the number of events still in the queue.
  virtual size_t GetEventCount () const = 0;

  /// Query the number of ticks before the specified event fires.
  virtual csTicks GetTimeLeft (size_t idx) const = 0;
};

#endif // __CS_IUTIL_TIMER_H__

