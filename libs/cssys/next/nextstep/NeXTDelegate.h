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
//	platform-specific assistant, NeXTAssistant.
//
//-----------------------------------------------------------------------------
#include "NeXTConfigFile.h"

#if !defined(__cplusplus)

#import <objc/Object.h>
#import <dpsclient/event.h>
#include "NeXTAssistant.h"
@class NeXTKeymap, View, Window;

@interface NeXTDelegate : Object
{
  NeXTAssistant assistant;
  NeXTKeymap* keymap;
  unsigned long modifiers;
  BOOL mouseHidden;
  BOOL paused;
  BOOL autoResume;
}

+ (NeXTDelegate*)startup:(NeXTAssistant)assistant;
+ (void)shutdown:(NeXTDelegate*)controller;

- (id)initWithDriver:(NeXTAssistant)assistant;
- (void)initApplicationMenu:(NeXTConfigHandle)handle style:(char const*)style;

- (void)pause;
- (void)unpause;
- (id)togglePause:(id)sender;

- (void)showMouse;
- (void)hideMouse;

- (void)flushGraphicsContext;

- (void)dispatchEvent:    (NXEvent*)p forView:(View*)v;
- (void)keyDown:          (NXEvent*)p forView:(View*)v;
- (void)keyUp:            (NXEvent*)p forView:(View*)v;
- (void)flagsChanged:     (NXEvent*)p forView:(View*)v;
- (void)mouseMoved:       (NXEvent*)p forView:(View*)v;
- (void)mouseDown:        (NXEvent*)p forView:(View*)v;
- (void)mouseUp:          (NXEvent*)p forView:(View*)v;
- (void)mouseDragged:     (NXEvent*)p forView:(View*)v;
- (void)rightMouseDown:   (NXEvent*)p forView:(View*)v;
- (void)rightMouseUp:     (NXEvent*)p forView:(View*)v;
- (void)rightMouseDragged:(NXEvent*)p forView:(View*)v;

@end

#else // __cplusplus

#define ND_PROTO(RET,FUNC) extern "C" RET NeXTDelegate_##FUNC

typedef void* NeXTAssistantHandle;
typedef void* NeXTDelegate;
typedef void* NeXTEvent;
typedef void* NeXTView;

ND_PROTO(NeXTDelegate,startup)(NeXTAssistantHandle);
ND_PROTO(void,shutdown)(NeXTDelegate);
ND_PROTO(void,init_app_menu)(NeXTDelegate, NeXTConfigHandle, char const*);
ND_PROTO(void,start_event_loop)(NeXTDelegate);
ND_PROTO(void,stop_event_loop)(NeXTDelegate);
ND_PROTO(void,dispatch_event)(NeXTDelegate, NeXTEvent, NeXTView);
ND_PROTO(void,hide_mouse_pointer)(NeXTDelegate);
ND_PROTO(void,show_mouse_pointer)(NeXTDelegate);
ND_PROTO(void,flush_graphics_context)(NeXTDelegate);

#undef ND_PROTO

#endif // __cplusplus

#endif // __NeXT_NeXTDelegate_h
