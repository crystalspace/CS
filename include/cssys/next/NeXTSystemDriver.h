#ifndef __NeXT_NeXTSystemDriver_h
#define __NeXT_NeXTSystemDriver_h
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
// NeXTSystemDriver.h
//
//	NeXT-specific hardware & operating/system drivers for CrystalSpace.
//
//-----------------------------------------------------------------------------
#include "cssys/system.h"
#include "ievent.h"
@class NeXTDelegate;

//-----------------------------------------------------------------------------
// NeXT-specific subclass of csSystemDriver.
//-----------------------------------------------------------------------------
class NeXTSystemDriver : public csSystemDriver
    {
    typedef csSystemDriver superclass;

private:
    bool initialized;		// System initialized?
    NeXTDelegate* controller;	// Application & Window delegate.
    iEventOutlet* event_outlet;	// Shared event outlet.

    void init_menu( iConfigFile* );
    void init_system();
    void shutdown_system();
    void start_loop();
    void stop_run_loop();
    bool continue_looping() const { return (!ExitLoop && continue_running()); }

public:
    DECLARE_IBASE_EXT(csSystemDriver);

    NeXTSystemDriver();
    virtual ~NeXTSystemDriver();
    virtual bool Initialize( int argc, char const* const argv[], char const* );
    virtual bool SystemExtension( char const*, ... );
    virtual void Loop();

    bool continue_running() const { return !Shutdown; }
    void timer_fired();
    void terminate();

    // Prevent AI temporal anomalies.
    void pause_clock() { SuspendVirtualTimeClock(); }
    void resume_clock() { ResumeVirtualTimeClock(); }

    // Implement iEventPlug interface.
    struct NeXTSystemEventPlug : public iEventPlug
	{
	DECLARE_EMBEDDED_IBASE(NeXTSystemDriver);
	virtual uint GetPotentiallyConflictingEvents();
	virtual uint QueryEventPriority( uint type );
	} scfiEventPlug;
    };

class SysSystemDriver : public NeXTSystemDriver {};

#endif // __NeXT_NeXTSystemDriver_h
