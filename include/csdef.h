/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
  
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

#if !defined(CSDEF_FRIEND)
#error You are not allowed to include this file! Use cssysdef.h instead.
#endif

#ifndef __CS_CSDEF_H__
#define __CS_CSDEF_H__

#include "platform.h"
#include "cstypes.h"

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

#if !defined(SIGN)
  #define SIGN(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))
#endif

#ifndef PI
 #define PI 3.1415926535897932385f
#endif
#ifndef HALF_PI
  #define HALF_PI PI / 2.0f
#endif
#ifndef TWO_PI
  #define TWO_PI PI * 2.0f
#endif

#undef EPSILON
  #define EPSILON 0.001f			/* Small value */
#undef SMALL_EPSILON
  #define SMALL_EPSILON 0.000001f		/* Very small value */
#undef SMALL_EPSILON_D
  #define SMALL_EPSILON_D 0.000000000001f	/* Very, very small value */

// A macro for defining a constant static table.  The default expansion is
// merely `static const'.  Typical usage is `CS_STATIC_TABLE Foo[] = {...};',
// which expands to `static const Foo[] = {...};'.  Some variants of GCC have
// been known to throw an internal compiler error exception when confronted
// with such an expression.  In this case, the platform-specific header file
// may override the definition of CS_STATIC_TABLE with one which works around
// the compiler bug.
#define CS_STATIC_TABLE static const

// Platforms with compilers which only understand old-style C++ casting syntax
// should define CS_USE_OLD_STYLE_CASTS.
#if defined(CS_USE_OLD_STYLE_CASTS)
  #define CS_CAST(C,T,V) ((T)(V))
#else
  #define CS_CAST(C,T,V) (C<T>(V))
#endif

#define STATIC_CAST(T,V)      CS_CAST(static_cast,T,V)
#define DYNAMIC_CAST(T,V)     CS_CAST(dynamic_cast,T,V)
#define REINTERPRET_CAST(T,V) CS_CAST(reinterpret_cast,T,V)
#define CONST_CAST(T,V)       CS_CAST(const_cast,T,V)

// Platforms with compilers which do not understand the new C++ keyword
// `explicit' should define CS_USE_FAKE_EXPLICIT_KEYWORD.
#if defined(CS_USE_FAKE_EXPLICIT_KEYWORD)
  #define explicit /* nothing */
#endif

// The smallest Z at which 3D clipping occurs
#define SMALL_Z 0.01f

// This macro causes a crash. Can be useful for debugging.
#define CRASH { int* a=0; *a = 1; }

#endif // __CS_CSDEF_H__
