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
#import <appkit/Application.h>
#import <appkit/Menu.h>
}

//-----------------------------------------------------------------------------
// Category of Application which supports recursive run loops.
//-----------------------------------------------------------------------------
@interface Application (NeXTSystemProxy)
- (void)runRecursively:(NeXTSystemProxy const*)proxy;
@end
@implementation Application (NeXTSystemProxy)
- (void)runRecursively:(NeXTSystemProxy const*)proxy
    {
    int const was_running = running;
    [self run];
    if (proxy->continue_running())
	running = was_running;
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
    driver = p;
    init_ticks();
    NXApp = [Application new];
    controller = [[NeXTDelegate alloc] initWithProxy:this];
    [NXApp setDelegate:controller];
    Menu* const menu = NeXTMenuGenerate();
    [menu setTitle:[NXApp appName]];
    [NXApp setMainMenu:menu];
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTSystemProxy::~NeXTSystemProxy()
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
void NeXTSystemProxy::start_loop()
    {
    if (continue_running())
	{
	[[NXApp delegate] resetTimer];
	[NXApp runRecursively:this];
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
    [NXApp stop:0];

    NXEvent e;
    e.type = NX_APPDEFINED;
    e.ctxt = [NXApp context];
    e.time = 0;
    e.flags = 0;
    e.window = 0;
    DPSPostEvent( &e, TRUE );
    }
