#ifndef __NeXT_NeXTSystemInterface_h
#define __NeXT_NeXTSystemInterface_h
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
// NeXTSystemInterface.h
//
//	A pure SCF interface to the NeXT-specific csSystemDriver.
//
//-----------------------------------------------------------------------------
#include "csutil/scf.h"

SCF_INTERFACE (iNeXTSystemDriver, 0, 0, 1) : public iBase
    {
    virtual int GetSimulatedDepth() const = 0;
    };

#endif // __NeXT_NeXTSystemInterface_h
