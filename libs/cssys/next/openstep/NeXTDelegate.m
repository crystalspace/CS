//=============================================================================
//
//	Copyright (C)1999-2001 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// NeXTDelegate.m
//
//	The application's delegate.  Acts as a gateway between the AppKit and
//	Crystal Space by forwarding Objective-C messages and events to the C++
//	system driver, NeXTSystemDriver.
//
//-----------------------------------------------------------------------------
#include "NeXTDelegate.h"
#include "NeXTMenu.h"
#include "isys/evdefs.h"
#include "cssys/next/NeXTSystemDriver.h"
#include "volatile.h"		// OS_NEXT_OPENSTEP, OS_NEXT_MACOSXS
#include <stdio.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSView.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSString.h>
#import <Foundation/NSUserDefaults.h>

#if defined(OS_NEXT_OPENSTEP) || defined(OS_NEXT_MACOSXS)
#import <AppKit/NSDPSContext.h>
#else
#import <AppKit/NSGraphicsContext.h>
#endif

typedef void* NeXTDelegateHandle;
typedef void* NeXTEventHandle;
typedef void* NeXTViewHandle;
#define ND_PROTO(RET,FUNC) RET NeXTDelegate_##FUNC

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


//-----------------------------------------------------------------------------
// continue_running
//-----------------------------------------------------------------------------
static inline BOOL continue_running(NeXTSystemDriver driver)
{
  int run;
  NeXTSystemDriver_system_extension(driver, "continuerunning", &run, 0, 0);
  return run;
}


//=============================================================================
// Category of NSApplication which supports recursive run loops.
//=============================================================================
@interface NSApplication (NeXTDelegate)
- (void)runRecursively:(NeXTSystemDriver)driver;
@end
@implementation NSApplication (NeXTDelegate)
- (void)runRecursively:(NeXTSystemDriver)driver
{
  int const was_running = _running; // The old run-loop invocation depth.
  [self run];
  if (continue_running(driver))
    _running = was_running; // Restore old depth rather than terminating.
}
@end


//=============================================================================
// Subclass of NSApplication which provides an -applicationDefined: delegate
// message much like the one provided by the old NextStep Application class.
//=============================================================================
@interface NeXTApplication : NSApplication
- (void)sendEvent:(NSEvent*)e;
@end
@implementation NeXTApplication
- (void)sendEvent:(NSEvent*)e
{
  id d = [self delegate];
  if ([e type] == NSApplicationDefined &&
      d != 0 && [d respondsToSelector:@selector(applicationDefined:)])
    [d performSelector:@selector(applicationDefined:) withObject:e];
  else
    [super sendEvent:e];
}
@end


//=============================================================================
// NeXTDelegate Implementation
//=============================================================================
@implementation NeXTDelegate

//-----------------------------------------------------------------------------
// scheduleEvent
//	Insert a do-nothing event at the tail end of the AppKit's event queue.
//	This event results in an invocation of -applicationDefined: (below).
//-----------------------------------------------------------------------------
- (void)scheduleEvent
{
  [NSApp postEvent:[NSEvent otherEventWithType:NSApplicationDefined
    location:NSMakePoint(0,0) modifierFlags:0 timestamp:0 windowNumber:0
    context:[NSApp context] subtype:0 data1:0 data2:0] atStart:NO];
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
- (void)applicationDefined:(NSEvent*)e
{
  NeXTSystemDriver_system_extension(driver, "advancestate", 0, 0, 0);
  if (!paused && continue_running(driver))
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

ND_PROTO(void,show_mouse)(NeXTDelegateHandle handle)
  { [(NeXTDelegate*)handle showMouse]; }


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

ND_PROTO(void,hide_mouse)(NeXTDelegateHandle handle)
  { [(NeXTDelegate*)handle hideMouse]; }


//-----------------------------------------------------------------------------
// flushGraphicsContext
//-----------------------------------------------------------------------------
- (void)flushGraphicsContext
{
#if defined(OS_NEXT_OPENSTEP) || defined(OS_NEXT_MACOSXS)
  [[NSDPSContext currentContext] flush];
#else
  [[NSGraphicsContext currentContext] flushGraphics];
#endif
}

ND_PROTO(void,flush_graphics_context)(NeXTDelegateHandle handle)
  { [(NeXTDelegate*)handle flushGraphicsContext]; }


//-----------------------------------------------------------------------------
// quit: -- Target of "Quit" menu item.  Terminate the application.
//-----------------------------------------------------------------------------
- (void)quit:(id)sender
{
  NeXTSystemDriver_system_extension(driver, "requestshutdown", 0, 0, 0);
}


//-----------------------------------------------------------------------------
// localize:toView:x:y: -- Convert event location to view coordinate system.
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
// classifyKeyDown:raw:cooked:
//	Translate OpenStep keystroke to Crystal Space codes.
//-----------------------------------------------------------------------------
- (BOOL)classifyKeyDown:(NSEvent*)p raw:(int*)raw cooked:(int*)cooked
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
  cs:(unsigned long)csmask key:(int)key
{
  BOOL const new_state = (([p modifierFlags] & nsmask) != 0);
  BOOL const old_state = ((modifiers & csmask) != 0);
  if (new_state != old_state)
  {
    char const* request;
    if (new_state)
    {
      modifiers |= csmask;
      request = "keydown";
    }
    else
    {
      modifiers &= ~csmask;
      request = "keyup";
    }
    NeXTSystemDriver_system_extension(
      driver, request, (void*)key, (void*)-1, 0);
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
      char const* request = flag ? "keydown" : "keyup";
      NeXTSystemDriver_system_extension(
	driver, request, (void*)raw, (void*)cooked, 0);
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
      NeXTSystemDriver_system_extension(
	driver, "mousemoved", (void*)x, (void*)y, 0);
  }
}

- (void)mouseUp:(NSEvent*)p forView:(NSView*)v button:(int)button
{
  if (!paused)
  {
    int x, y;
    [self localize:p toView:v x:&x y:&y];
    NeXTSystemDriver_system_extension(
      driver, "mouseup", (void*)button, (void*)x, (void*)y);
  }
}

- (void)mouseDown:(NSEvent*)p forView:(NSView*)v button:(int)button
{
  if (!paused)
  {
    int x, y;
    [self localize:p toView:v x:&x y:&y];
    NeXTSystemDriver_system_extension(
      driver, "mousedown", (void*)button, (void*)x, (void*)y);
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
  (NeXTDelegateHandle h, NeXTEventHandle event, NeXTViewHandle view)
  { [(NeXTDelegate*)h dispatchEvent:(NSEvent*)event forView:(NSView*)view]; }


//-----------------------------------------------------------------------------
// Activation / Deactivation
//-----------------------------------------------------------------------------
- (void)pause
{
  if (!paused)
  {
    paused = YES;
    [self showMouse];
    NeXTSystemDriver_system_extension(driver, "appdeactivated", 0, 0, 0);
  }
}

- (void)unpause
{
  if (paused)
  {
    paused = NO;
    [self scheduleEvent];
    NeXTSystemDriver_system_extension(driver, "appactivated", 0, 0, 0);
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


//-----------------------------------------------------------------------------
// initApplicationMenu:style:
//	Generate application menu based upon platform configuration.
//-----------------------------------------------------------------------------
- (void)initApplicationMenu:(NeXTConfigHandle)config style:(char const*)style
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSMenu* const menu = NeXTMenuGenerate(style, config);
  [menu setTitle:[[NSProcessInfo processInfo] processName]];
  [NSApp setMainMenu:menu];
  [pool release];
}

ND_PROTO(void,init_app_menu)
  (NeXTDelegateHandle handle, NeXTConfigHandle config, char const* style)
  { [(NeXTDelegate*)handle initApplicationMenu:config style:style]; }


//-----------------------------------------------------------------------------
// startEventLoop
//	Begin running an event-loop.  May be called recursively by CSWS.  Uses
//	special -runRecursively: method to handle recursive invocations of
//	event-loop.
//
// *CONTINUE-RUNNING*
//	Since -startEventLoop may be invoked recursively, special care must be
//	taken to avoid starting the new event-loop if the user has requested
//	application termination via an external means, such as closing the
//	AppKit window.  When the user does perform an external request for
//	termination, we forcibly terminate all CSWS modal loops.
//	Unfortunately, however, code which employs CSWS may not itself check
//	that termination has been requested, and may try launching another
//	modal session by recursively invoking -startEventLoop immediately
//	following the one which was just forcibly aborted.  In order to prevent
//	another modal session from actually launching in this circumstance we
//	check continue_running() before even starting the event-loop.
//-----------------------------------------------------------------------------
- (void)startEventLoop
{
  if (continue_running(driver))		// *CONTINUE-RUNNING*
  {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    [self scheduleEvent];
    [NSApp runRecursively:driver];
    [pool release];
  }
}

ND_PROTO(void,start_event_loop)(NeXTDelegateHandle handle)
  { [(NeXTDelegate*)handle startEventLoop]; }


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

ND_PROTO(void,stop_event_loop)(NeXTDelegateHandle handle)
  { [(NeXTDelegate*)handle stopEventLoop]; }


//-----------------------------------------------------------------------------
// initWithDriver:
//-----------------------------------------------------------------------------
- (id)initWithDriver:(NeXTSystemDriver)p
{
  [super init];
  driver = p;
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
// locateResourceDirectory
//	If the CrystalSpaceRoot default value is set, then its value is used to
//	set the current working directory.  This will allow Crystal Space to
//	locate its resource files even when launched from the Workspace which
//	does not otherwise provide a meaningful working directory.  However, if
//	CrystalSpaceRootIgnore is set to "yes", then ignore the
//	CrystalSpaceRoot setting.  This can be used to force a particular
//	application to ignore a CrystalSpaceRoot setting inherited from the
//	"global" domain.
//-----------------------------------------------------------------------------
+ (void)locateResourceDirectory
{
  NSUserDefaults* defs = [NSUserDefaults standardUserDefaults];
  NSString* s = [defs stringForKey:@"CrystalSpaceRootIgnore"];
  if (s == 0 || [s isEqualToString:@""] ||
     (![s hasPrefix:@"Y"] && ![s hasPrefix:@"y"] &&	// Yes
      ![s hasPrefix:@"T"] && ![s hasPrefix:@"t"] &&	// True
      ![s hasPrefix:@"O"] && ![s hasPrefix:@"o"] &&	// On
      ![s hasPrefix:@"1"]))				// 1
  {
    s = [defs stringForKey:@"CrystalSpaceRoot"];
    if (s != 0 && ![s isEqualToString:@""] &&
      ![[NSFileManager defaultManager] changeCurrentDirectoryPath:s])
      fprintf(stderr,
	"\nWARNING: Unable to set working directory from "
	"`CrystalSpaceRoot'\nWARNING: Value: %s\n\n",[s lossyCString]);
  }
}


//-----------------------------------------------------------------------------
// startup
//	Interaction with AppKit is initiated here with instantiation of an
//	NSApplication object and a NeXTDelegate which oversees AppKit-related
//	events and messages.
//-----------------------------------------------------------------------------
+ (NeXTDelegate*)startup:(NeXTSystemDriver)handle
{
  NSAutoreleasePool* pool;
  NeXTDelegate* controller;
  if (CS_GLOBAL_POOL == 0) CS_GLOBAL_POOL = [[NSAutoreleasePool alloc] init];
  pool = [[NSAutoreleasePool alloc] init];
  NSApp = [NeXTApplication sharedApplication];
  controller = [[NeXTDelegate alloc] initWithDriver:handle];
  [NSApp setDelegate:controller];
  [self locateResourceDirectory];
  [pool release];
  return controller;
}

ND_PROTO(NeXTDelegateHandle,startup)(NeXTSystemDriver handle)
  { return (NeXTDelegateHandle)[NeXTDelegate startup:handle]; }


//-----------------------------------------------------------------------------
// shutdown:
//-----------------------------------------------------------------------------
+ (void)shutdown:(NeXTDelegate*)controller
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  [NSApp setDelegate:0];
  [controller showMouse];
  [controller flushGraphicsContext]; // Flush any pending `showcursor'.
  [controller release];
  [pool release];
}

ND_PROTO(void,shutdown)(NeXTDelegateHandle handle)
  { [NeXTDelegate shutdown:(NeXTDelegate*)handle]; }

@end

#undef ND_PROTO
