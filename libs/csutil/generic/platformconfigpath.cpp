/*
    Copyright (C) 2003 by Matthias Braun

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

/**
 * This file implements a csGetPlatformConfigPath which returns a path in 
 * $HOME/.crystalspace which is appropriate for most posixish systems.
 */
#include <sys/stat.h>

#include "cssysdef.h"
#include "csutil/csstring.h"
#include "csutil/sysfunc.h"

csString csGetPlatformConfigPath (const char* key, bool /*local*/)
{
  csString dir;
  
  // Is $HOME set? otherwise fallback to standard mode
  const char* home = getenv("HOME");
  if (home == 0)
  {
    home = ".";
  }
  
  // Construct directory and filename of the config file
  csString path;
  path << home << CS_PATH_SEPARATOR << "." CS_PACKAGE_NAME 
    << CS_PATH_SEPARATOR << key;
  
  return path;
}
