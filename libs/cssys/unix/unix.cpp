/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "cssysdef.h"
#include "csutil/inifile.h"
#include "cssys/unix/unix.h"
#include "cssys/system.h"
#include "igraph3d.h"

//------------------------------------------------------ The System driver ---//

SysSystemDriver::SysSystemDriver () : csSystemDriver ()
{
}

void SysSystemDriver::Help ()
{
  csSystemDriver::Help ();
  Printf (MSG_STDOUT, "  -sdepth=<depth>    set simulated depth (8, 15, 16, or 32) (default=none)\n");
  Printf (MSG_STDOUT, "  -shm/noshm         SHM extension (default 'yes')\n");
}

void SysSystemDriver::Loop(void)
{
  while (!Shutdown && !ExitLoop)
  {
    static long prev_time = -1;
    long cur_time = Time ();
    NextFrame ((prev_time == -1) ? 0 : cur_time - prev_time, cur_time);
    prev_time = cur_time;
  }
}

#ifdef OS_SOLARIS
extern "C" {
int usleep(unsigned int);
}
#endif

void SysSystemDriver::Sleep (int SleepTime)
{
  usleep (SleepTime * 1000);
}
