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

#include <stdio.h>
#include <unistd.h>
#include <limits.h>

char* csExpandPath (const char* path)
{
  // Remember where we are.
  char old_path[CS_MAXPATHLEN];
  if (getcwd (old_path, sizeof (old_path)) == 0)
  {
    // In case of an error return 0.
    return 0;
  }

  // Normalize `path'.
  if (chdir (path) != 0)
  {
    chdir (old_path);
    return 0;
  }
  char normalized_path[CS_MAXPATHLEN];
  if (getcwd (normalized_path, sizeof (normalized_path)) == 0)
  {
    chdir (old_path);
    return 0;
  }

  // Restore working directory.
  chdir (old_path);

  return (csStrNew (normalized_path));
}

char* csGetAppPath (const char* argv0)
{
  if (!argv0 || (*argv0 == 0)) return 0;

  char* app = csStrNew (argv0);
  char* fullPath = 0;

  if (*app == PATH_SEPARATOR)
  {
    char* slash = strrchr (app, PATH_SEPARATOR);
    if (slash) *slash = 0;

    // Absolute path. 
    fullPath = csStrNew (app);
  } 
  else if (strchr (app, PATH_SEPARATOR) != 0)
  {
    char* slash = strrchr (app, PATH_SEPARATOR);
    if (slash) *slash = 0;

    // Relative path.
    fullPath = csExpandPath (app);
  }
  else 
  {
    // A bare name. Search PATH.
    char* envPATH = csStrNew (getenv ("PATH"));

    char* currentPart;
    char* nextPart = envPATH;

    do
    {
      currentPart = nextPart;
      nextPart = strchr (currentPart, ';');
      if (nextPart)
      {
	*nextPart++ = 0;
      }

      fullPath = csExpandPath (currentPart);
      if (access (fullPath, F_OK))
      {
	delete[] fullPath; fullPath = 0;
      }
    }
    while ((nextPart != 0) && (fullPath == 0));

    char* slash = strrchr (fullPath, PATH_SEPARATOR);
    if (slash) *slash = 0;

    delete[] envPATH;
  }

  delete[] app;

  return fullPath;
}

bool csPathsIdentical (const char* path1, const char* path2)
{
  return (strcmp (path1, path2) == 0);
}

