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

#ifndef __CS_MATH3D_H__
#define __CS_MATH3D_H__

#ifndef __CS_CSSYSDEFS_H__
#error "cssysdef.h must be included in EVERY source file!"
#endif

#include "csgeom/vector3.h"
#include "csgeom/plane3.h"
#include "csgeom/plane2.h"
#include "csgeom/segment.h"

class csDVector3;
class csPoly3D;
class csBox3;

inline float fSqr (float f)
{
  return f * f;
}

/**
 * Various assorted 3D mathematical functions.
 * This is a static class and contains only static member functions.
 */
class csMath3
{
public:
  /**
   * Tests which side of a plane the given 3D point is on.
   * Return -1 if point p is left of plane '0-v1-v2',
   *         1 if point p is right of plane '0-v1-v2',
   *      or 0 if point p lies on plane '0-v1-v2'.
   * Plane '0-v1-v2' is the plane passing through points <0,0,0>, v1, and v2.
   */
//@@@ Warning: This function fails if the three points are co-planar with
// origin <0,0,0>. Be warned that you should check this before using this
// function. One method is that since you have a plane that you used to check
// the vertices, add the normal of the plane to the three vertices. The normal
// is the only vector that can garuntee that the vertices will be shifted away
// from origin, and yield a valid result.

  static int WhichSide3D (const csVector3& p,
                          const csVector3& v1, const csVector3& v2)
  {
//    float s = p * (v1%v2);  (original expression: expanded to the below:)
    float s = p.x*(v1.y*v2.z-v1.z*v2.y) + p.y*(v1.z*v2.x-v1.x*v2.z) + 
              p.z*(v1.x*v2.y-v1.y*v2.x);
    if (s < 0) return 1;
    else if (s > 0) return -1;
    else return 0;
  }

  /**
   * Tests if the front face of a triangle is visible from the given point.
   * Visibility test (backface culling) to see if the triangle formed by
   * t1, t2, and t3 is visible from point p.
   */
  static bool Visible (const csVector3& p, const csVector3& t1,
                       const csVector3& t2, const csVector3& t3);

  /**
   * Check if the plane is visible from the given point.
   * This function does a back-face culling test to see whether the front
   * face of plane pl is visible from point p.
   */
  static bool Visible (const csVector3& p, const csPlane3& pl)
  { return pl.Classify (p) <= 0; }

  /**
   * Calculate the line, that is the result of the Intersection of 
   * triangle 1  and triangle 2. This Method returns false, if there is
   * no intersection. If there is an intersection, the start of the line
   * is in line[0] and the end of the line is in line[1] and the method
   * return true;
   */
  static bool FindIntersection(const csVector3  tri1[3], 
                               const csVector3  tri2[3],
                               csVector3        line[2]);

  /**
   * Calculates a vector lying a specified distance between two other vectors.
   * Given vectors v1 and v2, this function will calculate and return vector
   * v lying between them.
   * If pct != -1, vector v will be the point which is pct % of the
   * way from v1 to v2.
   * Otherwise, if pct equals -1, v will be the point along 'v1-v2' which is
   * distance wid from v1.
   */
  static void Between (const csVector3& v1, const csVector3& v2, csVector3& v,
                       float pct, float wid);

  /**
   * Set the min and max vector if this vector exceeds their current limits.
   * This function will check each component of vector v against the maximum
   * and minimum values specified by min and max.  If the limits are
   * exceeded, new min or max values will be set.
   */
  static void SetMinMax (const csVector3& v,
                         csVector3& min, csVector3& max)
  {
    if (v.x > max.x) max.x = v.x; else if (v.x < min.x ) min.x = v.x;
    if (v.y > max.y) max.y = v.y; else if (v.y < min.y ) min.y = v.y;
    if (v.z > max.z) max.z = v.z; else if (v.z < min.z ) min.z = v.z;
  }

  /**
   * Compute twice the signed area of triangle composed by three points.
   * This function returns 2 x the area of the triangle formed by the points
   * a, b, and c.
   */
  inline static float Area3 (const csVector3 &a, const csVector3 &b,
                             const csVector3 &c)
  {
    csVector3 v1 = b - a;
    csVector3 v2 = c - a;
    return ((v1.y * v2.z + v1.z * v2.x + v1.x * v2.y) -
            (v1.y * v2.x + v1.x * v2.z + v1.z * v2.y));
  }

  /**
   * Calculate a plane normal given three vectors.
   * This function will calculate the normal to the plane formed by vectors
   * v1, v2, and v3, and store the result in norm.
   */
  inline static void CalcNormal (csVector3& norm,     const csVector3& v1,
                                 const csVector3& v2, const csVector3& v3)
  {
    norm = (v1-v2)%(v1-v3);
  }

  /**
   * Compute the normal given two (u,v) vectors.
   * This function will calculate the normal to a polygon with two edges
   * represented by v and u.  The result is stored in norm.
   */
  static void CalcNormal (csVector3& norm,
                          const csVector3& v, const csVector3& u)
  { norm = u%v; /* NOT v%u - vertexes are defined clockwise */ }

  /**
   * Calculate the plane equation given three vectors.
   * Given three vectors v1, v2, and v3, forming a plane, this function
   * will calculate the plane equation and return the result in 'normal'
   * and 'D'.
   */
  static void CalcPlane (const csVector3& v1, const csVector3& v2,
         const csVector3& v3, csVector3& normal, float& D)
  {
    CalcNormal (normal, v1, v2, v3);
    D = - (normal * v1);
  }

  /**
   * Check if two planes are almost equal.
   * The function returns true iff each component of the plane equation for
   * one plane is within .001 of the corresponding component of the other
   * plane.
   */
  static bool PlanesEqual (const csPlane3& p1, const csPlane3& p2)
  {
    return ( ( p1.norm - p2.norm) < (float).001 ) &&
             (  ABS (p1.DD-p2.DD) < (float).001 );
  }

  /**
   * Check if two planes are close together.
   * Two planes are close if there are almost equal OR if
   * the normalized versions are almost equal.
   */
  static bool PlanesClose (const csPlane3& p1, const csPlane3& p2);

  /**
   * Calculate the set of outer planes between the two boxes. Is something
   * does not intersect this set of planes then it will not be between
   * the two boxes. The given array of planes should have place for at
   * least eight planes. This function returns the number of planes
   * that are put in 'planes'.
   */
  static int OuterPlanes (const csBox3& box1, const csBox3& box2,
    csPlane3* planes);

  /**
   * Find all observer sides on the first box that can see the
   * other box. Sides are numbered like this: 0=MinX(), 1=MaxX(),
   * 2=MinY(), 3=MaxY(), 4=MinZ(), 5=MaxZ().
   * The given array should have place for 6 sides.
   * This function returns the number of observer sides.
   */
  static int FindObserverSides (const csBox3& box1, const csBox3& box2,
  	int* sides);
};

/**
 * Some functions to perform squared distance calculations.
 * This is a static class and contains only static member functions.
 */
class csSquaredDist
{
public:
  /// Returns the squared distance between two points.
  static float PointPoint (const csVector3& p1, const csVector3& p2)
  { return fSqr (p1.x - p2.x) + fSqr (p1.y - p2.y) + fSqr (p1.z - p2.z); }

  /// Returns the squared distance between a point and a line.
  static float PointLine (const csVector3& p,
                          const csVector3& l1, const csVector3& l2);

  /// Returns the squared distance between a point and a normalized plane.
  static float PointPlane (const csVector3& p, const csPlane3& plane)
  { float r = plane.Classify (p);  return r * r; }

  /**
   * Returns the squared distance between a point and a polygon.
   * If sqdist is >= 0, then it is used as the pre-calculated point to
   * plane distance.  V is an array of vertices, n is the number of
   * vertices, and plane is the polygon plane.
   */
  static float PointPoly (const csVector3& p, csVector3 *V, int n,
                          const csPlane3& plane, float sqdist = -1);
};

/**
 * Some functions to perform various intersection calculations with 3D
 * line segments.  This is a static class and contains only static member
 * functions.
 */
class csIntersect3
{
public:
  /**
   * Intersect a plane with a 3D polygon and return
   * the line segment corresponding with this intersection.
   * Returns true if there is an intersection. If false
   * then 'segment' will not be valid.
   */
  static bool IntersectPolygon (const csPlane3& plane, csPoly3D* poly,
  	csSegment3& segment);

  /**
   * Intersect a segment with a frustum (given as a set of planes).
   * Returns the clipped segment (i.e. the part of the segment that is
   * visible in the frustum). Returns -1 if the segment is entirely
   * outside the frustum. Returns 0 if the segment is not modified and
   * returns 1 otherwise. The input segment will be modified.
   * @@@ WARNING! This function may not work completely ok. It has only
   * barely been tested and is now unused.
   */
  static int IntersectSegment (csPlane3* planes, int num_planes,
  	csSegment3& seg);

  /**
   * Intersect a 3D segment with a triangle. Returns true if there
   * is an intersection. In that case the intersection point will
   * be in 'isect'.
   */
  static bool IntersectTriangle (const csVector3& tr1,
  	const csVector3& tr2, const csVector3& tr3,
	const csSegment3& seg, csVector3& isect);

  /**
   * Intersect a 3D segment with a plane.  Returns true if there is an
   * intersection, with the intersection point returned in isect.
   */
  static void Plane (
    const csVector3& u, const csVector3& v,
    const csVector3& normal, const csVector3& a, // plane
    csVector3& isect, float& dist);              // intersection point

  /**
   * Intersect a 3D segment with a plane.  Returns true if there is an
   * intersection, with the intersection point returned in isect.
   * The distance from u to the intersection point is returned in dist.
   * The distance that is returned is a normalized distance with respect
   * to the given input vector. i.e. a distance of 0.5 means that the
   * intersection point is halfway u and v.
   */
  static bool Plane (
    const csVector3& u, const csVector3& v,
    const csPlane3& p,                     // plane Ax+By+Cz+D=0
    csVector3& isect,                     // intersection point
    float& dist);                       // distance from u to isect

  /**
   * Intersect 3 planes to get the point that is part of all three
   * planes. Returns true, if there is a single point that fits.
   * If some planes are parallel, then it will return false.
   */
  static bool Planes (const csPlane3& p1, const csPlane3& p2,
  	const csPlane3& p3, csVector3& isect);

  /**
   * Intersect a regular plane and an axis aligned plane and
   * return the intersection (line) as a 2D plane. This intersection
   * is defined on the axis aligned plane.
   * Returns false if there is no intersection.
   */
  static bool PlaneXPlane (const csPlane3& p1, float x2, csPlane2& isect);

  /**
   * Intersect a regular plane and an axis aligned plane and
   * return the intersection (line) as a 2D plane. This intersection
   * is defined on the axis aligned plane.
   * Returns false if there is no intersection.
   */
  static bool PlaneYPlane (const csPlane3& p1, float y2, csPlane2& isect);

  /**
   * Intersect a regular plane and an axis aligned plane and
   * return the intersection (line) as a 2D plane. This intersection
   * is defined on the axis aligned plane.
   * Returns false if there is no intersection.
   */
  static bool PlaneZPlane (const csPlane3& p1, float z2, csPlane2& isect);

  /**
   * Intersect a regular plane and an axis aligned plane and
   * return the intersection (line) as a 2D plane. This intersection
   * is defined on the axis aligned plane.
   * Returns false if there is no intersection.
   */
  static bool PlaneAxisPlane (const csPlane3& p1, int nr, float pos,
  	csPlane2& isect)
  {
    switch (nr)
    {
      case 0: return PlaneXPlane (p1, pos, isect); 
      case 1: return PlaneYPlane (p1, pos, isect); 
      case 2: return PlaneZPlane (p1, pos, isect); 
    }
    return false;
  }

  /**
   * Intersect a 3D segment with the z = 0 plane.  Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float Z0Plane (
    const csVector3& v1, const csVector3& v2,
    csVector3& isect);                    // intersection point

  /**
   * Intersect a 3D segment with the z = 0 plane.  Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float Z0Plane (
    const csSegment3& uv,
    csVector3& isect)                    // intersection point
  {
    return Z0Plane (uv.Start (), uv.End (), isect);
  }

  /**
   * Intersect a 3D segment with the plane z = zval. Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float ZPlane (float zval,      // plane z = zval
    const csVector3& u, const csVector3& v,
    csVector3& isect);                    // intersection point

  /**
   * Intersect a 3D segment with the plane z = zval. Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float ZPlane (float zval,      // plane z = zval
    const csSegment3& uv,
    csVector3& isect)                     // intersection point
  {
    return ZPlane (zval, uv.Start (), uv.End (), isect);
  }

  /**
   * Intersect a 3D segment with the frustum plane Ax + z = 0.
   * Assumes an intersection, and returns the intersection point in isect.
   */
  static float XFrustum (
    float A, const csVector3& u, const csVector3& v, csVector3& isect);

  /**
   * Intersect a 3D segment with the frustum plane Ax + z = 0.
   * Assumes an intersection, and returns the intersection point in isect.
   */
  static float XFrustum (
    float A, const csSegment3& uv, csVector3& isect)
  {
    return XFrustum (A, uv.Start (), uv.End (), isect);
  }

  /**
   * Intersect a 3D segment with the frustum plane By + z = 0.
   * Assumes an intersection, and returns the intersection point in isect.
   */
  static float YFrustum (
    float B, const csVector3& u, const csVector3& v, csVector3& isect);

  /**
   * Intersect a 3D segment with the frustum plane By + z = 0.
   * Assumes an intersection, and returns the intersection point in isect.
   */
  static float YFrustum (
    float B, const csSegment3& uv, csVector3& isect)
  {
    return YFrustum (B, uv.Start (), uv.End (), isect);
  }

  /**
   * Intersect a segment with a box and returns true if it intersects.
   * The intersection point is also returned.
   * If 'pr' is given then a number between 0 and 1 is returned which
   * corresponds to the position on the segment. If we were in the box
   * this this function will also return true. In this case 'isect' will
   * be set to the start of the segment and *pr to 0.
   */
  static bool BoxSegment (const csBox3& box, const csSegment3& segment,
  	csVector3& isect, float* pr = NULL);
};

#endif // __CS_MATH3D_H__
