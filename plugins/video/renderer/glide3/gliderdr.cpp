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
#include "cs3d/glide3/g3dglide.h"

#define DLL_NAME "Glide3xRender.dll"

static int gb_cRef = 0;
static DllRegisterData gb_regData=
{
  &CLSID_Glide3xGraphics3D,
  "crystalspace.graphics3d.glide.3x",
  "csGraphics3D Glide3x Driver",
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
  static csGraphics3DGlide3xFactory factory;
  
  if (rclsid == CLSID_Glide3xGraphics3D)
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

IMPLEMENT_UNKNOWN_NODELETE( csGraphics3DGlide3xFactory )

BEGIN_INTERFACE_TABLE (csGraphics3DGlide3xFactory)
  IMPLEMENTS_INTERFACE (IGraphicsContextFactory)
END_INTERFACE_TABLE ()


STDMETHODIMP csGraphics3DGlide3xFactory::CreateInstance(REFIID riid, ISystem* piSystem, void**ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_INVALIDARG;
  }
  
  csGraphics3DGlide3x* pNew = new csGraphics3DGlide3x(piSystem);
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }
		
  return pNew->QueryInterface(riid, ppv);
}

STDMETHODIMP csGraphics3DGlide3xFactory::LockServer(BOOL bLock)
{
  if (bLock)
    gb_cRef++;
  else
    gb_cRef--;
  
  return S_OK;
}
