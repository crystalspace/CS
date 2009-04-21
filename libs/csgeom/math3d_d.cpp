/*
    Copyright (C) 1998,1999,2000 by Jorrit Tyberghein
    Largely rewritten by Ivan Avramovic <ivan@avramovic.com>
    Converted to double by Thomas Hieber

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
#include "csgeom/math3d_d.h"
#include "csgeom/math3d.h"

//---------------------------------------------------------------------------
double csDVector3::Norm () const
{
  return sqrt (x * x + y * y + z * z);
}

double csDVector3::SquaredNorm () const
{
  return x * x + y * y + z * z;
}

void csDVector3::Normalize ()
{
  double len;
  len = this->Norm ();
  if (len > SMALL_EPSILON) *this /= len;
}

csDVector3::csDVector3 (const csVector3 &v)
{
  x = v.x;
  y = v.y;
  z = v.z;
}

//---------------------------------------------------------------------------
csDMatrix3::csDMatrix3 ()
{
  m12 = m13 = 0;
  m21 = m23 = 0;
  m31 = m32 = 0;
  m11 = m22 = m33 = 1;
}

csDMatrix3::csDMatrix3 (
  double m11,
  double m12,
  double m13,
  double m21,
  double m22,
  double m23,
  double m31,
  double m32,
  double m33)
{
  csDMatrix3::m11 = m11;
  csDMatrix3::m12 = m12;
  csDMatrix3::m13 = m13;
  csDMatrix3::m21 = m21;
  csDMatrix3::m22 = m22;
  csDMatrix3::m23 = m23;
  csDMatrix3::m31 = m31;
  csDMatrix3::m32 = m32;
  csDMatrix3::m33 = m33;
}

csDMatrix3 &csDMatrix3::operator+= (const csDMatrix3 &m)
{
  m11 += m.m11;
  m12 += m.m12;
  m13 += m.m13;
  m21 += m.m21;
  m22 += m.m22;
  m23 += m.m23;
  m31 += m.m31;
  m32 += m.m32;
  m33 += m.m33;
  return *this;
}

csDMatrix3 &csDMatrix3::operator-= (const csDMatrix3 &m)
{
  m11 -= m.m11;
  m12 -= m.m12;
  m13 -= m.m13;
  m21 -= m.m21;
  m22 -= m.m22;
  m23 -= m.m23;
  m31 -= m.m31;
  m32 -= m.m32;
  m33 -= m.m33;
  return *this;
}

csDMatrix3 &csDMatrix3::operator*= (const csDMatrix3 &m)
{
  csDMatrix3 r;
  r.m11 = m11 * m.m11 + m12 * m.m21 + m13 * m.m31;
  r.m12 = m11 * m.m12 + m12 * m.m22 + m13 * m.m32;
  r.m13 = m11 * m.m13 + m12 * m.m23 + m13 * m.m33;
  r.m21 = m21 * m.m11 + m22 * m.m21 + m23 * m.m31;
  r.m22 = m21 * m.m12 + m22 * m.m22 + m23 * m.m32;
  r.m23 = m21 * m.m13 + m22 * m.m23 + m23 * m.m33;
  r.m31 = m31 * m.m11 + m32 * m.m21 + m33 * m.m31;
  r.m32 = m31 * m.m12 + m32 * m.m22 + m33 * m.m32;
  r.m33 = m31 * m.m13 + m32 * m.m23 + m33 * m.m33;
  *this = r;
  return *this;
}

csDMatrix3 &csDMatrix3::operator*= (double s)
{
  m11 *= s;
  m12 *= s;
  m13 *= s;
  m21 *= s;
  m22 *= s;
  m23 *= s;
  m31 *= s;
  m32 *= s;
  m33 *= s;
  return *this;
}

csDMatrix3& csDMatrix3::operator/= (double s)
{
  m11 /= s;
  m12 /= s;
  m13 /= s;
  m21 /= s;
  m22 /= s;
  m23 /= s;
  m31 /= s;
  m32 /= s;
  m33 /= s;
  return *this;
}

void csDMatrix3::Identity ()
{
  m12 = m13 = 0;
  m21 = m23 = 0;
  m31 = m32 = 0;
  m11 = m22 = m33 = 1;
}

void csDMatrix3::Transpose ()
{
  double swap;
  swap = m12;
  m12 = m21;
  m21 = swap;
  swap = m13;
  m13 = m31;
  m31 = swap;
  swap = m23;
  m23 = m32;
  m32 = swap;
}

csDMatrix3 csDMatrix3::GetTranspose () const
{
  csDMatrix3 t;
  t.m12 = m21;
  t.m21 = m12;
  t.m13 = m31;
  t.m31 = m13;
  t.m23 = m32;
  t.m32 = m23;
  t.m11 = m11;
  t.m22 = m22;
  t.m33 = m33;
  return t;
}

double csDMatrix3::Determinant () const
{
  return m11 * (m22 * m33 - m23 * m32) - m12 * (m21 * m33 - m23 * m31) + m13 * (m21 * m32 - m22 * m31);
}

csDMatrix3 operator+ (const csDMatrix3 &m1, const csDMatrix3 &m2)
{
  return csDMatrix3 (
      m1.m11 + m2.m11,
      m1.m12 + m2.m12,
      m1.m13 + m2.m13,
      m1.m21 + m2.m21,
      m1.m22 + m2.m22,
      m1.m23 + m2.m23,
      m1.m31 + m2.m31,
      m1.m32 + m2.m32,
      m1.m33 + m2.m33);
}

csDMatrix3 operator- (const csDMatrix3 &m1, const csDMatrix3 &m2)
{
  return csDMatrix3 (
      m1.m11 - m2.m11,
      m1.m12 - m2.m12,
      m1.m13 - m2.m13,
      m1.m21 - m2.m21,
      m1.m22 - m2.m22,
      m1.m23 - m2.m23,
      m1.m31 - m2.m31,
      m1.m32 - m2.m32,
      m1.m33 - m2.m33);
}

csDMatrix3 operator * (const csDMatrix3 &m1, const csDMatrix3 &m2)
{
  return csDMatrix3 (
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

csDMatrix3 operator * (const csDMatrix3 &m, double f)
{
  return csDMatrix3 (
      m.m11 * f,
      m.m12 * f,
      m.m13 * f,
      m.m21 * f,
      m.m22 * f,
      m.m23 * f,
      m.m31 * f,
      m.m32 * f,
      m.m33 * f);
}

csDMatrix3 operator * (double f, const csDMatrix3 &m)
{
  return csDMatrix3 (
      m.m11 * f,
      m.m12 * f,
      m.m13 * f,
      m.m21 * f,
      m.m22 * f,
      m.m23 * f,
      m.m31 * f,
      m.m32 * f,
      m.m33 * f);
}

csDMatrix3 operator/ (const csDMatrix3 &m, double f)
{
  return csDMatrix3 (
      m.m11 / f,
      m.m12 / f,
      m.m13 / f,
      m.m21 / f,
      m.m22 / f,
      m.m23 / f,
      m.m31 / f,
      m.m32 / f,
      m.m33 / f);
}

bool operator== (const csDMatrix3 &m1, const csDMatrix3 &m2)
{
  if (m1.m11 != m2.m11 || m1.m12 != m2.m12 || m1.m13 != m2.m13)
    return false;
  if (m1.m21 != m2.m21 || m1.m22 != m2.m22 || m1.m23 != m2.m23)
    return false;
  if (m1.m31 != m2.m31 || m1.m32 != m2.m32 || m1.m33 != m2.m33)
    return false;
  return true;
}

bool operator!= (const csDMatrix3 &m1, const csDMatrix3 &m2)
{
  if (m1.m11 != m2.m11 || m1.m12 != m2.m12 || m1.m13 != m2.m13) return true;
  if (m1.m21 != m2.m21 || m1.m22 != m2.m22 || m1.m23 != m2.m23) return true;
  if (m1.m31 != m2.m31 || m1.m32 != m2.m32 || m1.m33 != m2.m33) return true;
  return false;
}

bool operator < (const csDMatrix3 &m, double f)
{
  return
    ABS(m.m11) < f &&
    ABS(m.m12) < f &&
    ABS(m.m13) < f &&
    ABS(m.m21) < f &&
    ABS(m.m22) < f &&
    ABS(m.m23) < f &&
    ABS(m.m31) < f &&
    ABS(m.m32) < f &&
    ABS(m.m33) < f;
}
bool operator> (double f, const csDMatrix3 &m)
{
  return
    ABS (m.m11) < f &&
    ABS (m.m12) < f &&
    ABS (m.m13) < f &&
    ABS (m.m21) < f &&
    ABS (m.m22) < f &&
    ABS (m.m23) < f &&
    ABS (m.m31) < f &&
    ABS (m.m32) < f &&
    ABS (m.m33) < f;
}

//---------------------------------------------------------------------------
void csDMath3::Between (
  const csDVector3 &v1,
  const csDVector3 &v2,
  csDVector3 &v,
  double pct,
  double wid)
{
  if (pct != -1)
    pct /= 100.;
  else
  {
    double dist = sqrt ((v1 - v2) * (v1 - v2));
    if (dist == 0) return ;
    pct = wid / dist;
  }

  v = v1 + pct * (v2 - v1);
}

bool csDMath3::Visible (
  const csDVector3 &p,
  const csDVector3 &t1,
  const csDVector3 &t2,
  const csDVector3 &t3)
{
  double x1 = t1.x - p.x;
  double y1 = t1.y - p.y;
  double z1 = t1.z - p.z;
  double x2 = t2.x - p.x;
  double y2 = t2.y - p.y;
  double z2 = t2.z - p.z;
  double x3 = t3.x - p.x;
  double y3 = t3.y - p.y;
  double z3 = t3.z - p.z;
  double c = x3 *
    ((z1 * y2) - (y1 * z2)) +
    y3 *
    ((x1 * z2) - (z1 * x2)) +
    z3 *
    ((y1 * x2) - (x1 * y2));
  return c > 0;
}

bool csDMath3::PlanesClose (const csDPlane &p1, const csDPlane &p2)
{
  if (PlanesEqual (p1, p2)) return true;

  csDPlane p1n = p1;
  p1n.Normalize ();

  csDPlane p2n = p2;
  p2n.Normalize ();
  return PlanesEqual (p1n, p2n);
}

//---------------------------------------------------------------------------
double csDSquaredDist::PointLine (
  const csDVector3 &p,
  const csDVector3 &l1,
  const csDVector3 &l2)
{
  csDVector3 W = l1 - p;
  csDVector3 L = l2 - l1;
  csDVector3 p2l = W - L * (W * L) / (L * L);
  return p2l * p2l;
}

double csDSquaredDist::PointPoly (
  const csDVector3 &p,
  csDVector3 *V,
  int n,
  const csDPlane &plane,
  double sqdist)
{
  csDVector3 W, L;
  bool lflag = true, lflag0 = true;
  int i;
  for (i = 0; i < n - 1; i++)
  {
    W = V[i] - p;
    if (i == 0)
    {
      if (!(W * (V[n - 1] - V[0]) > 0))
        lflag0 = false;
      else if (W * (V[1] - V[0]) > 0)
        return W * W;
      else
        lflag = false;
    }
    else if (!(W * (L = V[i - 1] - V[i]) > 0))
    {
      if (!lflag && W * (plane.norm % L) > 0)
      {
        L = W - L * (W * L) / (L * L);
        return L * L;
      }

      lflag = (W * (V[i + 1] - V[i]) > 0);
    }
    else if (W * (V[i + 1] - V[i]) > 0)
      return W * W;
    else
      lflag = false;
  }

  W = V[n - 1] - p;
  if (!lflag)
  {
    lflag = W * (L = V[n - 2] - V[n - 1]) <= 0;
    if (lflag && (W * (plane.norm % L) > 0))
    {
      L = W - L * (W * L) / (L * L);
      return L * L;
    }
  }

  if (!lflag0)
  {
    lflag0 = W * (L = V[0] - V[n - 1]) <= 0;
    if (lflag0 && (W * (plane.norm % L) < 0))
    {
      L = W - L * (W * L) / (L * L);
      return L * L;
    }
  }

  if (!lflag && !lflag0) return W * W;
  if (sqdist >= 0) return sqdist;
  return csDSquaredDist::PointPlane (p, plane);
}

//---------------------------------------------------------------------------
void csDIntersect3::Plane (
  const csDVector3 &u,
  const csDVector3 &v,
  const csDVector3 &normal,
  const csDVector3 &a,
  csDVector3 &isect)
{
  double counter = normal * (u - a);
  double divider = normal * (v - u);
  double dist;
  if (divider == 0)
  {
    isect = v;
    return ;
  }

  dist = counter / divider;
  isect = u + dist * (u - v);
}

bool csDIntersect3::Plane (
  const csDVector3 &u,
  const csDVector3 &v,
  double A,
  double B,
  double C,
  double D,
  csDVector3 &isect,
  double &dist)
{
  double x, y, z, denom;

  x = v.x - u.x;
  y = v.y - u.y;
  z = v.z - u.z;
  denom = A * x + B * y + C * z;
  if (ABS (denom) < SMALL_EPSILON) return false;  // they are parallel
  dist = -(A * u.x + B * u.y + C * u.z + D) / denom;
  if (dist < -SMALL_EPSILON || dist > 1 + SMALL_EPSILON) return false;

  isect.x = u.x + dist * x;
  isect.y = u.y + dist * y;
  isect.z = u.z + dist * z;
  return true;
}

bool csDIntersect3::Plane (
  const csDVector3 &u,
  const csDVector3 &v,
  const csDPlane &p,
  csDVector3 &isect,
  double &dist)
{
  double x, y, z, denom;

  x = v.x - u.x;
  y = v.y - u.y;
  z = v.z - u.z;
  denom = p.norm.x * x + p.norm.y * y + p.norm.z * z;
  if (ABS (denom) < SMALL_EPSILON) return false;  // they are parallel
  dist = -(p.norm * u + p.DD) / denom;
  if (dist < -SMALL_EPSILON || dist > 1 + SMALL_EPSILON) return false;

  isect.x = u.x + dist * x;
  isect.y = u.y + dist * y;
  isect.z = u.z + dist * z;
  return true;
}

bool csDIntersect3::Planes (
  const csDPlane &p1,
  const csDPlane &p2,
  const csDPlane &p3,
  csDVector3 &isect)
{
  //To find the one point that is on all three planes, we need to solve
  //the following equation system (we need to find the x, y and z which
  //are true for all equations):
  // A1*x+B1*y+C1*z+D1=0 //plane1
  // A2*x+B2*y+C2*z+D2=0 //plane2
  // A3*x+B3*y+C3*z+D3=0 //plane3
  //This can be solved according to Cramers rule by looking at the
  //determinants of the equation system.
  csDMatrix3 mdet (
              p1.A (),
              p1.B (),
              p1.C (),
              p2.A (),
              p2.B (),
              p2.C (),
              p3.A (),
              p3.B (),
              p3.C ());
  double det = mdet.Determinant ();
  if (det == 0) return false;                     //some planes are parallel.
  csDMatrix3 mx (
              -p1.D (),
              p1.B (),
              p1.C (),
              -p2.D (),
              p2.B (),
              p2.C (),
              -p3.D (),
              p3.B (),
              p3.C ());
  double xdet = mx.Determinant ();

  csDMatrix3 my (
              p1.A (),
              -p1.D (),
              p1.C (),
              p2.A (),
              -p2.D (),
              p2.C (),
              p3.A (),
              -p3.D (),
              p3.C ());
  double ydet = my.Determinant ();

  csDMatrix3 mz (
              p1.A (),
              p1.B (),
              -p1.D (),
              p2.A (),
              p2.B (),
              -p2.D (),
              p3.A (),
              p3.B (),
              -p3.D ());
  double zdet = mz.Determinant ();

  isect.x = xdet / det;
  isect.y = ydet / det;
  isect.z = zdet / det;
  return true;
}

double csDIntersect3::Z0Plane (
  const csDVector3 &u,
  const csDVector3 &v,
  csDVector3 &isect)
{
  double r = u.z / (u.z - v.z);
  isect.x = r * (v.x - u.x) + u.x;
  isect.y = r * (v.y - u.y) + u.y;
  isect.z = 0;
  return r;
}

double csDIntersect3::ZPlane (
  double zval,
  const csDVector3 &u,
  const csDVector3 &v,
  csDVector3 &isect)
{
  double r = (zval - u.z) / (v.z - u.z);
  isect.x = r * (v.x - u.x) + u.x;
  isect.y = r * (v.y - u.y) + u.y;
  isect.z = zval;
  return r;
}

double csDIntersect3::XFrustum (
  double A,
  const csDVector3 &u,
  const csDVector3 &v,
  csDVector3 &isect)
{
  double r = (A * u.x + u.z) / (A * (u.x - v.x) + u.z - v.z);
  isect.x = r * (v.x - u.x) + u.x;
  isect.y = r * (v.y - u.y) + u.y;
  isect.z = r * (v.z - u.z) + u.z;
  return r;
}

double csDIntersect3::YFrustum (
  double B,
  const csDVector3 &u,
  const csDVector3 &v,
  csDVector3 &isect)
{
  double r = (B * u.y + u.z) / (B * (u.y - v.y) + u.z - v.z);
  isect.x = r * (v.x - u.x) + u.x;
  isect.y = r * (v.y - u.y) + u.y;
  isect.z = r * (v.z - u.z) + u.z;
  return r;
}
