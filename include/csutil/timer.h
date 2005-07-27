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

#ifndef __CS_TIMER_H__
#define __CS_TIMER_H__

#include <stdarg.h>
#include <stdio.h>
#include "csextern.h"
#include "csutil/array.h"
#include "iutil/timer.h"

struct iEvent;
struct iObjectRegistry;
struct iVirtualClock;
struct iEventHandler;
struct timerevent;

/**
 * This class implements a timer. You can add operations to it and they
 * will be performed at the dedicated time.
 */
class CS_CRYSTALSPACE_EXPORT csEventTimer : public iEventTimer
{
private:
  iObjectRegistry* object_reg;
  csArray<timerevent> timerevents;
  iEventHandler* handler;
  csRef<iVirtualClock> vc;

  size_t FindTimerEvent (iTimerEvent* ev);

  // Optimization: to prevent having to loop over all timer events
  // all the time we keep the minimum time needed before the first
  // event is triggered.
  int32 minimum_time;
  int32 accumulate_elapsed;

public:
  csEventTimer (iObjectRegistry* object_reg);
  virtual ~csEventTimer ();

  bool HandleEvent (iEvent& e);

  SCF_DECLARE_IBASE;

  virtual void AddTimerEvent (iTimerEvent* ev, csTicks delay);
  virtual void RemoveTimerEvent (iTimerEvent* ev);
  virtual void RemoveAllTimerEvents ();

  virtual size_t GetEventCount () const { return timerevents.Length (); }
  virtual csTicks GetTimeLeft (size_t idx) const;

  /**
   * This is a static method to easily get the standard
   * global timer (name 'crystalspace.timer.standard' in the object
   * registry). If that timer doesn't exist yet it will be created.
   */
  static csPtr<iEventTimer> GetStandardTimer (iObjectRegistry* object_reg);
};

#endif // __CS_TIMER_H__

