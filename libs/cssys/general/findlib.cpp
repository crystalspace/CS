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
  virtual int File (const char *file, bool search_nodir) = 0;
};

class callbackLoadLibrary : public iCallback
{
public:
  callbackLoadLibrary ()
  {
    _handle = 0;
    _file_found = false;
  }

  virtual int File (const char *file, bool search_nodir)
  {
    int status;
    _handle = csLoadLibrary (file);
    if (!_handle)
    {
      if (search_nodir)
      {
	// There is no way to distinguish between a failed load
	// because the file does not exist or because loading
	// an existing file triggers errors (undefined symbols).
	// The only course of action is to keep trying.
	// _file_found is not set to true because we don't know
	// for sure.
	status = CALLBACK_CONTINUE;
      }
      else
      {
	// If load failed because the file contains errors (and not
	// because it does not exist), then we must stop and display
	// the error. If we kept trying to load other files, we would
	// lose the error message and become unable to figure out why
	// a given file is flawed.
	struct stat st;
	if (stat (file, &st) == 0)
	{
	  _file_found = true;
	  status = CALLBACK_STOP;
	}
	else
	{
	  status = CALLBACK_CONTINUE;
	}
      }
    }
    else
    {
      // This is only for consistency purpose since _file_found is only
      // supposed to be used for error checking, not when loading
      // succeeds.
      _file_found = true;
      status = CALLBACK_STOP;
    }
    return status;
  }

  // The dynamic library handle or 0 if failed to load
  csLibraryHandle _handle;
  // Set to true if we know for sure that a file exists but cannot be loaded
  // When implicitly searching in LD_LIBRARY_PATH or equivalent, we cannot
  // figure out if the file exists or not, therefore _file_found is false.
  bool _file_found;
};

class callbackPrint : public iCallback
{
public:
  virtual int File (const char *file, bool search_nodir)
  {
    if (search_nodir)
    {
      // The only way to figure out why the load failed when searching
      // in an implicit list of directories (LD_LIBRARY_PATH or
      // equivalent) is to try again and display the resulting error.
      // It may be a simple "file not found" but in the case where it is
      // "failed to resolve symbol XXX" it is a precious bit of
      // information that we need to display.
      fprintf (stderr, "\tloading '%s' from LD_LIBRARY_PATH or equivalent gives:\n", file);
      csLoadLibrary (file);
      csPrintLibraryError (file);
    }
    else
    {
      fprintf (stderr, "\t%s: file not found\n", file);
    }
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

      if (callback.File (lib, (i == -1 || sl == 0)) == CALLBACK_STOP)
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
      fprintf (stderr, "WARNING: %s, failed to load, because:\n", iName);
      csFindLoadLibraryHelper (iPrefix, iName, iSuffix, printer);
    }
  }

  return loader._handle;
}


