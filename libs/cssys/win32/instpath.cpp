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
#include "cssys/sysfunc.h"
#include "cssys/win32/shellstuff.h"
#include <windows.h>
#include <shlobj.h>
#include <winreg.h>
#include <stdio.h>
#include <string.h>

#include "csutil/util.h"

static inline bool
GetRegistryInstallPath (const HKEY parentKey, char *oInstallPath, size_t iBufferSize)
{
  char * pValueName = "InstallPath";
  DWORD dwType;
  DWORD bufSize = iBufferSize;
  HKEY m_pKey;
  LONG result;

  result =
    RegCreateKey(parentKey, "Software\\CrystalSpace", &m_pKey);
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

char* csGetConfigPath ()
{
  // override the default to get install path from
  // 1. CRYSTAL environment variable
  // 2. this machine's system registry
  // 3. if current working directory contains 'scf.cfg' use this dir.
  // 4. The dir where the app is
  // 5. A "Crystal" subfolder under the "Program Files" dir.
  // 6. hard-wired default path

  // try env variable first
  // we check this before we check registry, so that one app can putenv() to
  // use a special private build of CrystalSpace, while other apps fall back on
  // the shared systemwide registry strategy; this is the best approach unless
  // someone implements a SetInstallPath() to override it before Open() is
  // called.
  char *path = getenv ("CRYSTAL");
  if (path && *path)
    return csStrNew (path);

  // try the registry
  path = new char[1024];
  if (GetRegistryInstallPath (HKEY_CURRENT_USER, path, 1024))
    return path;
  if (GetRegistryInstallPath (HKEY_LOCAL_MACHINE, path, 1024))
    return path;

  // perhaps current drive/dir?
  FILE *test = fopen("scf.cfg", "r");
  if(test != NULL)
  {
    // use current dir
    fclose(test);
    strcpy(path, ".");
    return path;
  }

  // directory where app is?
  char apppath[MAX_PATH + 1];
  GetModuleFileName (0, apppath, sizeof(apppath)-1);
  char* slash = strrchr (apppath, '\\');
  if (slash) *(slash+1) = 0;

  char testfn[MAX_PATH];
  strcpy(testfn, apppath);
  strcat(testfn, "scf.cfg");

  test = fopen(testfn, "r");
  if(test != NULL)
  {
    // use current dir
    fclose(test);
    strncpy (path, apppath, 1024);
    return path;
  }

  // retrieve the path of the Program Files folder and append 
  // a "\Crystal\".
  if (MinShellDllVersion(5, 0))
  {
    LPMALLOC MAlloc;
    LPITEMIDLIST pidl;
    char programpath[MAX_PATH];

    programpath[0] = 0;
    if (SUCCEEDED(SHGetMalloc (&MAlloc)))
    {
      if (SUCCEEDED(SHGetSpecialFolderLocation (0, CSIDL_PROGRAM_FILES, &pidl)))
      {
	if (SUCCEEDED(SHGetPathFromIDList (pidl, programpath)))
	{
	  strncpy (path, programpath, MIN(sizeof(programpath), 1024-30));
	  strcat (path, "\\Crystal\\");
	}
	MAlloc->Free (pidl);
      }
      MAlloc->Release ();
    }
    if (programpath[0]) 
      return path;
  }

  // nothing helps, use default
  // which is "C:\Program Files\Crystal\"
  strncpy(path, "C:\\Program Files\\Crystal\\", 1024);

  return path;
}

char** csGetPluginPaths ()
{
  char** paths = new char* [4];

  char* configpath = csGetConfigPath ();
  char* temp = new char[MAX_PATH];
  strncpy (temp, configpath, MAX_PATH);
  strcat (temp, "\\lib");
    
  paths[0] = temp;
  paths[1] = configpath;
  paths[2] = ".";
  paths[3] = 0;

  return paths;
}

