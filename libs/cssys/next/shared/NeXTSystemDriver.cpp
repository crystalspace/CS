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
// NeXTSystemDriver.cpp
//
//	NeXT-specific hardware & operating/system drivers for CrystalSpace.
//
//-----------------------------------------------------------------------------
#include "NeXTSystemDriver.h"
#include "NeXTSystemProxy.h"
#include "version.h"
#include "csutil/inifile.h"

//-----------------------------------------------------------------------------
// SCF interface to NeXT-specific csSystemDriver.
//-----------------------------------------------------------------------------
IMPLEMENT_IBASE(SysSystemDriver)
    IMPLEMENTS_INTERFACE(iSystem)
    IMPLEMENTS_INTERFACE(iNeXTSystemDriver)
IMPLEMENT_IBASE_END


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
SysSystemDriver::SysSystemDriver() :
    csSystemDriver(), proxy(0), simulated_depth(0)
    {
    CONSTRUCT_IBASE(0);
    printf("Crystal Space for " OS_NEXT_DESCRIPTION " " VERSION "\nPorted to "
	OS_NEXT_DESCRIPTION " by Eric Sunshine <sunshine@sunshineco.com>\n\n");
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
SysSystemDriver::~SysSystemDriver()
    {
    if (proxy != 0)
	delete proxy;
    }


//-----------------------------------------------------------------------------
// Initialize -- Create the SCF --> Objective-C proxy.
//-----------------------------------------------------------------------------
bool SysSystemDriver::Initialize( int argc, char* argv[], char const* cfgfile )
    {
    proxy = new NeXTSystemProxy( this );
    return superclass::Initialize( argc, argv, cfgfile );
    }


//-----------------------------------------------------------------------------
// SetSystemDefaults
//-----------------------------------------------------------------------------
void SysSystemDriver::SetSystemDefaults( csIniFile* config )
    {
    superclass::SetSystemDefaults(config);
    char const* const s = GetOptionCL( "simdepth" );
    simulated_depth = (s != 0 ?
	atoi(s) : config->GetInt( "VideoDriver", "SimulateDepth", 0 ));
    }


//-----------------------------------------------------------------------------
// Help
//-----------------------------------------------------------------------------
void SysSystemDriver::Help()
    {
    superclass::Help();
    Printf( MSG_STDOUT,
	"  -simdepth=<depth>  simulate depth (15 or 32) (default=none)\n" );
    }


//-----------------------------------------------------------------------------
// Loop -- Start the Application's run-loop; return at termination.
//-----------------------------------------------------------------------------
void SysSystemDriver::Loop()
    {
    proxy->start_loop(); // Returns when user requests shutdown.
    }


//-----------------------------------------------------------------------------
// GetSimulatedDepth
//-----------------------------------------------------------------------------
int SysSystemDriver::GetSimulatedDepth() const
    {
    return simulated_depth;
    }
