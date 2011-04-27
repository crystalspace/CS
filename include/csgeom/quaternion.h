/*
    Copyright (C) 2000 by Norman Kramer
                  2006 by Marten Svanfeldt 

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

#ifndef __CS_QUATERNION_H__
#define __CS_QUATERNION_H__

/**\file 
 * Quaternions.
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"
#include "csqsqrt.h"

#include "csgeom/vector3.h"

class csMatrix3;

/**
 * Class for a quaternion.
 * A SE3 rotation represented as a normalized quaternion
 * \sa csDualQuaternion
 */
class CS_CRYSTALSPACE_EXPORT csQuaternion
{
public:
  // Constructors

  /// Initialize with identity
  csQuaternion ()
    : v (0.0f), w (1.0f)
  {}

  /// Initialize with given values. Does not normalize
  csQuaternion (float x, float y, float z, float w)
    : v (x, y, z), w (w)
  {}

  /// Construct from a vector and given w value
  csQuaternion (const csVector3& v, float w)
    : v (v), w (w)
  {}

  /// Copy-constructor
  csQuaternion (const csQuaternion& q)
    : v (q.v), w (q.w)
  {}

  /**
   * Set the components
   */
  inline void Set (float x, float y, float z, float w)
  {
    v.x = x;
    v.y = y; 
    v.z = z;
    this->w = w;
  }
  
  /// Set quaternion to identity rotation
  inline void SetIdentity () 
  {
    v.Set (0.0f); w = 1.0f;
  }

  /// Add two quaternions
  inline friend csQuaternion operator+ (const csQuaternion& q1, 
    const csQuaternion& q2)
  {
    return csQuaternion (q1.v+q2.v, q1.w+q2.w);
  }

  /// Add quaternion to this one
  inline csQuaternion& operator+= (const csQuaternion& q)
  {
    v += q.v; w += q.w;
    return *this;
  }

  /// Subtract two quaternions
  inline friend csQuaternion operator- (const csQuaternion& q1, 
    const csQuaternion& q2)
  {
    return csQuaternion (q1.v-q2.v, q1.w-q2.w);
  }

  /// Subtract quaternion from this one
  inline csQuaternion& operator-= (const csQuaternion& q)
  {
    v -= q.v; w -= q.w;
    return *this;
  }
  
  /// Get the negative quaternion (unary minus)
  inline friend csQuaternion operator- (const csQuaternion& q)
  {
    return csQuaternion (-q.v, -q.w);
  }

  /// Multiply two quaternions, Grassmann product
  inline friend csQuaternion operator* (const csQuaternion& q1,
    const csQuaternion& q2)
  {
    return csQuaternion (q1.v*q2.w + q1.w*q2.v + q1.v%q2.v, 
      q1.w*q2.w - q1.v*q2.v);
  }

  /// Multiply this quaternion by another
  inline csQuaternion& operator*= (const csQuaternion& q)
  {
    csVector3 newV = v*q.w + w*q.v + v%q.v;
    w = w*q.w - v*q.v;
    v = newV;
    return *this;
  }

  /// Multiply by scalar
  inline friend csQuaternion operator* (const csQuaternion& q, float f)
  {
    return csQuaternion (q.v*f, q.w*f);
  }

  /// Multiply by scalar
  inline friend csQuaternion operator* (float f, const csQuaternion& q)
  {
    return csQuaternion (q.v*f, q.w*f);
  }

  /// Multiply by scalar
  inline csQuaternion& operator*= (float f)
  {
    v *= f;
    w *= f;

    return *this;
  }

  /// Divide by scalar
  inline friend csQuaternion operator/ (const csQuaternion& q, float f)
  {
    float invF = 1.0f/f;
    return csQuaternion (q.v*invF, q.w*invF);
  }

  /// Divide by scalar
  inline friend csQuaternion operator/ (float f, const csQuaternion& q)
  {
    float invF = 1.0f/f;
    return csQuaternion (q.v*invF, q.w*invF);
  }

  /// Divide by scalar
  inline csQuaternion& operator/= (float f)
  {
    float invF = 1.0f/f;
    v *= invF;
    w *= invF;

    return *this;
  }

  /// Get the conjugate quaternion
  inline csQuaternion GetConjugate () const
  {
    return csQuaternion (-v, w);
  }

  /// Set this quaternion to its own conjugate
  inline void Conjugate () 
  {
    v = -v;
  }

  /// Return euclidian inner-product (dot)
  inline float Dot (const csQuaternion& q) const
  {
    return v*q.v + w*q.w;
  }

  /// Get the squared norm of this quaternion (equals dot with itself)
  inline float SquaredNorm () const
  {
    return Dot (*this);
  }

  /// Get the norm of this quaternion
  inline float Norm () const
  {
    return csQsqrt (SquaredNorm ());
  }

  /**
   * Return a unit-lenght version of this quaternion (also called sgn)
   * Attempting to normalize a zero-length quaternion will result in a divide by
   * zero error.  This is as it should be... fix the calling code.
   */
  inline csQuaternion Unit () const
  {
    return (*this) / Norm ();
  }

  /**
   * Rotate vector by quaternion.
   */
  inline csVector3 Rotate (const csVector3& src) const
  {
    csQuaternion p (src, 0);
    csQuaternion q = *this * p;
    q *= GetConjugate ();
    return q.v;
  }

  /**
   * Set a quaternion using axis-angle representation
   * \param axis
   * Rotation axis. Should be normalized before calling this function.
   * \param angle
   * Angle to rotate about axis (in radians)
   */
  inline void SetAxisAngle (const csVector3& axis, float angle)
  {
    v = axis * sinf (angle / 2.0f);
    w = cosf (angle / 2.0f);
  }

  /**
   * Get a quaternion as axis-angle representation
   * \param axis
   * Rotation axis.
   * \param angle
   * Angle to rotate about axis (in radians)
   */
  inline void GetAxisAngle (csVector3& axis, float& angle) const
  {
    angle = 2.0f * acosf (w);
    if (v.SquaredNorm () != 0)
      axis = v.Unit ();
    else
      axis.Set (1.0f, 0.0f, 0.0f);
  }

  /**
   * Set quaternion using Euler angles X, Y, Z, expressed in radians
   */
  void SetEulerAngles (const csVector3& angles);

  /**
   * Get quaternion as three Euler angles X, Y, Z, expressed in radians
   */
  csVector3 GetEulerAngles () const; 

  /**
   * Set quaternion using 3x3 rotation matrix
   */
  void SetMatrix (const csMatrix3& matrix);

  /**
   * Get quaternion as a 3x3 rotation matrix
   */
  csMatrix3 GetMatrix () const;

  /**
   * Interpolate this quaternion with another using normalized linear 
   * interpolation (nlerp) using given interpolation factor.
   */
  csQuaternion NLerp (const csQuaternion& q2, float t) const;

  /**
   * Interpolate this quaternion with another using spherical linear
   * interpolation (slerp) using given interpolation factor.
   */
  csQuaternion SLerp (const csQuaternion& q2, float t) const;

  /**
   * Get the logarithm of this quaternion
   */
  csQuaternion Log () const;

  /**
   * Get the exponential of this quaternion
   */
  csQuaternion Exp () const;

  /**
   * Interpolate this quaternion with another (q) using cubic linear
   * interpolation (squad) using given interpolation factor (t)
   * and tangents (t1 and t2)
   */
  csQuaternion Squad (const csQuaternion & t1, const csQuaternion & t2,
    const csQuaternion & q, float t) const;

  /// x, y and z components of the quaternion
  csVector3 v;

  /// w component of the quaternion
  float w;
};

/** @} */

#endif // __CS_QUATERNION_H__

