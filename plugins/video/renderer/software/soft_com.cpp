/*
    Software renderer DLL
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
#include "cs3d/software/soft_g3d.h"

static unsigned int gRefCount = 0;
static DllRegisterData gRegData =
{
  &CLSID_SoftwareGraphics3D,
  "crystalspace.graphics3d.software",
  "Crystal Space 3D software rendering driver"
};

#ifdef CS_STATIC_LINKED

void SoftRenderRegister ()
{
  static csGraphics3DSoftwareFactory gG3DSoftFactory;
  gRegData.pClass = &gG3DSoftFactory;
  csRegisterServer (&gRegData);
}

void SoftRenderUnregister ()
{
  csUnregisterServer (&gRegData);
}

#else

// This is the name of the DLL. Make sure to change this if you change the DLL name!
// DAN: this might have to be changed for each OS, cuz each OS has a different extension for DLLs.
#ifdef OS_WIN32
#  define DLL_NAME "SoftwareRender.dll"
#elif defined (OS_OS2)
#  define DLL_NAME "softrndr.dll"
#elif defined (OS_MACOS)
#  define DLL_NAME "SoftwareRender.shlb"
#elif defined (OS_NEXT)
#define DLL_NAME "softrndr.dl"
#else
#define DLL_NAME "softrndr.so"
#endif

// our main entry point...should be called when we're loaded.
STDAPI DllInitialize ()
{
  csCoInitialize (0);
  gRegData.szInProcServer = DLL_NAME;
  return TRUE;
}

void STDAPICALLTYPE ModuleRelease ()
{
  gRefCount--;
}

void STDAPICALLTYPE ModuleAddRef ()
{
  gRefCount++;
}

// return S_OK if it's ok to unload us now.
STDAPI DllCanUnloadNow()
{
  return gRefCount ? S_FALSE : S_OK;
}

// used to get a COM class object from us.
STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, void** ppv)
{
  static csGraphics3DSoftwareFactory gG3DSoftFactory;

  if (rclsid == CLSID_SoftwareGraphics3D)
    return gG3DSoftFactory.QueryInterface (riid, ppv);

  // if we get here, rclsid is a class we don't implement
  *ppv = NULL;
  return CLASS_E_CLASSNOTAVAILABLE;
}

// Called by RegSvr32.exe
STDAPI DllRegisterServer ()
{
  return csRegisterServer (&gRegData);
}

// Called by RegSvr32.exe
STDAPI DllUnregisterServer ()
{
  return csUnregisterServer (&gRegData);
}

#endif

// Implementation of csGraphics3DSoftwareFactory /////////////////

IMPLEMENT_UNKNOWN (csGraphics3DSoftwareFactory)

BEGIN_INTERFACE_TABLE (csGraphics3DSoftwareFactory)
  IMPLEMENTS_INTERFACE (IGraphicsContextFactory)
END_INTERFACE_TABLE ()

STDMETHODIMP csGraphics3DSoftwareFactory::CreateInstance (REFIID riid,
  ISystem* piSystem, void** ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_UNEXPECTED;
  }

  CHK (csGraphics3DSoftware* pNew = new csGraphics3DSoftware (piSystem));
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }
	
  return pNew->QueryInterface (riid, ppv);
}

STDMETHODIMP csGraphics3DSoftwareFactory::LockServer (COMBOOL bLock)
{
  if (bLock)
    gRefCount++;
  else
    gRefCount--;

  return S_OK;
}

