#ifndef __NeXT_NeXTDelegate2D_h
#define __NeXT_NeXTDelegate2D_h
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
// NeXTDelegate2D.h
//
//	Objective-C component of the AppKit-based 2D driver for Crystal Space.
//	This file contains methods which are specific to NextStep.  See
//	NeXTDriver2D.h for driver code which is shared between MacOS/X,
//	MacOS/X Server 1.0 (Rhapsody), OpenStep, and NextStep.
//
//-----------------------------------------------------------------------------
#include "ivideo/cursor.h"

#if !defined(__cplusplus)

#import <objc/Object.h>
#import <dpsclient/event.h>
#include "NeXTDriver2D.h"
@class NeXTView, View, Window;

@interface NeXTDelegate2D : Object
{
  NeXTDriver2D driver;
  Window* window;
  NeXTView* view;
  BOOL trackingMouse;
  BOOL hideMouse;
  char* plainTitle;
  char* pausedTitle;
}

- (id)initWithDriver:(NeXTDriver2D)driver;
- (id)free;
- (void)dispatchEvent:(NXEvent*)p forView:(View*)v;

- (int)bestBitsPerSample;
- (BOOL)openWindow:(char const*)title width:(int)width height:(int)height
    frameBuffer:(unsigned char*)frameBuffer bitsPerSample:(int)bitsPerSample;
- (void)closeWindow;
- (void)flush;
- (void)focusChanged:(BOOL)flag;
- (BOOL)setMouseCursor:(csMouseCursorID)shape;

@end

#else // __cplusplus

#define N2D_PROTO(RET,FUNC) extern "C" RET NeXTDelegate2D_##FUNC

typedef void* NeXTDriverHandle2D;
typedef void* NeXTDelegate2D;

N2D_PROTO(NeXTDelegate2D,new)(NeXTDriverHandle2D);
N2D_PROTO(void,dispose)(NeXTDelegate2D);
N2D_PROTO(int,best_bits_per_sample)(NeXTDelegate2D);
N2D_PROTO(int,open_window)(NeXTDelegate2D, char const* title, int width,
    int height, unsigned char* frame_buffer, int bits_per_sample);
N2D_PROTO(void,close_window)(NeXTDelegate2D);
N2D_PROTO(void,flush)(NeXTDelegate2D);
N2D_PROTO(void,focus_changed)(NeXTDelegate2D, int focused);
N2D_PROTO(int,set_mouse_cursor)(NeXTDelegate2D, csMouseCursorID);

#undef N2D_PROTO

#endif // __cplusplus

#endif // __NeXT_NeXTDelegate2D_h
