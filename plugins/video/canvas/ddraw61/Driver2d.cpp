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
#include "cs2d/ddraw61/g2d.h"

// This is the name of the DLL. Make sure to change this if you change the DLL name!
// DAN: this might have to be changed for each OS, cuz each OS has a different extension for DLLs.
#define DLL_NAME "DirectDraw2DDX61.dll"

static unsigned int gb_cRef = 0;

static DllRegisterData gb_reg3dData=
{
  &CLSID_DirectDrawDX61With3DGraphics2D,
  "crystalspace.graphics2d.direct3d.dx61",
  "csGraphics2D DirectDraw DX61 Implementation for Direct3D DX61",
  DLL_NAME
};

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
    static csGraphics2DWithDirect3DFactory s_g2dg3dFactory;
    
    if (rclsid == CLSID_DirectDrawDX61With3DGraphics2D)
        return s_g2dg3dFactory.QueryInterface(riid, ppv);
    
    // if we get here, rclsid is a class we don't implement
    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}

// Called by RegSvr32.exe
STDAPI DllRegisterServer()
{
    //HRESULT hRes;
   
    return csRegisterServer(&gb_reg3dData);
}

// Called by RegSvr32.exe
STDAPI DllUnregisterServer()
{
    //HRESULT hRes;
    
    return csUnregisterServer(&gb_reg3dData);
}

// Implementation of the csGraphics2DWithDirect3D factory... ///////////

IMPLEMENT_UNKNOWN_NODELETE( csGraphics2DWithDirect3DFactory )

BEGIN_INTERFACE_TABLE( csGraphics2DWithDirect3DFactory )
	IMPLEMENTS_INTERFACE( IGraphics2DFactory )
        IMPLEMENTS_INTERFACE( IGraphics2DDirect3DFactory )
END_INTERFACE_TABLE()


STDMETHODIMP csGraphics2DWithDirect3DFactory::CreateInstance(REFIID riid, ISystem* piSystem, void**ppv)
{
	if (!piSystem)
	{
		*ppv = 0;
		return E_INVALIDARG;
	}

	csGraphics2DDDraw6* pNew = new csGraphics2DDDraw6(piSystem, true);
	if (!pNew)
	{
		*ppv = 0;
		return E_OUTOFMEMORY;
	}
		
	return pNew->QueryInterface(riid, ppv);
}

STDMETHODIMP csGraphics2DWithDirect3DFactory::LockServer(BOOL bLock)
{
	if (bLock)
		gb_cRef++;
	else
		gb_cRef--;

	return S_OK;
}
