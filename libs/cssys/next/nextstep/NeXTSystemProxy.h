#ifndef __NeXT_NeXTSystemProxy_h
#define __NeXT_NeXTSystemProxy_h
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
// NeXTSystemProxy.h
//
//	C++ object which interacts with Objective-C world on behalf of
//	SysSystemDriver which can not directly interface with Objective-C on
//	account of COM-related conflicts.  See README.NeXT for details.
//
// *WARNING* Do NOT include any COM or Objective-C headers in this file.
//-----------------------------------------------------------------------------
#include "types.h"	// For bool.
class SysSystemDriver;
@class NeXTDelegate;

class NeXTSystemProxy
    {
private:
    SysSystemDriver* driver;	// Our owner, the SysSystemDriver.
    NeXTDelegate* controller;	// Application & Window delegate.
    long ticks;			// Time of previous call to step_frame().

    void init_ticks();
    void step_frame();
    void stop_run_loop();
    bool continue_looping() const;

public:
    NeXTSystemProxy( SysSystemDriver* );
    ~NeXTSystemProxy();

    void start_loop();
    void timer_fired();
    void terminate();
    bool continue_running() const;

    void key_up  ( int ) const;
    void key_down( int ) const;
    void mouse_up  ( int button, int x, int y ) const;
    void mouse_down( int button, int x, int y,
	bool shift, bool alt, bool ctrl ) const;
    void mouse_moved( int x, int y ) const;

    void focus_changed( bool active ) const;
    void clock_running( bool active );
    };

#endif // __NeXT_NeXTSystemProxy_h
