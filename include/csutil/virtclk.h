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

#ifndef __CS_VIRTCLK_H_
#define __CS_VIRTCLK_H_

#include "csutil/scf.h"
#include "iutil/virtclk.h"

/**
 * This is an implementation of a virtual clock. Using this
 * clock you can easily keep track of elapsed and current time
 * in a virtual setting.
 */
class csVirtualClock : public iVirtualClock
{
private:
  // Elapsed time between last two frames and absolute time in milliseconds
  csTicks ElapsedTime, CurrentTime;

public:
  csVirtualClock ();
  virtual ~csVirtualClock ();

  SCF_DECLARE_IBASE;

  virtual void Advance ();
  virtual void Suspend () { }
  virtual void Resume () { CurrentTime = csTicks (-1); }
  virtual csTicks GetElapsedTicks () const { return ElapsedTime; }
  virtual csTicks GetCurrentTicks () const { return CurrentTime; }
};

#endif // __CS_VIRTCLK_H_

