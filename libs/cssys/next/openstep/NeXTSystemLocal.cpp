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
// NeXTSystemLocal.cpp
//
//	NeXT-specific hardware & operating/system drivers for CrystalSpace.
//	This file contains methods which are specific to the MacOS/X Server
//	and OpenStep platforms.  See NeXTSystemDriver.cpp for methods which
//	are shared between MacOS/X Server, OpenStep, and NextStep.
//
//-----------------------------------------------------------------------------
#import "NeXTSystemDriver.h"
#import "NeXTDelegate.h"
#import "NeXTMenu.h"
extern "Objective-C" {
#import <AppKit/NSApplication.h>
#import <AppKit/NSDPSContext.h>
#import <AppKit/NSMenu.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSProcessInfo.h>
}

static NSAutoreleasePool* CS_GLOBAL_POOL = 0;

//-----------------------------------------------------------------------------
// Category of NSApplication which supports recursive run loops.
//-----------------------------------------------------------------------------
@interface NSApplication (NeXTSystemDriver)
- (void)runRecursively:(NeXTSystemDriver const*)sys;
@end
@implementation NSApplication (NeXTSystemDriver)
- (void)runRecursively:(NeXTSystemDriver const*)sys
    {
    int const was_running = _running;
    [self run];
    if (sys->continue_running())
	_running = was_running;
    }
@end


//-----------------------------------------------------------------------------
// init_system
//	Interaction with AppKit is initiated here with instantiation of an
//	NSApplication object and the 'controller' which oversees AppKit-
//	related events and messages.
//-----------------------------------------------------------------------------
void NeXTSystemDriver::init_system()
    {
    if (CS_GLOBAL_POOL == 0) CS_GLOBAL_POOL = [[NSAutoreleasePool alloc] init];
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    NSApp = [NSApplication sharedApplication];
    controller = [[NeXTDelegate alloc] initWithDriver:this];
    [NSApp setDelegate:controller];
    NSMenu* const menu = NeXTMenuGenerate();
    [menu setTitle:[[NSProcessInfo processInfo] processName]];
    [NSApp setMainMenu:menu];
    [pool release];
    }


//-----------------------------------------------------------------------------
// shutdown_system
//-----------------------------------------------------------------------------
void NeXTSystemDriver::shutdown_system()
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
void NeXTSystemDriver::start_loop()
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
// stop_run_loop
//	Stops the application's run-loop.  Unfortunately the run-loop does not 
//	actually stop until another event arrives, so we fake one up.  
//-----------------------------------------------------------------------------
void NeXTSystemDriver::stop_run_loop()
    {
    [NSApp stop:0];
    [NSApp postEvent:[NSEvent otherEventWithType:NSApplicationDefined
	location:NSMakePoint(0,0) modifierFlags:0 timestamp:0 windowNumber:0
	context:[NSApp context] subtype:0 data1:0 data2:0] atStart:YES];
    }
