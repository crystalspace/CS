/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny
  
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
#include "cscom/com.h"

extern "C" int DosScanEnv (const char *pszName, char **ppszValue);

unsigned long dll_handle;

#if defined (__EMX__)

extern "C" int _CRT_init (void);
extern "C" void _CRT_term (void);
extern "C" void __ctordtorInit (void);
extern "C" void __ctordtorTerm (void);

extern "C" unsigned long _DLL_InitTerm (unsigned long mod_handle, unsigned long flag)
{
  dll_handle = mod_handle;
  switch (flag)
  {
    case 0:
      if (_CRT_init ()) return 0;
      __ctordtorInit ();
      DllInitialize ();
      return 1;
    case 1:
      __ctordtorTerm ();
      _CRT_term ();
      return 1;
    default:
      return 0;
  }
}

#elif defined (COMP_WCC)

extern "C" unsigned __dll_initialize (unsigned long mod_handle)
{
  dll_handle = mod_handle;
  DllInitialize ();
  return 1;
}

#else

#error "no DLL initialization/finalization routines for current runtime"

#endif

// A version of getenv() that works from DLLs
extern "C" char *getenv (const char *name)
{
  char *value;
  if (DosScanEnv (name, &value))
    return NULL;
  else
    return value;
}
