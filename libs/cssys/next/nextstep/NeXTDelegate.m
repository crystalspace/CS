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
#include "NeXTKeymap.h"
#include "cssys/next/NeXTSystemDriver.h"
extern "Objective-C" {
#import <appkit/Application.h>
#import <appkit/View.h>
#import <dpsclient/wraps.h>
}
extern "C" {
#include <string.h>
}
int const TRACK_TAG = 9797;

#if !defined(NX_FUNCTIONSET)
#define NX_FUNCTIONSET 0xfe /* Cousin to NX_ASCIISET and NX_SYMBOLSET. */
#endif

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
    K_TAB		= '\t',		// ascii-set
    K_RETURN		= '\r',		// ascii-set
    K_BACKSPACE		= '\b',		// ascii-set
    K_ESCAPE		= 0x1b,		// ascii-set
    K_DELETE		= 0x7f,		// ascii-set
    K_LEFT		= 0xac,		// symbol-set, numeric-pad mask
    K_UP		= 0xad,		// symbol-set, numeric-pad mask
    K_RIGHT		= 0xae,		// symbol-set, numeric-pad mask
    K_DOWN		= 0xaf,		// symbol-set, numeric-pad mask
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
    K_KP_MULTIPLY	= '*',		// ascii-set, numeric-pad mask
    K_KP_DIVIDE		= '/',		// ascii-set, numeric-pad mask
    K_KP_PLUS		= '+',		// ascii-set, numeric-pad mask
    K_KP_MINUS		= '-',		// symbol-set, numeric-pad mask
    K_KP_ENTER		= 0x03,		// function-set
    K_ED_PAGE_UP	= 0x30,		// function-set
    K_ED_PAGE_DOWN	= 0x31,		// function-set
    K_ED_HOME		= 0x2e,		// function-set
    K_ED_END		= 0x2f,		// function-set
    K_ED_INSERT		= 0x2c,		// function-set
    K_ED_DELETE		= 0x2d,		// function-set
    K_F1		= 0x20,		// function-set
    K_F2		= 0x21,		// function-set
    K_F3		= 0x22,		// function-set
    K_F4		= 0x23,		// function-set
    K_F5		= 0x24,		// function-set
    K_F6		= 0x25,		// function-set
    K_F7		= 0x26,		// function-set
    K_F8		= 0x27,		// function-set
    K_F9		= 0x28,		// function-set
    K_F10		= 0x29,		// function-set
    K_F11		= 0x2a,		// function-set
    K_F12		= 0x2b,		// function-set
    };


//=============================================================================
// IMPLEMENTATION
//=============================================================================
@implementation NeXTDelegate

//-----------------------------------------------------------------------------
// timer_handler
//	Target of timer; forwards timer event to driver; runs in child thread.
//-----------------------------------------------------------------------------
static void timer_handler( DPSTimedEntry, double, void* data )
    {
    ((NeXTSystemDriver*)data)->timer_fired();
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
		driver, NX_BASETHRESHOLD );
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
//	contentView) at a position divisible by 8.
//
//	See also: CS/docs/texinfo/internal/platform/next.txi and the
//	NextStep 3.0 WindowServer release notes.
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
    driver->terminate();
    return self;
    }


//-----------------------------------------------------------------------------
// windowWillClose:
//	Terminate the application when the animation-window closes.
//-----------------------------------------------------------------------------
- (id)windowWillClose:(id)sender
    {
    if (sender == animationWindow)
	[self quit:0];
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
    *y = int(r.size.height - p.y - 1);	// CrystalSpace coords flipped.
    return (*x >= 0 && *y >= 0 && *x < r.size.width && *y < r.size.height);
    }


//-----------------------------------------------------------------------------
// classifyFunctionSet::
//-----------------------------------------------------------------------------
- (void)classifyFunctionSet:(int*)raw :(int*)cooked
    {
    *cooked = -1;
    switch (*raw)
	{
	case K_ED_PAGE_UP:   *raw = CSKEY_PGUP; break;
	case K_ED_PAGE_DOWN: *raw = CSKEY_PGDN; break;
	case K_ED_HOME:      *raw = CSKEY_HOME; break;
	case K_ED_END:       *raw = CSKEY_END;  break;
	case K_ED_INSERT:    *raw = CSKEY_INS;  break;
	case K_ED_DELETE:    *raw = CSKEY_DEL;  break;
	case K_F1:           *raw = CSKEY_F1;   break;
	case K_F2:           *raw = CSKEY_F2;   break;
	case K_F3:           *raw = CSKEY_F3;   break;
	case K_F4:           *raw = CSKEY_F4;   break;
	case K_F5:           *raw = CSKEY_F5;   break;
	case K_F6:           *raw = CSKEY_F6;   break;
	case K_F7:           *raw = CSKEY_F7;   break;
	case K_F8:           *raw = CSKEY_F8;   break;
	case K_F9:           *raw = CSKEY_F9;   break;
	case K_F10:          *raw = CSKEY_F10;  break;
	case K_F11:          *raw = CSKEY_F11;  break;
	case K_F12:          *raw = CSKEY_F12;  break;
	}
    }


//-----------------------------------------------------------------------------
// classifyAsciiSet::
//	*NOTE* The so-called "backspace" key on the keyboard actually sends
//	DEL, however Crystal Space would like to see it as CSKEY_BACKSPACE.
//-----------------------------------------------------------------------------
- (void)classifyAsciiSet:(int*)raw :(int*)cooked
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
// classifyKeypad::
//-----------------------------------------------------------------------------
- (void)classifyKeypad:(int*)raw :(int*)cooked
    {
    switch (*raw)
	{
	case K_LEFT:         *raw = CSKEY_LEFT;  *cooked = -1;   break;
	case K_RIGHT:        *raw = CSKEY_RIGHT; *cooked = -1;   break;
	case K_UP:           *raw = CSKEY_UP;    *cooked = -1;   break;
	case K_DOWN:         *raw = CSKEY_DOWN;  *cooked = -1;   break;
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
// classifyKeyDown:raw:cooked: -- Translate NextStep keystroke to CrystalSpace.
//-----------------------------------------------------------------------------
- (BOOL)classifyKeyDown:(NXEvent const*)p raw:(int*)raw cooked:(int*)cooked
    {
    BOOL ok = NO;
    *raw = *cooked = 0;
    if ((p->flags & NX_COMMANDMASK) == 0)
	{
	NeXTKeymap::Binding const& binding =
	    keymap->binding_for_scan_code( p->data.key.keyCode );
	if (binding.is_bound())
	    {
	    *raw = binding.code;
	    *cooked = p->data.key.charCode;
	    if ((p->flags & NX_NUMERICPADMASK) != 0)
		[self classifyKeypad:raw:cooked];
	    else if (binding.character_set == NX_FUNCTIONSET)
		[self classifyFunctionSet:raw:cooked];
	    else if (binding.character_set == NX_ASCIISET)
		[self classifyAsciiSet:raw:cooked];
	    ok = YES;
	    }
	}
    return ok;
    }


//-----------------------------------------------------------------------------
// check:nx:cs:key:
//	Track state of modifier (shift, alt, ctrl) keys.  NextStep does not
//	supply key up/down events for modifier flags so simulate these events
//	whenever a -flagsChanged: notification is posted.
//-----------------------------------------------------------------------------
- (void)check:(NXEvent const*)p nx:(unsigned long)nxmask
    cs:(unsigned long)csmask key:(int)key
    {
    BOOL const new_state = ((p->flags  & nxmask) != 0);
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
- (void)keyEvent:(NXEvent*)p down:(BOOL)flag
    {
    if (!paused)
	{
	int raw, cooked;
	if ([self classifyKeyDown:p raw:&raw cooked:&cooked])
	    {
	    char const* request = flag ? "keydown" : "keyup";
	    driver->SystemExtension( request, raw, cooked );
	    }
	}
    }

- (void)keyDown:(NXEvent*)p inView:(View*)v
    {
    [self keyEvent:p down:YES];
    }

- (void)keyUp:(NXEvent*)p inView:(View*)v
    {
    [self keyEvent:p down:NO];
    }

- (void)flagsChanged:(NXEvent*)p inView:(View*)v
    {
    if (!paused)
	{
	[self check:p nx:NX_SHIFTMASK     cs:CSMASK_SHIFT key:CSKEY_SHIFT];
	[self check:p nx:NX_ALTERNATEMASK cs:CSMASK_ALT   key:CSKEY_ALT  ];
	[self check:p nx:NX_CONTROLMASK   cs:CSMASK_CTRL  key:CSKEY_CTRL ];
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
	    driver->SystemExtension( "mousemoved", x, y );
	}
    }

- (void)mouseUp:(NXEvent*)p inView:(View*)v button:(int)button
    {
    if (!paused)
	{
	int x, y;
	[self localize:p toView:v x:&x y:&y];
	driver->SystemExtension( "mouseup", button, x, y );
	}
    }

- (void)mouseDown:(NXEvent*)p inView:(View*)v button:(int)button
    {
    if (!paused)
	{
	int x, y;
	[self localize:p toView:v x:&x y:&y];
	driver->SystemExtension( "mousedown", button, x, y );
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
	driver->pause_clock();
	driver->SystemExtension( "appdeactivated" );
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
	driver->resume_clock();
	driver->SystemExtension( "appactivated" );
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
// initWithDriver:
//-----------------------------------------------------------------------------
- (id)initWithDriver:(NeXTSystemDriver*)p
    {
    [super init];
    animationWindow = 0;
    oldEventMask = 0;
    driver = p;
    keymap = new NeXTKeymap;
    timer = 0;
    modifiers = 0;
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
    delete keymap;
    return [super free];
    }

@end
