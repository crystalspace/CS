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
//	Implement NextStep-specific dynamic library loading and symbol lookup.
//
//	When a module is loaded dynamically, Crystal Space expects the module
//	to keep its symbols private.  When it wants to know the address of a
//	symbol, it asks for it explicitly via SysGetProcAddress().  It further
//	expects that all dynamically loaded modules define the functions
//	DllInitialize(), DllGetClassObject(), DllRegisterServer(),
//	DllUnregisterServer(), DllCanUnloadNow(), ModuleAddRef(), and
//	ModuleRelease().
//
//	Unfortunately, NextStep's run-time loader, RLD, behaves in a very
//	different and completely imcompatible manner.  RLD actually attempts
//	to bind all symbols from the dynamically loaded module into the
//	running application at load time.  This results in warnings about
//	multiply-defined symbols since every Crystal Space module defines the
//	same set of functions.
//
//	In order to work around this limitation, upon loading a module, this
//	implementation manually looks up the aforementioned symbols, records
//	their values locally, and then instructs RLD to forget about them.
//	In this way, multiply-defined symbol conflicts are avoided when the
//	next module is loaded.
//
//-----------------------------------------------------------------------------
#include "sysdef.h"

#ifdef NO_COM_SUPPORT

#include "cscom/com.h"
extern "C" {
#include <mach-o/rld.h>
#include <objc/objc-load.h>
#include <streams/streams.h>
}

//-----------------------------------------------------------------------------
// Symbol Table -- Associates a {handle:symbol} pair with an address.
// Could be rewritten as a hash table for faster lookups.
//-----------------------------------------------------------------------------
class NeXTSymbolTable
    {
private:
	struct Tuple
	    {
	    CS_HLIBRARY handle;
	    char const* symbol;	// Does not copy symbol name.
	    PROC address;
	    };

	Tuple* table;
	int count;
	int capacity;
public:
	NeXTSymbolTable() : table(0), count(0), capacity(0) {}
	~ NeXTSymbolTable() { if (table != 0) free( table ); }
	void add( CS_HLIBRARY, char const*, PROC );
	PROC find( CS_HLIBRARY, char const* ) const;
    };

void NeXTSymbolTable::add( CS_HLIBRARY handle, char const* symbol, PROC addr )
    {
    if (count >= capacity)
	{
	capacity += 64;
	int const nbytes = capacity * sizeof(table[0]);
	if (table == 0)
	    table = (Tuple*)malloc( nbytes );
	else
	    table = (Tuple*)realloc( table, nbytes );
	}
    Tuple& r = table[ count++ ];
    r.handle = handle;
    r.symbol = symbol;
    r.address = addr;
    }

PROC NeXTSymbolTable::find( CS_HLIBRARY handle, char const* symbol ) const
    {
    Tuple const* p = table;
    for (int i = 0; i < count; i++, p++)
	if (p->handle == handle && strcmp(p->symbol, symbol) == 0)
	    return p->address;
    return 0;
    }


//-----------------------------------------------------------------------------
// shared_symbol_table
//-----------------------------------------------------------------------------
static NeXTSymbolTable& shared_symbol_table()
    {
    static NeXTSymbolTable* table = 0;
    if (table == 0)
	table = new NeXTSymbolTable;
    return *table;
    }


//-----------------------------------------------------------------------------
// native_lookup
//-----------------------------------------------------------------------------
static PROC native_lookup( char const* symbol )
    {
    unsigned long address = 0;
    NXStream* stream = NXOpenFile( 1, NX_WRITEONLY );
    if (rld_lookup( stream, symbol, &address ) == 0)
	NXPrintf( stream, "Symbol undefined: %s\n", symbol );
    NXClose( stream );
    return (PROC)address;
    }


//-----------------------------------------------------------------------------
// register_standard_symbols
//-----------------------------------------------------------------------------
static void register_standard_symbols( CS_HLIBRARY handle )
    {
    static char const* const STANDARD_SYMBOLS[] =
	{
	"_DllInitialize",
	"_DllGetClassObject",
	"_DllRegisterServer",
	"_DllUnregisterServer",
	"_DllCanUnloadNow",
	"_ModuleAddRef",
	"_ModuleRelease"
	};
    int const STANDARD_SYMBOLS_COUNT =
	sizeof(STANDARD_SYMBOLS) / sizeof(STANDARD_SYMBOLS[0]);

    NXStream* stream = NXOpenFile( 1, NX_WRITEONLY );
    NeXTSymbolTable& table = shared_symbol_table();
    for (int i = 0; i < STANDARD_SYMBOLS_COUNT; i++)
	{
	char const* const symbol = STANDARD_SYMBOLS[i];
	PROC const address = native_lookup( symbol );
	if (address != 0)
	    {
	    table.add( handle, symbol, address );
	    rld_forget_symbol( stream, symbol );
	    }
	}
    NXClose( stream );
    }


//-----------------------------------------------------------------------------
// initialize_module
//-----------------------------------------------------------------------------
static bool initialize_module( CS_HLIBRARY handle )
    {
    typedef HRESULT (*DLLInitFunc)();
    NeXTSymbolTable& table = shared_symbol_table();
    DLLInitFunc init_func = (DLLInitFunc)table.find(handle, "_DllInitialize");
    bool const okay = (init_func != 0);
    if (okay)
	init_func();
    return okay;
    }


//-----------------------------------------------------------------------------
// SysLoadLibrary
//-----------------------------------------------------------------------------
CS_HLIBRARY SysLoadLibrary( char const* obj )
    {
    CS_HLIBRARY handle = 0;
    char const* const objs[2] = { obj, 0 };
    struct mach_header* header = 0;
    NXStream* stream = NXOpenFile( 1, NX_WRITEONLY );
    if (objc_loadModules( (char**)objs, stream, 0, &header, 0 ) == 0)
	{
	handle = (CS_HLIBRARY)header;
	register_standard_symbols( handle );
	if (!initialize_module( handle ))
	    {
	    handle = 0;	// Failed to initialize.
	    NXPrintf( stream, "Unable to initialize library '%s'.\n", obj );
	    }
	}
    else
	NXPrintf( stream, "Unable to load library '%s'.\n", obj );
    NXClose( stream );
    return handle;
    }


//-----------------------------------------------------------------------------
// SysGetProcAddress
//-----------------------------------------------------------------------------
PROC SysGetProcAddress( CS_HLIBRARY handle, char const* s )
    {
    char* symbol = new char[ strlen(s) + 2 ];	// Prepend an underscore '_'.
    *symbol = '_';
    strcpy( symbol + 1, s );

    PROC address = shared_symbol_table().find( handle, symbol );
    if (address == 0)
	address = native_lookup( symbol );

    delete[] symbol;
    return address;
    }


//-----------------------------------------------------------------------------
// SysFreeLibrary
//-----------------------------------------------------------------------------
bool SysFreeLibrary( CS_HLIBRARY )
    {
    return true; // Unimplemented.
    }

#endif // NO_COM_SUPPORT
