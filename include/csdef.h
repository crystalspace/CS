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
#error You are not allowed to include this file! Include cssysdef.h instead.
#endif

#ifndef __CS_CSDEF_H__
#define __CS_CSDEF_H__

#include "csplatform.h"
#include "cstypes.h"

#include <stdio.h>
#include <stdlib.h>
#if defined(CS_HAVE_CMATH_H)
#include <cmath>
#else
#include <math.h>
#endif
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#ifdef CS_HAVE_SYS_PARAM_H
#include <sys/param.h>
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

#ifndef SIGN
  #define SIGN(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))
#endif

#ifndef PI
 #define PI 3.1415926535897932385f
#endif
#ifndef HALF_PI
  #define HALF_PI (PI / 2.0f)
#endif
#ifndef TWO_PI
  #define TWO_PI (PI * 2.0f)
#endif

#undef EPSILON
#define EPSILON 0.001f			/* Small value */
#undef SMALL_EPSILON
#define SMALL_EPSILON 0.000001f		/* Very small value */
#undef SMALL_EPSILON_D
#define SMALL_EPSILON_D 0.000000000001f	/* Very, very small value */

// Platforms which have floating-point variations of the standard math.h
// cos(), sin(), tan(), sqrt(), etc. functions should define
// CS_HAVE_MATH_H_FLOAT_FUNCS. For platforms which do not provide these
// macros or if strict-ANSI conformance is requested, we fake them up.
#if !defined(CS_HAVE_MATH_H_FLOAT_FUNCS) || defined(__STRICT_ANSI__)
  #define acosf(X)    static_cast<float> (acos(X))
  #define asinf(X)    static_cast<float> (asin(X))
  #define atan2f(X,Y) static_cast<float> (atan2(X,Y))
  #define atanf(X)    static_cast<float> (atan(X))
  #define cosf(X)     static_cast<float> (cos(X))
  #define exp2f(X)    static_cast<float> (exp2(X))
  #define expf(X)     static_cast<float> (exp(X))
  #define fabsf(X)    static_cast<float> (fabs(X))
  #define log10f(X)   static_cast<float> (log10(X))
  #define log2f(X)    static_cast<float> (log2(X))
  #define logf(X)     static_cast<float> (log(X))
  #define powf(X)     static_cast<float> (pow(X))
  #define sinf(X)     static_cast<float> (sin(X))
  #define sqrtf(X)    static_cast<float> (sqrt(X))
  #define tanf(X)     static_cast<float> (tan(X))
  #define floorf(X)   static_cast<float> (floor(X))
  #define ceilf(X)    static_cast<float> (ceil(X))
#endif

// Platforms which have the PRIx99 printf()-formatting directives should define
// CS_HAVE_C_FORMAT_MACROS. For platforms which do not provide these macros, we
// fake up the commonly used ones.
#if CS_LONG_SIZE == 8
  #define __CS_PRI64_PREFIX	"l"
#else
  #define __CS_PRI64_PREFIX	"ll"
#endif

#define CS_PRId64 __CS_PRI64_PREFIX "d"
#define CS_PRIu64 __CS_PRI64_PREFIX "u"
#define CS_PRIx64 __CS_PRI64_PREFIX "x"
#define CS_PRIX64 __CS_PRI64_PREFIX "X"

#if !defined(CS_HAVE_C_FORMAT_MACROS)
  #ifndef PRId8
    #define PRId8  "d"
  #endif
  #ifndef PRId16
    #define PRId16 "d"
  #endif
  #ifndef PRId32
    #define PRId32 "d"
  #endif
  #ifndef PRId64
    #define PRId64 __CS_PRI64_PREFIX "d"
  #endif
  #ifndef PRIu8
    #define PRIu8  "u"
  #endif
  #ifndef PRIu16
    #define PRIu16 "u"
  #endif
  #ifndef PRIu32
    #define PRIu32 "u"
  #endif
  #ifndef PRIu64
    #define PRIu64 __CS_PRI64_PREFIX "u"
  #endif
  #ifndef PRIx8
    #define PRIx8  "x"
  #endif
  #ifndef PRIx16
    #define PRIx16 "x"
  #endif
  #ifndef PRIx32
    #define PRIx32 "x"
  #endif
  #ifndef PRIx64
    #define PRIx64 __CS_PRI64_PREFIX "x"
  #endif
  #ifndef PRIX8
    #define PRIX8  "X"
  #endif
  #ifndef PRIX16
    #define PRIX16 "X"
  #endif
  #ifndef PRIX32
    #define PRIX32 "X"
  #endif
  #ifndef PRIX64
    #define PRIX64 __CS_PRI64_PREFIX "X"
  #endif
  
  #if CS_PROCESSOR_SIZE == 64
    #ifndef PRIuPTR
      #define PRIuPTR  PRIu64
    #endif

    #ifndef PRIxPTR
      #define PRIxPTR  PRIx64
    #endif
    
    #ifndef PRIdPTR
      #define PRIdPTR  PRId64
    #endif
    
    #ifndef PRIXPTR
      #define PRIXPTR  PRIX64
    #endif
  #else
    #ifndef PRIuPTR
      #define PRIuPTR  PRIu32
    #endif

    #ifndef PRIxPTR
      #define PRIxPTR  PRIx32
    #endif
    
    #ifndef PRIdPTR
      #define PRIdPTR  PRId32
    #endif
    
    #ifndef PRIXPTR
      #define PRIXPTR  PRIX32
    #endif
  #endif
#endif

// The smallest Z at which 3D clipping occurs
#define SMALL_Z 0.01f

#endif // __CS_CSDEF_H__
