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

#include "cssysdef.h"
#include <math.h>
#include "csgeom/math2d.h"
#include "csgeom/box.h"
#include "csgeom/poly2d.h"

//---------------------------------------------------------------------------

// This algorithm assumes that the polygon is convex and that
// the vertices of the polygon are oriented in clockwise ordering.
// If this was not the case then the polygon should not be drawn (culled)
// and this routine would not be called for it.
int csMath2::InPoly2D (
  const csVector2 &v,
  csVector2 *P,
  int n,
  csBox2 *bounding_box)
{
  if (!bounding_box->In (v.x, v.y)) return CS_POLY_OUT;

  int i, i1;
  int side;
  i1 = n - 1;
  for (i = 0; i < n; i++)
  {
    // If this vertex is left of the polygon edge we are outside the polygon.
    side = WhichSide2D (v, P[i1], P[i]);
    if (side < 0)
      return CS_POLY_OUT;
    else if (side == 0)
      return CS_POLY_ON;
    i1 = i;
  }

  return CS_POLY_IN;
}

bool csMath2::PlanesClose (const csPlane2 &p1, const csPlane2 &p2)
{
  if (PlanesEqual (p1, p2)) return true;

  csPlane2 p1n = p1;
  p1n.Normalize ();

  csPlane2 p2n = p2;
  p2n.Normalize ();
  return PlanesEqual (p1n, p2n);
}

//---------------------------------------------------------------------------
bool csIntersect2::PlanePolygon (
  const csPlane2 &plane,
  csPoly2D *poly,
  csSegment2 &seg)
{
  csVector2 &v1 = seg.Start ();
  csVector2 &v2 = seg.End ();
  int i, i1;
  float c, c1;
  csVector2 isect;
  float dist;
  i1 = poly->GetVertexCount () - 1;
  c1 = plane.Classify ((*poly)[i1]);

  bool found_v1 = false;
  bool found_v2 = false;
  for (i = 0; i < poly->GetVertexCount (); i++)
  {
    c = plane.Classify ((*poly)[i]);
    if ((c < 0 && c1 > 0) || (c1 < 0 && c > 0))
    {
      csIntersect2::SegmentPlane ((*poly)[i1], (*poly)[i], plane, isect, dist);
      if (!found_v1)
      {
        v1 = isect;
        found_v1 = true;
      }
      else
      {
        v2 = isect;
        found_v2 = true;
        break;
      }
    }

    i1 = i;
    c1 = c;
  }

  if (!found_v1) return false;
  if (!found_v2) v2 = v1;
  return true;
}

bool csIntersect2::SegmentSegment (
  const csSegment2 &a,
  const csSegment2 &b,
  csVector2 &isect,
  float &dist)
{
  float denom;
  float r, s;
  const csVector2 &a1 = a.Start ();
  const csVector2 &a2 = a.End ();
  const csVector2 &b1 = b.Start ();
  const csVector2 &b2 = b.End ();

  //            (Ya1-Yb1)(Xb2-Xb1)-(Xa1-Xb1)(Yb2-Yb1)
  //        r = -------------------------------------  (eqn 1)
  //            (Xa2-Xa1)(Yb2-Yb1)-(Ya2-Ya1)(Xb2-Xb1)
  //
  //            (Ya1-Yb1)(Xa2-Xa1)-(Xa1-Xb1)(Ya2-Ya1)
  //        s = -------------------------------------  (eqn 2)
  //            (Xa2-Xa1)(Yb2-Yb1)-(Ya2-Ya1)(Xb2-Xb1)
  denom = (a2.x - a1.x) * (b2.y - b1.y) - (a2.y - a1.y) * (b2.x - b1.x);
  if (ABS (denom) < EPSILON) return false;

  r = ((a1.y - b1.y) * (b2.x - b1.x) - (a1.x - b1.x) * (b2.y - b1.y)) / denom;
  s = ((a1.y - b1.y) * (a2.x - a1.x) - (a1.x - b1.x) * (a2.y - a1.y)) / denom;
  dist = r;

  if (
    (r < -SMALL_EPSILON || r > 1 + SMALL_EPSILON) ||
    (s < -SMALL_EPSILON || s > 1 + SMALL_EPSILON))
    return false;

  isect.x = a1.x + dist * (a2.x - a1.x);
  isect.y = a1.y + dist * (a2.y - a1.y);

  return true;
}

bool csIntersect2::SegmentLine (
  const csSegment2 &a,
  const csSegment2 &b,
  csVector2 &isect,
  float &dist)
{
  float denom;
  const csVector2 &a1 = a.Start ();
  const csVector2 &a2 = a.End ();
  const csVector2 &b1 = b.Start ();
  const csVector2 &b2 = b.End ();

  denom = (a2.x - a1.x) * (b2.y - b1.y) - (a2.y - a1.y) * (b2.x - b1.x);
  if (ABS (denom) < EPSILON) return false;

  dist = ((a1.y - b1.y) * (b2.x - b1.x) - (a1.x - b1.x) * (b2.y - b1.y))
  	/ denom;
  if (dist < -SMALL_EPSILON || dist > 1 + SMALL_EPSILON) return false;

  isect.x = a1.x + dist * (a2.x - a1.x);
  isect.y = a1.y + dist * (a2.y - a1.y);

  return true;
}

bool csIntersect2::LineLine (
  const csSegment2 &a,
  const csSegment2 &b,
  csVector2 &isect)
{
  float denom, r;
  const csVector2 &a1 = a.Start ();
  const csVector2 &a2 = a.End ();
  const csVector2 &b1 = b.Start ();
  const csVector2 &b2 = b.End ();

  denom = (a2.x - a1.x) * (b2.y - b1.y) - (a2.y - a1.y) * (b2.x - b1.x);
  if (ABS (denom) < EPSILON) return false;

  r = ((a1.y - b1.y) * (b2.x - b1.x) - (a1.x - b1.x) * (b2.y - b1.y)) / denom;

  isect.x = a1.x + r * (a2.x - a1.x);
  isect.y = a1.y + r * (a2.y - a1.y);

  return true;
}

bool csIntersect2::SegmentPlane (
  const csVector2 &u,
  const csVector2 &v,
  const csPlane2 &p,
  csVector2 &isect,
  float &dist)
{
  float x, y, denom;

  x = v.x - u.x;
  y = v.y - u.y;
  denom = p.norm.x * x + p.norm.y * y;
  if (ABS (denom) < SMALL_EPSILON) return false;  // they are parallel
  dist = -(p.norm * u + p.CC) / denom;
  if (dist < -SMALL_EPSILON || dist > 1 + SMALL_EPSILON) return false;

  isect.x = u.x + dist * x;
  isect.y = u.y + dist * y;
  return true;
}

bool csIntersect2::PlanePlane (
  const csPlane2 &p1,
  const csPlane2 &p2,
  csVector2 &isect)
{
  // p1: A1x+B1y+C1 = 0

  // p2: A2x+B2y+C2 = 0
  csVector2 start1, end1;
  csVector2 start2, end2;
  if (ABS (p1.A ()) < SMALL_EPSILON)
  {
    // Horizontal line.
    start1.Set (0, -p1.C () / p1.B ());
    end1.Set (1, -p1.C () / p1.B ());
  }
  else if (ABS (p1.B ()) < SMALL_EPSILON)
  {
    // Vertical line.
    start1.Set (-p1.C () / p1.A (), 0);
    end1.Set (-p1.C () / p1.A (), 1);
  }
  else
  {
    start1.Set (0, -p1.C () / p1.B ());
    end1.Set (1, (-p1.C () - p1.A ()) / p1.B ());
  }

  if (ABS (p2.A ()) < SMALL_EPSILON)
  {
    // Horizontal line.
    start2.Set (0, -p2.C () / p2.B ());
    end2.Set (1, -p2.C () / p2.B ());
  }
  else if (ABS (p2.B ()) < SMALL_EPSILON)
  {
    // Vertical line.
    start2.Set (-p2.C () / p2.A (), 0);
    end2.Set (-p2.C () / p2.A (), 1);
  }
  else
  {
    start2.Set (0, -p2.C () / p2.B ());
    end2.Set (1, (-p2.C () - p2.A ()) / p2.B ());
  }

  return LineLine (csSegment2 (start1, end1), csSegment2 (start2, end2), isect);

#if 0
  //@@@NOT SURE THAT THIS ROUTINE IS RIGHT AND OPTIMAL
  if (ABS (p1.B ()) < SMALL_EPSILON && ABS (p2.B ()) < SMALL_EPSILON)
    return false;
  if (ABS (p1.A ()) > ABS (p2.A ()))
  {
    isect.y = (p2.A () * p1.C () / p1.A () - p2.C ()) / (p2.B () - p2.A () * p1.B () / p1.A ());
    isect.x = -(p1.B () * isect.y + p1.C ()) / p1.A ();
  }
  else if (ABS (p2.A ()) > SMALL_EPSILON)
  {
    isect.y = (p1.A () * p2.C () / p2.A () - p1.C ()) / (p1.B () - p1.A () * p2.B () / p2.A ());
    isect.x = -(p2.B () * isect.y + p2.C ()) / p2.A ();
  }
  else
    return false; // parallel
  return true;
#endif
}

//---------------------------------------------------------------------------
