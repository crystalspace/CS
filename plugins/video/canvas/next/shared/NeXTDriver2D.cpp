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
// NeXTDriver2D.cpp
//
//	NeXT-specific subclass of csGraphics2D which implements 2D-graphic
//	functionality via the AppKit.
//
//-----------------------------------------------------------------------------
#include "NeXTDriver2D.h"
#include "NeXTProxy2D.h"
#include "isystem.h"

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTDriver2D::~NeXTDriver2D()
    {
    Close();
    if (proxy != 0)
	{
	free( Memory );
	delete proxy;
	}
    }


//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
void NeXTDriver2D::Initialize()
    {
    superclass::Initialize();
    Depth = 8;	// FIXME: Hardcoded for now to simulate palette environment.
    proxy = new NeXTProxy2D( Width, Height );
    Memory = (unsigned char*)malloc( Width * Height );
    }


//-----------------------------------------------------------------------------
// Open
//-----------------------------------------------------------------------------
bool NeXTDriver2D::Open( char* title )
    {
    return (superclass::Open(title) && proxy->open(title));
    }


//-----------------------------------------------------------------------------
// Close
//-----------------------------------------------------------------------------
void NeXTDriver2D::Close()
    {
    proxy->close();
    superclass::Close();
    }


//-----------------------------------------------------------------------------
// Print -- A misnomer; actually flushes frame buffer to display.
//-----------------------------------------------------------------------------
void NeXTDriver2D::Print( csRect* )
    {
    proxy->flush( Memory );
    }


//-----------------------------------------------------------------------------
// SetRGB -- See README.NeXT for discussion of simulated palette environment.
//-----------------------------------------------------------------------------
void NeXTDriver2D::SetRGB( int i, int r, int g, int b )
    {
    superclass::SetRGB( i, r, g, b );
    proxy->set_rgb( i, r, g, b );
    }


//-----------------------------------------------------------------------------
// SetMouseCursor
//-----------------------------------------------------------------------------
bool NeXTDriver2D::SetMouseCursor( int shape, ITextureHandle* bitmap )
    {
    return proxy->set_mouse_cursor( shape, bitmap );
    }
