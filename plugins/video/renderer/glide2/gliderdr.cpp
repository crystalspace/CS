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
#include "cs3d/glide2/g3dglide.h"

#ifdef OS_WIN32
#define DLL_NAME "Glide2xRender.dll"
#elif OS_LINUX
#define DLL_NAME "Glide2xRender.so"
#define IS_DEFAULT
#elif OS_MACOS
#define DLL_NAME "Glide2xRender.shlb"
#endif

extern const CLSID CLSID_Glide2xGraphics3D;

static int gb_cRef = 0;

static DllRegisterData gb_regData=
{
  &CLSID_Glide2xGraphics3D,
  "crystalspace.graphics3d.glide.2x",
  "csGraphics3D Glide2x Driver",
  DLL_NAME,
};

static DllRegisterData gb_regDataDefault=
{
  &CLSID_Glide2xGraphics3D,
  "crystalspace.graphics3d.glide",
  "csGraphics3D Glide2x Driver",
  DLL_NAME,
};

void STDAPICALLTYPE ModuleAddRef()
{
  gb_cRef++;
}

void STDAPICALLTYPE ModuleRelease()
{
  gb_cRef--;
}

#ifdef OS_WIN32

HINSTANCE DllHandle;

// our main entry point...should be called when we're loaded.
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD /*fdwReason*/, LPVOID /*lpvReserved*/)
{
  DllHandle = hinstDLL;
  
  return TRUE;
}

#endif

#if OS_LINUX || OS_MACOS
  STDAPI DllInitialize ()
  {
       csCoInitialize (0);
   
       return TRUE;
  }
#endif

// return S_OK if it's ok to unload us now.
STDAPI DllCanUnloadNow()
{
  return gb_cRef ? S_FALSE : S_OK;
}

// used to get a COM class object from us.
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
  static csGraphics3DGlide2xFactory factory;
  
  if (rclsid == CLSID_Glide2xGraphics3D)
    return factory.QueryInterface(riid, ppv);
  
  // if we get here, rclsid is a class we don't implement
  *ppv = NULL;
  return CLASS_E_CLASSNOTAVAILABLE;
}

// Called by RegSvr32.exe
STDAPI DllRegisterServer()
{
#ifdef IS_DEFAULT  
  HRESULT hRes;
  
  hRes = csRegisterServer(&gb_regDataDefault);
  if (FAILED(hRes))
    return hRes;
#endif
  return csRegisterServer(&gb_regData);
}

// Called by RegSvr32.exe
STDAPI DllUnregisterServer()
{
#ifdef IS_DEFAULT  
  HRESULT hRes;
  
  hRes = csRegisterServer(&gb_regDataDefault);
  if (FAILED(hRes))
    return hRes;
#endif
  return csUnregisterServer(&gb_regData);
}

// Implementation of the csGraphics2DGlide2x factory... ///////////

IMPLEMENT_UNKNOWN_NODELETE( csGraphics3DGlide2xFactory )

BEGIN_INTERFACE_TABLE( csGraphics3DGlide2xFactory )
IMPLEMENTS_INTERFACE( IGraphicsContextFactory )
END_INTERFACE_TABLE()


STDMETHODIMP csGraphics3DGlide2xFactory::CreateInstance(REFIID riid, ISystem* piSystem, void**ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_INVALIDARG;
  }
  
  csGraphics3DGlide2x* pNew = new csGraphics3DGlide2x(piSystem);
  if (!pNew)
    {
      *ppv = 0;
      return E_OUTOFMEMORY;
    }

  return pNew->QueryInterface(riid, ppv);
}

STDMETHODIMP csGraphics3DGlide2xFactory::LockServer(BOOL bLock)
{
  if (bLock)
    gb_cRef++;
  else
    gb_cRef--;
  
  return S_OK;
}
