#ifndef __NeXT_NeXTSystemDriver_h
#define __NeXT_NeXTSystemDriver_h
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
// NeXTSystemDriver.h
//
//	NeXT-specific hardware & operating/system drivers for CrystalSpace.
//
//-----------------------------------------------------------------------------
#include "sysdef.h"
#include "cssys/system.h"
#include "csinput/csinput.h"
#include "NeXTSystemInterface.h"
@class NeXTDelegate;

//-----------------------------------------------------------------------------
// NeXT-specific keyboard and mouse drivers.
//-----------------------------------------------------------------------------
class SysKeyboardDriver : public csKeyboardDriver {};
class SysMouseDriver    : public csMouseDriver    {};


//-----------------------------------------------------------------------------
// NeXT-specific subclass of csSystemDriver.
//-----------------------------------------------------------------------------
class NeXTSystemDriver : public csSystemDriver
    {
    typedef csSystemDriver superclass;

private:
    bool initialized;		// System initialized?
    NeXTDelegate* controller;	// Application & Window delegate.
    long ticks;			// Time of previous call to step_frame().
    int simulated_depth;	// Simulated depth, either 15 or 32.

    void init_ticks() { ticks = Time(); }
    void init_system();
    void shutdown_system();
    void step_frame();
    void start_loop();
    void stop_run_loop();
    bool continue_looping() const { return (!ExitLoop && continue_running()); }

public:
    NeXTSystemDriver();
    virtual ~NeXTSystemDriver();
    virtual bool Initialize( int argc, char* argv[], char const* cfgfile );
    virtual void SetSystemDefaults( csIniFile* );
    virtual void Help();
    virtual void Loop ();

    bool continue_running() const { return !Shutdown; }
    void timer_fired();
    void terminate();

    void pause_clock() {}
    void resume_clock() { init_ticks(); }  // Prevent AI temporal anomalies.

    // Implement iNeXTSystemDriver interface.
    struct NeXTSystemInterface : public iNeXTSystemDriver
	{
	DECLARE_EMBEDDED_IBASE(NeXTSystemDriver);
	virtual int GetSimulatedDepth() const;
	} scfiNeXTSystemDriver;
    friend struct NeXTSystemInterface;
    DECLARE_IBASE;
    };

class SysSystemDriver : public NeXTSystemDriver {};

#endif // __NeXT_NeXTSystemDriver_h
