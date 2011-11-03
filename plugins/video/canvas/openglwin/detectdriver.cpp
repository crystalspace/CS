/*
    Copyright (C) 2004 by Frank Richter

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
#include "csutil/csstring.h"
#include "csutil/util.h"
#include "csutil/stringquote.h"
#include "csutil/sysfunc.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>

#include "detectdriver.h"

#include "csutil/win32/wintools.h"

csDetectDriver::csDetectDriver()
{
}

csDetectDriver::~csDetectDriver()
{
}

void csDetectDriver::DetermineDriver (const char* monitorName)
{
  csString screenDriverName;

  // From the monitor, try to retrieve a driver name
  DEVMODEA dm;
  memset (&dm, 0, sizeof (dm));
  dm.dmSize = sizeof (dm);
  if (EnumDisplaySettingsA (monitorName, ENUM_CURRENT_SETTINGS, &dm))
  {
    screenDriverName.Replace ((char*)dm.dmDeviceName);
  }
  if (verbose)
    csPrintf ("csDetectDriver: driver name is %s\n",
	      CS::Quote::Single (screenDriverName.GetData ()));

  csString glDllName;
  if (!screenDriverName.IsEmpty())
  {
    // Try to read the OpenGL DLL name from the registry
    csString registryKey;
    registryKey.Format ("Software\\Microsoft\\%s\\CurrentVersion\\"
      "OpenGLdrivers\\%s", cswinIsWinNT() ? "Windows NT" : "Windows",
      screenDriverName.GetData ());

    HKEY key;
    if (RegOpenKeyExA (HKEY_LOCAL_MACHINE, registryKey.GetData(), 0,
      KEY_READ, &key) == ERROR_SUCCESS)
    {
      char dllName[MAX_PATH];
      DWORD dataSize = sizeof (dllName);
      DWORD type;
      if ((RegQueryValueExA (key, "Dll", 0, &type, (LPBYTE)dllName, 
	&dataSize) == ERROR_SUCCESS) && (type == REG_SZ))
      {
	glDllName.Replace ((char*)dllName, dataSize);
      }
      RegCloseKey (key);
    }
    else
    {
      // Another try: maybe the driver name is in a key under OpenGLdrivers
      registryKey.Format ("Software\\Microsoft\\%s\\CurrentVersion\\"
        "OpenGLdrivers", cswinIsWinNT() ? "Windows NT" : "Windows");
      if (RegOpenKeyExA (HKEY_LOCAL_MACHINE, registryKey.GetData(), 0,
        KEY_READ, &key) == ERROR_SUCCESS)
      {
        char dllName[MAX_PATH];
        DWORD dataSize = sizeof (dllName);
        DWORD type;
        if ((RegQueryValueExA (key, screenDriverName.GetData (), 0, &type, 
          (LPBYTE)dllName, &dataSize) == ERROR_SUCCESS) && (type == REG_SZ))
        {
          glDllName.Replace ((char*)dllName, dataSize);
        }
        RegCloseKey (key);
      }
    }
  }
  if (glDllName.IsEmpty() && !screenDriverName.IsEmpty())
  {
    if (verbose)
      csPrintf ("csDetectDriver: maybe DLL %s exists\n", 
      CS::Quote::Single (screenDriverName.GetData()));
    HMODULE dllHandle = LoadLibraryExA (screenDriverName, 0, 
      LOAD_LIBRARY_AS_DATAFILE);
    if (dllHandle != 0)
    {
      FreeLibrary (dllHandle);
      glDllName = screenDriverName;
    }
  }
  if (glDllName.IsEmpty())
  {
    if (verbose)
      csPrintf ("csDetectDriver: trying to find *some* driver\n");
    /*
      Could not determine display driver name. Just fetch the first in the
      registry
     */
    csString registryKey;
    registryKey.Format ("Software\\Microsoft\\%s\\CurrentVersion\\"
      "OpenGLdrivers", cswinIsWinNT() ? "Windows NT" : "Windows");

    HKEY key;
    if (RegOpenKeyExA (HKEY_LOCAL_MACHINE, registryKey.GetData(), 0,
      KEY_READ, &key) == ERROR_SUCCESS)
    {
      char subkeyName[MAX_PATH];
      DWORD subkeySize = sizeof (subkeyName);
      FILETIME ft;
      if (RegEnumKeyExA (key, 0, subkeyName, &subkeySize, 0, 0, 0, &ft) == 
	ERROR_SUCCESS)
      {
	HKEY driverKey;
	if (RegOpenKeyExA (key, subkeyName, 0,
	  KEY_READ, &driverKey) == ERROR_SUCCESS)
	{
	  char dllName[MAX_PATH];
	  DWORD dataSize = sizeof (dllName);
	  DWORD type;
	  if ((RegQueryValueExA (driverKey, "Dll", 0, &type, (LPBYTE)dllName, 
	    &dataSize) == ERROR_SUCCESS) && (type == REG_SZ))
	  {
	    glDllName.Replace ((char*)dllName);
	  }
	  RegCloseKey (driverKey);
	}
      }
      else if (RegEnumValueA (key, 0, subkeyName, &subkeySize, 0, 0, 0, 0) 
        == ERROR_SUCCESS)
      {
        char dllName[MAX_PATH];
        DWORD dataSize = sizeof (dllName);
        DWORD type;
        if ((RegQueryValueExA (key, subkeyName, 0, &type, (LPBYTE)dllName, 
          &dataSize) == ERROR_SUCCESS) && (type == REG_SZ))
        {
          glDllName.Replace ((char*)dllName);
        }
      }
      RegCloseKey (key);
    }
  }
  if (!glDllName.IsEmpty())
  {
    DriverDLL = glDllName;
    if (verbose)
      csPrintf ("%s: found DLL %s\n", CS_FUNCTION_NAME,
		CS::Quote::Single (DriverDLL.GetData()));
  }
}

void csDetectDriver::ScanForDriver ()
{
  csString glDllName;
  HANDLE ms; 
  MODULEENTRY32 me;
 
  // Initialize the modules list
  ms = CreateToolhelp32Snapshot (TH32CS_SNAPMODULE, GetCurrentProcessId());
  if (ms != INVALID_HANDLE_VALUE)
  {
    memset (&me, 0, sizeof (me));
    me.dwSize = sizeof (me); 

    // Iterate over modules
    if (Module32First (ms, &me)) 
    { 
      do 
      { // Check if the module exports an ICD API function
	if (GetProcAddress (me.hModule, "DrvGetProcAddress"))
	{
	  // ICD module found, remember its filename
	  glDllName.Replace(me.szModule);
	  break;
	}
      } while (Module32Next (ms, &me)); 
    }

    CloseHandle (ms);

    if (!glDllName.IsEmpty())
    {
      DriverDLL = glDllName;
      if (verbose)
        csPrintf ("%s: found DLL %s\n", CS_FUNCTION_NAME, 
          CS::Quote::Single (DriverDLL.GetData()));
    }
  }
}

void csDetectDriver::DetermineDriverVersion()
{
  DWORD dummy;
  /* The const_casts are needed as w32api from MinGW doesn't declare some 
     string parameters as 'const', whereas e.g. Windows SDK 6.0 does. 
     (w32api is probably not the only offender, older Platform SDKs aren't
     always const-friendly, too.) */
  DWORD verInfoSize = GetFileVersionInfoSizeA (
    const_cast<char*> (DriverDLL.GetData()), &dummy);
  if (verInfoSize == 0)
  {
    // Try appending ".dll"
    csString newName (DriverDLL);
    newName.Append (".dll");
    verInfoSize = GetFileVersionInfoSizeA (
      const_cast<char*> (newName.GetData()), &dummy);
  }
  if (verInfoSize != 0)
  {
    uint8* buffer = new uint8[verInfoSize];
    if (GetFileVersionInfoA (
      const_cast<char*> (DriverDLL.GetData()), 0, verInfoSize, buffer))
    {
      void* data;
      UINT dataLen;
      if (VerQueryValueA (buffer, const_cast<char*> ("\\"), &data, &dataLen) && 
	(dataLen == sizeof (VS_FIXEDFILEINFO)))
      {
	VS_FIXEDFILEINFO& ffi = *((VS_FIXEDFILEINFO*)data);
	csString verString;
	verString.Format ("%u.%u.%u.%u", (uint)(ffi.dwFileVersionMS >> 16), 
	  (uint)(ffi.dwFileVersionMS & 0xffff), 
	  (uint)(ffi.dwFileVersionLS >> 16), 
	  (uint)(ffi.dwFileVersionLS & 0xffff));
	DriverVersion = verString.GetData();
      }
    }
    delete[] buffer;
  }
}

void csDetectDriver::DoDetection (HWND window, HDC dc)
{
  DriverDLL.Empty();
  DriverVersion.Empty();

  int pixfmt = GetPixelFormat (dc);
  bool isAccelerated = false;
  PIXELFORMATDESCRIPTOR pfd;
  if (DescribePixelFormat (dc, pixfmt, sizeof (pfd), &pfd) != 0)
  {
    isAccelerated = !(pfd.dwFlags & PFD_GENERIC_FORMAT) ||
      (pfd.dwFlags & PFD_GENERIC_ACCELERATED);
  }

  if (isAccelerated)
  {
    if (cswinIsWinNT())
    {
      /* The approach DetermineDriverVersion() does at least not work on 
         Vista. It's not entirely robust anyway. Instead, scan the loaded
         DLLs for the one that's probably the OpenGL ICD.
       */
      ScanForDriver();
    }
    else
    {
      /*
        Don't even bother if no acceleration (and thus likely no driver)
        is present
       */
      csString screenDevice;

      // Determine the monitor we're on
      MONITORINFOEXA mi;

      HMONITOR monitor = MonitorFromWindow (window,
	MONITOR_DEFAULTTONEAREST);

      if (monitor != 0)
      {
	memset (&mi, 0, sizeof (mi));
	mi.cbSize = sizeof (mi);
	if (GetMonitorInfoA (monitor, &mi))
	{
	  screenDevice.Replace (mi.szDevice);
	}
      }

      if (verbose)
        csPrintf ("csDetectDriver: monitor name is %s\n",
		  CS::Quote::Single (screenDevice.GetData ()));
      
      // Try to determine the driver
      DetermineDriver (screenDevice.GetData ());
    }

    if (!DriverDLL.IsEmpty())
    {
      DetermineDriverVersion();
    }
  }
}

