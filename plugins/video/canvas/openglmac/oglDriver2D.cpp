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
#include "cs2d/openglmac/oglg2d.h"

#define DLL_NAME "OpenGL2D.shlb"

static unsigned int gb_cRef = 0;

static DllRegisterData gb_regData=
{
  &CLSID_OpenGLGraphics2D,
  "crystalspace.graphics2d.defaultgl",
  "Crystal Space 2D driver for OpenGL on MacOS",
  DLL_NAME
};


void STDAPICALLTYPE ModuleRelease(void)
{
  gb_cRef--;
}

void STDAPICALLTYPE ModuleAddRef(void)
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
  static csGraphics2DOpenGLFactory s_g2dFactory;
   
  if (rclsid == CLSID_OpenGLGraphics2D)
    return s_g2dFactory.QueryInterface(riid, ppv);
    
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

// Implementation of the csGraphics2DOpenGL factory... ///////////

IMPLEMENT_UNKNOWN_NODELETE (csGraphics2DOpenGLFactory)

BEGIN_INTERFACE_TABLE (csGraphics2DOpenGLFactory)
  IMPLEMENTS_INTERFACE (IGraphics2DFactory)
  IMPLEMENTS_INTERFACE (IGraphics2DOpenGLFactory)
END_INTERFACE_TABLE()

STDMETHODIMP csGraphics2DOpenGLFactory::CreateInstance(REFIID riid, ISystem* piSystem, void**ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_INVALIDARG;
  }

  csGraphics2DOpenGL* pNew = new csGraphics2DOpenGL(piSystem, false);
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }
		
  return pNew->QueryInterface(riid, ppv);
}

STDMETHODIMP csGraphics2DOpenGLFactory::LockServer(BOOL bLock)
{
  if (bLock)
    gb_cRef++;
  else
    gb_cRef--;

  return S_OK;
}
