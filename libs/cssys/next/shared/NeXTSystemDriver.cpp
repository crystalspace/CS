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
#include "NeXTSystemDriver.h"
#include "version.h"

IMPLEMENT_EMBEDDED_IBASE(NeXTSystemDriver::NeXTSystemInterface)
  IMPLEMENTS_INTERFACE(iNeXTSystemDriver)
IMPLEMENT_EMBEDDED_IBASE_END

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
NeXTSystemDriver::NeXTSystemDriver() : csSystemDriver(), initialized(false),
    controller(0), simulated_depth(0), next_config(0)
    {
    CONSTRUCT_EMBEDDED_IBASE(scfiNeXTSystemDriver);
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
    delete next_config;
    }


//-----------------------------------------------------------------------------
// QueryInterface
//-----------------------------------------------------------------------------
void* NeXTSystemDriver::QueryInterface( char const*iInterfaceID, int iVersion )
    {
    IMPLEMENTS_EMBEDDED_INTERFACE(iNeXTSystemDriver)
    return csSystemDriver::QueryInterface (iInterfaceID, iVersion);
    }


//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
bool NeXTSystemDriver::Initialize( int argc, char const* const argv[],
    char const* cfgfile )
    {
    bool ok = false;
    init_system();
    initialized = true;
    if (superclass::Initialize( argc, argv, cfgfile ))
	{
	next_config = new csIniFile( VFS, "/config/next.cfg" );
	init_menu();
	ok = true;
	}
    return ok;
    }


//-----------------------------------------------------------------------------
// SetSystemDefaults
//-----------------------------------------------------------------------------
#if 0
    This one should move to the corresponding canvas driver
void NeXTSystemDriver::SetSystemDefaults( iConfigFile* config )
    {
    superclass::SetSystemDefaults( config );
    char const* const s = GetOptionCL( "simdepth" );
    simulated_depth = (s != 0 ?
	atoi(s) : config->GetInt( "VideoDriver", "SimulateDepth", 0 ));
    }
#endif


//-----------------------------------------------------------------------------
// Help
//-----------------------------------------------------------------------------
void NeXTSystemDriver::Help()
    {
    superclass::Help();
    Printf( MSG_STDOUT,
	"  -simdepth=<depth>  simulate depth (15 or 32) (default=none)\n" );
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
    step_frame();
    if (!continue_looping())
	stop_run_loop();
    }


//-----------------------------------------------------------------------------
// step_frame
//-----------------------------------------------------------------------------
void NeXTSystemDriver::step_frame()
    {
    NextFrame();
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
// GetSimulatedDepth
//-----------------------------------------------------------------------------
int NeXTSystemDriver::NeXTSystemInterface::GetSimulatedDepth() const
    {
    return scfParent->simulated_depth;
    }
