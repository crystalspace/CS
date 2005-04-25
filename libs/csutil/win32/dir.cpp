/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#ifdef CS_WIN32_USE_CUSTOM_OPENDIR

struct DIR
{
  HANDLE findHandle;
  dirent currentEntry;
  WIN32_FIND_DATA findData;

  DIR()
  {
    findHandle = INVALID_HANDLE_VALUE;
  }
};

extern "C"
{

DIR *opendir (const char *name)
{
  DIR *dh = new DIR;
  if (!dh)
    return 0;

  const size_t namelen = strlen (name);
  CS_ALLOC_STACK_ARRAY (char, fullname, namelen + 3);
  strcpy (fullname, name);
  char* nameend = &fullname[namelen - 1];
  if (*nameend != CS_PATH_SEPARATOR)
  {
    nameend++;
    *nameend = CS_PATH_SEPARATOR;
  }
  strcpy (nameend + 1, "*");

  if ((dh->findHandle = FindFirstFileA (fullname, &dh->findData)) == 
    INVALID_HANDLE_VALUE)
  {
    delete dh;
    return 0;
  }
 
  return dh;
}

dirent *readdir (DIR *dirp)
{
  if (dirp->findHandle != INVALID_HANDLE_VALUE)
  {
    size_t nameLen = strlen (dirp->findData.cFileName);
    nameLen = MIN (nameLen, sizeof (dirp->currentEntry.d_name) - 1);
    memcpy (dirp->currentEntry.d_name, dirp->findData.cFileName, 
      nameLen);
    dirp->currentEntry.d_name[nameLen] = 0;
    dirp->currentEntry.d_size = dirp->findData.nFileSizeLow;
    dirp->currentEntry.dwFileAttributes = dirp->findData.dwFileAttributes;
    if (!FindNextFileA (dirp->findHandle, &dirp->findData))
    {
      FindClose (dirp->findHandle);
      dirp->findHandle = INVALID_HANDLE_VALUE;
    }
    return &dirp->currentEntry;
  }
  return 0;
}

int closedir (DIR *dirp)
{
  if (dirp->findHandle != INVALID_HANDLE_VALUE)
  {
    FindClose (dirp->findHandle);
  }
  delete dirp;
  return 0;
}
    
bool isdir (const char *path, dirent *de)
{
  (void)path;
  return (de->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

}
#endif
