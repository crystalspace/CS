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

/**\file
 */
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

/**\name Specific sized types.
 * The following types should be used whenever you need a variable of
 * a specific size (in bits).  If these types are already defined by system
 * headers for a particular platform, then define CS_BUILTIN_SIZED_TYPES to
 * avoid duplicate type definition here.
 * @{ */
#if !defined(CS_BUILTIN_SIZED_TYPES)
/// unsigned 8-bit integer (0..255)
typedef unsigned char uint8;
/// signed 8-bit integer (-128..127)
typedef char int8;
/// unsigned 16-bit integer (0..65 535)
typedef unsigned short uint16;
/// signed 16-bit integer (-32 768..32 767)
typedef short int16;
/// unsigned 32-bit integer (0..4 294 967 295)
typedef unsigned long uint32;
/// signed 32-bit integer (-2 147 483 648..2 147 483 647)
typedef long int32;
#endif
/** @} */

/// Used for uniquely generated id numbers
typedef uint32 CS_ID;		

/**\name Shortcuts for normal C types
 * @{ */
/// Default unsigned int, for lazy people
typedef unsigned int uint;
/** @} */

/// Time in milli (1/1000) seconds
typedef unsigned int csTicks;

/// A pointer to something
typedef void *csSome;
/// A pointer to some constant
typedef const void *csConstSome;

#endif // __CS_CSTYPES_H__
