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
// NeXTLoadLibrary.cpp
//
//	OpenStep-specific dynamic library loading and symbol lookup.
//
//-----------------------------------------------------------------------------
#include "sysdef.h"
#include "cssys/csshlib.h"

extern "C" {
#include <stdio.h>
#define bool dyld_bool
#define __private_extern__
#include <mach-o/dyld.h>
#undef __private_extern__
#undef bool
}

//-----------------------------------------------------------------------------
// csLoadLibrary
//-----------------------------------------------------------------------------
csLibraryHandle csLoadLibrary( char const* lib )
    {
    static char const LIBRARY_EXT[] = ".dylib";
    char* file = new char[strlen(lib) + sizeof(LIBRARY_EXT)]; // Includes '\0'.
    strcpy( file, lib );
    strcat( file, LIBRARY_EXT );

    csLibraryHandle handle = 0;
    NSObjectFileImage image = 0;
    NSObjectFileImageReturnCode rc =
	NSCreateObjectFileImageFromFile( file, &image );
    if (rc == NSObjectFileImageSuccess)
	{
	NSModule const module = NSLinkModule( image, file, TRUE );
	if (module != 0)
	    handle = (csLibraryHandle)module;
	else
	    fprintf( stderr, "Unable to link library '%s'.\n", file );
	}
    else
	fprintf( stderr, "Unable to load library '%s' (%d).\n", file, rc );

    delete file;
    return handle;
    }


//-----------------------------------------------------------------------------
// csGetLibrarySymbol
//-----------------------------------------------------------------------------
void* csGetLibrarySymbol( csLibraryHandle handle, char const* s )
    {
    char* symbol = new char[ strlen(s) + 2 ];	// Prepend an underscore '_'.
    *symbol = '_';
    strcpy( symbol + 1, s );

    void* const address = NSAddressOfSymbol( NSLookupAndBindSymbol(symbol) );
    if (address == 0)
	fprintf( stderr, "Symbol undefined: %s\n", symbol );

    delete[] symbol;
    return address;
    }


//-----------------------------------------------------------------------------
// csUnloadLibrary
//-----------------------------------------------------------------------------
bool csUnloadLibrary( csLibraryHandle )
    {
    return true; // Unimplemented.
    }
