/*
    Copyright (C) 1998 by Jorrit Tyberghein and Dan Ogles.
  
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
#include <stdarg.h>
#include "sysdef.h"
#include "cssys/system.h"

#include <windows.h>

static bool console_ok = false;

// to be called before all printf() calls
void csSystemDriver::console_open ()
{
#ifdef WIN32_USECONSOLE
  AllocConsole();
  freopen("CONOUT$", "a", stderr); // Redirect stderr to console   
  freopen("CONOUT$", "a", stdout); // Redirect stdout to console   
#endif

  console_ok = true;
}

// to be called before shutdown
void csSystemDriver::console_close ()
{
  console_ok = false;

#ifdef WIN32_USECONSOLE
  FreeConsole();
#endif
}

// to be called instead of printf (exact same prototype/functionality of printf)
void csSystemDriver::console_out (const char *str)
{
  if (console_ok)
    fputs (str, stdout);
}  
