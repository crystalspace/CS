//
//  OSXView.m
//  
//
//  Created by Matt Reda on Mon Feb 11 2002.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#import "csplugincommon/macosx/OSXView.h"
#import "csplugincommon/macosx/OSXDelegate2D.h"


@implementation OSXView

// initWithFrame
// Initialize object and superclass
- (id) initWithFrame:(NSRect) frame
{
    self = [super initWithFrame:frame];
    
    if (self != nil)
    {
        delegate = nil;
    };
    
    return self;
}


// dealloc
// Deallocate object and instance variables
- (void) dealloc
{
    [delegate release];
    [super dealloc];
}

// Set the delegate
- (void) setDelegate:(OSXDelegate2D *) inDelegate
{
    [inDelegate retain];
    [delegate release];
    delegate = inDelegate;
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
    [delegate dispatchEvent:ev forView:self];
}

- (void) keyUp:(NSEvent *) ev
{
    [delegate dispatchEvent:ev forView:self];
}

- (void) flagsChanged:(NSEvent *) ev
{
    [delegate dispatchEvent:ev forView:self];
}

- (void) mouseMoved:(NSEvent *) ev
{
    [delegate dispatchEvent:ev forView:self];
}

- (void) mouseDown:(NSEvent *) ev
{
    [delegate dispatchEvent:ev forView:self];
}

- (void) mouseUp:(NSEvent *) ev
{
    [delegate dispatchEvent:ev forView:self];
}

- (void) mouseDragged:(NSEvent *) ev
{
    [delegate dispatchEvent:ev forView:self];
}

- (void) rightMouseDown:(NSEvent *) ev
{
    [delegate dispatchEvent:ev forView:self];
}

- (void) rightMouseUp:(NSEvent *) ev
{
    [delegate dispatchEvent:ev forView:self];
}

- (void) rightMouseDragged:(NSEvent *) ev
{
    [delegate dispatchEvent:ev forView:self];
}

- (void) otherMouseDown:(NSEvent *) ev
{
    [delegate dispatchEvent:ev forView:self];
}

- (void) otherMouseUp:(NSEvent *) ev
{
    [delegate dispatchEvent:ev forView:self];
}

- (void) otherMouseDragged:(NSEvent *) ev
{
    [delegate dispatchEvent:ev forView:self];
}

@end
