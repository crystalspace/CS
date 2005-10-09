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


csVirtualClock::csVirtualClock ()
  : scfImplementationType (this), ElapsedTime (0), CurrentTime (csTicks (-1))
{
}

csVirtualClock::~csVirtualClock ()
{
}

void csVirtualClock::Advance ()
{
  csTicks last = CurrentTime;
  CurrentTime = csGetTicks ();
  if (last == csTicks(-1))
    ElapsedTime = 0;
  else
  {
      if (CurrentTime < last)
          // csTicks(-1) is the period for a unsigend value
          ElapsedTime = CurrentTime + (csTicks(-1) - last) + 1;
      else
          ElapsedTime = CurrentTime - last;
  }
}

