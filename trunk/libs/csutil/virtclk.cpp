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
  scfImplementationType (this), elapsedTime (0), currentVirtualTime (0), 
  currentRealTime(0), flags (flagFirstShot)
{ }

csVirtualClock::~csVirtualClock () { }

void csVirtualClock::Advance ()
{
  if (flags & flagSuspended) return;
  csTicks last = currentRealTime;
  currentRealTime = csGetTicks ();
  if (flags & flagFirstShot)
  {
    flags &= ~flagFirstShot;
    elapsedTime = 0;
  }
  else
  {
    if (currentRealTime < last)
      // csTicks(-1) is the period for a unsigend value
      elapsedTime = currentRealTime + (csTicks(-1) - last) + 1;
    else
      elapsedTime = currentRealTime - last;
    currentVirtualTime += elapsedTime;
  }
}

void csVirtualClock::Suspend ()
{
  flags |= flagSuspended;
  elapsedTime = 0;
}

void csVirtualClock::Resume ()
{
  if (flags & flagSuspended)
  {
    flags &= ~flagSuspended;
    flags |= flagFirstShot;
  }
}
