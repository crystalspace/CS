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

IMPLEMENT_IBASE_EXT (SysSystemDriver)
  IMPLEMENTS_INTERFACE (iUnixSystemDriver)
IMPLEMENT_IBASE_EXT_END

SysSystemDriver::SysSystemDriver () : csSystemDriver ()
{
}

void SysSystemDriver::SetSystemDefaults (csIniFile *config)
{
  csSystemDriver::SetSystemDefaults (config);
  SimDepth = Config->GetInt ("VideoDriver", "SIMULATE_DEPTH", 0);
  UseSHM = Config->GetYesNo ("VideoDriver", "XSHM", true);
  HardwareCursor = Config->GetYesNo ("VideoDriver", "SystemMouseCursor", true);

  const char *val;
  if ((val = GetOptionCL ("shm")))
    UseSHM = true;

  if ((val = GetOptionCL ("noshm")))
    UseSHM = false;

  if ((val = GetOptionCL ("sdepth")))
  {
    SimDepth = atol (val);
    if (SimDepth != 8 && SimDepth != 15 && SimDepth != 16 && SimDepth != 32)
    {
      Printf (MSG_FATAL_ERROR, "Crystal Space can't run in this simulated depth! (use 8, 15, 16, or 32)!\n");
      exit (0);
    }
  }
}

void SysSystemDriver::Help ()
{
  csSystemDriver::Help ();
  Printf (MSG_STDOUT, "  -sdepth=<depth>    set simulated depth (8, 15, 16, or 32) (default=none)\n");
  Printf (MSG_STDOUT, "  -shm/noshm         SHM extension (default '%sshm')\n",
    UseSHM ? "" : "no");
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

void SysSystemDriver::Sleep (int SleepTime)
{
  usleep (SleepTime * 1000);
}

//------------------------------------------------------ XUnixSystemDriver ---//

void SysSystemDriver::GetExtSettings (int &oSimDepth,
  bool &oUseSHM, bool &oHardwareCursor)
{
  oSimDepth = SimDepth;
  oUseSHM = UseSHM;
  oHardwareCursor = HardwareCursor;
}
