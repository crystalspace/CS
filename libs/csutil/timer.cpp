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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cssysdef.h"
#include "csutil/timer.h"
#include "iutil/event.h"
#include "iutil/evdefs.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/virtclk.h"

struct timerevent
{
  csRef<iTimerEvent> event;
  csTicks delay;
  csTicks time_left;
};

//-----------------------------------------------------------------------

class csTimerEventHandler : public iEventHandler
{
private:
  csEventTimer* timer;

public:
  csTimerEventHandler (csEventTimer* timer)
  {
    SCF_CONSTRUCT_IBASE (0);
    csTimerEventHandler::timer = timer;
  }
  SCF_DECLARE_IBASE;
  virtual bool HandleEvent (iEvent& e)
  {
    return timer->HandleEvent (e);
  }
};

SCF_IMPLEMENT_IBASE (csTimerEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

//-----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csEventTimer)
  SCF_IMPLEMENTS_INTERFACE (iEventTimer)
SCF_IMPLEMENT_IBASE_END

csEventTimer::csEventTimer (iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE (0);
  handler = csPtr<iEventHandler> (new csTimerEventHandler (this));
  csRef<iEventQueue> q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
  CS_ASSERT (q != 0);
  if (q != 0)
    q->RegisterListener (handler, CSMASK_Nothing);
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  CS_ASSERT (vc != 0);

  minimum_time = 2000000000;
}

csEventTimer::~csEventTimer ()
{
  if (handler)
  {
    csRef<iEventQueue> q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q != 0)
      q->RemoveListener (handler);
    handler = 0;
  }
}

bool csEventTimer::HandleEvent (iEvent& event)
{
  if (event.Type != csevBroadcast
   || event.Command.Code != cscmdFinalProcess)
    return false;

  csTicks elapsed = vc->GetElapsedTicks ();
  minimum_time -= elapsed;

  if (minimum_time > 0) return true;

  minimum_time = 2000000000;
  int i;
  for (i = 0 ; i < timerevents.Length () ; i++)
  {
    timerevent& te = timerevents[i];
    te.time_left -= elapsed;
    if (te.time_left <= 0)
    {
      if (te.event->Perform (te.event))
      {
        te.time_left = te.delay;
	if (te.time_left < minimum_time) minimum_time = te.time_left;
      }
      else
      {
        timerevents.DeleteIndex (i);
	i--;
      }
    }
    else
    {
      if (te.time_left < minimum_time) minimum_time = te.time_left;
    }
  }

  return true;
}

int csEventTimer::FindTimerEvent (iTimerEvent* ev)
{
  int i;
  for (i = 0 ; i < timerevents.Length () ; i++)
  {
    if (timerevents[i].event == ev)
      return i;
  }
  return -1;
}

void csEventTimer::AddTimerEvent (iTimerEvent* ev, csTicks delay)
{
  timerevent te;
  te.event = ev;
  te.delay = delay;
  te.time_left = delay;
  timerevents.Push (te);
  if (delay < minimum_time) minimum_time = delay;
}

void csEventTimer::RemoveTimerEvent (iTimerEvent* ev)
{
  int idx = FindTimerEvent (ev);
  if (idx != -1)
    timerevents.DeleteIndex (idx);
}

void csEventTimer::RemoveAllTimerEvents ()
{
  timerevents.DeleteAll ();
  minimum_time = 2000000000;
}

