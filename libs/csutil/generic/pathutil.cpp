/*
    Copyright (C) 2003 by Frank Richter

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
#include "csutil/stringquote.h"
#include "csutil/sysfunc.h"
#include "csutil/syspath.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

bool getcwdcheck (char* path, size_t size)
{
  if (getcwd (path, size) == 0)
  {
      csPrintfErr ("csPathUtilities: getcwd() error for %s (errno = %d)!\n",
        CS::Quote::Single (path), errno);
    return false;
  }
  return true;
}

bool chdircheck (const char* path)
{
  if (chdir (path) != 0)
  {
    const int errcode = errno;
    // some "file not found" errors are to be expected, so filter them out.
    if (errcode != ENOENT)
      csPrintfErr ("csPathUtilities: chdir() error for %s (errno = %d)!\n",
        path, errcode);
    return false;
  }
  return true;
}

char* csPathsUtilities::ExpandPath (const char* path)
{
  // Remember where we are.
  char old_path[CS_MAXPATHLEN];
  if (!getcwdcheck (old_path, sizeof (old_path)))
    // In case of an error return 0.
    return 0;

  // Normalize `path'.
  if (!chdircheck (path))
  {
    if (!chdircheck (old_path))
      return 0;
    return 0;
  }
  char normalized_path[CS_MAXPATHLEN];
  if (!getcwdcheck (normalized_path, sizeof (normalized_path)))
  {
    if (!chdircheck (old_path))
      return 0;
    return 0;
  }

  // Restore working directory.
  if (!chdircheck (old_path))
    return (csStrNew (normalized_path)); // return normalized path nevertheless

  return (csStrNew (normalized_path));
}

bool csPathsUtilities::PathsIdentical (const char* path1, const char* path2)
{
  return (strcmp (path1, path2) == 0);
}

namespace CS
{
  namespace Platform
  {

    csString GetTempDirectory ()
    {
      return "/tmp";
    }
  
    csString GetTempFilename (const char* /*path*/)
    {
      char filename[64];
  
      // Fallback
      cs_snprintf (filename, 64, "cs%x.tmp", getpid ());
  
      return filename;
    }

  }
}
