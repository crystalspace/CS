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
// NeXTSystemDriver.cpp
//
//	NeXT-specific hardware & operating/system drivers for Crystal Space.
//	This file contains code which is shared between MacOS/X, MacOS/X
//	Server 1.0 (Rhapsody), OpenStep, and NextStep.  See NeXTDelegate.m for
//	the platform-specific portion of the system driver.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "cssys/next/NeXTSystemDriver.h"
#include "NeXTDelegate.h"
#include "icfgnew.h"
#include "version.h"
#include <stdarg.h>

typedef void* NeXTSystemHandle;
#define NSD_PROTO(RET,FUNC) extern "C" RET NeXTSystemDriver_##FUNC

#define STR_SWITCH(X) { char const* switched_str__=(X); if (0) {
#define STR_CASE(X) } else if (strcmp(switched_str__,(#X)) == 0) {
#define STR_DEFAULT } else {
#define STR_SWITCH_END }}

IMPLEMENT_IBASE_EXT(NeXTSystemDriver)
    IMPLEMENTS_EMBEDDED_INTERFACE(iEventPlug)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE(NeXTSystemDriver::NeXTSystemEventPlug)
    IMPLEMENTS_INTERFACE(iEventPlug)
IMPLEMENT_EMBEDDED_IBASE_END

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
NeXTSystemDriver::NeXTSystemDriver() :
    csSystemDriver(), controller(0), event_outlet(0)
    {
    CONSTRUCT_EMBEDDED_IBASE( scfiEventPlug );
    printf("Crystal Space for " CS_PLATFORM_NAME " " CS_VERSION "\nPorted to "
	CS_PLATFORM_NAME " by Eric Sunshine <sunshine@sunshineco.com>\n\n");
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTSystemDriver::~NeXTSystemDriver()
    {
    if (controller != 0)
	NeXTDelegate_shutdown( controller );
    if (event_outlet != 0)
	event_outlet->DecRef();
    }


//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
bool NeXTSystemDriver::Initialize( int argc, char const* const argv[],
    char const* cfgfile )
    {
    bool ok = false;
    controller = NeXTDelegate_startup( this );
    event_outlet = CreateEventOutlet( &scfiEventPlug );
    if (superclass::Initialize( argc, argv, cfgfile ))
	{
	iConfigFileNew* next_config = CreateConfigNew( "/config/next.cfg" );
	init_menu( next_config );
	next_config->DecRef();
	ok = true;
	}
    return ok;
    }


//-----------------------------------------------------------------------------
// init_menu
//	Generate application menu based upon platform configuration.
//-----------------------------------------------------------------------------
void NeXTSystemDriver::init_menu( iConfigFileNew* next_config )
    {
    char const* style =
	next_config->GetStr( "Platform." OS_NEXT_DESCRIPTION ".menu", 0);
    if (style != 0)
	NeXTDelegate_init_app_menu( controller, next_config, style );
    }


//-----------------------------------------------------------------------------
// Loop
//	Start the application's run-loop.  Is called at application launch time
//	and each time CSWS launches a modal window.  Returns when application
//	terminates and when CSWS modal window is dismissed.
//-----------------------------------------------------------------------------
void NeXTSystemDriver::Loop()
    {
    NeXTDelegate_start_event_loop( controller );
    }


//-----------------------------------------------------------------------------
// timer_fired
//	Target of periodic animation timer.  Invokes the engine's NextFrame()
//	method.
//-----------------------------------------------------------------------------
void NeXTSystemDriver::timer_fired()
    {
    NextFrame();
    if (!continue_looping())
	{
	NeXTDelegate_stop_event_loop( controller );
	ExitLoop = false;
	}
    }


//-----------------------------------------------------------------------------
// SystemExtension
//
// Perform a system-specific extension.  The following requests are understood:
//
//	timerfired
//	    The periodic animation timer fired.  Invokes the engine's
//	    NextFrame() method.
//	continuelooping <int*>
//	    Query whether or not the application's event loop should continue
//	    running.  The result is returned as a boolean result in the integer
//	    variable referenced by the argument.  Returns `true' if the system
//	    driver variables ExitLoop and Shutdown are both false.
//	continuerunning <int*>
//	    Query whether or not the application's event loop should continue
//	    running.  The result is returned as a boolean result in the integer
//	    variable referenced by the argument.  Returns `true' if the system
//	    driver variable Shutdown is false.
//	flushgraphicscontext
//	    Flush the connection to the current graphics context (the Quartz
//	    or DPS server, for instance).  This forces the graphics context to
//	    perform all pending drawing operations.
//	dispatchevent <void*:event> <void*:view>
//	    Interpret an AppKit event and post the appropriate csEvent to the
//	    Crystal Space event queue.  The first argument is a pointer to an
//	    NSEvent (Cocoa and OpenStep) or an NXEvent (NextStep).  The second
//	    argument is a pointer to the view with which the event is
//	    associated, or NULL if not associated with any view.  The view
//	    argument refers to an NSView (Cocoa and OpenStep) or a View
//	    (NextStep).
//	keydown <int:raw> <int:cooked>
//	    Dispatch a key-down event.  The first number is the raw key code,
//	    and the second is the cooked character code.
//	keyup <int:raw> <int:cooked>
//	    Dispatch a key-up event.  The first number is the raw key code,
//	    and the second is the cooked character code.
//	mousedown <int:button> <int:x> <int:y>
//	    Dispatch a mouse-down event at location (x,y).  The mouse-button
//	    number 1-based.
//	mouseup <int:button> <int:x> <int:y>
//	    Dispatch a mouse-up event at location (x,y).  The mouse-button
//	    number 1-based.
//	mousemoved <int:x> <int:y>
//	    Dispatch a mouse-moved event at location (x,y).
//	hidemouse
//	    Hide the mouse pointer.
//	showmouse
//	    Unhide the mouse pointer.
//	appactivated
//	    The application came the foreground.
//	appdeactivated
//	    The application was sent to the background.
//	requestshutdown
//	    Ask to have both the AppKit and Crystal Space even-loops terminated
//	    and then exit from the outermost invocation of iSystem::Loop().
//
// When canvas coordinates accompany requests, they are specified in terms of
// the Crystal Space coordinate system where `x' increases from left to right,
// and `y' increases from top to bottom.
//-----------------------------------------------------------------------------
bool NeXTSystemDriver::SystemExtension( char const* cmd, ... )
{
    bool ok = true;
    va_list args;
    va_start( args, cmd );
#define AGET(T) va_arg(args,T)
    
    STR_SWITCH (cmd)
	STR_CASE (timerfired)
	    timer_fired();
	STR_CASE (continuelooping)
	    int* flag = AGET(int*);
	    *flag = continue_looping();
	STR_CASE (continuerunning)
	    int* flag = AGET(int*);
	    *flag = continue_running();
	STR_CASE (flushgraphicscontext)
	    NeXTDelegate_flush_graphics_context( controller );
	STR_CASE (dispatchevent)
	    NeXTEvent const event = AGET(NeXTEvent);
	    NeXTView  const view  = AGET(NeXTView );
	    NeXTDelegate_dispatch_event( controller, event, view );
	STR_CASE (keydown)
	    int const raw = AGET(int);
	    int const cooked = AGET(int);
	    event_outlet->Key( raw, cooked, true );
	STR_CASE (keyup)
	    int const raw = AGET(int);
	    int const cooked = AGET(int);
	    event_outlet->Key( raw, cooked, false );
	STR_CASE (mousedown)
	    int const button = AGET(int);
	    int const x = AGET(int);
	    int const y = AGET(int);
	    event_outlet->Mouse( button, true, x, y );
	STR_CASE (mouseup)
	    int const button = AGET(int);
	    int const x = AGET(int);
	    int const y = AGET(int);
	    event_outlet->Mouse( button, false, x, y );
	STR_CASE (mousemoved)
	    int const x = AGET(int);
	    int const y = AGET(int);
	    event_outlet->Mouse( 0, false, x, y );
	STR_CASE (hidemouse)
	    NeXTDelegate_hide_mouse( controller );
	STR_CASE (showmouse)
	    NeXTDelegate_show_mouse( controller );
	STR_CASE (appactivated)
	    ResumeVirtualTimeClock();
	    event_outlet->ImmediateBroadcast( cscmdFocusChanged, (void*)true);
	STR_CASE (appdeactivated)
	    SuspendVirtualTimeClock();
	    event_outlet->ImmediateBroadcast( cscmdFocusChanged, (void*)false);
	STR_CASE (requestshutdown)
	    event_outlet->ImmediateBroadcast( cscmdQuit, 0 );
	    NeXTDelegate_stop_event_loop( controller );
	STR_DEFAULT
	    ok = false;
    STR_SWITCH_END

#undef AGET
    va_end(args);
    return ok;
}

NSD_PROTO(int,system_extension)
    ( NeXTSystemHandle h, char const* msg, void* d1, void* d2, void* d3)
    { return ((NeXTSystemDriver*)h)->SystemExtension( msg, d1, d2, d3 ); }


//-----------------------------------------------------------------------------
// iEventPlug Implementation
//-----------------------------------------------------------------------------
uint NeXTSystemDriver::NeXTSystemEventPlug::GetPotentiallyConflictingEvents()
    { return (CSEVTYPE_Keyboard | CSEVTYPE_Mouse); }
uint NeXTSystemDriver::NeXTSystemEventPlug::QueryEventPriority( uint type )
    { return 150; }

#undef STR_SWITCH_END
#undef STR_DEFAULT
#undef STR_CASE
#undef STR_SWITCH
#undef NSD_PROTO
