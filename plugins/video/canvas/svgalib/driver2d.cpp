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
#include "cs2d/svgalib/svga.h"

// This is the name of the DLL
#define DLL_NAME "svga2d.so"

static unsigned int gRefCount = 0;
static DllRegisterData gRegData =
{
  &CLSID_SVGALibGraphics2D,
  "crystalspace.graphics2d.svgalib",
  "Crystal Space 2D driver for SVGALib"
};

csGraphics2DSVGALibFactory gG2DSVGAlibFactory;

#ifdef CS_STATIC_LINKED

void SVGALib2DRegister ()
{
  gRegData.pClass = &gG2DSVGAlibFactory;
  csRegisterServer (&gRegData);
}

void SVGALib2DUnregister ()
{
  csUnregisterServer (&gRegData);
}

#else

STDAPI DllInitialize ()
{
  csCoInitialize (0);
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
  if (rclsid == CLSID_SVGALibGraphics2D)
    return gG2DSVGAlibFactory.QueryInterface(riid, ppv);
    
  // if we get here, rclsid is a class we don't implement
  *ppv = NULL;
  return CLASS_E_CLASSNOTAVAILABLE;
}

// Called by RegSvr32.exe
STDAPI DllRegisterServer()
{
  HRESULT hRes;
  hRes = csRegisterServer(&gRegData);
  return hRes;
}

// Called by RegSvr32.exe
STDAPI DllUnregisterServer()
{
  HRESULT hRes;
  hRes = csRegisterServer(&gRegData);
  return hRes;
}
#endif

// Implementation of the csGraphics2DSVGALib factory... ///////////

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DSVGALibFactory)

BEGIN_INTERFACE_TABLE (csGraphics2DSVGALibFactory)
	IMPLEMENTS_INTERFACE (IGraphics2DFactory)
END_INTERFACE_TABLE ()

STDMETHODIMP csGraphics2DSVGALibFactory::CreateInstance (REFIID riid, ISystem* piSystem, void**ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_INVALIDARG;
  }

  CHK (csGraphics2DSVGALib* pNew = new csGraphics2DSVGALib (piSystem));
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }
		
  return pNew->QueryInterface (riid, ppv);
}

STDMETHODIMP csGraphics2DSVGALibFactory::LockServer(COMBOOL bLock)
{
  if (bLock)
    gRefCount++;
  else
    gRefCount--;
  return S_OK;
}

