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

#ifdef NO_COM_SUPPORT

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include "sysdef.h"
#include "cscom/com.h"

#ifdef DEBUG
#  define DLOPEN_MODE 	RTLD_NOW		// handy for debugging
#else
#  define DLOPEN_MODE 	RTLD_LAZY
#endif

CS_HLIBRARY SysLoadLibrary (char* szLibName)
{
  CS_HLIBRARY Handle;
  Handle = (CS_HLIBRARY) dlopen (szLibName, DLOPEN_MODE);
  if (!Handle)
  {
    char path [1255];
    getcwd (path, 1024);
    strcat (path, "/");
    strcat (path, szLibName);
    Handle = (CS_HLIBRARY) dlopen (path, DLOPEN_MODE);
  }
  if (Handle)
  {
    HRESULT (*DllInitialize) () = (HRESULT (*) ())dlsym ((void*)Handle, "DllInitialize");
    if (!DllInitialize)
    {
      fprintf (stderr, "%s: Unable to find DllInitialize entry\n", szLibName);
      dlclose ((void*)Handle);
      return (CS_HLIBRARY)0;
    }
    DllInitialize ();
  }
  else
    fprintf (stderr, "%s\n", dlerror ());

  return Handle;
}

PROC SysGetProcAddress (CS_HLIBRARY Handle, char* szProcName)
{
  return (PROC)dlsym ((void *)Handle, szProcName);
}

bool SysFreeLibrary (CS_HLIBRARY Handle)
{
  return (dlclose ((void *)Handle) == 0);
}

#endif
