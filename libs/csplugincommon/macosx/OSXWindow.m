//
//  OSXWindow.m
//
//
//  Created by mreda on Tue Nov 06 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#import "OSXWindow.h"


@implementation OSXWindow


// canBecomeKeyWindow
// Returns YES to indicate that the window can become key
// Normal NSWindows cannot become key if they are borderless
- (BOOL) canBecomeKeyWindow
{
    return YES;
}

@end
