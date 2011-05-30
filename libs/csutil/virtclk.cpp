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

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "csutil/virtclk.h"
#include "iutil/virtclk.h"

csVirtualClock::csVirtualClock () : 
  scfImplementationType (this),
  elapsedTime (0), currentVirtualTime (0), currentRealTime (0),
  elapsedTimeM (0), currentVirtualTimeM (0), currentRealTimeM (0),
  flags (flagFirstShot), elapsedSecondsValid (false)
{ }

csVirtualClock::~csVirtualClock () { }

void csVirtualClock::Advance ()
{
  if (flags & flagSuspended) return;

  csTicks last = currentRealTime;
  csMicroTicks lastM = currentRealTimeM;

  currentRealTime = csGetTicks ();
  currentRealTimeM = csGetMicroTicks ();

  if (flags & flagFirstShot)
  {
    flags &= ~flagFirstShot;
    elapsedTime = 0;
    elapsedTimeM = 0;
  }
  else
  {
    if (currentRealTimeM < lastM)
      // csMicroTicks(-1) is the period for a unsigned value
      elapsedTimeM = currentRealTimeM + (csMicroTicks(-1) - lastM) + 1;
    else
      elapsedTimeM = currentRealTimeM - lastM;
    currentVirtualTimeM += elapsedTimeM;

    if (currentRealTime < last)
      // csMicroTicks(-1) is the period for a unsigned value
      elapsedTime = currentRealTime + (csTicks(-1) - last) + 1;
    else
      elapsedTime = currentRealTime - last;
    currentVirtualTime += elapsedTime;
  }
  elapsedSecondsValid = false;
}

void csVirtualClock::Suspend ()
{
  flags |= flagSuspended;
  elapsedTime = 0;
  elapsedTimeM = 0;
}

void csVirtualClock::Resume ()
{
  if (flags & flagSuspended)
  {
    flags &= ~flagSuspended;
    flags |= flagFirstShot;
  }
}

float csVirtualClock::GetElapsedSeconds ()
{
  if (!elapsedSecondsValid)
  {
    elapsedSecondsValid = true;
    elapsedSeconds = float (elapsedTimeM) / 1000000.0f;
  }
  return elapsedSeconds;
}

