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

#ifndef VECTOR2_H
#define VECTOR2_H

/**
 * A 2D vector.
 */
class csVector2
{
public:
  ///
  float x;
  ///
  float y;

  /// Make a new vector. No initialization is done.
  csVector2 () {}

  /// Make a new vector and initialize with the given values.
  csVector2 (float x, float y) { csVector2::x = x; csVector2::y = y; }

  /// Set vector to given values.
  inline void Set (float ix, float iy)
  { x = ix; y = iy; }

  /// Set vector to given values.
  inline void Set (const csVector2& v)
  { x = v.x; y = v.y; }

  /// Return the norm (magnitude) of a 2D vector.
  static float Norm (const csVector2& v);

  /// Return the norm (magnitude) of this vector.
  float Norm () const;

  /// Return the squared norm (magnitude) of this vector.
  float SquaredNorm () const
  { return x * x + y * y; }

  /// Rotate vector around the origin by a given angle in radians.
  void Rotate (float angle);

  /// Add another vector to this vector.
  csVector2& operator+= (const csVector2& v)
  { x += v.x;  y += v.y;  return *this; }

  /// Subtract another vector from this vector.
  csVector2& operator-= (const csVector2& v)
  { x -= v.x;  y -= v.y;  return *this; }

  /// Multiply this vector by a scalar.
  csVector2& operator*= (float f) { x *= f;  y *= f;  return *this; }

  /// Divide this vector by a scalar.
  csVector2& operator/= (float f) { x /= f;  y /= f;  return *this; }

  /// Unary + operator.
  inline csVector2 operator+ () const { return *this; }

  /// Unary - operator.
  inline csVector2 operator- () const { return csVector2(-x,-y); }

  /// Add two vectors.
  friend csVector2 operator+ (const csVector2& v1, const csVector2& v2);
  /// Subtract two vectors.
  friend csVector2 operator- (const csVector2& v1, const csVector2& v2);
  /// Take the dot product of two vectors.
  friend float operator* (const csVector2& v1, const csVector2& v2);
  /// Multiply a vector and a scalar.
  friend csVector2 operator* (const csVector2& v, float f);
  /// Multiply a vector and a scalar.
  friend csVector2 operator* (float f, const csVector2& v);
  /// Divide a vector by a scalar.
  friend csVector2 operator/ (const csVector2& v, float f);
  /// Check if two vectors are equal.
  friend bool operator== (const csVector2& v1, const csVector2& v2);
  /// Check if two vectors are not equal.
  friend bool operator!= (const csVector2& v1, const csVector2& v2);

  /// Test if each component of a vector is less than a small epsilon value.
  inline friend bool operator< (const csVector2& v, float f)
  { return ABS(v.x)<f && ABS(v.y)<f; }

  /// Test if each component of a vector is greater than a small epsilon value.
  inline friend bool operator> (float f, const csVector2& v)
  { return ABS(v.x)<f && ABS(v.y)<f; }
};

#endif /*VECTOR2_H*/
