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

#include <stdlib.h>
#include "sysdef.h"
#include "cscom/com.h"
#include "cs2d/ddraw/g2d.h"

#define DLL_NAME "DirectDraw2D.dll"

static unsigned int gb_cRef = 0;

static DllRegisterData gb_regData =
{
  &CLSID_DirectDrawGraphics2D,
  SOFTWARE_2D_DRIVER,
  "csGraphics2D DirectDraw DX3 Implementation",
  DLL_NAME
};

static DllRegisterData gb_reg3dData =
{
  &CLSID_DirectDrawWith3DGraphics2D,
  "crystalspace.graphics2d.direct3d.dx5",
  "csGraphics2D DirectDraw DX5 Implementation for Direct3D DX5",
  DLL_NAME
};

#ifdef CS_STATIC_LINKED

void DDraw2DRegister ()
{
  static csGraphics2DDDraw3Factory s_g2dFactory;
  gb_regData.pClass = &s_g2dFactory;
  csRegisterServer (&gb_regData);

  static csGraphics2DDDraw3WithDirect3DFactory s_g2dg3dFactory;
  gb_reg3dData.pClass = &s_g2dg3dFactory;
  csRegisterServer (&gb_reg3dData);
}

void DDraw2DUnregister ()
{
  csUnregisterServer (&gb_regData);
  csUnregisterServer (&gb_reg3dData);
}

#else

EXTERN_C void STDAPICALLTYPE ModuleRelease(void)
{
  gb_cRef--;
}

EXTERN_C void STDAPICALLTYPE ModuleAddRef(void)
{
  gb_cRef++;
}   

// our main entry point...should be called when we're loaded.
STDAPI DllInitialize()
{
  csCoInitialize(0);
  return S_OK;
}

// return S_OK if it's ok to unload us now.
STDAPI DllCanUnloadNow()
{
  return gb_cRef ? S_FALSE : S_OK;
}

// used to get a COM class object from us.
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
  static csGraphics2DDDraw3Factory s_g2dFactory;
  static csGraphics2DDDraw3WithDirect3DFactory s_g2dg3dFactory;
  
  if (rclsid == CLSID_DirectDrawGraphics2D)
    return s_g2dFactory.QueryInterface(riid, ppv);
  else if (rclsid == CLSID_DirectDrawWith3DGraphics2D)
    return s_g2dg3dFactory.QueryInterface(riid, ppv);
  
  // if we get here, rclsid is a class we don't implement
  *ppv = NULL;
  return CLASS_E_CLASSNOTAVAILABLE;
}

// Called by RegSvr32.exe
STDAPI DllRegisterServer()
{
  HRESULT hRes;
  hRes = csRegisterServer(&gb_regData);
  if (FAILED(hRes))
    return hRes;
  
  return csRegisterServer(&gb_reg3dData);
}

// Called by RegSvr32.exe
STDAPI DllUnregisterServer()
{
  HRESULT hRes;
  
  hRes = csUnregisterServer(&gb_regData);
  if (FAILED(hRes))
    return hRes;
  
  return csUnregisterServer(&gb_reg3dData);
}

#endif // CS_STATIC_LINKED

// Implementation of the csGraphics2DDDraw3 factory... ///////////

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DDDraw3Factory)

BEGIN_INTERFACE_TABLE (csGraphics2DDDraw3Factory)
IMPLEMENTS_INTERFACE (IGraphics2DFactory)
IMPLEMENTS_INTERFACE (IGraphics2DDirect3DFactory)
END_INTERFACE_TABLE ()

BEGIN_INTERFACE_TABLE (csGraphics2DDDraw3WithDirect3DFactory)
IMPLEMENTS_INTERFACE (IGraphics2DFactory)
IMPLEMENTS_INTERFACE (IGraphics2DDirect3DFactory)
END_INTERFACE_TABLE ()

STDMETHODIMP csGraphics2DDDraw3Factory::CreateInstance(REFIID riid, ISystem* piSystem, void**ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_INVALIDARG;
  }
  
  csGraphics2DDDraw3* pNew = new csGraphics2DDDraw3(piSystem, false);
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }
		
  return pNew->QueryInterface(riid, ppv);
}

STDMETHODIMP csGraphics2DDDraw3Factory::LockServer(BOOL bLock)
{
  if (bLock)
    gb_cRef++;
  else
    gb_cRef--;
  
  return S_OK;
}

STDMETHODIMP csGraphics2DDDraw3WithDirect3DFactory::CreateInstance(REFIID riid, ISystem* piSystem, void**ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_INVALIDARG;
  }
  
  csGraphics2DDDraw3* pNew = new csGraphics2DDDraw3(piSystem, true);
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }
		
  return pNew->QueryInterface(riid, ppv);
}
