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
// An extensible array of records.
//-----------------------------------------------------------------------------
class NeXTRecordArray
    {
private:
	char* table;
	int record_size;
	int count;
	int capacity;
protected:
virtual	bool compare( void const*, void const* ) const = 0;
virtual	void destroy( void* ) = 0;
	void* search( void const* ) const;
public:
	NeXTRecordArray( int rec_size ) :
		table(0), record_size(rec_size), count(0), capacity(0) {}
	virtual ~NeXTRecordArray() { empty(); }
	void* append();	// Returns address of new record.
	void empty();
    };

void* NeXTRecordArray::append()
    {
    if (count >= capacity)
	{
	capacity += 64;
	int const nbytes = capacity * record_size;
	if (table == 0)
	    table = (char*)malloc( nbytes );
	else
	    table = (char*)realloc( table, nbytes );
	}
    return (table + (record_size * count++));
    }

void* NeXTRecordArray::search( void const* key ) const
    {
    char* p = table;
    for (int i = 0; i < count; i++, p += record_size)
	if (compare( key, p ))
	    return p;
    return 0;
    }

void NeXTRecordArray::empty()
    {
    if (count > 0)
	{
	char* p = table;
	for (int i = 0; i < count; i++, p+= record_size)
	    destroy(p);
	free( table );
	table = 0;
	count = 0;
	capacity = 0;
	}
    }


//-----------------------------------------------------------------------------
// Image File Table -- Associates an image file name with a handle.
//-----------------------------------------------------------------------------
class NeXTImageFileTable : public NeXTRecordArray
    {
private:
	struct Tuple
	    {
	    char const* name;
	    CS_HLIBRARY handle;
	    };
protected:
virtual	bool compare( void const*, void const* ) const;
virtual	void destroy( void* );
public:
	NeXTImageFileTable() : NeXTRecordArray( sizeof(Tuple) ) {}
	void add( char const* name, CS_HLIBRARY );
	CS_HLIBRARY find( char const* ) const;
    };

void NeXTImageFileTable::add( char const* name, CS_HLIBRARY handle )
    {
    Tuple* p = (Tuple*)append();
    p->name = strdup( name );
    p->handle = handle;
    }

CS_HLIBRARY NeXTImageFileTable::find( char const* name ) const
    {
    Tuple const key = { name, 0 };
    Tuple const* tuple = (Tuple const*)search( &key );
    return (tuple == 0 ? 0 : tuple->handle);
    }

bool NeXTImageFileTable::compare( void const* p1, void const* p2 ) const
    {
    return (strcmp(((Tuple const*)p1)->name, ((Tuple const*)p2)->name) == 0);
    }

void NeXTImageFileTable::destroy( void* p )
    {
    free( (char*)((Tuple*)p)->name );
    }


//-----------------------------------------------------------------------------
// Symbol Table -- Associates a {handle:symbol} pair with an address.
//-----------------------------------------------------------------------------
class NeXTSymbolTable : public NeXTRecordArray
    {
private:
	struct Tuple
	    {
	    CS_HLIBRARY handle;
	    char const* symbol;
	    PROC address;
	    };
protected:
virtual	bool compare( void const*, void const* ) const;
virtual	void destroy( void* );
public:
	NeXTSymbolTable() : NeXTRecordArray( sizeof(Tuple) ) {}
	void add( CS_HLIBRARY, char const*, PROC );
	PROC find( CS_HLIBRARY, char const* ) const;
    };

void NeXTSymbolTable::add( CS_HLIBRARY handle, char const* symbol, PROC addr )
    {
    Tuple* p = (Tuple*)append();
    p->handle = handle;
    p->symbol = strdup( symbol );
    p->address = addr;
    }

PROC NeXTSymbolTable::find( CS_HLIBRARY handle, char const* symbol ) const
    {
    Tuple const key = { handle, symbol, 0 };
    Tuple const* tuple = (Tuple const*)search( &key );
    return (tuple == 0 ? 0 : tuple->address);
    }

bool NeXTSymbolTable::compare( void const* p1, void const* p2 ) const
    {
    Tuple const* t1 = (Tuple const*)p1;
    Tuple const* t2 = (Tuple const*)p2;
    return (t1->handle == t2->handle && strcmp(t1->symbol, t2->symbol) == 0);
    }

void NeXTSymbolTable::destroy( void* p )
    {
    free( (char*)((Tuple*)p)->symbol );
    }


//-----------------------------------------------------------------------------
// shared_image_table
//-----------------------------------------------------------------------------
static NeXTImageFileTable& shared_image_table()
    {
    static NeXTImageFileTable* table = 0;
    if (table == 0)
	table = new NeXTImageFileTable;
    return *table;
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
static PROC native_lookup( char const* symbol, bool required = true )
    {
    void* const address = NSAddressOfSymbol( NSLookupAndBindSymbol(symbol) );
    if (address == 0 && required)
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
	PROC const address = native_lookup( symbol, false );
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
    NeXTImageFileTable& images = shared_image_table();
    CS_HLIBRARY handle = images.find( obj );
    if (handle == 0)
	{
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
		images.add( obj, handle );
		register_standard_symbols( handle );
		if (!initialize_module( handle ))
		    {
		    handle = 0; // Failed to initialize.
		    fprintf(stderr,"Unable to initialize library '%s'.\n",obj);
		    }
		}
	    else
		fprintf( stderr, "Unable to link library '%s'.\n", obj );
	    }
	else
	    fprintf( stderr, "Unable to load library '%s' (%d).\n", obj, rc );
	}
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
