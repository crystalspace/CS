//=============================================================================
//
//	Copyright (C) 2002 by Matt Reda <mreda@mac.com>
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
// License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXInstallPath.cpp
//
//	Platform-specific function to determine configuration and plugin paths.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "OSXInstallPath.h"
#include <stdio.h>
#include <stdlib.h>

#include "csutil/util.h"

//-----------------------------------------------------------------------------
// csGetConfigPath
//-----------------------------------------------------------------------------
char* csGetConfigPath()
{
  char* buff = new char[FILENAME_MAX];
  if (OSXGetInstallPath(buff, FILENAME_MAX, PATH_SEPARATOR) == 0)
  {
    char const* path = getenv("CRYSTAL");
    if (path == 0 || *path == 0)
      strncpy(buff, ".", FILENAME_MAX);
    else
      strncpy(buff, path, FILENAME_MAX);
  }

  // Add path separator if not present.
  int const length = strlen(buff);
  if (buff[length - 1] != PATH_SEPARATOR && length <= FILENAME_MAX - 2)
  {
    buff[length] = PATH_SEPARATOR;
    buff[length + 1] = '\0';
  }
  
  return buff;
}


//-----------------------------------------------------------------------------
// csGetPluginPaths
//-----------------------------------------------------------------------------
char** csGetPluginPaths()
{
  char** paths = new char*[5];			// Caller frees.
  char*  buff1 = new char[FILENAME_MAX];	// Caller frees.
  char*  buff2 = new char[FILENAME_MAX];	// Caller frees.
  char*  cpath = csGetConfigPath();		// Caller frees.

  strncpy(buff1, cpath, FILENAME_MAX);
  strncat(buff1, "/lib", FILENAME_MAX);
  buff1[FILENAME_MAX - 1] = '\0';
  
  strncpy(buff2, buff1, FILENAME_MAX);
  strncat(buff2, "/crystal", FILENAME_MAX);
  buff2[FILENAME_MAX - 1] = '\0';
  
  paths[0] = buff1;
  paths[1] = buff2;
  paths[2] = cpath;
  paths[3] = csStrNew(".");
  paths[4] = 0;

  return paths;
}
