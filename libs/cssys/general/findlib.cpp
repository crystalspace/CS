/*
    Copyright (C) 2000 by Andrew Zabolotny
    Shared library path support
  
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

#define SYSDEF_PATH
#include "cssysdef.h"
#include "cssys/csshlib.h"
#include "cssys/system.h"
#include "csutil/csstrvec.h"
#include "csutil/util.h"

static csStrVector LibPath (4, 4);
bool findlib_search_nodir = true;

void csAddLibraryPath (const char *iPath)
{
  LibPath.Push (csStrNew (iPath));
}

csLibraryHandle csFindLoadLibrary (const char *iPrefix, const char *iName,
  const char *iSuffix)
{
  for (int i = findlib_search_nodir ? -1 : 0; i < LibPath.Length (); i++)
  {
    char lib [MAXPATHLEN + 1];

    // For i == -1 try without any prefix at all
    if (i >= 0)
      strcpy (lib, LibPath.Get (i));
    else
      lib [0] = 0;

    size_t sl = strlen (lib);
    for (int j = 0; j < 2; j++)
    {
      if (j)
        strcpy (lib + sl, iPrefix);
      else
        lib [sl] = 0;
      strcat (strcat (lib + sl, iName), iSuffix);
      csLibraryHandle lh = csLoadLibrary (lib);
      if (lh)
        return lh;
      if (!iPrefix)
        break;
    }
  }

  csPrintLibraryError (iName);
  return (csLibraryHandle)0;
}
