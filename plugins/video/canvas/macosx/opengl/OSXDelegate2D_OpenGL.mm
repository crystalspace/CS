//
//  OSXDelegate2D_OpenGL.m
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#import "video/canvas/macosx/common/OSXDelegate2D.h"


@implementation OSXDelegate2D (OpenGL)


// Need to update the OpenGL context when the window is resized
// This cannot be done in the standard Resize handling because that is done in the delegate method,
// which means the window is going to resize (but hasn't yet)
- (void) windowResized:(NSNotification *) notification
{
    [[NSOpenGLContext currentContext] update];
}

@end
