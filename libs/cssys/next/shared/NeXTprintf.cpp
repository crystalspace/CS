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
// NeXTprintf.cpp
//
//	Implement NeXT-specific printf() function on behalf of csSystemDriver.
//	Rhapsody DR2 (MacOS/X Server) generates link errors when ::printf() is
//	used, so we can not use system/general/printf.cpp.  We work around the
//	problem by going straight to vprintf().
//
//-----------------------------------------------------------------------------
#include "system/system.h"
#include <stdarg.h>
#include <stdio.h>

void csSystemDriver::printf_init () {}
void csSystemDriver::printf_close() {}

int csSystemDriver::printf( char* fmt, ... )
    {
    va_list args;
    va_start( args, fmt );
    int const rc = vprintf( fmt, args );
    va_end( args );
    fflush( stdout );
    return rc;
    }
