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

#include "sysdef.h"

#ifdef NO_COM_SUPPORT

#include <stdio.h>
#include <stdlib.h>
#include "cscom/com.h"

PROC SysGetProcAddress (CS_LIBRARY, const char*);

struct LibraryList
{
  CS_LIBRARY Handle;
  LibraryList *Next;
};

static LibraryList *LibList = NULL;

CS_HLIBRARY SysLoadLibrary (const char* szLibName)
{
#if 0
  LibraryList* Item;
  CS_LIBRARY Handle;
  HRESULT (*DllInitialize) ();

  Handle = (CS_LIBRARY)dlopen (szLibName, RTLD_LAZY);

  if (Handle)
  {
	DllInitialize = (HRESULT (*) ())SysGetProcAddress (Handle, "DllInitialize");
	if (!DllInitialize)
	{
	  printf("Unable to find DllInitialize in %s\n", szLibName);
	  return 0;
	}
	(DllInitialize) ();

	Item = new LibraryList;
	Item->Next = LibList;
	Item->Handle = Handle;
	LibList = Item;
  }
  else
  {
	printf ("Error opening library '%s'!\nReason '%s'!\n", szLibName, dlerror ());
  }

  return Handle;
#else
	return 0;
#endif
}

PROC SysGetProcAddress (CS_LIBRARY Handle, const char* szProcName)
{
#if 0
  return dlsym ((void*)Handle, szProcName);
#else
	return 0;
#endif
}

bool SysFreeLibrary(CS_HLIBRARY)
{
  return true;
}

bool SysFreeAllLibraries()
{
#if 0
  LibraryList* Current = LibList;

  while (Current)
  {
	LibraryList* Next = Current->Next;
	dlclose ((void*)Current->Handle);
	delete Current;
	Current = Next;
  }
#endif
  return true;
}

#endif
