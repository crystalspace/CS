//
//  OSXDelegate2D_OpenGL.m
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#import "csplugincommon/macosx/OSXDelegate2D.h"


// Create a small category for NSOpenGLContext to give access to the CGL context
@interface NSOpenGLContext (CGLContextAccess)

// Return the CGL context
- (CGLContextObj) getCGLContext;

@end

@implementation NSOpenGLContext (CGLContextAccess)

// getCGLContext
// Returns the (private) CGL context
- (CGLContextObj) getCGLContext
{
    return _contextAuxiliary;
}

@end

///// Delegate category


@interface OSXDelegate2D (OpenGL)

// Create an OpenGL Context
- (CGLContextObj) createOpenGLContext:(int) depth display:(CGDirectDisplayID) display;

// Update OpenGL context (bind to current window, etc)
- (void) updateOpenGLContext;

@end


@implementation OSXDelegate2D (OpenGL)

NSOpenGLContext *context;

// createOpenGLContext
// Create an OpenGL context in the window and return it
// Return a C representation since the core class can't handle the ObjC object
// Safe to call without a window - will create an unbound context
- (CGLContextObj) createOpenGLContext:(int) depth display:(CGDirectDisplayID) display
{
    NSOpenGLPixelFormat *pixelFormat;

    // Attributes for OpenGL contexts (0 is for fullscreen, 1 is for window)
    NSOpenGLPixelFormatAttribute attribs[] = {
        NSOpenGLPFAWindow, NSOpenGLPFADoubleBuffer, NSOpenGLPFAAccelerated,
        NSOpenGLPFAColorSize, depth, NSOpenGLPFADepthSize, 32,
        NSOpenGLPFAStencilSize, 8,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(display), nil
    };

    // Create a pixel format
    pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
    if (pixelFormat == nil)
        return 0;

    // Create a GL context
    context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    [pixelFormat release];
    if (context == nil)
        return 0;

    // Need to know when window is resized so we can update the OpenGL context
    if (window != nil)
    {
        [[NSNotificationCenter defaultCenter] addObserver:self 
                selector:@selector(windowResized:)
                name:NSWindowDidResizeNotification object:window];

        // Bind context
        [context setView:[window contentView]];
    }

    // Make the context we created be the current GL context
    [context makeCurrentContext];

    return [context getCGLContext];
}


// updateOpenGLContext
// Update OpenGL context (bind to current window, etc)
- (void) updateOpenGLContext
{
    // Listen for resizes on new window
    [[NSNotificationCenter defaultCenter] addObserver:self 
                            selector:@selector(windowResized:)
                            name:NSWindowDidResizeNotification object:window];

    [context setView:[window contentView]];
    [context makeCurrentContext];
    [context update];
}


// Need to update the OpenGL context when the window is resized
// This cannot be done in the standard Resize handling because that is done in the delegate method,
// which means the window is going to resize (but hasn't yet)
- (void) windowResized:(NSNotification *) notification
{
    [[NSOpenGLContext currentContext] update];
}



@end


// C API to OSXDelegate2D class (OpenGL category)
#define DEL2D_FUNC(ret, func) __private_extern__ inline ret OSXDelegate2D_##func

typedef void *OSXDelegate2DHandle;


// C API to driver delegate class - wrappers around methods
DEL2D_FUNC(CGLContextObj, createOpenGLContext)(OSXDelegate2DHandle delegate, int depth,
                                                CGDirectDisplayID display)
{
    return [(OSXDelegate2D *) delegate createOpenGLContext:depth display:display];
}

DEL2D_FUNC(void, updateOpenGLContext)(OSXDelegate2DHandle delegate)
{
    [(OSXDelegate2D *) delegate updateOpenGLContext];
}


#undef DEL2D_FUNC
