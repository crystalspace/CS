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
// NeXTLocal2D.cpp
//
//	NeXT-specific subclass of csGraphics2D which implements 2D-graphic
//	functionality via the AppKit.  This file contains methods which are
//	shared between the MacOS/X Server and OpenStep platforms.  See
//	NeXTDriver2D.cpp for methods which are shared between MacOS/X Server,
//	OpenStep, and NextStep.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "NeXTDriver2D.h"
#include "NeXTDelegate.h"
#include "NeXTFrameBuffer.h"
#include "NeXTView.h"

extern "Objective-C" {
#import <AppKit/NSApplication.h>
#import <AppKit/NSBitmapImageRep.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSDPSContext.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSWindow.h>
}

//-----------------------------------------------------------------------------
// window_server_depth
//	Directly query the Window Server for its preferred depth.  Note that
//	this value may be different from the depth limit reported by the
//	NSWindow class.  See get_device_depth() for a full discussion.  The
//	Window Server is queried for its preferred depth by creating a small
//	image cache which is painted with color so as to promote it from gray
//	to color.  The Window Server is then asked to return an
//	NSBitmapImageRep holding the contents of the image cache.  The Window
//	Server always returns the NSBitmapImageRep in the format which it most
//	prefers.
//-----------------------------------------------------------------------------
static NSWindowDepth window_server_depth()
    {
    NSRect const r = {{ 0, 0 }, { 4, 4 }};
    NSImage* image = [[NSImage alloc] initWithSize:r.size];
    [image lockFocus];
    [[NSColor blueColor] set];
    NSRectFill(r);
    NSBitmapImageRep* p = [[NSBitmapImageRep alloc] initWithFocusedViewRect:r];
    NSWindowDepth depth = NSBestDepth( [p colorSpaceName], [p bitsPerSample],
	[p bitsPerPixel], [p isPlanar], 0 );
    [p release];
    [image unlockFocus];
    [image release];
    return depth;
    }


//-----------------------------------------------------------------------------
// best_bits_per_sample
//	Determine the ideal number of bits per sample for the default display
//	depth.  All display depths are supported, though optimizations only
//	work for 12-bit RGB and 24-bit RGB.  Consequently this function only
//	reports 4 or 8 bits per sample, representing 12-bit and 24-bit depths,
//	respectively.  Other depths still work, but more slowly.  See
//	CS/docs/texinfo/internal/platform/next.txi for details.
//
//	Note that as of OpenStep 4.1, the Window Server may prefer a depth
//	greater than that of the "deepest" screen as reported by the Window
//	class.  The reason for this is that "true" RGB/5:5:5 was implemented
//	in OpenStep 4.1.  Previously this had been simulated with 4:4:4.  A
//	consequence of this change is that for 5:5:5 displays, the Window
//	Server actually prefers 24-bit rather than 12-bit RGB as was the case
//	with previous versions.  It is important to note that the NSWindow
//	class still reports a depth limit of 12-bit even though the Window
//	Server prefers 24-bit.  Consequently, window_server_depth() is used to
//	directly query the WindowServer for its preference instead.  The
//	behavior in the OpenStep Window Server impacts all applications,
//	including those compiled with earlier versions of OpenStep or
//	NextStep.
//-----------------------------------------------------------------------------
int NeXTDriver2D::best_bits_per_sample()
    {
    NSWindowDepth const depth = window_server_depth();
    NSString* const s = NSColorSpaceFromDepth( depth );
    if ([s isEqualToString:NSCalibratedRGBColorSpace] ||
	[s isEqualToString:NSDeviceRGBColorSpace])
        {
	int const bps = NSBitsPerSampleFromDepth( depth );
	if (bps == 4 || bps == 8)
	    return bps;
	}
    return 4;
    }


//-----------------------------------------------------------------------------
// shutdown_driver
//-----------------------------------------------------------------------------
void NeXTDriver2D::shutdown_driver()
    {
    [[NSApp delegate] showMouse];
    [[NSApp delegate] registerAnimationWindow:0];
    NSWindow* window = [view window];
    [window setDelegate:0];
    [window close];
    [window release];	// Window releases NeXTView.
    }


//-----------------------------------------------------------------------------
// open_window
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
bool NeXTDriver2D::open_window( char const* title )
    {
    NSRect const r = {{ 0, 0 }, { Width, Height }};
    NSWindow* window = [[NSWindow alloc]
	initWithContentRect:r
	styleMask:(NSTitledWindowMask | NSClosableWindowMask)
	backing:NSBackingStoreRetained
	defer:YES];
    [window setTitle:[NSString stringWithCString:title]];
    [window setReleasedWhenClosed:NO];

    view = [[NeXTView alloc] initWithFrame:r];
    [view setFrameBuffer:frame_buffer->get_cooked_buffer()
	bitsPerSample:frame_buffer->bits_per_sample()];
    [window setContentView:view];

    [window center];
    [window makeFirstResponder:view];
    [window makeKeyAndOrderFront:0];
    [[NSApp delegate] registerAnimationWindow:window];	// *NOTE*
    return true;
    }


//-----------------------------------------------------------------------------
// Close
//-----------------------------------------------------------------------------
void NeXTDriver2D::Close()
    {
    [[view window] close];
    superclass::Close();
    }


//-----------------------------------------------------------------------------
// flush
//-----------------------------------------------------------------------------
void NeXTDriver2D::flush()
    {
    [view flush];
    [[NSDPSContext currentContext] flush];
    }


//-----------------------------------------------------------------------------
// SetMouseCursor
//-----------------------------------------------------------------------------
bool NeXTDriver2D::SetMouseCursor( csMouseCursorID shape )
    {
    bool handled = false;
    if (shape == csmcArrow)
	{
	[[NSCursor arrowCursor] set];
	handled = true;
	}

    if (handled)
	[[NSApp delegate] showMouse];
    else
	[[NSApp delegate] hideMouse];
    return handled;
    }
