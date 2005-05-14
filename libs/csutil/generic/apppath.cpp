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
#include "csutil/sysfunc.h"
#include "csutil/syspath.h"
#include "csutil/csstring.h"
#include "csutil/util.h"

#include <stdio.h>
#include <unistd.h>
#include <limits.h>

csString csInstallationPathsHelper::GetAppPath (const char* argv0)
{
  csString fullpath;

  if (argv0 == 0 || *argv0 == '\0')
  {
    fullpath.Clear();
  }
  else if (*argv0 == CS_PATH_SEPARATOR) // Absolute: exact location of app.
  {
    fullpath = argv0;  
  } 
  else if (strchr (argv0, CS_PATH_SEPARATOR) != 0) // Relative:containing / ?
  {
    char dir[CS_MAXPATHLEN];		// Yes, getcwd()+relpath gives location
    if (getcwd(dir, sizeof(dir)) != 0)	// of app.
      fullpath << dir << CS_PATH_SEPARATOR << argv0;
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
      nextPart = strchr (currentPart, CS_PATH_DELIMITER);
      if (nextPart)
	*nextPart++ = 0;

      csString apppath;
      apppath << currentPart << CS_PATH_SEPARATOR << argv0;
      if (access (apppath, F_OK) == 0)
      {
        fullpath = apppath;
        break;
      }
    } while (nextPart != 0);

    delete[] envPATH;
  }
  
  return fullpath;
}
