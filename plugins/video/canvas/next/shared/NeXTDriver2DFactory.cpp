//=============================================================================
//
//	Copyright (C)1998 by Jorrit Tyberghein
//	Copyright (C)1998 by Dan Ogles <DOgles@peachtree.com>
//	Copyright (C)1999 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// NeXTDriver2DFactory.cpp
//
//	The NeXTDriver2D factory object for use with COM.  Vends instances of
//	NeXTDriver2D to clients and also performs COM registration for
//	both statically and dynamically linked applications.
//
//-----------------------------------------------------------------------------
#include "NeXTDriver2DFactory.h"
#include "NeXTDriver2D.h"
#include "cscom/com.h"

#define DLL_NAME "libnext2d.dl"

static unsigned int FACTORY_REF_COUNT = 0;

//-----------------------------------------------------------------------------
// COM registration information.
//-----------------------------------------------------------------------------
extern CLSID const CLSID_NeXTGraphics2D;
static DllRegisterData COM_REG_DATA =
    {
    &CLSID_NeXTGraphics2D,
    "crystalspace.graphics2d.next",
    "Crystal Space 2D driver for MacOS/X Server, OpenStep, and NextStep"
    };


//=============================================================================
// Implementation of COM registration for statically linked applications.
//=============================================================================
#ifdef CS_STATIC_LINKED

//-----------------------------------------------------------------------------
// NeXT2DRegister
//-----------------------------------------------------------------------------
void NeXT2DRegister()
    {
    static NeXTDriver2DFactory factory;
    COM_REG_DATA.pClass = &factory;
    csRegisterServer( &COM_REG_DATA );
    }


//-----------------------------------------------------------------------------
// NeXT2DUnregister
//-----------------------------------------------------------------------------
void NeXT2DUnregister ()
    {
    csUnregisterServer( &COM_REG_DATA );
    }


//=============================================================================
// Implementation of COM registration for dynamically linked applications.
//=============================================================================
#else // CS_STATIC_LINKED

void STDAPICALLTYPE ModuleRelease() { FACTORY_REF_COUNT--; }
void STDAPICALLTYPE ModuleAddRef()  { FACTORY_REF_COUNT++; }   
STDAPI DllRegisterServer()   { return csRegisterServer( &COM_REG_DATA ); }
STDAPI DllUnregisterServer() { return csRegisterServer( &COM_REG_DATA ); }
STDAPI DllCanUnloadNow() { return (FACTORY_REF_COUNT == 0) ? S_OK: S_FALSE; }

//-----------------------------------------------------------------------------
// DllInitialize
//-----------------------------------------------------------------------------
STDAPI DllInitialize()
    {
    csCoInitialize(0);
    COM_REG_DATA.szInProcServer = DLL_NAME;
    return TRUE;
    }


//-----------------------------------------------------------------------------
// DllGetClassObject
//-----------------------------------------------------------------------------
STDAPI DllGetClassObject( REFCLSID rclsid, REFIID riid, void** ppv )
    {
    static NeXTDriver2DFactory factory;
    if (rclsid == CLSID_NeXTGraphics2D)
	return factory.QueryInterface( riid, ppv );
    *ppv = 0;
    return CLASS_E_CLASSNOTAVAILABLE;
    }

#endif	// CS_STATIC_LINKED


//=============================================================================
// The actual factory object used for both static and dynamic linking.
//=============================================================================
IMPLEMENT_UNKNOWN_NODELETE(NeXTDriver2DFactory)
BEGIN_INTERFACE_TABLE(NeXTDriver2DFactory)
    IMPLEMENTS_INTERFACE(IGraphics2DFactory)
END_INTERFACE_TABLE()


//-----------------------------------------------------------------------------
// CreateInstance
//-----------------------------------------------------------------------------
STDMETHODIMP
NeXTDriver2DFactory::CreateInstance( REFIID riid, ISystem* sys, void** ppv )
    {
    HRESULT rc;
    *ppv = 0;
    if (sys == 0)
	rc = E_INVALIDARG;
    else
	{
	NeXTDriver2D* pNew = new NeXTDriver2D( sys );
	if (pNew == 0)
	    rc = E_OUTOFMEMORY;
	else
	    rc = pNew->QueryInterface( riid, ppv );
	}
    return rc;
    }


//-----------------------------------------------------------------------------
// LockServer
//-----------------------------------------------------------------------------
STDMETHODIMP NeXTDriver2DFactory::LockServer( BOOL lock )
    {
    if (lock)
	FACTORY_REF_COUNT++;
    else
	FACTORY_REF_COUNT--;
    return S_OK;
    }
