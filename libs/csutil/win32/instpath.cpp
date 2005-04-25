/*
    Copyright (C) 2001 by Wouter Wijngaards

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
#include "shellstuff.h"
#include <windows.h>
#include <shlobj.h>
#include <winreg.h>
#include <stdio.h>
#include <string.h>

#include "csutil/util.h"

static inline bool GetRegistryInstallPath (const HKEY parentKey, 
					   char *oInstallPath, 
					   DWORD iBufferSize)
{
  char * pValueName = "InstallPath";
  DWORD dwType;
  DWORD bufSize = iBufferSize;
  HKEY m_pKey;
  LONG result;

  result = RegOpenKeyEx (parentKey, "Software\\CrystalSpace",
    0, KEY_READ, &m_pKey);
  if (result == ERROR_SUCCESS)
  {
    result = RegQueryValueEx(
      m_pKey,
      pValueName,
      0,
      &dwType,
      (unsigned char *)oInstallPath,
      &bufSize);

    RegCloseKey (m_pKey);

    if ((ERROR_SUCCESS == result) && 
      ((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ)))
    {
      if (dwType == REG_EXPAND_SZ)
      {
	char expandedPath[MAX_PATH];

	ExpandEnvironmentStrings (oInstallPath, expandedPath, 
	  sizeof(expandedPath));
	strcpy (oInstallPath, expandedPath);
      }
      return true;
    }
  }
  return false;
}

// ensures that the path has no trailing path delimiter
static inline char* NewPathWOTrailingDelim (const char *path)
{
  char *newPath = csExpandPath (path);
  if ((newPath != 0) && (strlen (newPath) > 0))
  {
    char *end = &newPath[strlen(newPath) - 1];
    if ((*end == '/') || (*end == '\\'))
      *end = 0;
    return newPath;
  }
  else
  {
    return 0;
  }
}

static inline char* FindConfigPath ()
{
  char* retPath = 0;
  // override the default to get install path from
  // 1. CRYSTAL environment variable
  // 2. this machine's system registry
  // 3. if current working directory contains 'vfs.cfg' use this dir.
  // 4. The dir where the app is
  // 5. A "CrystalSpace" subfolder under the "Program Files" dir.
  // 6. hard-wired default path

  // try env variable first
  // we check this before we check registry, so that one app can putenv() to
  // use a special private build of CrystalSpace, while other apps fall back on
  // the shared systemwide registry strategy; this is the best approach unless
  // someone implements a SetInstallPath() to override it before Open() is
  // called.
  char *envpath = getenv ("CRYSTAL");
  if (envpath && *envpath)
  {
    // Multiple paths. Take first one...
    // @@@ Check for vfs.cfg?
    csString crystalPath (envpath);

    size_t colon = crystalPath.FindFirst (';'); 
      // MSYS converts :-separated paths to ;-separation in Win32 style.
    size_t subStrLen;
    if (colon == (size_t)-1)
      subStrLen = crystalPath.Length();
    else
      subStrLen = colon;
    
    if ((retPath = NewPathWOTrailingDelim (
      crystalPath.Slice (0, subStrLen))) != 0) return retPath;
  }

  // try the registry
  char path[1024];
  if (GetRegistryInstallPath (HKEY_CURRENT_USER, path, 1024))
    if ((retPath = NewPathWOTrailingDelim (path)) != 0) return retPath;
  if (GetRegistryInstallPath (HKEY_LOCAL_MACHINE, path, 1024))
    if ((retPath = NewPathWOTrailingDelim (path)) != 0) return retPath;

  // perhaps current drive/dir?
  FILE *test = fopen("vfs.cfg", "r");
  if(test != 0)
  {
    // use current dir
    fclose(test);
    strcpy(path, ".");
    return csStrNew (path);
  }

  // directory where app is?
  char apppath[MAX_PATH + 1];
  GetModuleFileName (0, apppath, sizeof(apppath)-1);
  char* slash = strrchr (apppath, '\\');
  if (slash) *(slash+1) = 0;

  char testfn[MAX_PATH];
  strcpy(testfn, apppath);
  strcat(testfn, "vfs.cfg");

  test = fopen(testfn, "r");
  if(test != 0)
  {
    // use current dir
    fclose(test);
    if ((retPath = NewPathWOTrailingDelim (apppath)) != 0) return retPath;
  }

  // retrieve the path of the Program Files folder and append 
  // a "\CrystalSpace".
  {
    char programpath [MAX_PATH+1];

    if (GetShellFolderPath (CSIDL_PROGRAM_FILES, programpath))
    {
      size_t maxLen = MIN(sizeof(programpath), 1024-30);
      memcpy (path, programpath, maxLen);
      path[maxLen] = 0;
      strcat (path, "\\" CS_PACKAGE_NAME);
      return csStrNew (path);
    }
  }

  // nothing helps, use default
  // which is "C:\Program Files\CrystalSpace"
  strcpy (path, "C:\\Program Files\\" CS_PACKAGE_NAME);

  return csStrNew (path);
}

// cache configuration path
struct _CfgPath {
  char* path;

  _CfgPath() : path(0) { };
  ~_CfgPath() { delete[] path; };
};

CS_IMPLEMENT_STATIC_VAR (getCachedCfgPath, _CfgPath, ())

csString csGetConfigPath ()
{
  _CfgPath *cachedCfgPath = getCachedCfgPath();
  if (cachedCfgPath->path == 0)
  {
    cachedCfgPath->path = FindConfigPath();
  }
  return cachedCfgPath->path;
}


