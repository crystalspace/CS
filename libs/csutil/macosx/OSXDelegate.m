//=============================================================================
//
//	Copyright (C)1999-2003 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXDelegate.m
//
//	An object which acts as a gateway between the AppKit and Crystal Space
//	by forwarding Objective-C messages and events to the C++ platform-
//	specific assistant, OSXAssistant.  Also acts as a listener for
//	interesting notifications from the AppKit.
//
//-----------------------------------------------------------------------------
#include "OSXDelegate.h"
#include "OSXMenu.h"
#include "iutil/evdefs.h"
#include "csconfig.h"		// CS_PLATFORM_MACOSXS
#include <stdio.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSGraphicsContext.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSView.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSString.h>
#import <Foundation/NSUserDefaults.h>

typedef void* OSXDelegateHandle;
typedef void* OSXEventHandle;
typedef void* OSXViewHandle;
#define ND_PROTO(RET,FUNC) RET OSXDelegate_##FUNC

static NSAutoreleasePool* CS_GLOBAL_POOL = 0;

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
// Keystrokes which must be translated to Crystal Space-specific codes.
//-----------------------------------------------------------------------------
enum
{
  K_TAB          = '\t',
  K_RETURN       = '\r',
  K_BACKSPACE    = '\b',
  K_ESCAPE       = 0x1b,
  K_DELETE       = 0x7f,
  K_LEFT         = NSLeftArrowFunctionKey,
  K_UP           = NSUpArrowFunctionKey,
  K_RIGHT        = NSRightArrowFunctionKey,
  K_DOWN         = NSDownArrowFunctionKey,
  K_KP_CENTER    = '5',
  K_KP_LEFT      = '4',
  K_KP_UP        = '8',
  K_KP_RIGHT     = '6',
  K_KP_DOWN      = '2',
  K_KP_PAGE_UP   = '9',
  K_KP_PAGE_DOWN = '3',
  K_KP_HOME      = '7',
  K_KP_END       = '1',
  K_KP_INSERT    = '0',
  K_KP_DELETE    = '.',
  K_KP_MULTIPLY  = '*',
  K_KP_DIVIDE    = '/',
  K_KP_PLUS      = '+',
  K_KP_MINUS     = '-',
  K_KP_ENTER     = 0x03,
  K_ED_PAGE_UP   = NSPageUpFunctionKey,
  K_ED_PAGE_DOWN = NSPageDownFunctionKey,
  K_ED_HOME      = NSHomeFunctionKey,
  K_ED_END       = NSEndFunctionKey,
  K_ED_INSERT    = NSInsertFunctionKey,
  K_ED_DELETE    = NSDeleteFunctionKey,
  K_F1		 = NSF1FunctionKey,
  K_F2		 = NSF2FunctionKey,
  K_F3		 = NSF3FunctionKey,
  K_F4		 = NSF4FunctionKey,
  K_F5		 = NSF5FunctionKey,
  K_F6		 = NSF6FunctionKey,
  K_F7		 = NSF7FunctionKey,
  K_F8		 = NSF8FunctionKey,
  K_F9		 = NSF9FunctionKey,
  K_F10		 = NSF10FunctionKey,
  K_F11		 = NSF11FunctionKey,
  K_F12		 = NSF12FunctionKey,
  };


//=============================================================================
// Subclass of NSApplication which provides an -applicationDefined: delegate
// message much like the one provided by the old NextStep Application class.
//=============================================================================
#define OSXAppDefinedNotification @"OSXAppDefinedNotification"
#define OSXAppDefinedSubType 0x3d8a
#define OSXAppDefinedData1 0x1827
#define OSXAppDefinedData2 0x4851

@interface OSXApplication : NSApplication
- (void)sendEvent:(NSEvent*)e;
@end
@implementation OSXApplication
- (void)sendEvent:(NSEvent*)e
{
  if ([e type   ] == NSApplicationDefined &&
      [e subtype] == OSXAppDefinedSubType &&
      [e data1  ] == OSXAppDefinedData1   &&
      [e data2  ] == OSXAppDefinedData2)
    [[NSNotificationCenter defaultCenter]
      postNotificationName:OSXAppDefinedNotification object:0];
  else
    [super sendEvent:e];
}
@end


//=============================================================================
// OSXDelegate Implementation
//=============================================================================
@implementation OSXDelegate

//-----------------------------------------------------------------------------
// scheduleEvent
//	Insert a do-nothing event at the tail end of the AppKit's event queue.
//	This event results in an invocation of -applicationDefined: (below).
//-----------------------------------------------------------------------------
- (void)scheduleEvent
{
  [NSApp postEvent:
    [NSEvent otherEventWithType:NSApplicationDefined
      location:NSMakePoint(0,0)
      modifierFlags:0
      timestamp:0
      windowNumber:0
      context:[NSApp context]
      subtype:OSXAppDefinedSubType
      data1:OSXAppDefinedData1
      data2:OSXAppDefinedData2]
    atStart:NO];
}


//-----------------------------------------------------------------------------
// applicationDefined:
//	Crystal Space applications are typically game-oriented and demand a
//	high frame rate.  This means that iSystem::NextFrame() expects to be
//	called as often as possible.  One approach is to use an NSTimer to
//	generate a steady flow of calls to NextFrame(), however such timers run
//	only at a fixed interval and would therefore artificially limit the
//	frame rate to that interval.  Instead, we employ the technique of
//	inserting a dummy event at the tail end of the AppKit's event queue
//	after each invocation of NextFrame().  This event results in an call to
//	this method, -applicationDefined:, which invokes the system extension
//	"advancestate" which invokes NextFrame() and then inserts another dummy
//	event into the event queue.  In this fashion, Crystal Space
//	applications can run at the highest frame rate supported by the
//	particular hardware rather than being artificially limited by a
//	fixed-interval timer.  Because the dummy event is inserted at the tail
//	end of the AppKit's event queue, the AppKit is still able to process
//	other events, such as those generated by the keyboard or mouse.
//-----------------------------------------------------------------------------
- (void)applicationDefined:(NSNotification*)n
{
  OSXAssistant_advance_state(assistant);
  if ((OSXAssistant_always_runs(assistant) || !paused) &&
     OSXAssistant_continue_running(assistant))
    [self scheduleEvent];
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

ND_PROTO(void,show_mouse_pointer)(OSXDelegateHandle handle)
  { [(OSXDelegate*)handle showMouse]; }


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

ND_PROTO(void,hide_mouse_pointer)(OSXDelegateHandle handle)
  { [(OSXDelegate*)handle hideMouse]; }


//-----------------------------------------------------------------------------
// flushGraphicsContext
//-----------------------------------------------------------------------------
- (void)flushGraphicsContext
{
  [[NSGraphicsContext currentContext] flushGraphics];
}

ND_PROTO(void,flush_graphics_context)(OSXDelegateHandle handle)
  { [(OSXDelegate*)handle flushGraphicsContext]; }


//-----------------------------------------------------------------------------
// quit:
//	Target of "Quit" menu item.  Terminate the application.
//-----------------------------------------------------------------------------
- (void)quit:(id)sender
{
  OSXAssistant_request_shutdown(assistant);
}


//-----------------------------------------------------------------------------
// localize:toView:x:y:
//	Convert event location to view coordinate system.
//-----------------------------------------------------------------------------
- (BOOL)localize:(NSEvent*)event toView:(NSView*)view x:(int*)x y:(int*)y
{
  NSPoint const p = [view convertPoint:[event locationInWindow] fromView:0];
  NSRect const r = [view bounds];
  *x = (int)p.x;
  *y = (int)(r.size.height - p.y - 1); // Crystal Space coordinates flipped.
  return (*x >= 0 && *y >= 0 && *x < r.size.width && *y < r.size.height);
}


//-----------------------------------------------------------------------------
// classifyFunctionKey::
//-----------------------------------------------------------------------------
- (void)classifyFunctionKey:(unsigned int*)raw :(unsigned int*)cooked
{
  unsigned int k = (unsigned int)~0;
  switch (*raw)
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
    case K_F1:           k = CSKEY_F1;    break;
    case K_F2:           k = CSKEY_F2;    break;
    case K_F3:           k = CSKEY_F3;    break;
    case K_F4:           k = CSKEY_F4;    break;
    case K_F5:           k = CSKEY_F5;    break;
    case K_F6:           k = CSKEY_F6;    break;
    case K_F7:           k = CSKEY_F7;    break;
    case K_F8:           k = CSKEY_F8;    break;
    case K_F9:           k = CSKEY_F9;    break;
    case K_F10:          k = CSKEY_F10;   break;
    case K_F11:          k = CSKEY_F11;   break;
    case K_F12:          k = CSKEY_F12;   break;
  }
  if (k != (unsigned int)~0)
  {
    *raw = k;
    *cooked = k;
  }
}


//-----------------------------------------------------------------------------
// classifyOtherKey::
//
// *1* For the cooked code, Crystal Space applications would like to see
//     everything that looks like a Return or Enter mapped to CSKEY_ENTER.
// *2* The so-called "backspace" key on the keyboard actually sends DEL,
//     however Crystal Space would like to see it as CSKEY_BACKSPACE.
// *3* For 7-bit alphabetic characters, Crystal Space prefers that the raw
//     code be lowercase.
//-----------------------------------------------------------------------------
- (void)classifyOtherKey:(unsigned int*)raw :(unsigned int*)cooked
{
  switch (*raw)
  {
    case K_ESCAPE:    *cooked = CSKEY_ESC;       break;
    case K_RETURN:    *cooked = CSKEY_ENTER;     break;	// *1*
    case K_TAB:       *cooked = CSKEY_TAB;       break;
    case K_BACKSPACE: *cooked = CSKEY_BACKSPACE; break;
    case K_DELETE:    *cooked = CSKEY_BACKSPACE; break; // *2*
    default:
      if (*raw <= 0x7f) // Is it 7-bit ASCII?
	*raw = CS_DOWN_CASE[ *raw ];			// *3*
      break;
  }
}


//-----------------------------------------------------------------------------
// classifyNumericPadKey::
//-----------------------------------------------------------------------------
- (void)classifyNumericPadKey:(unsigned int*)raw :(unsigned int*)cooked
{
  unsigned int k = (unsigned int)~0;
  switch (*raw)
  {
    case K_KP_CENTER:    k = CSKEY_CENTER;   break;
    case K_KP_LEFT:      k = CSKEY_LEFT;     break;
    case K_KP_UP:        k = CSKEY_UP;       break;
    case K_KP_RIGHT:     k = CSKEY_RIGHT;    break;
    case K_KP_DOWN:      k = CSKEY_DOWN;     break;
    case K_KP_PAGE_UP:   k = CSKEY_PGUP;     break;
    case K_KP_PAGE_DOWN: k = CSKEY_PGDN;     break;
    case K_KP_HOME:      k = CSKEY_HOME;     break;
    case K_KP_END:       k = CSKEY_END;      break;
    case K_KP_INSERT:    k = CSKEY_INS;      break;
    case K_KP_DELETE:    k = CSKEY_DEL;      break;
    case K_KP_MULTIPLY:  k = CSKEY_PADMULT;  break;
    case K_KP_DIVIDE:    k = CSKEY_PADDIV;   break;
    case K_KP_PLUS:      k = CSKEY_PADPLUS;  break;
    case K_KP_MINUS:     k = CSKEY_PADMINUS; break;
    case K_KP_ENTER:     k = CSKEY_ENTER;    break;
  }
  if (k != (unsigned int)~0)
  {
    *raw = k;
    *cooked = k;
  }
}


//-----------------------------------------------------------------------------
// classifyKeyDown:raw:cooked:
//	Translate OpenStep keystroke to Crystal Space codes.
//-----------------------------------------------------------------------------
- (BOOL)classifyKeyDown:(NSEvent*)p
  raw:(unsigned int*)raw cooked:(unsigned int*)cooked
{
  BOOL ok = NO;
  unsigned int const flags = [p modifierFlags];
  *raw = *cooked = 0;
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
  cs:(unsigned long)csmask key:(unsigned int)key
{
  BOOL const new_state = (([p modifierFlags] & nsmask) != 0);
  BOOL const old_state = ((modifiers & csmask) != 0);
  if (new_state != old_state)
  {
    if (new_state)
    {
      modifiers |= csmask;
      OSXAssistant_key_down(assistant, key, key);
    }
    else
    {
      modifiers &= ~csmask;
      OSXAssistant_key_up(assistant, key, key);
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
    unsigned int raw, cooked;
    if ([self classifyKeyDown:p raw:&raw cooked:&cooked])
    {
      if (flag)
        OSXAssistant_key_down(assistant, raw, cooked);
      else
        OSXAssistant_key_up(assistant, raw, cooked);
    }
  }
}

- (void)keyDown:(NSEvent*)p forView:(NSView*)v
{
  [self keyEvent:p down:YES];
}

- (void)keyUp:(NSEvent*)p forView:(NSView*)v
{
  [self keyEvent:p down:NO];
}

- (void)flagsChanged:(NSEvent*)p forView:(NSView*)v
{
  if (!paused)
  {
    [self check:p ns:NSShiftKeyMask     cs:CSMASK_SHIFT key:CSKEY_SHIFT];
    [self check:p ns:NSAlternateKeyMask cs:CSMASK_ALT   key:CSKEY_ALT  ];
    [self check:p ns:NSControlKeyMask   cs:CSMASK_CTRL  key:CSKEY_CTRL ];
  }
}


//-----------------------------------------------------------------------------
// Mouse -- Note: Crystal Space button numbers start at 1.
//-----------------------------------------------------------------------------
- (void)mouseMoved:(NSEvent*)p forView:(NSView*)v
{
  if (!paused)
  {
    int x, y;
    if ([self localize:p toView:v x:&x y:&y])
      OSXAssistant_mouse_moved(assistant, x, y);
  }
}

- (void)mouseUp:(NSEvent*)p forView:(NSView*)v button:(int)button
{
  if (!paused)
  {
    int x, y;
    [self localize:p toView:v x:&x y:&y];
    OSXAssistant_mouse_up(assistant, button, x, y);
  }
}

- (void)mouseDown:(NSEvent*)p forView:(NSView*)v button:(int)button
{
  if (!paused)
  {
    int x, y;
    [self localize:p toView:v x:&x y:&y];
    OSXAssistant_mouse_down(assistant, button, x, y);
  }
}

- (void)mouseDragged:(NSEvent*)p forView:(NSView*)v
  { [self mouseMoved:p forView:v]; }

- (void)mouseUp:(NSEvent*)p forView:(NSView*)v
  { [self mouseUp:p forView:v button:1]; }

- (void)mouseDown:(NSEvent*)p forView:(NSView*)v
  { [self mouseDown:p forView:v button:1]; }

- (void)rightMouseDragged:(NSEvent*)p forView:(NSView*)v
  { [self mouseMoved:p forView:v]; }

- (void)rightMouseUp:(NSEvent*)p forView:(NSView*)v
  { [self mouseUp:p forView:v button:2]; }

- (void)rightMouseDown:(NSEvent*)p forView:(NSView*)v
{
  if (paused)
    [[v nextResponder] rightMouseDown:p]; // Allow main menu to pop up.
  else
    [self mouseDown:p forView:v button:2];
}

- (void)dispatchEvent:(NSEvent*)e forView:(NSView*)v
{
  switch ([e type])
  {
    case NSKeyDown:           [self keyDown:e           forView:v]; break;
    case NSKeyUp:             [self keyUp:e             forView:v]; break;
    case NSFlagsChanged:      [self flagsChanged:e      forView:v]; break;
    case NSMouseMoved:        [self mouseMoved:e        forView:v]; break;
    case NSLeftMouseDown:     [self mouseDown:e         forView:v]; break;
    case NSLeftMouseUp:       [self mouseUp:e           forView:v]; break;
    case NSLeftMouseDragged:  [self mouseDragged:e      forView:v]; break;
    case NSRightMouseDown:    [self rightMouseDown:e    forView:v]; break;
    case NSRightMouseUp:      [self rightMouseUp:e      forView:v]; break;
    case NSRightMouseDragged: [self rightMouseDragged:e forView:v]; break;
    default:                                                        break;
  }
}

ND_PROTO(void,dispatch_event)
  (OSXDelegateHandle h, OSXEventHandle event, OSXViewHandle view)
  { [(OSXDelegate*)h dispatchEvent:(NSEvent*)event forView:(NSView*)view]; }


//-----------------------------------------------------------------------------
// Activation / Deactivation / Hiding / Revealing
//-----------------------------------------------------------------------------
- (void)pause
{
  if (!paused)
  {
    paused = YES;
    [self showMouse];
    OSXAssistant_application_deactivated(assistant);
  }
}

- (void)unpause
{
  if (paused)
  {
    paused = NO;
    [self scheduleEvent];
    OSXAssistant_application_activated(assistant);
  }
}

- (void)togglePause:(id)sender
{
  if (paused)
    [self unpause];
  else
    [self pause];
}

- (void)applicationDidBecomeActive:(NSNotification*)n
{
  if (autoResume)
    [self unpause];
}

- (void)applicationDidResignActive:(NSNotification*)n
{
  autoResume = !paused;
  [self pause];
}

- (void)applicationDidHide: (NSNotification*)n
{
  OSXAssistant_application_hidden(assistant);
}

- (void)applicationDidUnhide: (NSNotification*)n
{
  OSXAssistant_application_unhidden(assistant);
}


//-----------------------------------------------------------------------------
// initApplicationMenu:style:
//	Generate application menu based upon platform configuration.
//-----------------------------------------------------------------------------
- (void)initApplicationMenu:(OSXConfigHandle)config style:(char const*)style
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSMenu* const menu = OSXMenuGenerate(self, style, config);
  [menu setTitle:[[NSProcessInfo processInfo] processName]];
  [NSApp setMainMenu:menu];
  [pool release];
}

ND_PROTO(void,init_app_menu)
  (OSXDelegateHandle handle, OSXConfigHandle config, char const* style)
  { [(OSXDelegate*)handle initApplicationMenu:config style:style]; }


//-----------------------------------------------------------------------------
// startEventLoop
//	Begin running the event-loop.  This method will be called when Crystal
//	Space requests that the event-loop commences running at application
//	launch time.  Does not return until either user or Crystal Space
//	requests application termination.  It is illegal to invoke this method
//	recursively.
//-----------------------------------------------------------------------------
- (void)startEventLoop
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  [self scheduleEvent];
  [NSApp run];
  [pool release];
}

ND_PROTO(void,start_event_loop)(OSXDelegateHandle handle)
  { [(OSXDelegate*)handle startEventLoop]; }


//-----------------------------------------------------------------------------
// stopEventLoop
//	Stops the application's event-loop.  Unfortunately the run-loop does
//	not actually stop until another event arrives, so we fake one up.
//-----------------------------------------------------------------------------
- (void)stopEventLoop
{
  [NSApp stop:0];
  [self scheduleEvent];
}

ND_PROTO(void,stop_event_loop)(OSXDelegateHandle handle)
  { [(OSXDelegate*)handle stopEventLoop]; }


//-----------------------------------------------------------------------------
// initWithDriver:
//-----------------------------------------------------------------------------
- (id)initWithDriver:(OSXAssistant)p
{
  [super init];
  assistant = p;
  modifiers = 0;
  mouseHidden = NO;
  paused = NO;
  autoResume = NO;
  return self;
}


//-----------------------------------------------------------------------------
// dealloc
//-----------------------------------------------------------------------------
- (void)dealloc
{
  [super dealloc];
}


//-----------------------------------------------------------------------------
// startup
//	Interaction with AppKit is initiated here with instantiation of an
//	NSApplication object and a OSXDelegate which oversees AppKit-related
//	events and messages.
//-----------------------------------------------------------------------------
+ (OSXDelegate*)startup:(OSXAssistant)handle
{
  NSAutoreleasePool* pool;
  OSXDelegate* controller;
  if (CS_GLOBAL_POOL == 0) CS_GLOBAL_POOL = [[NSAutoreleasePool alloc] init];
  pool = [[NSAutoreleasePool alloc] init];
  NSApp = [OSXApplication sharedApplication];
  controller = [[OSXDelegate alloc] initWithDriver:handle];

  [[NSNotificationCenter defaultCenter]
    addObserver:controller
    selector:@selector(applicationDidBecomeActive:)
    name:NSApplicationDidBecomeActiveNotification
    object:NSApp];

  [[NSNotificationCenter defaultCenter]
    addObserver:controller
    selector:@selector(applicationDidResignActive:)
    name:NSApplicationDidResignActiveNotification
    object:NSApp];

  [[NSNotificationCenter defaultCenter]
    addObserver:controller
    selector:@selector(applicationDidHide:) 
    name:NSApplicationDidHideNotification   
    object:NSApp];

  [[NSNotificationCenter defaultCenter]
    addObserver:controller
    selector:@selector(applicationDidUnhide:)
    name:NSApplicationDidUnhideNotification
    object:NSApp];

  [[NSNotificationCenter defaultCenter]
    addObserver:controller
    selector:@selector(applicationDefined:)
    name:OSXAppDefinedNotification
    object:0];
  [pool release];
  return controller;
}

ND_PROTO(OSXDelegateHandle,startup)(OSXAssistant handle)
  { return (OSXDelegateHandle)[OSXDelegate startup:handle]; }


//-----------------------------------------------------------------------------
// shutdown:
//-----------------------------------------------------------------------------
+ (void)shutdown:(OSXDelegate*)controller
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  [[NSNotificationCenter defaultCenter] removeObserver:controller];
  [controller showMouse];
  [controller flushGraphicsContext]; // Flush any pending `showcursor'.
  [controller release];
  [pool release];
}

ND_PROTO(void,shutdown)(OSXDelegateHandle handle)
  { [OSXDelegate shutdown:(OSXDelegate*)handle]; }

@end

#undef ND_PROTO
