#ifndef __NeXT_NeXTLoadLibrary_h
#define __NeXT_NeXTLoadLibrary_h
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
// NeXTLoadLibrary.h
//
//	Platform-specific functions for loading dynamic link libraries.
//
//-----------------------------------------------------------------------------
#if defined(__cplusplus)
extern "C" {
#endif

typedef void* NeXTLibraryHandle;

NeXTLibraryHandle NeXTLoadLibrary( char const* path );
void* NeXTGetLibrarySymbol( NeXTLibraryHandle, char const* name );
int NeXTUnloadLibrary( NeXTLibraryHandle );
char const* NeXTGetLibraryError();

#if defined(__cplusplus)
}
#endif

#endif // __NeXT_NeXTLoadLibrary_h
