//
//  OSXDelegate2D.h
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

// For setting mouse cursor
#include "ivideo/cursor.h"

#ifndef __OSXDELEGATE2D_H
#define __OSXDELEGATE2D_H

#include <OpenGL/OpenGL.h>


// Part of this API must be callable from C++ code.  So we have to generate a C API that provides access to
// this class
#if !defined(__cplusplus)

#import <Cocoa/Cocoa.h>

#import "OSXDriver2D.h"


@interface OSXDelegate2D : NSResponder
{
    // Keep track of maouse tracking state
    NSTrackingRectTag trackingMouseTag;
    BOOL trackingMouse;
    BOOL hideMouse;				// YES if mouse is not visible

    // Window - created even in fullscreen mode to get events (but with a different style)
    // Window can have one of two titles - Paused or active
    NSWindow *window;
    int style;
    NSString *title, *pausedTitle;

    // Is window paused (out of focus, etc)
    BOOL isPaused;

    // Driver that this object works with
    OSXDriver2D driver;
}

// Initialize with driver
- (id) initWithDriver:(OSXDriver2D) drv;

// Deallocate object
- (void) dealloc;

// Returns YES to indicate that it will become the first responder
- (BOOL) acceptsFirstResponder;

// Open a window if none open
- (BOOL) openWindow:(char *) winTitle width:(int) w height:(int) h depth:(int) d fullscreen:(BOOL) fs;

// Set the window's title
- (void) setTitle:(char *) newTitle;

// Set the mouse cursor
- (BOOL) setMouseCursor:(csMouseCursorID) cursor;

// Start/Stop tracking mouse position
- (void) startTrackingMouse;
- (void) stopTrackingMouse;

// Handle mouse entering or leaving the tracking area
- (void) mouseEntered:(NSEvent *) ev;
- (void) mouseExited:(NSEvent *) ev;

// Close window (destroys OpenGL context as well)
- (void) closeWindow;

// Change focus of window and adjust title
- (void) focusChanged:(BOOL) focused;

// Events - passed to driver
- (void) keyDown:(NSEvent *) ev;
- (void) keyUp:(NSEvent *) ev;
- (void) flagsChanged:(NSEvent *) ev;
- (void) mouseMoved:(NSEvent *) ev;
- (void) mouseDown:(NSEvent *) ev;
- (void) mouseUp:(NSEvent *) ev;
- (void) mouseDragged:(NSEvent *) ev;
- (void) rightMouseDown:(NSEvent *) ev;
- (void) rightMouseUp:(NSEvent *) ev;
- (void) rightMouseDragged:(NSEvent *) ev;



@end


#else


#define DEL2D_FUNC(ret, func) __private_extern__ "C" inline ret OSXDelegate2D_##func

typedef void *OSXDelegate2D;
typedef void *csGraphics2DHandle;

// C API to driver delegate class
DEL2D_FUNC(OSXDelegate2D, new)(csGraphics2DHandle drv);
DEL2D_FUNC(void, delete)(OSXDelegate2D delegate);
DEL2D_FUNC(bool, openWindow)(OSXDelegate2D delegate, char *title, int w, int h, int d, bool fs);
DEL2D_FUNC(void, closeWindow)(OSXDelegate2D delegate);
DEL2D_FUNC(void, setTitle)(OSXDelegate2D delegate, char *title);
DEL2D_FUNC(bool, setMouseCursor)(OSXDelegate2D delegate, csMouseCursorID cursor);
DEL2D_FUNC(void, focusChanged)(OSXDelegate2D delegate, bool focused);

#undef DEL2D_FUNC

#endif

#endif
