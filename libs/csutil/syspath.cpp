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
#include "csutil/csstring.h"
#include "csutil/syspath.h"

void csPathsList::Entry::FixSeparators()
{
  if (CS_PATH_SEPARATOR == '/') return;

  size_t p = 0;
  while ((p = path.FindFirst ('/', p)) != (size_t)-1)
  {
    path[p] = CS_PATH_SEPARATOR;
    p++;
  }
}

csPathsList::csPathsList (const char* pathList)
{
  csStringFast<CS_MAXPATHLEN> scratch;

  const char* remain = pathList;
  const char* delim;
  while ((delim = strchr (remain, CS_PATH_DELIMITER)) != 0)
  {
    scratch.Replace (remain, delim - remain);
    AddUnique (scratch);
    remain = delim + 1;
  }
  if ((remain != 0) && (*remain != 0))
  {
    AddUnique (remain);
  }
}

csPathsList::csPathsList (const char* pathList[])
{
  const char* const* pp = pathList;

  while (*pp)
  {
    AddUnique (*pp);
    pp++;
  }
}

size_t csPathsList::AddUnique (const char* path1, bool scanRecursive, 
			       const char* type, bool overrideRecursive)
{
  if (path1 == 0) return (size_t)-1;

  csString path (path1);
  if ((path.Length() > 1) && ((path[path.Length()-1] == CS_PATH_SEPARATOR)
    || (path[path.Length()-1] == '/')))
    path.Truncate (path.Length()-1);

  size_t i;
  for (i = 0; i < paths.Length(); i++)
  {
    if (csPathsUtilities::PathsIdentical (path, paths[i].path))
    {
      if (overrideRecursive)
      {
	paths[i].scanRecursive = scanRecursive;
      }
      paths[i].type = type;
      return i;
    }
  }

  Entry pluginPath (path, type, scanRecursive);
  return (paths.Push (pluginPath));
}

size_t csPathsList::AddUniqueExpanded (const char* path, bool scanRecursive, 
				       const char* type, bool overrideRecursive)
{
  char* pathExpanded = csPathsUtilities::ExpandPath (path);
  if (pathExpanded == 0) return (size_t)-1;
  size_t ret = AddUnique (pathExpanded, scanRecursive, type, overrideRecursive);
  delete[] pathExpanded;
  return ret;
}

size_t csPathsList::AddUnique (const Entry& path, bool overrideRecursive)
{
  return AddUnique (path.path, path.scanRecursive, path.type, overrideRecursive);
}

size_t csPathsList::AddUniqueExpanded (const Entry& path, 
				       bool overrideRecursive)
{
  return AddUniqueExpanded (path.path, path.scanRecursive, path.type, overrideRecursive);
}

csPathsList operator* (const csPathsList& left, const csPathsList& right)
{
  csPathsList paths;
  csStringFast<CS_MAXPATHLEN> scratch;

  for (size_t l = 0; l < left.Length(); l++)
  {
    const csPathsList::Entry& p1 = left[l];
    for (size_t r = 0; r < right.Length(); r++)
    {
      const csPathsList::Entry& p2 = right[r];
      scratch.Replace (p1.path);
      if ((scratch.Length() > 1) 
	|| ((scratch.Length() == 1) && (scratch.GetAt(0) != CS_PATH_SEPARATOR)))
      {
	scratch << CS_PATH_SEPARATOR;
      }
      scratch << p2.path;

      paths.AddUnique (scratch, p1.scanRecursive && p2.scanRecursive,
	p1.type);
    }
  }

  return paths;
}

//---------------------------------------------------------------------------

// compatibility goop
#ifndef F_OK
#define F_OK  0
#endif
#ifndef W_OK
#define W_OK  2
#endif
#ifndef R_OK
#define R_OK  4
#endif

#if defined(CS_PLATFORM_WIN32)
#include <io.h>
#if !defined(__CYGWIN__)
#define access	_access
#endif
#endif

void csPathsUtilities::FilterInvalid (csPathsList& paths)
{
  size_t i = paths.Length();
  while (i-- > 0)
  {
    if (access (paths[i].path, F_OK) != 0)
      paths.DeleteIndex (i);
  }
}
csPathsList csPathsUtilities::ExpandAll (const csPathsList& paths)
{
  csPathsList newPaths;
  for (size_t i = 0; i < paths.Length(); i++)
  {
    newPaths.AddUniqueExpanded (paths[i]);
  }
  return newPaths;
}

//---------------------------------------------------------------------------

// @@@ Re-enable recursive scanning after we rethink it.  Presently, many
// developers run applications from within the source tree, and recursion
// causes a lot of needless scanning. For now it is disabled.
#define DO_SCAN_RECURSION false

csPathsList* csInstallationPathsHelper::GetPluginPaths (const char* argv0)
{
  static const char* pluginSubdirs[] = {
    "lib", 
    "lib/" CS_PACKAGE_NAME, 
    CS_PACKAGE_NAME "/lib", 
    CS_PACKAGE_NAME,
    "",
    0};

  csPathsList paths;

  csString appPath = GetAppDir (argv0);
  csString resPath = GetResourceDir (argv0);
  csPathsList* configPaths = GetPlatformInstallationPaths ();
  
  // Don't add "/" since it won't work on Windows.
  if (!resPath.IsEmpty() && resPath != CS_PATH_SEPARATOR)
    paths.AddUnique (resPath, DO_SCAN_RECURSION, "app");
  if (!appPath.IsEmpty() && appPath != CS_PATH_SEPARATOR)
    paths.AddUnique (appPath, DO_SCAN_RECURSION, "app");

  paths.AddUnique (*configPaths * csPathsList (pluginSubdirs));

  const char* crystal_plugin = getenv("CRYSTAL_PLUGIN");
  if (crystal_plugin)
    paths.AddUniqueExpanded(crystal_plugin, DO_SCAN_RECURSION, CS_PACKAGE_NAME);

#ifdef CS_PLUGINDIR
  paths.AddUnique (CS_PLUGINDIR, DO_SCAN_RECURSION, CS_PACKAGE_NAME);
#endif

  delete configPaths;

  csPathsList* newPaths = 
    new csPathsList (csPathsUtilities::ExpandAll (paths));
  csPathsUtilities::FilterInvalid (*newPaths);

  return newPaths;
}
