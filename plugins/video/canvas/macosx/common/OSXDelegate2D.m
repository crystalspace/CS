//
//  OSXDelegate2D.m
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#import "OSXDelegate2D.h"

#import "OSXWindow.h"

@interface OSXDelegate2D (PrivateMethods)

// Return window style
- (int) getWindowStyleForMode:(BOOL) fs;

// Set up strings for active and paused titles
- (void) configureTitles:(char *) newTitle;

// Set the title of the window based on the value of paused
- (void) adjustTitle;

// Notifications regarding window state
- (void) windowDidBecomeKey:(NSNotification *) notification;
- (void) windowDidResignKey:(NSNotification *) notification;

// Called when the window is resized
- (NSSize) windowWillResize:(NSWindow *) sender toSize:(NSSize) frameSize;

// Return YES to indicate the window can close - do some clean up first
- (BOOL) windowShouldClose:(id) sender;

@end


@implementation OSXDelegate2D


// initWithDriver
// Initialize data
- (id) initWithDriver:(OSXDriver2D) drv
{
    self = [super init];

    if (self != nil)
    {
        window = nil;
        title = nil;
        pausedTitle = nil;

        isPaused = NO;

        hideMouse = NO;
        trackingMouse = NO;
        trackingMouseTag = 0;

        driver = drv;
    };

    return self;
};


// dealloc
// Deallocate object
- (void) dealloc
{
    if (window != nil)
        [self closeWindow];

    [title release];
    [pausedTitle release];

    [super dealloc];
};


// acceptsFirstResponder
// Returns YES to indicate that it will become the first responder
- (BOOL) acceptsFirstResponder
{
    return YES;
};


// openWindow
// Open a window if none open
// In fullscreen mode, opens a zero-sized window to get events
- (BOOL) openWindow:(char *) winTitle width:(int) w height:(int) h depth:(int) d fullscreen:(BOOL) fs onDisplay:(CGDirectDisplayID) display onScreen:(int) screen;
{
    NSRect rect = NSZeroRect;

    if (window != nil)
        return YES;

    // Position window in upper left in fullscreen mode, because although CG will switch resolutions,
    // NSScreen does not reflect this change, and I don't think whatever mechanism NSWindow uses to position
    // itself reflects this change either, because putting the rect at 0,0 puts it off the bottom of the screen
    // Center rect in Windowed mode (using CG to get correct screen dimensions)
    if (fs == YES)
        rect = NSMakeRect(0, [[NSScreen mainScreen] frame].size.height - h, w - 1, h - 1);
    else
    {
        int dispWidth = CGDisplayPixelsWide(display);
        int dispHeight = CGDisplayPixelsHigh(display);
        rect = NSMakeRect((dispWidth - w) / 2, (dispHeight - h) / 2, w - 1, h - 1);
    };

    // Create window with correct style
    style = [self getWindowStyleForMode:fs];
    window = [[OSXWindow alloc] initWithContentRect:rect styleMask:style
                backing:NSBackingStoreBuffered defer:NO 
                screen:[[NSScreen screens] objectAtIndex:screen]];

    if (window == nil)
        return NO;

    // Set up window stuff
    if (fs == YES)
        [window setLevel:CGShieldingWindowLevel()];

    [self configureTitles:winTitle];
    [self adjustTitle];

    [window useOptimizedDrawing:YES];
    [window setDelegate:self];
    [window makeFirstResponder:self];
    [[window contentView] setNextResponder:self];
    [window makeKeyAndOrderFront:nil];

    // Start tracking mouse
    [self startTrackingMouse];

    return YES;
};


// setTitle
// Set the window's title
- (void) setTitle:(char *) newTitle
{
    [self configureTitles:newTitle];
    [self adjustTitle];
};


// setMouseCursor
// Set the mouse cursor
- (BOOL) setMouseCursor:(csMouseCursorID) cursor
{
    hideMouse = YES;
    if (cursor == csmcArrow)
    {
        [[NSCursor arrowCursor] set];
        hideMouse = NO;
    };

    if (hideMouse == YES)
        OSXDriver2D_HideMouse(driver);
    else
        OSXDriver2D_ShowMouse(driver);

    return !hideMouse;
};


// startTrackingMouse
// Start tracking mouse position
- (void) startTrackingMouse
{
    if ((trackingMouse == NO) && (window != nil))
    {
        NSView *contentView = [window contentView];
        NSRect contentRect = [contentView bounds];
        NSPoint point = [contentView convertPoint:[window mouseLocationOutsideOfEventStream] fromView:nil];
        BOOL insideContentRect = [contentView mouse:point inRect:contentRect];

        // Set up tag for mouseEntered and mouseExited events
        trackingMouseTag = [contentView addTrackingRect:contentRect owner:self userData:nil
                                                                                assumeInside:insideContentRect];

        // If inside, accept movement events
        [window setAcceptsMouseMovedEvents:insideContentRect];

        if ((hideMouse == YES) && (insideContentRect == YES))
            OSXDriver2D_HideMouse(driver);

        trackingMouse = YES;
    };
};


// stopTrackingMouse
// Stop tracking the mouse's position
- (void) stopTrackingMouse
{
    if ((trackingMouse == YES) && (window != nil))
    {
        //Stop tracking mouse
        [window setAcceptsMouseMovedEvents:NO];

        // Stop tracking mouse entered/exited
        [[window contentView] removeTrackingRect:trackingMouseTag];

        trackingMouse = NO;
    };
};


// mouseEntered
// Handle mouse entering the tracking area by accepting movement events
- (void) mouseEntered:(NSEvent *) ev
{
    if ([ev trackingNumber] == trackingMouseTag)
    {
        [window setAcceptsMouseMovedEvents:YES];

        if (hideMouse == YES)
            OSXDriver2D_HideMouse(driver);
    };
};



// mouseExited
// Mouse has left tracking rect, so stop listening for mouse movement
- (void) mouseExited:(NSEvent *) ev
{
    if ([ev trackingNumber] == trackingMouseTag)
    {
        [window setAcceptsMouseMovedEvents:NO];
    };
};


// closeWindow
// Close the window if it is open
- (void) closeWindow
{
    if (window != nil)
    {
        [self stopTrackingMouse];

        [window release];
        window = nil;
    };
};



// focusChanged
// Window focus changed
- (void) focusChanged:(BOOL) focused
{
    isPaused = !focused;
    [self adjustTitle];

    if (isPaused == YES)
        [self stopTrackingMouse];
    else
        [self startTrackingMouse];
};


// Events
// All these methods just relay the event to the assistant
- (void) keyDown:(NSEvent *) ev
{
    OSXDriver2D_DispatchEvent(driver, ev, [window contentView]);
};

- (void) keyUp:(NSEvent *) ev
{
    OSXDriver2D_DispatchEvent(driver, ev, [window contentView]);
};

- (void) flagsChanged:(NSEvent *) ev
{
    OSXDriver2D_DispatchEvent(driver, ev, [window contentView]);
};

- (void) mouseMoved:(NSEvent *) ev
{
    OSXDriver2D_DispatchEvent(driver, ev, [window contentView]);
};

- (void) mouseDown:(NSEvent *) ev
{
    OSXDriver2D_DispatchEvent(driver, ev, [window contentView]);
};

- (void) mouseUp:(NSEvent *) ev
{
    OSXDriver2D_DispatchEvent(driver, ev, [window contentView]);
};

- (void) mouseDragged:(NSEvent *) ev
{
    OSXDriver2D_DispatchEvent(driver, ev, [window contentView]);
};

- (void) rightMouseDown:(NSEvent *) ev
{
    OSXDriver2D_DispatchEvent(driver, ev, [window contentView]);
};

- (void) rightMouseUp:(NSEvent *) ev
{
    OSXDriver2D_DispatchEvent(driver, ev, [window contentView]);
};

- (void) rightMouseDragged:(NSEvent *) ev
{
    OSXDriver2D_DispatchEvent(driver, ev, [window contentView]);
};


@end



@implementation OSXDelegate2D (PrivateMethods)



// getWindowStyleForMode
// fs windows use a different style than windowed mode - this function returns the correct type
- (int) getWindowStyleForMode:(BOOL) fs
{
    int winStyle = 0;
    if (fs == YES)
        winStyle = NSBorderlessWindowMask;
    else
        winStyle = NSTitledWindowMask | NSResizableWindowMask;

    return winStyle;
};


// configureTitles
// Set up strings for active and paused titles
- (void) configureTitles:(char *) newTitle
{
    [title release];
    [pausedTitle release];
    title = [[NSString alloc] initWithCString:newTitle];
    pausedTitle = [[title stringByAppendingString:@"  [Paused]"] retain];
};


// adjustTitle
// Set the title of the window based on the value of paused
- (void) adjustTitle
{
    [window setTitle:(isPaused == YES) ? pausedTitle : title];
};


// windowDidBecomeKey
// Window became key - track mouse
- (void) windowDidBecomeKey:(NSNotification *) notification
{
    [self startTrackingMouse];
};


// windowDidResignKey
// Window is no longer key - stop mouse tracking
- (void) windowDidResignKey:(NSNotification *) notification
{
    [self stopTrackingMouse];
};


// windowWillResize
// Called when the window is resized
- (NSSize) windowWillResize:(NSWindow *) sender toSize:(NSSize) frameSize
{
    // Want to resize canvas based on content rect size, not frame rect size
    NSRect newFrameRect = [window frame];
    NSSize contentSize;

    newFrameRect.size = frameSize;
    contentSize = [NSWindow contentRectForFrameRect:newFrameRect styleMask:style].size;

    if (OSXDriver2D_Resize(driver, contentSize.width, contentSize.height) == YES)
    {
        NSRect rect = NSMakeRect(0, 0, contentSize.width - 1, contentSize.height - 1);
        newFrameRect = [NSWindow frameRectForContentRect:rect styleMask:style];
        return newFrameRect.size;
    };

    return [window frame].size;
};


// windowShouldClose
// Return YES to indicate the window can close - do some clean up first
- (BOOL) windowShouldClose:(id) sender
{
  [self stopTrackingMouse];

  if (hideMouse == YES)
    OSXDriver2D_ShowMouse(driver);

  return YES;
}

@end



// C API to DriverDelegate class
#define DEL2D_FUNC(ret, func) __private_extern__ ret inline OSXDelegate2D_##func

typedef void *OSXDelegate2DHandle;
typedef void *OSXDriver2DHandle;

// C API to driver delegate class - wrappers around methods
DEL2D_FUNC(OSXDelegate2DHandle, new)(OSXDriver2DHandle drv)
{
    return [[OSXDelegate2D alloc] initWithDriver:drv];
};

DEL2D_FUNC(void, delete)(OSXDelegate2DHandle delegate)
{
    [(OSXDelegate2D *) delegate release];
};

DEL2D_FUNC(bool, openWindow)(OSXDelegate2DHandle delegate, char *title, int w, int h, int d, bool fs, CGDirectDisplayID display, int screen)
{
    return [(OSXDelegate2D *) delegate openWindow:title width:w height:h depth:d fullscreen:fs onDisplay:display onScreen:screen];
};

DEL2D_FUNC(void, closeWindow)(OSXDelegate2DHandle delegate)
{
    [(OSXDelegate2D *) delegate closeWindow];
};

DEL2D_FUNC(void, setTitle)(OSXDelegate2DHandle delegate, char *title)
{
    [(OSXDelegate2D *) delegate setTitle:title];
};

DEL2D_FUNC(BOOL, setMouseCursor)(OSXDelegate2DHandle delegate, csMouseCursorID cursor)
{
    return [(OSXDelegate2D *) delegate setMouseCursor:cursor];
};

DEL2D_FUNC(void, focusChanged)(OSXDelegate2DHandle delegate, BOOL focused)
{
    [(OSXDelegate2D *) delegate focusChanged:focused];
};

#undef DEL2D_FUNC

