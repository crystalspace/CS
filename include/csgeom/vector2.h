/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Largely rewritten by Ivan Avramovic <ivan@avramovic.com>

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

#ifndef __CS_VECTOR2_H__
#define __CS_VECTOR2_H__

#include "csextern.h"

/**\file
 * 2D vector.
 */
/**
 * \addtogroup geom_utils
 * @{ */

class csString;

/**
 * A 2D vector.
 */
class CS_CRYSTALSPACE_EXPORT csVector2
{
public:
  /// X component of vector.
  float x;
  /// Y component of vector.
  float y;

  /// Make a new vector. No initialization is done.
  csVector2 () {}

  /// Make a new vector and initialize with the given values.
  csVector2 (float v) 
    : x (v), y (v)
  {}

  /// Make a new vector and initialize with the given values.
  csVector2 (float x, float y) 
    : x (x), y (y) 
  { }

  /// Copyconstructor
  csVector2 (const csVector2& o) 
    : x (o.x), y (o.y)
  {}

  /// Return a textual representation of the vector in the form "x,y".
  csString Description() const;

  /// Set vector to given values.
  inline void Set (float ix, float iy)
  { x = ix; y = iy; }

  /// Set vector to given values.
  inline void Set (csVector2 const& v)
  { x = v.x; y = v.y; }

  /// Set the value of this vector.
  inline void Set (float const* v) { x = v[0]; y = v[1]; }

  /// Set the value of this vector so that all components are the same.
  inline void Set (float v) { x = y = v; }

  /// Get the value of this vector.
  inline void Get (float* v) { v[0] = x; v[1] = y; }

  /// Return the norm (magnitude) of a 2D vector.
  static float Norm (csVector2 const& v);

  /// Return the norm (magnitude) of this vector.
  float Norm () const;

  /// Return the squared norm (magnitude) of this vector.
  inline float SquaredNorm () const
  { return x * x + y * y; }

  /// Rotate vector around the origin by a given angle in radians.
  void Rotate (float angle);

  /**
   * Test if this point is left of the line through p0 and p1.
   * \return >0 if this point is left, 0 if on the line and <0 if right.
   */
  inline float IsLeft (const csVector2& p0, const csVector2& p1)
  {
    return (p1.x - p0.x)*(y - p0.y) - (x - p0.x)*(p1.y - p0.y);
  }

  /// Add another vector to this vector.
  csVector2& operator+= (const csVector2& v)
  { x += v.x;  y += v.y;  return *this; }

  /// Subtract another vector from this vector.
  csVector2& operator-= (const csVector2& v)
  { x -= v.x;  y -= v.y;  return *this; }

  /// Multiply this vector by a scalar.
  csVector2& operator*= (float f) { x *= f;  y *= f;  return *this; }

  /// Divide this vector by a scalar.
  csVector2& operator/= (float f)
  {
    f = 1.0f / f;
    x *= f;
    y *= f;
    return *this;
  }

  /// Unary + operator.
  inline csVector2 operator+ () const { return *this; }

  /// Unary - operator.
  inline csVector2 operator- () const { return csVector2(-x,-y); }

  /// Add two vectors.
  friend CS_CRYSTALSPACE_EXPORT csVector2 operator+ (const csVector2& v1, 
    const csVector2& v2);
  /// Subtract two vectors.
  friend CS_CRYSTALSPACE_EXPORT csVector2 operator- (const csVector2& v1, 
    const csVector2& v2);
  /// Take the dot product of two vectors.
  friend CS_CRYSTALSPACE_EXPORT float operator* (const csVector2& v1, 
    const csVector2& v2);
  /// Multiply a vector and a scalar.
  friend CS_CRYSTALSPACE_EXPORT csVector2 operator* (const csVector2& v,
  	float f);
  /// Multiply a vector and a scalar.
  friend CS_CRYSTALSPACE_EXPORT csVector2 operator* (float f,
  	const csVector2& v);
  /// Divide a vector by a scalar.
  friend CS_CRYSTALSPACE_EXPORT csVector2 operator/ (const csVector2& v,
  	float f);
  /// Check if two vectors are equal.
  friend CS_CRYSTALSPACE_EXPORT bool operator== (const csVector2& v1, 
    const csVector2& v2);
  /// Check if two vectors are not equal.
  friend CS_CRYSTALSPACE_EXPORT bool operator!= (const csVector2& v1, 
    const csVector2& v2);

  /// Test if each component of a vector is less than a small epsilon value.
  inline friend bool operator< (const csVector2& v, float f)
  { return ABS(v.x)<f && ABS(v.y)<f; }

  /// Test if each component of a vector is greater than a small epsilon value.
  inline friend bool operator> (float f, const csVector2& v)
  { return ABS(v.x)<f && ABS(v.y)<f; }


  /// Returns n-th component of the vector.
  inline float operator[] (int n) const { return !n?x:y; }
  /// Returns n-th component of the vector.
  inline float & operator[] (int n) { return !n?x:y; }
};

/** @} */

#endif // __CS_VECTOR2_H__
