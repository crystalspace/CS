/*
    Copyright (C) 2008 by Marten Svanfeldt 

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

#ifndef __CS_CSGEOM_DUALQUATERNION_H__
#define __CS_CSGEOM_DUALQUATERNION_H__

/**\file 
 * Dual quaternions.
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"
#include "csqsqrt.h"

#include "csgeom/vector3.h"
#include "csgeom/quaternion.h"
#include "csutil/tuple.h"

class csMatrix3;

/// 
typedef csTuple2<float, float> csDualNumber;

/**
 * Dual quaternion is a combination
 * q = q0 + e*qe where e is the dual identity element (e^2 = 0)
 * For the background, read "Preliminary Sketch of Biquaternions" [W Clifford, 1873]
 * \sa csQuaternion
 */
class csDualQuaternion
{
public:
  // Constructors

  /// Initialize with identity
  inline csDualQuaternion ()
    : real (0,0,0,1), dual (0,0,0,0)
  {}

  /// Initialize with given values.
  inline csDualQuaternion (const csQuaternion& real, const csQuaternion& dual)
    : real (real), dual (dual)
  {}

  /// Construct from quaternion (pure rotation)
  inline csDualQuaternion (const csQuaternion& real)
    : real (real)
  {}

  /// Construct from quaternion and a vector
  inline csDualQuaternion (const csQuaternion& real, const csVector3& t)
    : real (real), dual (0.5 * (csQuaternion (t, 0)*real))
  {}

  /// Copy-constructor
  inline csDualQuaternion (const csDualQuaternion& q)
    : real (q.real), dual (q.dual)
  {}


  /// Set quaternion to identity rotation and no rotation
  inline void SetIdentity () 
  {
    real.SetIdentity ();
    dual.Set (0, 0, 0, 0);
  }

  /// Add two dual quaternions
  inline friend csDualQuaternion operator+ (const csDualQuaternion& q1, 
    const csDualQuaternion& q2)
  {
    return csDualQuaternion (q1.real + q2.real, q1.dual + q2.dual);
  }

  /// Add dual quaternion to this one
  inline csDualQuaternion& operator+= (const csDualQuaternion& q)
  {
    real += q.real;
    dual += q.dual;
    return *this;
  }

  /// Subtract two dual quaternions
  inline friend csDualQuaternion operator- (const csDualQuaternion& q1, 
    const csDualQuaternion& q2)
  {
    return csDualQuaternion (q1.real - q2.real, q1.dual - q2.dual);
  }

  /// Subtract dual quaternion from this one
  inline csDualQuaternion& operator-= (const csDualQuaternion& q)
  {
    real -= q.real;
    dual -= q.dual;
    return *this;
  }
  
  /// Get the negative dual quaternion (unary minus)
  inline friend csDualQuaternion operator- (const csDualQuaternion& q)
  {
    return csDualQuaternion (-q.real, -q.dual);
  }

  /// Multiply two dual quaternions
  inline friend csDualQuaternion operator* (const csDualQuaternion& q1,
    const csDualQuaternion& q2)
  {
    return csDualQuaternion (q1.real * q2.real,
      q1.real * q2.dual + q2.real*q1.dual);
  }

  /// Multiply this quaternion by another
  inline csDualQuaternion& operator*= (const csDualQuaternion& q)
  {
    const csQuaternion oldReal (real);
    
    real *= q.real;
    dual = (oldReal*q.dual + q.real*dual);

    return *this;
  }

  /// Multiply by scalar
  inline friend csDualQuaternion operator* (const csDualQuaternion& q, float f)
  {
    return csDualQuaternion (q.real*f, q.dual*f);
  }

  /// Multiply by scalar
  inline friend csDualQuaternion operator* (float f, const csDualQuaternion& q)
  {
    return csDualQuaternion (q.real*f, q.dual*f);
  }

  /// Multiply by scalar
  inline csDualQuaternion& operator*= (float f)
  {
    real *= f;
    dual *= f;
    return *this;
  }

  /// Divide by scalar
  inline friend csDualQuaternion operator/ (const csDualQuaternion& q, float f)
  {
    float invF = 1.0f/f;
    return csDualQuaternion (q.real*invF, q.dual*invF);
  }

  /// Divide by scalar
  inline friend csDualQuaternion operator/ (float f, const csDualQuaternion& q)
  {
    float invF = 1.0f/f;
    return csDualQuaternion (q.real*invF, q.dual*invF);
  }

  /// v by scalar
  inline csDualQuaternion& operator/= (float f)
  {
    real /= f;
    dual /= f;
    return *this;
  }

  /// Get the conjugate dual quaternion
  inline csDualQuaternion GetConjugate () const
  {
    return csDualQuaternion (real.GetConjugate (), dual.GetConjugate ());
  }

  /// Set this dual quaternion to its own conjugate
  inline void Conjugate () 
  {
    real.Conjugate ();
    dual.Conjugate ();
  }

  /// Return euclidian inner-product (dot)
  inline csDualNumber Dot (const csDualQuaternion& q) const
  {
    return csDualNumber (real.Dot (q.real), dual.Dot (q.dual));
  }  

  /// Get the norm of this quaternion
  inline csDualNumber Norm () const
  {
    return csDualNumber (real.Norm (), real.Dot (dual) / real.Norm ());
  }

  /**
   * Return a unit-length version of this dual quaternion 
   * Attempting to normalize a dual quaternion with zero-norm real part will 
   * result in a divide by zero error.  
   * This is as it should be... fix the calling code.
   */
  inline csDualQuaternion Unit () const
  {    
    const float lenReal = real.Norm ();

    if (lenReal == 0)
    {
      return *this;
    }

    const float lenRealInv = 1.0f / lenReal;
    
    csQuaternion _real = real * lenRealInv;
    csQuaternion _dual = dual * lenRealInv;

    csQuaternion r (_real);
    r *= _real.Dot (_dual);
    r *= -1.0f;

    _dual += r;

    return csDualQuaternion (_real, _dual);
  }

  /**
   * Get the inverse dual quaternion.
   * Inverse is only defined when the real part is non-zero.
   */
  inline csDualQuaternion GetInverse () const
  {
    const float realNorm = real.Norm ();
    const float rdd = real.Dot (dual);

    return csDualQuaternion (
      real.GetConjugate ()*realNorm,
      dual.GetConjugate ()*(realNorm - 2*rdd));
  }

  /**
   * Transform a vector by a dual quaternion as if the vector is a position
   */
  inline csVector3 TransformPoint (const csVector3& v) const
  {
    csVector3 position = v + 2.0f * (real.v % ((real.v % v) + real.w * v));
    csVector3 trans = 2.0f * (real.w * dual.v - dual.w * real.v + (real.v % dual.v));

    return position + trans;
  }

  /**
   * Transform a vector by a dual quaternion
   */
  inline csVector3 Transform (const csVector3& v) const
  {
    csVector3 position = v + 2.0f * (real.v % ((real.v % v) + real.w * v));
    //csVector3 trans = 2.0f * (real.w * dual.v - dual.w * real.v + (real.v % dual.v));

    return position /*+ trans*/;
  }

  // Data
  /// The real part, representing rotational component
  csQuaternion real;

  /// The dual part, representing translational component
  csQuaternion dual;
};

/** @} */

#endif // __CS_QUATERNION_H__

