/*
    Copyright (C) 2001 by Wouter Wijngaards

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
#include <string.h>
#include "cssysdef.h"
#include "cssys/system.h"

bool csGetInstallPath (char *oInstallPath, size_t iBufferSize)
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
    // default install place
    if (!access ("/usr/local/crystal/scf.cfg", F_OK))
    {
      strncpy (oInstallPath, "/usr/local/crystal/", iBufferSize);
      return true;
    }
    // debian install place
    if (!access ("/usr/lib/crystalspace/scf.cfg", F_OK))
    {
      strncpy (oInstallPath, "/usr/lib/crystalspace/", iBufferSize);
      return true;
    }
    /// no install location can be found
    fprintf(stderr,
      "Warning: Crystal Space installation directory not detected.\n");
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
