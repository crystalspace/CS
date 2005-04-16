/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein
    DirectDetection.cpp: implementation of the DirectDetection class.

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
#include "csplugincommon/directx/guids.h"

#include "csutil/csunicode.h"
#include "csutil/csstring.h"
#include "csutil/util.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include <stdlib.h>
#include <windows.h>

#include "csutil/win32/wintools.h"

//
//To compile this canvas on a plain MSVC6 compiler
//without any DirectX SDK installed, this #include below
//is required (and it does not hurt if there is an SDK
//installed anyway). groton
#ifdef CS_COMPILER_MSVC
#include <multimon.h>
#endif

#include <ddraw.h>

#include <d3d.h>
#include <d3dcaps.h>

#include "csplugincommon/directx/directdetection.h"

#ifdef CS_COMPILER_BCC
# define _strdup _fstrdup
#endif
#ifdef __CYGWIN__
#include <string.h>
# define _strdup strdup
#endif

void DirectDetection::ReportResult (int severity, char *str, HRESULT hRes)
{
  csString szMsg;
  //if (FAILED (hRes))
  {
    char* errmsg = cswinGetErrorMessage (hRes);
      // in the length formula above the format specifier lengths were subtracted.
    szMsg.Format ("%s\nLast Error: %s [0x%.8x]", str, errmsg, (int)hRes);
    delete[] errmsg;
  }

  csReport (object_reg, severity, "crystalspace.canvas.ddraw.directdetection", 
    "%s", !szMsg.IsEmpty() ? szMsg.GetData() : str);
}

void DirectDetection::SystemFatalError (char *str, HRESULT hRes)
{
  ReportResult (CS_REPORTER_SEVERITY_ERROR,
    str, hRes);
  exit (1);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DirectDetection::DirectDetection ()
{
  object_reg = 0;
}

DirectDetection::~DirectDetection ()
{
}

/// find the best 2d device
const DirectDetectionDevice* DirectDetection::FindBestDevice (int displayNumber)
{
  // If displayNumber is 0, then we use the primary display; otherwise, if
  // it is greater than 0, then we try using the indicated display.  If the
  // indicated display does not exist, then we use the primary display.
  if (displayNumber != 0)
  {
    csString devName2d("\\\\.\\Display");
    devName2d.Append(displayNumber);
    for (size_t i = 0; i < Devices.Length(); i++)
    {
      const DirectDetectionDevice& cur = Devices[i];
      char const* const s = cur.DeviceName2D;
      if (s != 0 && devName2d.CompareNoCase(s))
        return &cur;
    }
    // Requested display not found; fall through and search for primary.
  }
  
  for (size_t i = 0; i < Devices.Length(); i++)
  {
    const DirectDetectionDevice& cur = Devices[i];
    if (cur.IsPrimary2D) 
      return &cur;
  }
  
  return 0;
}

/// add a 2d device in list
int DirectDetection::AddDevice (const DirectDetectionDevice& dd2d)
{
  Devices.Push (dd2d);
  return 0;
}

/// Enumeration of directdraw devices
static BOOL WINAPI DirectDetectionDDrawEnumCallback (GUID FAR * lpGUID,
  LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext,
  HMONITOR p_notUsed)
{
  LPDIRECTDRAW pDD = 0;
  DDCAPS DriverCaps;
  DDCAPS HELCaps;
  DirectDetectionDevice dd2d;
  DirectDetection *ddetect = (DirectDetection *)lpContext;
  HRESULT hRes;

  if (FAILED (hRes = DirectDrawCreate (lpGUID, &pDD, 0)))
  {
    ddetect->ReportResult (
      CS_REPORTER_SEVERITY_WARNING, 
      "Can't create DirectDraw device",
      hRes);
    return DDENUMRET_OK;
  }

  ZeroMemory (&DriverCaps, sizeof (DDCAPS));
  DriverCaps.dwSize = sizeof (DDCAPS);
  ZeroMemory (&HELCaps, sizeof (DDCAPS));
  HELCaps.dwSize = sizeof (DDCAPS);

  if (FAILED (hRes = pDD->GetCaps (&DriverCaps, &HELCaps)))
  {
    ddetect->ReportResult (
      CS_REPORTER_SEVERITY_WARNING, 
      "Can't get device capabilities for DirectDraw device",
      hRes);
    return DDENUMRET_OK;
  }

  // some informations about device
  wchar_t* buf;

  buf = cswinAnsiToWide (lpDriverName);
  dd2d.DeviceName2D = csStrNew (buf);
  delete[] buf;

  buf = cswinAnsiToWide (lpDriverDescription);
  dd2d.DeviceDescription2D = csStrNew (buf);
  delete[] buf;

  if (lpGUID != 0)
  {
    CopyMemory (&dd2d.Guid2D, lpGUID, sizeof (GUID));
    dd2d.IsPrimary2D = false;
  }
  else
  {
    ZeroMemory (&dd2d.Guid2D, sizeof (GUID));
    dd2d.IsPrimary2D = true;
  }

  // can run in windowed mode
#if (DIRECTDRAW_VERSION < 0x0600)
  if (DriverCaps.dwCaps & DDCAPS_GDI)
#else
  if (DriverCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)
#endif
    dd2d.Windowed = true;

  // add this device
  ddetect->AddDevice (dd2d);

  pDD->Release ();

  return DDENUMRET_OK;
}

bool DirectDetection::CheckDevices ()
{
  return CheckDevices2D ();
}

static BOOL WINAPI OldCallback(GUID FAR *lpGUID, LPSTR pDesc, LPSTR pName,
                                   LPVOID pContext)
{
  return DirectDetectionDDrawEnumCallback (lpGUID, pDesc, pName, pContext,
    0);
}

/// check 2d devices
bool DirectDetection::CheckDevices2D ()
{
  HINSTANCE libraryHandle = LoadLibrary ("ddraw.dll");

  // If ddraw.dll doesn't exist in the search path,
  // then DirectX probably isn't installed, so fail.
  if (!libraryHandle)
    SystemFatalError ("Error: couldn't load ddraw.dll!", 0);

  // Note that you must know which version of the
  // function to retrieve (see the following text).
  // [res] Sigh. the *W version is "not implemented".
  LPDIRECTDRAWENUMERATEEXA lpDDEnumEx;

  lpDDEnumEx = (LPDIRECTDRAWENUMERATEEXA) GetProcAddress (
    libraryHandle, "DirectDrawEnumerateExA");

  // If the function is there, call it to enumerate all display
  // devices attached to the desktop, and any non-display DirectDraw
  // devices.
  if (!lpDDEnumEx)
  {
    HRESULT hRes;
    if (FAILED (hRes = DirectDrawEnumerateA (OldCallback, this)))
    {
      //SystemFatalError ("Error when enumerating DirectDraw devices.", hRes);
      ReportResult (CS_REPORTER_SEVERITY_WARNING, 
	"Error when enumerating DirectDraw devices.",
	hRes);
    }
  } 
  else
  {
    // enumerate DDraw device
    HRESULT hRes;
    if (FAILED (hRes = lpDDEnumEx (
      DirectDetectionDDrawEnumCallback, this,
      DDENUM_DETACHEDSECONDARYDEVICES | DDENUM_ATTACHEDSECONDARYDEVICES |
      DDENUM_NONDISPLAYDEVICES)))
    {
      ReportResult (CS_REPORTER_SEVERITY_WARNING, 
	"Error when enumerating DirectDraw devices.",
	hRes);
    }
  }
  //Free the library.
  FreeLibrary (libraryHandle);

  if (Devices.Length() == 0)
  {
    ReportResult (CS_REPORTER_SEVERITY_WARNING, 
      "No 2D devices found.",
      ERROR_SUCCESS);
  }

  return true;
}

/// have 2d devices into list ?
bool DirectDetection::Have2DDevice ()
{
  for (size_t i = 0; i < Devices.Length(); i++)
  {
    if (Devices[i].IsPrimary2D) return true;
  }

  return false;
}
