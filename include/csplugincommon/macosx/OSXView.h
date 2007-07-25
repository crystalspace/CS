//
//  OSXView.h
//  
//
//  Created by Matt Reda on Mon Feb 11 2002.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#import <AppKit/AppKit.h>
#include "csextern_osx.h"

@class OSXDelegate2D;

@interface OSXView : NSView
{
  /// Delegate passes along events
  OSXDelegate2D *delegate;
}

/// Initialize object
- (id) initWithFrame:(NSRect) frame;

/// Deallocate object
- (void) dealloc;

/// Set the delegate
- (void) setDelegate:(OSXDelegate2D *) inDelegate;

/// Returns YES to indicate that it will become the first responder
- (BOOL) acceptsFirstResponder;

/// Events - passed to driver
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
- (void) otherMouseDown:(NSEvent *) ev;
- (void) otherMouseUp:(NSEvent *) ev;
- (void) otherMouseDragged:(NSEvent *) ev;

@end
