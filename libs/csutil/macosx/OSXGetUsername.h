#ifndef __MACOSX_OSXGetUsername_h
#define __MACOSX_OSXGetUsername_h
//=============================================================================
//
//	Copyright (C)2002 by Eric Sunshine <sunshine@sunshineco.com>
//
// The contents of this file are copyrighted by Eric Sunshine.  This work is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  You may distribute this file provided that this
// copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXGetUsername.h
//
//	Platform-specific function to return the name of the logged in user.
//
//-----------------------------------------------------------------------------
#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

int OSXGetUsername(char* buff, size_t sz);

#if defined(__cplusplus)
}
#endif

#endif // __MACOSX_OSXGetUsername_h
