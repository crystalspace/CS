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
#include "csutil/util.h"

// @@@ Re-enable recursive scanning after we rethink it.  Presently, many
// developers run applications from within the source tree, and recursion
// causes a lot of needless scanning. For now it is disabled.
#define DO_SCAN_RECURSION false

csPathsList* csGetPluginPaths (const char* argv0)
{
  csPathsList* paths = new csPathsList;

  csString appPath = csInstallationPathsHelper::GetAppDir (argv0);
  csString resPath = csInstallationPathsHelper::GetResourceDir (argv0);
  csString configPath = csGetConfigPath ();
  
  // Don't add "/" since it won't work on Windows.
  if (!resPath.IsEmpty() && resPath != CS_PATH_SEPARATOR)
    paths->AddUniqueExpanded (resPath, DO_SCAN_RECURSION, "app");
  if (!appPath.IsEmpty() && appPath != CS_PATH_SEPARATOR)
    paths->AddUniqueExpanded (appPath, DO_SCAN_RECURSION, "app");

  if (!configPath.IsEmpty())
  {
    csString tmp;
    tmp << configPath << CS_PATH_SEPARATOR << "lib";
    paths->AddUniqueExpanded (tmp, DO_SCAN_RECURSION, CS_PACKAGE_NAME);

    tmp.Clear();
    tmp << configPath << CS_PATH_SEPARATOR << "lib" << CS_PATH_SEPARATOR
	<< CS_PACKAGE_NAME;
    paths->AddUniqueExpanded (tmp, DO_SCAN_RECURSION, CS_PACKAGE_NAME);

    tmp.Clear();
    tmp << configPath << CS_PATH_SEPARATOR << CS_PACKAGE_NAME
	<< CS_PATH_SEPARATOR << "lib";
    paths->AddUniqueExpanded (tmp, DO_SCAN_RECURSION, CS_PACKAGE_NAME);

    tmp.Clear();
    tmp << configPath << CS_PATH_SEPARATOR << CS_PACKAGE_NAME;
    paths->AddUniqueExpanded (tmp, DO_SCAN_RECURSION, CS_PACKAGE_NAME);

    paths->AddUniqueExpanded (configPath, false, CS_PACKAGE_NAME);

#ifdef CS_PLUGINDIR
    paths->AddUniqueExpanded (CS_PLUGINDIR, DO_SCAN_RECURSION, CS_PACKAGE_NAME);
#endif
  }
    
  return paths;
}
