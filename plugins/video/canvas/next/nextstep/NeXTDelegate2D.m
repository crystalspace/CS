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
//	This file contains methods which are specific to NextStep.  See
//	NeXTDriver2D.cpp for driver code which is shared between MacOS/X,
//	MacOS/X Server 1.0 (Rhapsody), OpenStep, and NextStep.
//
//-----------------------------------------------------------------------------
#include "NeXTDelegate2D.h"
#include "NeXTView.h"
#include <string.h>
#import <appkit/Application.h>
#import <appkit/NXCursor.h>
#import <appkit/NXImage.h>
#import <appkit/Window.h>

typedef void* NeXTDelegateHandle2D;
#define N2D_PROTO(RET,FUNC) RET NeXTDelegate2D_##FUNC

#define TRACK_TAG 9797

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
  NXWindowDepth const depth = [Window defaultDepthLimit];
  if (NXColorSpaceFromDepth(depth) == NX_RGBColorSpace)
  {
    int const bps = NXBPSFromDepth(depth);
    if (bps == 4 || bps == 8)
      return bps;
  }
  return 4;
}

N2D_PROTO(int,best_bits_per_sample)(NeXTDelegateHandle2D handle)
  { return [(NeXTDelegate2D*)handle bestBitsPerSample]; }


//-----------------------------------------------------------------------------
// copyString:suffix:
//-----------------------------------------------------------------------------
- (char*)copyString:(char const*)s suffix:(char const*)x
{
  char* p = (char*)malloc(strlen(s) + (x == 0 ? 0 : strlen(x)) + 1);
  strcpy(p, s);
  if (x != 0)
    strcat(p, x);
  return p;
}


//-----------------------------------------------------------------------------
// freeTitles
//-----------------------------------------------------------------------------
- (void)freeTitles
{
  if (plainTitle != 0)
  {
    free(plainTitle ); plainTitle  = 0;
    free(pausedTitle); pausedTitle = 0;
  }
}


//-----------------------------------------------------------------------------
// configureTitles
//-----------------------------------------------------------------------------
- (void)configureTitles:(char const*)title
{
  [self freeTitles];
  plainTitle  = [self copyString:title suffix:0];
  pausedTitle = [self copyString:title suffix:"  [Paused]"];
}


//-----------------------------------------------------------------------------
// adjustTitle
//-----------------------------------------------------------------------------
- (void)adjustTitle
{
  [window setTitle:(paused ? pausedTitle : plainTitle)];
}


//-----------------------------------------------------------------------------
// enableMouseMoved:
//-----------------------------------------------------------------------------
- (void)enableMouseMoved:(BOOL)enable
{
  if (enable)
    [window addToEventMask:NX_MOUSEMOVEDMASK];
  else
    [window removeFromEventMask:NX_MOUSEMOVEDMASK];
}


//-----------------------------------------------------------------------------
// startTrackingMouse
//-----------------------------------------------------------------------------
- (void)startTrackingMouse
{
  if (!trackingMouse)
  {
    NXPoint p;
    NXRect r;
    BOOL mouseInside;
    [window getMouseLocation:&p];
    [view getBounds:&r];
    [view convertRect:&r toView:0];
    mouseInside = [view mouse:&p inRect:&r];
    [window setTrackingRect:&r inside:mouseInside owner:self
      tag:TRACK_TAG left:NO right:NO];
    [self enableMouseMoved:mouseInside];
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
    [self enableMouseMoved:NO];
    [window discardTrackingRect:TRACK_TAG];
    NeXTDriver2D_system_extension(driver, "showmouse");
    trackingMouse = NO;
  }
}


//-----------------------------------------------------------------------------
// adjustWindowPosition
//	For best video performance on NeXT hardware, align left-edge of
//	NeXTView (the window's contentView) at a position divisible by 8.
//
//	See also: CS/docs/texinfo/internal/platform/next.txi and the
//	NextStep 3.0 WindowServer release notes for an explanation.
//-----------------------------------------------------------------------------
- (void)adjustWindowPosition
{
  int const ALIGN = (1 << 3) - 1; // 8-pixel alignment
  int x;
  NXRect vr;
  [view getBounds:&vr];
  [view convertPoint:&vr.origin toView:0];
  [window convertBaseToScreen:&vr.origin];
  x = (int)vr.origin.x;
  if ((x & ALIGN) != 0)
  {
    NXRect wr;
    float dx;
    x &= ~ALIGN;
    [window getFrame:&wr];
    dx = vr.origin.x - wr.origin.x;
    [window moveTo:(x - dx):wr.origin.y];
  }
}


//-----------------------------------------------------------------------------
// openWindow:frameBuffer:bitsPerSample:
//	Opens a titled window and installs a NeXTView as its contentView.
//	Modifies the window's event mask to accept flags-changed, and
//	mouse-dragged events.  Sets up a tracking rectangle around the NeXTView
//	to capture the mouse when it enters the view's area so that mouse-moved
//	events can be enabled.  Passes the raw frame-buffer along to the
//	NeXTView which blits it directly to the WindowServer via
//	NXBitmapImageRep.
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
  NXRect r = {{ 0, 0 }, { width, height }};
  window = [[Window alloc]
    initContent:&r
    style:NX_TITLEDSTYLE
    backing:NX_RETAINED
    buttonMask:NX_CLOSEBUTTONMASK
    defer:YES];
  
  view = [[NeXTView alloc] initFrame:&r];
  [view setFrameBuffer:frameBuffer bitsPerSample:bitsPerSample];
  [[window setContentView:view] free];
  
  [self configureTitles:title];
  [self adjustTitle];
  [window setFreeWhenClosed:NO];
  [window addToEventMask:
    NX_FLAGSCHANGEDMASK | NX_LMOUSEDRAGGEDMASK | NX_RMOUSEDRAGGEDMASK];
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
    [NXArrow set];
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
- (id)mouseEntered:(NXEvent*)p
{
  if (p->data.tracking.trackingNum == TRACK_TAG)
  {
    [self enableMouseMoved:YES];
    if (hideMouse)
      NeXTDriver2D_system_extension(driver, "hidemouse");
  }
  return self;
}


//-----------------------------------------------------------------------------
// mouseExited:
//-----------------------------------------------------------------------------
- (id)mouseExited:(NXEvent*)p
{
  if (p->data.tracking.trackingNum == TRACK_TAG)
  {
    [self enableMouseMoved:NO];
    NeXTDriver2D_system_extension(driver, "showmouse");
  }
  return self;
}


//-----------------------------------------------------------------------------
// windowDidBecomeKey:
//-----------------------------------------------------------------------------
- (id)windowDidBecomeKey:(id)sender
{
  [self startTrackingMouse];
  return self;
}


//-----------------------------------------------------------------------------
// windowDidResignKey:
//-----------------------------------------------------------------------------
- (id)windowDidResignKey:(id)sender
{
  [self stopTrackingMouse];
  return self;
}


//-----------------------------------------------------------------------------
// windowDidMove:
//-----------------------------------------------------------------------------
- (id)windowDidMove:(id)sender
{
  [self adjustWindowPosition];
  return self;
}


//-----------------------------------------------------------------------------
// windowWillClose:
//	Terminate the application when the animation-window closes.
//-----------------------------------------------------------------------------
- (id)windowWillClose:(id)sender
{
  [self stopTrackingMouse];
  if (hideMouse)
    NeXTDriver2D_system_extension(driver, "showmouse");
  NeXTDriver2D_user_close(driver);
  return self;
}


//-----------------------------------------------------------------------------
// dispatchEvent:forView:
//-----------------------------------------------------------------------------
- (void)dispatchEvent:(NXEvent*)e forView:(View*)v
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
// free
//-----------------------------------------------------------------------------
- (id)free
{
  [window setDelegate:0];
  [window close];
  [window free];	// Window frees NeXTView.
  [self freeTitles];
  return [super free];
}

N2D_PROTO(void,dispose)(NeXTDelegateHandle2D handle)
  { [(NeXTDelegate2D*)handle free]; }

@end

#undef TRACK_TAG
#undef N2D_PROTO
