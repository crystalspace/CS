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
//	and events to SysSystemDriver's C++ proxy, NeXTSystemProxy.  In 
//	particular, mouse and keyboard related events from the animation view 
//	are translated into CrystalSpace format and forwarded.  Application 
//	and Window events (such as application termination) are also handled.
//
//-----------------------------------------------------------------------------
extern "Objective-C" {
#import <Foundation/NSObject.h>
#import <AppKit/NSView.h>	// NSTrackingRectTag
}
@class NSString, NSTimer, NSWindow;
class NeXTSystemProxy;

@interface NeXTDelegate : NSObject
    {
    NSWindow* animationWindow;
    NeXTSystemProxy* proxy;
    NSTimer* timer;
    BOOL stateShift;
    BOOL stateAlt;
    BOOL stateCtrl;
    BOOL mouseHidden;
    BOOL paused;
    BOOL autoResume;
    BOOL tracking;
    NSTrackingRectTag trackingTag;
    NSString* savedTitle;
    }

- (id)initWithProxy:(NeXTSystemProxy*)proxy;
- (BOOL)windowShouldClose:(id)sender;
- (void)windowDidMove:(NSNotification*)n;
- (void)registerAnimationWindow:(NSWindow*)w; // Must have valid windowNum.
- (void)resetTimer;

- (void)pause;
- (void)unpause;
- (void)togglePause:(id)sender;

- (void)showMouse;
- (void)hideMouse;

- (void)mouseEntered:(NSEvent*)p;
- (void)mouseExited: (NSEvent*)p;

- (void)keyDown:          (NSEvent*)p inView:(NSView*)v;
- (void)keyUp:            (NSEvent*)p inView:(NSView*)v;
- (void)flagsChanged:     (NSEvent*)p inView:(NSView*)v;
- (void)mouseMoved:       (NSEvent*)p inView:(NSView*)v;
- (void)mouseDown:        (NSEvent*)p inView:(NSView*)v;
- (void)mouseUp:          (NSEvent*)p inView:(NSView*)v;
- (void)mouseDragged:     (NSEvent*)p inView:(NSView*)v;
- (void)rightMouseDown:   (NSEvent*)p inView:(NSView*)v;
- (void)rightMouseUp:     (NSEvent*)p inView:(NSView*)v;
- (void)rightMouseDragged:(NSEvent*)p inView:(NSView*)v;

@end

#endif // __NeXT_NeXTDelegate_h
