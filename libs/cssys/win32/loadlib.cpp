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
#include "cssys/csshlib.h"
#ifdef __CYGWIN__
#include <sys/cygwin.h>
#endif

csLibraryHandle csFindLoadLibrary (const char *iName)
{
  return csFindLoadLibrary (NULL, iName, ".dll");
}

csLibraryHandle csLoadLibrary (const char* iName)
{
#ifdef __CYGWIN__
  // cygwin wants to have win32 paths not unix paths
  char *tmp=new char[160];
  if (cygwin_conv_to_win32_path (iName,tmp))
      return 0;
  csLibraryHandle handle = LoadLibrary(tmp);
  delete[] tmp;
  return handle;
#else
  return LoadLibrary (iName);
#endif
}

void* csGetLibrarySymbol(csLibraryHandle Handle, const char* Name)
{
  void *ptr = GetProcAddress ((HMODULE)Handle, Name);
  if (!ptr)
  {
    char *Name2;
    Name2 = new char [strlen (Name) + 2];
    strcpy (Name2, "_");
    strcat (Name2, Name);
    ptr = GetProcAddress ((HMODULE)Handle, Name2);
    delete [] Name2;
  }
  return ptr;
}

bool csUnloadLibrary (csLibraryHandle Handle)
{
  return FreeLibrary ((HMODULE)Handle)!=0;
}

void csPrintLibraryError (const char * /*iModule*/)
{
}
