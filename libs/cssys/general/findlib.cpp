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

#include "cssysdef.h"
#include "cssys/csshlib.h"
#include "cssys/system.h"
#include "csutil/csstrvec.h"
#include "csutil/util.h"

static bool verbose_errors = true;

void csSetLoadLibraryVerbose(bool b) { verbose_errors = b; }
bool csGetLoadLibraryVerbose() { return verbose_errors; }


CS_IMPLEMENT_STATIC_VAR (GetLibPath, csStrVector, (4, 4))
bool findlib_search_nodir = true;

void csAddLibraryPath (char const* iPath)
{
  GetLibPath ()->Push (csStrNew (iPath));
}

const int CALLBACK_STOP = 0;
const int CALLBACK_CONTINUE = 1;

class iLoadLibCallback
{
public:
  virtual int File (char const* file, bool search_nodir) = 0;
};

class csLoadLibLoader : public iLoadLibCallback
{
public:
  csLoadLibLoader ()
  {
    handle = 0;
    file_found = false;
  }

  virtual int File (char const* file, bool search_nodir)
  {
    int status;
    handle = csLoadLibrary (file);
    if (!handle)
    {
      if (search_nodir)
      {
	// There is no way to distinguish between a failed load
	// because the file does not exist or because loading
	// an existing file triggers errors (undefined symbols).
	// The only course of action is to keep trying.
	// file_found is not set to true because we don't know
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
	  file_found = true;
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
      // This is only for consistency purpose since file_found is only
      // supposed to be used for error checking, not when loading
      // succeeds.
      file_found = true;
      status = CALLBACK_STOP;
    }
    return status;
  }

  // The dynamic library handle or 0 if failed to load
  csLibraryHandle handle;
  // Set to true if we know for sure that a file exists but cannot be loaded
  // When implicitly searching in LD_LIBRARY_PATH or equivalent, we cannot
  // figure out if the file exists or not, therefore file_found is false.
  bool file_found;
};

class csLoadLibPrintErr : public iLoadLibCallback
{
public:
  virtual int File (char const* file, bool search_nodir)
  {
    if (search_nodir)
    {
      // The only way to figure out why the load failed when searching
      // in an implicit list of directories (LD_LIBRARY_PATH or
      // equivalent) is to try again and display the resulting error.
      // It may be a simple "file not found" but in the case where it is
      // "failed to resolve symbol XXX" it is a precious bit of
      // information that we need to display.
      csLoadLibrary (file);
      csPrintLibraryError (file);
    }
    else
    {
      fprintf (stderr, "%s: File not found\n", file);
    }
    return CALLBACK_CONTINUE;
  }
};

static void csFindLoadLibraryHelper (char const* iPrefix, char const* iName,
  char const* iSuffix, iLoadLibCallback& callback)
{
  for (int i = findlib_search_nodir?-1:0, n = GetLibPath()->Length(); i<n; i++)
  {
    char lib [CS_MAXPATHLEN + 1];

    // For i == -1 try without any prefix at all
    if (i >= 0)
      strcpy (lib, GetLibPath ()->Get (i));
    else
      lib [0] = 0;

    size_t const sl = strlen (lib);
    for (int j = 0 ; j < 2 ; j++)
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

csLibraryHandle csFindLoadLibrary (char const* iPrefix, char const* iName,
  char const* iSuffix)
{
  csLoadLibLoader loader;
  csFindLoadLibraryHelper(iPrefix, iName, iSuffix, loader);

  if (!loader.handle)
  {
    if (!csGetLoadLibraryVerbose())
    {
      fprintf(stderr,
        "Warning: Failed to load `%s'; use -verbose for details.\n", iName);
    }
    else
    {
      fprintf(stderr, "Warning: Failed to load `%s'; reason:\n", iName);
      if (loader.file_found)
      {
        csPrintLibraryError (iName);
      }
      else
      {
        csLoadLibPrintErr printer;
        csFindLoadLibraryHelper (iPrefix, iName, iSuffix, printer);
      }
    }
  }

  return loader.handle;
}
