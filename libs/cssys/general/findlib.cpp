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

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CS_SYSDEF_PROVIDE_PATH
#include "cssysdef.h"
#include "cssys/csshlib.h"
#include "cssys/system.h"
#include "csutil/csstrvec.h"
#include "csutil/util.h"

CS_IMPLEMENT_STATIC_VAR (GetLibPath, csStrVector, (4, 4))
bool findlib_search_nodir = true;

void csAddLibraryPath (const char *iPath)
{
  GetLibPath ()->Push (csStrNew (iPath));
}

const int CALLBACK_STOP = 0;
const int CALLBACK_CONTINUE = 1;

class iCallback
{
public:
  virtual int File (const char *file) = 0;
};

class callbackLoadLibrary : public iCallback
{
public:
  callbackLoadLibrary ()
  {
    _handle = 0;
    _file_found = 0;
  }
  
  virtual int File (const char *file)
  {
    //struct stat st;
    //if (stat (file, &st) == 0)
    //{
      _handle = csLoadLibrary (file);
      if (!_handle) return CALLBACK_CONTINUE;
      _file_found++;
      return CALLBACK_STOP;
    //}
    //return CALLBACK_CONTINUE;
  }

  csLibraryHandle _handle;
  int _file_found;
};

class callbackPrint : public iCallback
{
public:
  virtual int File (const char *file)
  {
    fprintf (stderr, "\t%s: ", file);
    csPrintLibraryError (file);
    fprintf (stderr, "\n");
    return CALLBACK_CONTINUE;
  }
};

static void csFindLoadLibraryHelper (const char *iPrefix, const char *iName,
  const char *iSuffix, iCallback& callback)
{
  int i, j;

  for (i = findlib_search_nodir ? -1 : 0; i < GetLibPath ()->Length (); i++)
  {
    char lib [CS_MAXPATHLEN + 1];

    // For i == -1 try without any prefix at all
    if (i >= 0)
      strcpy (lib, GetLibPath ()->Get (i));
    else
      lib [0] = 0;

    size_t sl = strlen (lib);
    for (j = 0 ; j < 2 ; j++)
    {
      if (j)
        strcpy (lib + sl, iPrefix);
      else
        lib [sl] = 0;
      strcat (strcat (lib + sl, iName), iSuffix);

      if (callback.File (lib) == CALLBACK_STOP)
        return;

      if (!iPrefix || iPrefix[0] == '\0')
        break;
    }
  }

}

csLibraryHandle csFindLoadLibrary (const char *iPrefix, const char *iName,
  const char *iSuffix)
{
  callbackLoadLibrary loader;

  csFindLoadLibraryHelper(iPrefix, iName, iSuffix, loader);

  if (!loader._handle)
  {
    if (loader._file_found)
    {
      csPrintLibraryError (iName);
    }
    else
    {
      callbackPrint printer;
      fprintf (stderr, "WARNING: %s, file not found, tried:\n", iName);
      csFindLoadLibraryHelper (iPrefix, iName, iSuffix, printer);
    }
  }

  return loader._handle;
}
