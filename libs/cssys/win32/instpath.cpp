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
#include "cssys/system.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>

bool csGetInstallPath (char *oInstallPath, size_t iBufferSize)
{
  // override the default to get install path from
  // 1. CRYSTAL environment variable
  // 2. this machine's system registry
  // 3. if current working directory contains 'scf.cfg' use this dir.
  // 4. hard-wired default path

  //@@@ this function is called several times; maybe cache the found dir

  // try env variable first
  // we check this before we check registry, so that one app can putenv() to
  // use a special private build of CrystalSpace, while other apps fall back on
  // the shared systemwide registry strategy; this is the best approach unless
  // someone implements a SetInstallPath() to override it before Open() is
  // called.
  if (1)
  {
    char *path = getenv ("CRYSTAL");
    if (path && *path)
    {
      strncpy(oInstallPath, path, iBufferSize);
      goto got_value;
    }
  }

  // try the registry
  if (1)
  {
    char * pValueName = "installpath";
    DWORD dwType;
    DWORD bufSize = iBufferSize;
    HKEY m_pKey;
    LONG result;

    result =
      RegCreateKey(HKEY_LOCAL_MACHINE, "Software\\CrystalSpace", &m_pKey);
    if (result == ERROR_SUCCESS)
    {
      result = RegQueryValueEx(
	m_pKey, 
	pValueName, 
	0,
	&dwType,
	(unsigned char *)oInstallPath,
	&bufSize);
 
      if (ERROR_SUCCESS == result)
      {
	goto got_value;
      }
    }
  }

  //@@@ might try GetModuleFilename(NULL,...) here for application's directory

  // perhaps current drive/dir? 
  if (1)
  {
    FILE *test = fopen("scf.cfg", "r");
    if(test != NULL)
    {
      // usr current dir
      fclose(test);
      strncpy(oInstallPath, "", iBufferSize);
      goto got_value;
    }
  }

  // nothing helps, use default
  // which is C:\Program Files\Crystal\ 
  strncpy(oInstallPath, "C:\\Program Files\\Crystal\\", iBufferSize);

got_value:
  // got the value in oInstallPath, check for ending '/' or '\'.
  int len = strlen(oInstallPath);
  if(len == 0) return false;
  if( oInstallPath[len-1] == '/' || oInstallPath[len-1] == '\\' )
    return true;
  if(len+1 >= (int)iBufferSize)
  {
    strncpy(oInstallPath, "", iBufferSize); //make empty if possible
    return false;
  }
  oInstallPath[len] = '\\';
  oInstallPath[len+1] = 0;
  return true;
}
