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
// NeXTProxy2D.cpp
//
//	C++ object which interacts with Objective-C world on behalf of
//	NeXTDriver2D which can not directly interface with Objective-C on
//	account of COM-related conflicts.  See README.NeXT for details.
//
// *WARNING* Do NOT include any COM headers in this file.
//-----------------------------------------------------------------------------
#include "driver2d/NeXTProxy2D.h"
#include "driver2d/NeXTView.h"
#include "NeXTDelegate.h"

extern "Objective-C" {
#import <appkit/Application.h>
#import <appkit/NXBitmapImageRep.h>
#import <appkit/NXCursor.h>
#import <appkit/Window.h>
}
extern "C" {
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach.h>
}

//-----------------------------------------------------------------------------
// vm_roundup -- Round 'size' up to a multiple of vm_page_size.
//-----------------------------------------------------------------------------
static unsigned int vm_roundup( unsigned int size )
    {
    if (size % vm_page_size != 0)
	size = ((unsigned int)(size / vm_page_size) + 1) * vm_page_size;
    return size;
    }


//-----------------------------------------------------------------------------
// allocate_frame_buffer
//	Allocation via vm_allocate() is guaranteed to be page-aligned which
//	is required for best video optimization.  See README.NeXT for details.
//-----------------------------------------------------------------------------
static unsigned char* allocate_frame_buffer( unsigned int nbytes )
    {
    unsigned char* p = 0;
    vm_allocate( task_self(), (vm_address_t*)&p, nbytes, TRUE );
    assert( p != 0 );
    return p;
    }


//-----------------------------------------------------------------------------
// free_frame_buffer
//-----------------------------------------------------------------------------
static void free_frame_buffer( void* p, unsigned int nbytes )
    {
    vm_deallocate( task_self(), (vm_address_t)p, nbytes );
    }


//-----------------------------------------------------------------------------
// best_bits_per_sample
//	Determine the ideal number of bits per sample for the default display
//	depth.  All display depths are supported, though optimizations only
//	work for 12-bit RGB and 24-bit RGB.  Consequently this function only
//	reports 4 or 8 bits per sample, representing 12-bit and 24-bit depths,
//	respectively.  Other depths still work, but more slowly.  See
//	README.NeXT for details.
//-----------------------------------------------------------------------------
static int best_bits_per_sample()
    {
    return ([NXApp mainScreen]->depth == NX_TwentyFourBitRGBDepth ? 8 : 4);
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
NeXTProxy2D::NeXTProxy2D( unsigned int w, unsigned int h ) :
    window(0), view(0), width(w), height(h)
    {
    bits_per_sample = best_bits_per_sample();

    int const bytes_per_pixel = bits_per_sample / 2;
    frame_buffer_size = vm_roundup( bytes_per_pixel * width * height );
    frame_buffer = allocate_frame_buffer( frame_buffer_size );
    memset( frame_buffer, 0xff, frame_buffer_size );

    unsigned int const psize = 256 * bytes_per_pixel;
    palette = (unsigned char*)malloc( psize );
    memset( palette, 0xff, psize );	// 0xff == Opaque; see README.NeXT.
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTProxy2D::~NeXTProxy2D()
    {
    [[NXApp delegate] showMouse];
    [[NXApp delegate] registerAnimationWindow:0];
    [window setDelegate:0];
    [window free];	// Window frees NeXTView.
    free( palette );
    free_frame_buffer( frame_buffer, frame_buffer_size );
    }


//-----------------------------------------------------------------------------
// open
//	Opens a titled Window and installs a NeXTView as its contentView.
//	Registers the window with the application's delegate as its "animation
//	window".  Passes the raw frame-buffer along to the NeXTView which blits
//	it directly to the WindowServer via NXBitmapImageRep.
//
// *NOTE*
//	Window must have valid PostScript windowNum before registering with 
//	application's delegate since a tracking rectangle is set up.  
//	Therefore window must be on-screen before registering the window.  The 
//	alternative of using a non-deferred window does not seem to be an 
//	option since, for some inexplicable reason, the contents of a Retained 
//	non-deferred window are never drawn.  
//-----------------------------------------------------------------------------
bool NeXTProxy2D::open( char const* title )
    {
    NXRect const r = {{ 0, 0 }, { width, height }};
    window = [[Window alloc]
	initContent:&r
	style:NX_TITLEDSTYLE
	backing:NX_RETAINED
	buttonMask:NX_CLOSEBUTTONMASK
	defer:YES];
    [window setTitle:title];
    [window setFreeWhenClosed:NO];

    view = [[NeXTView alloc] initFrame:&r];
    [view setFrameBuffer:frame_buffer bitsPerSample:bits_per_sample];
    [[window setContentView:view] free];

    NXSize s; [NXApp getScreenSize:&s];	// Center window on-screen.
    NXRect frame; [window getFrame:&frame];
    [window moveTo:int((s.width  - frame.size.width)  / 2) :
		   int((s.height - frame.size.height) / 2)];

    [window makeFirstResponder:view];
    [window makeKeyAndOrderFront:0];
    [[NXApp delegate] registerAnimationWindow:window];	// *NOTE*
    return true;
    }


//-----------------------------------------------------------------------------
// close
//-----------------------------------------------------------------------------
void NeXTProxy2D::close()
    {
    [window close];
    }


//-----------------------------------------------------------------------------
// flush_12
//-----------------------------------------------------------------------------
void NeXTProxy2D::flush_12( unsigned char const* src )
    {
    unsigned short* dst = (unsigned short*)frame_buffer;
    unsigned short* pal = (unsigned short*)palette;
    for (unsigned int n = width * height; n-- > 0; )
	*dst++ = pal[ *src++ ];
    }


//-----------------------------------------------------------------------------
// flush_24
//-----------------------------------------------------------------------------
void NeXTProxy2D::flush_24( unsigned char const* src )
    {
    unsigned long* dst = (unsigned long*)frame_buffer;
    unsigned long* pal = (unsigned long*)palette;
    for (unsigned int n = width * height; n-- > 0; )
	*dst++ = pal[ *src++ ];
    }


//-----------------------------------------------------------------------------
// flush
//	Convert the palettized CrystalSpace frame-buffer into NeXT format
//	"true color" data which includes an alpha-channel, then blit the image
//	to the display.
//-----------------------------------------------------------------------------
void NeXTProxy2D::flush( unsigned char const* src )
    {
    assert( bits_per_sample == 4 || bits_per_sample == 8 );
    if (bits_per_sample == 4)
	flush_12( src );
    else // (bits_per_sample == 8)
	flush_24( src );
    [view flush];
    DPSFlush();
    }


//-----------------------------------------------------------------------------
// set_rgb
//	Video optimizations require that frame-buffer sent to WindowServer
//	have an alpha-channel (always opaque), so we build a palette, parallel
//	to the CrystalSpace palette, which includes an alpha-channel.  See
//	README.NeXT for a full discussion.
//-----------------------------------------------------------------------------
void NeXTProxy2D::set_rgb( int i, int r, int g, int b )
    {
    assert( bits_per_sample == 4 || bits_per_sample == 8 );
    int const bytes_per_pixel = bits_per_sample / 2;
    unsigned char* p = palette + i * bytes_per_pixel;
    if (bits_per_sample == 4)
	{
	*p++ = (r & 0xf0) | (g >> 4);
	*p++ = (b & 0xf0) | 0x0f;	// 0x0f == opaque
	}
    else // (bits_per_sample == 8)
	{
	*p++ = r;
	*p++ = g;
	*p++ = b;
	*p++ = 0xff;			// 0xff == opaque
	}
    }


//-----------------------------------------------------------------------------
// set_mouse_cursor
//-----------------------------------------------------------------------------
bool NeXTProxy2D::set_mouse_cursor( int shape, IMipMapContainer* )
    {
    bool handled = false;
    if (shape == 0)	// 0 == csmcArrow  (Can not include COM header.)
	{
	[NXArrow set];
	handled = true;
	}

    if (handled)
	[[NXApp delegate] showMouse];
    else
	[[NXApp delegate] hideMouse];
    return handled;
    }
