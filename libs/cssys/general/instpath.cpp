/*
    Shared Library Path Support
    Copyright (C) 2001 by Jorrit Tyberghein
  
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

#include <stdlib.h>
#include <string.h>
#define CS_SYSDEF_PROVIDE_PATH
#include "cssysdef.h"
#include "cssys/system.h"

bool csSystemDriver::InstallPath (char *oInstallPath, size_t iBufferSize)
{
  if (iBufferSize == 0)
    return false;

  char *path = getenv ("CRYSTAL");
  if (!path || !*path)
  {
    oInstallPath [0] = '\0';
    return true;
  }

  size_t pl = strlen (path);
  // See if we have to add an ending path separator to the directory
  bool addsep = (path [pl - 1] != PATH_SEPARATOR)
#if defined (OS_DOS) || defined (OS_OS2) || defined (OS_WIN32)
             && (path [pl - 1] != '/')
#endif
    ;
  if (addsep)
    pl++;

  if (pl >= iBufferSize)
    pl = iBufferSize - 1;
  memcpy (oInstallPath, path, pl);
  if (addsep)
    oInstallPath [pl - 1] = PATH_SEPARATOR;
  oInstallPath [pl] = 0;
  return true;
}
