/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
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

#ifndef __CS_MATH2D_H__
#define __CS_MATH2D_H__

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

// These are also defined in plane2.h
#ifndef __CS_POLY_MACROS__
#define __CS_POLY_MACROS__
#define CS_POLY_IN 1
#define CS_POLY_ON 0
#define CS_POLY_OUT -1
#endif

#include "csextern.h"

#include "csgeom/vector2.h"
#include "csgeom/plane2.h"
#include "csgeom/segment.h"

class csBox2;
class csPoly2D;

/**
 * Various functions in 2D, such as 2D vector functions.
 * This is a static class and contains only static member functions.
 */
class CS_CSGEOM_EXPORT csMath2
{
public:
  /**
   * Calculates which side of a line a given point is on.
   * Returns -1 if point v is left of line segment 's1-s2',
   *          1 if point v is right of segment 's1-s2'
   *       or 0 if point v lies on segment 's1-s2'.
   */
  static int WhichSide2D (const csVector2& v,
                          const csVector2& s1, const csVector2& s2)
  {
    float k  = (s1.y - v.y)*(s2.x - s1.x);
    float k1 = (s1.x - v.x)*(s2.y - s1.y);
    if (k < k1) return -1;
    else if (k > k1) return 1;
    else return 0;
  }

  /**
   * Calculates which side of a line a given point is on.
   * Returns -1 if point v is left of line segment 'seg'
   *          1 if point v is right of segment 'seg'
   *       or 0 if point v lies on segment 'seg'.
   */
  static int WhichSide2D (const csVector2& v,
                          const csSegment2& s)
  {
    return WhichSide2D (v, s.Start (), s.End ());
  }

  /**
   * Calculates whether a vector lies inside a given 2D polygon.
   * Return CS_POLY_IN, CS_POLY_OUT, or CS_POLY_ON for this vector with
   * respect to the given polygon. The polygon is given as an array of 2D
   * vectors with a bounding box.
   * WARNING: does no safety checking for P or bounding_box.
   */
  static int InPoly2D (const csVector2& v,
                       csVector2* P, int n, csBox2* bounding_box);

  /**
   * Calculates 2 x the area of a given triangle.
   * Returns twice the signed area of the triangle determined by a,b,c,
   * positive if a,b,c are oriented ccw, and negative if cw.
   */
  static float Area2 (const csVector2& a,
  		      const csVector2& b,
  		      const csVector2& c)
  {
    return
      a.x * b.y - a.y * b.x +
      a.y * c.x - a.x * c.y +
      b.x * c.y - c.x * b.y;
  }

  /**
   * Calculates whether a point lies to the right of a given line.
   * Returns true iff c is strictly to the right of the directed
   * line through a to b.
   */
  static float Right (const csVector2& a,
  		      const csVector2& b,
  		      const csVector2& c)
  {
    return Area2 (a, b, c) <= -SMALL_EPSILON;
  }

  /**
   * Calculates whether a point lies to the left of a given line.
   * Returns true iff c is strictly to the left of the directed
   * line through a to b.
   */
  static float Left (const csVector2& a,
  		     const csVector2& b,
  		     const csVector2& c)
  {
    return Area2 (a, b, c) >= SMALL_EPSILON;
  }

  /**
   * Check if the plane is visible from the given point.
   * This function does a back-face culling test to see whether the front
   * face of plane pl is visible from point p.
   */
  static bool Visible (const csVector2& p, const csPlane2& pl)
  { return pl.Classify (p) <= 0; }

  /**
   * Check if two planes are almost equal.
   * The function returns true iff each component of the plane equation for
   * one plane is within .001 of the corresponding component of the other
   * plane.
   */
  static bool PlanesEqual (const csPlane2& p1, const csPlane2& p2)
  {
    return ( ( p1.norm - p2.norm) < (float).001 ) &&
             (  ABS (p1.CC-p2.CC) < (float).001 );
  }

  /**
   * Check if two planes are close together.
   * Two planes are close if there are almost equal OR if
   * the normalized versions are almost equal.
   */
  static bool PlanesClose (const csPlane2& p1, const csPlane2& p2);
};

/**
 * Some functions to perform various intersection calculations with 2D
 * line segments.  This is a static class and contains only static member
 * functions.
 */
class CS_CSGEOM_EXPORT csIntersect2
{
public:
  /**
   * Intersect a plane with a 2D polygon and return
   * the line segment corresponding with this intersection.
   * Returns true if there is an intersection. If false
   * then 'segment' will not be valid.
   */
  static bool PlanePolygon (const csPlane2& plane, csPoly2D* poly,
  	csSegment2& segment);

  /**
   * Compute the intersection of the 2D segments.  Return true if they
   * intersect, with the intersection point returned in isect,  and the
   * distance from a1 of the intersection in dist.
   */
  static bool SegmentSegment (
    const csSegment2& a, const csSegment2& b,	// Two segments.
    csVector2& isect, float& dist);         // intersection point and distance

  /**
   * Compute the intersection of a 2D segment and a line.  Return true if they
   * intersect, with the intersection point returned in isect,  and the
   * distance from a1 of the intersection in dist.
   */
  static bool SegmentLine (
    const csSegment2& a,		// First segment.
    const csSegment2& b,		// A line (end is only direction)
    csVector2& isect, float& dist);     // intersection point and distance

  /**
   * Compute the intersection of 2D lines.  Return true if they
   * intersect, with the intersection point returned in isect.
   */
  static bool LineLine (
    // Two lines (end is only direction).
    const csSegment2& a, const csSegment2& b,
    csVector2& isect);                      // intersection point

  /**
   * Intersect a 2D segment with a plane.  Returns true if there is an
   * intersection, with the intersection point returned in isect.
   * The distance from u to the intersection point is returned in dist.
   * The distance that is returned is a normalized distance with respect
   * to the given input vector. i.e. a distance of 0.5 means that the
   * intersection point is halfway u and v.
   */
  static bool SegmentPlane (
    const csVector2& u, const csVector2& v,
    const csPlane2& p,                     // plane Ax+By+Cz+D=0
    csVector2& isect,                     // intersection point
    float& dist);                       // distance from u to isect

  /**
   * Intersect a 2D segment with a plane.  Returns true if there is an
   * intersection, with the intersection point returned in isect.
   * The distance from u to the intersection point is returned in dist.
   * The distance that is returned is a normalized distance with respect
   * to the given input vector. i.e. a distance of 0.5 means that the
   * intersection point is halfway u and v.
   */
  static bool SegmentPlane (
    const csSegment2& uv,	// Segment.
    const csPlane2& p,                     // plane Ax+By+Cz+D=0
    csVector2& isect,                     // intersection point
    float& dist)                        // distance from u to isect
  {
    return SegmentPlane (uv.Start (), uv.End (), p, isect, dist);
  }

  /**
   * Return the intersection point. This version does not test if
   * there really is an intersection. It just assumes there is one.
   */
  static void SegmentPlaneNoTest (const csVector2& u, const csVector2& v,
                     const csPlane2& p, csVector2& isect, float& dist)
  {
    float x,y, denom;
    x = v.x-u.x;  y = v.y-u.y;
    denom = p.norm.x*x + p.norm.y*y;
    dist = -(p.norm*u + p.CC) / denom;
    isect.x = u.x + dist*x;  isect.y = u.y + dist*y;
  }

  /**
   * Return the intersection point. This version does not test if
   * there really is an intersection. It just assumes there is one.
   */
  static void SegmentPlaneNoTest (const csSegment2& uv,
                     const csPlane2& p, csVector2& isect, float& dist)
  {
    SegmentPlaneNoTest (uv.Start (), uv.End (), p, isect, dist);
  }

  /**
   * Intersect 2 planes to get the point that is part of all two
   * planes. Returns true, if there is a single point that fits.
   * If the planes are parallel, then it will return false.
   */
  static bool PlanePlane (const csPlane2& p1, const csPlane2& p2,
                      csVector2& isect);

};

/** @} */

#endif // __CS_MATH2D_H__
