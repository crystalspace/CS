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

#ifndef __CS_TYPES_H
#define __CS_TYPES_H

#include "platform.h"

//-----------------------------------------------------------------------------
// If your compiler complains about 'true', 'false', and 'bool' it
// may be an older C++ compiler which doesn't understand these constructs.
// In that case, set NEED_FAKE_BOOL to 'yes' in the makefile, or define the
// C-macro NO_BOOL_TYPE in your project file.
//-----------------------------------------------------------------------------
#ifdef NO_BOOL_TYPE
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

#endif // __CS_TYPES_H
