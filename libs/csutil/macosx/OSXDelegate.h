#ifndef __MACOSX_OSXDelegate_h
#define __MACOSX_OSXDelegate_h
//=============================================================================
//
//	Copyright (C)1999-2003 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXDelegate.h
//
//	An object which acts as a gateway between the AppKit and Crystal Space
//	by forwarding Objective-C messages and events to the C++ platform-
//	specific assistant, OSXAssistant.  Also acts as a listener for
//	interesting notifications from the AppKit.
//
//-----------------------------------------------------------------------------
#include "OSXConfigFile.h"

#if !defined(__cplusplus)

#import <Foundation/NSObject.h>
#include "OSXAssistant.h"
@class NSEvent, NSView, NSWindow;

@interface OSXDelegate : NSObject
{
  OSXAssistant assistant;
  unsigned long modifiers;
  BOOL mouseHidden;
  BOOL paused;
  BOOL autoResume;
}

+ (OSXDelegate*)startup:(OSXAssistant)assistant;
+ (void)shutdown:(OSXDelegate*)controller;

- (id)initWithDriver:(OSXAssistant)assistant;
- (void)initApplicationMenu:(OSXConfigHandle)handle style:(char const*)style;

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

#define ND_PROTO(RET,FUNC) extern "C" RET OSXDelegate_##FUNC

typedef void* OSXSystemHandle;
typedef void* OSXDelegate;
typedef void* OSXEvent;
typedef void* OSXView;

ND_PROTO(OSXDelegate,startup)(OSXSystemHandle);
ND_PROTO(void,shutdown)(OSXDelegate);
ND_PROTO(void,init_app_menu)(OSXDelegate, OSXConfigHandle, char const*);
ND_PROTO(void,start_event_loop)(OSXDelegate);
ND_PROTO(void,stop_event_loop)(OSXDelegate);
ND_PROTO(void,dispatch_event)(OSXDelegate, OSXEvent, OSXView);
ND_PROTO(void,hide_mouse_pointer)(OSXDelegate);
ND_PROTO(void,show_mouse_pointer)(OSXDelegate);
ND_PROTO(void,flush_graphics_context)(OSXDelegate);

#undef ND_PROTO

#endif // __cplusplus

#endif // __MACOSX_OSXDelegate_h
