#ifndef __NeXT_NeXTDriver2D_h
#define __NeXT_NeXTDriver2D_h
//=============================================================================
//
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
// NeXTDriver2D.h
//
//	NeXT-specific subclass of csGraphics2D which implements 2D-graphic
//	functionality via the AppKit.
//
//-----------------------------------------------------------------------------
#include "sysdef.h"	// Defines wchar_t required by com.h below.
#include "cscom/com.h"
#include "cs2d/common/graph2d.h"
#include "cssys/next/shared/NeXTSystemInterface.h"
class NeXTProxy2D;

class NeXTDriver2D : public csGraphics2D
    {
    typedef csGraphics2D superclass;

private:
    INeXTSystemDriver* next_system;
    NeXTProxy2D* proxy;	// Interface to Objective-C world; see README.NeXT.

    void setup_rgb_15();
    void setup_rgb_32();

public:
    NeXTDriver2D::NeXTDriver2D( ISystem* );
    virtual ~NeXTDriver2D();
    virtual void Initialize();
    virtual bool Open( char const* title );
    virtual void Close();
    virtual void Print( csRect* = 0 );
    virtual bool SetMouseCursor( int shape, ITextureHandle* );
    };

#endif //__NeXT_NeXTDriver2D_h
