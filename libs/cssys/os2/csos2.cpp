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
#include "sysdef.h"
#include "csos2.h"
#include "scancode.h"
#include "csutil/inifile.h"
#include "isystem.h"

#undef SEVERITY_ERROR
#define INCL_DOS
#include <os2.h>

//== class SysSystemDriver =====================================================

IMPLEMENT_IBASE (SysSystemDriver)
  IMPLEMENTS_INTERFACE (iSystem)
  IMPLEMENTS_INTERFACE (iOS2SystemDriver)
IMPLEMENT_IBASE_END

SysSystemDriver::SysSystemDriver () : csSystemDriver ()
{
  CONSTRUCT_IBASE (NULL)
  // Lower the priority of the main thread
  DosSetPriority (PRTYS_THREAD, PRTYC_REGULAR, PRTYD_MINIMUM, 0);
}

void SysSystemDriver::SetSystemDefaults (csIniFile *config)
{
  csSystemDriver::SetSystemDefaults (config);
  WindowX = config->GetInt ("VideoDriver", "WINDOWX", INT_MIN);
  WindowY = config->GetInt ("VideoDriver", "WINDOWY", INT_MIN);
  WindowWidth = config->GetInt ("VideoDriver", "WINDOWWIDTH", -1);
  WindowHeight = config->GetInt ("VideoDriver", "WINDOWHEIGHT", -1);
  HardwareCursor = config->GetYesNo ("VideoDriver", "SYS_MOUSE_CURSOR", true);

  const char *val;
  if ((val = GetOptionCL ("winsize")))
  {
    int wres, hres;
    if (sscanf (val, "%d,%d", &wres, &hres) == 2)
    {
      WindowWidth = wres;
      WindowHeight = hres;
    }
  }

  if ((val = GetOptionCL ("winpos")))
  {
    int xpos, ypos;
    if (sscanf (val, "%d,%d", &xpos, &ypos) == 2)
    {
      WindowX = xpos;
      WindowY = ypos;
    }
  }
}

void SysSystemDriver::Help ()
{
  csSystemDriver::Help ();
  Printf (MSG_STDOUT, "  -winpos=<x>,<y>    set window position in percent of screen (default=center)\n");
  Printf (MSG_STDOUT, "  -winsize=<w>,<h>   set window size (default=max fit mode multiple)\n");
}

void SysSystemDriver::Loop ()
{
  while (!Shutdown && !ExitLoop)
  {
    static long prev_time = -1;
    long new_prev_time = Time ();
    NextFrame ((prev_time == -1) ? 0 : new_prev_time - prev_time, Time ());
    prev_time = new_prev_time;
  }
}

void SysSystemDriver::Sleep (int SleepTime)
{
  DosSleep (SleepTime);
}

//== class XOS2SystemDriver ====================================================

void SysSystemDriver::GetExtSettings (int &oWindowX, int &oWindowY,
  int &oWindowWidth, int &oWindowHeight, bool &oHardwareCursor)
{
  oWindowX = WindowX;
  oWindowY = WindowY;
  oWindowWidth = WindowWidth;
  oWindowHeight = WindowHeight;
  oHardwareCursor = HardwareCursor;
}

void SysSystemDriver::KeyboardEvent (int ScanCode, bool Down)
{
  QueueKeyEvent (ScancodeToChar [ScanCode], Down);
}

//== class SysKeyboardDriver ===================================================

SysKeyboardDriver::SysKeyboardDriver () : csKeyboardDriver ()
{
  // Initialize scancode->char conversion table with additional codes
  ScancodeToChar [SCANCODE_ESC]         = CSKEY_ESC;
  ScancodeToChar [SCANCODE_RALT]        = CSKEY_ALT;
  ScancodeToChar [SCANCODE_ALT]         = CSKEY_ALT;
  ScancodeToChar [SCANCODE_RCTRL]       = CSKEY_CTRL;
  ScancodeToChar [SCANCODE_CTRL]        = CSKEY_CTRL;
  ScancodeToChar [SCANCODE_LSHIFT]      = CSKEY_SHIFT;
  ScancodeToChar [SCANCODE_RSHIFT]      = CSKEY_SHIFT;
  ScancodeToChar [SCANCODE_UP]          = CSKEY_UP;
  ScancodeToChar [SCANCODE_GRAYUP]      = CSKEY_UP;
  ScancodeToChar [SCANCODE_DOWN]        = CSKEY_DOWN;
  ScancodeToChar [SCANCODE_GRAYDOWN]    = CSKEY_DOWN;
  ScancodeToChar [SCANCODE_LEFT]        = CSKEY_LEFT;
  ScancodeToChar [SCANCODE_GRAYLEFT]    = CSKEY_LEFT;
  ScancodeToChar [SCANCODE_RIGHT]       = CSKEY_RIGHT;
  ScancodeToChar [SCANCODE_GRAYRIGHT]   = CSKEY_RIGHT;
  ScancodeToChar [SCANCODE_PGUP]        = CSKEY_PGUP;
  ScancodeToChar [SCANCODE_GRAYPGUP]    = CSKEY_PGUP;
  ScancodeToChar [SCANCODE_PGDN]        = CSKEY_PGDN;
  ScancodeToChar [SCANCODE_GRAYPGDN]    = CSKEY_PGDN;
  ScancodeToChar [SCANCODE_GRAYINS]     = CSKEY_INS;
  ScancodeToChar [SCANCODE_INS]         = CSKEY_INS;
  ScancodeToChar [SCANCODE_GRAYDEL]     = CSKEY_DEL;
  ScancodeToChar [SCANCODE_DEL]         = CSKEY_DEL;
  ScancodeToChar [SCANCODE_GRAYHOME]    = CSKEY_HOME;
  ScancodeToChar [SCANCODE_HOME]        = CSKEY_HOME;
  ScancodeToChar [SCANCODE_GRAYEND]     = CSKEY_END;
  ScancodeToChar [SCANCODE_END]         = CSKEY_END;
  ScancodeToChar [SCANCODE_GRAYENTER]   = CSKEY_ENTER;

  ScancodeToChar [SCANCODE_F1]          = CSKEY_F1;
  ScancodeToChar [SCANCODE_F2]          = CSKEY_F2;
  ScancodeToChar [SCANCODE_F3]          = CSKEY_F3;
  ScancodeToChar [SCANCODE_F4]          = CSKEY_F4;
  ScancodeToChar [SCANCODE_F5]          = CSKEY_F5;
  ScancodeToChar [SCANCODE_F6]          = CSKEY_F6;
  ScancodeToChar [SCANCODE_F7]          = CSKEY_F7;
  ScancodeToChar [SCANCODE_F8]          = CSKEY_F8;
  ScancodeToChar [SCANCODE_F9]          = CSKEY_F9;
  ScancodeToChar [SCANCODE_F10]         = CSKEY_F10;
  ScancodeToChar [SCANCODE_F11]         = CSKEY_F11;
  ScancodeToChar [SCANCODE_F12]         = CSKEY_F12;
  ScancodeToChar [SCANCODE_CENTER]      = CSKEY_CENTER;
}

//== class SysMouseDriver ======================================================

SysMouseDriver::SysMouseDriver () : csMouseDriver ()
{
  // Nothing to do
}
