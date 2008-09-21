/*
    Copyright (C) 2002 by Frank Richter

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

// Support for platform-specific VFS variables.
#include "cssysdef.h"
#include "csutil/vfsplat.h"
#include "csutil/sysfunc.h"

#ifdef __CYGWIN__
#include <sys/cygwin.h>
#endif

#include "shellstuff.h"

// Windows has built-in var "SystemRoot"
// (env var on NT, but not 9x; so we provide it this way)
const char* csCheckPlatformVFSVar(const char* VarName)
{
  if (!strcasecmp(VarName, "systemroot"))
  {
    static char szWindowsDirectory[MAX_PATH+1] = {'\0'};

    if (!*szWindowsDirectory) 
    {
      GetWindowsDirectoryA(szWindowsDirectory, MAX_PATH);
    }
    return szWindowsDirectory;
  }
  
  if (!strcasecmp(VarName, "homedir"))
  {
    static char szMyDocs[MAX_PATH+1] = {'\0'};

    if (!*szMyDocs) 
    {
      if (!GetShellFolderPath (CSIDL_PERSONAL, szMyDocs)) return 0;
    }
    return szMyDocs;
  }

  if (!strcasecmp(VarName, "localappdata"))
  {
    static char localAppDataPath[MAX_PATH+13] = {'\0'};

    if (!*localAppDataPath) 
    {
      csString path (csGetPlatformConfigPath ("CrystalSpace", true));
      strcpy (localAppDataPath, path);
    }
    return localAppDataPath;
  }

  return 0;
}

void csExpandPlatformFilename(const char *inputFilename, char *outputFilename)
{
#ifdef __CYGWIN__
  // Convert any cygwin paths to win32 paths
  if (cygwin_conv_to_win32_path(inputFilename, outputFilename) == 0)
    return;
#endif
  strcpy(outputFilename, inputFilename);
}
