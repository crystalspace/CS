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

#include "cssys/csshlib.h"
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifdef DEBUG
#  define DLOPEN_MODE 	RTLD_NOW		// handy for debugging
#else
#  define DLOPEN_MODE 	RTLD_LAZY
#endif

/**
 * We try to find the library in the following sequence:
 * <iName>.so, lib<iName>.so, <cwd><iName>.so, <cwd>lib<iName>.so
 */
csLibraryHandle csLoadLibrary (const char* iName)
{
  csLibraryHandle Handle;
  for (int Try = 0; Try < 4; Try++)
  {
    char name [1255];
    name [0] = 0;
    if (Try >= 2)
    {
      getcwd (name, sizeof (name));
      strcat (name, "/");
    }
    if (Try & 1)
      strcat (name, "lib");
    strcat (strcat (name, iName), ".so");
    if (Handle = dlopen (name, DLOPEN_MODE))
      break;
  }
  if (!Handle)
    fprintf (stderr, "%s\n", dlerror ());

  return Handle;
}

void *csGetLibrarySymbol (csLibraryHandle Handle, const char *iName)
{
  return dlsym (Handle, iName);
}

bool csUnloadLibrary (csLibraryHandle Handle)
{
  return (dlclose (Handle) == 0);
}
