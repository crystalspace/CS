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
#include "cssys/sysfunc.h"
#include "cssys/syspath.h"
#include "csutil/util.h"

// @@@ Re-enable recursive scanning after we rethink it.  Presently, many
// developers run applications from within the source tree, and recursion
// causes a lot of needless scanning. For now it is disabled.
#define DO_SCAN_RECURSION false

csPluginPaths* csGetPluginPaths (const char* argv0)
{
  csPluginPaths* paths = new csPluginPaths;

  char* appPath = csGetAppDir (argv0);
  char* resPath = csGetResourceDir (argv0);
  char* configPath = csGetConfigPath ();
  
  // Don't add "/" since it won't work on Windows.
  if (resPath && *resPath && (resPath[1] != 0 || *resPath != PATH_SEPARATOR))
    paths->AddOnce (resPath, DO_SCAN_RECURSION, "app");
  if (appPath && *appPath && (appPath[1] != 0 || *appPath != PATH_SEPARATOR))
    paths->AddOnce (appPath, DO_SCAN_RECURSION, "app");

  if (configPath)
  {
    csString tmp;
    tmp << configPath << PATH_SEPARATOR << "lib";
    paths->AddOnce (tmp, DO_SCAN_RECURSION, "crystal");

    tmp.Clear();
    tmp << configPath << PATH_SEPARATOR << "crystal";
    paths->AddOnce (tmp, DO_SCAN_RECURSION, "crystal");

    paths->AddOnce (configPath, false, "crystal");
  }
    
  delete[] configPath;
  delete[] appPath;
  delete[] resPath;

  return paths;
}
