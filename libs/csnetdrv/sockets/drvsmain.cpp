/*
    Network interface DLL

    Copyright (C) 1998, 1999 by Jorrit Tyberghein
    Copyright (C) 1998, 1999 by Serguei 'Snaar' Narojnyi
    Written by Serguei 'Snaar' Narojnyi

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
#define SYSDEF_SOCKETS
#include "sysdef.h"
#include "cscom/com.h"
#include "csnetdrv/sockets/netsdrv.h"
#include "inetdrv.h"

static unsigned int gRefCount = 0;
static DllRegisterData gRegData =
{
  &CLSID_SocketsNetworkDriver,
  "crystalspace.network.driver.sockets",
  "Crystal Space BSD Sockets Network Driver"
};

#ifdef CS_STATIC_LINKED

void NetSocksRegister ()
{
  static csNetworkDriverSocketsFactory gNetDrvSocksFactory;
  gRegData.pClass = &gNetDrvSocksFactory;
  csRegisterServer (&gRegData);
}

void NetSocksUnregister ()
{
  csUnregisterServer (&gRegData);
}

#else

// This is the name of the DLL
#ifdef OS_WIN32
#define DLL_NAME "NetworkDriverSockets.dll"
#elif defined (OS_OS2)
#define DLL_NAME "netdrvs.dll"
#elif defined (OS_NEXT)
#define DLL_NAME "netdrvs.dl"
#else
#define DLL_NAME "netdrvs.so"
#endif

// our main entry point... should be called when we're loaded.
STDAPI DllInitialize ()
{
  csCoInitialize(0);
  gRegData.szInProcServer = DLL_NAME;
  return TRUE;
}

void STDAPICALLTYPE ModuleRelease()
{
  gRefCount--;
}

void STDAPICALLTYPE ModuleAddRef()
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
  static csNetworkDriverSocketsFactory gNetDrvSocksFactory;
  if (rclsid == CLSID_SocketsNetworkDriver)
    return gNetDrvSocksFactory.QueryInterface (riid, ppv);

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
  return csUnregisterServer(&gRegData);
}

#endif

//--------------------------------- Implementation of csNetworkSocketsFactory --

IMPLEMENT_UNKNOWN_NODELETE (csNetworkDriverSocketsFactory)

BEGIN_INTERFACE_TABLE (csNetworkDriverSocketsFactory)
  IMPLEMENTS_INTERFACE (INetworkDriverFactory)
END_INTERFACE_TABLE ()

STDMETHODIMP csNetworkDriverSocketsFactory::CreateInstance(REFIID riid, ISystem* piSystem, void** ppv)
{
  if (!piSystem)
  {
    *ppv = 0;
    return E_UNEXPECTED;
  }

  csNetworkDriverSockets* pNew = new csNetworkDriverSockets(piSystem);
  if (!pNew)
  {
    *ppv = 0;
    return E_OUTOFMEMORY;
  }

  return pNew->QueryInterface (riid, ppv);
}

STDMETHODIMP csNetworkDriverSocketsFactory::LockServer(BOOL bLock)
{
  if (bLock)
    gRefCount++;
  else
    gRefCount--;

  return S_OK;
}

