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
 * 16.16 fixed-point number.
 */
/**
 * \addtogroup geom_utils
 * @{ */

/**
 * Encapsulation of a 16.16 fixed-point number.
 * \todo More complete docs.
 * \todo More operators.
 */
class csFixed16
{
  int32 v;
public:
  csFixed16& operator= (float f)
  {
    v = csQfixed16 (f);
    return *this;
  }

  inline friend csFixed16 operator- (const csFixed16& v1, 
    const csFixed16& v2)
  { 
    csFixed16 v;
    v.v = v1.v - v2.v; 
    return v;
  }
  inline friend csFixed16 operator- (float v1, 
    const csFixed16& v2)
  { 
    csFixed16 v;
    v.v = csQfixed16 (v1) - v2.v; 
    return v;
  }
  inline friend csFixed16 operator- (const csFixed16& v1, 
    float v2)
  { 
    csFixed16 v;
    v.v = v1.v - csQfixed16 (v2); 
    return v;
  }
  inline friend csFixed16 operator* (const csFixed16& v1, 
    float v2)
  { 
    csFixed16 v;
    v.v = (int32)(v1.v * v2);
    return v;
  }
  inline csFixed16& operator+= (const csFixed16& x)
  {
    v += x.v;
    return *this;
  }
  inline operator int() const
  { return v >> 16; }
  inline int32 GetFixed() const { return v; }
  inline friend csFixed16 operator>> (const csFixed16& v1, int n)
  {
    csFixed16 vn;
    vn.v = v1.v >> n;
    return vn;
  }
};

/**
 * Encapsulation of a 8.24 fixed-point number.
 * \todo More complete docs.
 * \todo More operators.
 */
class csFixed24
{
  int32 v;
public:
  csFixed24& operator= (float f)
  {
    v = csQfixed24 (f);
    return *this;
  }

  inline friend csFixed24 operator- (const csFixed24& v1, 
    const csFixed24& v2)
  { 
    csFixed24 v;
    v.v = v1.v - v2.v; 
    return v;
  }
  inline friend csFixed24 operator- (float v1, 
    const csFixed24& v2)
  { 
    csFixed24 v;
    v.v = csQfixed24 (v1) - v2.v; 
    return v;
  }
  inline friend csFixed24 operator- (const csFixed24& v1, 
    float v2)
  { 
    csFixed24 v;
    v.v = v1.v - csQfixed24 (v2); 
    return v;
  }
  inline friend csFixed24 operator* (const csFixed24& v1, 
    float v2)
  { 
    csFixed24 v;
    v.v = (int32)(v1.v * v2);
    return v;
  }
  inline csFixed24& operator+= (const csFixed24& x)
  {
    v += x.v;
    return *this;
  }
  inline operator int() const
  { return v >> 16; }
  inline int32 GetFixed() const { return v; }
};

/** @} */

#endif // __CS_CSGEOM_FIXED_H__

