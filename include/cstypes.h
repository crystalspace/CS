/*
    Copyright (C) 1998-2004 by Jorrit Tyberghein
  
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
#include "csplatform.h"
#include <float.h>

#if defined(CS_HAS_STDINT_H)
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#endif

#if defined(CS_HAS_INTTYPES_H)
#include <inttypes.h>
#endif

/**\name Specific sized types
 * These types should be used ONLY when you need a variable of an explicit
 * number of bits.  For all other cases, you should use normal char, short,
 * int, long, etc., types since they are treated as "natural" types and will
 * generally have better performance characteristics than the explicitly-sized
 * types. Use the explicitly-sized types sparingly.
 * @{ */

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
#if defined(CS_COMPILER_GCC)
/// unsigned 64-bit integer
typedef unsigned long long uint64;
/// signed 64-bit integer
typedef long long int64;
#elif defined(CS_COMPILER_MSVC) || defined(CS_COMPILER_BCC)
/// unsigned 64 bit integer
typedef unsigned __int64 uint64;
/// signed 64 bit integer
typedef __int64 int64;
#else
#warning Do not know how to declare 64-bit integers
#endif // CS_COMPILER_GCC

#else // CS_HAS_STDINT_H

typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int64_t int64;
#endif

#ifdef CS_HAS_INT64_C

/**\def CONST_INT64
 * Specify a 64 bit integer constant.
 * Compilers differ in 64-bit wide number specification. Employ this
 * macro to use the appropriate mechanism automatically.
 */
#define CONST_INT64(x) INT64_C(x)

/**\def CONST_UINT64
 * Specify 64 bit unsigned integer constant.
 * Compilers differ in 64-bit wide number specification. Employ this
 * macro to use the appropriate mechanism automatically.
 */
#define CONST_UINT64(x) UINT64_C(x)

#else // CS_HAS_INT64_C

#if defined(CS_COMPILER_GCC)
#define CONST_INT64(x)  x ## LL
#define CONST_UINT64(x) x ## ULL
#elif defined(CS_COMPILER_MSVC) || defined(CS_COMPILER_BCC)
#define CONST_INT64(x)  x##i64
#define CONST_UINT64(x) x##ui64
#else
#warning Do not know how to contruct 64-bit integer constants
#endif // CS_COMPILER_GCC

#endif // CS_HAS_INT64_C

// Provide intptr_t and uintptr_t. If the configure script determined that
// these types exist in the standard headers, then just employ those types.
// For MSVC, where the configure script is not used, check <stddef.h>, which is
// one of several headers which may provide these types. We can tell if
// <stddef.h> provided the types by checking if _INTPTR_T_DEFINED has been
// #defined; newer versions of MSVC will provide them; older ones will not.  If
// all else fails, then we fake up these types on our own.
#include <stddef.h>
#if !defined(CS_HAS_INTPTR_T) && !defined(_INTPTR_T_DEFINED)
/// Integer as wide as a pointer
typedef int intptr_t;
/// Unsigned integer as wide as a pointer
typedef unsigned int uintptr_t;
#define _INTPTR_T_DEFINED
#define _UINTPTR_T_DEFINED
#endif

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
