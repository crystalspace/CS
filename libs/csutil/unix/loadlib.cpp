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

#include "cssysdef.h"
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "csutil/csshlib.h"
#include "csutil/csstring.h"

#ifdef CS_DEBUG
#  ifdef CS_HAVE_RTLD_NOW
#    define DLOPEN_MODE   RTLD_NOW // | RTLD_GLOBAL	// handy for debugging
#  else
#    define DLOPEN_MODE   RTLD_LAZY // | RTLD_GLOBAL
#  endif
#else
#  define DLOPEN_MODE   RTLD_LAZY // | RTLD_GLOBAL
#endif

csLibraryHandle csLoadLibrary (const char* iName)
{
  // iName will have an ".csplugin" suffix
  int nameLen = strlen (iName);
  char* binName = new char[nameLen + 3];
  strcpy (binName, iName);
  
  if ((nameLen >= 7) && 
    (strcasecmp (binName + nameLen - 9, ".csplugin") == 0))
  {
    strcpy (binName + nameLen - 9, ".so");
  }
  else if ((nameLen >= 3) && 
    (strcasecmp (binName + nameLen - 3, ".so") != 0))
  {
    strcat (binName, ".so");
  }
  
  csLibraryHandle lib = dlopen (binName, DLOPEN_MODE);
  
  delete[] binName;
  
  return lib;
}

void csPrintLibraryError (const char *iModule)
{
  const char* dlerr = dlerror();
  if (dlerr)
    fprintf (stderr, "DLERROR (%s): %s\n", iModule? iModule : "", dlerr);
}

void *csGetLibrarySymbol (csLibraryHandle Handle, const char *iName)
{
  void* p = dlsym (Handle, iName);
  if (p == 0)
  {
    csString sym;
    sym << '_' << iName;
    p = dlsym (Handle, sym);
  }
  return p;
}

bool csUnloadLibrary (csLibraryHandle Handle)
{
  return (dlclose (Handle) == 0);
}
