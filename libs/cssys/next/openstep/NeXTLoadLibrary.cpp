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
//	Implement OpenStep-specific dynamic library loading and symbol lookup.
//
//	When a module is loaded dynamically, Crystal Space expects the module
//	to keep its symbols private.  When it wants to know the address of a
//	symbol, it asks for it explicitly via SysGetProcAddress().  It further
//	expects that all dynamically loaded modules define the functions
//	DllInitialize(), DllGetClassObject(), DllRegisterServer(),
//	DllUnregisterServer(), DllCanUnloadNow(), ModuleAddRef(), and
//	ModuleRelease().
//
//	Unfortunately, OpenStep's dynamic loader, DYLD, behaves in a very
//	different and completely imcompatible manner.  DYLD actually attempts
//	to bind all symbols from the dynamically loaded module into the
//	running application at load time.  This results in warnings about
//	multiply-defined symbols since every Crystal Space module defines the
//	same set of functions.
//
//	In order to work around this limitation, upon loading a module, this
//	implementation manually looks up the aforementioned symbols and
//	records their values locally.  When a collision occurs as the next
//	module is loaded dynamically, DYLD is instructed to forget the old
//	symbol and install the new.
//
//-----------------------------------------------------------------------------
#include "sysdef.h"

#ifdef NO_COM_SUPPORT

#include "cscom/com.h"
extern "C" {
#include <stdio.h>
#define bool dyld_bool
#define __private_extern__
#include <mach-o/dyld.h>
#undef __private_extern__
#undef bool
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
    void* const address = NSAddressOfSymbol( NSLookupAndBindSymbol(symbol) );
    if (address == 0)
	fprintf( stderr, "Symbol undefined: %s\n", symbol );
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

    NeXTSymbolTable& table = shared_symbol_table();
    for (int i = 0; i < STANDARD_SYMBOLS_COUNT; i++)
	{
	char const* const symbol = STANDARD_SYMBOLS[i];
	PROC const address = native_lookup( symbol );
	if (address != 0)
	    table.add( handle, symbol, address );
	}
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
// handle_collision
//-----------------------------------------------------------------------------
static NSModule handle_collision(NSSymbol sym, NSModule m_old, NSModule m_new)
    {
    return m_new;
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
// SysLoadLibrary
//-----------------------------------------------------------------------------
CS_HLIBRARY SysLoadLibrary( char const* obj )
    {
    CS_HLIBRARY handle = 0;
    initialize_loader();
    NSObjectFileImage image = 0;
    NSObjectFileImageReturnCode rc =
	NSCreateObjectFileImageFromFile( obj, &image );
    if (rc == NSObjectFileImageSuccess)
	{
	NSModule module = NSLinkModule( image, obj, TRUE );
	if (module != 0)
	    {
	    handle = (CS_HLIBRARY)module;
	    register_standard_symbols( handle );
	    if (!initialize_module( handle ))
		{
		handle = 0; // Failed to initialize.
		fprintf( stderr, "Unable to initialize library '%s'.\n", obj );
		}
	    }
	else
	    fprintf( stderr, "Unable to link library '%s'.\n", obj );
	}
    else
	fprintf( stderr, "Unable to load library '%s' (%d).\n", obj, rc );
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
