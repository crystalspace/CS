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

#include "cssys/csshlib.h"
#include <string.h>

// pseudo-prototypes (but compatible) for some OS/2 dynamic-library functions
extern "C" int DosLoadModule (const char *ErrorBuff, unsigned long ErrorBuffSize,
  const char *ModuleName, csLibraryHandle *ModuleHandle);
extern "C" int DosFreeModule (csLibraryHandle ModuleHandle);
extern "C" int DosQueryProcAddr (csLibraryHandle ModuleHandle,
  unsigned long Ordinal, const char *FunctionName, void *FunctionAddress);

csLibraryHandle csLoadLibrary (const char* iName)
{
  char name [260];
  strcat (strcpy (name, iName), ".dll");
  csLibraryHandle Handle;
  return DosLoadModule ((const char *)0, 0, name, &Handle) ?
    (csLibraryHandle)0 : Handle;
}

void *csGetLibrarySymbol (csLibraryHandle Handle, const char *iName)
{
  void *Proc;
  if (DosQueryProcAddr (Handle, 0, iName, &Proc))
    return (void *)0;
  return Proc;
}

bool csUnloadLibrary (csLibraryHandle Handle)
{
  return (DosFreeModule (Handle) == 0);
}
