//
//  OSXCanvasView.m
//  
//
//  Created by Matt Reda on Mon Feb 11 2002.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#include "cssysdef.h"

#import "OSXCanvasView.h"
#import "OSXDriver2D.h"


@implementation OSXCanvasView

// initWithFrame
// Initialize object and superclass
- (id) initWithFrame:(NSRect) frame
{
    self = [super initWithFrame:frame];
    
    if (self != nil)
        driver = nil;
    
    return self;
}


// dealloc
// Deallocate object and instance variables
- (void) dealloc
{
    [super dealloc];
}

// Set the corrsponding driver
- (void) setDriver:(OSXDriver2D *) inDriver
{
    driver = inDriver;
}


// acceptsFirstResponder
// Returns YES to indicate that it will become the first responder
- (BOOL) acceptsFirstResponder
{
    return YES;
}


// Events
// All these methods just relay the event to the assistant
- (void) keyDown:(NSEvent *) ev
{
    driver->DispatchEvent(ev, self);
}

- (void) keyUp:(NSEvent *) ev
{
    driver->DispatchEvent(ev, self);
}

- (void) flagsChanged:(NSEvent *) ev
{
    driver->DispatchEvent(ev, self);
}

- (void) mouseMoved:(NSEvent *) ev
{
    driver->DispatchEvent(ev, self);
}

- (void) mouseDown:(NSEvent *) ev
{
    driver->DispatchEvent(ev, self);
}

- (void) mouseUp:(NSEvent *) ev
{
    driver->DispatchEvent(ev, self);
}

- (void) mouseDragged:(NSEvent *) ev
{
    driver->DispatchEvent(ev, self);
}

- (void) rightMouseDown:(NSEvent *) ev
{
    driver->DispatchEvent(ev, self);
}

- (void) rightMouseUp:(NSEvent *) ev
{
    driver->DispatchEvent(ev, self);
}

- (void) rightMouseDragged:(NSEvent *) ev
{
    driver->DispatchEvent(ev, self);
}

- (void) otherMouseDown:(NSEvent *) ev
{
    driver->DispatchEvent(ev, self);
}

- (void) otherMouseUp:(NSEvent *) ev
{
    driver->DispatchEvent(ev, self);
}

- (void) otherMouseDragged:(NSEvent *) ev
{
    driver->DispatchEvent(ev, self);
}

@end
