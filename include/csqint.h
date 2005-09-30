/*
    Copyright (C) 2005 by Eric Sunshine
  
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

#ifndef __CS_CSQINT_H__
#define __CS_CSQINT_H__

/**
 * \addtogroup floating_point
 * @{ */

/** \file
    Quick floating point to integer conversions.
*/

/**
 * Quickly truncate the fractional part of a floating-point value and convert
 * it to a long integer using processor and/or number format quirks if
 * available.
 */
template<typename T>
static inline long csQint(T n) { return (long)n; }

/**
 * Quickly round a floating-point value and convert it to a long integer using
 * processor and/or number format quirks if available.
 */
template<typename T>
static inline long csQround (T n)
{ return (long)(n + (n < 0 ? (T)-0.5 : (T)0.5)); }

/**\fn static inline long csQfixed8 (float)
 * Quickly convert a floating-point number to 24.8 fixed-point value.
 */
static inline long csQfixed8 (float n) { return (int)(n * 256.0f); }

/**\fn static inline long csQfixed16 (float)
 * Quickly convert a floating-point number to 16.16 fixed-point value.
 */
static inline long csQfixed16 (float n) { return (int)(n * 65536.0f); }

/**\fn static inline long csQfixed24 (float)
 * Quickly convert a floating-point number to 8.24 fixed-point value.
 */
static inline long csQfixed24 (float n) { return (int)(n * 16777216.0f); }

/** @} */
    
// Backward compatibility. Use csQfixedn() instead.
CS_DEPRECATED_METHOD
static inline long csQint8 (float n) { return csQfixed8 (n); }
CS_DEPRECATED_METHOD
static inline long csQint16(float n) { return csQfixed16(n); }
CS_DEPRECATED_METHOD
static inline long csQint24(float n) { return csQfixed24(n); }

#endif // __CS_CSQINT_H__
