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

#define CS_SYSDEF_PROVIDE_DIR
#include "cssysdef.h"
#include "cssys/system.h"
#include "cssys/win32/win32.h"

#ifdef __NEED_OPENDIR_PROTOTYPE

extern "C" 
{

DIR *opendir (const char *name)
{
  DIR *dh = new DIR;
  if (!dh)
    return NULL;

  char tname [CS_MAXPATHLEN + 1];
  strcpy (tname, name);
  strcat (tname, "\\*");

  if ((dh->handle = _findfirst (tname, &dh->fd)) == -1L)
  {
    delete dh;
    return NULL;
  } /* endif */

  dh->valid = true;
  return dh;
}

dirent *readdir (DIR *dirp)
{
  while (dirp->valid)
  {
    strcpy (dirp->de.d_name, dirp->fd.name);
    dirp->de.d_size = dirp->fd.size;
    dirp->de.d_attr = dirp->fd.attrib;
    dirp->valid = (_findnext (dirp->handle, &dirp->fd) == 0);
    if (strcmp (dirp->de.d_name, ".")
     && strcmp (dirp->de.d_name, ".."))
      return &dirp->de;
  } /* endwhile */
  return NULL;
}

int closedir (DIR *dirp)
{
  _findclose (dirp->handle);
  delete dirp;
  return 0;
}

}
#endif
