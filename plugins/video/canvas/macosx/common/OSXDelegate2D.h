//
//  OSXDelegate2D.h
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

// For setting mouse cursor
#include "ivideo/cursor.h"

#ifndef __CS_OSXDELEGATE2D_H__
#define __CS_OSXDELEGATE2D_H__

#import <Cocoa/Cocoa.h>


class OSXDriver2D;


@interface OSXDelegate2D : NSObject
{
  // Corresponding driver object
  OSXDriver2D *driver;

  // Keep track of mouse tracking state
  NSTrackingRectTag trackingMouseTag; 
  BOOL trackingMouse;
}

// Initialize with driver
- (id) initWithDriver:(OSXDriver2D *) drv;

// Deallocate object
- (void) dealloc;

// Start/Stop tracking mouse position
- (void) startTrackingMouseInWindow:(NSWindow *) window;
- (void) stopTrackingMouseInWindow:(NSWindow *) window;

// Handle mouse entering or leaving the tracking area
- (void) mouseEntered:(NSEvent *) ev;
- (void) mouseExited:(NSEvent *) ev;

@end

#endif // __CS_OSXDELEGATE2D_H__
