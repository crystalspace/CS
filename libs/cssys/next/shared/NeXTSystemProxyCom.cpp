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
// NeXTSystemProxyCom.cpp
//
//	C++ object which interacts with Objective-C world on behalf of 
//	SysSystemDriver which can not directly interface with Objective-C on 
//	account of COM-related conflicts.  This is the COM-only portion of 
//	NeXTSystemProxy.  See NeXTSystemProxy.cpp for the Objective-C-only 
//	portion of NeXTSystemProxy.  Also see README.NeXT for details.  
//
// *WARNING* Do NOT include any Objective-C headers in this file.
//-----------------------------------------------------------------------------
#include "NeXTSystemProxy.h"
#include "NeXTSystemDriver.h"

//-----------------------------------------------------------------------------
// init_ticks
//-----------------------------------------------------------------------------
void NeXTSystemProxy::init_ticks()
    {
    ticks = driver->Time();
    }


//-----------------------------------------------------------------------------
// continue_running
//-----------------------------------------------------------------------------
bool NeXTSystemProxy::continue_running() const
    {
    return !driver->Shutdown;
    }


//-----------------------------------------------------------------------------
// continue_looping
//-----------------------------------------------------------------------------
bool NeXTSystemProxy::continue_looping() const
    {
    return (!driver->ExitLoop && continue_running());
    }


//-----------------------------------------------------------------------------
// terminate
//-----------------------------------------------------------------------------
void NeXTSystemProxy::terminate()
    {
    driver->Shutdown = true;
    stop_run_loop();
    }


//-----------------------------------------------------------------------------
// step_frame
//-----------------------------------------------------------------------------
void NeXTSystemProxy::step_frame()
    {
    long const now = driver->Time();
    long const elapsed = now - ticks;
    ticks = now;
    driver->NextFrame( elapsed, now );
    }


//-----------------------------------------------------------------------------
// Event Handling
//-----------------------------------------------------------------------------
void NeXTSystemProxy::key_up( int k ) const
    { driver->Keyboard->do_keyrelease( driver->Time(), k ); }

void NeXTSystemProxy::key_down( int k ) const
    { driver->Keyboard->do_keypress( driver->Time(), k ); }

void NeXTSystemProxy::mouse_moved( int x, int y ) const
    { driver->Mouse->do_mousemotion( driver->Time(), x, y ); }

void NeXTSystemProxy::mouse_up( int btn, int x, int y ) const
    { driver->Mouse->do_buttonrelease( driver->Time(), btn, x, y ); }

void NeXTSystemProxy::mouse_down( int btn, int x, int y,
    bool shift, bool alt, bool ctrl ) const
    {
    driver->Mouse->do_buttonpress(driver->Time(), btn, x, y, shift, alt, ctrl);
    }


//-----------------------------------------------------------------------------
// focus_changed
//-----------------------------------------------------------------------------
void NeXTSystemProxy::focus_changed( bool active ) const
    {
    driver->QueueFocusEvent( active );
    }


//-----------------------------------------------------------------------------
// clock_running
//	Reset clock when application resumes after having been paused.  This
//	prevents program AI from experiencing temporal anomalies.
//-----------------------------------------------------------------------------
void NeXTSystemProxy::clock_running( bool active )
    {
    if (active)
	init_ticks();
    }

