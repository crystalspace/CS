/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
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

#ifndef __CS_TRANSFORM_H__
#define __CS_TRANSFORM_H__

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/matrix3.h"
#include "csgeom/plane3.h"
#include "csgeom/sphere.h"

class csReversibleTransform;

/**
 * A class which defines a transformation from one coordinate system to
 * another. The two coordinate systems are refered to as 'other'
 * and 'this'. The transform defines a transformation from 'other'
 * to 'this'.
 */
class CS_CRYSTALSPACE_EXPORT csTransform
{
protected:
  /// Transformation matrix from 'other' space to 'this' space.
  csMatrix3 m_o2t;
  /// Location of the origin for 'this' space.
  csVector3 v_o2t;

public:
  // Needed for GCC4. Otherwise emits a flood of "virtual functions but
  // non-virtual destructor" warnings.
  virtual ~csTransform() {};
  /**
   * Initialize with the identity transformation.
   */
  csTransform () : m_o2t (), v_o2t (0, 0, 0) {}

  /**
   * Initialize with the given transformation. The transformation
   * is given as a 3x3 matrix and a vector. The transformation
   * is defined to mean T=M*(O-V) with T the vector in 'this' space,
   * O the vector in 'other' space, M the transformation matrix and
   * V the transformation vector.
   */
  csTransform (const csMatrix3& other2this, const csVector3& origin_pos) :
  	m_o2t (other2this), v_o2t (origin_pos) {}

  /**
   * Reset this transform to the identity transform.
   */
  void Identity ()
  {
    SetO2TTranslation (csVector3 (0));
    SetO2T (csMatrix3 ());
  }

  /**
   * Returns true if this transform is an identity transform.
   * This tests all fields so don't call this before every operation.
   */
  bool IsIdentity () const
  {
    if (ABS (v_o2t.x) >= SMALL_EPSILON) return false;
    if (ABS (v_o2t.y) >= SMALL_EPSILON) return false;
    if (ABS (v_o2t.z) >= SMALL_EPSILON) return false;
    if (ABS (m_o2t.m11-1) >= SMALL_EPSILON) return false;
    if (ABS (m_o2t.m12) >= SMALL_EPSILON) return false;
    if (ABS (m_o2t.m13) >= SMALL_EPSILON) return false;
    if (ABS (m_o2t.m21) >= SMALL_EPSILON) return false;
    if (ABS (m_o2t.m22-1) >= SMALL_EPSILON) return false;
    if (ABS (m_o2t.m23) >= SMALL_EPSILON) return false;
    if (ABS (m_o2t.m31) >= SMALL_EPSILON) return false;
    if (ABS (m_o2t.m32) >= SMALL_EPSILON) return false;
    if (ABS (m_o2t.m33-1) >= SMALL_EPSILON) return false;
    return true;
  }

  /**
   * Get 'other' to 'this' transformation matrix. This is the 3x3
   * matrix M from the transform equation T=M*(O-V).
   */
  inline const csMatrix3& GetO2T () const { return m_o2t; }

  /**
   * Get 'world' to 'this' translation. This is the vector V
   * from the transform equation T=M*(O-V). This is equivalent
   * to calling GetOrigin().
   */
  inline const csVector3& GetO2TTranslation () const { return v_o2t; }

  /**
   * Get origin of transformed coordinate system. This is equivalent
   * to calling GetO2TTranslation().
   */
  inline const csVector3& GetOrigin () const { return v_o2t; }

  /**
   * Set 'other' to 'this' transformation matrix.
   * This is the 3x3 matrix M from the transform equation T=M*(O-V).
   */
  virtual void SetO2T (const csMatrix3& m) { m_o2t = m; }

  /**
   * Set 'world' to 'this' translation. This is the vector V
   * from the transform equation T=M*(O-V). This is equivalent to
   * calling SetOrigin().
   */
  virtual void SetO2TTranslation (const csVector3& v) { v_o2t = v; }

  /**
   * Set origin of transformed coordinate system. This is equivalent
   * to calling SetO2TTranslation().
   */
  inline void SetOrigin (const csVector3& v) { SetO2TTranslation (v); }

  /**
   * Move the 'other' to 'this' translation by a specified amount.
   * Basically this will add 'v' to the origin or translation of this
   * transform so that the new transform looks like T=M*(O-(V+v)).
   */
  inline void Translate (const csVector3& v) { SetO2TTranslation (v_o2t + v); }

  /**
   * Transform vector in 'other' space v to a vector in 'this' space.
   * This is the basic transform function. This will calculate and return
   * M*(v-V).
   */
  inline csVector3 Other2This (const csVector3& v) const
  {
    return m_o2t * (v - v_o2t);
  }

  /**
   * Convert vector v in 'other' space to a vector in 'this' space.
   * Use the origin of 'other' space. This will calculate and return
   * M*v (so the translation or V of this transform is ignored).
   */
  csVector3 Other2ThisRelative (const csVector3& v) const
  { return m_o2t * v; }

  /**
   * Convert a plane in 'other' space to 'this' space. If 'p' is expressed
   * as (N,D) (with N a vector for the A,B,C components of 'p') then this will
   * return a new plane which looks like (M*N,D+(M*N)*(M*V)).
   */
  csPlane3 Other2This (const csPlane3& p) const;

  /**
   * Convert a plane in 'other' space to 'this' space.
   * This version ignores translation. If 'p' is expressed as (N,D) (with
   * N a vector for the A,B,C components of 'p') then this will return a new
   * plane which looks like (M*N,D).
   */
  csPlane3 Other2ThisRelative (const csPlane3& p) const;

  /**
   * Convert a plane in 'other' space to 'this' space. This is an optimized
   * version for which a point on the new plane is known (point). The result
   * is stored in 'result'. If 'p' is expressed as (N,D) (with N a vector
   * for the A,B,C components of 'p') then this will return a new plane
   * in 'result' which looks like (M*N,-(M*N)*point).
   */
  void Other2This (const csPlane3& p, const csVector3& point,
  	csPlane3& result) const;

  /**
   * Convert a sphere in 'other' space to 'this' space.
   */
  csSphere Other2This (const csSphere& s) const;

  /**
   * Apply a transformation to a 3D vector. This corresponds exactly
   * to calling t.Other2This (v).
   */
  friend CS_CRYSTALSPACE_EXPORT csVector3 operator* (const csVector3& v, 
    const csTransform& t);

  /**
   * Apply a transformation to a 3D vector. This corresponds exactly
   * to calling t.Other2This (v).
   */
  friend CS_CRYSTALSPACE_EXPORT csVector3 operator* (const csTransform& t, 
    const csVector3& v);

  /**
   * Apply a transformation to a 3D vector. This corresponds exactly
   * to calling v = t.Other2This(v).
   */
  friend CS_CRYSTALSPACE_EXPORT csVector3& operator*= (csVector3& v, 
    const csTransform& t);

  /**
   * Apply a transformation to a Plane. This corresponds exactly
   * to calling t.Other2This(p).
   */
  friend CS_CRYSTALSPACE_EXPORT csPlane3 operator* (const csPlane3& p, 
    const csTransform& t);

  /**
   * Apply a transformation to a Plane. This corresponds exactly
   * to calling t.Other2This(p).
   */
  friend CS_CRYSTALSPACE_EXPORT csPlane3 operator* (const csTransform& t, 
    const csPlane3& p);

  /**
   * Apply a transformation to a Plane. This corresponds exactly
   * to calling p = t.Other2This(p).
   */
  friend CS_CRYSTALSPACE_EXPORT csPlane3& operator*= (csPlane3& p, 
    const csTransform& t);

  /**
   * Apply a transformation to a sphere. This corresponds exactly
   * to calling t.Other2This(p).
   */
  friend CS_CRYSTALSPACE_EXPORT csSphere operator* (const csSphere& p, 
    const csTransform& t);

  /**
   * Apply a transformation to a sphere. This corresponds exactly
   * to calling t.Other2This(p).
   */
  friend CS_CRYSTALSPACE_EXPORT csSphere operator* (const csTransform& t, 
    const csSphere& p);

  /**
   * Apply a transformation to a sphere. This corresponds exactly
   * to calling p = t.Other2This(p).
   */
  friend CS_CRYSTALSPACE_EXPORT csSphere& operator*= (csSphere& p, 
    const csTransform& t);

  /**
   * Multiply a matrix with the transformation matrix. This will calculate
   * and return m*M.
   */
  friend CS_CRYSTALSPACE_EXPORT csMatrix3 operator* (const csMatrix3& m, 
    const csTransform& t);

  /**
   * Multiply a matrix with the transformation matrix. This will calculate
   * and return M*m.
   */
  friend CS_CRYSTALSPACE_EXPORT csMatrix3 operator* (const csTransform& t, 
    const csMatrix3& m);

  /**
   * Multiply a matrix with the transformation matrix.
   * This corresponds exactly to m*=M.
   */
  friend CS_CRYSTALSPACE_EXPORT csMatrix3& operator*= (csMatrix3& m, 
    const csTransform& t);

  /**
   * Combine two transforms, rightmost first. Given the following
   * definitions:
   * <ul>
   * <li>'t1' expressed as T=t1.M*(O-t1.V)
   * <li>'t2' expressed as T=t2.M*(O-t2.V)
   * <li>t2.Minv is the inverse of t2.M
   * </ul>
   * Then this will return a new transform
   * T=(t1.M*t2.M)*(O-(t2.V+t2.Minv*t1.V)).
   */
  friend CS_CRYSTALSPACE_EXPORT csTransform operator* (const csTransform& t1,
                              const csReversibleTransform& t2);

  /**
   * Return a transform that represents a mirroring across a plane.
   * This function will return a csTransform which represents a reflection
   * across the plane pl.
   */
  static csTransform GetReflect (const csPlane3& pl);
};

/**
 * A class which defines a reversible transformation from one coordinate
 * system to another by maintaining an inverse transformation matrix.
 * This version is similar to csTransform (in fact, it is a sub-class)
 * but it is more efficient if you plan to do inverse transformations
 * often.
 */
class CS_CRYSTALSPACE_EXPORT csReversibleTransform : public csTransform
{
protected:
  /// Inverse transformation matrix ('this' to 'other' space).
  csMatrix3 m_t2o;

  /**
   * Initialize transform with both transform matrix and inverse tranform.
   */
  csReversibleTransform (const csMatrix3& o2t, const csMatrix3& t2o,
    const csVector3& pos) : csTransform (o2t,pos), m_t2o (t2o) {}

public:
  /**
   * Initialize with the identity transformation.
   */
  csReversibleTransform () : csTransform (), m_t2o () {}

  /**
   * Initialize with the given transformation. The transformation
   * is given as a 3x3 matrix and a vector. The transformation
   * is defined to mean T=M*(O-V) with T the vector in 'this' space,
   * O the vector in 'other' space, M the transformation matrix and
   * V the transformation vector.
   */
  csReversibleTransform (const csMatrix3& o2t, const csVector3& pos) :
    csTransform (o2t,pos) { m_t2o = m_o2t.GetInverse (); }

  /**
   * Initialize with the given transformation.
   */
  csReversibleTransform (const csTransform& t) :
    csTransform (t) { m_t2o = m_o2t.GetInverse (); }

  /**
   * Initialize with the given transformation.
   */
  csReversibleTransform (const csReversibleTransform& t) :
    csTransform (t) { m_t2o = t.m_t2o; }

  /**
   * Get 'this' to 'other' transformation matrix. This corresponds
   * to the inverse of M.
   */
  inline const csMatrix3& GetT2O () const { return m_t2o; }

  /**
   * Get 'this' to 'other' translation. This will calculate
   * and return -(M*V).
   */
  inline csVector3 GetT2OTranslation () const { return -m_o2t*v_o2t; }

  /**
   * Get the inverse of this transform.
   */
  csReversibleTransform GetInverse () const
  { return csReversibleTransform (m_t2o, m_o2t, -m_o2t*v_o2t); }

  /**
   * Set 'other' to 'this' transformation matrix.
   * This is the 3x3 matrix M from the transform equation T=M*(O-V).
   */
  virtual void SetO2T (const csMatrix3& m)
  { m_o2t = m;  m_t2o = m_o2t.GetInverse (); }

  /**
   * Set 'this' to 'other' transformation matrix.
   * This is equivalent to SetO2T() except that you can now give the
   * inverse matrix.
   */
  virtual void SetT2O (const csMatrix3& m)
  { m_t2o = m;  m_o2t = m_t2o.GetInverse (); }

  /**
   * Convert vector v in 'this' space to 'other' space.
   * This is the basic inverse transform operation and it corresponds
   * with the calculation of V+Minv*v (with Minv the inverse of M).
   */
  csVector3 This2Other (const csVector3& v) const
  { return v_o2t + m_t2o * v; }

  /**
   * Convert vector v in 'this' space to a vector in 'other' space,
   * relative to local origin. This calculates and returns
   * Minv*v (with Minv the inverse of M).
   */
  inline csVector3 This2OtherRelative (const csVector3& v) const
  { return m_t2o * v; }

  /**
   * Convert a plane in 'this' space to 'other' space. If 'p' is expressed
   * as (N,D) (with N a vector for the A,B,C components of 'p') then this will
   * return a new plane which looks like (Minv*N,D-N*(M*V)) (with Minv
   * the inverse of M).
   */
  csPlane3 This2Other (const csPlane3& p) const;

  /**
   * Convert a plane in 'this' space to 'other' space.
   * This version ignores translation. If 'p' is expressed as (N,D) (with
   * N a vector for the A,B,C components of 'p') then this will return a new
   * plane which looks like (Minv*N,D) (with Minv the inverse of M).
   */
  csPlane3 This2OtherRelative (const csPlane3& p) const;

  /**
   * Convert a plane in 'this' space to 'other' space. This is an optimized
   * version for which a point on the new plane is known (point). The result
   * is stored in 'result'. If 'p' is expressed as (N,D) (with N a vector
   * for the A,B,C components of 'p') then this will return a new
   * plane which looks like (Minv*N,-(Minv*N)*point) (with Minv the inverse
   * of M).
   */
  void This2Other (const csPlane3& p, const csVector3& point,
  	csPlane3& result) const;

  /**
   * Convert a sphere in 'this' space to 'other' space.
   */
  csSphere This2Other (const csSphere& s) const;

  /**
   * Rotate the transform by the angle (radians) around the given vector,
   * in other coordinates. Note: this function rotates the transform, not
   * the coordinate system.
   */
  void RotateOther (const csVector3& v, float angle);

  /**
   * Rotate the transform by the angle (radians) around the given vector,
   * in these coordinates. Note: this function rotates the tranform,
   * not the coordinate system.
   */
  void RotateThis (const csVector3& v, float angle);

  /**
   * Use the given transformation matrix, in other space,
   * to reorient the transformation. Note: this function rotates the
   * transformation, not the coordinate system. This basically
   * calculates Minv=m*Minv (with Minv the inverse of M). M will be
   * calculated accordingly.
   */
  void RotateOther (const csMatrix3& m) { SetT2O (m * m_t2o); }

  /**
   * Use the given transformation matrix, in this space,
   * to reorient the transformation. Note: this function rotates the
   * transformation, not the coordinate system. This basically
   * calculates Minv=Minv*m (with Minv the inverse of M). M will be
   * calculated accordingly.
   */
  void RotateThis (const csMatrix3& m) { SetT2O (m_t2o * m); }

  /**
   * Let this transform look at the given (x,y,z) point, using up as
   * the up-vector. 'v' should be given relative to the position
   * of the origin of this transform. For example, if the transform is
   * located at pos=(3,1,9) and you want it to look at location
   * loc=(10,2,8) while keeping the orientation so that the up-vector is
   * upwards then you can use: LookAt (loc-pos, csVector3 (0, 1, 0)).
   */
  void LookAt (const csVector3& v, const csVector3& up);

  /**
   * Reverse a transformation on a 3D vector. This corresponds exactly
   * to calling t.This2Other(v).
   */
  friend CS_CRYSTALSPACE_EXPORT csVector3 operator/ (const csVector3& v,
  	const csReversibleTransform& t);

  /**
   * Reverse a transformation on a 3D vector. This corresponds exactly
   * to calling v=t.This2Other(v).
   */
  friend CS_CRYSTALSPACE_EXPORT csVector3& operator/= (csVector3& v, 
    const csReversibleTransform& t);

  /**
   * Reverse a transformation on a Plane. This corresponds exactly
   * to calling t.This2Other(p).
   */
  friend CS_CRYSTALSPACE_EXPORT csPlane3 operator/ (const csPlane3& p, 
    const csReversibleTransform& t);

  /**
   * Reverse a transformation on a Plane. This corresponds exactly to
   * calling p = t.This2Other(p).
   */
  friend CS_CRYSTALSPACE_EXPORT csPlane3& operator/= (csPlane3& p, 
    const csReversibleTransform& t);

  /**
   * Reverse a transformation on a sphere. This corresponds exactly to
   * calling t.This2Other(p).
   */
  friend CS_CRYSTALSPACE_EXPORT csSphere operator/ (const csSphere& p, 
    const csReversibleTransform& t);

  /**
   * Combine two transforms, rightmost first. Given the following
   * definitions:
   * <ul>
   * <li>'t1' expressed as T=t1.M*(O-t1.V)
   * <li>'t2' expressed as T=t2.M*(O-t2.V)
   * <li>t1.Minv is the inverse of t1.M
   * <li>t2.Minv is the inverse of t2.M
   * </ul>
   * Then this will calculate a new transformation in 't1' as follows:
   * T=(t1.M*t2.M)*(O-(t2.Minv*t1.V+t2.V)).
   */
  friend csReversibleTransform& operator*= (csReversibleTransform& t1,
                                          const csReversibleTransform& t2)
  {
    t1.v_o2t = t2.m_t2o*t1.v_o2t;
    t1.v_o2t += t2.v_o2t;
    t1.m_o2t *= t2.m_o2t;
    t1.m_t2o *= t1.m_t2o;
    return t1;
  }

  /**
   * Combine two transforms, rightmost first. Given the following
   * definitions:
   * <ul>
   * <li>'t1' expressed as T=t1.M*(O-t1.V)
   * <li>'t2' expressed as T=t2.M*(O-t2.V)
   * <li>t1.Minv is the inverse of t1.M
   * <li>t2.Minv is the inverse of t2.M
   * </ul>
   * Then this will calculate a new transformation in 't1' as follows:
   * T=(t1.M*t2.M)*(O-(t2.Minv*t1.V+t2.V)).
   */
  friend csReversibleTransform operator* (const csReversibleTransform& t1,
                                        const csReversibleTransform& t2)
  {
    return csReversibleTransform (t1.m_o2t*t2.m_o2t, t2.m_t2o*t1.m_t2o,
                             t2.v_o2t + t2.m_t2o*t1.v_o2t);
  }

#if !defined(SWIG) /* Otherwise Swig 1.3.22 thinks this is multiply declared */
  /**
   * Combine two transforms, rightmost first. Given the following
   * definitions:
   * <ul>
   * <li>'t1' expressed as T=t1.M*(O-t1.V)
   * <li>'t2' expressed as T=t2.M*(O-t2.V)
   * <li>t1.Minv is the inverse of t1.M
   * <li>t2.Minv is the inverse of t2.M
   * </ul>
   * Then this will calculate a new transformation in 't1' as follows:
   * T=(t1.M*t2.M)*(O-(t2.Minv*t1.V+t2.V)).
   */
  friend CS_CRYSTALSPACE_EXPORT csTransform operator* (const csTransform& t1,
                              const csReversibleTransform& t2);
#endif

  /**
   * Combine two transforms, reversing t2 then applying t1.
   * Given the following definitions:
   * <ul>
   * <li>'t1' expressed as T=t1.M*(O-t1.V)
   * <li>'t2' expressed as T=t2.M*(O-t2.V)
   * <li>t1.Minv is the inverse of t1.M
   * <li>t2.Minv is the inverse of t2.M
   * </ul>
   * Then this will calculate a new transformation in 't1' as follows:
   * T=(t1.M*t2.Minv)*(O-(t2.M*(t1.V-t2.V))).
   */
  friend CS_CRYSTALSPACE_EXPORT csReversibleTransform& operator/= (
    csReversibleTransform& t1, const csReversibleTransform& t2);

  /**
   * Combine two transforms, reversing t2 then applying t1.
   * Given the following definitions:
   * <ul>
   * <li>'t1' expressed as T=t1.M*(O-t1.V)
   * <li>'t2' expressed as T=t2.M*(O-t2.V)
   * <li>t1.Minv is the inverse of t1.M
   * <li>t2.Minv is the inverse of t2.M
   * </ul>
   * Then this will calculate a new transformation in 't1' as follows:
   * T=(t1.M*t2.Minv)*(O-(t2.M*(t1.V-t2.V))).
   */
  friend CS_CRYSTALSPACE_EXPORT csReversibleTransform operator/ (
    const csReversibleTransform& t1, const csReversibleTransform& t2);
};

/**
 * A class which defines a reversible transformation from one coordinate
 * system to another by maintaining an inverse transformation matrix.
 * This is a variant which only works on orthonormal transformations (like
 * the camera transformation) and is consequently much more optimal.
 */
class csOrthoTransform : public csReversibleTransform
{
public:
  /**
   * Initialize with the identity transformation.
   */
  csOrthoTransform () : csReversibleTransform () {}

  /**
   * Initialize with the given transformation.
   */
  csOrthoTransform (const csMatrix3& o2t, const csVector3& pos) :
    csReversibleTransform (o2t, o2t.GetTranspose (), pos) { }

  /**
   * Initialize with the given transformation.
   */
  csOrthoTransform (const csTransform& t) :
    csReversibleTransform (t.GetO2T (), t.GetO2T ().GetTranspose (),
    	t.GetO2TTranslation ())
  { }

  /**
   * Set 'other' to 'this' transformation matrix.
   * This is the 3x3 matrix M from the transform equation T=M*(O-V).
   */
  virtual void SetO2T (const csMatrix3& m)
  { m_o2t = m;  m_t2o = m_o2t.GetTranspose (); }

  /**
   * Set 'this' to 'other' transformation matrix.
   * This is equivalent to SetO2T() except that you can now give the
   * inverse matrix.
   */
  virtual void SetT2O (const csMatrix3& m)
  { m_t2o = m;  m_o2t = m_t2o.GetTranspose (); }
};

/** @} */

#endif // __CS_TRANSFORM_H__

