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
#include "csutil/sysfunc.h"

#include <windows.h>
#include "detectdriver.h"

#include "csutil/win32/wintools.h"

#ifndef MONITOR_DEFAULTTONEAREST
// "guess" whether multimon types/#defs are present already

#if !defined(__MINGW32__) && !defined(__CYGWIN__)
// MinGW & Cygwin(more exactly, the w32api headers) already have them defined

struct MONITORINFOEXA
{  
  DWORD cbSize; 
  RECT rcMonitor; 
  RECT rcWork; 
  DWORD dwFlags; 
  CHAR szDevice[CCHDEVICENAME];
};
DECLARE_HANDLE(HMONITOR);
#endif

typedef MONITORINFOEXA* LPMONITORINFOEXA; 
#define MONITOR_DEFAULTTONEAREST  2

#endif

#define CS_API_NAME		MultiMon
#define CS_API_FUNCTIONS	"plugins/video/canvas/openglwin/MultiMonAPI.fun"
#define CS_API_DLL		"user32.dll"
#define CS_API_EXPORT

#include "csutil/win32/APIdeclare.inc"
#include "libs/csutil/win32/APIdefine.inc"  // @@@

csDetectDriver::csDetectDriver()
{
  MultiMon::IncRef();

  DriverDLL = 0;
  DriverVersion = 0;
}

csDetectDriver::~csDetectDriver()
{
  delete[] DriverDLL;
  delete[] DriverVersion;

  MultiMon::DecRef();
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
    csPrintf ("csDetectDriver: driver name is '%s'\n", screenDriverName.GetData ());

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
      csPrintf ("csDetectDriver: maybe DLL '%s' exists\n", 
      screenDriverName.GetData());
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
    DriverDLL = csStrNew (glDllName);
    if (verbose)
      csPrintf ("csDetectDriver: found DLL '%s'\n", DriverDLL);
  }
}

void csDetectDriver::DetermineDriverVersion()
{
  DWORD dummy;
  DWORD verInfoSize = GetFileVersionInfoSizeA (DriverDLL, &dummy);
  if (verInfoSize == 0)
  {
    // Try appending ".dll"
    csString newName (DriverDLL);
    newName.Append (".dll");
    verInfoSize = GetFileVersionInfoSizeA (
      CS_CONST_CAST(char*,newName.GetData()), &dummy);
  }
  if (verInfoSize != 0)
  {
    uint8* buffer = new uint8[verInfoSize];
    if (GetFileVersionInfoA (DriverDLL, 0, verInfoSize, buffer))
    {
      void* data;
      UINT dataLen;
      if (VerQueryValueA (buffer, "\\", &data, &dataLen) && 
	(dataLen == sizeof (VS_FIXEDFILEINFO)))
      {
	VS_FIXEDFILEINFO& ffi = *((VS_FIXEDFILEINFO*)data);
	csString verString;
	verString.Format ("%u.%u.%u.%u", (uint)(ffi.dwFileVersionMS >> 16), 
	  (uint)(ffi.dwFileVersionMS & 0xffff), 
	  (uint)(ffi.dwFileVersionLS >> 16), 
	  (uint)(ffi.dwFileVersionLS & 0xffff));
	DriverVersion = csStrNew (verString.GetData());
      }
    }
    delete[] buffer;
  }
}

void csDetectDriver::DoDetection (HWND window, HDC dc)
{
  delete[] DriverDLL; DriverDLL = 0;
  delete[] DriverVersion; DriverVersion = 0;

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
    /*
      Don't even bother if no acceleration (and thus likely no driver)
      is present
     */
    csString screenDevice;

    // Determine the monitor we're on
    if (MultiMon::MultiMonAvailable())
    {
      MONITORINFOEXA mi;

      HMONITOR monitor = MultiMon::MonitorFromWindow (window,
	MONITOR_DEFAULTTONEAREST);

      if (monitor != 0)
      {
	memset (&mi, 0, sizeof (mi));
	mi.cbSize = sizeof (mi);
	if (MultiMon::GetMonitorInfoA (monitor, &mi))
	{
	  screenDevice.Replace (mi.szDevice);
	}
      }
    }

    if (verbose)
      csPrintf ("csDetectDriver: monitor name is '%s'\n", screenDevice.GetData ());
    
    // Try to determine the driver
    DetermineDriver (screenDevice.GetData ());

    if (DriverDLL != 0)
    {
      DetermineDriverVersion();
    }
  }
}

