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
  char *szMsg = 0;
  //if (FAILED (hRes))
  {
    char* errmsg = cswinGetErrorMessage (hRes);
    const char* szStdMessage = "%s\nLast Error: %s [0x%.8x]";
    szMsg = new char [strlen (szStdMessage) + strlen (errmsg) - 2 + strlen (str) - 2 + 8 - 4 + 1];
      // in the length formula above the format specifier lengths were subtracted.
    scsPrintf (szMsg, szStdMessage, str, errmsg, (int)hRes);
    delete[] errmsg;
  }

  csRef<iReporter> rep = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (rep)
    rep->Report (severity, "crystalspace.canvas.ddraw.directdetection", "%s", 
      szMsg ? szMsg : str);
  else
  {
    csPrintf ("%s", szMsg ? szMsg : str);
    csPrintf ("\n");
  }

  delete[] szMsg;
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
  Devices = 0;
  object_reg = 0;
}

DirectDetection::~DirectDetection ()
{
  if (Devices)
  {
    DirectDetectionDevice *cur = Devices;
    while (cur)
    {
      DirectDetectionDevice *next = cur->next;
      delete[] cur->DeviceName2D;
      delete[] cur->DeviceDescription2D;
      delete cur;
      cur = next;
    }
  }

  Devices = 0;
}

/// find the best 2d device
DirectDetectionDevice * DirectDetection::findBestDevice2D(int displayNumber)
{
  DirectDetectionDevice * cur;

  // If displayNumber is 0, then we use the primary display; otherwise, if
  // it is greater than 0, then we try using the indicated display.  If the
  // indicated display does not exist, then we use the primary display.
  if (displayNumber != 0)
  {
    csString devName2d("\\\\.\\Display");
    devName2d.Append(displayNumber);
    for(cur = Devices; cur != 0; cur = cur->next)
    {
      char const* const s = cur->DeviceName2D;
      if (s != 0 && devName2d.CompareNoCase(s))
        return cur;
    }
    // Requested display not found; fall through and search for primary.
  }
  
  for (cur = Devices; cur != 0; cur = cur->next)
  {
    if (cur->Only2D && cur->IsPrimary2D) 
      return cur;
  }
  
  return 0;
}

/// find the best 3d device
DirectDetectionDevice *DirectDetection::findBestDevice3D (bool fscreen)
{
  DirectDetectionDevice *ret = 0;
  DirectDetectionDevice *cur = Devices;
  int poids = 0;

  while (cur != 0)
  {
    // This device have 3d device
    if (!cur->Only2D && cur->Can3D)
    {
      int curpoids = 0;

      // calculation of weight
      if (cur->Hardware) curpoids += 200;
      if (cur->Texture) curpoids += 50;
      if (cur->HighColor) curpoids += 25;
      if (cur->VideoMemoryTexture) curpoids += 15;
      if (cur->Mipmap) curpoids += 35;
      if (cur->Perspective) curpoids += 25;
      if (cur->ZBuffer) curpoids += 50;
      if (cur->AlphaBlend && cur->AlphaBlendType == 1) curpoids += 50;
      if (cur->AlphaBlend && cur->AlphaBlendType == 2) curpoids += 25;

      // is better and support windowed mode if display is not fullscreen ?
      if (curpoids > poids && (!fscreen ? cur->Windowed : true))
      {
        ret = cur;
        poids = curpoids;
      }
    }
    cur = cur->next;
  }

  return ret;
}

/// add a 2d device in list
int DirectDetection::addDevice (DirectDetection2D *dd2d)
{
  DirectDetectionDevice *ddd = new DirectDetectionDevice ();

  memcpy ((DirectDetection2D *)ddd, dd2d, sizeof (DirectDetection2D));
  ddd->Only2D = true;

  ddd->next = Devices;
  Devices = ddd;

  return 0;
}

/// add a 3d device in list
int DirectDetection::addDevice (DirectDetection3D *dd3d)
{
  DirectDetectionDevice * ddd = new DirectDetectionDevice ();

  memcpy ((DirectDetection3D *)ddd, dd3d, sizeof (DirectDetection3D));
  ddd->Only2D = false;

  ddd->next = Devices;
  Devices = ddd;

  return 0;
}

/// Enumeration of direct3d devices
static HRESULT WINAPI DirectDetectionD3DEnumCallback (LPGUID lpGuid,
  LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC lpHWDesc,
  LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext)
{
  struct toy
  {
    DirectDetection * ddetect;
    DirectDetection2D * dd2d;
  } *toy = (struct toy *)lpContext;

  DirectDetection *ddetect = toy->ddetect;
  DirectDetection3D dd3d (toy->dd2d);

  // don't accept software devices.
  // eventually this will be an option.
  if (!lpHWDesc->dcmColorModel) return D3DENUMRET_OK;

  // Record the D3D driver's information
  wchar_t* buf;

  buf = cswinAnsiToWide (lpDeviceName);
  dd3d.DeviceName3D = csStrNew (buf);
  delete[] buf;

  buf = cswinAnsiToWide (lpDeviceDescription);
  dd3d.DeviceDescription3D = csStrNew (buf);
  delete[] buf;

  if (lpGuid != 0)
  {
    CopyMemory (&dd3d.Guid3D, lpGuid, sizeof (GUID));
    dd3d.IsPrimary3D = false;
  }
  else
  {
    ZeroMemory (&dd3d.Guid3D, sizeof (GUID));
    dd3d.IsPrimary3D = false;
  }

  // whether this is hardware or not.
  if (lpHWDesc->dcmColorModel)
  {
    dd3d.Hardware = true;
    CopyMemory (&dd3d.Desc3D, lpHWDesc, sizeof (D3DDEVICEDESC));
  }
  else
  {
    dd3d.Hardware = false;
    CopyMemory (&dd3d.Desc3D, lpHELDesc, sizeof (D3DDEVICEDESC));
  }

  // does this driver to texture-mapping?
  dd3d.Perspective =
    (dd3d.Desc3D.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE) ? true : false;

  // z-buffer?
  dd3d.ZBuffer = dd3d.Desc3D.dwDeviceZBufferBitDepth ? true : false;

  // alpha transparency?
  if ((dd3d.Desc3D.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCCOLOR)
   && (dd3d.Desc3D.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_DESTCOLOR))
  {
    dd3d.AlphaBlend = true;
    dd3d.AlphaBlendType = 1;
  }
  else if ((dd3d.Desc3D.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA)
        && (dd3d.Desc3D.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_SRCCOLOR))
  {
    dd3d.AlphaBlend = true;
    dd3d.AlphaBlendType = 2;
  }

  if ((dd3d.Desc3D.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA)
   && (dd3d.Desc3D.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR))
  {
    dd3d.AlphaBlendHalo = true;
  }

  // is hi-color?
  dd3d.HighColor = (dd3d.Desc3D.dwDeviceRenderBitDepth & DDBD_16) ? true : false;

  // can load textures into video-memory?
  dd3d.VideoMemoryTexture = (dd3d.Desc3D.dwDevCaps & D3DDEVCAPS_TEXTUREVIDEOMEMORY) ? true : false;

  // add this device
  ddetect->addDevice (&dd3d);

  return (D3DENUMRET_OK);
}

/// Enumeration of directdraw devices
static BOOL WINAPI DirectDetectionDDrawEnumCallback (GUID FAR * lpGUID,
  LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext,
  HMONITOR p_notUsed)
{
  LPDIRECTDRAW pDD = 0;
  DDCAPS DriverCaps;
  DDCAPS HELCaps;
  DirectDetection2D dd2d;
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

  // can enable a 3d device
  if (DriverCaps.dwCaps & DDCAPS_3D)
    dd2d.Can3D = true;

  // can run in windowed mode
#if (DIRECTDRAW_VERSION < 0x0600)
  if (DriverCaps.dwCaps & DDCAPS_GDI)
#else
  if (DriverCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)
#endif
    dd2d.Windowed = true;

  // can have mipmapped surfaces
  if (DriverCaps.ddsCaps.dwCaps & DDSCAPS_MIPMAP)
    dd2d.Mipmap = true;

  // can have textured surfaces
  if (DriverCaps.ddsCaps.dwCaps & DDSCAPS_TEXTURE)
    dd2d.Texture = true;

  // add this device
  ddetect->addDevice (&dd2d);

  pDD->Release ();

  return DDENUMRET_OK;
}

bool DirectDetection::checkDevices ()
{
  return checkDevices3D ();
}

bool DirectDetection::checkDevices3D ()
{
  // first check 2d devices
  if (checkDevices2D ())
  {
    DirectDetectionDevice *cur = Devices;
    while (cur)
    {
      // for each 2d device which have a 3d device
      if (cur->Only2D && cur->Can3D)
      {
        LPDIRECTDRAW lpDD = 0;
        LPDIRECT3D lpD3D = 0;
        struct toy
        {
          DirectDetection * ddetect;
          DirectDetection2D * dd2d;
        } toy = {this, cur};

        LPGUID pGuid = 0;
        if (!cur->IsPrimary2D) pGuid = &cur->Guid2D;

        HRESULT hRes;
        if (FAILED (hRes = DirectDrawCreate (pGuid, &lpDD, 0)))
	{
	  ReportResult (CS_REPORTER_SEVERITY_WARNING, 
	    "Can't create DirectDraw device",
	    hRes);
	  cur = cur->next;
	  continue;
	}

        lpDD->QueryInterface (IID_IDirect3D, (LPVOID *)&lpD3D);
        if (FAILED (hRes = lpD3D->EnumDevices (DirectDetectionD3DEnumCallback, (LPVOID *)&toy)))
	{
	  ReportResult (CS_REPORTER_SEVERITY_WARNING, 
	    "Error when enumerating Direct3D devices.",
	    hRes);
	  cur = cur->next;
	  continue;
	}

        lpD3D->Release ();
        lpDD->Release ();
      }
      cur = cur->next;
    }
  }
  return true;
}

static BOOL WINAPI OldCallback(GUID FAR *lpGUID, LPSTR pDesc, LPSTR pName,
                                   LPVOID pContext)
{
  return DirectDetectionDDrawEnumCallback (lpGUID, pDesc, pName, pContext,
    0);
}

/// check 2d devices
bool DirectDetection::checkDevices2D ()
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

  if (Devices == 0)
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
  DirectDetectionDevice * cur = Devices;

  while (cur)
  {
    if (cur->Only2D && cur->IsPrimary2D) return true;
    cur = cur->next;
  }

  return false;
}

/// have 3d devices into list ?
bool DirectDetection::Have3DDevice ()
{
  DirectDetectionDevice * cur = Devices;

  while (cur)
  {
    if (!cur->Only2D && cur->Can3D) return true;
    cur = cur->next;
  }

  return false;
}
