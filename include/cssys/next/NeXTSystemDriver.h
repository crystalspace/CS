#ifndef __NeXT_NeXTSystemDriver_h
#define __NeXT_NeXTSystemDriver_h
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
// NeXTSystemDriver.h
//
//	NeXT-specific hardware & operating/system drivers for Crystal Space.
//	This file contains code which is shared between MacOS/X, MacOS/X
//	Server 1.0 (Rhapsody), OpenStep, and NextStep.  See NeXTDelegate.h for
//	the platform-specific portion of the system driver.
//
//-----------------------------------------------------------------------------
#include "cssys/system.h"
struct iObjectRegistry;

class SysSystemDriver : public csSystemDriver
{
public:
  SysSystemDriver(iObjectRegistry* r) : csSystemDriver(r) {}
};

#endif // __NeXT_NeXTSystemDriver_h
