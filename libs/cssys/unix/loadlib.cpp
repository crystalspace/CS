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
#include <stdlib.h>
#include "csutil/csstring.h"

#ifdef CS_DEBUG
#  define DLOPEN_MODE   RTLD_NOW | RTLD_GLOBAL	// handy for debugging
#else
#  define DLOPEN_MODE   RTLD_LAZY | RTLD_GLOBAL
#endif

// Upon initialization add the current directory to the
// list of paths searched for plugins
class __loadlib_init
{
public:
  __loadlib_init ()
  {
    extern bool findlib_search_nodir;
    findlib_search_nodir = false;
    char path [1255];
    getcwd (path, sizeof (path));
    strcat (path, "/");
    csAddLibraryPath (path);
  }
} __loadlib_init_dummy;

csLibraryHandle csFindLoadLibrary (const char *iName)
{
  return csFindLoadLibrary ("lib", iName, ".so");
}

csLibraryHandle csLoadLibrary (const char* iName)
{
  csLibraryHandle Handle = dlopen (iName, DLOPEN_MODE);
  if (!Handle)
    fprintf (stderr, "DLERROR: %s\n", dlerror ());
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
