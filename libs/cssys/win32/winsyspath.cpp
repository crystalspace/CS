/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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
#include "cssys/sysfunc.h"
#include "cssys/syspath.h"

#include <windows.h>

char* csExpandPath (const char* path)
{
  char fullName[MAX_PATH];
  GetFullPathName (path, sizeof(fullName), fullName, 0);
  if (GetLongPathName (fullName, fullName, sizeof (fullName)) == 0) 
  {
    return 0;
  }

  return (csStrNew (fullName));
}

char* csGetAppPath (const char*)
{
  char appPath[MAX_PATH];
  GetModuleFileName (0, appPath, sizeof (appPath) - 1);
  char* slash = strrchr (appPath, PATH_SEPARATOR);
  if (slash) *slash = 0;

  return (csStrNew (appPath));
}

bool csPathsIdentical (const char* path1, const char* path2)
{
  return (strcasecmp (path1, path2) == 0);
}

