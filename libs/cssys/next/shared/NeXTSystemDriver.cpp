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
SysSystemDriver::SysSystemDriver() : csSystemDriver(), proxy(0)
    {
    printf("Crystal Space for " OS_NEXT_DESCRIPTION" " VERSION "\nPorted to "
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
void SysSystemDriver::Initialize( int argc, char* argv[] )
    {
    proxy = new NeXTSystemProxy( this );
    superclass::Initialize( argc, argv );
    }


//-----------------------------------------------------------------------------
// Loop -- Start the Application's run-loop; return at termination.
//-----------------------------------------------------------------------------
void SysSystemDriver::Loop()
    {
    proxy->start_loop(); // Returns when user requests shutdown.
    }
