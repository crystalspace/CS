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
// NeXTfopen.cpp
//
//	Implement NeXT-specific fopen() function on behalf of csSystemDriver.
//	MacOS/X Server generates link errors when ::fopen() is used, so we can
//	not use system/general/fopen.cpp.  We work around the problem by
//	adding an extra layer of indirection.
//
//-----------------------------------------------------------------------------
#include "sysdef.h"
#include "cssys/common/system.h"
#include <stdio.h>

static FILE* call_fopen( char const* filename, char const* mode )
    {
    return fopen( filename, mode );
    }

FILE* csSystemDriver::fopen( char const* filename, char const* mode )
    {
    return call_fopen( filename, mode );
    }
