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

#include <windows.h>
#ifdef __CYGWIN__
#include <sys/cygwin.h>
#endif
#include "cachedll.h"

typedef DWORD (STDAPICALLTYPE* PFNGETLONGPATHNAMEA) (LPCSTR lpszShortPath, 
						     LPSTR lpszLongPath,
						     DWORD cchBuffer);

static DWORD STDAPICALLTYPE MyGetLPN (LPCSTR lpszShortPath, LPSTR lpszLongPath, 
				      DWORD cchBuffer)
{
  // @@@ Deal with UNC paths?

  // Because the we know the parameters with which this function will be 
  // called, some safety checks can be omitted.
  
  // In case ShortPath == LongPath
  CS_ALLOC_STACK_ARRAY (char, nshort, strlen (lpszShortPath) + 1);
  strcpy (nshort, lpszShortPath);
  lpszShortPath = nshort;

  const char* nextpos = lpszShortPath;
  size_t bufRemain = cchBuffer;
  *lpszLongPath = 0;

#define BUFCAT(s)				  \
  {						  \
    size_t len = strlen (s);			  \
    if (bufRemain > 0)				  \
    {						  \
      strncat (lpszLongPath, s, bufRemain);	  \
      bufRemain -= len;				  \
    }						  \
  }


  while (nextpos != 0)
  {
    char buf[MAX_PATH];
    const char* pos = nextpos;
    char* bs = (char*)strchr (pos, '\\');
    if (bs)
    {
      memcpy (buf, pos, (bs - pos));
      buf[bs - pos] = 0;
    }
    else
    {
      strcpy (buf, pos);
    }
    if (buf[1] == ':')
    {
      BUFCAT (buf);
    }
    else
    {
      BUFCAT ("\\");
      char* bufEnd = (char*)strchr (lpszLongPath, 0);
      memcpy (bufEnd, buf, bufRemain - 1);
      bufEnd[bufRemain - 1] = 0;

      WIN32_FIND_DATA fd;
      HANDLE hFind = FindFirstFile (lpszLongPath, &fd);
      if (hFind != INVALID_HANDLE_VALUE)
      {
	*bufEnd = 0;
	BUFCAT (fd.cFileName);
	FindClose (hFind);
      }
      else
      {
	return 0;
      }
    }
    nextpos = bs ? bs + 1 : 0;
  }
  return (cchBuffer - bufRemain);
#undef BUFCAT
}

// Can't put this inside the function because Cygwin would segfault
// on the destructor for some unknown reason.
static cswinCacheDLL hKernel32 ("kernel32.dll");

char* csExpandPath (const char* path)
{
  if (path == 0 || *path == '\0')
    return 0;

#ifdef __CYGWIN__
  char winpath[MAX_PATH];
  if (cygwin_conv_to_win32_path(path, winpath) == 0)
    path = winpath;
#endif

  char fullName[MAX_PATH];
  GetFullPathName (path, sizeof(fullName), fullName, 0);

  DWORD result = 0;
  PFNGETLONGPATHNAMEA GetLongPathNameFunc = 0;
  // unfortunately, GetLongPathName() is only supported on Win98+/W2k+
  if (hKernel32 != 0)
  {
    GetLongPathNameFunc = 
      (PFNGETLONGPATHNAMEA)GetProcAddress (hKernel32, "GetLongPathNameA");
    if (GetLongPathNameFunc == 0)
    {
      GetLongPathNameFunc = MyGetLPN;
    }
    result = GetLongPathNameFunc (fullName, fullName, sizeof (fullName));
  }
  if (result == 0) 
  {
    return 0;
  }

  return (csStrNew (fullName));
}

bool csPathsIdentical (const char* path1, const char* path2)
{
  return (strcasecmp (path1, path2) == 0);
}

csString csGetAppPath (const char*)
{
  char appPath[MAX_PATH];
  GetModuleFileName (0, appPath, MAX_PATH - 1);
  appPath[MAX_PATH - 1] = '\0';
  return appPath;
}
