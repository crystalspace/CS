/*
  Copyright (C) 2005 by Marten Svanfeldt
  
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

#ifndef __CS_MATH_H__
#define __CS_MATH_H__

#include "csutil/algorithms.h"

/**\file 
 * Generic mathematic utility functions.
 */
/**
 * \addtogroup geom_utils
 * @{ */

/**
 * Returns bigger of a and b. If they are equal, a or b can be returned.
 */
template<class T>
const T& csMax (const T& a, const T& b)
{
  if (b < a) return a;
  return b;
}

/**
 * Returns smaller of a and b. If they are equal, a or b can be returned.
 */
template<class T>
const T& csMin (const T& a, const T& b)
{
  if (a < b) return a;
  return b;
}

/**
 * Sort a and b in order of size.
 */
template<class T>
void csSort (T& a, T& b)
{
  if (b < a)
    CS::Swap (a, b);
}

/**
 * Sort a and b in order of size.
 * If swapping them, also swap x and y
 */
template<class T, class U>
void csSort (T& a, T& b, U& x, U& y)
{
  if (b < a)
  {
    CS::Swap (a, b);
    CS::Swap (x, y);
  }
}


/**
 * Clamp a between max and min.
 */
template<class T>
T csClamp (const T& a, T max, T min)
{
  return csMin (csMax (a, min), max);
}

/**
 * Performs a smooth interpolation of a on range min to max.
 * \return Smooth interporlated value if \a min \< \a a \< \a max, 
 *  and 0 resp. 1 if \a a is smaller than \a min resp. larger than \a max.
 */
template<class T>
T csSmoothStep (const T& a, T max, T min)
{
  T tmp, tmp2;
  if (a <= min)
    tmp = 0.0f;
  else if (a >= max)
    tmp = 1.0f;
  else
  {
    tmp2 = (a - min) / (max-min);
    tmp = tmp2*tmp2 * (3.0 - 2.0*tmp2);
  }
  return tmp;
}

/**
 * Performs a linear interpolation between \a a and \a b with the factor
 * \a f.
 */
template<class T, class Tfactor>
T csLerp (const T& a, const T& b, const Tfactor& f)
{
  return (a + (b - a) * f);
}

/**
 * Returns the square of the argument
 */
template<class T>
T csSquare (const T& x)
{
  return x * x;
}

namespace CS
{
  /** \name Floating point utilities
   * @{ */
  /* IsNaN() is implemented by looking at the binary values directly
     as using built-in functions turned out to be unreliable.
     (Specifically, gcc's built-in isnan() always returns false if
     -ffast-math is enabled.) */
  /// Checks if a floating point value is not-a-number.
  CS_FORCEINLINE bool IsNaN (float f)
  {
  #ifdef CS_IEEE_DOUBLE_FORMAT
    const uint32 exponentMask = 0x7f800000;
    const uint32 mantissaMask = 0x007fffff;
    union
    {
      float f;
      uint32 ui32;
    } u;
    u.f = f;
    return ((u.ui32 & exponentMask) == exponentMask)
	&& ((u.ui32 & mantissaMask) != 0);
  #else
    #error Do not know how to test for NaN
  #endif
  }
  // External definition; used below as a workaround for __STRICT_ANSI__
  CS_CRYSTALSPACE_EXPORT bool IsNaN_ext (double d);
  /// Checks if a double-precision floating point value is not-a-number.
  CS_FORCEINLINE bool IsNaN (double d)
  {
  #ifdef __STRICT_ANSI__
    return IsNaN_ext (d);
  #else
  #ifdef CS_IEEE_DOUBLE_FORMAT
    const uint64 exponentMask = CONST_UINT64(0x7ff0000000000000);
    const uint64 mantissaMask = CONST_UINT64(0x000fffffffffffff);
    union
    {
      double d;
      uint64 ui64;
    } u;
    u.d = d;
    return ((u.ui64 & exponentMask) == exponentMask)
	&& ((u.ui64 & mantissaMask) != 0);
  #else
    #error Do not know how to test for NaN
  #endif
  #endif
  }
  
  /// Checks if a floating point value is finite.
  CS_FORCEINLINE bool IsFinite (float f)
  {
  #ifdef CS_IEEE_DOUBLE_FORMAT
    const uint32 exponentMask = 0x7f800000;
    union
    {
      float f;
      uint32 ui32;
    } u;
    u.f = f;
    return ((u.ui32 & exponentMask) != exponentMask);
  #else
    #error Do not know how to test for NaN
  #endif
  }
  // External definition; used below as a workaround for __STRICT_ANSI__
  CS_CRYSTALSPACE_EXPORT bool IsFinite_ext (double d);
  /// Checks if a double-precision floating point value is finite.
  CS_FORCEINLINE bool IsFinite (double d)
  {
  #ifdef __STRICT_ANSI__
    return IsFinite_ext (d);
  #else
  #ifdef CS_IEEE_DOUBLE_FORMAT
    const uint64 exponentMask = CONST_UINT64(0x7ff0000000000000);
    union
    {
      double d;
      uint64 ui64;
    } u;
    u.d = d;
    return ((u.ui64 & exponentMask) != exponentMask);
  #else
    #error Do not know how to test for NaN
  #endif
  #endif
  }
  /** @} */
} // namespace CS

//@{
/**
 * Checks if a floating point value is finite.
 * \deprecated Deprecated in 1.9. Use CS::IsFinite() instead.
 */
CS_DEPRECATED_METHOD_MSG("Use CS::IsFinite(x) instead")
CS_FORCEINLINE bool csFinite (float f)
{
  return CS::IsFinite (f);
}
/**
 * Checks if a double-precision floating point value is finite.
 * \deprecated Deprecated in 1.9. Use CS::IsFinite() instead.
 */
CS_DEPRECATED_METHOD_MSG("Use CS::IsFinite(x) instead")
CS_FORCEINLINE bool csFinite (double d)
{
  return CS::IsFinite (d);
}

/**
 * Checks if a floating point value is not-a-number.
 * \deprecated Deprecated in 1.9. Use CS::IsNan() instead.
 */
CS_DEPRECATED_METHOD_MSG("Use CS::IsNaN(x) instead")
CS_FORCEINLINE bool csNaN (float f)
{
  return CS::IsNaN (f);
}
/**
 * Checks if a double-precision floating point value is not-a-number.
 * \deprecated Deprecated in 1.9. Use CS::IsNaN() instead.
 */
CS_DEPRECATED_METHOD_MSG("Use CS::IsNaN(x) instead")
CS_FORCEINLINE bool csNaN (double d)
{
  return CS::IsNaN (d);
}

/**
 * Checks if a floating point value is normal (not infinite or nan).
 * \deprecated Deprecated in 1.9. Usage not recommended as results are
 *   inconsistent across platforms.
 */
CS_DEPRECATED_METHOD_MSG("Usage not recommended, inconsistent results")
CS_FORCEINLINE bool csNormal (float f)
{
#if defined (CS_HAVE_NORMALF)
  return normalf (f);
#elif defined (CS_HAVE_STD__ISNORMAL)
  return std::isnormal (f);
#elif defined(CS_HAVE_ISNORMAL)
  return isnormal (f);
#else
  return CS::IsFinite(f) && !CS::IsNaN(f);
#endif
}
/**
 * Checks if a double-precision floating point value is normal.
 * \deprecated Deprecated in 1.9. Usage not recommended as results are
 *   inconsistent across platforms.
 */
CS_DEPRECATED_METHOD_MSG("Usage not recommended, inconsistent results")
CS_FORCEINLINE bool csNormal (double d)
{
#if defined (CS_HAVE_STD__ISNORMAL)
  return std::isnormal (d);
#elif defined(CS_HAVE_ISNORMAL)
  return isnormal (d);
#else
  return CS::IsFinite(d) && !CS::IsNaN(d);
#endif
}
//@}


namespace CS
{
namespace Math
{
  /// Return the next number in the sequence 0,1,2 modulo 3
  template<typename T>
  CS_FORCEINLINE_TEMPLATEMETHOD T NextModulo3 (T number)
  {
    return (1 << number) & 0x3;
  }
}
}


/** @} */

#endif //__CS_MATH_H__
