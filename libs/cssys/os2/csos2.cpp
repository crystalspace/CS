/*
    OS/2 support for Crystal Space 3D library
    Copyright (C) 1998 by Andrew Zabolotny <bit@eltech.ru>

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

#include <limits.h>
#include <stdarg.h>
#include "cssysdef.h"
#include "cssys/os2/csos2.h"
#include "csutil/inifile.h"
#include "isystem.h"

#undef SEVERITY_ERROR
#define INCL_DOS
#include <os2.h>

//== class SysSystemDriver =====================================================

SysSystemDriver::SysSystemDriver () : csSystemDriver ()
{
  // Lower the priority of the main thread
  DosSetPriority (PRTYS_THREAD, PRTYC_IDLETIME, PRTYD_MAXIMUM, 0);
}

void SysSystemDriver::Sleep (int SleepTime)
{
  DosSleep (SleepTime);
}
