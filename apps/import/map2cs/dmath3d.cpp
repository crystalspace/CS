/*
    Copyright (C) 1998 by Jorrit Tyberghein
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
#include "dmath3d.h"

//---------------------------------------------------------------------------

double CdVector3::Norm () const
{ return sqrt (x*x + y*y + z*z); }

//---------------------------------------------------------------------------

CdMatrix3::CdMatrix3 ()
{
  m12 = m13 = 0;
  m21 = m23 = 0;
  m31 = m32 = 0;
  m11 = m22 = m33 = 1;
}

CdMatrix3::CdMatrix3 (double m11, double m12, double m13,
  	    	  double m21, double m22, double m23,
  	   	  double m31, double m32, double m33)
{
  CdMatrix3::m11 = m11;
  CdMatrix3::m12 = m12;
  CdMatrix3::m13 = m13;
  CdMatrix3::m21 = m21;
  CdMatrix3::m22 = m22;
  CdMatrix3::m23 = m23;
  CdMatrix3::m31 = m31;
  CdMatrix3::m32 = m32;
  CdMatrix3::m33 = m33;
}

CdMatrix3& CdMatrix3::operator+= (const CdMatrix3& m)
{
  m11 += m.m11; m12 += m.m12; m13 += m.m13;
  m21 += m.m21; m22 += m.m22; m23 += m.m23;
  m31 += m.m31; m32 += m.m32; m33 += m.m33;
  return *this;
}

CdMatrix3& CdMatrix3::operator-= (const CdMatrix3& m)
{
  m11 -= m.m11; m12 -= m.m12; m13 -= m.m13;
  m21 -= m.m21; m22 -= m.m22; m23 -= m.m23;
  m31 -= m.m31; m32 -= m.m32; m33 -= m.m33;
  return *this;
}

CdMatrix3& CdMatrix3::operator*= (const CdMatrix3& m)
{
  CdMatrix3 r;
  r.m11 = m11*m.m11 + m12*m.m21 + m13*m.m31;
  r.m12 = m11*m.m12 + m12*m.m22 + m13*m.m32;
  r.m13 = m11*m.m13 + m12*m.m23 + m13*m.m33;
  r.m21 = m21*m.m11 + m22*m.m21 + m23*m.m31;
  r.m22 = m21*m.m12 + m22*m.m22 + m23*m.m32;
  r.m23 = m21*m.m13 + m22*m.m23 + m23*m.m33;
  r.m31 = m31*m.m11 + m32*m.m21 + m33*m.m31;
  r.m32 = m31*m.m12 + m32*m.m22 + m33*m.m32;
  r.m33 = m31*m.m13 + m32*m.m23 + m33*m.m33;
  *this = r;
  return *this;
}

CdMatrix3& CdMatrix3::operator*= (double s)
{
  m11 *= s; m12 *= s; m13 *= s;
  m21 *= s; m22 *= s; m23 *= s;
  m31 *= s; m32 *= s; m33 *= s;
  return *this;
}

void CdMatrix3::Identity ()
{
  m12 = m13 = 0;
  m21 = m23 = 0;
  m31 = m32 = 0;
  m11 = m22 = m33 = 1;
}

void CdMatrix3::Transpose ()
{
  double swap;
  swap = m12; m12 = m21; m21 = swap;
  swap = m13; m13 = m31; m31 = swap;
  swap = m23; m23 = m32; m32 = swap;
}

CdMatrix3 CdMatrix3::GetTranspose () const
{
  CdMatrix3 t;
  t.m12 = m21; t.m21 = m12;
  t.m13 = m31; t.m31 = m13;
  t.m23 = m32; t.m32 = m23;
  t.m11 = m11; t.m22 = m22; t.m33 = m33;
  return t;
}

double CdMatrix3::Determinant () const
{
  return
    m11 * (m22*m33 - m23*m32)
   -m12 * (m21*m33 - m23*m31)
   +m13 * (m21*m32 - m22*m31);
}

CdMatrix3 operator+ (const CdMatrix3& m1, const CdMatrix3& m2)
{
  return CdMatrix3 (m1.m11+m2.m11, m1.m12+m2.m12, m1.m13+m2.m13,
                  m1.m21+m2.m21, m1.m22+m2.m22, m1.m23+m2.m23,
                  m1.m31+m2.m31, m1.m32+m2.m32, m1.m33+m2.m33);
}

CdMatrix3 operator- (const CdMatrix3& m1, const CdMatrix3& m2)
{
  return CdMatrix3 (m1.m11-m2.m11, m1.m12-m2.m12, m1.m13-m2.m13,
                  m1.m21-m2.m21, m1.m22-m2.m22, m1.m23-m2.m23,
                  m1.m31-m2.m31, m1.m32-m2.m32, m1.m33-m2.m33);
}
CdMatrix3 operator* (const CdMatrix3& m1, const CdMatrix3& m2)
{
  return CdMatrix3 (
   m1.m11*m2.m11 + m1.m12*m2.m21 + m1.m13*m2.m31,
   m1.m11*m2.m12 + m1.m12*m2.m22 + m1.m13*m2.m32,
   m1.m11*m2.m13 + m1.m12*m2.m23 + m1.m13*m2.m33,
   m1.m21*m2.m11 + m1.m22*m2.m21 + m1.m23*m2.m31,
   m1.m21*m2.m12 + m1.m22*m2.m22 + m1.m23*m2.m32,
   m1.m21*m2.m13 + m1.m22*m2.m23 + m1.m23*m2.m33,
   m1.m31*m2.m11 + m1.m32*m2.m21 + m1.m33*m2.m31,
   m1.m31*m2.m12 + m1.m32*m2.m22 + m1.m33*m2.m32,
   m1.m31*m2.m13 + m1.m32*m2.m23 + m1.m33*m2.m33 );
}

CdMatrix3 operator* (const CdMatrix3& m, double f)
{
  return CdMatrix3 (m.m11*f, m.m12*f, m.m13*f,
                  m.m21*f, m.m22*f, m.m23*f,
                  m.m31*f, m.m32*f, m.m33*f);
}

CdMatrix3 operator* (double f, const CdMatrix3& m)
{
  return CdMatrix3 (m.m11*f, m.m12*f, m.m13*f,
                  m.m21*f, m.m22*f, m.m23*f,
                  m.m31*f, m.m32*f, m.m33*f);
}

CdMatrix3 operator/ (const CdMatrix3& m, double f)
{
  return CdMatrix3 (m.m11/f, m.m12/f, m.m13/f,
                  m.m21/f, m.m22/f, m.m23/f,
                  m.m31/f, m.m32/f, m.m33/f);
}

bool operator== (const CdMatrix3& m1, const CdMatrix3& m2)
{
  if (m1.m11 != m2.m11 || m1.m12 != m2.m12 || m1.m13 != m2.m13) return false;
  if (m1.m21 != m2.m21 || m1.m22 != m2.m22 || m1.m23 != m2.m23) return false;
  if (m1.m31 != m2.m31 || m1.m32 != m2.m32 || m1.m33 != m2.m33) return false;
  return true;
}

bool operator!= (const CdMatrix3& m1, const CdMatrix3& m2)
{
  if (m1.m11 != m2.m11 || m1.m12 != m2.m12 || m1.m13 != m2.m13) return true;
  if (m1.m21 != m2.m21 || m1.m22 != m2.m22 || m1.m23 != m2.m23) return true;
  if (m1.m31 != m2.m31 || m1.m32 != m2.m32 || m1.m33 != m2.m33) return true;
  return false;
}

bool operator< (const CdMatrix3& m, double f)
{
  return ABS(m.m11)<f && ABS(m.m12)<f && ABS(m.m13)<f &&
         ABS(m.m21)<f && ABS(m.m22)<f && ABS(m.m23)<f &&
         ABS(m.m31)<f && ABS(m.m32)<f && ABS(m.m33)<f;
}

bool operator> (double f, const CdMatrix3& m)
{
  return ABS(m.m11)<f && ABS(m.m12)<f && ABS(m.m13)<f &&
         ABS(m.m21)<f && ABS(m.m22)<f && ABS(m.m23)<f &&
         ABS(m.m31)<f && ABS(m.m32)<f && ABS(m.m33)<f;
}


//---------------------------------------------------------------------------

void CdMath3::Between (const CdVector3& v1, const CdVector3& v2,
		       CdVector3& v, double pct, double wid)
{
  if (pct != -1)
    pct /= 100.;
  else
  {
    double dist = (double) sqrt((v1-v2)*(v1-v2));
    if (dist == 0) return;
    pct = wid / dist;
  }
  v = v1 + pct*(v2-v1);
}

bool CdMath3::Visible (const CdVector3& p, const CdVector3& t1,
		       const CdVector3& t2, const CdVector3& t3)
{
   double x1 = t1.x-p.x;
   double y1 = t1.y-p.y;
   double z1 = t1.z-p.z;
   double x2 = t2.x-p.x;
   double y2 = t2.y-p.y;
   double z2 = t2.z-p.z;
   double x3 = t3.x-p.x;
   double y3 = t3.y-p.y;
   double z3 = t3.z-p.z;
   double c = x3*((z1*y2)-(y1*z2))+
             y3*((x1*z2)-(z1*x2))+
             z3*((y1*x2)-(x1*y2));
   return c > 0;
}

bool CdMath3::PlanesClose (const CdPlane& p1, const CdPlane& p2)
{
  if (PlanesEqual (p1, p2)) return true;
  CdPlane p1n = p1; p1n.Normalize ();
  CdPlane p2n = p2; p2n.Normalize ();
  return PlanesEqual (p1n, p2n);
}

//---------------------------------------------------------------------------

double CdSquaredDist::PointLine (const CdVector3& p,
                           const CdVector3& l1, const CdVector3& l2)
{
  CdVector3 W = l1-p;
  CdVector3 L = l2-l1;
  CdVector3 p2l = W - L * (W*L)/(L*L);
  return p2l * p2l;
}

double CdSquaredDist::PointPoly (const CdVector3& p, CdVector3 *V, int n,
                          const CdPlane& plane, double sqdist)
{
  CdVector3 W, L;
  bool lflag = true, lflag0 = true;
  int i;
  for (i=0; i<n-1; i++)
  {
    W = V[i] - p;
    if (i==0)
    {
      if ( !(W*(V[n-1]-V[0]) > 0) ) lflag0 = false;
      else if (W*(V[1]-V[0]) > 0) return W*W;
      else lflag = false;
    }
    else if ( !(W*(L = V[i-1]-V[i]) > 0) )
    {
      if ( !lflag && W*(plane.norm % L) > 0 )
      {
        L = W - L * (W*L)/(L*L);
        return L*L;
      }
      lflag = (W*(V[i+1]-V[i]) > 0);
    }
    else if (W*(V[i+1]-V[i]) > 0) return W*W;
    else lflag = false;
  }

  W = V[n-1] - p;
  if (!lflag)
  {
    lflag = W * (L = V[n-2]-V[n-1]) <= 0;
    if ( lflag && (W*(plane.norm % L) > 0) )
    {
      L = W - L * (W*L)/(L*L);
      return L*L;
    }
  }
  if (!lflag0)
  {
    lflag0 = W * (L = V[0]-V[n-1]) <= 0;
    if ( lflag0 && (W*(plane.norm % L) < 0) )
    {
      L = W - L * (W*L)/(L*L);
      return L*L;
    }
  }

  if (!lflag && !lflag0) return W*W;
  if (sqdist >= 0) return sqdist;
  return CdSquaredDist::PointPlane (p,plane);
}

//---------------------------------------------------------------------------

void CdIntersect3::Plane(const CdVector3& u, const CdVector3& v,
                       const CdVector3& normal, const CdVector3& a,
                       CdVector3& isect)
{
  double counter = normal * (u - a);
  double divider = normal * (v - u);
  double dist;
  if (divider == 0) { isect = v; return; }
  dist = counter / divider;
  isect = u + dist*(u - v);
}

bool CdIntersect3::Plane(const CdVector3& u, const CdVector3& v,
                       double A, double B, double C, double D,
                       CdVector3& isect, double& dist)
{
  double x,y,z, denom;

  x = v.x-u.x;  y = v.y-u.y;  z = v.z-u.z;
  denom = A*x + B*y + C*z;
  if (ABS (denom) < SMALL_EPSILON) return false; // they are parallel

  dist = -(A*u.x + B*u.y + C*u.z + D) / denom;
  if (dist < -SMALL_EPSILON || dist > 1+SMALL_EPSILON) return false;

  isect.x = u.x + dist*x;  isect.y = u.y + dist*y;  isect.z = u.z + dist*z;
  return true;
}

bool CdIntersect3::Plane(const CdVector3& u, const CdVector3& v,
                       const CdPlane& p,
                       CdVector3& isect, double& dist)
{
  double x,y,z, denom;

  x = v.x-u.x;  y = v.y-u.y;  z = v.z-u.z;
  denom = p.norm.x*x + p.norm.y*y + p.norm.z*z;
  if (ABS (denom) < SMALL_EPSILON) return false; // they are parallel

  dist = -(p.norm*u + p.DD) / denom;
  if (dist < -SMALL_EPSILON || dist > 1+SMALL_EPSILON) return false;

  isect.x = u.x + dist*x;  isect.y = u.y + dist*y;  isect.z = u.z + dist*z;
  return true;
}

bool CdIntersect3::Planes(const CdPlane& p1, const CdPlane& p2,
                          const CdPlane& p3, CdVector3& isect)
{
  //To find the one point that is on all three planes, we need to solve
  //the following equation system (we need to find the x, y and z which
  //are true for all equations):
  // A1*x+B1*y+C1*z+D1=0 //plane1
  // A2*x+B2*y+C2*z+D2=0 //plane2
  // A3*x+B3*y+C3*z+D3=0 //plane3
  //This can be solved according to Cramers rule by looking at the
  //determinants of the equation system.
  CdMatrix3 mdet(p1.A(), p1.B(), p1.C(),
                 p2.A(), p2.B(), p2.C(),
                 p3.A(), p3.B(), p3.C());
  double det = mdet.Determinant();
  if (det == 0) return false; //some planes are parallel.

  CdMatrix3 mx(-p1.D(),  p1.B(),  p1.C(),
               -p2.D(),  p2.B(),  p2.C(),
               -p3.D(),  p3.B(),  p3.C());
  double xdet = mx.Determinant();

  CdMatrix3 my( p1.A(), -p1.D(),  p1.C(),
                p2.A(), -p2.D(),  p2.C(),
                p3.A(), -p3.D(),  p3.C());
  double ydet = my.Determinant();

  CdMatrix3 mz( p1.A(),  p1.B(), -p1.D(),
                p2.A(),  p2.B(), -p2.D(),
                p3.A(),  p3.B(), -p3.D());
  double zdet = mz.Determinant();

  isect.x = xdet/det;
  isect.y = ydet/det;
  isect.z = zdet/det;
  return true;
}


double CdIntersect3::Z0Plane(
  const CdVector3& u, const CdVector3& v, CdVector3& isect)
{
  double r = u.z / (u.z-v.z);
  isect.x = r * (v.x-u.x) + u.x;
  isect.y = r * (v.y-u.y) + u.y;
  isect.z = 0;
  return r;
}

double CdIntersect3::ZPlane(
  double zval, const CdVector3& u, const CdVector3& v, CdVector3& isect)
{
  double r = (zval-u.z) / (v.z-u.z);
  isect.x = r * (v.x-u.x) + u.x;
  isect.y = r * (v.y-u.y) + u.y;
  isect.z = zval;
  return r;
}

double CdIntersect3::XFrustum(
  double A, const CdVector3& u, const CdVector3& v, CdVector3& isect)
{
  double r = (A*u.x+u.z) / ( A*(u.x-v.x) + u.z-v.z );
  isect.x = r * (v.x-u.x) + u.x;
  isect.y = r * (v.y-u.y) + u.y;
  isect.z = r * (v.z-u.z) + u.z;
  return r;
}

double CdIntersect3::YFrustum(
  double B, const CdVector3& u, const CdVector3& v, CdVector3& isect)
{
  double r = (B*u.y+u.z) / ( B*(u.y-v.y) + u.z-v.z );
  isect.x = r * (v.x-u.x) + u.x;
  isect.y = r * (v.y-u.y) + u.y;
  isect.z = r * (v.z-u.z) + u.z;
  return r;
}

//---------------------------------------------------------------------------
