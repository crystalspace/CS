//
//  OSXDelegate2D.m
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#include "cssysdef.h"

#import "OSXDelegate2D.h"

#import "OSXDriver2D.h"

@implementation OSXDelegate2D


// initWithDriver
// Initialize data
- (id) initWithDriver:(OSXDriver2D *) drv
{
    self = [super init];
    if (self != nil)
    {
        trackingMouse = NO;
        trackingMouseTag = 0;
        driver = drv;
    }
    
    return self;
}


// dealloc
// Deallocate object
- (void) dealloc
{
    [super dealloc];
}


// startTrackingMouse
// Start tracking mouse position
- (void) startTrackingMouseInWindow:(NSWindow *) window
{
    if ((trackingMouse == NO) && (window != nil))
    {
        NSView *contentView = [window contentView];
        NSRect contentRect = [contentView bounds];
        NSPoint point = [contentView convertPoint:[window mouseLocationOutsideOfEventStream] fromView:nil];
        BOOL insideContentRect = [contentView mouse:point inRect:contentRect];

        // Set up tag for mouseEntered and mouseExited events
        trackingMouseTag = [contentView addTrackingRect:contentRect owner:self 
                                userData:nil assumeInside:insideContentRect];

        // If inside, accept movement events
        [window setAcceptsMouseMovedEvents:insideContentRect];

        if ((driver->MouseIsHidden() == YES) && (insideContentRect == YES))
            driver->HideMouse();
            
        trackingMouse = YES;
    }
}


// stopTrackingMouse
// Stop tracking the mouse's position
- (void) stopTrackingMouseInWindow:(NSWindow *) window
{
    if ((trackingMouse == YES) && (window != nil))
    {
        //Stop tracking mouse
        [window setAcceptsMouseMovedEvents:NO];

        // Stop tracking mouse entered/exited
        [[window contentView] removeTrackingRect:trackingMouseTag];

        trackingMouse = NO;
    }
}


// mouseEntered
// Handle mouse entering the tracking area by accepting movement events
- (void) mouseEntered:(NSEvent *) ev
{
    if ([ev trackingNumber] == trackingMouseTag)
    {
        [[ev window] setAcceptsMouseMovedEvents:YES];

        if (driver->MouseIsHidden() == YES)
            driver->HideMouse();
    }
}



// mouseExited
// Mouse has left tracking rect, so stop listening for mouse movement
- (void) mouseExited:(NSEvent *) ev
{
    if ([ev trackingNumber] == trackingMouseTag)
    {
        [[ev window] setAcceptsMouseMovedEvents:NO];
    }
}



// windowDidBecomeKey
// Window became key - track mouse
- (void) windowDidBecomeKey:(NSNotification *) notification
{
    [self startTrackingMouseInWindow:[notification object]];
}


// windowDidResignKey
// Window is no longer key - stop mouse tracking
- (void) windowDidResignKey:(NSNotification *) notification
{
    [self stopTrackingMouseInWindow:[notification object]];
}


// windowWillResize
// Called when the window is resized
- (NSSize) windowWillResize:(NSWindow *) window toSize:(NSSize) frameSize
{
    // Want to resize canvas based on content rect size, not frame rect size
    NSSize contentSize;
    NSRect newFrameRect = [window frame];
    unsigned int style = [window styleMask];
    
    newFrameRect.size = frameSize;
    contentSize = 
        [NSWindow contentRectForFrameRect:newFrameRect styleMask:style].size;

    if (driver->Resize((int)contentSize.width, (int)contentSize.height) == YES)
    {
        NSRect rect = NSMakeRect(0, 0, contentSize.width - 1, contentSize.height - 1);
        newFrameRect = [NSWindow frameRectForContentRect:rect styleMask:style];
        return newFrameRect.size;
    }

    return [window frame].size;
}


// windowShouldClose
// Return YES to indicate the window can close - do some clean up first
- (BOOL) windowShouldClose:(NSWindow *) window
{
  [self stopTrackingMouseInWindow:window];
  driver->ShowMouse();

  return YES;
}

@end
