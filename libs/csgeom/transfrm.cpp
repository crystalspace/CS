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

#include "cssysdef.h"
#include <math.h>
#include "csqsqrt.h"

#include "csgeom/matrix3.h"
#include "csgeom/plane3.h"
#include "csgeom/sphere.h"
#include "csgeom/transfrm.h"


//---------------------------------------------------------------------------
csTransform csTransform::GetReflect (const csPlane3 &pl)
{
  // Suppose that n is the plane normal in the direction of th reflection.
  // Suppose that u is the unit vector in the direction of the reflection
  // normal.  For any vector v, the component of v in the direction of
  // u is equal to (v * u) * u.  Thus, if v is reflected across a plane
  // through the origin with the given normal, the resulting vector is
  //  v' = v - 2 * [ (v * u) * u ] = v - 2 [ (v * n) * n ] / (n * n)
  //
  // x = <1,0,0>  =>  x' = <1,0,0> - 2 ( n.x * n ) / (n*n)
  // y = <0,1,0>  =>  y' = <0,1,0> - 2 ( n.y * n ) / (n*n)
  // z = <0,0,1>  =>  z' = <0,0,1> - 2 ( n.z * n ) / (n*n)
  //
  // 3x3 transformation matrix = [x' y' z']
  float i_normsq = 1 / (pl.norm * pl.norm);
  csVector3 xvec = (-2 * pl.norm.x * i_normsq) * pl.norm;
  csVector3 yvec = (-2 * pl.norm.y * i_normsq) * pl.norm;
  csVector3 zvec = (-2 * pl.norm.z * i_normsq) * pl.norm;
  xvec.x += 1;
  yvec.y += 1;
  zvec.z += 1;

  return csTransform (
      csMatrix3 (
        xvec.x,
        yvec.x,
        zvec.x,
        xvec.y,
        yvec.y,
        zvec.y,
        xvec.z,
        yvec.z,
        zvec.z),
      /* neworig = */(-2 * pl.DD * i_normsq) * pl.norm);
}

//---------------------------------------------------------------------------
csPlane3 csTransform::Other2This (const csPlane3 &p) const
{
  csVector3 newnorm = m_o2t * p.norm;

  // let N represent norm <A,B,C>, and X represent point <x,y,z>
  //
  // Old plane equation: N*X + D = 0
  // There exists point X = <r*A,r*B,r*C> = r*N which satisfies the
  // plane equation.
  //  => r*(N*N) + D = 0
  //  => r = -D/(N*N)
  //
  // New plane equation: N'*X' + D' = 0
  // If M is the transformation matrix, and V the transformation vector,
  // N' = M*N, and X' = M*(X-V).  Assume that N' is already calculated.
  //  => N'*(M*(X-V)) + D' = 0
  //  => D' = -N'*(M*X) + N'*(M*V)
  //        = -N'*(M*(r*N)) + N'*(M*V)
  //        = -r*(N'*N') + N'*(M*V) = D*(N'*N')/(N*N) + N'*(M*V)
  // Since N' is a rotation of N, (N'*N') = (N*N), thus
  //  D' = D + N'*(M*V)
  //
  return csPlane3 (newnorm, p.DD + newnorm * (m_o2t * v_o2t));
}

csPlane3 csTransform::Other2ThisRelative (const csPlane3 &p) const
{
  csVector3 newnorm = m_o2t * p.norm;
  return csPlane3 (newnorm, p.DD);
}

void csTransform::Other2This (
  const csPlane3 &p,
  const csVector3 &point,
  csPlane3 &result) const
{
  result.norm = m_o2t * p.norm;
  result.DD = -(result.norm * point);
}

csSphere csTransform::Other2This (const csSphere &s) const
{
  csSphere news;
  news.SetCenter (Other2This (s.GetCenter ()));

  // @@@ It would be nice if we could quickly detect if a given
  // transformation is orthonormal. In that case we don't need to transform
  // the radius.
  // To transform the radius we transform a vector with the radius

  // relative to the transform.
  csVector3 v_radius (s.GetRadius ());
  v_radius = Other2ThisRelative (v_radius);

  float radius = (float)fabs (v_radius.x);
  if (radius < (float)fabs (v_radius.y)) radius = (float)fabs (v_radius.y);
  if (radius < (float)fabs (v_radius.z)) radius = (float)fabs (v_radius.z);
  news.SetRadius (radius);
  return news;
}

csVector3 operator * (const csVector3 &v, const csTransform &t)
{
  return t.Other2This (v);
}

csVector3 operator * (const csTransform &t, const csVector3 &v)
{
  return t.Other2This (v);
}

csVector3 &operator*= (csVector3 &v, const csTransform &t)
{
  v = t.Other2This (v);
  return v;
}

csPlane3 operator * (const csPlane3 &p, const csTransform &t)
{
  return t.Other2This (p);
}

csPlane3 operator * (const csTransform &t, const csPlane3 &p)
{
  return t.Other2This (p);
}

csPlane3 &operator*= (csPlane3 &p, const csTransform &t)
{
  p.norm = t.m_o2t * p.norm;
  p.DD += p.norm * (t.m_o2t * t.v_o2t);
  return p;
}

csSphere operator * (const csSphere &p, const csTransform &t)
{
  return t.Other2This (p);
}

csSphere operator * (const csTransform &t, const csSphere &p)
{
  return t.Other2This (p);
}

csSphere &operator*= (csSphere &p, const csTransform &t)
{
  p.SetCenter (t.Other2This (p.GetCenter ()));

  // @@@ It would be nice if we could quickly detect if a given
  // transformation is orthonormal. In that case we don't need to transform
  // the radius.
  // To transform the radius we transform a vector with the radius
  // relative to the transform.
  csVector3 v_radius (p.GetRadius ());
  v_radius = t.Other2ThisRelative (v_radius);

  float radius = (float)fabs (v_radius.x);
  if (radius < (float)fabs (v_radius.y)) radius = (float)fabs (v_radius.y);
  if (radius < (float)fabs (v_radius.z)) radius = (float)fabs (v_radius.z);
  p.SetRadius (radius);
  return p;
}

csMatrix3 operator * (const csMatrix3 &m, const csTransform &t)
{
  return m * t.m_o2t;
}

csMatrix3 operator * (const csTransform &t, const csMatrix3 &m)
{
  return t.m_o2t * m;
}

csMatrix3 &operator*= (csMatrix3 &m, const csTransform &t)
{
  return m *= t.m_o2t;
}

//---------------------------------------------------------------------------
csPlane3 csReversibleTransform::This2Other (const csPlane3 &p) const
{
  csVector3 newnorm = m_t2o * p.norm;
  return csPlane3 (newnorm, p.DD - p.norm * (m_o2t * v_o2t));
}

void csReversibleTransform::This2Other (
  const csPlane3 &p,
  const csVector3 &point,
  csPlane3 &result) const
{
  result.norm = m_t2o * p.norm;
  result.DD = -(result.norm * point);
}

csPlane3 csReversibleTransform::This2OtherRelative (const csPlane3 &p) const
{
  csVector3 newnorm = m_t2o * p.norm;
  return csPlane3 (newnorm, p.DD);
}

csSphere csReversibleTransform::This2Other (const csSphere &s) const
{
  csSphere news;
  news.SetCenter (This2Other (s.GetCenter ()));

  // @@@ It would be nice if we could quickly detect if a given
  // transformation is orthonormal. In that case we don't need to transform
  // the radius.
  // To transform the radius we transform a vector with the radius
  // relative to the transform.
  csVector3 v_radius (s.GetRadius ());
  v_radius = This2OtherRelative (v_radius);

  float radius = (float)fabs (v_radius.x);
  if (radius < (float)fabs (v_radius.y)) radius = (float)fabs (v_radius.y);
  if (radius < (float)fabs (v_radius.z)) radius = (float)fabs (v_radius.z);
  news.SetRadius (radius);
  return news;
}

csVector3 operator/ (const csVector3 &v, const csReversibleTransform &t)
{
  return t.This2Other (v);
}

csVector3 &operator/= (csVector3 &v, const csReversibleTransform &t)
{
  v = t.This2Other (v);
  return v;
}

csPlane3 operator/ (const csPlane3 &p, const csReversibleTransform &t)
{
  return t.This2Other (p);
}

csSphere operator/ (const csSphere &p, const csReversibleTransform &t)
{
  return t.This2Other (p);
}

csPlane3 &operator/= (csPlane3 &p, const csReversibleTransform &t)
{
  p.DD -= p.norm * (t.m_o2t * t.v_o2t);
  p.norm = t.m_t2o * p.norm;
  return p;
}

csTransform operator * (
  const csTransform &t1,
  const csReversibleTransform &t2)
{
  return csTransform (t1.m_o2t * t2.m_o2t, t2.v_o2t + t2.m_t2o * t1.v_o2t);
}

csReversibleTransform &operator/= (
  csReversibleTransform &t1,
  const csReversibleTransform &t2)
{
  t1.v_o2t = t2.m_o2t * (t1.v_o2t - t2.v_o2t);
  t1.m_o2t *= t2.m_t2o;
  t1.m_t2o = t2.m_o2t * t1.m_t2o;
  return t1;
}

csReversibleTransform operator/ (
  const csReversibleTransform &t1,
  const csReversibleTransform &t2)
{
  return csReversibleTransform (
      t1.m_o2t * t2.m_t2o,
      t2.m_o2t * t1.m_t2o,
      t2.m_o2t * (t1.v_o2t - t2.v_o2t));
}

void csReversibleTransform::RotateOther (const csVector3 &v, float angle)
{
  csVector3 u = v;
  float ca, sa, omcaux, omcauy, omcauz, uxsa, uysa, uzsa;
  u = csVector3::Unit (u);
  ca = (float)cos (angle);
  sa = (float)sin (angle);
  omcaux = (1 - ca) * u.x;
  omcauy = (1 - ca) * u.y;
  omcauz = (1 - ca) * u.z;
  uxsa = u.x * sa;
  uysa = u.y * sa;
  uzsa = u.z * sa;

  SetT2O (
    csMatrix3 (
        u.x * omcaux + ca,
        u.y * omcaux - uzsa,
        u.z * omcaux + uysa,
        u.x * omcauy + uzsa,
        u.y * omcauy + ca,
        u.z * omcauy - uxsa,
        u.x * omcauz - uysa,
        u.y * omcauz + uxsa,
        u.z * omcauz + ca) * GetT2O ());
}

void csReversibleTransform::RotateThis (const csVector3 &v, float angle)
{
  csVector3 u = v;
  float ca, sa, omcaux, omcauy, omcauz, uxsa, uysa, uzsa;
  u = csVector3::Unit (u);
  ca = (float)cos (angle);
  sa = (float)sin (angle);
  omcaux = (1 - ca) * u.x;
  omcauy = (1 - ca) * u.y;
  omcauz = (1 - ca) * u.z;
  uxsa = u.x * sa;
  uysa = u.y * sa;
  uzsa = u.z * sa;

  SetT2O (
    GetT2O () * csMatrix3 (
        u.x * omcaux + ca,
        u.y * omcaux - uzsa,
        u.z * omcaux + uysa,
        u.x * omcauy + uzsa,
        u.y * omcauy + ca,
        u.z * omcauy - uxsa,
        u.x * omcauz - uysa,
        u.y * omcauz + uxsa,
        u.z * omcauz + ca));
}

void csReversibleTransform::LookAt (
  const csVector3 &v,
  const csVector3 &upNeg)
{
  csMatrix3 m;  /* initialized to be the identity matrix */
  csVector3 w1, w2, w3 = v;
  csVector3 up = -upNeg;

  float sqr;
  sqr = v * v;
  if (sqr > SMALL_EPSILON)
  {
    w3 *= csQisqrt (sqr);
    w1 = w3 % up;
    sqr = w1 * w1;
    if (sqr < SMALL_EPSILON)
    {
      w1 = w3 % csVector3 (0, 0, -1);
      sqr = w1 * w1;
      if (sqr < SMALL_EPSILON)
      {
        w1 = w3 % csVector3 (0, -1, 0);
        sqr = w1 * w1;
      }
    }

    w1 *= csQisqrt (sqr);
    w2 = w3 % w1;

    m.m11 = w1.x;
    m.m12 = w2.x;
    m.m13 = w3.x;
    m.m21 = w1.y;
    m.m22 = w2.y;
    m.m23 = w3.y;
    m.m31 = w1.z;
    m.m32 = w2.z;
    m.m33 = w3.z;
  }

  SetT2O (m);
}

//---------------------------------------------------------------------------
