/*
    Fast computation of sqrt(x) and 1/sqrt(x)
    Copyright (C) 2002 by Matthew Reda <reda@mac.com> (PowerPC version)
  
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

/**\file
 * Fast computation of sqrt(x) and 1/sqrt(x).<p> Define CS_NO_QSQRT if you
 * experience mysterious problems with CS which you think are related to the
 * specially optimized csQsqrt() not behaving properly.
 */

#ifndef __CS_CSQSQRT_H__
#define __CS_CSQSQRT_H__

#include "cssysdef.h"
#include <math.h>

/**
 * \addtogroup floating_point
 * @{ */

/**
 * This routine computes sqrt(x) very quickly on Intel and PowerPC platforms.
 */
static inline float csQsqrt (float x);

/**
 * This routine is basically equivalent to csQsqrt() except that it returns
 * 1/sqrt(x) rather than the proper square root. It should be used anywhere
 * you need the inverse root (in 3D graphics it is a common situation),
 * because the routine is a little faster than csQsqrt() and also you avoid
 * a division.
 */
static inline float csQisqrt (float x);

/** @} */

#if !defined(CS_NO_QSQRT) && \
    defined(CS_PROCESSOR_POWERPC) && defined(CS_COMPILER_GCC)

/*
 * Use the PowerPC fsqrte to get an estimate of 1/sqrt(x) Then apply two
 * Newton-Rhaphson refinement steps to get a more accurate response Finally
 * multiply by x to get x/sqrt(x) = sqrt(x).  Add additional refinement steps
 * to get a more accurate result.  Zero is treated as a special case, otherwise
 * we end up returning NaN (Not a Number).
 */
static inline float csQsqrt(float x)
{
  float y0 = 0.0;

  if (x != 0.0)
  {
    float x0 = x * 0.5f;

    __asm__ __volatile__ ("frsqrte %0,%1" : "=f" (y0) : "f" (x));
    
    y0 = y0 * (1.5f - x0 * y0 * y0);
    y0 = (y0 * (1.5f - x0 * y0 * y0)) * x;
  };
    
  return y0;
};

/*
 * Similar to csQsqrt() above, except we do not multiply by x at the end, and
 * return 1/sqrt(x).
 */
static inline float csQisqrt(float x)
{
  float x0 = x * 0.5f;
  float y0;
  __asm__ __volatile__ ("frsqrte %0,%1" : "=f" (y0) : "f" (x));
    
  y0 = y0 * (1.5f - x0 * y0 * y0);
  y0 = y0 * (1.5f - x0 * y0 * y0);

  return y0;
};

#else

static inline float csQsqrt (float x) { return sqrtf(x); }
static inline float csQisqrt(float x) { return 1.0f / sqrtf(x); }

#endif

#endif // __CS_CSQSQRT_H__
