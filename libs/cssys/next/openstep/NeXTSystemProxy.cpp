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
// NeXTSystemProxy.cpp
//
//	C++ object which interacts with Objective-C world on behalf of 
//	SysSystemDriver which can not directly interface with Objective-C on 
//	account of COM-related conflicts.  This is the Objective-C-only 
//	portion of NeXTSystemProxy.  See NeXTSystemProxyCom.cpp for the 
//	COM-only portion of NeXTSystemProxy.  Also see README.NeXT for 
//	details.  
//
// *WARNING* Do NOT include any COM headers in this file.
//-----------------------------------------------------------------------------
#import "NeXTSystemProxy.h"
#import "NeXTDelegate.h"
#import "NeXTMenu.h"
extern "Objective-C" {
#import <AppKit/NSApplication.h>
#import <AppKit/NSDPSContext.h>
#import <AppKit/NSMenu.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSProcessInfo.h>
}

static NSAutoreleasePool* GLOBAL_POOL = 0;

//-----------------------------------------------------------------------------
// Category of NSApplication which supports recursive run loops.
//-----------------------------------------------------------------------------
@interface NSApplication (NeXTSystemProxy)
- (void)runRecursively:(NeXTSystemProxy const*)proxy;
@end
@implementation NSApplication (NeXTSystemProxy)
- (void)runRecursively:(NeXTSystemProxy const*)proxy
    {
    int const was_running = _running;
    [self run];
    if (proxy->continue_running())
	_running = was_running;
    }
@end


//-----------------------------------------------------------------------------
// Constructor
//	Interaction with AppKit is initiated here with instantiation of an
//	Application object and the 'controller' which oversees AppKit-related
//	events and messages.
//-----------------------------------------------------------------------------
NeXTSystemProxy::NeXTSystemProxy( SysSystemDriver* p )
    {
    if (GLOBAL_POOL == 0) GLOBAL_POOL = [[NSAutoreleasePool alloc] init];
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    driver = p;
    init_ticks();
    NSApp = [NSApplication sharedApplication];
    controller = [[NeXTDelegate alloc] initWithProxy:this];
    [NSApp setDelegate:controller];
    NSMenu* const menu = NeXTMenuGenerate();
    [menu setTitle:[[NSProcessInfo processInfo] processName]];
    [NSApp setMainMenu:menu];
    [pool release];
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTSystemProxy::~NeXTSystemProxy()
    {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    [[NSApp delegate] showMouse];
    [NSApp setDelegate:0];
    [controller registerAnimationWindow:0];	// Clears window's delegate.
    [controller release];
    [[NSDPSContext currentContext] flush];	// Force flush 'showcursor'.
    [pool release];
    }


//-----------------------------------------------------------------------------
// start_loop
//	Begin a run-loop.  May be called recursively by CSWS.  Uses special
//	-runRecursively: method to handle recursive invocations of run-loop.
//-----------------------------------------------------------------------------
void NeXTSystemProxy::start_loop()
    {
    if (continue_running())
	{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	[[NSApp delegate] resetTimer];
	[NSApp runRecursively:this];
	[pool release];
	}
    }


//-----------------------------------------------------------------------------
// timer_fired
//	Target of timer.  Forwards timer event to proxy.
//-----------------------------------------------------------------------------
void NeXTSystemProxy::timer_fired()
    {
    step_frame();
    if (!continue_looping())
	stop_run_loop();
    }


//-----------------------------------------------------------------------------
// stop_run_loop
//	Stops the application's run-loop.  Unfortunately the run-loop does not 
//	actually stop until another event arrives, so we fake one up.  
//-----------------------------------------------------------------------------
void NeXTSystemProxy::stop_run_loop()
    {
    [NSApp stop:0];
    [NSApp postEvent:[NSEvent otherEventWithType:NSApplicationDefined
	location:NSMakePoint(0,0) modifierFlags:0 timestamp:0 windowNumber:0
	context:[NSApp context] subtype:0 data1:0 data2:0] atStart:YES];
    }
