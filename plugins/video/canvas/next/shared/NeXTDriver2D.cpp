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
#include "NeXTFrameBuffer.h"
#include "NeXTProxy2D.h"
#include "isystem.h"

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
NeXTDriver2D::NeXTDriver2D( ISystem* s ) :
    csGraphics2D(s), next_system(0), proxy(0)
    {
    if (FAILED(system->QueryInterface( IID_INeXTSystemDriver,
	(void**)&next_system )))
	{
	system->Print( MSG_FATAL_ERROR, "FATAL: The system driver does not "
		"support the INeXTSystemDriver interface\n" );
	exit(1);
	}
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTDriver2D::~NeXTDriver2D()
    {
    Close();
    if (proxy != 0)
	delete proxy;
    Memory = 0;
    }


//-----------------------------------------------------------------------------
// Initialize
//	We only support depth of 15 or 32.  See README.NeXT for an explanation.
//-----------------------------------------------------------------------------
void NeXTDriver2D::Initialize()
    {
    superclass::Initialize();
    int simulated_depth;
    next_system->GetSimulatedDepth( simulated_depth );
    proxy = new NeXTProxy2D( Width, Height, simulated_depth );
    NeXTFrameBuffer const* const frame_buffer = proxy->get_frame_buffer();
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


//-----------------------------------------------------------------------------
// setup_rgb_15
//-----------------------------------------------------------------------------
void NeXTDriver2D::setup_rgb_15()
    {
    DrawPixel  = DrawPixel16;
    WriteChar  = WriteChar16;
    GetPixelAt = GetPixelAt16;
    DrawSprite = DrawSprite16;
    }


//-----------------------------------------------------------------------------
// setup_rgb_32
//-----------------------------------------------------------------------------
void NeXTDriver2D::setup_rgb_32()
    {
    DrawPixel  = DrawPixel32;
    WriteChar  = WriteChar32;
    GetPixelAt = GetPixelAt32;
    DrawSprite = DrawSprite32;
    }


//-----------------------------------------------------------------------------
// Open
//-----------------------------------------------------------------------------
bool NeXTDriver2D::Open( const char* title )
    {
    bool okay = false;
    if (proxy->get_frame_buffer() == 0)
	system->Print( MSG_FATAL_ERROR, "Unsupported display depth\n" );
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
bool NeXTDriver2D::SetMouseCursor( int shape, ITextureHandle* bitmap )
    {
    return proxy->set_mouse_cursor( shape, bitmap );
    }
