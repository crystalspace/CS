#ifndef __MACOSX_OSXConfigFile_h
#define __MACOSX_OSXConfigFile_h
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
// OSXConfigFile.h
//
//	Plain-C wrapper of a few functions from the SCF iConfigFile interface.
//	Used by the Objective-C OSXMenu.m.
//
//-----------------------------------------------------------------------------
#if defined(__cplusplus)
#define NCF_PROTO(RET,FUNC) extern "C" RET OSXConfigFile_##FUNC
#else
#define NCF_PROTO(RET,FUNC) RET OSXConfigFile_##FUNC
#endif

typedef void* OSXConfigHandle;
typedef void* OSXConfigIterator;

NCF_PROTO(char const*,lookup)(
  OSXConfigHandle, char const* key, char const* fallback);
NCF_PROTO(int,exists)(OSXConfigHandle, char const* section);
NCF_PROTO(OSXConfigIterator,new_iterator)(
  OSXConfigHandle, char const* section);
NCF_PROTO(void,dispose_iterator)(OSXConfigIterator);
NCF_PROTO(int,iterator_next)(OSXConfigIterator);
NCF_PROTO(char const*,iterator_key)(OSXConfigIterator);
NCF_PROTO(char const*,iterator_data)(OSXConfigIterator);

#undef NCF_PROTO

#endif // __MACOSX_OSXConfigFile_h
