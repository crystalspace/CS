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
#include "cssys/unix/unix.h"
#include "cssys/system.h"
#include "ivideo/graph3d.h"

// Put an #ifndef OS_XXX around this if your system does not support locale.
// In a long-term perspective, if we find such a system, we should add the
// detection of locale to unixconf.sh
#define I18N

#ifdef I18N
#  include <locale.h>
#endif

//------------------------------------------------------ The System driver ---//

SysSystemDriver::SysSystemDriver () : csSystemDriver ()
{
#ifdef I18N
  // Never do "LC_ALL" because this will break some things like numeric format
  setlocale (LC_COLLATE, "");
  setlocale (LC_CTYPE, "");
  setlocale (LC_MESSAGES, "");
  setlocale (LC_TIME, "");
#endif
}

void SysSystemDriver::Sleep (int SleepTime)
{
  usleep (SleepTime * 1000);
}

bool SysSystemDriver::GetInstallPath (char *oInstallPath, size_t iBufferSize)
{
  char *path = getenv ("CRYSTAL");
  if (!path)
  {
    // no setting, use the default.
    // is the current dir a possible install? try opening scf.cfg,
    if (!access ("scf.cfg", F_OK))
    {
      strncpy (oInstallPath, "", iBufferSize);
      return true;
    }
    // default.
    strncpy (oInstallPath, "/usr/local/crystal/", iBufferSize);
    return true;
  }

  // Well, we won't check for brain-damaged cases here such as iBufferSize < 3
  // since we presume user has even a little brains...

  strncpy (oInstallPath, path, iBufferSize);
  // check for ending '/'
  size_t len = strlen (oInstallPath);
  // empty stands for current directory
  if (!len)
    oInstallPath [len++] = '.';
  if (oInstallPath [len - 1] != '/')
    oInstallPath [len++] = '/';
  oInstallPath [len] = 0;
  return true;
}
