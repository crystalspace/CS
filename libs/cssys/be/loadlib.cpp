/*
  BeOS support for Crystal Space 3D library
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
#include "sysdef.h"
#include "cscom/com.h"

#include <kernel/image.h>

CS_HLIBRARY SysLoadLibrary (char* szLibName)
{
  image_id Handle = load_add_on (szLibName);
  if (Handle > 0)
  {
//  printf("add on loaded with id=%lx\n",Handle);
    HRESULT (*DllInitialize) ();
    if (get_image_symbol (Handle, "DllInitialize", B_SYMBOL_TYPE_TEXT, (void**)&DllInitialize) != B_OK)
    {
      printf ("Unable to find DllInitialize in %s\n", szLibName);
      return 0;
    }
    DllInitialize ();
  }
  else
    printf ("Error opening library '%s'!\n", szLibName);

  return (CS_HLIBRARY)Handle;
}

PROC SysGetProcAddress (CS_HLIBRARY Handle, char* szProcName)
{
  void *func;
  return (get_image_symbol(Handle, szProcName, B_SYMBOL_TYPE_TEXT, &func) == B_OK) ? (PROC)func : NULL;
}

bool SysFreeLibrary (CS_HLIBRARY Handle)
{
//  -*- warning: should return true if success, false if failed
//  -*- check the line below and remove this comment
  return (unload_add_on (Handle) == B_OK);
}

#endif
