#ifndef __NeXT_NeXTConfigFile_h
#define __NeXT_NeXTConfigFile_h
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
// NeXTConfigFile.h
//
//	Plain-C wrapper of a few functions from the SCF iConfigFile interface.
//	Used by the Objective-C NeXTMenu.m.
//
//-----------------------------------------------------------------------------
#if defined(__cplusplus)
#define NCF_PROTO(RET,FUNC) extern "C" RET NeXTConfigFile_##FUNC
#else
#define NCF_PROTO(RET,FUNC) RET NeXTConfigFile_##FUNC
#endif

typedef void* NeXTConfigHandle;
typedef void* NeXTConfigIterator;

NCF_PROTO(char const*,lookup)(
    NeXTConfigHandle, char const* key, char const* fallback );
NCF_PROTO(int,exists)( NeXTConfigHandle, char const* section );
NCF_PROTO(NeXTConfigIterator,new_iterator)(
    NeXTConfigHandle, char const* section );
NCF_PROTO(void,dispose_iterator)( NeXTConfigIterator );
NCF_PROTO(int,iterator_next)( NeXTConfigIterator );
NCF_PROTO(char const*,iterator_key)( NeXTConfigIterator );
NCF_PROTO(char const*,iterator_data)( NeXTConfigIterator );

#undef NCF_PROTO

#endif // __NeXT_NeXTConfigFile_h
