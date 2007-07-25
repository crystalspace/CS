/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "csutil/util.h"

// Generic csExpandName for all platforms.

#if defined (CS_PLATFORM_DOS)
  #define IS_PATH_SEPARATOR(c)	\
    (((c) == CS_PATH_SEPARATOR) || ((c) == '/') || ((c) == ':'))
#else
  #define IS_PATH_SEPARATOR(c)	\
    (((c) == CS_PATH_SEPARATOR) || ((c) == '/'))
#endif

#ifdef CS_COMPILER_BCC
static int __getcwd (char drive, char *buffer, int buffersize) {
  _getdcwd(drive, buffer, buffersize);
  return strlen(buffer);
}
#endif

#if defined (CS_PLATFORM_DOS)
// We need a function to retrieve current working directory on specific drive

static int __getcwd (char drive, char *buffer, int buffersize)
{
  unsigned int old_drive, num_drives;
  _dos_getdrive (&old_drive);
  _dos_setdrive (drive, &num_drives);
  getcwd (buffer, buffersize);
  _dos_setdrive (old_drive, &num_drives);
  return strlen (buffer);
}

#endif // defined (CS_PLATFORM_DOS)

char *csExpandName (const char *iName)
{
  char outname [CS_MAXPATHLEN + 1];
  int inp = 0, outp = 0, namelen = strlen (iName);
  while ((outp < CS_MAXPATHLEN)
      && (inp < namelen))
  {
    char tmp [CS_MAXPATHLEN + 1];
    int ptmp = 0;
    while ((inp < namelen) && (!IS_PATH_SEPARATOR(iName[inp]) ))
      tmp [ptmp++] = iName [inp++];
    tmp [ptmp] = 0;

    if ((ptmp > 0)
     && (outp == 0)
#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
     && ((inp >= namelen)
      || (iName [inp] != ':'))
#endif
        )
    {
      getcwd (outname, sizeof (outname));
      outp = strlen (outname);
      if (strcmp (tmp, "."))
        outname [outp++] = CS_PATH_SEPARATOR;
    } /* endif */

#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
    // If path starts with '/' (root), get current drive
    if ((ptmp == 0)
     && (outp == 0))
    {
      getcwd (outname, sizeof (outname));
      if (outname [1] == ':')
        outp = 2;
    } /* endif */
#endif

    if (strcmp (tmp, "..") == 0)
    {
      while ((outp > 0)
          && ((outname [outp - 1] == '/')
           || (outname [outp - 1] == CS_PATH_SEPARATOR)
#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
           || (outname [outp - 1] == ':')
#endif
             )
            )
        outp--;
      while ((outp > 0)
          && (outname [outp - 1] != '/')
          && (outname [outp - 1] != CS_PATH_SEPARATOR)
#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
          && (outname [outp - 1] != ':')
#endif
            )
        outp--;
    }
    else if (strcmp (tmp, ".") == 0)
    {
      // do nothing
    }
    else if (strcmp (tmp, "~") == 0)
    {
      // strip all output path; start from scratch
      strcpy (outname, "~/");
      outp = 2;
    }
    else
    {
      memcpy (&outname [outp], tmp, ptmp);
      outp += ptmp;
      if (inp < namelen)
      {
#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
        if ((inp == 1)
         && (iName [inp] == ':'))
          if ((iName [inp + 1] == '/')
           || (iName [inp + 1] == CS_PATH_SEPARATOR))
            outname [outp++] = ':';
          else
            outp += __getcwd (iName [inp - 1], outname + outp - 1,
	      sizeof (outname) - outp + 1) - 1;
        if ((outname [outp - 1] != '/')
         && (outname [outp - 1] != CS_PATH_SEPARATOR))
#endif
        outname [outp++] = CS_PATH_SEPARATOR;
      }
    } /* endif */
    while ((inp < namelen)
        && ((iName [inp] == '/')
         || (iName [inp] == CS_PATH_SEPARATOR)
#if defined (CS_PLATFORM_DOS) || defined (CS_PLATFORM_WIN32)
         || (iName [inp] == ':')
#endif
           )
          )
      inp++;
  } /* endwhile */

  char *ret = new char [outp + 1];
  memcpy (ret, outname, outp);
  ret [outp] = 0;
  return ret;
}
