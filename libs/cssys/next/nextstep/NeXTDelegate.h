#ifndef __NeXT_NeXTDelegate_h
#define __NeXT_NeXTDelegate_h
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
// NeXTDelegate.h
//
//	A delegate to the Application and animation Window.  Acts as a gateway
//	between the AppKit and CrystalSpace by forwarding Objective-C messages
//	and events to the C++ system driver, NeXTSystemDriver.  In particular,
//	mouse and keyboard related events from the animation view are
//	translated into CrystalSpace format and forwarded.  Application and
//	Window events (such as application termination) are also handled.
//
//-----------------------------------------------------------------------------
extern "Objective-C" {
#import <objc/Object.h>
}
extern "C" {
#import <dpsclient/dpsNeXT.h>
#import <dpsclient/event.h>
}
@class View, Window;
class NeXTKeymap;
class NeXTSystemDriver;

@interface NeXTDelegate : Object
    {
    NeXTSystemDriver* driver;
    NeXTKeymap* keymap;
    Window* animationWindow;
    int oldEventMask;
    DPSTimedEntry timer;
    unsigned long modifiers;
    BOOL mouseHidden;
    BOOL paused;
    BOOL autoResume;
    BOOL tracking;
    char* savedTitle;
    }

- (id)initWithDriver:(NeXTSystemDriver*)driver;
- (id)windowWillClose:(id)sender;
- (id)windowDidMove:(id)sender;
- (void)registerAnimationWindow:(Window*)w; // Must have valid windowNum.
- (void)resetTimer;

- (void)pause;
- (void)unpause;
- (id)togglePause:(id)sender;

- (void)showMouse;
- (void)hideMouse;

- (void)mouseEntered:(NXEvent*)p;
- (void)mouseExited: (NXEvent*)p;

- (void)keyDown:          (NXEvent*)p inView:(View*)v;
- (void)keyUp:            (NXEvent*)p inView:(View*)v;
- (void)flagsChanged:     (NXEvent*)p inView:(View*)v;
- (void)mouseMoved:       (NXEvent*)p inView:(View*)v;
- (void)mouseDown:        (NXEvent*)p inView:(View*)v;
- (void)mouseUp:          (NXEvent*)p inView:(View*)v;
- (void)mouseDragged:     (NXEvent*)p inView:(View*)v;
- (void)rightMouseDown:   (NXEvent*)p inView:(View*)v;
- (void)rightMouseUp:     (NXEvent*)p inView:(View*)v;
- (void)rightMouseDragged:(NXEvent*)p inView:(View*)v;

@end

#endif // __NeXT_NeXTDelegate_h
