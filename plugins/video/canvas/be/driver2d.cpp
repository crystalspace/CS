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
#include <sys/param.h>
#include "sysdef.h"
#include "cscom/com.h"
#include "cs2d/be/belibg2d.h"

// This is the name of the DLL. Make sure to change this if you change the DLL name!
// DAN: this might have to be changed for each OS, cuz each OS has a different extension for DLLs.
#define DLL_NAME "be2d.so"

static unsigned int gRefCount = 0;
static DllRegisterData gRegData =
{
  &CLSID_BeLibGraphics2D,
  "crystalspace.graphics2d.be",
  "Crystal Space 2D driver for BeOS"
};

#ifdef CS_STATIC_LINKED

void Be2DRegister ()
{
  static csGraphics2DBeLibFactory gG2DBeFactory;
  gRegData.pClass = &gG2DBeFactory;
  csRegisterServer (&gRegData);
}

void Be2DUnregister ()
{
  csUnregisterServer (&gRegData);
}

#else

STDAPI DllInitialize ()
{
  csCoInitialize(0);
  gRegData.szInProcServer = DLL_NAME;
  return TRUE;
}

void STDAPICALLTYPE ModuleRelease(void)
{
  gRefCount--;
}

void STDAPICALLTYPE ModuleAddRef(void)
{
  gRefCount++;
}   

// return S_OK if it's ok to unload us now.
STDAPI DllCanUnloadNow()
{
  return gRefCount ? S_FALSE : S_OK;
}

// used to get a COM class object from us.
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
  static csGraphics2DBeLibFactory gG2DBeFactory;
  if (rclsid == CLSID_BeLibGraphics2D)
    return gG2DBeFactory.QueryInterface(riid, ppv);
    
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

#endif

// Implementation of the csGraphics2DBeLib factory... ///////////

IMPLEMENT_UNKNOWN_NODELETE( csGraphics2DBeLibFactory )

BEGIN_INTERFACE_TABLE( csGraphics2DBeLibFactory )
	IMPLEMENTS_INTERFACE( IGraphics2DFactory )
END_INTERFACE_TABLE()

STDMETHODIMP csGraphics2DBeLibFactory::CreateInstance(REFIID riid, ISystem* piSystem, void**ppv)
{
	if (!piSystem)
	{
		*ppv = 0;
		return E_INVALIDARG;
	}

	CHK (csGraphics2DBeLib* pNew = new csGraphics2DBeLib(piSystem, false));
	if (!pNew)
	{
		*ppv = 0;
		return E_OUTOFMEMORY;
	}
		
	return pNew->QueryInterface(riid, ppv);
}

STDMETHODIMP csGraphics2DBeLibFactory::LockServer(COMBOOL bLock)
{
	if (bLock)
		gRefCount++;
	else
		gRefCount--;

	return S_OK;
}

