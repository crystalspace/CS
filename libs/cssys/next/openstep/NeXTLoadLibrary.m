//=============================================================================
//
//	Copyright (C)1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
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
//	MacOS/X Server and OpenStep handle symbolic references within
//	dynamically loaded libraries in a different manner than most other
//	platforms.  On most platforms, all symbols linked into the plug-in
//	module are considered private unless explicitly exported.  On MacOS/X
//	Server and OpenStep, on the other hand, all symbols linked into the
//	plug-in module are public by default.  This can easily lead to
//	symbolic clashes when two or more plug-in modules have been linked
//	with the same libraries or define like-named symbols.  To work around
//	this problem, this dynamic module loader takes advantage the
//	NSLinkEditErrorHandlers facility to instruct DYLD to ignore duplicate
//	symbolic reference at module load time.
//
//	One alternative approach for dealing with this problem would be to
//	ensure that the none of the plug-in modules export like-named symbols,
//	but this is difficult to control in an environment where plug-in
//	modules may come from a variety of sources.  A second alternative
//	would be to strip unneeded symbols from the plug-in module at build
//	time.  This is a fairly decent insurance against symbolic collisions
//	and, in fact, this scheme is used successfully on NextStep.
//	Unfortunately, it is not so easily employed on OpenStep since it also
//	strips away references to Objective-C classes which are defined within
//	the plug-in itself, and the OpenStep run-time complains bitterly about
//	these missing symbols even though the NextStep run-time has no problem
//	with their absence.  (It is possible to instruct the "strip" utility
//	to preserve these symbols, but doing so requires manual intervention
//	and potential future maintenance costs as the list of exported symbols
//	changes.)
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "cssys/csshlib.h"
#include "csutil/csstring.h"

extern "C" {
#include <stdio.h>
#include <unistd.h> // access()
#define bool dyld_bool
#define __private_extern__
#include <mach-o/dyld.h>
#undef __private_extern__
#undef bool
}

static csString CS_ERROR_MESSAGE;
static void clear_error_message() { CS_ERROR_MESSAGE.Free(); }

//-----------------------------------------------------------------------------
// handle_collision
//	When a symbolic collision occurs, choose to publish one of the
//	symbols; the choice of which is rather arbitrary.
//-----------------------------------------------------------------------------
static NSModule handle_collision(NSSymbol sym, NSModule m_old, NSModule m_new)
    {
    return m_old;
    }


//-----------------------------------------------------------------------------
// initialize_loader
//-----------------------------------------------------------------------------
static void initialize_loader()
    {
    static bool installed = false;
    if (!installed)
        {
        installed = true;
        static NSLinkEditErrorHandlers const handlers =
                { 0, handle_collision, 0 };
        NSInstallLinkEditErrorHandlers( (NSLinkEditErrorHandlers*)&handlers );
        }
    }


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
    csLibraryHandle handle = 0;
    initialize_loader();
    clear_error_message();
    if (access( path, F_OK ) == 0)
	{
	NSObjectFileImage image = 0;
	NSObjectFileImageReturnCode rc =
	    NSCreateObjectFileImageFromFile( path, &image );
	if (rc == NSObjectFileImageSuccess)
	    {
	    NSModule const module = NSLinkModule( image, path, TRUE );
	    if (module != 0)
		handle = (csLibraryHandle)module;
	    else
		CS_ERROR_MESSAGE << "NSLinkModule(" << path << ") failed";
	    }
	else
	    CS_ERROR_MESSAGE << "NSCreateObjectFileImageFromFile(" << path
		<< ") " << "failed (error " << rc << ')';
	}
    else
	CS_ERROR_MESSAGE << "File does not exist (" << path << ')';
    return handle;
    }


//-----------------------------------------------------------------------------
// csGetLibrarySymbol
//-----------------------------------------------------------------------------
void* csGetLibrarySymbol( csLibraryHandle handle, char const* s )
    {
    char* symbol = new char[ strlen(s) + 2 ]; // Prepend an underscore '_'.
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


//-----------------------------------------------------------------------------
// csPrintLibraryError
//-----------------------------------------------------------------------------
void csPrintLibraryError( char const* name )
    {
    fprintf( stderr, "ERROR: Failed to load plug-in module `%s'.\n", name );
    if (!CS_ERROR_MESSAGE.IsEmpty())
	{
	fprintf( stderr, "Reason: %s\n", CS_ERROR_MESSAGE.GetData() );
	clear_error_message();
	}
    }
