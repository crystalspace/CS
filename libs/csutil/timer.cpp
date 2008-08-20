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

#include "cssysdef.h"
#include "csutil/timer.h"
#include "csutil/event.h"
#include "csutil/csevent.h"
#include "iutil/event.h"
#include "iutil/evdefs.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/virtclk.h"

//-----------------------------------------------------------------------

class csTimerEventHandler : public scfImplementation1<csTimerEventHandler,
                                                      iEventHandler>
{
private:
  csEventTimer* timer;

public:
  csTimerEventHandler (csEventTimer* timer)
    : scfImplementationType (this), timer (timer)
  {
  }
  virtual ~csTimerEventHandler()
  {
  }
  virtual bool HandleEvent (iEvent& e)
  {
    return timer->HandleEvent (e);
  }
  CS_EVENTHANDLER_PHASE_FRAME("crystalspace.timer")
};


//-----------------------------------------------------------------------


csEventTimer::csEventTimer (iObjectRegistry* object_reg) : 
  scfImplementationType (this), 
  object_reg (object_reg),
  Frame (csevFrame (object_reg))
{
  csRef<iEventQueue> q = csQueryRegistry<iEventQueue> (object_reg);
  CS_ASSERT (q != 0);
  if (q != 0)
  {
    handler = new csTimerEventHandler (this);
    q->RegisterListener (handler, Frame);

    handler->DecRef ();
  }
  else
  {
    handler = 0;
  }
  vc = csQueryRegistry<iVirtualClock> (object_reg);
  CS_ASSERT (vc != 0);

  minimum_time = 2000000000;
  accumulate_elapsed = 0;
}

csEventTimer::~csEventTimer ()
{
  if (handler)
  {
    // Jorrit: if I enable this then I get a crash at exit because
    // the object registry is already being destructed. No idea how I
    // should solve this...
    // @@@@@
    //csRef<iEventQueue> q = csQueryRegistry<iEventQueue> (object_reg);
    //if (q != 0)
      //q->RemoveListener (handler);
  }
}

bool csEventTimer::HandleEvent (iEvent& event)
{
  (void)event; // unused except for this assert so silence the warning
  CS_ASSERT(event.Name == Frame);

  csTicks elapsed = vc->GetElapsedTicks ();
  minimum_time -= elapsed;

  if (minimum_time > 0)
  {
    accumulate_elapsed += elapsed;
    return true;
  }

  minimum_time = 2000000000;
  size_t i;
  for (i = timerevents.GetSize () ; i > 0 ; i--)
  {
    const size_t idx = i - 1;
    timerevent& te = timerevents[idx];
    te.time_left -= elapsed+accumulate_elapsed;
    if (te.time_left <= 0)
    {
      if (te.event->Perform (te.event))
      {
        te.time_left = te.delay;
	if (te.time_left < minimum_time) minimum_time = te.time_left;
      }
      else
      {
        timerevents.DeleteIndex (idx);
	//i--;
      }
    }
    else
    {
      if (te.time_left < minimum_time) minimum_time = te.time_left;
    }
  }
  accumulate_elapsed = 0;

  return true;
}

csTicks csEventTimer::GetTimeLeft (size_t idx) const
{
  return csTicks (timerevents[idx].time_left-accumulate_elapsed);
}

size_t csEventTimer::FindTimerEvent (iTimerEvent* ev)
{
  size_t i;
  for (i = 0 ; i < timerevents.GetSize () ; i++)
  {
    if (timerevents[i].event == ev)
      return i;
  }
  return (size_t)-1;
}

void csEventTimer::AddTimerEvent (iTimerEvent* ev, csTicks delay)
{
  timerevent te;
  te.event = ev;
  te.delay = delay;
  te.time_left = delay;
  timerevents.Push (te);
  if (minimum_time == 2000000000)
  {
    // Minimum time was equal to infinite. In that case we
    // have to clear accumulate_elapsed.
    minimum_time = delay;
    accumulate_elapsed = 0;
  }
  else if (delay < (csTicks)minimum_time) minimum_time = delay;
}

void csEventTimer::RemoveTimerEvent (iTimerEvent* ev)
{
  size_t idx = FindTimerEvent (ev);
  if (idx != (size_t)-1)
    timerevents.DeleteIndex (idx);
}

void csEventTimer::RemoveAllTimerEvents ()
{
  timerevents.DeleteAll ();
  minimum_time = 2000000000;
  accumulate_elapsed = 0;
}

csPtr<iEventTimer> csEventTimer::GetStandardTimer (iObjectRegistry* object_reg)
{
  csRef<iEventTimer> timer = csQueryRegistryTagInterface<iEventTimer>
  	(object_reg, "crystalspace.timer.standard");
  if (!timer)
  {
    timer = csPtr<iEventTimer> (new csEventTimer (object_reg));
    object_reg->Register (timer, "crystalspace.timer.standard");
  }
  return csPtr<iEventTimer> (timer);
}

