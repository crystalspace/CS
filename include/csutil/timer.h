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
class csEventTimer : public iEventTimer
{
private:
  iObjectRegistry* object_reg;
  csArray<timerevent> timerevents;
  csRef<iEventHandler> handler;
  csRef<iVirtualClock> vc;

  int FindTimerEvent (iTimerEvent* ev);

  // Optimization: to prevent having to loop over all timer events
  // all the time we keep the minimum time needed before the first
  // event is triggered.
  csTicks minimum_time;

public:
  csEventTimer (iObjectRegistry* object_reg);
  virtual ~csEventTimer ();

  bool HandleEvent (iEvent& e);

  SCF_DECLARE_IBASE;

  virtual void AddTimerEvent (iTimerEvent* ev, csTicks delay);
  virtual void RemoveTimerEvent (iTimerEvent* ev);
  virtual void RemoveAllTimerEvents ();
};

#endif // __CS_TIMER_H__

