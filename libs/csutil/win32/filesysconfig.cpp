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
#include <sys/stat.h>
#include "csutil/cfgfile.h"
#include "csutil/sysfunc.h"
#include "shellstuff.h"

static void ReplaceReserved (csString& key, size_t offset)
{
  size_t p = offset;
  while ((p = key.FindFirst ("<>:\"\\/|*?", p)) != (size_t)-1)
    key[p] = '_';
}

static void ReplaceSeparators (csString& key, size_t offset)
{
  size_t p = offset;
  while ((p = key.FindFirst (".", p)) != (size_t)-1)
    key[p] = '\\';
}

static void MakeDir (const char* name)
{
  struct stat stats;
  if (stat (name, &stats) == 0)
    return;

  const char* bslash = strrchr (name, '\\');
  if (!bslash)
    return;
  const size_t len = bslash - name;
  CS_ALLOC_STACK_ARRAY (char, upPath, len + 1);
  strncpy (upPath, name, len);
  upPath[len] = 0;

  MakeDir (upPath);
  CreateDirectoryA (name, 0);
}

csPtr<iConfigFile> csGetPlatformConfig (const char* Key)
{
  csString path = csGetPlatformConfigPath (Key);
  path << ".cfg";

  size_t bslash = path.FindLast ('\\');
  if (bslash != (size_t)-1)
    path[bslash] = 0;
  // @@@ Would be nicer if this was only done when the config file is really 
  // saved to disk.
  MakeDir (path.GetData());
  if (bslash != (size_t)-1)
    path[bslash] = '\\';

  // Create/read a config file; okay if missing; will be created when written
  return new csConfigFile (path);
}

csString csGetPlatformConfigPath (const char* key)
{
  char appDataPath [MAX_PATH + 1];
  csString path;
  
  // Try to retrieve "Application Data" directory
  if (!GetShellFolderPath (CSIDL_APPDATA, appDataPath))
  {
    // Fall back to My Documents
    if (!GetShellFolderPath (CSIDL_PERSONAL, appDataPath))
    {
      // Guess...
      strcpy (appDataPath, ".");
    }
  }

  path << appDataPath << "\\" << key;
  const size_t adpl = strlen (appDataPath) + 1;
  ReplaceReserved (path, adpl);
  ReplaceSeparators (path, adpl);
  
  return path;
}
