/*
    Copyright (C) 2002,2003 by Frank Richter

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

#ifndef __CS_SYS_WIN32_SHELLSTUFF_H__
#define __CS_SYS_WIN32_SHELLSTUFF_H__

#include <shlobj.h>

// This file contains some newer SHELL32 stuff, for example not found
// in MinGW Win32 headers.

typedef HRESULT (STDAPICALLTYPE* SHGETFOLDERPATHAPROC)(HWND hwndOwner, 
					      int nFolder, 
					      HANDLE hToken, 
					      DWORD dwFlags, 
					      LPCSTR pszPath);
					      
typedef HRESULT (STDAPICALLTYPE* SHGETFOLDERPATHWPROC) (HWND hwndOwner,
                                                int nFolder,
						HANDLE hToken,
						DWORD dwFlags,
						LPCSTR pszPath);


#ifndef CSIDL_APPDATA 
#define CSIDL_APPDATA			0x001a
#endif
#ifndef CSIDL_PROGRAM_FILES
#define CSIDL_PROGRAM_FILES             0x0026
#endif
#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT		0
#endif

static inline bool
GetShellFolderPath (int CSIDL, char* path)
{
  HINSTANCE hinstDll;
  bool result = false;

  // ShFolder.dll can 'emulate' special folders on older Windowses.
  hinstDll = LoadLibrary ("shfolder.dll");
	
  if(hinstDll)
  {
    SHGETFOLDERPATHAPROC SHGetFolderPathA;
    // Somehow or another, an app (walktest) is requesting to load
    // SHGetFolderPathW from shell32.dll.
    // That function does not exist in shell32.dll for versions < 5.0.
    // It does exist in shfolder.dll for versions < 5.0
    // versions < 5.0 include NT4, Win98 and Win98SE
    SHGETFOLDERPATHWPROC SHGetFolderPathW;

    SHGetFolderPathA = 
      (SHGETFOLDERPATHAPROC) GetProcAddress (hinstDll, "SHGetFolderPathA");
      
    if (SHGetFolderPathA)
    {
      result = (SHGetFolderPathA (0, CSIDL, 0, SHGFP_TYPE_CURRENT, path) == S_OK);
    }
      
    SHGetFolderPathW =
            (SHGETFOLDERPATHWPROC) GetProcAddress (hinstDll, "SHGetFolderPathW");       

    if (SHGetFolderPathW == 0)
    {
        // If SHGetFolderPathW is not found in shfolder.dll 
    }
    else
    {
        result = (SHGetFolderPathW (0, CSIDL, 0, SHGFP_TYPE_CURRENT, path) == S_OK);
     }


    FreeLibrary(hinstDll);
  }
  else
  {
    // no shfolder? Try normal shell32 instead
    LPMALLOC MAlloc;
    LPITEMIDLIST pidl;

    if (SUCCEEDED(SHGetMalloc (&MAlloc)))
    {
      if (SUCCEEDED(SHGetSpecialFolderLocation (0, CSIDL, &pidl)))
      {
	result = SHGetPathFromIDList (pidl, path);
	MAlloc->Free (pidl);
      }
      MAlloc->Release ();
    }
  }
  return result;
}

#endif // __CS_SYS_WIN32_SHELLSTUFF_H__
