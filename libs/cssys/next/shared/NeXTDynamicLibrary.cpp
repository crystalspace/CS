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
// NeXTDynamicLibrary.cpp
//
//	Platform-independent C++ cover functions for platform-specific
//	functions for loading dynamic link libraries.  The actual library
//	loading functions have Objective-C bindings and are accessed via
//	plain-C stubs.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "cssys/csshlib.h"
#include "NeXTLoadLibrary.h"

//-----------------------------------------------------------------------------
// csFindLoadLibrary
//-----------------------------------------------------------------------------
csLibraryHandle csFindLoadLibrary( char const* name )
    {
    static bool initialized = false;
    if (!initialized)
	{
	initialized = true;
        csAddLibraryPath( OS_NEXT_PLUGIN_DIR );
	}
    return csFindLoadLibrary( 0, name, OS_NEXT_PLUGIN_EXT );
    }


//-----------------------------------------------------------------------------
// csLoadLibrary
//-----------------------------------------------------------------------------
csLibraryHandle csLoadLibrary( char const* path )
    {
    return (csLibraryHandle)NeXTLoadLibrary( path );
    }


//-----------------------------------------------------------------------------
// csGetLibrarySymbol
//-----------------------------------------------------------------------------
void* csGetLibrarySymbol( csLibraryHandle handle, char const* s )
    {
    return NeXTGetLibrarySymbol( handle, s );
    }


//-----------------------------------------------------------------------------
// csUnloadLibrary
//-----------------------------------------------------------------------------
bool csUnloadLibrary( csLibraryHandle handle )
    {
    return (bool)NeXTUnloadLibrary( handle );
    }


//-----------------------------------------------------------------------------
// csPrintLibraryError
//-----------------------------------------------------------------------------
void csPrintLibraryError( char const* name )
    {
    fprintf( stderr, "ERROR: Failed to load plug-in module `%s'.\n", name );
    char const* s = NeXTGetLibraryError();
    if (s != 0)
	fprintf( stderr, "Reason: %s\n", s );
    }
