//=============================================================================
//
//	Copyright (C)1999-2001 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// NeXTDelegate2D.m
//
//	Objective-C component of the AppKit-based 2D driver for Crystal Space.
//	This file contains methods which are specific to MacOS/X, MacOS/X
//	Server 1.0 (Rhapsody), and OpenStep.  See NeXTDriver2D.cpp for driver
//	code which is shared between MacOS/X, MacOS/X Server 1.0 (Rhapsody),
//	OpenStep, and NextStep.
//
//-----------------------------------------------------------------------------
#include "NeXTDelegate2D.h"
#include "NeXTView.h"
#import <AppKit/NSApplication.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSWindow.h>
#import <Foundation/NSString.h>

typedef void* NeXTDelegateHandle2D;
#define N2D_PROTO(RET,FUNC) RET NeXTDelegate2D_##FUNC

@implementation NeXTDelegate2D

//-----------------------------------------------------------------------------
// best_bits_per_sample
//	Determine the ideal number of bits per sample for the default display
//	depth.  All display depths are supported, though optimizations only
//	work for 12-bit RGB and 24-bit RGB.  Consequently this function only
//	reports 4 or 8 bits per sample, representing 12-bit and 24-bit depths,
//	respectively.  Other depths still work, but more slowly.  See
//	CS/docs/texinfo/internal/platform/next.txi for details.
//-----------------------------------------------------------------------------
- (int)bestBitsPerSample
{
  NSWindowDepth const depth = [NSWindow defaultDepthLimit];
  NSString* const s = NSColorSpaceFromDepth(depth);
  if ([s isEqualToString:NSCalibratedRGBColorSpace] ||
      [s isEqualToString:NSDeviceRGBColorSpace])
  {
    int const bps = NSBitsPerSampleFromDepth(depth);
    if (bps == 4 || bps == 8)
      return bps;
  }
  return 4;
}

N2D_PROTO(int,best_bits_per_sample)(NeXTDelegateHandle2D handle)
  { return [(NeXTDelegate2D*)handle bestBitsPerSample]; }


//-----------------------------------------------------------------------------
// releaseTitles
//-----------------------------------------------------------------------------
- (void)releaseTitles
{
  [plainTitle  release];
  [pausedTitle release];
}


//-----------------------------------------------------------------------------
// configureTitles
//-----------------------------------------------------------------------------
- (void)configureTitles:(char const*)title
{
  [self releaseTitles];
  plainTitle  = [[NSString stringWithCString:title] retain];
  pausedTitle = [[plainTitle stringByAppendingString:@"  [Paused]"] retain];
}


//-----------------------------------------------------------------------------
// adjustTitle
//-----------------------------------------------------------------------------
- (void)adjustTitle
{
  [window setTitle:(paused ? pausedTitle : plainTitle)];
}


//-----------------------------------------------------------------------------
// startTrackingMouse
//-----------------------------------------------------------------------------
- (void)startTrackingMouse
{
  if (!trackingMouse)
  {
    NSRect r = [view bounds];
    NSPoint p = [view convertPoint:
      [window mouseLocationOutsideOfEventStream] fromView:0];
    BOOL mouseInside = [view mouse:p inRect:r];
    trackingTag = [view addTrackingRect:r owner:self userData:0
      assumeInside:mouseInside];
    [window setAcceptsMouseMovedEvents:mouseInside];
    if (hideMouse && mouseInside)
      NeXTDriver2D_system_extension(driver, "hidemouse");
    trackingMouse = YES;
  }
}


//-----------------------------------------------------------------------------
// stopTrackingMouse
//-----------------------------------------------------------------------------
- (void)stopTrackingMouse
{
  if (trackingMouse)
  {
    [window setAcceptsMouseMovedEvents:NO];
    [view removeTrackingRect:trackingTag];
    NeXTDriver2D_system_extension(driver, "showmouse");
    trackingMouse = NO;
  }
}


//-----------------------------------------------------------------------------
// adjustWindowPosition:
//	For best video performance on NeXT hardware, align left-edge of
//	NeXTView (the window's contentView) at a position divisible by 8.
//
//	See also: CS/docs/texinfo/internal/platform/next.txi and the
//	NextStep 3.0 WindowServer release notes for an explanation.
//-----------------------------------------------------------------------------
- (void)adjustWindowPosition
{
  NSPoint const p = [window convertBaseToScreen:
    [view convertPoint:[view bounds].origin toView:0]];
  int const ALIGN = (1 << 3) - 1;	// 8-pixel alignment
  int x = (int)p.x;
  if ((x & ALIGN) != 0)
  {
    NSRect const wr = [window frame];
    float const dx = p.x - wr.origin.x;
    x &= ~ALIGN;
    [window setFrameOrigin:NSMakePoint(x - dx, wr.origin.y)];
  }
}


//-----------------------------------------------------------------------------
// openWindow:frameBuffer:bitsPerSample:
//	Opens a titled window and installs a NeXTView as its contentView.  Sets
//	up a tracking rectangle aroundt the NeXTView to capture the mouse when
//	it enters the view's area so that mouse-moved events can be enabled.
//	Passes the raw frame-buffer along to the NeXTView which blits it
//	directly to the WindowServer via NSBitmapImageRep.
//
// *NOTE*
//	Window must have valid WindowServer context (called a `windowNum' in
//	AppKit parlance) before setting up the tracking rectangle.  Therefore,
//	the window must be on-screen before doing so.  The alternative approach
//	of using a non-deferred window to ensure a valid `windowNum' does not
//	seem to be an option since, for some inexplicable reason, the contents
//	of a Retained non-deferred window are never drawn.
//-----------------------------------------------------------------------------
- (BOOL)openWindow:(char const*)title width:(int)width height:(int)height
  frameBuffer:(unsigned char*)frameBuffer bitsPerSample:(int)bitsPerSample
{
  NSRect const r = {{ 0, 0 }, { width, height }};
  window = [[NSWindow alloc]
    initWithContentRect:r
    styleMask:(NSTitledWindowMask | NSClosableWindowMask)
    backing:NSBackingStoreRetained
    defer:YES];

  view = [[NeXTView alloc] initWithFrame:r];
  [view setFrameBuffer:frameBuffer bitsPerSample: bitsPerSample];
  [window setContentView:view];

  [self configureTitles:title];
  [self adjustTitle];
  [window setReleasedWhenClosed:NO];
  [window center];
  [self adjustWindowPosition];
  [window setDelegate:self];
  [window makeFirstResponder:view];
  [window makeKeyAndOrderFront:0];

  [self startTrackingMouse];	// *NOTE*
  return YES;
}

N2D_PROTO(int,open_window)(NeXTDelegateHandle2D handle, char const* title,
  int w, int h, unsigned char* frame_buffer, int bits_per_sample)
  { return [(NeXTDelegate2D*)handle openWindow:title width:w height:h
  frameBuffer:frame_buffer bitsPerSample:bits_per_sample]; }


//-----------------------------------------------------------------------------
// closeWindow
//-----------------------------------------------------------------------------
- (void)closeWindow
{
  [self stopTrackingMouse];
  [window close];
}

N2D_PROTO(void,close_window)(NeXTDelegateHandle2D handle)
  { [(NeXTDelegate2D*)handle closeWindow]; }


//-----------------------------------------------------------------------------
// setWindowTitle:
//-----------------------------------------------------------------------------
- (void)setWindowTitle:(char const*)title
{
  [self configureTitles:title];
  [self adjustTitle];
}

N2D_PROTO(void,set_window_title)(NeXTDelegateHandle2D handle, char const* s)
  { [(NeXTDelegate2D*)handle setWindowTitle:s]; }


//-----------------------------------------------------------------------------
// focusChanged
//-----------------------------------------------------------------------------
- (void)focusChanged:(BOOL)focused
{
  paused = !focused;
  [self adjustTitle];
  if (paused)
    [self stopTrackingMouse];
  else
    [self startTrackingMouse];
}

N2D_PROTO(void,focus_changed)(NeXTDelegateHandle2D handle, int focused)
  { [(NeXTDelegate2D*)handle focusChanged:(BOOL)focused]; }


//-----------------------------------------------------------------------------
// flush
//-----------------------------------------------------------------------------
- (void)flush
{
  [view flush];
  NeXTDriver2D_system_extension(driver, "flushgraphicscontext");
}

N2D_PROTO(void,flush)(NeXTDelegateHandle2D handle)
  { [(NeXTDelegate2D*)handle flush]; }


//-----------------------------------------------------------------------------
// setMouseCursor:
//-----------------------------------------------------------------------------
- (BOOL)setMouseCursor:(csMouseCursorID)shape
{
  hideMouse = YES;
  if (shape == csmcArrow)
  {
    [[NSCursor arrowCursor] set];
    hideMouse = NO;
  }
  
  if (hideMouse)
    NeXTDriver2D_system_extension(driver, "hidemouse");
  else
    NeXTDriver2D_system_extension(driver, "showmouse");
  return !hideMouse;
}

N2D_PROTO(int,set_mouse_cursor)
  (NeXTDelegateHandle2D handle, csMouseCursorID shape)
  { return [(NeXTDelegate2D*)handle setMouseCursor:shape]; }


//-----------------------------------------------------------------------------
// mouseEntered:
//-----------------------------------------------------------------------------
- (void)mouseEntered:(NSEvent*)p 
{
  if ([p trackingNumber] == trackingTag)
  {
    [window setAcceptsMouseMovedEvents:YES];
    if (hideMouse)
      NeXTDriver2D_system_extension(driver, "hidemouse");
  }
}


//-----------------------------------------------------------------------------
// mouseExited:
//-----------------------------------------------------------------------------
- (void)mouseExited:(NSEvent*)p 
{
  if ([p trackingNumber] == trackingTag)
  {
    [window setAcceptsMouseMovedEvents:NO];
    NeXTDriver2D_system_extension(driver, "showmouse");
  }
}


//-----------------------------------------------------------------------------
// windowDidBecomeKey:
//-----------------------------------------------------------------------------
- (void)windowDidBecomeKey:(NSNotification*)n
{
  [self startTrackingMouse];
}


//-----------------------------------------------------------------------------
// windowDidResignKey:
//-----------------------------------------------------------------------------
- (void)windowDidResignKey:(NSNotification*)n
{
  [self stopTrackingMouse];
}


//-----------------------------------------------------------------------------
// windowDidMove:
//-----------------------------------------------------------------------------
- (void)windowDidMove:(NSNotification*)n
{
  [self adjustWindowPosition];
}


//-----------------------------------------------------------------------------
// windowShouldClose:
//	Terminate the application when the animation-window closes.
//-----------------------------------------------------------------------------
- (BOOL)windowShouldClose:(id)sender
{
  [self stopTrackingMouse];
  if (hideMouse)
    NeXTDriver2D_system_extension(driver, "showmouse");
  NeXTDriver2D_user_close(driver);
  return YES;
}


//-----------------------------------------------------------------------------
// dispatchEvent:forView:
//-----------------------------------------------------------------------------
- (void)dispatchEvent:(NSEvent*)e forView:(NSView*)v
{
  NeXTDriver2D_system_extension(driver, "dispatchevent", e, v);
}


//-----------------------------------------------------------------------------
// initWithDriver:
//-----------------------------------------------------------------------------
- (id)initWithDriver:(NeXTDriver2D)p
{
  [super init];
  driver = p;
  window = 0;
  view = 0;
  trackingTag = 0;
  trackingMouse = NO;
  hideMouse = NO;
  paused = NO;
  plainTitle = 0;
  pausedTitle = 0;
  return self;
}

N2D_PROTO(NeXTDelegateHandle2D,new)(NeXTDriver2D driver)
  { return [[NeXTDelegate2D alloc] initWithDriver:driver]; }


//-----------------------------------------------------------------------------
// dealloc
//-----------------------------------------------------------------------------
- (void)dealloc
{
  [window setDelegate:0];
  [window close];
  [window release]; // NSWindow releases NeXTView.
  [self releaseTitles];
  [super dealloc];
}

N2D_PROTO(void,dispose)(NeXTDelegateHandle2D handle)
  { [(NeXTDelegate2D*)handle release]; }

@end

#undef N2D_PROTO
