#ifndef __Cocoa_CocoaDelegate2D_h
#define __Cocoa_CocoaDelegate2D_h
//=============================================================================
//
//	Copyright (C)1999-2002 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// CocoaDelegate2D.h
//
//	Objective-C component of the AppKit-based 2D driver for Crystal Space.
//
//-----------------------------------------------------------------------------
#include "ivideo/cursor.h"

#if !defined(__cplusplus)

#import <Foundation/NSObject.h>
#import <AppKit/NSView.h>	// NSTrackingRectTag
#include "CocoaDriver2D.h"
@class CocoaView, NSEvent, NSView, NSWindow;

@interface CocoaDelegate2D : NSObject
{
  CocoaDriver2D driver;
  NSWindow* window;
  CocoaView* view;
  NSTrackingRectTag trackingTag;
  BOOL trackingMouse;
  BOOL hideMouse;
  BOOL paused;
  NSString* plainTitle;
  NSString* pausedTitle;
}

- (id)initWithDriver:(CocoaDriver2D)driver;
- (void)dealloc;
- (void)dispatchEvent:(NSEvent*)p forView:(NSView*)v;

- (int)bestBitsPerSample;
- (BOOL)openWindow:(char const*)title width:(int)width height:(int)height
    frameBuffer:(unsigned char*)frameBuffer bitsPerSample:(int)bitsPerSample;
- (void)closeWindow;
- (void)setWindowTitle:(char const*)title;
- (void)flush;
- (void)focusChanged:(BOOL)flag shouldPause:(BOOL) pause;
- (BOOL)setMouseCursor:(csMouseCursorID)shape;

@end

#else // __cplusplus

#define N2D_PROTO(RET,FUNC) extern "C" RET CocoaDelegate2D_##FUNC

typedef void* CocoaDriverHandle2D;
typedef void* CocoaDelegate2D;

N2D_PROTO(CocoaDelegate2D,new)(CocoaDriverHandle2D);
N2D_PROTO(void,dispose)(CocoaDelegate2D);
N2D_PROTO(int,best_bits_per_sample)(CocoaDelegate2D);
N2D_PROTO(int,open_window)(CocoaDelegate2D, char const* title, int width,
    int height, unsigned char* frame_buffer, int bits_per_sample);
N2D_PROTO(void,close_window)(CocoaDelegate2D);
N2D_PROTO(void,set_window_title)(CocoaDelegate2D, char const* title);
N2D_PROTO(void,flush)(CocoaDelegate2D);
N2D_PROTO(void,focus_changed)(CocoaDelegate2D, int focused, int shouldPause);
N2D_PROTO(int,set_mouse_cursor)(CocoaDelegate2D, csMouseCursorID);

#undef N2D_PROTO

#endif // __cplusplus

#endif // __Cocoa_CocoaDelegate2D_h
