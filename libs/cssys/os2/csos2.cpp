/*
  OS/2 support for Crystal Space 3D library
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

BEGIN_INTERFACE_TABLE (SysSystemDriver)
  IMPLEMENTS_COMPOSITE_INTERFACE (System)
  IMPLEMENTS_COMPOSITE_INTERFACE (OS2SystemDriver)
END_INTERFACE_TABLE ()

IMPLEMENT_UNKNOWN_NODELETE (SysSystemDriver)

SysSystemDriver::SysSystemDriver () : csSystemDriver ()
{
  /* Set main thread to idle priority */
  DosSetPriority (PRTYS_THREAD, PRTYC_IDLETIME, PRTYD_MAXIMUM, 0L);
}

void SysSystemDriver::SetSystemDefaults (csIniFile *config)
{
  csSystemDriver::SetSystemDefaults (config);
  WindowX = config->GetInt ("VideoDriver", "WINDOWX", INT_MIN);
  WindowY = config->GetInt ("VideoDriver", "WINDOWY", INT_MIN);
  WindowWidth = config->GetInt ("VideoDriver", "WINDOWWIDTH", -1);
  WindowHeight = config->GetInt ("VideoDriver", "WINDOWHEIGHT", -1);
  HardwareCursor = config->GetYesNo ("VideoDriver", "SYS_MOUSE_CURSOR", true);
}

void SysSystemDriver::Help ()
{
  csSystemDriver::Help ();
  Printf (MSG_STDOUT, "  -winsize <w>,<y>   set window size (default=max fit mode multiple)\n");
  Printf (MSG_STDOUT, "  -winpos <w>,<y>    set window position in percent of screen (default=center)\n");
}

bool SysSystemDriver::ParseArg (int argc, char* argv[], int &i)
{
  if (stricmp ("-winsize", argv[i]) == 0)
  {
    i++;
    if (i < argc)
    {
      int wres, hres;

      if (sscanf (argv[i], "%d,%d", &wres, &hres) == 2)
      {
        WindowWidth = wres;
        WindowHeight = hres;
      }
    }
  }
  else if (stricmp ("-winpos", argv[i]) == 0)
  {
    i++;
    if (i < argc)
    {
      int xpos, ypos;

      if (sscanf (argv[i], "%d,%d", &xpos, &ypos) == 2)
      {
        WindowX = xpos;
        WindowY = ypos;
      }
    }
  }
  else
    return csSystemDriver::ParseArg (argc, argv, i);
  return true;
}

void SysSystemDriver::TerminateHandler ()
{
  Shutdown = true;
}

void SysSystemDriver::FocusHandler (bool Enable)
{
  if (EventQueue)
    do_focus (Enable);
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

IMPLEMENT_COMPOSITE_UNKNOWN_AS_EMBEDDED (SysSystemDriver, OS2SystemDriver)

STDMETHODIMP SysSystemDriver::XOS2SystemDriver::GetSettings (int &WindowX,
  int &WindowY, int &WindowWidth, int &WindowHeight, bool &HardwareCursor)
{
  METHOD_PROLOGUE (SysSystemDriver, OS2SystemDriver)
  WindowX = pThis->WindowX;
  WindowY = pThis->WindowY;
  WindowWidth = pThis->WindowWidth;
  WindowHeight = pThis->WindowHeight;
  HardwareCursor = pThis->HardwareCursor;
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XOS2SystemDriver::KeyboardEvent
  (int ScanCode, bool Down)
{
  METHOD_PROLOGUE (SysSystemDriver, OS2SystemDriver)
  ((SysKeyboardDriver *)pThis->Keyboard)->Handler (ScanCode, Down);
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XOS2SystemDriver::MouseEvent
  (int Button, int Down, int x, int y, int ShiftFlags)
{
  METHOD_PROLOGUE (SysSystemDriver, OS2SystemDriver)
  ((SysMouseDriver *)pThis->Mouse)->Handler (Button, Down, x, y, ShiftFlags);
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XOS2SystemDriver::TerminateEvent ()
{
  METHOD_PROLOGUE (SysSystemDriver, OS2SystemDriver)
  pThis->TerminateHandler ();
  return S_OK;
}

STDMETHODIMP SysSystemDriver::XOS2SystemDriver::FocusEvent (bool Enable)
{
  METHOD_PROLOGUE (SysSystemDriver, OS2SystemDriver)
  pThis->FocusHandler (Enable);
  return S_OK;
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

void SysKeyboardDriver::Handler (unsigned char ScanCode, bool Down)
{
  if (!Ready ())
    return;

  int key = ScancodeToChar [ScanCode];
  if (key)
  {
    time_t time = System->Time ();

    if (Down)
      do_keypress (time, key);
    else
      do_keyrelease (time, key);
  } /* endif */
}

//== class SysMouseDriver ======================================================

SysMouseDriver::SysMouseDriver () : csMouseDriver ()
{
  // Nothing to do
}

void SysMouseDriver::Handler (int Button, int Down, int x, int y, int ShiftFlags)
{
  if (!Ready ())
    return;

  time_t time = System->Time ();

  if (Button == 0)
    do_mousemotion (time, x, y);
  else if (Down)
    do_buttonpress (time, Button, x, y, ShiftFlags & CSMASK_SHIFT,
      ShiftFlags & CSMASK_ALT, ShiftFlags & CSMASK_CTRL);
  else
    do_buttonrelease (time, Button, x, y);
}
