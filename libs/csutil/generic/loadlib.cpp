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
#include "csutil/sysfunc.h"

#ifdef NVALGRIND
// Force Valgrind client requests enabled
#undef NVALGRIND
#endif
#ifdef CS_HAVE_VALGRIND_VALGRIND_H
#include <valgrind/valgrind.h>
#endif
#ifndef RUNNING_ON_VALGRIND
#define RUNNING_ON_VALGRIND	0
#endif

#ifndef CS_PLUGIN_META_EXT
#  define CS_PLUGIN_META_EXT ".csplugin"
#endif
#ifndef CS_PLUGIN_EXT
#  define CS_PLUGIN_EXT ".so"
#endif

#ifdef CS_DEBUG
#  ifdef CS_HAVE_RTLD_NOW
#    define DLOPEN_MODE   RTLD_NOW | RTLD_GLOBAL	// handy for debugging
#  else
#    define DLOPEN_MODE   RTLD_LAZY | RTLD_GLOBAL
#  endif
#else
#  define DLOPEN_MODE   RTLD_LAZY | RTLD_GLOBAL
#endif

csLibraryHandle csLoadLibrary (const char* iName)
{
  // iName will have an ".csplugin" suffix
  size_t metaLen = strlen (CS_PLUGIN_META_EXT);
  size_t extLen = strlen (CS_PLUGIN_EXT);
  size_t nameLen = strlen (iName);
  char* binName = new char[nameLen + extLen + 1];
  strcpy (binName, iName);
  
  if ((nameLen >= metaLen) && 
    (strcasecmp (binName + nameLen - metaLen, CS_PLUGIN_META_EXT) == 0))
  {
    strcpy (binName + nameLen - metaLen, CS_PLUGIN_EXT);
  }
  else if ((nameLen >= extLen) && 
    (strcasecmp (binName + nameLen - extLen, CS_PLUGIN_EXT) != 0))
  {
    strcat (binName, CS_PLUGIN_EXT);
  }
  
  csLibraryHandle lib = dlopen (binName, DLOPEN_MODE);
  
  delete[] binName;
  
  return lib;
}

void csPrintLibraryError (const char *iModule)
{
  const char* dlerr = dlerror();
  if (dlerr)
    csFPrintf (stderr, "DLERROR (%s): %s\n", iModule? iModule : "", dlerr);
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
  if (RUNNING_ON_VALGRIND)
  {
    return true;
    /* Don't dlclose() on Valgrind. Otherwise debug symbols get lost (and
       reported call stacks less useful).
       This is the workaround stated in the Valgrind FAQ.
     */
  }
  return (dlclose (Handle) == 0);
}
