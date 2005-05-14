//=============================================================================
//
//	Copyright (C) 2003,2004 by Eric Sunshine <sunshine@sunshineco.com>
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
// OSXGetDirs.cpp
//
//	Cocoa-specific functions for determining application and resource
//	paths.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "csutil/syspath.h"
#include "csutil/csstring.h"
#include "csutil/util.h"
#include <string.h>

#define OSX_WRAPPER_GRIST ".app/Contents/MacOS"
#define OSX_RESOURCES_GRIST ".app/Contents/Resources"


//-----------------------------------------------------------------------------
// app_info
//	Given argv[0], returns absolute path of the directory containing the
//	.app wrapper for GUI applications, or the directory containing the
//	executable for console applications.  Also returns the basename of the
//	executable itself.  Guarantees that `dir' and `name' will be empty if
//	unable to determine the application's path, or some other fatal problem
//	occurs.
//-----------------------------------------------------------------------------
bool app_info(char const* argv0, csString& dir, csString& name, bool& is_gui)
{
  bool ok = false;
  csString apppath = csInstallationPathsHelper::GetAppPath(argv0);
  if (!apppath.IsEmpty())
  {
    size_t slash = apppath.FindLast(CS_PATH_SEPARATOR);
    CS_ASSERT(slash != (size_t)-1);
    name = apppath.Slice(slash + 1, apppath.Length() - slash - 1);
    apppath.Truncate(slash);

    is_gui = false;
    int const n = apppath.Length();
    int const ngrist = sizeof(OSX_WRAPPER_GRIST) - 1;
    if (strcasecmp((char const*)apppath + n - ngrist, OSX_WRAPPER_GRIST) == 0)
    {
      is_gui = true;
      apppath.Truncate(n - ngrist);
      slash = apppath.FindLast(CS_PATH_SEPARATOR);
      CS_ASSERT(slash != (size_t)-1);
      apppath.Truncate(slash);
    }

    dir = apppath;
    ok = true;
  }
  else
  {
    dir.Clear();
    name.Clear();
    is_gui = false;
  }
  return ok;
}


//-----------------------------------------------------------------------------
// csGetAppDir
//	Return the directory containing the application.  Unlike most other
//	platforms, GUI executables on MacOS/X reside within an .app wrapper.
//	When CS invokes csGetAppDir(), it expects the directory in which the
//	wrapper itself resides; not the directory in which the actual
//	executable resides.  Consequently, MacOS/X requires a custom version of
//	csGetAppDir().  Note that for console applications, which do not reside
//	within a wrapper, we just return the directory containing the
//	executable.  Returns the empty string if unable to determine the
//	directory.
//-----------------------------------------------------------------------------
csString csInstallationPathsHelper::GetAppDir(char const* argv0)
{
  csString dir, name;
  bool is_gui;
  app_info(argv0, dir, name, is_gui);
  return dir;
}


//-----------------------------------------------------------------------------
// csGetResourceDir
//	Return the directory containing the application's resources.  Unlike
//	most other platforms, GUI executables on MacOS/X reside within an .app
//	wrapper, and application resources might reside within the wrapper's
//	"Resources" directory.  Consequently, MacOS/X requires a custom version
//	of csGetResourceDir().  Note that for console applications, which do
//	not reside within a wrapper, we just return the directory containing
//	the executable, which happens to be the default behavior for
//	non-MacOS/X platforms.  Returns the empty string if unable to determine
//	the directory.
//-----------------------------------------------------------------------------
csString csInstallationPathsHelper::GetResourceDir(char const* argv0)
{
  csString dir, name;
  bool is_gui;
  if (app_info(argv0, dir, name, is_gui) && is_gui)
    dir << CS_PATH_SEPARATOR << name << OSX_RESOURCES_GRIST;
  return dir;
}
