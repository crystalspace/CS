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
// NeXTSystemDriver.cpp
//
//	NeXT-specific hardware & operating/system drivers for CrystalSpace.
//	This file contains methods which are shared between MacOS/X Server,
//	OpenStep, and NextStep platforms.  See NeXTSystemLocal.cpp for
//	platform-specific implementation.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "cssys/next/NeXTSystemDriver.h"
#include "icfgfile.h"
#include "version.h"
#include <stdarg.h>

IMPLEMENT_IBASE_EXT(NeXTSystemDriver)
    IMPLEMENTS_EMBEDDED_INTERFACE(iEventPlug)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE(NeXTSystemDriver::NeXTSystemEventPlug)
    IMPLEMENTS_INTERFACE(iEventPlug)
IMPLEMENT_EMBEDDED_IBASE_END

static inline bool streq( char const* a, char const* b )
    { return strcmp(a,b) == 0; }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
NeXTSystemDriver::NeXTSystemDriver() :
    csSystemDriver(), initialized(false), controller(0), event_outlet(0)
    {
    CONSTRUCT_EMBEDDED_IBASE(scfiEventPlug);
    printf("Crystal Space for " CS_PLATFORM_NAME " " CS_VERSION "\nPorted to "
	CS_PLATFORM_NAME " by Eric Sunshine <sunshine@sunshineco.com>\n\n");
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTSystemDriver::~NeXTSystemDriver()
    {
    if (initialized)
	shutdown_system();
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
    init_system();
    event_outlet = CreateEventOutlet( &scfiEventPlug );
    initialized = true;
    if (superclass::Initialize( argc, argv, cfgfile ))
	{
	iConfigFile* next_config = CreateConfig( "/config/next.cfg" );
	init_menu( next_config );
	next_config->DecRef();
	ok = true;
	}
    return ok;
    }


//-----------------------------------------------------------------------------
// Loop -- Start the Application's run-loop; return at termination.
//-----------------------------------------------------------------------------
void NeXTSystemDriver::Loop()
    {
    start_loop(); // Returns when user requests shutdown.
    }


//-----------------------------------------------------------------------------
// timer_fired -- Target of timer.  Forwards timer event to proxy.
//-----------------------------------------------------------------------------
void NeXTSystemDriver::timer_fired()
    {
    NextFrame();
    if (!continue_looping())
	{
	stop_run_loop();
	ExitLoop = false;
	}
    }


//-----------------------------------------------------------------------------
// terminate
//-----------------------------------------------------------------------------
void NeXTSystemDriver::terminate()
    {
    Shutdown = true;
    stop_run_loop();
    }


//-----------------------------------------------------------------------------
// SystemExtension -- Perform a system-specific extension.
//
// When canvas coordinates accompany requests, they are specified in terms of
// the Crystal Space coordinate system where `x' increases from left to right,
// and `y' increases from top to bottom.
//
// The following requests are understood:
//
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
//	appactivated
//	    The application came the foreground.
//	appdeactivated
//	    The application was sent to the background.
//-----------------------------------------------------------------------------
bool NeXTSystemDriver::SystemExtension( char const* cmd, ... )
{
    bool ok = true;
    va_list args;
    va_start( args, cmd );
#define AGET(T) va_arg(args,T)
    
    if (streq( cmd, "keydown" ))
	{
	int const raw = AGET(int);
	int const cooked = AGET(int);
	event_outlet->Key( raw, cooked, true );
	}
    else if (streq( cmd, "keyup" ))
	{
	int const raw = AGET(int);
	int const cooked = AGET(int);
	event_outlet->Key( raw, cooked, false );
	}
    else if (streq(cmd, "mousedown"))
	{
	int const button = AGET(int);
	int const x = AGET(int);
	int const y = AGET(int);
	event_outlet->Mouse( button, true, x, y );
	}    
    else if (streq(cmd, "mouseup"))
	{
	int const button = AGET(int);
	int const x = AGET(int);
	int const y = AGET(int);
	event_outlet->Mouse( button, false, x, y );
	}    
    else if (streq(cmd, "mousemoved"))
	{
	int const x = AGET(int);
	int const y = AGET(int);
	event_outlet->Mouse( 0, false, x, y );
	}
    else if (streq(cmd, "appactivated"))
	{
	event_outlet->Broadcast( cscmdFocusChanged, (void*)true);
	}
    else if (streq(cmd, "appdeactivated"))
	{
	event_outlet->Broadcast( cscmdFocusChanged, (void*)false);
	}

#undef AGET
    va_end(args);
    return ok;
}


//-----------------------------------------------------------------------------
// iEventPlug Implementation
//-----------------------------------------------------------------------------
uint NeXTSystemDriver::NeXTSystemEventPlug::GetPotentiallyConflictingEvents()
    { return (CSEVTYPE_Keyboard | CSEVTYPE_Mouse); }
uint NeXTSystemDriver::NeXTSystemEventPlug::QueryEventPriority( uint type )
    { return 150; }
