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
// NeXTConfigFile.cpp
//
//	Plain-C wrapper of a few functions from the SCF iConfigFile interface.
//	Used by the Objective-C NeXTMenu.m.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "iutil/icfgfile.h"
#include "NeXTConfigFile.h"

#define NCF_PROTO(RET,FUNC) RET NeXTConfigFile_##FUNC

NCF_PROTO(char const*,lookup)(
    NeXTConfigHandle handle, char const* key, char const* fallback )
    { return ((iConfigFile*)handle)->GetStr( key, fallback ); }
NCF_PROTO(int,exists)( NeXTConfigHandle handle, char const* section )
    { return ((iConfigFile*)handle)->SubsectionExists( section ); }
NCF_PROTO(NeXTConfigIterator,new_iterator)( NeXTConfigHandle handle,
    char const* section ) { return (NeXTConfigIterator)
    ((iConfigFile*)handle)->Enumerate( section ); }
NCF_PROTO(void,dispose_iterator)( NeXTConfigIterator handle )
    { ((iConfigIterator*)handle)->DecRef(); }
NCF_PROTO(int,iterator_next)( NeXTConfigIterator handle )
    { return ((iConfigIterator*)handle)->Next(); }
NCF_PROTO(char const*,iterator_key)( NeXTConfigIterator handle )
    { return ((iConfigIterator*)handle)->GetKey( true ); }
NCF_PROTO(char const*,iterator_data)( NeXTConfigIterator handle )
    { return ((iConfigIterator*)handle)->GetStr(); }

#undef NCF_PROTO
