//
//  OSXDelegate2D.h
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#ifndef __CS_OSXDELEGATE2D_H__
#define __CS_OSXDELEGATE2D_H__

#include "csextern_osx.h"
#include "ivideo/cursor.h"

#include <OpenGL/OpenGL.h>

// Part of this API must be callable from C++ code.  So we have to generate a C
// API that provides access to this class
#if !defined(__cplusplus)

#import <Cocoa/Cocoa.h>
#import "OSXDriver2D.h"

@interface OSXDelegate2D : NSObject
{
  // Keep track of mouse tracking state
  NSTrackingRectTag trackingMouseTag;
  BOOL trackingMouse;
  BOOL hideMouse; // YES if mouse is not visible

  // Window - created even in fullscreen mode to get events (but with a
  // different style) Window can have one of two titles - Paused or active
  NSWindow *window;
  int style;
  NSString *title, *pausedTitle;

  // Is window paused (out of focus, etc)
  BOOL isPaused;

  // Driver that this object works with
  OSXDriver2D driver;

  // Last processed event type.
  int lastEventType;
}

// Initialize with driver
- (id) initWithDriver:(OSXDriver2D) drv;

// Deallocate object
- (void) dealloc;

// Open a window if none open
- (BOOL) openWindow:(char *) winTitle width:(int) w height:(int) h
  depth:(int) d fullscreen:(BOOL) fs onDisplay:(CGDirectDisplayID)
  display onScreen:(int) screen;

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
- (void) focusChanged:(BOOL) focused shouldPause:(BOOL) pause;

// Dispatch an event to the driver
- (void) dispatchEvent:(NSEvent *) ev forView:(NSView *) view;

@end

#else // __cplusplus

#include <ApplicationServices/ApplicationServices.h>

#define DEL2D_FUNC(ret, func) __private_extern__ "C" ret OSXDelegate2D_##func

typedef void *OSXDelegate2D;
typedef void *csGraphics2DHandle;

// C API to driver delegate class
DEL2D_FUNC(OSXDelegate2D, new)(csGraphics2DHandle drv);
DEL2D_FUNC(void, delete)(OSXDelegate2D);
DEL2D_FUNC(bool, openWindow)(OSXDelegate2D, char *title, int w, int h,
  int d, bool fs, CGDirectDisplayID display, int screen);
DEL2D_FUNC(void, closeWindow)(OSXDelegate2D);
DEL2D_FUNC(void, setTitle)(OSXDelegate2D, char *title);
DEL2D_FUNC(bool, setMouseCursor)(OSXDelegate2D, csMouseCursorID);
DEL2D_FUNC(void, focusChanged)(OSXDelegate2D, bool focused, bool shouldPause);
DEL2D_FUNC(void, setLevel)(OSXDelegate2D, int level);
DEL2D_FUNC(void, setMousePosition)(OSXDelegate2D, CGPoint point);

#undef DEL2D_FUNC

#endif // __cplusplus

#endif // __CS_OSXDELEGATE2D_H__
