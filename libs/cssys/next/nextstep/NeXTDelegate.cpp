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
#import <appkit/Application.h>
#import <appkit/View.h>
#import <dpsclient/wraps.h>
}
extern "C" {
#import <string.h>
}
int const TRACK_TAG = 9797;

//-----------------------------------------------------------------------------
// Keystrokes which must be translated to CrystalSpace-specific codes.
//-----------------------------------------------------------------------------
enum
    {
    K_TAB		= '\t',		// ascii-set
    K_RETURN		= '\r',		// ascii-set
    K_BACKSPACE		= '\b',		// ascii-set
    K_ESCAPE		= 0x1b,		// ascii-set
    K_DELETE		= 0x7f,		// ascii-set
    K_LEFT		= 0xac,		// symbol-set
    K_UP		= 0xad,		// symbol-set
    K_RIGHT		= 0xae,		// symbol-set
    K_DOWN		= 0xaf,		// symbol-set
    K_KP_CENTER		= '5',		// ascii-set, numeric-pad mask
    K_KP_LEFT		= '4',		// ascii-set, numeric-pad mask
    K_KP_UP		= '8',		// ascii-set, numeric-pad mask
    K_KP_RIGHT		= '6',		// ascii-set, numeric-pad mask
    K_KP_DOWN		= '2',		// ascii-set, numeric-pad mask
    K_KP_PAGE_UP	= '9',		// ascii-set, numeric-pad mask
    K_KP_PAGE_DOWN	= '3',		// ascii-set, numeric-pad mask
    K_KP_HOME		= '7',		// ascii-set, numeric-pad mask
    K_KP_END		= '1',		// ascii-set, numeric-pad mask
    K_KP_INSERT		= '0',		// ascii-set, numeric-pad mask
    K_KP_DELETE		= '.',		// ascii-set, numeric-pad mask
    K_KP_ENTER		= 0x03,		// 254-set
    K_ED_PAGE_UP	= 0x30,		// 254-set
    K_ED_PAGE_DOWN	= 0x31,		// 254-set
    K_ED_HOME		= 0x2e,		// 254-set
    K_ED_END		= 0x2f,		// 254-set
    K_ED_INSERT		= 0x2c,		// 254-set
    K_ED_DELETE		= 0x2d,		// 254-set
    };

//=============================================================================
// IMPLEMENTATION
//=============================================================================
@implementation NeXTDelegate

//-----------------------------------------------------------------------------
// timer_handler
//	Target of timer.  Forwards timer event to proxy.  Runs in child thread.
//-----------------------------------------------------------------------------
static void timer_handler( DPSTimedEntry, double, void* data )
    {
    ((NeXTSystemProxy*)data)->timer_fired();
    }


//-----------------------------------------------------------------------------
// startTimer
//-----------------------------------------------------------------------------
- (void)startTimer
    {
    if (timer == 0)
	{
	float const CS_FPS = 15;
	timer = DPSAddTimedEntry( 1.0 / CS_FPS, timer_handler,
		proxy, NX_BASETHRESHOLD );
	}
    }


//-----------------------------------------------------------------------------
// stopTimer
//-----------------------------------------------------------------------------
- (void)stopTimer
    {
    if (timer != 0)
	{
	DPSRemoveTimedEntry( timer );
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
- (void)startTracking:(Window*)w
    {
    if (!tracking)
	{
	tracking = YES;
	View* const v = [w contentView];
	NXRect r; [v getBounds:&r];
	[v convertRect:&r toView:0];
	[w setTrackingRect:&r inside:NO owner:self tag:TRACK_TAG
	    left:NO right:NO];
	}
    }


//-----------------------------------------------------------------------------
// stopTracking:
//-----------------------------------------------------------------------------
- (void)stopTracking:(Window*)w
    {
    if (tracking)
	{
	tracking = NO;
	[w removeFromEventMask:NX_MOUSEMOVEDMASK];
	[w discardTrackingRect:TRACK_TAG];
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
	PSshowcursor();
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
	PShidecursor();
	}
    }


//-----------------------------------------------------------------------------
// freeWindowTitle
//-----------------------------------------------------------------------------
- (void)freeWindowTitle
    {
    if (savedTitle != 0)
	{
	free( savedTitle );
	savedTitle = 0;
	}
    }


//-----------------------------------------------------------------------------
// copyWindowTitleWithExtraBytes:
//-----------------------------------------------------------------------------
- (char*)copyWindowTitleWithExtraBytes:(int)extra
    {
    char const* title = [animationWindow title];
    char* s = (char*)malloc( strlen(title) + extra + 1 );
    strcpy( s, title );
    return s;
    }


//-----------------------------------------------------------------------------
// saveWindowTitle
//-----------------------------------------------------------------------------
- (void)saveWindowTitle
    {
    [self freeWindowTitle];
    savedTitle = [self copyWindowTitleWithExtraBytes:0];
    }


//-----------------------------------------------------------------------------
// restoreWindowTitle
//-----------------------------------------------------------------------------
- (void)restoreWindowTitle
    {
    if (savedTitle != 0)
	{
	[animationWindow setTitle:savedTitle];
	[self freeWindowTitle];
	}
    }


//-----------------------------------------------------------------------------
// appendToWindowTitle:
//-----------------------------------------------------------------------------
- (void)appendToWindowTitle:(char const*)s
    {
    char* title = [self copyWindowTitleWithExtraBytes:strlen(s)];
    strcat( title, s );
    [animationWindow setTitle:title];
    free( title );
    }


//-----------------------------------------------------------------------------
// adjustWindowPosition:
//	For best video performance align left-edge of NeXTView (the window's
//	contentView) at a position divisible by 8.  See also: README.NeXT and
//	the NextStep 3.0 WindowServer release notes.
//-----------------------------------------------------------------------------
- (void)adjustWindowPosition:(Window*)w
    {
    View* v = [w contentView];		// The NeXTView.
    NXRect vr; [v getBounds:&vr];
    [v convertPoint:&vr.origin toView:0];
    [w convertBaseToScreen:&vr.origin];
    int const ALIGN = (1 << 3) - 1;	// 8-pixel alignment
    int x = int( vr.origin.x );
    if ((x & ALIGN) != 0)
	{
	x &= ~ALIGN;
	NXRect wr; [w getFrame:&wr];
	float const dx = vr.origin.x - wr.origin.x;
	[w moveTo:(x - dx):wr.origin.y];
	}
    }


//-----------------------------------------------------------------------------
// prepareWindow:
//	Register for additional events we are interested in: flags-changed, 
//	mouse-dragged.  Also set up a tracking area so that we can request 
//	mouse-moved events when mouse is inside the window's content area.  
//-----------------------------------------------------------------------------
- (void)prepareWindow:(Window*)w
    {
    if (w != 0)
	{
	int const MASK =
		NX_FLAGSCHANGEDMASK  |
		NX_LMOUSEDRAGGEDMASK |
		NX_RMOUSEDRAGGEDMASK;
	oldEventMask = [w addToEventMask:MASK];
	[self startTracking:w];
	[self adjustWindowPosition:w];
	[w setDelegate:self];
	}
    }


//-----------------------------------------------------------------------------
// unprepareWindow:
//-----------------------------------------------------------------------------
- (void)unprepareWindow:(Window*)w
    {
    if (w != 0)
	{
	[self stopTracking:w];
	[w setDelegate:0];
	[w setEventMask:oldEventMask];
	}
    }


//-----------------------------------------------------------------------------
// registerAnimationWindow:
//-----------------------------------------------------------------------------
- (void)registerAnimationWindow:(Window*)w
    {
    [self unprepareWindow:animationWindow];
    animationWindow = w;
    [self prepareWindow:animationWindow];
    [self startTimer];
    }


//-----------------------------------------------------------------------------
// quit: -- Terminate the application.
//-----------------------------------------------------------------------------
- (id)quit:(id)sender
    {
    [self showMouse];
    [self stopTimer];
    [self unprepareWindow:animationWindow];
    [animationWindow close];
    proxy->terminate();
    return self;
    }


//-----------------------------------------------------------------------------
// windowWillClose:
//	Terminate the application when the animation-window closes.
//-----------------------------------------------------------------------------
- (id)windowWillClose:(id)sender
    {
    if (sender == animationWindow)
	[self quit:self];
    return self;
    }


//-----------------------------------------------------------------------------
// windowDidMove:
//-----------------------------------------------------------------------------
- (id)windowDidMove:(id)sender
    {
    if (sender == animationWindow)
	[self adjustWindowPosition:animationWindow];
    return self;
    }


//-----------------------------------------------------------------------------
// localize:toView:x:y: -- Convert event location to view coordinate system.
//-----------------------------------------------------------------------------
- (BOOL)localize:(NXEvent const*)event toView:(View*)view x:(int*)x y:(int*)y
    {
    NXPoint p = event->location;
    NXRect r; [view getBounds:&r];
    [view convertPoint:&p fromView:0];
    *x = int(p.x);
    *y = int(r.size.height - p.y);	// CrystalSpace uses flipped coords.
    return (*x >= 0 && *y >= 0 && *x < r.size.width && *y < r.size.height);
    }


//-----------------------------------------------------------------------------
// classifyOtherSet:
//-----------------------------------------------------------------------------
- (int)classifyOtherSet:(int)c
    {
    int k = 0;
    switch (c)
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
// classifyAsciiSet:
//	*NOTE* CrystalSpace wants control-keys translated to lower-case
//	equivalents; that is: 'ctrl-c' --> 'c'.
//-----------------------------------------------------------------------------
- (int)classifyAsciiSet:(int)c
    {
    int k = 0;
    switch (c)
	{
	case K_ESCAPE:    k = CSKEY_ESC;       break;
	case K_RETURN:    k = CSKEY_ENTER;     break;
	case K_TAB:       k = CSKEY_TAB;       break;
	case K_BACKSPACE: k = CSKEY_BACKSPACE; break;
	case K_DELETE:    k = CSKEY_BACKSPACE; break;
	default: k = (c < ' ' ? c + '`' : c);  break;	// *NOTE*
	}
    return k;
    }


//-----------------------------------------------------------------------------
// classifyKeypad:
//-----------------------------------------------------------------------------
- (int)classifyKeypad:(int)c
    {
    int k = 0;
    switch (c)
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
// classifyKeyDown: -- Translate NeXT keystroke to CrystalSpace.
//-----------------------------------------------------------------------------
- (int)classifyKeyDown:(NXEvent const*)p
    {
    int k = 0;
    if ((p->flags & NX_COMMANDMASK) == 0)
	{
	int const c = p->data.key.charCode;
	if (p->data.key.charSet != NX_ASCIISET)
	    k = [self classifyOtherSet:c];
	else if ((p->flags & NX_NUMERICPADMASK) != 0)
	    k = [self classifyKeypad:c];
	else
	    k = [self classifyAsciiSet:c];
	}
    return k;
    }


//-----------------------------------------------------------------------------
// check:key:mask:flag:
//	Track state of modifier (shift, alt, ctrl) keys.  NextStep does not
//	supply key up/down events for modifier flags so simulate these events
//	whenever a -flagsChanged: notification is posted.
//-----------------------------------------------------------------------------
- (void)check:(NXEvent const*)p key:(int)key mask:(int)mask flag:(BOOL*)flag
    {
    BOOL const state = ((p->flags & mask) != 0);
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
- (void)keyDown:(NXEvent*)p inView:(View*)v
    {
    if (!paused)
	proxy->key_down( [self classifyKeyDown:p] );
    }

- (void)keyUp:(NXEvent*)p inView:(View*)v
    {
    if (!paused)
	proxy->key_up( [self classifyKeyDown:p] );
    }

- (void)flagsChanged:(NXEvent*)p inView:(View*)v
    {
    if (!paused)
	{
	[self check:p key:CSKEY_SHIFT mask:NX_SHIFTMASK     flag:&stateShift];
	[self check:p key:CSKEY_ALT   mask:NX_ALTERNATEMASK flag:&stateAlt  ];
	[self check:p key:CSKEY_CTRL  mask:NX_CONTROLMASK   flag:&stateCtrl ];
	}
    }


//-----------------------------------------------------------------------------
// Mouse -- Note: CrystalSpace button numbers start at 1.
//-----------------------------------------------------------------------------
- (void)mouseEntered:(NXEvent*)p
    {
    if (!paused && p->data.tracking.trackingNum == TRACK_TAG)
	[animationWindow addToEventMask:NX_MOUSEMOVEDMASK];
    }

- (void)mouseExited: (NXEvent*)p
    {
    if (p->data.tracking.trackingNum == TRACK_TAG)
	{
	[animationWindow removeFromEventMask:NX_MOUSEMOVEDMASK];
	[self showMouse];
	}
    }

- (void)mouseMoved:(NXEvent*)p inView:(View*)v
    {
    if (!paused)
	{
	int x, y;
	if ([self localize:p toView:v x:&x y:&y])
	    proxy->mouse_moved( x, y );
	}
    }

- (void)mouseUp:(NXEvent*)p inView:(View*)v button:(int)button
    {
    if (!paused)
	{
	int x, y;
	[self localize:p toView:v x:&x y:&y];
	proxy->mouse_up( button, x, y );
	}
    }

- (void)mouseDown:(NXEvent*)p inView:(View*)v button:(int)button
    {
    if (!paused)
	{
	int x, y;
	[self localize:p toView:v x:&x y:&y];
	proxy->mouse_down( button, x, y, stateShift, stateAlt, stateCtrl );
	}
    }

- (void)mouseDragged:(NXEvent*)p inView:(View*)v
    { [self mouseMoved:p inView:v]; }

- (void)mouseUp:(NXEvent*)p inView:(View*)v
    { [self mouseUp:p inView:v button:1]; }

- (void)mouseDown:(NXEvent*)p inView:(View*)v
    { [self mouseDown:p inView:v button:1]; }

- (void)rightMouseDragged:(NXEvent*)p inView:(View*)v
    { [self mouseMoved:p inView:v]; }

- (void)rightMouseUp:(NXEvent*)p inView:(View*)v
    { [self mouseUp:p inView:v button:2]; }

- (void)rightMouseDown:(NXEvent*)p inView:(View*)v
    {
    if (paused)
	[[v nextResponder] rightMouseDown:p];	// Allow main menu to pop up.
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
	[self saveWindowTitle];
	[self appendToWindowTitle:"  [Paused]"];
	proxy->focus_changed( false );
	proxy->clock_running( false );
	}
    }

- (void)unpause
    {
    if (paused)
	{
	paused = NO;
	[self restoreWindowTitle];
	[self startTracking:animationWindow];
	[self startTimer];
	proxy->focus_changed( true );
	proxy->clock_running( true );
	}
    }

- (id)togglePause:(id)sender
    {
    if (paused)
	[self unpause];
    else
	[self pause];
    }

- (id)windowDidBecomeKey:(id)sender
    {
    if (autoResume && sender == animationWindow)
	[self unpause];
    }

- (id)windowDidResignKey:(id)sender
    {
    if (sender == animationWindow)
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
    oldEventMask = 0;
    proxy = p;
    timer = 0;
    stateShift = NO;
    stateAlt   = NO;
    stateCtrl  = NO;
    mouseHidden = NO;
    paused = NO;
    tracking = NO;
    savedTitle = 0;
    return self;
    }


//-----------------------------------------------------------------------------
// free
//-----------------------------------------------------------------------------
- (id)free
    {
    [self stopTimer];
    [self freeWindowTitle];
    return [super free];
    }

@end
