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
//	Platform-specific function to determine installation path.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "OSXInstallPath.h"
#include <stdlib.h>

#include "csutil/util.h"

//-----------------------------------------------------------------------------
// csGetInstallPath
//-----------------------------------------------------------------------------
char* csGetConfigPath ()
{
  char* buff = new char[1024];
    
  if (OSXGetInstallPath(buff, 1024, PATH_SEPARATOR))
  {
    return buff;
  }
  
  char const* path = getenv("CRYSTAL");
  if (!path || *path == 0)
  {
    strncpy (buff, ".", 1024);
    return buff;
  }
  
  strncpy (buff, path, 1024);  
  return buff;
}

char** csGetPluginPaths ()
{
  char** paths = new char* [4];

  paths[0] = csGetConfigPath ();
  paths[1] = csStrNew (OS_MACOSX_PLUGIN_DIR);
  paths[2] = csStrNew (".");
  paths[3] = 0;

  return paths; 
}    
