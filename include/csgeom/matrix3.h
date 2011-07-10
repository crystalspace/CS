/*
    Copyright (C) 1998,1999,2000 by Jorrit Tyberghein
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

#ifndef __CS_MATRIX3_H__
#define __CS_MATRIX3_H__

/**\file
 * 3x3 matrix.
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"
#include "csgeom/vector3.h"

class csQuaternion;

/**
 * A 3x3 matrix.
 */
class CS_CRYSTALSPACE_EXPORT csMatrix3
{
public:
  float m11, m12, m13;
  float m21, m22, m23;
  float m31, m32, m33;

public:
  /// Construct a matrix, initialized to be the identity.
  csMatrix3 ()
    : m11(1), m12(0), m13(0),
      m21(0), m22(1), m23(0),
      m31(0), m32(0), m33(1)
  {}

  /// Construct a matrix and initialize it.
  csMatrix3 (float am11, float am12, float am13,
             float am21, float am22, float am23,
             float am31, float am32, float am33)
    : m11(am11), m12(am12), m13(am13),
      m21(am21), m22(am22), m23(am23),
      m31(am31), m32(am32), m33(am33)
  {}

  /// Copy constructor.
  csMatrix3 (csMatrix3 const& o)
    : m11(o.m11), m12(o.m12), m13(o.m13),
      m21(o.m21), m22(o.m22), m23(o.m23),
      m31(o.m31), m32(o.m32), m33(o.m33)
  {}

  /// Construct a matrix from axis-angle specifier.
  csMatrix3 (float x,float y, float z, float angle);

  /// Construct a matrix with a quaternion.
  explicit csMatrix3 (const csQuaternion &quat)
  { Set (quat); }

  /// Return a textual representation of the matrix
  csString Description() const;

  /// Get the first row of this matrix as a vector.
  inline csVector3 Row1() const { return csVector3 (m11, m12, m13); }
  void SetRow1 (const csVector3& r) { m11 = r.x; m12 = r.y; m13 = r.z; }

  /// Get the second row of this matrix as a vector.
  inline csVector3 Row2() const { return csVector3 (m21, m22, m23); }
  void SetRow2 (const csVector3& r) { m21 = r.x; m22 = r.y; m23 = r.z; }

  /// Get the third row of this matrix as a vector.
  inline csVector3 Row3() const { return csVector3 (m31, m32, m33); }
  void SetRow3 (const csVector3& r) { m31 = r.x; m32 = r.y; m33 = r.z; }

  /// Get a row from this matrix as a vector.
  inline csVector3 Row(size_t n) const
  {
    return !n ? csVector3 (m11, m12, m13) :
      n&1 ? csVector3 (m21, m22, m23) :
      csVector3 (m31, m32, m33);
  }
  void SetRow (size_t n, const csVector3& r)
  {
    if (n == 0) SetRow1 (r);
    else if (n == 1) SetRow2 (r);
    else SetRow3 (r);
  }

  /// Get the first column of this matrix as a vector.
  inline csVector3 Col1() const { return csVector3 (m11, m21, m31); }
  void SetCol1 (const csVector3& c) { m11 = c.x; m21 = c.y; m31 = c.z; }

  /// Get the second column of this matrix as a vector.
  inline csVector3 Col2() const { return csVector3 (m12, m22, m32); }
  void SetCol2 (const csVector3& c) { m12 = c.x; m22 = c.y; m32 = c.z; }

  /// Get the third column of this matrix as a vector.
  inline csVector3 Col3() const { return csVector3 (m13, m23, m33); }
  void SetCol3 (const csVector3& c) { m13 = c.x; m23 = c.y; m33 = c.z; }

  /// Get a column from this matrix as a vector.
  inline csVector3 Col(size_t n) const
  {
    return !n ? csVector3 (m11, m21, m31) :
      n&1 ? csVector3 (m12, m22, m32) :
      csVector3 (m13, m23, m33);
  }
  void SetCol (size_t n, const csVector3& c)
  {
    if (n == 0) SetCol1 (c);
    else if (n == 1) SetCol2 (c);
    else SetCol3 (c);
  }

  /// Set matrix values.
  inline void Set (float o11, float o12, float o13,
                   float o21, float o22, float o23,
                   float o31, float o32, float o33)
  {
    m11 = o11; m12 = o12; m13 = o13;
    m21 = o21; m22 = o22; m23 = o23;
    m31 = o31; m32 = o32; m33 = o33;
  }

  /// Set matrix values.
  inline void Set (csMatrix3 const &o)
  {
    m11 = o.m11; m12 = o.m12; m13 = o.m13;
    m21 = o.m21; m22 = o.m22; m23 = o.m23;
    m31 = o.m31; m32 = o.m32; m33 = o.m33;
  }

  /// Initialize matrix with a quaternion.
  void Set (const csQuaternion&);

  /// Assign another matrix to this one.
  inline csMatrix3& operator= (const csMatrix3& o)
  { Set(o); return *this; }

  /// Add another matrix to this matrix.
  inline csMatrix3& operator+= (const csMatrix3& m)
  {
    m11 += m.m11; m12 += m.m12; m13 += m.m13;
    m21 += m.m21; m22 += m.m22; m23 += m.m23;
    m31 += m.m31; m32 += m.m32; m33 += m.m33;
    return *this;
  }

  /// Subtract another matrix from this matrix.
  inline csMatrix3& operator-= (const csMatrix3& m)
  {
    m11 -= m.m11; m12 -= m.m12; m13 -= m.m13;
    m21 -= m.m21; m22 -= m.m22; m23 -= m.m23;
    m31 -= m.m31; m32 -= m.m32; m33 -= m.m33;
    return *this;
  }

  /// Multiply another matrix with this matrix.
  inline csMatrix3& operator*= (const csMatrix3& m)
  {
    float old_m11 = m11;
    m11 = m11 * m.m11 + m12 * m.m21 + m13 * m.m31;

    float old_m12 = m12;
    m12 = old_m11 * m.m12 + m12 * m.m22 + m13 * m.m32;
    m13 = old_m11 * m.m13 + old_m12 * m.m23 + m13 * m.m33;

    float old_m21 = m21;
    m21 = m21 * m.m11 + m22 * m.m21 + m23 * m.m31;

    float old_m22 = m22;
    m22 = old_m21 * m.m12 + m22 * m.m22 + m23 * m.m32;
    m23 = old_m21 * m.m13 + old_m22 * m.m23 + m23 * m.m33;

    float old_m31 = m31;
    m31 = m31 * m.m11 + m32 * m.m21 + m33 * m.m31;

    float old_m32 = m32;
    m32 = old_m31 * m.m12 + m32 * m.m22 + m33 * m.m32;
    m33 = old_m31 * m.m13 + old_m32 * m.m23 + m33 * m.m33;
    return *this;
  }

  /// Multiply this matrix with a scalar.
  inline csMatrix3& operator*= (float s)
  {
    m11 *= s; m12 *= s; m13 *= s;
    m21 *= s; m22 *= s; m23 *= s;
    m31 *= s; m32 *= s; m33 *= s;
    return *this;
  }

  /// Divide this matrix by a scalar.
  inline csMatrix3& operator/= (float s)
  {
    s = 1.0f/s;
    m11 *= s; m12 *= s; m13 *= s;
    m21 *= s; m22 *= s; m23 *= s;
    m31 *= s; m32 *= s; m33 *= s;
    return *this;
  }

  /// Unary + operator.
  inline csMatrix3 operator+ () const
  { return *this; }

  /// Unary - operator.
  inline csMatrix3 operator- () const
  {
    return csMatrix3(-m11,-m12,-m13,
                     -m21,-m22,-m23,
                     -m31,-m32,-m33);
  }

  /// Transpose this matrix.
  inline void Transpose ()
  {
    float swap;
    swap = m12; m12 = m21; m21 = swap;
    swap = m13; m13 = m31; m31 = swap;
    swap = m23; m23 = m32; m32 = swap;
  }

  /// Return the transpose of this matrix.
  csMatrix3 GetTranspose () const
  {
    return csMatrix3 (
      m11, m21, m31,
      m12, m22, m32,
      m13, m23, m33);
  }

  /// Return the inverse of this matrix.
  inline csMatrix3 GetInverse () const
  {
    csMatrix3 C(
             (m22*m33 - m23*m32), -(m12*m33 - m13*m32),  (m12*m23 - m13*m22),
            -(m21*m33 - m23*m31),  (m11*m33 - m13*m31), -(m11*m23 - m13*m21),
             (m21*m32 - m22*m31), -(m11*m32 - m12*m31),  (m11*m22 - m12*m21));
    float s = (float)1./(m11*C.m11 + m12*C.m21 + m13*C.m31);
    C *= s;
    return C;
  }

  /// Invert this matrix.
  inline void Invert()
  { *this = GetInverse (); }

  /// Compute the determinant of this matrix.
  float Determinant () const
  {
    return m11 * (m22 * m33 - m23 * m32)
         - m12 * (m21 * m33 - m23 * m31)
         + m13 * (m21 * m32 - m22 * m31);
  }

  /// Set this matrix to the identity matrix.
  inline void Identity ()
  {
    m11 = m22 = m33 = 1.0;
    m12 = m13 = m21 = m23 = m31 = m32 = 0.0;
  }

  /// Check if the matrix is identity
  inline bool IsIdentity () const
  {
    return (m11 == 1.0) && (m12 == 0.0) && (m13 == 0.0) &&
           (m21 == 0.0) && (m22 == 1.0) && (m23 == 0.0) &&
           (m31 == 0.0) && (m32 == 0.0) && (m33 == 1.0);
  }

  /// Add two matricies.
  inline friend csMatrix3 operator+ (const csMatrix3& m1, const csMatrix3& m2)
  {
    return csMatrix3 (
      m1.m11 + m2.m11, m1.m12 + m2.m12, m1.m13 + m2.m13,
      m1.m21 + m2.m21, m1.m22 + m2.m22, m1.m23 + m2.m23,
      m1.m31 + m2.m31, m1.m32 + m2.m32, m1.m33 + m2.m33);
  }

  /// Subtract two matricies.
  inline friend csMatrix3 operator- (const csMatrix3& m1, const csMatrix3& m2)
  {
    return csMatrix3 (
      m1.m11 - m2.m11, m1.m12 - m2.m12, m1.m13 - m2.m13,
      m1.m21 - m2.m21, m1.m22 - m2.m22, m1.m23 - m2.m23,
      m1.m31 - m2.m31, m1.m32 - m2.m32, m1.m33 - m2.m33);
  }

  /// Multiply two matricies.
  inline friend csMatrix3 operator* (const csMatrix3& m1, const csMatrix3& m2)
  {
    return csMatrix3 (
      m1.m11 * m2.m11 + m1.m12 * m2.m21 + m1.m13 * m2.m31,
      m1.m11 * m2.m12 + m1.m12 * m2.m22 + m1.m13 * m2.m32,
      m1.m11 * m2.m13 + m1.m12 * m2.m23 + m1.m13 * m2.m33,
      m1.m21 * m2.m11 + m1.m22 * m2.m21 + m1.m23 * m2.m31,
      m1.m21 * m2.m12 + m1.m22 * m2.m22 + m1.m23 * m2.m32,
      m1.m21 * m2.m13 + m1.m22 * m2.m23 + m1.m23 * m2.m33,
      m1.m31 * m2.m11 + m1.m32 * m2.m21 + m1.m33 * m2.m31,
      m1.m31 * m2.m12 + m1.m32 * m2.m22 + m1.m33 * m2.m32,
      m1.m31 * m2.m13 + m1.m32 * m2.m23 + m1.m33 * m2.m33);
  }

  /// Multiply a vector by a matrix (transform it).
  inline friend csVector3 operator* (const csMatrix3& m, const csVector3& v)
  {
    return csVector3 (m.m11*v.x + m.m12*v.y + m.m13*v.z,
                      m.m21*v.x + m.m22*v.y + m.m23*v.z,
                      m.m31*v.x + m.m32*v.y + m.m33*v.z);
  }

  /// Multiply a matrix and a scalar.
  inline friend csMatrix3 operator* (const csMatrix3& m, float f)
  {
    return csMatrix3 (
      m.m11 * f, m.m12 * f, m.m13 * f,
      m.m21 * f, m.m22 * f, m.m23 * f,
      m.m31 * f, m.m32 * f, m.m33 * f);
  }

  /// Multiply a matrix and a scalar.
  inline friend csMatrix3 operator* (float f, const csMatrix3& m)
  {
    return csMatrix3 (
      m.m11 * f, m.m12 * f, m.m13 * f,
      m.m21 * f, m.m22 * f, m.m23 * f,
      m.m31 * f, m.m32 * f, m.m33 * f);
  }

  /// Divide a matrix by a scalar.
  inline friend csMatrix3 operator/ (const csMatrix3& m, float f)
  {
    float inv_f = 1 / f;
    return csMatrix3 (
      m.m11 * inv_f, m.m12 * inv_f, m.m13 * inv_f,
      m.m21 * inv_f, m.m22 * inv_f, m.m23 * inv_f,
      m.m31 * inv_f, m.m32 * inv_f, m.m33 * inv_f);
  }

  /// Check if two matricies are equal.
  inline friend bool operator== (const csMatrix3& m1, const csMatrix3& m2)
  {
    if (m1.m11 != m2.m11 || m1.m12 != m2.m12 || m1.m13 != m2.m13)
      return false;
    if (m1.m21 != m2.m21 || m1.m22 != m2.m22 || m1.m23 != m2.m23)
      return false;
    if (m1.m31 != m2.m31 || m1.m32 != m2.m32 || m1.m33 != m2.m33)
      return false;
    return true;
  }

  /// Check if two matricies are not equal.
  inline friend bool operator!= (const csMatrix3& m1, const csMatrix3& m2)
  {
    if (m1.m11 != m2.m11 || m1.m12 != m2.m12 || m1.m13 != m2.m13) return true;
    if (m1.m21 != m2.m21 || m1.m22 != m2.m22 || m1.m23 != m2.m23) return true;
    if (m1.m31 != m2.m31 || m1.m32 != m2.m32 || m1.m33 != m2.m33) return true;
    return false;
  }

  /// Test if each component of a matrix is less than a small epsilon value.
  inline friend bool operator< (const csMatrix3& m, float f)
  {
    return fabsf (m.m11) < f && fabsf (m.m12) < f && fabsf (m.m13) < f &&
           fabsf (m.m21) < f && fabsf (m.m22) < f && fabsf (m.m23) < f &&
           fabsf (m.m31) < f && fabsf (m.m32) < f && fabsf (m.m33) < f;
  }

  /// Test if each component of a matrix is greater than a small epsilon value.
  inline friend bool operator> (float f, const csMatrix3& m)
  {
    return !(m < f);
  }
};

/// An instance of csMatrix3 that is initialized as a rotation about X
class CS_CRYSTALSPACE_EXPORT csXRotMatrix3 : public csMatrix3
{
public:
  /**
   * Return a rotation matrix around the X axis.  'angle' is given in radians.
   * Looking along the X axis with Y pointing to the right and Z pointing up a
   * rotation of PI/2 will rotate 90 degrees in anti-clockwise direction (i.e.
   * 0,1,0 -> 0,0,1).
   */
  csXRotMatrix3 (float angle);
};

/// An instance of csMatrix3 that is initialized as a rotation about Y.
class CS_CRYSTALSPACE_EXPORT csYRotMatrix3 : public csMatrix3
{
public:
  /**
   * Return a rotation matrix around the Y axis.  'angle' is given in radians.
   * Looking along the Y axis with X pointing to the right and Z pointing up a
   * rotation of PI/2 will rotate 90 degrees in anti-clockwise direction (i.e.
   * 1,0,0 -> 0,0,1).
   */
  csYRotMatrix3 (float angle);
};

/// An instance of csMatrix3 that is initialized as a rotation about Z.
class CS_CRYSTALSPACE_EXPORT csZRotMatrix3 : public csMatrix3
{
public:
  /**
   * Return a rotation matrix around the Z axis.  'angle' is given in radians.
   * Looking along the Z axis with X pointing to the right and Y pointing up a
   * rotation of PI/2 will rotate 90 degrees in anti-clockwise direction (i.e.
   * 1,0,0 -> 0,1,0).
   */
  csZRotMatrix3 (float angle);
};

/// An instance of csMatrix3 that is initialized to scale the X dimension.
class CS_CRYSTALSPACE_EXPORT csXScaleMatrix3 : public csMatrix3
{
public:
  /**
   * Return a matrix which scales in the X dimension.
   */
  csXScaleMatrix3 (float scaler)
    : csMatrix3(scaler, 0, 0, 0, 1, 0, 0, 0, 1)
  {}
};

/// An instance of csMatrix3 that is initialized to scale the Y dimension.
class CS_CRYSTALSPACE_EXPORT csYScaleMatrix3 : public csMatrix3
{
public:
  /**
   * Return a matrix which scales in the Y dimension.
   */
  csYScaleMatrix3 (float scaler)
    : csMatrix3(1, 0, 0, 0, scaler, 0, 0, 0, 1)
  {}
};

/// An instance of csMatrix3 that is initialized to scale the Z dimension.
class CS_CRYSTALSPACE_EXPORT csZScaleMatrix3 : public csMatrix3
{
public:
  /**
   * Return a matrix which scales in the Z dimension.
   */
  csZScaleMatrix3 (float scaler)
    : csMatrix3(1, 0, 0, 0, 1, 0, 0, 0, scaler)
  {}
};


/** @} */

#endif // __CS_MATRIX3_H__
