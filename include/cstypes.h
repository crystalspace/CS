/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef __CS_CSTYPES_H__
#define __CS_CSTYPES_H__

#ifndef __CS_CSSYSDEFS_H__
#error "cssysdef.h must be included in EVERY source file!"
#endif

#include "platform.h"
#include <float.h>

//-----------------------------------------------------------------------------
// If your compiler complains about 'true', 'false', and 'bool' it may be an
// older C++ compiler which doesn't understand these constructs.  In that case,
// set CS_USE_FAKE_BOOL_TYPE to 'yes' in the makefile, or define the C-macro
// CS_USE_FAKE_BOOL_TYPE in your project file.
//-----------------------------------------------------------------------------
#ifdef CS_USE_FAKE_BOOL_TYPE
typedef int bool;
#undef  true
#define true 1
#undef  false
#define false 0
#endif

//-----------------------------------------------------------------------------
// Make sure the following six types are correct for your system.
//-----------------------------------------------------------------------------
typedef unsigned char UByte;	// Unsigned 8 bit value
typedef signed char SByte;	// Signed 8 bit value
typedef unsigned short UShort;	// Unsigned 16 bit value
typedef signed short SShort;	// Signed 16 bit value
typedef unsigned long ULong;	// Unsigned 32 bit value
typedef signed long SLong;	// Signed 32 bit value
typedef unsigned int UInt;	// Unsigned int value (16..32 bit)
typedef signed int SInt; 	// Signed int value (16..32 bit)

typedef unsigned long CS_ID;    // Used for uniquely generated id numbers

//-----------------------------------------------------------------------------
// The following types should be used whenever you need a variable of
// a specific size (in bits).  If these types are already defined by system
// headers for a particular platform, then define CS_BUILTIN_SIZED_TYPES to
// avoid duplicate type definition here.
//-----------------------------------------------------------------------------
#if !defined(CS_BUILTIN_SIZED_TYPES)
typedef unsigned char uint8;
typedef char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef int int32;
#endif

//------------------------------
// Shortcuts for normal C types
//------------------------------
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// Time in milli (1/1000) seconds
typedef unsigned int csTicks;

// A pointer to something
typedef void *csSome;
// A pointer to some constant
typedef const void *csConstSome;

#endif // __CS_CSTYPES_H__
