/*
    Copyright (C) 1998 by Jorrit Tyberghein
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

//---------------------------------------------------------------------------

int csMath2::WhichSide2D (const csVector2& v, 
                          const csVector2& s1, const csVector2& s2)
{
  float k  = (s1.y - v.y)*(s2.x - s1.x);
  float k1 = (s1.x - v.x)*(s2.y - s1.y);

  if (k < k1) return -1;
  else if (k > k1) return 1;
  else return 0;
}

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

//---------------------------------------------------------------------------

