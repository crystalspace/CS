/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Matthias Braun
	      (C) 2003 by Frank Richter

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

#include "shellstuff.h"

#if 0
static void ReplaceReserved (char* key)
{
  size_t len = (size_t)strlen (key);
  size_t p;
  while ((p = strcspn (key, "<>:\"\\/|*?")) < len)
  {
    *(key + p) = '_';
  }
}

static void ReplaceSeparators (char* key)
{
  size_t len = (size_t)strlen (key);
  size_t p;
  while ((p = strcspn (key, ".")) < len)
  {
    *(key + p) = '\\';
  }
}

static void MakeDir (char* name)
{
  struct stat stats;
  if (stat (name, &stats) == 0) return;

  char* bslash = strrchr (name, '\\');
  if (!bslash) return;
  *bslash = 0;
  CS_ALLOC_STACK_ARRAY (char, upPath,
    strlen (name) + 1);
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
    // fall back to My Documents
    if (!GetShellFolderPath (CSIDL_APPDATA, appDataPath))
    {
      // fail
      return 0;
    }
  }

  /*
    @@@ kludge: To avoid "app data" pollution, prepend
    a "CrystalSpaceApp" to Keys that don't have a 
    separator char (.). It's assumed that keys that 
    contain such separator use some kind of hierarchy, 
    e.g "<Maker>.<Application>". People really should make 
    app IDs in this style.
   */
  if (!strchr (Key, '.'))
  {
    strcat (appDataPath, "\\CrystalSpace");
  }

  CS_ALLOC_STACK_ARRAY (char, realPath, 
    strlen (appDataPath) + 1 + strlen (Key) + 5); 
  sprintf (realPath, "%s\\%s", appDataPath, Key);
  char* rpKey = realPath + strlen (appDataPath) + 1;
  ReplaceReserved (rpKey);
  ReplaceSeparators (rpKey);
  strcat (realPath, ".cfg");

  char* bslash = strrchr (realPath, '\\');
  if (bslash) *bslash = 0;
  MakeDir (realPath);
  if (bslash) *bslash = '\\';

  // create/read a config file
  csRef<csConfigFile> configfile = csPtr<csConfigFile> (new csConfigFile (realPath));
  /*
    It is NOT a failure if the config can't load. 
    When it's saved, the file is created automatically.
   */
    
  return csPtr<iConfigFile> (configfile);
}
#endif
