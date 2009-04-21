#ifndef __MACOSX_OSXInstallPath_h
#define __MACOSX_OSXInstallPath_h
//=============================================================================
//
//	Copyright (C) 2002 by Matt Reda <mreda@mac.com>
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
// License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//=============================================================================
//-----------------------------------------------------------------------------
// OSXInstallPath.h
//
//	Platform-specific function to determine installation path.
//
//-----------------------------------------------------------------------------
#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

int OSXGetInstallPath(char* buff, size_t sz, char path_ep);

#if defined(__cplusplus)
}
#endif

#endif // __MACOSX_OSXInstallPath_h
