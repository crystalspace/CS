/*
    Copyright (C) 2005 by Jorrit Tyberghein
		  2005 by Frank Richter

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

#ifndef __CS_CSGEOM_FIXED_H__
#define __CS_CSGEOM_FIXED_H__

#include "csqint.h"

/**\file 
 * Fixed-point number classes.
 */
/**
 * \addtogroup geom_utils
 * @{ */

/**
 * Encapsulation of a 16.16 fixed-point number.
 * \todo More operators.
 */
class csFixed16
{
  int32 v;
public:
  /// Assign floating point number
  csFixed16& operator= (float f)
  {
    v = csQfixed16 (f);
    return *this;
  }

  /// Subtract two fixed point numbers
  inline friend csFixed16 operator- (const csFixed16& v1, 
    const csFixed16& v2)
  { 
    csFixed16 v;
    v.v = v1.v - v2.v; 
    return v;
  }
  /// Subtract a fixed point number from a float, result is fixed
  inline friend csFixed16 operator- (float v1, 
    const csFixed16& v2)
  { 
    csFixed16 v;
    v.v = csQfixed16 (v1) - v2.v; 
    return v;
  }
  /// Subtract a float number from a fixed point, result is fixed
  inline friend csFixed16 operator- (const csFixed16& v1, 
    float v2)
  { 
    csFixed16 v;
    v.v = v1.v - csQfixed16 (v2); 
    return v;
  }
  
  /// Multiply a fixed point number with a float, result is fixed
  inline friend csFixed16 operator* (const csFixed16& v1, 
    float v2)
  { 
    csFixed16 v;
    v.v = (int32)(v1.v * v2);
    return v;
  }
  
  /// Add a fixed point number to another
  inline csFixed16& operator+= (const csFixed16& x)
  {
    v += x.v;
    return *this;
  }

  /// Get integer part
  inline operator int() const
  { return v >> 16; }

  /// Get "raw" fixed point number
  inline int32 GetFixed() const { return v; }

  /// Shift right
  inline friend csFixed16 operator>> (const csFixed16& v1, int n)
  {
    csFixed16 vn;
    vn.v = v1.v >> n;
    return vn;
  }
};

/**
 * Encapsulation of a 8.24 fixed-point number.
 * \todo More operators.
 */
class csFixed24
{
  int32 v;
public:
  /// Assign floating point number
  csFixed24& operator= (float f)
  {
    v = csQfixed24 (f);
    return *this;
  }

  /// Subtract two fixed point numbers
  inline friend csFixed24 operator- (const csFixed24& v1, 
    const csFixed24& v2)
  { 
    csFixed24 v;
    v.v = v1.v - v2.v; 
    return v;
  }
  /// Subtract a fixed point number from a float, result is fixed
  inline friend csFixed24 operator- (float v1, 
    const csFixed24& v2)
  { 
    csFixed24 v;
    v.v = csQfixed24 (v1) - v2.v; 
    return v;
  }
  /// Subtract a float number from a fixed point, result is fixed
  inline friend csFixed24 operator- (const csFixed24& v1, 
    float v2)
  { 
    csFixed24 v;
    v.v = v1.v - csQfixed24 (v2); 
    return v;
  }
  
  /// Multiply a fixed point number with a float, result is fixed
  inline friend csFixed24 operator* (const csFixed24& v1, 
    float v2)
  { 
    csFixed24 v;
    v.v = (int32)(v1.v * v2);
    return v;
  }

  /// Add a fixed point number to another
  inline csFixed24& operator+= (const csFixed24& x)
  {
    v += x.v;
    return *this;
  }

  /// Get integer part
  inline operator int() const
  { return v >> 16; }

  /// Get "raw" fixed point number
  inline int32 GetFixed() const { return v; }
};

/** @} */

#endif // __CS_CSGEOM_FIXED_H__

