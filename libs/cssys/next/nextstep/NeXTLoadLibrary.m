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
// NeXTLoadLibrary.m
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
#include "NeXTLoadLibrary.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // access()
#include <mach-o/rld.h>
#include <objc/objc-load.h>
#include <streams/streams.h>

static char* CS_ERROR_MESSAGE = 0;

//-----------------------------------------------------------------------------
// clear_error_message
//-----------------------------------------------------------------------------
static void clear_error_message()
    {
    if (CS_ERROR_MESSAGE != 0)
	{
	free( CS_ERROR_MESSAGE );
	CS_ERROR_MESSAGE = 0;
	}
    }


//-----------------------------------------------------------------------------
// set_error_message
//-----------------------------------------------------------------------------
static void set_error_message( NXStream* stream )
    {
    char* buff;
    int len, maxlen;
    clear_error_message();
    NXGetMemoryBuffer( stream, &buff, &len, &maxlen );
    if (len > 0)
	{
	CS_ERROR_MESSAGE = (char*)malloc( len + 1 );
	strncpy( CS_ERROR_MESSAGE, buff, len );
	CS_ERROR_MESSAGE[ len ] = '\0';
	}
    }


//-----------------------------------------------------------------------------
// NeXTLoadLibrary
//-----------------------------------------------------------------------------
NeXTLibraryHandle NeXTLoadLibrary( char const* path )
    {
    NeXTLibraryHandle handle = 0;
    NXStream* stream = NXOpenMemory( 0, 0, NX_WRITEONLY );
    if (access( path, F_OK ) == 0)
	{
	char const* const files[2] = { path, 0 };
	struct mach_header* header = 0;
	if (objc_loadModules( (char**)files, stream, 0, &header, 0 ) == 0)
	    handle = (NeXTLibraryHandle)header;
	}
    else
	NXPrintf( stream, "File does not exist (%s)", path );
    set_error_message( stream );
    NXCloseMemory( stream, NX_FREEBUFFER );
    return handle;
    }


//-----------------------------------------------------------------------------
// NeXTGetLibrarySymbol
//-----------------------------------------------------------------------------
void* NeXTGetLibrarySymbol( NeXTLibraryHandle handle, char const* s )
    {
    unsigned long address = 0;
    NXStream* stream = NXOpenFile( 2, NX_WRITEONLY );
    char* symbol = (char*)malloc( strlen(s) + 2 ); // '_' + s + '\0'
    *symbol = '_';
    strcpy( symbol + 1, s );
    if (rld_lookup( stream, symbol, &address ) == 0)
	NXPrintf( stream, "Symbol undefined: %s\n", symbol );
    NXClose( stream );
    free( symbol );
    return (void*)address;
    }


//-----------------------------------------------------------------------------
// NeXTUnloadLibrary
//-----------------------------------------------------------------------------
int NeXTUnloadLibrary( NeXTLibraryHandle handle )
    {
    (void)handle;
    return 1; // Unimplemented (1=success).
    }


//-----------------------------------------------------------------------------
// NeXTGetLibraryError
//-----------------------------------------------------------------------------
char const* NeXTGetLibraryError()
    {
    return CS_ERROR_MESSAGE;
    }
