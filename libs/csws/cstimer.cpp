/*
    Crystal Space Windowing System: timer class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>
    Copyright (C) 2002 by Mat Sutcliffe <oktal@gmx.co.uk>

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
#include "csws/cstimer.h"
#include "csws/csapp.h"
#include "iutil/event.h"
#include "csutil/csevent.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/evdefs.h"

#include <time.h>

SCF_IMPLEMENT_IBASE (csTimer::csTimerEvent)
  SCF_IMPLEMENTS_INTERFACE (iEvent)
SCF_IMPLEMENT_IBASE_END

void csTimer::Init (unsigned iPeriod)
{
  TimerEvent.Time = 0;
  TimerEvent.Type = csevCommand;
  TimerEvent.Command.Code = cscmdTimerPulse;
  eventh = 0;
  evento = 0;
  state |= CSS_TRANSPARENT;
  timeout = iPeriod;
  pausetime = 0;
  Restart ();
}

csTimer::csTimer (csComponent *iParent, unsigned iPeriod)
  : csComponent (iParent)
{
  Init (iPeriod);
}

csTimer::csTimer (iEventHandler *iEventH, unsigned iPeriod, intptr_t iInfo)
  : csComponent (0)
{
  Init (iPeriod);
  eventh = iEventH;
  TimerEvent.Command.Info = iInfo;
}

csTimer::csTimer (iEventQueue *iEventQ, unsigned iPeriod, intptr_t iInfo)
  : csComponent (0)
{
  Init (iPeriod);
  evento = iEventQ->GetEventOutlet ();
  TimerEvent.Command.Info = iInfo;
}

bool csTimer::HandleEvent (iEvent &Event)
{
  if (!Stopped
   && (Event.Type == csevBroadcast)
   && (Event.Command.Code == cscmdPreProcess))
  {
    unsigned current = app ? app->GetCurrentTime () : time (0);
    unsigned delta = current - start;
    if (pausetime >= delta)
      return false;
    pausetime = 0;
    if (delta >= timeout)
    {
      if (parent)
        parent->SendCommand (cscmdTimerPulse, (intptr_t)this);
      else if (eventh)
        eventh->HandleEvent (TimerEvent);
      else if (evento)
        evento->Post (& TimerEvent);
      // if we're not too far behind, switch to next pulse
      // otherwise we'll have to jump far to the current time
      start += timeout;
      if (current - start >= timeout)
        start = current;
    } /* endif */
  } /* endif */
  return false;
}

void csTimer::Stop ()
{
  Stopped = true;
}

void csTimer::Restart ()
{
  Stopped = false;
  start = app ? app->GetCurrentTime () : time (0);
}

void csTimer::Pause (unsigned iPause)
{
  pausetime = iPause;
  Restart ();
}
