//
//  OSXWindow.h
//
//
//  Created by mreda on Tue Nov 06 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "csextern_osx.h"

@interface OSXWindow : NSWindow

/**
 * Returns YES to indicate that the window can become key
 * Normal NSWindows cannot become key if they are borderless
 */
- (BOOL) canBecomeKeyWindow;

@end
