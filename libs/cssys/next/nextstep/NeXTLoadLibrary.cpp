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
//	NextStep-specific dynamic library loading and symbol lookup.
//
//	NextStep handle symbolic references within dynamically loaded
//	libraries in a different manner than most other platforms.  On most
//	platforms, all symbols linked into the plug-in module are considered
//	private unless explicitly exported.  On NextStep, on the other hand,
//	all symbols linked into the plug-in module are public by default.
//	This can easily lead to symbolic clashes when two or more plug-in
//	modules have been linked with the same libraries or define like-named
//	symbols.  To work around this problem, unneeded symbols are stripped
//	from the plug-in module at build time, such that the only symbols
//	which are actually published are the ones which SCF expects to be
//	present in a plug-in module.  (Specifically, SCF expects the
//	`module_GetClassTable' symbol to be present.)
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "cssys/csshlib.h"
#include "csutil/csstring.h"

extern "C" {
#include <unistd.h> // access()
#include <mach-o/rld.h>
#include <objc/objc-load.h>
#include <streams/streams.h>
}

static csString CS_ERROR_MESSAGE;
static void clear_error_message() { CS_ERROR_MESSAGE.Free(); }

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
    clear_error_message();
    if (access( path, F_OK ) == 0)
	{
	char const* const files[2] = { path, 0 };
	struct mach_header* header = 0;
	NXStream* stream = NXOpenMemory( 0, 0, NX_WRITEONLY );
	if (objc_loadModules( (char**)files, stream, 0, &header, 0 ) == 0)
	    handle = (csLibraryHandle)header;
	else
	    {
	    char* buff;
	    int len, maxlen;
	    NXGetMemoryBuffer( stream, &buff, &len, &maxlen );
	    CS_ERROR_MESSAGE.Append( buff, len );
	    }
	NXCloseMemory( stream, NX_FREEBUFFER );
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
    csString symbol;
    symbol << '_' << s;
    unsigned long address = 0;
    NXStream* stream = NXOpenFile( 2, NX_WRITEONLY );
    if (rld_lookup( stream, symbol, &address ) == 0)
	NXPrintf( stream, "Symbol undefined: %s\n", symbol.GetData() );
    NXClose( stream );
    return (void*)address;
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
