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
//	This file contains methods which are specific to the NextStep
//	platform.  See NeXTSystemDriver.cpp for methods which are shared
//	between MacOS/X Server, OpenStep, and NextStep.
//
//-----------------------------------------------------------------------------
#import "NeXTSystemDriver.h"
#import "NeXTDelegate.h"
#import "NeXTMenu.h"
extern "Objective-C" {
#import <appkit/Application.h>
#import <appkit/Menu.h>
}

//-----------------------------------------------------------------------------
// Category of Application which supports recursive run loops.
//-----------------------------------------------------------------------------
@interface Application (NeXTSystemDriver)
- (void)runRecursively:(NeXTSystemDriver const*)sys;
@end
@implementation Application (NeXTSystemDriver)
- (void)runRecursively:(NeXTSystemDriver const*)sys
    {
    int const was_running = running;
    [self run];
    if (sys->continue_running())
	running = was_running;
    }
@end


//-----------------------------------------------------------------------------
// init_system
//	Interaction with AppKit is initiated here with instantiation of an
//	Application object and the 'controller' which oversees AppKit-related
//	events and messages.
//-----------------------------------------------------------------------------
void NeXTSystemDriver::init_system()
    {
    NXApp = [Application new];
    controller = [[NeXTDelegate alloc] initWithDriver:this];
    [NXApp setDelegate:controller];
    Menu* const menu = NeXTMenuGenerate();
    [menu setTitle:[NXApp appName]];
    [NXApp setMainMenu:menu];
    }


//-----------------------------------------------------------------------------
// shutdown_system
//-----------------------------------------------------------------------------
void NeXTSystemDriver::shutdown_system()
    {
    [[NXApp delegate] showMouse];
    [NXApp setDelegate:0];
    [controller registerAnimationWindow:0];	// Clears window's delegate.
    [controller free];
    DPSFlush();					// Force flush 'showcursor'.
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
	[[NXApp delegate] resetTimer];
	[NXApp runRecursively:this];
	}
    }


//-----------------------------------------------------------------------------
// stop_run_loop
//	Stops the application's run-loop.  Unfortunately the run-loop does not
//	actually stop until another event arrives, so we fake one up.
//-----------------------------------------------------------------------------
void NeXTSystemDriver::stop_run_loop()
    {
    [NXApp stop:0];

    NXEvent e;
    e.type = NX_APPDEFINED;
    e.ctxt = [NXApp context];
    e.time = 0;
    e.flags = 0;
    e.window = 0;
    DPSPostEvent( &e, TRUE );
    }
