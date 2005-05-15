/*
    Copyright (C) 2005 by Frank Richter

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __IMAGELIB_COMMON_H__
#define __IMAGELIB_COMMON_H__

#include "cssysdef.h"
#include "cstypes.h"
#include "csgeom/math.h"

/* Types used by ImageLib, but are typedefed by the Win32 SDK. */
#ifndef WIN32
typedef uint8 BYTE;
typedef uint16 WORD;
typedef uint32 DWORD;
typedef bool BOOL;
#define TRUE true
#define FALSE false
#endif

#ifndef __min
#define __min(a,b)	csMin((long)(a), (long)(b))
#endif
#ifndef __max
#define __max(a,b)	csMax((long)(a), (long)(b))
#endif

#endif // __IMAGELIB_COMMON_H__
