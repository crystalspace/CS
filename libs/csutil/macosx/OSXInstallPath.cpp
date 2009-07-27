//=============================================================================
//
//	Copyright (C) 2004 by Eric Sunshine <sunshine@sunshineco.com>
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
/* Small hack: include the _template_ as only the major & minor version is
   needed here */
#include "csver.h.template"
#include "csutil/stringarray.h"
#include "csutil/syspath.h"
#include "OSXInstallPath.h"
#include <stdio.h>
#include <stdlib.h>

#define VERSION_STR         CS_VERSION_MAJOR "_" CS_VERSION_MINOR
#define VERSION_STR_DOTTED  CS_VERSION_MAJOR "." CS_VERSION_MINOR

//-----------------------------------------------------------------------------
// is_config_dir
//-----------------------------------------------------------------------------
static bool is_config_dir(csString path)
{
  if (!path.IsEmpty() && path[path.Length() - 1] != CS_PATH_SEPARATOR)
    path << CS_PATH_SEPARATOR;
  path << "vfs.cfg";
  return (access(path, F_OK) == 0);
}


//-----------------------------------------------------------------------------
// test_config_dir
//-----------------------------------------------------------------------------
static csString test_config_dir(csString path)
{
  if (!path.IsEmpty())
  {
    if (is_config_dir(path))
      return path;

    if (path[path.Length() - 1] != CS_PATH_SEPARATOR)
      path << CS_PATH_SEPARATOR;
    path << "etc" << CS_PATH_SEPARATOR << CS_PACKAGE_NAME;
    if (is_config_dir(path))
      return path;
  }
  return "";
}


//-----------------------------------------------------------------------------
// csGetConfigPath
//-----------------------------------------------------------------------------
csString csGetConfigPath()
{
  csStringArray candidates;
  char buff[FILENAME_MAX];
  char* env;

  if (OSXGetInstallPath(buff, FILENAME_MAX, CS_PATH_SEPARATOR))
    candidates.Push(buff);
  env = getenv("CRYSTAL_" VERSION_STR);
  if (!env || !*env)
    env = getenv ("CRYSTAL");
  if (env != 0 && *env != '\0')
  {
    csString crystalPath (env);

    size_t pos = 0;
    while (pos < crystalPath.Length())
    {
      size_t colon = crystalPath.FindFirst (':', pos);
      size_t subStrLen;
      if (colon == (size_t)-1)
	subStrLen = crystalPath.Length() - pos;
      else
	subStrLen = colon - pos;

      candidates.Push (crystalPath.Slice (pos, subStrLen));
      
      pos += subStrLen + 1;
    }
  }
  candidates.Push(".");
#if defined(CS_CONFIGDIR)
  candidates.Push(CS_CONFIGDIR);
#endif
  candidates.Push("/Library/CrystalSpace" VERSION_STR_DOTTED);
  candidates.Push("/Library/CrystalSpace");
  candidates.Push("/usr/local");

  for (size_t i = 0, n = candidates.GetSize(); i < n; i++)
  {
    csString const path = test_config_dir(candidates[i]);
    if (!path.IsEmpty())
      return path;
  }

  return ".";
}


//-----------------------------------------------------------------------------
// csInstallationPathsHelper::GetPlatformInstallationPaths
//-----------------------------------------------------------------------------
csPathsList* csInstallationPathsHelper::GetPlatformInstallationPaths()
{
  csPathsList* paths = new csPathsList;

  char const* envpath = getenv ("CRYSTAL_" VERSION_STR);
  if (!envpath || !*envpath)
    envpath = getenv ("CRYSTAL");
  if (envpath != 0 && *envpath != '\0')
    paths->AddUnique(csPathsUtilities::ExpandAll(csPathsList(envpath)));

  char buff[FILENAME_MAX];
  if (OSXGetInstallPath(buff, FILENAME_MAX, CS_PATH_SEPARATOR))
    paths->AddUniqueExpanded(buff);

  paths->AddUniqueExpanded(".");
#if defined(CS_CONFIGDIR)
  paths->AddUniqueExpanded(CS_CONFIGDIR);
#endif
  paths->AddUniqueExpanded("/Library/CrystalSpace" VERSION_STR_DOTTED);
  paths->AddUniqueExpanded("/Library/CrystalSpace");
  paths->AddUniqueExpanded("/usr/local");
  return paths;
}
