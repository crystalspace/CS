/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#include "sysdef.h"
#include "cscom/com.h"
#include "cs3d/direct3d6/g3d_d3d.h"

#define DLL_NAME "Direct3DDX6Render.dll"

extern const CLSID CLSID_Direct3DDX6Graphics3D;

static int gb_cRef = 0;
static DllRegisterData gb_regData=
{
  &CLSID_Direct3DDX6Graphics3D,
  "crystalspace.graphics3d.direct3d.dx6",
  "csGraphics3D Direct3D DX6 Driver",
  DLL_NAME
};

void _stdcall ModuleAddRef()
{
  gb_cRef++;
}

void _stdcall ModuleRelease()
{
  gb_cRef--;
}

HINSTANCE DllHandle;

// our main entry point...should be called when we're loaded.
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  DllHandle = hinstDLL;
  
  return TRUE;
}

// return S_OK if it's ok to unload us now.
STDAPI DllCanUnloadNow()
{
  return gb_cRef ? S_FALSE : S_OK;
}

// used to get a COM class object from us.
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
  static csGraphics3DDirect3DDx6Factory factory;
  
  if (rclsid == CLSID_Direct3DDX6Graphics3D)
    return factory.QueryInterface(riid, ppv);
  
  // if we get here, rclsid is a class we don't implement
  *ppv = NULL;
  return CLASS_E_CLASSNOTAVAILABLE;
}

// Called by RegSvr32.exe
STDAPI DllRegisterServer()
{
  return csRegisterServer(&gb_regData);
}

// Called by RegSvr32.exe
STDAPI DllUnregisterServer()
{
  return csUnregisterServer(&gb_regData);
}

// Implementation of the csGraphics2DWin32 factory... ///////////

IMPLEMENT_UNKNOWN( csGraphics3DDirect3DDx6Factory )

BEGIN_INTERFACE_TABLE( csGraphics3DDirect3DDx6Factory )
IMPLEMENTS_INTERFACE( IGraphicsContextFactory )
END_INTERFACE_TABLE()


STDMETHODIMP csGraphics3DDirect3DDx6Factory::CreateInstance(REFIID riid, ISystem* piSystem, void**ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_INVALIDARG;
  }
  
  csGraphics3DDirect3DDx6* pNew = new csGraphics3DDirect3DDx6(piSystem);
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }
		
  return pNew->QueryInterface(riid, ppv);
}

STDMETHODIMP csGraphics3DDirect3DDx6Factory::LockServer(BOOL bLock)
{
  if (bLock)
    gb_cRef++;
  else
    gb_cRef--;
  
  return S_OK;
}
