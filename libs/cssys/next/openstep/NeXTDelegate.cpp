//=============================================================================
//
//	Copyright (C)1999 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// NeXTDelegate.cpp
//
//	A delegate to the Application and animation Window.  Acts as a gateway 
//	between the AppKit and CrystalSpace by forwarding Objective-C messages 
//	and events to SysSystemDriver's C++ proxy, NeXTSystemProxy.  In 
//	particular, mouse and keyboard related events from the animation view 
//	are translated into CrystalSpace format and forwarded.  Application 
//	and Window events (such as application termination) are also handled.
//
//-----------------------------------------------------------------------------
#import "NeXTDelegate.h"
#import "NeXTSystemProxy.h"
#import "csinput/csevdefs.h"
extern "Objective-C" {
#import <AppKit/NSApplication.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>
}

//-----------------------------------------------------------------------------
// Keystrokes which must be translated to CrystalSpace-specific codes.
//-----------------------------------------------------------------------------
enum
    {
    K_TAB		= '\t',
    K_RETURN		= '\r',
    K_BACKSPACE		= '\b',
    K_ESCAPE		= 0x1b,
    K_DELETE		= 0x7f,
    K_LEFT		= NSLeftArrowFunctionKey,
    K_UP		= NSUpArrowFunctionKey,
    K_RIGHT		= NSRightArrowFunctionKey,
    K_DOWN		= NSDownArrowFunctionKey,
    K_KP_CENTER		= '5',
    K_KP_LEFT		= '4',
    K_KP_UP		= '8',
    K_KP_RIGHT		= '6',
    K_KP_DOWN		= '2',
    K_KP_PAGE_UP	= '9',
    K_KP_PAGE_DOWN	= '3',
    K_KP_HOME		= '7',
    K_KP_END		= '1',
    K_KP_INSERT		= '0',
    K_KP_DELETE		= '.',
    K_KP_ENTER		= 0x03,
    K_ED_PAGE_UP	= NSPageUpFunctionKey,
    K_ED_PAGE_DOWN	= NSPageDownFunctionKey,
    K_ED_HOME		= NSHomeFunctionKey,
    K_ED_END		= NSEndFunctionKey,
    K_ED_INSERT		= NSInsertFunctionKey,
    K_ED_DELETE		= NSDeleteFunctionKey,
    };

//=============================================================================
// IMPLEMENTATION
//=============================================================================
@implementation NeXTDelegate

//-----------------------------------------------------------------------------
// timerFired:
//	Target of timer.  Forwards timer event to proxy.  Runs in child thread.
//-----------------------------------------------------------------------------
- (void)timerFired:(NSTimer*)p
    {
    proxy->timer_fired();
    }


//-----------------------------------------------------------------------------
// startTimer
//-----------------------------------------------------------------------------
- (void)startTimer
    {
    if (timer == 0)
	{
	float const CS_FPS = 15;
	timer = [[NSTimer scheduledTimerWithTimeInterval:(1.0 / CS_FPS)
		target:self selector:@selector(timerFired:) userInfo:0
		repeats:YES] retain];
	}
    }


//-----------------------------------------------------------------------------
// stopTimer
//-----------------------------------------------------------------------------
- (void)stopTimer
    {
    if (timer != 0)
	{
	[timer invalidate];
	[timer release];;
	timer = 0;
	}
    }


//-----------------------------------------------------------------------------
// resetTimer
//-----------------------------------------------------------------------------
- (void)resetTimer
    {
    [self stopTimer];
    [self startTimer];
    }


//-----------------------------------------------------------------------------
// startTracking:
//-----------------------------------------------------------------------------
- (void)startTracking:(NSWindow*)w
    {
    if (!tracking)
	{
	tracking = YES;
	NSView* const v = [w contentView];
	trackingTag = [v addTrackingRect:[v bounds] owner:self userData:0
		assumeInside:NO];
	}
    }


//-----------------------------------------------------------------------------
// stopTracking:
//-----------------------------------------------------------------------------
- (void)stopTracking:(NSWindow*)w
    {
    if (tracking)
	{
	tracking = NO;
	[w setAcceptsMouseMovedEvents:NO];
	[[w contentView] removeTrackingRect:trackingTag];
	}
    }


//-----------------------------------------------------------------------------
// showMouse
//-----------------------------------------------------------------------------
- (void)showMouse
    {
    if (mouseHidden)
	{
	mouseHidden = NO;
	[NSCursor unhide];
	}
    }


//-----------------------------------------------------------------------------
// hideMouse
//-----------------------------------------------------------------------------
- (void)hideMouse
    {
    if (!mouseHidden)
	{
	mouseHidden = YES;
	[NSCursor hide];
	}
    }


//-----------------------------------------------------------------------------
// adjustWindowPosition:
//	For best video performance align left-edge of NeXTView (the window's
//	contentView) at a position divisible by 8.  See also: README.NeXT and
//	the NextStep 3.0 WindowServer release notes.
//-----------------------------------------------------------------------------
- (void)adjustWindowPosition:(NSWindow*)w
    {
    NSView* const v = [w contentView];	// The NeXTView.
    NSPoint const p =
	[w convertBaseToScreen:[v convertPoint:[v bounds].origin toView:0]];
    int const ALIGN = (1 << 3) - 1;	// 8-pixel alignment
    int x = int( p.x );
    if ((x & ALIGN) != 0)
	{
	x &= ~ALIGN;
	NSRect const wr = [w frame];
	float const dx = p.x - wr.origin.x;
	[w setFrameOrigin:NSMakePoint(x - dx, wr.origin.y)];
	}
    }


//-----------------------------------------------------------------------------
// prepareWindow:
//	Set up a tracking area so that we can request mouse-moved events when 
//	mouse is inside the window's content area.  
//-----------------------------------------------------------------------------
- (void)prepareWindow:(NSWindow*)w
    {
    if (w != 0)
	{
	[self startTracking:w];
	[self adjustWindowPosition:w];
	[w setDelegate:self];
	}
    }


//-----------------------------------------------------------------------------
// unprepareWindow:
//-----------------------------------------------------------------------------
- (void)unprepareWindow:(NSWindow*)w
    {
    if (w != 0)
	{
	[self stopTracking:w];
	[w setDelegate:0];
	}
    }


//-----------------------------------------------------------------------------
// registerAnimationWindow:
//-----------------------------------------------------------------------------
- (void)registerAnimationWindow:(NSWindow*)w
    {
    [self unprepareWindow:animationWindow];
    animationWindow = w;
    [self prepareWindow:animationWindow];
    [self startTimer];
    }


//-----------------------------------------------------------------------------
// quit: -- Terminate the application.
//-----------------------------------------------------------------------------
- (void)quit:(id)sender
    {
    [self showMouse];
    [self stopTimer];
    [self unprepareWindow:animationWindow];
    [animationWindow close];
    proxy->terminate();
    }


//-----------------------------------------------------------------------------
// windowShouldClose:
//	Terminate the application when the animation-window closes.
//-----------------------------------------------------------------------------
- (BOOL)windowShouldClose:(id)sender
    {
    if (sender == animationWindow)
	[self quit:self];
    return YES;
    }


//-----------------------------------------------------------------------------
// windowDidMove:
//-----------------------------------------------------------------------------
- (void)windowDidMove:(NSNotification*)n
    {
    if ([n object] == animationWindow)
	[self adjustWindowPosition:animationWindow];
    }


//-----------------------------------------------------------------------------
// localize:toView:x:y: -- Convert event location to view coordinate system.
//-----------------------------------------------------------------------------
- (BOOL)localize:(NSEvent*)event toView:(NSView*)view x:(int*)x y:(int*)y
    {
    NSPoint const p = [view convertPoint:[event locationInWindow] fromView:0];
    NSRect const r = [view bounds];
    *x = int(p.x);
    *y = int(r.size.height - p.y);	// CrystalSpace coords flipped.
    return (*x >= 0 && *y >= 0 && *x < r.size.width && *y < r.size.height);
    }


//-----------------------------------------------------------------------------
// classifyFunctionKey:
//-----------------------------------------------------------------------------
- (int)classifyFunctionKey:(NSString*)s
    {
    int k = 0;
    switch ([s characterAtIndex:0])
	{
	case K_LEFT:         k = CSKEY_LEFT;  break;
	case K_RIGHT:        k = CSKEY_RIGHT; break;
	case K_UP:           k = CSKEY_UP;    break;
	case K_DOWN:         k = CSKEY_DOWN;  break;
	case K_ED_PAGE_UP:   k = CSKEY_PGUP;  break;
	case K_ED_PAGE_DOWN: k = CSKEY_PGDN;  break;
	case K_ED_HOME:      k = CSKEY_HOME;  break;
	case K_ED_END:       k = CSKEY_END;   break;
	case K_ED_INSERT:    k = CSKEY_INS;   break;
	case K_ED_DELETE:    k = CSKEY_DEL;   break;
	}
    return k;
    }


//-----------------------------------------------------------------------------
// classifyControlKey:
//	CrystalSpace wants control-keys translated to lower-case equivalents; 
//	that is: 'ctrl-c' --> 'c'.  
//-----------------------------------------------------------------------------
- (int)classifyControlKey:(NSString*)s
    {
    return [[s lowercaseString] characterAtIndex:0];
    }


//-----------------------------------------------------------------------------
// classifyOtherKey:
//-----------------------------------------------------------------------------
- (int)classifyOtherKey:(NSString*)s
    {
    int k = 0;
    unichar const c = [s characterAtIndex:0];
    switch (c)
	{
	case K_ESCAPE:    k = CSKEY_ESC;       break;
	case K_RETURN:    k = CSKEY_ENTER;     break;
	case K_TAB:       k = CSKEY_TAB;       break;
	case K_BACKSPACE: k = CSKEY_BACKSPACE; break;
	case K_DELETE:    k = CSKEY_BACKSPACE; break;
	default:          k = c;               break;
	}
    return k;
    }


//-----------------------------------------------------------------------------
// classifyNumericPadKey:
//-----------------------------------------------------------------------------
- (int)classifyNumericPadKey:(NSString*)s
    {
    int k = 0;
    switch ([s characterAtIndex:0])
	{
	case K_KP_CENTER:    k = CSKEY_CENTER; break;
	case K_KP_LEFT:      k = CSKEY_LEFT;   break;
	case K_KP_UP:        k = CSKEY_UP;     break;
	case K_KP_RIGHT:     k = CSKEY_RIGHT;  break;
	case K_KP_DOWN:      k = CSKEY_DOWN;   break;
	case K_KP_PAGE_UP:   k = CSKEY_PGUP;   break;
	case K_KP_PAGE_DOWN: k = CSKEY_PGDN;   break;
	case K_KP_HOME:      k = CSKEY_HOME;   break;
	case K_KP_END:       k = CSKEY_END;    break;
	case K_KP_INSERT:    k = CSKEY_INS;    break;
	case K_KP_DELETE:    k = CSKEY_DEL;    break;
	case K_KP_ENTER:     k = CSKEY_ENTER;  break;
	};
    return k;
    }


//-----------------------------------------------------------------------------
// classifyKeyDown: -- Translate OpenStep keystroke to CrystalSpace.
//-----------------------------------------------------------------------------
- (int)classifyKeyDown:(NSEvent*)p 
    {
    int k = 0;
    unsigned int const flags = [p modifierFlags];
    if ((flags & NSCommandKeyMask) == 0)
	{
	NSString* const s = [p charactersIgnoringModifiers];
	if ((flags & NSFunctionKeyMask) != 0)
	    k = [self classifyFunctionKey:s];
	else if ((flags & NSNumericPadKeyMask) != 0)
	    k = [self classifyNumericPadKey:s];
	else if ((flags & NSControlKeyMask) != 0)
	    k = [self classifyControlKey:s];
	else
	    k = [self classifyOtherKey:s];
	}
    return k;
    }


//-----------------------------------------------------------------------------
// check:key:mask:flag:
//	Track state of modifier (shift, alt, ctrl) keys.  OpenStep does not
//	supply key up/down events for modifier flags so simulate these events
//	whenever a -flagsChanged: notification is posted.
//-----------------------------------------------------------------------------
- (void)check:(NSEvent*)p key:(int)key mask:(unsigned int)mask flag:(BOOL*)flag
    {
    BOOL const state = (([p modifierFlags] & mask) != 0);
    if (state != *flag)
	{
	if (state)
	    proxy->key_down( key );
	else
	    proxy->key_up( key );
	*flag = state;
	}
    }


//-----------------------------------------------------------------------------
// Keyboard
//-----------------------------------------------------------------------------
- (void)keyDown:(NSEvent*)p inView:(NSView*)v
    {
    if (!paused)
	proxy->key_down( [self classifyKeyDown:p] );
    }

- (void)keyUp:(NSEvent*)p inView:(NSView*)v
    {
    if (!paused)
	proxy->key_up( [self classifyKeyDown:p] );
    }

- (void)flagsChanged:(NSEvent*)p inView:(NSView*)v
    {
    if (!paused)
	{
	[self check:p key:CSKEY_SHIFT mask:NSShiftKeyMask     flag:&stateShift];
	[self check:p key:CSKEY_ALT   mask:NSAlternateKeyMask flag:&stateAlt  ];
	[self check:p key:CSKEY_CTRL  mask:NSControlKeyMask   flag:&stateCtrl ];
	}
    }


//-----------------------------------------------------------------------------
// Mouse -- Note: CrystalSpace button numbers start at 1.
//-----------------------------------------------------------------------------
- (void)mouseEntered:(NSEvent*)p 
    {
    if (!paused && [p trackingNumber] == trackingTag)
	[animationWindow setAcceptsMouseMovedEvents:YES];
    }

- (void)mouseExited:(NSEvent*)p 
    {
    if ([p trackingNumber] == trackingTag)
	{
	[animationWindow setAcceptsMouseMovedEvents:NO];
	[self showMouse];
	}
    }

- (void)mouseMoved:(NSEvent*)p inView:(NSView*)v
    {
    if (!paused)
	{
	int x, y;
	if ([self localize:p toView:v x:&x y:&y])
	    proxy->mouse_moved( x, y );
	}
    }

- (void)mouseUp:(NSEvent*)p inView:(NSView*)v button:(int)button
    {
    if (!paused)
	{
	int x, y;
	[self localize:p toView:v x:&x y:&y];
	proxy->mouse_up( button, x, y );
	}
    }

- (void)mouseDown:(NSEvent*)p inView:(NSView*)v button:(int)button
    {
    if (!paused)
	{
	int x, y;
	[self localize:p toView:v x:&x y:&y];
	proxy->mouse_down( button, x, y, stateShift, stateAlt, stateCtrl );
	}
    }

- (void)mouseDragged:(NSEvent*)p inView:(NSView*)v
    { [self mouseMoved:p inView:v]; }

- (void)mouseUp:(NSEvent*)p inView:(NSView*)v
    { [self mouseUp:p inView:v button:1]; }

- (void)mouseDown:(NSEvent*)p inView:(NSView*)v
    { [self mouseDown:p inView:v button:1]; }

- (void)rightMouseDragged:(NSEvent*)p inView:(NSView*)v
    { [self mouseMoved:p inView:v]; }

- (void)rightMouseUp:(NSEvent*)p inView:(NSView*)v
    { [self mouseUp:p inView:v button:2]; }

- (void)rightMouseDown:(NSEvent*)p inView:(NSView*)v
    {
    if (paused)
	[[v nextResponder] rightMouseDown:p];
    else
	[self mouseDown:p inView:v button:2];
    }


//-----------------------------------------------------------------------------
// Activation / Deactivation
//-----------------------------------------------------------------------------
- (void)pause
    {
    if (!paused)
	{
	paused = YES;
	[self showMouse];
	[self stopTimer];
	[self stopTracking:animationWindow];
	[savedTitle release];
	savedTitle = [[animationWindow title] copy];
	[animationWindow setTitle:
		[savedTitle stringByAppendingString:@"  [Paused]"]];
	proxy->focus_changed( false );
	proxy->clock_running( false );
	}
    }

- (void)unpause
    {
    if (paused)
	{
	paused = NO;
	[animationWindow setTitle:savedTitle];
	[self startTracking:animationWindow];
	[self startTimer];
	proxy->focus_changed( true );
	proxy->clock_running( true );
	}
    }

- (void)togglePause:(id)sender
    {
    if (paused)
	[self unpause];
    else
	[self pause];
    }

- (void)windowDidBecomeKey:(NSNotification*)n
    {
    if (autoResume && [n object] == animationWindow)
	[self unpause];
    }

- (void)windowDidResignKey:(NSNotification*)n
    {
    if ([n object] == animationWindow)
	{
	autoResume = !paused;
	[self pause];
	}
    }


//-----------------------------------------------------------------------------
// initWithProxy:
//-----------------------------------------------------------------------------
- (id)initWithProxy:(NeXTSystemProxy*)p
    {
    [super init];
    animationWindow = 0;
    proxy = p;
    timer = 0;
    stateShift = NO;
    stateAlt   = NO;
    stateCtrl  = NO;
    mouseHidden = NO;
    paused = NO;
    tracking = NO;
    savedTitle = [@"" retain];
    return self;
    }


//-----------------------------------------------------------------------------
// dealloc
//-----------------------------------------------------------------------------
- (void)dealloc
    {
    [self stopTimer];
    [savedTitle release];
    [super dealloc];
    }

@end
