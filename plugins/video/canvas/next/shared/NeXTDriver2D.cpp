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
#include "sysdef.h"
#include "NeXTDriver2D.h"
#include "NeXTFrameBuffer.h"
#include "NeXTProxy2D.h"
#include "isystem.h"

IMPLEMENT_FACTORY(NeXTDriver2D)

EXPORT_CLASS_TABLE(next2d)
    EXPORT_CLASS(NeXTDriver2D, "crystalspace.graphics2d.next",
	"Crystal Space 2D driver for MacOS/X Server, OpenStep, and NextStep")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE(NeXTDriver2D)
    IMPLEMENTS_INTERFACE(iPlugIn)
    IMPLEMENTS_INTERFACE(iGraphics2D)
IMPLEMENT_IBASE_END

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
NeXTDriver2D::NeXTDriver2D( iBase* p ) :
    csGraphics2D(), next_system(0), proxy(0)
    {
    CONSTRUCT_IBASE(p);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTDriver2D::~NeXTDriver2D()
    {
    Close();
    if (proxy != 0)
	delete proxy;
    if (next_system != 0)
	next_system->DecRef();
    Memory = 0;
    }


//-----------------------------------------------------------------------------
// Initialize
//	We only support depth of 15 or 32.  See README.NeXT for an explanation.
//-----------------------------------------------------------------------------
bool NeXTDriver2D::Initialize(iSystem* s)
    {
    bool ok = superclass::Initialize(s);
    if (ok)
	{
	next_system = QUERY_INTERFACE (System, iNeXTSystemDriver);
	if (next_system != 0)
	    {
	    int simulated_depth = next_system->GetSimulatedDepth();
	    proxy = new NeXTProxy2D( Width, Height, simulated_depth );
	    NeXTFrameBuffer const* frame_buffer = proxy->get_frame_buffer();
	    if (frame_buffer != 0)
		{
		Depth  = frame_buffer->depth();
		Memory = frame_buffer->get_raw_buffer();
		pfmt.RedMask    = frame_buffer->red_mask();
		pfmt.GreenMask  = frame_buffer->green_mask();
		pfmt.BlueMask   = frame_buffer->blue_mask();
		pfmt.PixelBytes = frame_buffer->bytes_per_pixel();
		pfmt.PalEntries = frame_buffer->palette_entries();
		complete_pixel_format();
		switch (Depth)
		    {
		    case 15: setup_rgb_15(); break;
		    case 32: setup_rgb_32(); break;
		    }
		}
	    }
	else
	    {
	    ok = false;
	    System->Print( MSG_FATAL_ERROR, "FATAL: The system driver does "
		"not support the iNeXTSystemDriver interface\n" );
	    }
	}
    return ok;
    }


//-----------------------------------------------------------------------------
// setup_rgb_15
//-----------------------------------------------------------------------------
void NeXTDriver2D::setup_rgb_15()
    {
    _DrawPixel  = DrawPixel16;
    _WriteChar  = WriteChar16;
    _GetPixelAt = GetPixelAt16;
    _DrawSprite = DrawSprite16;
    }


//-----------------------------------------------------------------------------
// setup_rgb_32
//-----------------------------------------------------------------------------
void NeXTDriver2D::setup_rgb_32()
    {
    _DrawPixel  = DrawPixel32;
    _WriteChar  = WriteChar32;
    _GetPixelAt = GetPixelAt32;
    _DrawSprite = DrawSprite32;
    }


//-----------------------------------------------------------------------------
// Open
//-----------------------------------------------------------------------------
bool NeXTDriver2D::Open( char const* title )
    {
    bool okay = false;
    if (proxy->get_frame_buffer() == 0)
	System->Print( MSG_FATAL_ERROR, "Unsupported display depth\n" );
    else if (superclass::Open( title ))
	okay = proxy->open( title );
    return okay;
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
    proxy->flush();
    }


//-----------------------------------------------------------------------------
// SetMouseCursor
//-----------------------------------------------------------------------------
bool NeXTDriver2D::SetMouseCursor( csMouseCursorID c, iTextureHandle* bitmap )
    {
    return proxy->set_mouse_cursor( c, bitmap );
    }
