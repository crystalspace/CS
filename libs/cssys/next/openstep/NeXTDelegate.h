#ifndef __NeXT_NeXTDelegate_h
#define __NeXT_NeXTDelegate_h
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
// NeXTDelegate.h
//
//	The application's delegate.  Acts as a gateway between the AppKit and
//	Crystal Space by forwarding Objective-C messages and events to the C++
//	system driver, NeXTSystemDriver.
//
//-----------------------------------------------------------------------------
#include "NeXTConfigFile.h"

#if !defined(__cplusplus)

#import <Foundation/NSObject.h>
#include "cssys/next/NeXTSystemDriver.h"
@class NSEvent, NSView, NSWindow;

@interface NeXTDelegate : NSObject
{
  NeXTSystemDriver driver;
  unsigned long modifiers;
  BOOL mouseHidden;
  BOOL paused;
  BOOL autoResume;
}

+ (NeXTDelegate*)startup:(NeXTSystemDriver)driver;
+ (void)shutdown:(NeXTDelegate*)controller;

- (id)initWithDriver:(NeXTSystemDriver)driver;
- (void)initApplicationMenu:(NeXTConfigHandle)handle style:(char const*)style;

- (void)pause;
- (void)unpause;
- (void)togglePause:(id)sender;

- (void)showMouse;
- (void)hideMouse;

- (void)flushGraphicsContext;

- (void)dispatchEvent:    (NSEvent*)p forView:(NSView*)v;
- (void)keyDown:          (NSEvent*)p forView:(NSView*)v;
- (void)keyUp:            (NSEvent*)p forView:(NSView*)v;
- (void)flagsChanged:     (NSEvent*)p forView:(NSView*)v;
- (void)mouseMoved:       (NSEvent*)p forView:(NSView*)v;
- (void)mouseDown:        (NSEvent*)p forView:(NSView*)v;
- (void)mouseUp:          (NSEvent*)p forView:(NSView*)v;
- (void)mouseDragged:     (NSEvent*)p forView:(NSView*)v;
- (void)rightMouseDown:   (NSEvent*)p forView:(NSView*)v;
- (void)rightMouseUp:     (NSEvent*)p forView:(NSView*)v;
- (void)rightMouseDragged:(NSEvent*)p forView:(NSView*)v;

@end

#else // __cplusplus

#define ND_PROTO(RET,FUNC) extern "C" RET NeXTDelegate_##FUNC

typedef void* NeXTSystemHandle;
typedef void* NeXTDelegate;
typedef void* NeXTEvent;
typedef void* NeXTView;

ND_PROTO(NeXTDelegate,startup)(NeXTSystemHandle);
ND_PROTO(void,shutdown)(NeXTDelegate);
ND_PROTO(void,init_app_menu)(NeXTDelegate, NeXTConfigHandle, char const*);
ND_PROTO(void,start_event_loop)(NeXTDelegate);
ND_PROTO(void,stop_event_loop)(NeXTDelegate);
ND_PROTO(void,dispatch_event)(NeXTDelegate, NeXTEvent, NeXTView);
ND_PROTO(void,hide_mouse)(NeXTDelegate);
ND_PROTO(void,show_mouse)(NeXTDelegate);
ND_PROTO(void,flush_graphics_context)(NeXTDelegate);

#undef ND_PROTO

#endif // __cplusplus

#endif // __NeXT_NeXTDelegate_h
