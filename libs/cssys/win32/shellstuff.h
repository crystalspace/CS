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

// This file contains some newer SHELL32 stuff, for example not found
// in MinGW Win32 headers.

// from shfolder.h...
typedef struct _DLLVERSIONINFO
{
    DWORD cbSize;
    DWORD dwMajorVersion;                   // Major version
    DWORD dwMinorVersion;                   // Minor version
    DWORD dwBuildNumber;                    // Build number
    DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
} DLLVERSIONINFO;

typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

#ifndef CSIDL_PROGRAM_FILES
#define CSIDL_PROGRAM_FILES             0x0026
#endif

static inline bool 
MinShellDllVersion(DWORD vMajor, DWORD vMinor)
{
  HINSTANCE hinstDll;
  bool result = false;

  hinstDll = LoadLibrary ("shell32.dll");
	
  if(hinstDll)
  {
    DLLGETVERSIONPROC pDllGetVersion;

    pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress (hinstDll, "DllGetVersion");

    if (pDllGetVersion)
    {
      DLLVERSIONINFO dvi;
      HRESULT hr;

      memset (&dvi, 0, sizeof (dvi));
      dvi.cbSize = sizeof (dvi);

      hr = (*pDllGetVersion)(&dvi);

      if (SUCCEEDED (hr))
      {
	result = ((dvi.dwMajorVersion > vMajor) ||
	  ((dvi.dwMajorVersion == vMajor) && 
	   (dvi.dwMinorVersion >= vMinor)));
      }
    }
        
    FreeLibrary(hinstDll);
  }
  return result;
}

#endif // __CS_SYS_WIN32_SHELLSTUFF_H__
