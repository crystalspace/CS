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
#define CS_SYSDEF_PROVIDE_PATH
#include "cssysdef.h"
#include "cssys/next/NeXTSystemDriver.h"
#include "NeXTAssistant.h"
#include "csver.h"

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
NeXTSystemDriver::NeXTSystemDriver() : csSystemDriver(), assistant(0)
{
  printf("Crystal Space for " CS_PLATFORM_NAME " " CS_VERSION "\nPorted to "
    CS_PLATFORM_NAME " by Eric Sunshine <sunshine@sunshineco.com>\n\n");
}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
NeXTSystemDriver::~NeXTSystemDriver()
{
  if (assistant != 0)
  {
    object_reg.Unregister(assistant, "NeXTAssistant");
    assistant->orphan();
    assistant->DecRef();
  }
}


//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
bool NeXTSystemDriver::Initialize(int argc, char const* const argv[])
{
  assistant = new NeXTAssistant(this);
  object_reg.Register(assistant, "NeXTAssistant");
  return superclass::Initialize(argc, argv);
}


//-----------------------------------------------------------------------------
// Loop
//	Start the application's run-loop.  This method is called exactly once
//	at application launch time by Crystal Space to begin the run-loop.  It
//	is illegal to call this method recursively.  Returns when application
//	terminates.
//-----------------------------------------------------------------------------
void NeXTSystemDriver::Loop()
{
  assistant->start_event_loop();
}
