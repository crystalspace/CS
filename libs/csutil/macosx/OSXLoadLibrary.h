#ifndef __MACOSX_OSXLoadLibrary_h
#define __MACOSX_OSXLoadLibrary_h
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
// OSXLoadLibrary.h
//
//	Platform-specific functions for loading dynamic link libraries.
//
//-----------------------------------------------------------------------------
#if defined(__cplusplus)
extern "C" {
#endif

typedef void* OSXLibraryHandle;

OSXLibraryHandle OSXLoadLibrary(char const* path);
void* OSXGetLibrarySymbol(OSXLibraryHandle, char const* name);
int OSXUnloadLibrary(OSXLibraryHandle);
char const* OSXGetLibraryError();

#if defined(__cplusplus)
}
#endif

#endif // __MACOSX_OSXLoadLibrary_h
