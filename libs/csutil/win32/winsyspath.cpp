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
#include "csutil/sysfunc.h"
#include "csutil/syspath.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <basetyps.h>

#ifdef __CYGWIN__
#include <sys/cygwin.h>
#endif

char* csPathsUtilities::ExpandPath (const char* path)
{
  if (path == 0 || *path == '\0')
    return 0;

#ifdef __CYGWIN__
  wchar_t winpath[MAX_PATH];
  csString winpath_utf8;
  if (cygwin_conv_path (CCP_POSIX_TO_WIN_W, path,
                        winpath, sizeof (winpath)) == 0)
  {
    winpath_utf8 = winpath;
    path = winpath_utf8.GetData();
  }
#endif

  size_t pathLen (strlen (path));
  size_t pathWSize (pathLen + 1);
  CS_ALLOC_STACK_ARRAY(wchar_t, pathW, pathWSize);
  csUnicodeTransform::UTF8toWC (pathW, pathWSize,
                                (utf8_char*)path, pathLen);

  wchar_t fullName[MAX_PATH];
  GetFullPathNameW (pathW, sizeof(fullName)/sizeof(fullName[0]), fullName, 0);

  DWORD result = 0;
  result = GetLongPathNameW(fullName, fullName, sizeof (fullName)/sizeof(fullName[0]));
  if (result == 0) 
  {
    return 0;
  }

  return (csStrNew (fullName));
}

bool csPathsUtilities::PathsIdentical (const char* path1, const char* path2)
{
  /* See: http://blogs.msdn.com/b/michkap/archive/2005/10/17/481600.aspx
     for an explanation of proper filename comparison */
  size_t path1len (strlen (path1));
  size_t path1Wlen (path1len+1);
  CS_ALLOC_STACK_ARRAY(wchar_t, path1W, path1Wlen);
  csUnicodeTransform::UTF8toWC (path1W, path1Wlen,
                                (utf8_char*)path1, path1len);
  size_t path2len (strlen (path2));
  size_t path2Wlen (path2len+1);
  CS_ALLOC_STACK_ARRAY(wchar_t, path2W, path2Wlen);
  csUnicodeTransform::UTF8toWC (path2W, path2Wlen,
                                (utf8_char*)path2, path2len);

  CharUpperBuffW (path1W, DWORD (wcslen (path1W)));
  CharUpperBuffW (path2W, DWORD (wcslen (path2W)));
  return (wcscmp (path1W, path2W) == 0);
}

csString csInstallationPathsHelper::GetAppPath (const char*)
{
  wchar_t appPath[MAX_PATH];
  GetModuleFileNameW (0, appPath, MAX_PATH - 1);
  appPath[MAX_PATH - 1] = '\0';
  return appPath;
}

csString csInstallationPathsHelper::GetAppFilename (const char* basename)
{
  csString filename;
  return filename << basename << ".exe";
}


namespace CS
{
  namespace Platform
  {

    csString GetTempDirectory ()
    {
      wchar_t tmpDir[MAX_PATH*2];
      ::GetTempPathW (MAX_PATH*2-1, tmpDir);
      tmpDir[MAX_PATH*2-1] = '\0';
    
      return tmpDir;
    }
    
    csString GetTempFilename (const char* path)
    {
      wchar_t filename[MAX_PATH];
    
      size_t pathLen (strlen (path));
      size_t pathWSize (pathLen + 1);
      CS_ALLOC_STACK_ARRAY(wchar_t, pathW, pathWSize);
      csUnicodeTransform::UTF8toWC (pathW, pathWSize,
                                    (utf8_char*)path, pathLen);
      if (::GetTempFileNameW (path ? pathW : L".", L"CS", 0, filename) > 0)
      {
	filename[MAX_PATH-1] = '\0';
	csString result (filename);
    
	size_t pos = result.FindLast (CS_PATH_SEPARATOR);
	if (pos != (size_t)-1 && pos < result.Length () - 2)
	{
	  result = result.Slice (pos+1);
	}
    
    
	return result;
      }
    
      // Fallback
      char filename_narrow[MAX_PATH];
      cs_snprintf (filename_narrow, MAX_PATH, "cs%x.tmp", _getpid ());
    
      return filename;
    }

  }
}
