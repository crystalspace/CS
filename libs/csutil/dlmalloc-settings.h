/*
  Copyright (C) 2006 by Jorrit Tyberghein
            (C) 2006 by Frank Richter

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

#ifndef __CS_CSUTIL_DLMALLOC_SETTINGS_H__
#define __CS_CSUTIL_DLMALLOC_SETTINGS_H__

#define MSPACES 1
#define ONLY_MSPACES 1
#define USE_LOCKS 0
/*#define FOOTERS 1*/
/* Footers...
 * Pro: You can free memory allocated from other modules; if libcrystalspace
 *      is a static library, malloc()/free() behave more like it would be a 
 *      dynamic lib.
 * Con: Overhead.
 */
#if defined (_WIN64)
// Win64 needs 16-byte alignment of malloc
#define MALLOC_ALIGNMENT	16
#else
// Default dlmalloc alignment
#define MALLOC_ALIGNMENT	8
#endif

#ifndef DLMALLOC_DEFINES_ONLY
#ifdef _MSC_VER
/* silence "conversion from 'size_t' to 'bindex_t', possible loss of data" */
#pragma warning(disable:4267)
/* We want speed here */
#pragma optimize("gty", on)
#endif

/* Workaround for recent ming64 releases; they define FORCEINLINE
   to contain an 'extern', which conflicts with the 'static' they
   declared with in dlmalloc.c and ptmalloc3.c. So provide a safe
   definition here. */
#if defined(WIN32) && defined (__GNUC__)
#ifdef FORCEINLINE
#undef FORCEINLINE
#endif
#define FORCEINLINE __inline __attribute__((always_inline))
#endif

/* Cygwin has funny issues with atexit() that ptmalloc seems to tickle.
 * So within ptmalloc we use our own single-use implementation of atexit()
 * when on Cygwin.  
 *
 * With the MSVC runtime(ie MSVC itself and MingW), the catch is that 
 * atexit() functions are called before global static objects are destroyed.
 *
 * On Linux it also seems that sometimes atexit() is called before global static
 * objects are destroyed.
 *
 * See cs_atexit in libs/csutil/ptmalloc_wrap.cpp.
 */
extern int cs_atexit(void(*func)(void));

#define atexit cs_atexit

#endif // DLMALLOC_DEFINES_ONLY

#include "csplatform.h"
#ifdef CS_HAVE_VALGRIND_VALGRIND_H
#define HAVE_VALGRIND_VALGRIND_H
#endif
#ifdef CS_HAVE_VALGRIND_MEMCHECK_H
#define HAVE_VALGRIND_MEMCHECK_H
#endif

#endif // __CS_CSUTIL_DLMALLOC_SETTINGS_H__
