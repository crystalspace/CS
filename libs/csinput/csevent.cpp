/*
  Crystal Space Windowing System: Event manager
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@eltech.ru>

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

#include "sysdef.h"
#include "csinput/csevent.h"

csEvent::csEvent (long iTime, int eType, int kCode, int kShiftKeys)
{
  Time = iTime;
  Type = eType;
  Key.Code = kCode;
  Key.ShiftKeys = kShiftKeys;
}

csEvent::csEvent (long iTime, int eType, int mx, int my, int mbutton, int mShiftKeys)
{
  Time = iTime;
  Type = eType;
  Mouse.x = mx;
  Mouse.y = my;
  Mouse.Button = mbutton;
  Mouse.ShiftKeys = mShiftKeys;
}

csEvent::csEvent (long iTime, int eType, int cCode, void *cInfo)
{
  Time = iTime;
  Type = eType;
  Command.Code = cCode;
  Command.Info = cInfo;
}

csEvent::~csEvent ()
{
}
