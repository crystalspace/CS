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

#include "sysdef.h"

#ifdef NO_COM_SUPPORT

#include "cscom/com.h"

typedef unsigned long HMODULE;
extern "C" int DosLoadModule (const char *pszName, unsigned long cbName,
  const char *pszModname, HMODULE *phmod);
extern "C" int DosFreeModule (HMODULE hmod);
extern "C" int DosQueryProcAddr (HMODULE hmod, unsigned long ordinal,
  const char *pszName, PROC* ppfn);

CS_HLIBRARY SysLoadLibrary (const char* szLibName)
{
  HMODULE Handle;
  return (DosLoadModule (NULL, 0, szLibName, &Handle) ? 0 : Handle);
}

PROC SysGetProcAddress (CS_HLIBRARY Handle, const char* szProcName)
{
  PROC Func;
  return (DosQueryProcAddr (Handle, 0, szProcName, &Func) ? NULL : Func);
}

bool SysFreeLibrary (CS_HLIBRARY Handle)
{
  return (DosFreeModule (Handle) == 0);
}

#endif
