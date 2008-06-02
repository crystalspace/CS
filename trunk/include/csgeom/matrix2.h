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

#ifndef __CS_MATRIX2_H__
#define __CS_MATRIX2_H__

#include "csgeom/vector2.h"

/**\file 
 * 2x2 matrix.
 */
/**
 * \addtogroup geom_utils
 * @{ */

/**
 * A 2x2 matrix.
 */
class csMatrix2
{
public:
  float m11, m12;
  float m21, m22;

public:
  /// Construct a matrix, initialized to be the identity.
  csMatrix2 ()
    : m11 (1), m12 (0), m21 (0), m22 (1)
  {}

  /// Construct a matrix and initialize it.
  csMatrix2 (float m11, float m12,
             float m21, float m22)
    : m11 (m11), m12 (m12), m21 (m21), m22 (m22)
  {}


  /// Get the first row of this matrix as a vector.
  inline csVector2 Row1() const 
  { return csVector2 (m11,m12); }

  /// Get the second row of this matrix as a vector.
  inline csVector2 Row2() const 
  { return csVector2 (m21,m22); }

  /// Get the first column of this matrix as a vector.
  inline csVector2 Col1() const 
  { return csVector2 (m11,m21); }

  /// Get the second column of this matrix as a vector.
  inline csVector2 Col2() const 
  { return csVector2 (m12,m22); }

  /// Set matrix values.
  inline void Set (float m11, float m12,
                   float m21, float m22)
  {
    csMatrix2::m11 = m11; csMatrix2::m12 = m12;
    csMatrix2::m21 = m21; csMatrix2::m22 = m22;
  }

  /// Add another matrix to this matrix.
  inline csMatrix2& operator+= (const csMatrix2& m)
  {
    m11 += m.m11; m12 += m.m12;
    m21 += m.m21; m22 += m.m22;
    return *this;
  }

  /// Subtract another matrix from this matrix.
  inline csMatrix2& operator-= (const csMatrix2& m)
  {
    m11 -= m.m11; m12 -= m.m12;
    m21 -= m.m21; m22 -= m.m22;
    return *this;
  }

  /// Multiply another matrix with this matrix.
  inline csMatrix2& operator*= (const csMatrix2& m)
  {
    csMatrix2 r (*this);
    m11 = r.m11 * m.m11 + r.m12 * m.m21;
    m12 = r.m11 * m.m12 + r.m12 * m.m22;
    m21 = r.m21 * m.m11 + r.m22 * m.m21;
    m22 = r.m21 * m.m12 + r.m22 * m.m22;
    return *this;
  }

  /// Multiply this matrix with a scalar.
  inline csMatrix2& operator*= (float s)
  {
    m11 *= s; m12 *= s;
    m21 *= s; m22 *= s;
    return *this;
  }

  /// Divide this matrix by a scalar.
  inline csMatrix2& operator/= (float s)
  {
    s=1.0f/s;
    m11 *= s; m12 *= s;
    m21 *= s; m22 *= s;
    return *this;
  }

  /// Unary + operator.
  inline csMatrix2 operator+ () const 
  { return *this; }

  /// Unary - operator.
  inline csMatrix2 operator- () const
  {
    return csMatrix2 (-m11,-m12, -m21,-m22);
  }

  /// Transpose this matrix.
  inline void Transpose ()
  {
    float swap = m12;
    m12 = m21;
    m21 = swap;
  }

  /// Return the transpose of this matrix.
  inline csMatrix2 GetTranspose () const
  {
    return csMatrix2 (m11, m21, m12, m22);
  }

  /// Return the inverse of this matrix.
  inline csMatrix2 GetInverse () const
  {
    float inv_det = 1 / (m11 * m22 - m12 * m21);
    return csMatrix2 (m22 * inv_det, -m12 * inv_det,
    		      -m21 * inv_det, m11 * inv_det);
  }

  /// Invert this matrix.
  inline void Invert () 
  { *this = GetInverse (); }

  /// Compute the determinant of this matrix.
  inline float Determinant () const
  { return m11 * m22 - m12 * m21; }

  /// Set this matrix to the identity matrix.
  inline void Identity ()
  {
    m11 = m22 = 1;
    m12 = m21 = 0;
  }

  /// Add two matricies.
  inline friend csMatrix2 operator+ (const csMatrix2& m1, const csMatrix2& m2)
  {
    return csMatrix2 (
      m1.m11 + m2.m11, m1.m12 + m2.m12,
      m1.m21 + m2.m21, m1.m22 + m2.m22);
  }

  /// Subtract two matricies.
  inline friend csMatrix2 operator- (const csMatrix2& m1, const csMatrix2& m2)
  {
    return csMatrix2 (
      m1.m11 - m2.m11, m1.m12 - m2.m12,
      m1.m21 - m2.m21, m1.m22 - m2.m22);
  }

  /// Multiply two matricies.
  inline friend csMatrix2 operator* (const csMatrix2& m1, const csMatrix2& m2)
  {
    return csMatrix2 (
      m1.m11 * m2.m11 + m1.m12 * m2.m21,
      m1.m11 * m2.m12 + m1.m12 * m2.m22,
      m1.m21 * m2.m11 + m1.m22 * m2.m21,
      m1.m21 * m2.m12 + m1.m22 * m2.m22);
  }

  /// Multiply a vector by a matrix (transform it).
  inline friend csVector2 operator* (const csMatrix2& m, const csVector2& v)
  { return csVector2 (m.m11*v.x + m.m12*v.y, m.m21*v.x + m.m22*v.y); }

  /// Multiply a matrix and a scalar.
  inline friend csMatrix2 operator* (const csMatrix2& m, float f)
  { return csMatrix2 (m.m11 * f, m.m12 * f, m.m21 * f, m.m22 * f); }

  /// Multiply a matrix and a scalar.
  inline friend csMatrix2 operator* (float f, const csMatrix2& m)
  { return csMatrix2 (m.m11 * f, m.m12 * f, m.m21 * f, m.m22 * f); }

  /// Divide a matrix by a scalar.
  inline friend csMatrix2 operator/ (const csMatrix2& m, float f)
  {
    float inv_f = 1 / f;
    return csMatrix2 (
      m.m11 * inv_f, m.m12 * inv_f,
      m.m21 * inv_f, m.m22 * inv_f);
  }
};

/** @} */

#endif // __CS_MATRIX2_H__

