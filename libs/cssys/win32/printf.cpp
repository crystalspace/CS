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

// PRINTF.CPP
// A printf function that works with Win32
// written (quickly) by dan
//
// 21-Jan-97 19:22
//	Modified (even quicker) by Robert Blum (r.blum@gmx.net) 
//	Now there are no buffer overrun problems any more 
//	However, I think the console should be encapsulated in a
//	class

#include <stdio.h>
#include <stdarg.h>
#include "sysdef.h"
#include "cssys/common/system.h"

#include <windows.h>
HANDLE stdout_handle;		// Win32 console stdout handle


bool pprintf_initialized=false;

// to be called before all pprintf() calls
void pprintf_init(void)
{
#ifdef WIN32_USECONSOLE
  AllocConsole();
  freopen("CONOUT$","a",stderr); // Redirect stderr to console   
  freopen("CONOUT$","a",stdout); // Redirect stdout to console   
#endif

  pprintf_initialized=true;
}

// to be called before shutdown
void pprintf_close(void)
{
  pprintf_initialized=false;

#ifdef WIN32_USECONSOLE
  FreeConsole();
#endif
}

// to be called instead of printf (exact same prototype/functionality of printf)
int pprintf(const char *str, ...)
{
  va_list arg;
  int nResult;
 
  if(!pprintf_initialized) return -1;

  va_start (arg, str);
  nResult=vfprintf (stdout, str, arg);
  va_end (arg);

  return nResult;
}  
