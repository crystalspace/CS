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
#include "cs3d/opengl/ogl_g3d.h"

static unsigned int gRefCount = 0;
static DllRegisterData gRegData =
{
  &CLSID_OpenGLGraphics3D,
  "crystalspace.graphics3d.opengl",
  "Crystal Space 3D OpenGL rendering driver"
};

csGraphics3DOpenGLFactory gG3DOpenGLFactory;

#ifdef CS_STATIC_LINKED

void OpenGLRenderRegister ()
{
  gRegData.pClass = &gG3DOpenGLFactory;
  csRegisterServer (&gRegData);
}

void OpenGLRenderUnregister ()
{
  csUnregisterServer (&gRegData);
}

#else

// This is the name of the DLL
#if defined (OS_WIN32)
#define DLL_NAME "OpenGLRender.dll"
#elif defined (OS_OS2)
#define DLL_NAME "glrender.dll"
#elif defined (OS_MACOS)
#define DLL_NAME "OpenGLRender.shlb"
#elif defined (OS_NEXT)
#define DLL_NAME "glrender.dylib"
#else
#define DLL_NAME "glrender.so"
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
  if (rclsid == CLSID_OpenGLGraphics3D)
    return gG3DOpenGLFactory.QueryInterface (riid, ppv);

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


// Implementation of csGraphics3DOpenGLFactory /////////////////

IMPLEMENT_UNKNOWN (csGraphics3DOpenGLFactory)

BEGIN_INTERFACE_TABLE (csGraphics3DOpenGLFactory)
	IMPLEMENTS_INTERFACE (IGraphicsContextFactory)
END_INTERFACE_TABLE ()

STDMETHODIMP csGraphics3DOpenGLFactory::CreateInstance (REFIID riid,
	ISystem* piSystem, void** ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_UNEXPECTED;
  }

  CHK (csGraphics3DOpenGL* pNew = new csGraphics3DOpenGL (piSystem));
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }
	
  return pNew->QueryInterface (riid, ppv);
}

STDMETHODIMP csGraphics3DOpenGLFactory::LockServer (COMBOOL bLock)
{
  if (bLock)
    gRefCount++;
  else
    gRefCount--;

  return S_OK;
}

