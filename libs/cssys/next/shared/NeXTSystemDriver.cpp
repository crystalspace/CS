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
// COM interface to NeXT-specific csSystemDriver.
//-----------------------------------------------------------------------------
BEGIN_INTERFACE_TABLE(SysSystemDriver)
    IMPLEMENTS_COMPOSITE_INTERFACE(System)
    IMPLEMENTS_COMPOSITE_INTERFACE(NeXTSystemDriver)
END_INTERFACE_TABLE()

IMPLEMENT_UNKNOWN_NODELETE(SysSystemDriver)
IMPLEMENT_COMPOSITE_UNKNOWN_AS_EMBEDDED(SysSystemDriver, NeXTSystemDriver)


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
SysSystemDriver::SysSystemDriver() :
    csSystemDriver(), proxy(0), simulated_depth(0)
    {
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
// Initialize -- Create the COM --> Objective-C proxy.
//-----------------------------------------------------------------------------
bool SysSystemDriver::Initialize( int argc, char* argv[], IConfig* pconfig )
    {
    proxy = new NeXTSystemProxy( this );
    return superclass::Initialize( argc, argv, pconfig );
    }


//-----------------------------------------------------------------------------
// SetSystemDefaults
//-----------------------------------------------------------------------------
void SysSystemDriver::SetSystemDefaults()
    {
    superclass::SetSystemDefaults();
    if (config != 0)
	simulated_depth = config->GetInt( "VideoDriver", "SIMULATE_DEPTH", 0 );
    }


//-----------------------------------------------------------------------------
// ParseArg
//-----------------------------------------------------------------------------
bool SysSystemDriver::ParseArg( int argc, char* argv[], int& i )
    {
    bool okay = true;
    if (strcasecmp( "-simdepth", argv[i] ) == 0)
	{
	if (++i < argc)
	    simulated_depth = atoi( argv[i] );
	}
    else
	okay = superclass::ParseArg( argc, argv, i );
    return okay;
    }


//-----------------------------------------------------------------------------
// Help
//-----------------------------------------------------------------------------
void SysSystemDriver::Help()
    {
    superclass::Help();
    Printf( MSG_STDOUT,
	"  -simdepth <depth>  simulate depth (15 or 32) (default=none)\n" );
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
STDMETHODIMP SysSystemDriver::XNeXTSystemDriver::GetSimulatedDepth( int& d )
    {
    METHOD_PROLOGUE( SysSystemDriver, NeXTSystemDriver )
    d = pThis->simulated_depth;
    return S_OK;
    }
