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

#define INITGUID

#include "cssysdef.h"
#include <stdlib.h>
#include <windowsx.h>

#include <ddraw.h>

#include <d3d.h>
#include <d3dcaps.h>

#include "directdetection.h"

#ifdef COMP_BC
# define _strdup _fstrdup
#endif

void sys_fatalerror (char *str, HRESULT hRes)
{
  LPVOID lpMsgBuf;
  char *szMsg;
  char szStdMessage [] = "\nLast Error: ";
  if (FAILED (hRes))
  {
    DWORD dwResult;
    dwResult = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, hRes, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
  
    if (dwResult != 0)
    {
      szMsg = new char [strlen ((const char*)lpMsgBuf) + strlen (str) + strlen (szStdMessage) + 1];
      strcpy (szMsg, str);
      strcat (szMsg, szStdMessage);
      strcat (szMsg, (const char*)lpMsgBuf);
      LocalFree (lpMsgBuf);
      
      MessageBox (NULL, szMsg, "Fatal Error", MB_OK | MB_TOPMOST);
      delete szMsg;

      exit (1);
    }
  }

  MessageBox (NULL, str, "Fatal Error", MB_OK | MB_TOPMOST);
  exit (1);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DirectDetection::DirectDetection ()
{
  Devices = NULL;
}

DirectDetection::~DirectDetection ()
{
  if (Devices)
  {
    DirectDetectionDevice *cur = Devices;
    while (cur)
    {
      DirectDetectionDevice *next = cur->next;
      delete cur;
      cur = next;
    }
  }

  Devices = NULL;
}

/// find the best 2d device
DirectDetectionDevice * DirectDetection::findBestDevice2D ()
{
  DirectDetectionDevice * cur = Devices;
  
  while (cur != NULL)
  {
    // in fact we just need primary device
    if (cur->Only2D && cur->IsPrimary2D) return cur;
    cur = cur->next;
  }
  
  return NULL;
}

/// find the best 3d device
DirectDetectionDevice *DirectDetection::findBestDevice3D (bool fscreen)
{
  DirectDetectionDevice *ret = NULL;
  DirectDetectionDevice *cur = Devices;
  int poids = 0;
  
  while (cur != NULL)
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
  dd3d.DeviceName3D = _strdup (lpDeviceName);
  dd3d.DeviceDescription3D = _strdup (lpDeviceDescription);
  if (lpGuid != NULL)
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
  LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext)
{
  LPDIRECTDRAW pDD = NULL;
  DDCAPS DriverCaps;
  DDCAPS HELCaps;
  DirectDetection2D dd2d;
  DirectDetection *ddetect = (DirectDetection *)lpContext;
  HRESULT hRes;

  if (FAILED (hRes = DirectDrawCreate (lpGUID, &pDD, NULL)))
    sys_fatalerror ("Can't create DirectDraw device", hRes);
  
  ZeroMemory (&DriverCaps, sizeof (DDCAPS));
  DriverCaps.dwSize = sizeof (DDCAPS);
  ZeroMemory (&HELCaps, sizeof (DDCAPS));
  HELCaps.dwSize = sizeof (DDCAPS);
  
  if (FAILED (hRes = pDD->GetCaps (&DriverCaps, &HELCaps)))
    sys_fatalerror ("Can't get device capabilities for DirectDraw device", hRes);
  
  // some informations about device
  dd2d.DeviceName2D = _strdup (lpDriverName);
  dd2d.DeviceDescription2D = _strdup (lpDriverDescription);
  if (lpGUID != NULL)
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
        LPDIRECTDRAW lpDD = NULL;
        LPDIRECT3D lpD3D = NULL;
        struct toy
        {
          DirectDetection * ddetect;
          DirectDetection2D * dd2d;
        } toy = {this, cur};
        
        LPGUID pGuid = NULL;
        if (!cur->IsPrimary2D) pGuid = &cur->Guid2D;

        HRESULT hRes;
        if (FAILED (hRes = DirectDrawCreate (pGuid, &lpDD, NULL)))
          sys_fatalerror ("Can't create DirectDraw device", hRes);

        lpDD->QueryInterface (IID_IDirect3D, (LPVOID *)&lpD3D);
        if (FAILED (hRes = lpD3D->EnumDevices (DirectDetectionD3DEnumCallback, (LPVOID *)&toy)))
          sys_fatalerror ("Error when enumerating Direct3D devices.", hRes);
        
        lpD3D->Release ();
        lpDD->Release ();
      }  
      cur = cur->next;
    }
  }
  return true;
}

/// check 2d devices
bool DirectDetection::checkDevices2D ()
{
  // enumerate DDraw device
  HRESULT hRes;
  if (FAILED (hRes = DirectDrawEnumerate (DirectDetectionDDrawEnumCallback, this)))
    sys_fatalerror ("Error when enumerating DirectDraw devices.", hRes);
  
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
