//=============================================================================
//
//	Copyright (C)1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
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
//	functionality via the AppKit.  This file contains methods which are
//	shared between the MacOS/X Server, OpenStep, and NextStep platforms.
//	See NeXTLocal2D.cpp for platform-specific implementation.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "NeXTDriver2D.h"
#include "NeXTFrameBuffer15.h"
#include "NeXTFrameBuffer32.h"
#include "icfgfile.h"
#include "ievent.h"
#include "isystem.h"
#include "version.h"

IMPLEMENT_FACTORY(NeXTDriver2D)

EXPORT_CLASS_TABLE(next2d)
    EXPORT_CLASS_DEP(NeXTDriver2D, "crystalspace.graphics2d.next",
	"Crystal Space 2D driver for MacOS/X Server, OpenStep, and NextStep",
        "crystalspace.font.server.")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE(NeXTDriver2D)
    IMPLEMENTS_INTERFACE(iPlugIn)
    IMPLEMENTS_INTERFACE(iGraphics2D)
IMPLEMENT_IBASE_END

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
NeXTDriver2D::NeXTDriver2D( iBase* p ) :
    csGraphics2D(), initialized(false), frame_buffer(0), view(0)
    {
    CONSTRUCT_IBASE(p);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTDriver2D::~NeXTDriver2D()
    {
    if (initialized)
	shutdown_driver();
    if (frame_buffer != 0)
	delete frame_buffer;
    Memory = 0;
    }


//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
bool NeXTDriver2D::Initialize( iSystem* s )
    {
    bool ok = superclass::Initialize(s);
    if (ok)
	{
	initialized = true;
	ok = init_driver( get_desired_depth() );
	}
    return ok;
    }


//-----------------------------------------------------------------------------
// init_driver
//-----------------------------------------------------------------------------
bool NeXTDriver2D::init_driver( int desired_depth )
    {
    bool ok = true;
    switch (determine_bits_per_sample( desired_depth ))
	{
	case 4: frame_buffer = new NeXTFrameBuffer15( Width, Height ); break;
	case 8: frame_buffer = new NeXTFrameBuffer32( Width, Height ); break;
	default: ok = false; break;
	}

    if (ok)
	{
	Depth  = frame_buffer->depth();
	Memory = frame_buffer->get_raw_buffer();
	pfmt.RedMask    = frame_buffer->red_mask();
	pfmt.GreenMask  = frame_buffer->green_mask();
	pfmt.BlueMask   = frame_buffer->blue_mask();
	pfmt.PixelBytes = frame_buffer->bytes_per_pixel();
	pfmt.PalEntries = frame_buffer->palette_entries();
	pfmt.complete ();
	switch (Depth)
	    {
	    case 15: setup_rgb_15(); break;
	    case 32: setup_rgb_32(); break;
	    }
	}
    else
	System->Printf( MSG_FATAL_ERROR, "FATAL: Bizarre internal error; "
		"support for 15- and 32-bit RGB only\n" );
    return ok;
    }


//-----------------------------------------------------------------------------
// get_desired_depth()
//	Check if default device depth setting is overriden with a command-line
//	switch or via a configuration file setting.
//-----------------------------------------------------------------------------
int NeXTDriver2D::get_desired_depth() const
    {
    int depth = 0;
    char const* s = System->GetOptionCL( "simdepth" );
    if (s != 0)
	depth = atoi(s);
    else
	{
	iConfigFile* cfg = System->GetConfig();
	if (cfg != 0)
	    depth = cfg->GetInt( "VideoDriver", "SimulateDepth", 0 );
	}
    return depth;
    }


//-----------------------------------------------------------------------------
// determine_bits_per_sample
//	Only depths of 15- and 32-bit are supported.  16-bit is treated as an
//	alias for 15-bit.  See CS/docs/texinfo/internal/platform/next.txi for
//	complete details concerning this limitation.
//-----------------------------------------------------------------------------
int NeXTDriver2D::determine_bits_per_sample( int desired_depth )
    {
    int bps;
    switch (desired_depth)
	{
	case 15: bps = 4; break;
	case 16: bps = 4; break; // An alias for 15-bit.
	case 32: bps = 8; break;
	default: bps = best_bits_per_sample(); break;
	}
    return bps;
    }


//-----------------------------------------------------------------------------
// setup_rgb_15
//-----------------------------------------------------------------------------
void NeXTDriver2D::setup_rgb_15()
    {
    _DrawPixel = DrawPixel16;
    _WriteString = WriteString16;
    _GetPixelAt = GetPixelAt16;
    }


//-----------------------------------------------------------------------------
// setup_rgb_32
//-----------------------------------------------------------------------------
void NeXTDriver2D::setup_rgb_32()
    {
    _DrawPixel = DrawPixel32;
    _WriteString = WriteString32;
    _GetPixelAt = GetPixelAt32;
    }


//-----------------------------------------------------------------------------
// Open
//-----------------------------------------------------------------------------
bool NeXTDriver2D::Open( char const* title )
    {
    System->Printf( MSG_INITIALIZATION, CS_PLATFORM_NAME
	" 2D graphics driver for Crystal Space " CS_VERSION_NUMBER "\n"
	"Written by Eric Sunshine <sunshine@sunshineco.com>\n\n" );
    bool okay = false;
    if (superclass::Open( title ))
	okay = open_window( title );
    return okay;
    }


//-----------------------------------------------------------------------------
// Print -- A misnomer; actually flushes frame buffer to display.
//-----------------------------------------------------------------------------
void NeXTDriver2D::Print( csRect* )
    {
    frame_buffer->cook();
    flush();
    }


//-----------------------------------------------------------------------------
// HandleEvent
//-----------------------------------------------------------------------------
bool NeXTDriver2D::HandleEvent( iEvent& e )
    {
    bool rc = false;
    if (System != 0 &&
	e.Type == csevBroadcast && e.Command.Code == cscmdCommandLineHelp)
	{
	System->Printf( MSG_STDOUT,
	    "Options for " CS_PLATFORM_NAME " 2D graphics driver:\n"
	    "  -simdepth=<depth>  "
		"simulate depth (15, 16, or 32) (default=none)\n" );
	rc = true;
	}
    return rc;
    }
