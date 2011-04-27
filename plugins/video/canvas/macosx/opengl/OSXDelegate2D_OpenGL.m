//
//  OSXDelegate2D_OpenGL.m
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#import "csplugincommon/macosx/OSXDelegate2D.h"


///// Delegate category

@interface OSXDelegate2D (OpenGL)

// Create an OpenGL Context
- (CGLContextObj) createOpenGLContext:(int) depth display:(CGDirectDisplayID) display;
// Get the PixelFormat values for BitDepth info
- (long *) getOpenGLPixelFormatValues;
// Update OpenGL context (bind to current window, etc)
- (void) updateOpenGLContext;

@end


@implementation OSXDelegate2D (OpenGL)

static long *pixelFormatValues; // FIXME: Should be instance variable.

static NSOpenGLContext *context; // FIXME: Should be instance variable.

// createOpenGLContext
// Create an OpenGL context in the window and return it
// Return a C representation since the core class can't handle the ObjC object
// Safe to call without a window - will create an unbound context
- (CGLContextObj) createOpenGLContext:(int) depth display:(CGDirectDisplayID) display
{
    NSOpenGLPixelFormat *pixelFormat;
    GLint return_value;

    NSOpenGLPixelFormatAttribute attribs[] = {
        NSOpenGLPFAWindow,
	NSOpenGLPFADoubleBuffer,
	NSOpenGLPFAAccelerated,
        NSOpenGLPFAColorSize, depth,
	NSOpenGLPFADepthSize, 32,
        NSOpenGLPFAStencilSize, 8,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(display),
	0
    };
    NSOpenGLPixelFormatAttribute attribsNoAccel[] = {
        NSOpenGLPFAWindow,
	NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, depth,
	NSOpenGLPFADepthSize, 32,
        NSOpenGLPFAStencilSize, 8,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(display),
	0
    };

    // Create a pixel format
    pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
    if (pixelFormat == nil)
    {
      pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribsNoAccel];
      if (pixelFormat == nil)
	return 0;
    }

    // Store pixelFormatValues before pixelFormat is release'd
    // pixelFormatValues is free'd in GLOSXDriver2D.cpp
	// -- A non-author's note, this caused crashes when using ptmalloc
	// and also would cause crashes if the values were requested again.
	// Right now this is left un-freed, we should make a destructor for
	// this class.
    pixelFormatValues = (long *)malloc(sizeof(long) * 5);

    [pixelFormat    getValues:&return_value 
                    forAttribute:NSOpenGLPFAColorSize
                    forVirtualScreen:0];
    pixelFormatValues[0] = return_value;

    [pixelFormat    getValues:&return_value 
                    forAttribute:NSOpenGLPFAAlphaSize
                    forVirtualScreen:0];
    pixelFormatValues[1] = return_value;

    [pixelFormat    getValues:&return_value 
                    forAttribute:NSOpenGLPFADepthSize
                    forVirtualScreen:0];
    pixelFormatValues[2] = return_value;

    [pixelFormat    getValues:&return_value 
                    forAttribute:NSOpenGLPFAStencilSize
                    forVirtualScreen:0];
    pixelFormatValues[3] = return_value;

    [pixelFormat    getValues:&return_value 
                    forAttribute:NSOpenGLPFAAccumSize
                    forVirtualScreen:0];
    pixelFormatValues[4] = return_value;

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

    return (CGLContextObj)[context CGLContextObj];
}

// getOpenGLPixelFormat
// Get the PixelFormat object for BitDepth info
- (long *) getOpenGLPixelFormatValues
{
    // Must call createOpenGLContext first for this to work!
    return pixelFormatValues;
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
#define DEL2D_FUNC(ret, func) __private_extern__ ret OSXDelegate2D_##func

typedef void *OSXDelegate2DHandle;


// C API to driver delegate class - wrappers around methods
DEL2D_FUNC(CGLContextObj, createOpenGLContext)(OSXDelegate2DHandle delegate, int depth,
                                                CGDirectDisplayID display)
{
    return [(OSXDelegate2D *) delegate createOpenGLContext:depth display:display];
}

DEL2D_FUNC(long *, getOpenGLPixelFormatValues)
(OSXDelegate2DHandle delegate)
{
    return [(OSXDelegate2D *) delegate getOpenGLPixelFormatValues];
}

DEL2D_FUNC(void, updateOpenGLContext)(OSXDelegate2DHandle delegate)
{
    [(OSXDelegate2D *) delegate updateOpenGLContext];
}


#undef DEL2D_FUNC
