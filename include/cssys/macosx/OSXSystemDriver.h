#ifndef __OSX_OSXSystemDriver_h
#define __OSX_OSXSystemDriver_h
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
// OSXSystemDriver.h
//
//	MacOS/X-specific hardware & operating/system drivers for Crystal Space.
//	See OSXDelegate.h for the Cocoa-specific portion of the system driver.
//
//-----------------------------------------------------------------------------
#include "cssys/system.h"
struct iObjectRegistry;

class SysSystemDriver : public csSystemDriver
{
public:
  SysSystemDriver(iObjectRegistry* r) : csSystemDriver(r) {}
};

#endif // __OSX_OSXSystemDriver_h
