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
#include <string.h>
#include "sysdef.h"
#include "cscom/com.h"
#include "MacGraphics.h"

#define DLL_NAME "Driver2D.shlb"

static unsigned int gRefCount = 0;
static DllRegisterData gRegData =
{
  &CLSID_MacGraphics2D,
  "crystalspace.graphics2d.mac",
  "Crystal Space 2D graphics driver for Macintosh",
  DLL_NAME,
};

void STDAPICALLTYPE ModuleRelease();
void STDAPICALLTYPE ModuleAddRef();

void STDAPICALLTYPE ModuleRelease()
{
    gRefCount--;
}

void STDAPICALLTYPE ModuleAddRef()
{
    gRefCount++;
}   

// our main entry point...should be called when we're loaded.
STDAPI DllInitialize()
{
    csCoInitialize(0);
//	gRegData.szInProcServer = DLL_NAME;
    return TRUE;
}

// return S_OK if it's ok to unload us now.
STDAPI DllCanUnloadNow()
{
    return gRefCount ? S_FALSE : S_OK;
}

// used to get a COM class object from us.
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    static csGraphics2DMacFactory s_g2dFactory;
    
    if (rclsid == CLSID_MacGraphics2D)
        return s_g2dFactory.QueryInterface(riid, ppv);
    
    // if we get here, rclsid is a class we don't implement
    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}

// Called by RegSvr32.exe
STDAPI DllRegisterServer()
{
    return csRegisterServer(&gRegData);
}

// Called by RegSvr32.exe
STDAPI DllUnregisterServer()
{
    return csRegisterServer(&gRegData);
}

// Implementation of the csGraphics2DMac factory... ///////////

IMPLEMENT_UNKNOWN_NODELETE( csGraphics2DMacFactory )

BEGIN_INTERFACE_TABLE( csGraphics2DMacFactory )
	IMPLEMENTS_INTERFACE( IGraphics2DFactory )
END_INTERFACE_TABLE()


STDMETHODIMP csGraphics2DMacFactory::CreateInstance(REFIID riid, ISystem* piSystem, void**ppv)
{
	if (!piSystem)
	{
		*ppv = 0;
		return E_INVALIDARG;
	}

	csGraphics2DMac* pNew = new csGraphics2DMac(piSystem);
	if (!pNew)
	{
		*ppv = 0;
		return E_OUTOFMEMORY;
	}
		
	return pNew->QueryInterface(riid, ppv);
}

STDMETHODIMP csGraphics2DMacFactory::LockServer(BOOL bLock)
{
	if (bLock)
		gRefCount++;
	else
		gRefCount--;

	return S_OK;
}

