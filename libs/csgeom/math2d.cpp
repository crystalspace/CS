/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein
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

#include <math.h>
#include "sysdef.h"
#include "csgeom/math2d.h"
#include "csgeom/box.h"
#include "csgeom/poly2d.h"

//---------------------------------------------------------------------------

// This algorithm assumes that the polygon is convex and that
// the vertices of the polygon are oriented in clockwise ordering.
// If this was not the case then the polygon should not be drawn (culled)
// and this routine would not be called for it.
int csMath2::InPoly2D (const csVector2& v, 
                       csVector2* P, int n, csBox* bounding_box)
{
  if (!bounding_box->In (v.x, v.y)) return CS_POLY_OUT;
  int i, i1;
  int side;
  i1 = n-1;
  for (i = 0 ; i < n ; i++)
  {
    // If this vertex is left of the polygon edge we are outside the polygon.
    side = WhichSide2D (v, P[i1], P[i]);
    if (side < 0) return CS_POLY_OUT;
    else if (side == 0) return CS_POLY_ON;
    i1 = i;
  }
  return CS_POLY_IN;
}

bool csMath2::PlanesClose (const csPlane2& p1, const csPlane2& p2)
{
  if (PlanesEqual (p1, p2)) return true;
  csPlane2 p1n = p1; p1n.Normalize ();
  csPlane2 p2n = p2; p2n.Normalize ();
  return PlanesEqual (p1n, p2n);
}

//---------------------------------------------------------------------------

float csVector2::Norm (const csVector2& v)
{ return sqrt (v*v); }

float csVector2::Norm () const
{ return sqrt (x*x+y*y); }

void csVector2::Rotate (float angle)
{
  float s = sin (angle);
  float c = cos (angle);
  float nx = x * c + y * s;
  y = -x * s + y * c;
  x = nx;
}

//---------------------------------------------------------------------------

csVector2 operator+ (const csVector2& v1, const csVector2& v2)
{ return csVector2(v1.x+v2.x, v1.y+v2.y); }

csVector2 operator- (const csVector2& v1, const csVector2& v2)
{ return csVector2(v1.x-v2.x, v1.y-v2.y); }

float operator* (const csVector2& v1, const csVector2& v2)
{ return v1.x*v2.x+v1.y*v2.y; }

csVector2 operator* (const csVector2& v, float f)
{ return csVector2(v.x*f, v.y*f); }

csVector2 operator* (float f, const csVector2& v)
{ return csVector2(v.x*f, v.y*f); }

csVector2 operator/ (const csVector2& v, float f)
{ return csVector2(v.x/f, v.y/f); }

bool operator== (const csVector2& v1, const csVector2& v2)
{ return (v1.x==v2.x) && (v1.y==v2.y); }

bool operator!= (const csVector2& v1, const csVector2& v2)
{ return (v1.x!=v2.x) || (v1.y!=v2.y); }

//---------------------------------------------------------------------------

bool csIntersect2::Segments (
  const csVector2& a1, const csVector2& a2, /* First segment */
  const csVector2& b1, const csVector2& b2, /* Second segment */
  csVector2& isect, float& dist) 
{
  float denom;
  float r, s;

//            (Ya1-Yb1)(Xb2-Xb1)-(Xa1-Xb1)(Yb2-Yb1)
//        r = -------------------------------------  (eqn 1)
//            (Xa2-Xa1)(Yb2-Yb1)-(Ya2-Ya1)(Xb2-Xb1)
// 
//            (Ya1-Yb1)(Xa2-Xa1)-(Xa1-Xb1)(Ya2-Ya1)
//        s = -------------------------------------  (eqn 2)
//            (Xa2-Xa1)(Yb2-Yb1)-(Ya2-Ya1)(Xb2-Xb1)

  denom = (a2.x-a1.x)*(b2.y-b1.y) - (a2.y-a1.y)*(b2.x-b1.x);
  if (ABS (denom) < EPSILON) return false;

  r = ((a1.y-b1.y)*(b2.x-b1.x) - (a1.x-b1.x)*(b2.y-b1.y)) / denom;
  s = ((a1.y-b1.y)*(a2.x-a1.x) - (a1.x-b1.x)*(a2.y-a1.y)) / denom;
  dist = r;

  if ( ( r < -SMALL_EPSILON || r > 1+SMALL_EPSILON ) ||
       ( s < -SMALL_EPSILON || s > 1+SMALL_EPSILON ) ) return false;

  isect.x = a1.x + dist*(a2.x-a1.x);
  isect.y = a1.y + dist*(a2.y-a1.y);

  return true;
}

bool csIntersect2::SegmentLine (
  const csVector2& a1, const csVector2& a2, /* segment */
  const csVector2& b1, const csVector2& b2, /* line */
  csVector2& isect, float& dist) 
{
  float denom;

  denom = (a2.x-a1.x)*(b2.y-b1.y) - (a2.y-a1.y)*(b2.x-b1.x);
  if (ABS (denom) < EPSILON) return false;

  dist = ((a1.y-b1.y)*(b2.x-b1.x) - (a1.x-b1.x)*(b2.y-b1.y)) / denom;
  if ( dist < -SMALL_EPSILON || dist > 1+SMALL_EPSILON ) return false;

  isect.x = a1.x + dist*(a2.x-a1.x);
  isect.y = a1.y + dist*(a2.y-a1.y);

  return true;
}

bool csIntersect2::Lines (
  const csVector2& a1, const csVector2& a2, /* First line */
  const csVector2& b1, const csVector2& b2, /* Second line */
  csVector2& isect) 
{
  float denom, r;

  denom = (a2.x-a1.x)*(b2.y-b1.y) - (a2.y-a1.y)*(b2.x-b1.x);
  if (ABS (denom) < EPSILON) return false;

  r = ((a1.y-b1.y)*(b2.x-b1.x) - (a1.x-b1.x)*(b2.y-b1.y)) / denom;

  isect.x = a1.x + r*(a2.x-a1.x);
  isect.y = a1.y + r*(a2.y-a1.y);

  return true;
}

bool csIntersect2::Plane(const csVector2& u, const csVector2& v,
                         const csPlane2& p, csVector2& isect, float& dist)
{
  float x,y, denom;

  x = v.x-u.x;  y = v.y-u.y;
  denom = p.norm.x*x + p.norm.y*y;
  if (ABS (denom) < SMALL_EPSILON) return false; // they are parallel

  dist = -(p.norm*u + p.CC) / denom;
  if (dist < -SMALL_EPSILON || dist > 1+SMALL_EPSILON) return false;

  isect.x = u.x + dist*x;  isect.y = u.y + dist*y;
  return true;
}

//---------------------------------------------------------------------------

bool csPlane2::IntersectPolygon (csPoly2D* poly, csVector2& v1, csVector2& v2)
{
  int i, i1;
  float c, c1;
  csVector2 isect;
  float dist;
  i1 = poly->GetNumVertices ()-1;
  c1 = Classify ((*poly)[i1]);
  bool found_v1 = false;
  bool found_v2 = false;
  for (i = 0 ; i < poly->GetNumVertices () ; i++)
  {
    c = Classify ((*poly)[i]);
    if ((c < 0 && c1 > 0) || (c1 < 0 && c > 0))
    {
      csIntersect2::Plane ((*poly)[i1], (*poly)[i],
      	  *this, isect, dist);
      if (!found_v1) { v1 = isect; found_v1 = true; }
      else { v2 = isect; found_v2 = true; break; }
    }

    i1 = i;
    c1 = c;
  }
  if (!found_v1) return false;
  if (!found_v2) v2 = v1;
  return true;
}

//---------------------------------------------------------------------------

csMatrix2::csMatrix2 () 
{
  m11 = m22 = 1;
  m12 = m21 = 0;
}

csMatrix2::csMatrix2 (float m11, float m12,
  	    	      float m21, float m22)
{
  csMatrix2::m11 = m11;
  csMatrix2::m12 = m12;
  csMatrix2::m21 = m21;
  csMatrix2::m22 = m22;
}

csMatrix2& csMatrix2::operator+= (const csMatrix2& m)
{
  m11 += m.m11; m12 += m.m12;
  m21 += m.m21; m22 += m.m22;
  return *this;
}

csMatrix2& csMatrix2::operator-= (const csMatrix2& m)
{
  m11 -= m.m11; m12 -= m.m12;
  m21 -= m.m21; m22 -= m.m22;
  return *this;
}

csMatrix2& csMatrix2::operator*= (const csMatrix2& m)
{
  csMatrix2 r (*this);
  m11 = r.m11*m.m11 + r.m12*m.m21;
  m12 = r.m11*m.m12 + r.m12*m.m22;
  m21 = r.m21*m.m11 + r.m22*m.m21;
  m22 = r.m21*m.m12 + r.m22*m.m22;
  return *this;
}

csMatrix2& csMatrix2::operator*= (float s)
{
  m11 *= s; m12 *= s;
  m21 *= s; m22 *= s;
  return *this;
}

void csMatrix2::Identity ()
{
  m11 = m22 = 1;
  m12 = m21 = 0;
}

void csMatrix2::Transpose ()
{
  float swap = m12; m12 = m21; m21 = swap;
}

csMatrix2 csMatrix2::GetTranspose () const
{
  return csMatrix2 (m11, m21, m12, m22);
}

float csMatrix2::Determinant () const
{
  return (m11 * m22 - m12 * m21);
}

csMatrix2 operator+ (const csMatrix2& m1, const csMatrix2& m2) 
{
  return csMatrix2 (m1.m11+m2.m11, m1.m12+m2.m12,
                    m1.m21+m2.m21, m1.m22+m2.m22);
}
                  
csMatrix2 operator- (const csMatrix2& m1, const csMatrix2& m2)
{
  return csMatrix2 (m1.m11-m2.m11, m1.m12-m2.m12,
                    m1.m21-m2.m21, m1.m22-m2.m22);
}
csMatrix2 operator* (const csMatrix2& m1, const csMatrix2& m2)
{
  return csMatrix2 (m1.m11*m2.m11 + m1.m12*m2.m21,
                    m1.m11*m2.m12 + m1.m12*m2.m22,
                    m1.m21*m2.m11 + m1.m22*m2.m21,
                    m1.m21*m2.m12 + m1.m22*m2.m22);
}

csMatrix2 operator* (const csMatrix2& m, float f)
{
  return csMatrix2 (m.m11*f, m.m12*f,
                    m.m21*f, m.m22*f);
}

csMatrix2 operator* (float f, const csMatrix2& m)
{
  return csMatrix2 (m.m11*f, m.m12*f,
                    m.m21*f, m.m22*f);
}

csMatrix2 operator/ (const csMatrix2& m, float f)
{
  float inv_f = 1 / f;
  return csMatrix2 (m.m11*inv_f, m.m12*inv_f,
                    m.m21*inv_f, m.m22*inv_f);
}

//---------------------------------------------------------------------------
