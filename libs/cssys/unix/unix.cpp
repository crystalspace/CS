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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "cssysdef.h"
#include "csutil/inifile.h"
#include "cssys/unix/unix.h"
#include "cssys/system.h"
#include "igraph3d.h"

//------------------------------------------------------ The System driver ---//

SysSystemDriver::SysSystemDriver () : csSystemDriver ()
{
  // Pretty complex system driver, don't you think?
}

#ifdef OS_SOLARIS
extern "C" int usleep (unsigned);
#endif

void SysSystemDriver::Sleep (int SleepTime)
{
  usleep (SleepTime * 1000);
}

bool SysSystemDriver::GetInstallPath (char *oInstallPath, size_t iBufferSize)
{
  char *path = getenv ("CRYSTAL");
  if(!path || !*path)
  {
    // no setting, use the default.
    strncpy(oInstallPath, "/usr/local/crystal/", iBufferSize);
    return true;
  }
  strncpy(oInstallPath, path, iBufferSize);
  // check for ending '/'
  int len = strlen(oInstallPath);
  if(len == 0) return false;
  if( oInstallPath[len-1] == '/' )
    return true;
  if(len+1 >= (int) iBufferSize)
  {
    strncpy(oInstallPath, "", iBufferSize); //make empty if possible
    return false;
  }
  oInstallPath[len] = '/';
  oInstallPath[len+1] = 0;
  return true;
}

