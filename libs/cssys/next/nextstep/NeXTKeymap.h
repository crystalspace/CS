#ifndef __NeXT_NeXTKeymap_h
#define __NeXT_NeXTKeymap_h
//=============================================================================
//
//	Copyright (C)2000 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// NeXTKeymap.h
//
//	A translation table which maps hardware scan codes to the raw
//	characters generated by those scan codes without any modifier keys
//	active.
//
//	See NeXTKeymap.cpp for an exhaustive explanation of purpose and use of
//	this class.
//
//-----------------------------------------------------------------------------
struct DataStream;

class NeXTKeymap
    {
public:
    struct Binding
	{
	unsigned short code;
	unsigned short character_set;
	enum { INVALID = (unsigned short)~0 };
	int is_bound() const
	    { return (code != INVALID && character_set != INVALID); }
	};
    friend struct Binding;

private:
    Binding* bindings;
    void allocate_binding_records( unsigned long );
    void unparse_characters( DataStream* );
    void unparse_keymap_data( DataStream* );
    int unparse_active_keymap();

public:
    NeXTKeymap();
    ~NeXTKeymap();
    Binding const& binding_for_scan_code( unsigned short n )
	{ return bindings[n]; }
    };

#endif // __NeXT_NeXTKeymap_h
