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
#include <stdlib.h>

#include "csutil/util.h"

//-----------------------------------------------------------------------------
// csGetConfigPath
//-----------------------------------------------------------------------------
char* csGetConfigPath()
{
  char* buff = new char[1024];
  if (!OSXGetInstallPath(buff, sizeof(buff), PATH_SEPARATOR))
  {
    char const* path = getenv("CRYSTAL");
    if (!path || *path == 0)
      strncpy (buff, ".", sizeof(buff));
    else
      strncpy (buff, path, sizeof(buff));
  }
  return buff;
}


//-----------------------------------------------------------------------------
// csGetPluginPaths
//-----------------------------------------------------------------------------
char** csGetPluginPaths()
{
  char** paths = new char* [4];     // Caller frees.
  char*  buff  = new char[1024];    // Caller frees.
  char*  cpath = csGetConfigPath(); // Caller frees.

  strncpy(buff, cpath, sizeof(buff));
  strncat(buff, OS_MACOSX_PLUGIN_DIR, sizeof(buff));
  buff[sizeof(buff) - 1] = '\0';

  paths[0] = buff;
  paths[1] = cpath;
  paths[2] = csStrNew(".");
  paths[3] = 0;

  return paths;
}
