//=============================================================================
//
//	Copyright (C)1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
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
//	and events to the C++ system driver, NeXTSystemDriver.  In particular,
//	mouse and keyboard related events from the animation view are
//	translated into CrystalSpace format and forwarded.  Application and
//	Window events (such as application termination) are also handled.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "NeXTDelegate.h"
#include "cssys/next/NeXTSystemDriver.h"
extern "Objective-C" {
#import <AppKit/NSApplication.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>
}

//-----------------------------------------------------------------------------
// For each keystroke, Crystal Space expects a raw key code and a cooked
// character code.  For ASCII codes, Crystal Space expects the raw key code to
// be in canonic lower-case form, and the cooked character code to be in
// native cooked form.  Here are some examples:
//
//      Input    Raw  Cooked
//      -------  ---  ------
//      ctrl-C   c    ^C
//      d        d    d
//      shift-e  e    E
//      F        f    F
//      alt-m    m    "mu" *
//      shift-4  4    $
//
// (*) For alt-m, the cooked character depends upon the user's current key
// mapping.  It may actually be mapped to any character, but is often mapped to
// Greek "mu", as it was in this example.
//
// The following table translates all raw ASCII key codes to their lower-case
// equivalents in order to satisfy Crystal Space's canonic form requirement.
//-----------------------------------------------------------------------------
static char const CS_DOWN_CASE[] =
{
'2', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\',']', '6', '-',
' ', '1', '\'','3', '4', '5', '7', '\'','9', '0', '8', '=', ',', '-', '.', '/',
'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ';', ';', ',', '=', '.', '/',
'2', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\',']', '6', '-',
'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\',']', '`', 127
};


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
    K_KP_MULTIPLY	= '*',
    K_KP_DIVIDE		= '/',
    K_KP_PLUS		= '+',
    K_KP_MINUS		= '-',
    K_KP_ENTER		= 0x03,
    K_ED_PAGE_UP	= NSPageUpFunctionKey,
    K_ED_PAGE_DOWN	= NSPageDownFunctionKey,
    K_ED_HOME		= NSHomeFunctionKey,
    K_ED_END		= NSEndFunctionKey,
    K_ED_INSERT		= NSInsertFunctionKey,
    K_ED_DELETE		= NSDeleteFunctionKey,
    K_F1		= NSF1FunctionKey,
    K_F2		= NSF2FunctionKey,
    K_F3		= NSF3FunctionKey,
    K_F4		= NSF4FunctionKey,
    K_F5		= NSF5FunctionKey,
    K_F6		= NSF6FunctionKey,
    K_F7		= NSF7FunctionKey,
    K_F8		= NSF8FunctionKey,
    K_F9		= NSF9FunctionKey,
    K_F10		= NSF10FunctionKey,
    K_F11		= NSF11FunctionKey,
    K_F12		= NSF12FunctionKey,
    };

//=============================================================================
// IMPLEMENTATION
//=============================================================================
@implementation NeXTDelegate

//-----------------------------------------------------------------------------
// timerFired:
//	Target of timer; forwards timer event to driver; runs in child thread.
//-----------------------------------------------------------------------------
- (void)timerFired:(NSTimer*)p
    {
    driver->timer_fired();
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
//	contentView) at a position divisible by 8.
//
//	See also: CS/docs/texinfo/internal/platform/next.txi and the
//	NextStep 3.0 WindowServer release notes.
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
    driver->terminate();
    }


//-----------------------------------------------------------------------------
// windowShouldClose:
//	Terminate the application when the animation-window closes.
//-----------------------------------------------------------------------------
- (BOOL)windowShouldClose:(id)sender
    {
    if (sender == animationWindow)
	[self quit:0];
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
    *y = int(r.size.height - p.y - 1);	// CrystalSpace coords flipped.
    return (*x >= 0 && *y >= 0 && *x < r.size.width && *y < r.size.height);
    }


//-----------------------------------------------------------------------------
// classifyFunctionKey::
//-----------------------------------------------------------------------------
- (void)classifyFunctionKey:(int*)raw :(int*)cooked
    {
    *cooked = -1;
    switch (*raw)
	{
	case K_LEFT:         *raw = CSKEY_LEFT;  break;
	case K_RIGHT:        *raw = CSKEY_RIGHT; break;
	case K_UP:           *raw = CSKEY_UP;    break;
	case K_DOWN:         *raw = CSKEY_DOWN;  break;
	case K_ED_PAGE_UP:   *raw = CSKEY_PGUP;  break;
	case K_ED_PAGE_DOWN: *raw = CSKEY_PGDN;  break;
	case K_ED_HOME:      *raw = CSKEY_HOME;  break;
	case K_ED_END:       *raw = CSKEY_END;   break;
	case K_ED_INSERT:    *raw = CSKEY_INS;   break;
	case K_ED_DELETE:    *raw = CSKEY_DEL;   break;
	case K_F1:           *raw = CSKEY_F1;    break;
	case K_F2:           *raw = CSKEY_F2;    break;
	case K_F3:           *raw = CSKEY_F3;    break;
	case K_F4:           *raw = CSKEY_F4;    break;
	case K_F5:           *raw = CSKEY_F5;    break;
	case K_F6:           *raw = CSKEY_F6;    break;
	case K_F7:           *raw = CSKEY_F7;    break;
	case K_F8:           *raw = CSKEY_F8;    break;
	case K_F9:           *raw = CSKEY_F9;    break;
	case K_F10:          *raw = CSKEY_F10;   break;
	case K_F11:          *raw = CSKEY_F11;   break;
	case K_F12:          *raw = CSKEY_F12;   break;
	}
    }


//-----------------------------------------------------------------------------
// classifyOtherKey::
//	*NOTE* The so-called "backspace" key on the keyboard actually sends
//	DEL, however Crystal Space would like to see it as CSKEY_BACKSPACE.
//-----------------------------------------------------------------------------
- (void)classifyOtherKey:(int*)raw :(int*)cooked
    {
    switch (*raw)
	{
	case K_ESCAPE:    *raw = CSKEY_ESC;       break;
	case K_RETURN:    *raw = CSKEY_ENTER;     break;
	case K_TAB:       *raw = CSKEY_TAB;       break;
	case K_BACKSPACE: *raw = CSKEY_BACKSPACE; break;
	case K_DELETE:    *raw = CSKEY_BACKSPACE; break; // *NOTE*
	default:
	    if (*raw <= 0x7f) // Is it 7-bit ASCII?
		*raw = CS_DOWN_CASE[ *raw ];
	    break;
	}
    }


//-----------------------------------------------------------------------------
// classifyNumericPadKey::
//-----------------------------------------------------------------------------
- (void)classifyNumericPadKey:(int*)raw :(int*)cooked
    {
    switch (*raw)
	{
	case K_KP_CENTER:    *raw = CSKEY_CENTER;                break;
	case K_KP_LEFT:      *raw = CSKEY_LEFT;                  break;
	case K_KP_UP:        *raw = CSKEY_UP;                    break;
	case K_KP_RIGHT:     *raw = CSKEY_RIGHT;                 break;
	case K_KP_DOWN:      *raw = CSKEY_DOWN;                  break;
	case K_KP_PAGE_UP:   *raw = CSKEY_PGUP;                  break;
	case K_KP_PAGE_DOWN: *raw = CSKEY_PGDN;                  break;
	case K_KP_HOME:      *raw = CSKEY_HOME;                  break;
	case K_KP_END:       *raw = CSKEY_END;                   break;
	case K_KP_INSERT:    *raw = CSKEY_INS;                   break;
	case K_KP_DELETE:    *raw = CSKEY_DEL;                   break;
	case K_KP_MULTIPLY:  *raw = CSKEY_PADMULT;               break;
	case K_KP_DIVIDE:    *raw = CSKEY_PADDIV;                break;
	case K_KP_PLUS:      *raw = CSKEY_PADPLUS;               break;
	case K_KP_MINUS:     *raw = CSKEY_PADMINUS;              break;
	case K_KP_ENTER:     *raw = CSKEY_ENTER; *cooked = '\n'; break;
	};
    }


//-----------------------------------------------------------------------------
// classifyKeyDown:raw:cooked: -- Translate OpenStep keystroke to CrystalSpace.
//-----------------------------------------------------------------------------
- (BOOL)classifyKeyDown:(NSEvent*)p raw:(int*)raw cooked:(int*)cooked
    {
    BOOL ok = NO;
    *raw = *cooked = 0;
    unsigned int const flags = [p modifierFlags];
    if ((flags & NSCommandKeyMask) == 0)
	{
	*raw = [[p charactersIgnoringModifiers] characterAtIndex:0];
	*cooked = [[p characters] characterAtIndex:0];
	if ((flags & NSFunctionKeyMask) != 0)
	    [self classifyFunctionKey:raw:cooked];
	else if ((flags & NSNumericPadKeyMask) != 0)
	    [self classifyNumericPadKey:raw:cooked];
	else
	    [self classifyOtherKey:raw:cooked];
	ok = YES;
	}
    return ok;
    }


//-----------------------------------------------------------------------------
// check:ns:cs:key:
//	Track state of modifier (shift, alt, ctrl) keys.  OpenStep does not
//	supply key up/down events for modifier flags so simulate these events
//	whenever a -flagsChanged: notification is posted.
//-----------------------------------------------------------------------------
- (void)check:(NSEvent*)p ns:(unsigned int)nsmask
    cs:(unsigned long)csmask key:(int)key
    {
    BOOL const new_state = (([p modifierFlags] & nsmask) != 0);
    BOOL const old_state = ((modifiers & csmask) != 0);
    if (new_state != old_state)
	{
	if (new_state)
	    {
	    modifiers |= csmask;
	    driver->SystemExtension( "keydown", key, -1 );
	    }
	else
	    {
	    modifiers &= ~csmask;
	    driver->SystemExtension( "keyup", key, -1 );
	    }
	}
    }


//-----------------------------------------------------------------------------
// Keyboard
//-----------------------------------------------------------------------------
- (void)keyEvent:(NSEvent*)p down:(BOOL)flag
    {
    if (!paused)
	{
	int raw, cooked;
	if ([self classifyKeyDown:p raw:&raw cooked:&cooked])
	    {
	    char const* const request = flag ? "keydown" : "keyup";
	    driver->SystemExtension( request, raw, cooked );
	    }
	}
    }

- (void)keyDown:(NSEvent*)p inView:(NSView*)v
    {
    [self keyEvent:p down:YES];
    }

- (void)keyUp:(NSEvent*)p inView:(NSView*)v
    {
    [self keyEvent:p down:NO];
    }

- (void)flagsChanged:(NSEvent*)p inView:(NSView*)v
    {
    if (!paused)
	{
	[self check:p ns:NSShiftKeyMask     cs:CSMASK_SHIFT key:CSKEY_SHIFT];
	[self check:p ns:NSAlternateKeyMask cs:CSMASK_ALT   key:CSKEY_ALT  ];
	[self check:p ns:NSControlKeyMask   cs:CSMASK_CTRL  key:CSKEY_CTRL ];
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
	    driver->SystemExtension( "mousemoved", x, y );
	}
    }

- (void)mouseUp:(NSEvent*)p inView:(NSView*)v button:(int)button
    {
    if (!paused)
	{
	int x, y;
	[self localize:p toView:v x:&x y:&y];
	driver->SystemExtension( "mouseup", button, x, y );
	}
    }

- (void)mouseDown:(NSEvent*)p inView:(NSView*)v button:(int)button
    {
    if (!paused)
	{
	int x, y;
	[self localize:p toView:v x:&x y:&y];
	driver->SystemExtension( "mousedown", button, x, y );
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
	driver->pause_clock();
	driver->SystemExtension( "appdeactivated" );
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
	driver->resume_clock();
	driver->SystemExtension( "appactivated" );
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
// initWithDriver:
//-----------------------------------------------------------------------------
- (id)initWithDriver:(NeXTSystemDriver*)p
    {
    [super init];
    animationWindow = 0;
    driver = p;
    timer = 0;
    modifiers = 0;
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
