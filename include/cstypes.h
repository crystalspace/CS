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
/**\addtogroup util
 * @{
 */
#ifndef __CS_CSSYSDEFS_H__
#error "cssysdef.h must be included in EVERY source file!"
#endif

#include "platform.h"
#include <float.h>

/**\name Specific sized types.
 * The following types should be used whenever you need a variable of
 * a specific size (in bits).  If these types are already defined by system
 * headers for a particular platform, then define CS_BUILTIN_SIZED_TYPES to
 * avoid duplicate type definition here.
 * @{ */
#if !defined(CS_BUILTIN_SIZED_TYPES)


#ifndef CS_HAS_STDINT_H
/// unsigned 8-bit integer (0..255)
typedef unsigned char uint8;
/// signed 8-bit integer (-128..127)
typedef char int8;
/// unsigned 16-bit integer (0..65 535)
typedef unsigned short uint16;
/// signed 16-bit integer (-32 768..32 767)
typedef short int16;
/// unsigned 32-bit integer (0..4 294 967 295)
typedef unsigned int uint32;
/// signed 32-bit integer (-2 147 483 648..2 147 483 647)
typedef int int32;
#ifdef COMP_GCC
/// unsigned 64-bit integer
typedef unsigned long long uint64;
/// signed 64-bit integer
typedef long long int64;
/**\def CONST_INT64
 * Specify a 64 bit integer constant.
 * Different compilers differ in 64-bit wide number specification. Use this
 * macro to automatically use the appropriate way.
 */
#define CONST_INT64(x) x ## LL
/**\def CONST_UINT64
 * Specify 64 bit unsigned integer constant.
 * Different compilers differ in 64-bit wide number specification. Use this
 * macro to automatically use the appropriate way.
 */
#define CONST_UINT64(x) x ## ULL
#else
# if defined(COMP_VC) || defined(COMP_BC) || defined(__BORLANDC__)
/// unsigned 64 bit integer
typedef unsigned __int64 uint64;
/// signed 64 bit integer
typedef __int64 int64;
/// specify 64 bit integer constant
#define CONST_INT64(x) x##i64
/// specify 64 bit unsigned integer constant
#define CONST_UINT64(x) x##ui64
# else
#  warning NO definition for 64 bit integers defined for your compiler
# endif
#endif // end of #ifdef COMP_GCC

#else
// We're happy and can simply use stdint.h.
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int64_t int64;
#define CONST_INT64(x) INT64_C(x)
#define CONST_UINT64(x) UINT64_C(x)
#endif

#endif // end of #if !defined(CS_BUILTIN_SIZED_TYPES)
/** @} */

/// Used for uniquely generated id numbers XXX: remove this sometime
typedef uint32 CS_ID;

/**
 * A time value measured in milliseconds (1/1000 of a second).  Ticks do not
 * represent wall clock time or any other Epoch-based time.  Instead, ticks are
 * useful only for measuring differences between points on a timeline, or for
 * specifying intervals.
 */
typedef unsigned int csTicks;

/**\name Shortcuts for normal C types
 * @{ */
/// Default unsigned int.
typedef unsigned int uint;
/** @} */

/** @} */

#endif // __CS_CSTYPES_H__
