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

#include <sys/stat.h>
#include "cssysdef.h"
#include "csutil/cfgfile.h"
#include "csutil/sysfunc.h"
#include "shellstuff.h"

static void ReplaceReserved (char* key)
{
  size_t len = (size_t)strlen (key);
  size_t p;
  while ((p = strcspn (key, "<>:\"\\/|*?")) < len)
    *(key + p) = '_';
}

static void ReplaceSeparators (char* key)
{
  size_t len = (size_t)strlen (key);
  size_t p;
  while ((p = strcspn (key, ".")) < len)
    *(key + p) = '\\';
}

static void MakeDir (char* name)
{
  struct stat stats;
  if (stat (name, &stats) == 0)
    return;

  char* bslash = strrchr (name, '\\');
  if (!bslash)
    return;
  *bslash = 0;
  CS_ALLOC_STACK_ARRAY (char, upPath, strlen (name) + 1);
  strcpy (upPath, name);
  *bslash = '\\';

  MakeDir (upPath);
  CreateDirectoryEx (upPath, name, 0);
}

csPtr<iConfigFile> csGetPlatformConfig(const char* Key)
{
  char appDataPath [MAX_PATH + 1];

  // Try to retrieve "Application Data" directory
  if (!GetShellFolderPath (CSIDL_APPDATA, appDataPath))
  {
    // Fall back to My Documents
    if (!GetShellFolderPath (CSIDL_PERSONAL, appDataPath))
    {
      // Fail
      return 0;
    }
  }

  CS_ALLOC_STACK_ARRAY (
    char, realPath, strlen (appDataPath) + 1 + strlen (Key) + 5); 
  sprintf (realPath, "%s\\%s", appDataPath, Key);
  char* rpKey = realPath + strlen (appDataPath) + 1;
  ReplaceReserved (rpKey);
  ReplaceSeparators (rpKey);
  strcat (realPath, ".cfg");

  char* bslash = strrchr (realPath, '\\');
  if (bslash)
    *bslash = 0;
  // @@@ Would be nicer if this was only done when the config file is really 
  // saved to disk.
  MakeDir (realPath);
  if (bslash)
    *bslash = '\\';

  // Create/read a config file; okay if missing; will be created when written
  return new csConfigFile (realPath);
}
