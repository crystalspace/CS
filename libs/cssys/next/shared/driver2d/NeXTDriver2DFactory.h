#ifndef __NeXT_NeXTDriver2DFactory_h
#define __NeXT_NeXTDriver2DFactory_h
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
// NeXTDriver2DFactory.h
//
//	The NeXTDriver2D factory object for use with COM.  Vends instances of
//	NeXTDriver2D to clients and also performs COM registration for
//	both statically and dynamically linked applications.
//
//-----------------------------------------------------------------------------
#include "render/graph2d.h"
#include "cscom/com.h"

interface INeXTDriver2DFactory : public IGraphics2DFactory {};

class NeXTDriver2DFactory : public INeXTDriver2DFactory
    {
public:
    DECLARE_IUNKNOWN()
    DECLARE_INTERFACE_TABLE(csGraphics2DNextStepLibFactory)
    STDMETHOD(CreateInstance)( REFIID riid, ISystem*, void** );
    STDMETHOD(LockServer)( BOOL );
    };

#endif //__NeXT_NeXTDriver2DFactory_h
