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

#ifndef __DEF_H__
#define __DEF_H__

#if !defined(OK_TO_INCLUDE_DEFS_IM_A_FRIEND)
#error "You are not allowed to include this file! Use sysdef.h instead."
#endif

#include "types.h"
#include "debug/memory.h"

//---------------------------------------------------------------
// Define the appropriate PROC_ flag for the current architecture
// for NextStep/OpenStep/Rhapsody multi-architecture binary (MAB)
// compilations.
//---------------------------------------------------------------

#if defined(OS_NEXT)
#  if defined(m68k)
#    if !defined(PROC_M68K)
#      define PROC_M68K
#    endif
#  elif defined(i386)
#    if !defined(PROC_INTEL)
#      define PROC_INTEL
#    endif
#  elif defined(sparc)
#    if !defined(PROC_SPARC)
#      define PROC_SPARC
#    endif
#  elif defined(hppa)
#    if !defined(PROC_HPPA)
#      define PROC_HPPA
#    endif
#  elif defined(ppc)
#    if !defined(PROC_POWERPC)
#      define PROC_POWERPC
#    endif
#  else
#    if !defined(PROC_UNKNOWN)
#      define PROC_UNKNOWN
#    endif
#  endif
#endif

#if defined(OS_NEXT)
#  if defined(__BIG_ENDIAN__)
#    define PORT_BYTESEX_BIG_ENDIAN
#  else
#    undef  PORT_BYTESEX_BIG_ENDIAN
#  endif
#endif

//---------------------------------------------------------------
// Test if the makefile correctly defines all the operating
// system (one of the OS_ flags), the compiler (one of the
// COMP_ flags) and the processor (one of the PROC_ flags).
//---------------------------------------------------------------

#if (defined(OS_SOLARIS) || defined(OS_LINUX) || defined(OS_IRIX) || defined(OS_BSD) || defined(OS_BE) || defined(OS_NEXT)) && !defined(OS_UNIX)
#  define OS_UNIX
#endif

#if !defined(OS_SOLARIS) && !defined(OS_LINUX) && !defined(OS_DOS) && !defined(OS_UNIX) && !defined(OS_MACOS) && !defined(OS_AMIGAOS) && !defined(OS_WIN32) && !defined(OS_OS2) && !defined(OS_IRIX) && !defined(OS_BSD) && !defined(OS_BE) && !defined(OS_NEXT)
#  error "Please specify the operating system in the makefile! (OS=...)"
#endif

#if !defined(COMP_GCC) && !defined(COMP_WCC) && !defined(COMP_UNKNOWN) && !defined(COMP_MWERKS) && !defined(COMP_VC)
#  error "Please specify the compiler in the makefile! (COMP=...)"
#endif

#if !defined(PROC_INTEL) && !defined(PROC_SPARC) && !defined(PROC_MIPS) && !defined(PROC_UNKNOWN) && !defined(PROC_POWERPC) && !defined(PROC_M68K) && !defined(PROC_HPPA)
#  error "Please specify the processor in the makefile! (PROC=...)"
#endif

//---------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef ABS
#define ABS(x) ((x)<0?-(x):(x))
#endif

#if !defined(SIGN) && !defined(OS_AMIGAOS)
#define SIGN(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))
#endif

#undef EPSILON
#define EPSILON 0.001				/* Small value */
#undef SMALL_EPSILON
#define SMALL_EPSILON 0.000001			/* Very small value */

#ifndef PI
#  define PI		3.14159265358979323	/* You know this number, don't you? */
#endif
#ifndef M_PI
#  define M_PI PI
#endif
#ifndef M_PI_2
#  define M_PI_2	1.57079632679489661923	/* PI/2 */
#endif

#if defined(COMP_WCC)
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

#if defined(COMP_VC)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

// NextStep 3.3 compiler frequently crashes when initializing static const
// tables of unknown size.  Ex: static const Foo[] = { ... };  Work around
// the problem by removing 'const'.
#if defined(OS_NEXT)
#define CS_STATIC_TABLE static
#else
#define CS_STATIC_TABLE static const
#endif

#if defined(OS_NEXT)
#define CS_USE_OLD_CASTS
#endif

#if defined(CS_USE_OLD_CASTS)
#define CS_CAST(C,T) (T)
#else
#define CS_CAST(C,T) C<T>
#endif

#define STATIC_CAST(T)      CS_CAST(static_cast,T)
#define DYNAMIC_CAST(T)     CS_CAST(dynamic_cast,T)
#define REINTERPRET_CAST(T) CS_CAST(reinterpret_cast,T)
#define CONST_CAST(T)       CS_CAST(const_cast,T)

//#define SMALL_Z .1
#define SMALL_Z .01

// Some useful macros: these should be true at least for 32-bit processors
#define LONGFROM2SHORT(s1,s2) (((short)s1) << 16 | (((short)s2) & 0xffff))
#define SHORT1FROMLONG(l)     (short)(((long)l) >> 16)
#define SHORT2FROMLONG(l)     (short)(((long)l) & 0xffff)

enum SubSystemsIDs
{
  G3D_ID = 0,
  G2D_ID,
  GI_ID,
  NetDrv_ID,
  NetMan_ID
};

#endif // __DEF_H__
