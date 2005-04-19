/*
    Copyright (C) 1998-2005 by Jorrit Tyberghein
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

/**\file 
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "csextern.h"

#include "csgeom/vector3.h"
#include "csgeom/plane3.h"
#include "csgeom/plane2.h"
#include "csgeom/segment.h"
#include "csgeom/box.h"
#include "csgeom/frustum.h"
#include "iutil/dbghelp.h"

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
class CS_CRYSTALSPACE_EXPORT csMath3
{
public:
  /**
   * Tests which side of a plane the given 3D point is on.
   * Return -1 if point p is left of plane '0-v1-v2',
   *         1 if point p is right of plane '0-v1-v2',
   *      or 0 if point p lies on plane '0-v1-v2'.
   * Plane '0-v1-v2' is the plane passing through points <0,0,0>, v1, and v2.
   *<p>
   * Warning: the result of this function when 'p' is exactly on the plane
   * 0-v1-v2 is undefined. It should return 0 but it will not often do that
   * due to numerical inaccuracies. So you should probably test for this
   * case separately.
   */
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
   * Compute twice the area of triangle composed by three points.
   * This function returns 2 x the area of the triangle formed by the points
   * a, b, and c.
   */
  inline static float DoubleArea3 (const csVector3 &a, const csVector3 &b,
                             const csVector3 &c)
  {
    csVector3 v1 = b - a;
    csVector3 v2 = c - a;
    return (v1 % v2).Norm ();
  }

  /**
   * Returns < 0 or > 0 depending on the direction of the triangle.
   */
  inline static float Direction3 (const csVector3 &a, const csVector3 &b,
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

  /**
   * Given two angles in radians, calculate the position on the sphere
   * around (0,0,0) with radius 1. The first angle is the angle along
   * the horizontal (x/z) plane. The second angle is the vertical angle.
   */
  static void SpherePosition (float angle_xz, float angle_vert,
  	csVector3& pos);
};

/**
 * Some functions to perform squared distance calculations.
 * This is a static class and contains only static member functions.
 */
class CS_CRYSTALSPACE_EXPORT csSquaredDist
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
class CS_CRYSTALSPACE_EXPORT csIntersect3
{
private:
  static bool BoxPlaneInternal (const csVector3& normal,
	const csVector3& vert, const csVector3& boxhalfsize);

public:
  /**
   * Intersect a plane with a 3D polygon and return
   * the line segment corresponding with this intersection.
   * Returns true if there is an intersection. If false
   * then 'segment' will not be valid.
   */
  static bool PlanePolygon (const csPlane3& plane, csPoly3D* poly,
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
  static int SegmentFrustum (csPlane3* planes, int num_planes,
  	csSegment3& seg);

  /**
   * Intersect a 3D segment with a triangle. Returns true if there
   * is an intersection. In that case the intersection point will
   * be in 'isect'.
   */
  static bool SegmentTriangle (const csSegment3& seg,
  	const csVector3& tr1,
  	const csVector3& tr2, const csVector3& tr3,
	csVector3& isect);

  /**
   * Intersect a 3D segment with a polygon. Returns true if there
   * is an intersection. In that case the intersection point will
   * be in 'isect'. Note that this function doesn't do
   * backface culling.
   */
  static bool SegmentPolygon (const csSegment3& seg, const csPoly3D& poly,
  	const csPlane3& poly_plane, csVector3& isect);

  /**
   * If a number of planes enclose a convex space (with their normals
   * pointing outwards).
   * This method returns true if they are intersected by a segment.
   * isect contains the closest intersection point.
   * dist contains the distance to that point (with distance between u and v
   * being 1)
   */
  static bool SegmentPlanes (
     const csVector3& u, const csVector3& v,
     const csPlane3* planes, int length,
     csVector3& isect, float& dist);

  /**
   * Intersect a 3D segment with a plane.  Returns true if there is an
   * intersection, with the intersection point returned in isect.
   */
  static bool SegmentPlane (
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
   * There are two cases in which this method will return false:
   * -  If the plane and the segment are parallel, then 'dist' will be set equal
   * to 0, and 'isect' equal to 'v'.
   * -  If the segment does not cross the plane (i.e. if 'dist'>1+epsilon or
   * 'dist'<-epsilon, where epsilon is a very small value near to zero) then
   * 'isect's value is (0, 0, 0).
   * \remarks
   * 'p' is the plane, expressed as: A x + B y + C z + D = 0 , where (A,B,C) is 
   * the normal vector of the plane.
   * 'u' and 'v' are the start (U point) and the end (V point) of the segment.
   * 'isect' is searched along the segment U + x (V - U); the unknown 'x' value
   * is got by: x = [(A,B,C) * U + D ] / (A,B,C) * (U - V), where * is the dot 
   * product.  
   */
  static bool SegmentPlane (
    const csVector3& u, const csVector3& v,
    const csPlane3& p,                     // plane Ax+By+Cz+D=0
    csVector3& isect,                     // intersection point
    float& dist);                       // distance from u to isect

  /**
   * Intersect 3 planes to get the point that is part of all three
   * planes. Returns true, if there is a single point that fits.
   * If some planes are parallel, then it will return false.
   */
  static bool ThreePlanes (const csPlane3& p1, const csPlane3& p2,
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
  static float SegmentZ0Plane (
    const csVector3& v1, const csVector3& v2,
    csVector3& isect);                    // intersection point

  /**
   * Intersect a 3D segment with the z = 0 plane.  Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float SegmentZ0Plane (
    const csSegment3& uv,
    csVector3& isect)                    // intersection point
  {
    return SegmentZ0Plane (uv.Start (), uv.End (), isect);
  }

  /**
   * Intersect a 3D segment with the plane x = xval. Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float SegmentXPlane (
    const csVector3& u, const csVector3& v,
    float xval,
    csVector3& isect);                    // intersection point

  /**
   * Intersect a 3D segment with the plane x = xval. Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float SegmentXPlane (
    const csSegment3& uv,
    float xval,
    csVector3& isect)                     // intersection point
  {
    return SegmentXPlane (uv.Start (), uv.End (), xval, isect);
  }

  /**
   * Intersect a 3D segment with the plane y = yval. Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float SegmentYPlane (
    const csVector3& u, const csVector3& v,
    float yval,      // plane y = yval
    csVector3& isect);                    // intersection point

  /**
   * Intersect a 3D segment with the plane y = yval. Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float SegmentYPlane (
    const csSegment3& uv,
    float yval,      // plane y = yval
    csVector3& isect)                     // intersection point
  {
    return SegmentYPlane (uv.Start (), uv.End (), yval, isect);
  }

  /**
   * Intersect a 3D segment with the plane z = zval. Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float SegmentZPlane (
    const csVector3& u, const csVector3& v,
    float zval,      // plane z = zval
    csVector3& isect);                    // intersection point

  /**
   * Intersect a 3D segment with the plane z = zval. Assumes that there
   * is an intersection (fails if the segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float SegmentZPlane (
    const csSegment3& uv,
    float zval,      // plane z = zval
    csVector3& isect)                     // intersection point
  {
    return SegmentZPlane (uv.Start (), uv.End (), zval, isect);
  }

  /**
   * Intersect a 3D segment with an axis aligned plane and
   * return the intersection (fails if segment is parallel to the plane),
   * and returns the distance from u to the intersection point.
   * The intersection point is returned in isect.
   */
  static float SegmentAxisPlane (const csVector3& u, const csVector3& v,
  	int nr, float pos, csVector3& isect)
  {
    switch (nr)
    {
      case 0: return SegmentXPlane (u, v, pos, isect);
      case 1: return SegmentYPlane (u, v, pos, isect);
      case 2: return SegmentZPlane (u, v, pos, isect);
    }
    return 0.0;
  }

  /**
   * Intersect a 3D segment with the frustum plane Ax + z = 0.
   * Assumes an intersection, and returns the intersection point in isect.
   */
  static float SegmentXFrustum (
    const csVector3& u, const csVector3& v, float A, csVector3& isect);

  /**
   * Intersect a 3D segment with the frustum plane Ax + z = 0.
   * Assumes an intersection, and returns the intersection point in isect.
   */
  static float SegmentXFrustum (
    const csSegment3& uv, float A, csVector3& isect)
  {
    return SegmentXFrustum (uv.Start (), uv.End (), A, isect);
  }

  /**
   * Intersect a 3D segment with the frustum plane By + z = 0.
   * Assumes an intersection, and returns the intersection point in isect.
   */
  static float SegmentYFrustum (
    const csVector3& u, const csVector3& v, float B, csVector3& isect);

  /**
   * Intersect a 3D segment with the frustum plane By + z = 0.
   * Assumes an intersection, and returns the intersection point in isect.
   */
  static float SegmentYFrustum (
    const csSegment3& uv, float B, csVector3& isect)
  {
    return SegmentYFrustum (uv.Start (), uv.End (), B, isect);
  }

  /**
   * Intersect a segment with a box and returns one of CS_BOX_SIDE_... if it
   * intersects, CS_BOX_INSIDE if inside, or -1 otherwise.
   * The intersection point is also returned.
   * If 'pr' is given then a number between 0 and 1 is returned which
   * corresponds to the position on the segment. If we were in the box
   * this this function will return CS_BOX_INSIDE. In this case 'isect' will
   * be set to the start of the segment and *pr to 0.
   */
  static int BoxSegment (const csBox3& box, const csSegment3& segment,
  	csVector3& isect, float* pr = 0);

  /**
   * Intersect an AABB with a frustum. The frustum may contain up to
   * 32 planes. Active planes are defined using the 'inClipMask'. It will
   * return true if AABB is visible in frustum. If the AABB intersects
   * with the frustum then 'outClipMask' will contain the mask for all planes
   * intersecting with the AABB. This can be used as 'inClipMask' for subsequent
   * frustum tests with children of the AABB (i.e. other AABB inside
   * this AABB).
   */
  static bool BoxFrustum (const csBox3& box, csPlane3* frustum,
  	uint32 inClipMask, uint32& outClipMask);

  /**
   * Test if a box intersects with a sphere. The intersection is not
   * computed. The sphere is given with squared radius.
   */
  static bool BoxSphere (const csBox3& box, const csVector3& center,
		  float sqradius);

  /**
   * Test if a plane intersects with a box.
   */
  static bool BoxPlane (const csBox3& box, const csPlane3& plane);

  /**
   * Test if a plane intersects with a box.
   * 'vert' is one point on the plane.
   */
  static bool BoxPlane (const csBox3& box, const csVector3& normal,
  	const csVector3& vert);

  /**
   * Test if a triangle intersects with a box.
   */
  static bool BoxTriangle (const csBox3& box,
	const csVector3& tri0, const csVector3& tri1, const csVector3& tri2);

  /**
   * Test if two boxes intersect.
   */
  static bool BoxBox (const csBox3& box1, const csBox3& box2)
  {
    return box1.TestIntersect (box2);
  }

  /**
   * Calculate intersection of two frustums.
   */
  static csPtr<csFrustum> FrustumFrustum (const csFrustum& f1,
  	const csFrustum& f2)
  {
    return f1.Intersect (f2);
  }

  /**
   * Calculate intersection of two frustums.
   */
  static csPtr<csFrustum> FrustumFrustum (const csFrustum& f1,
  	csVector3* poly, int num)
  {
    return f1.Intersect (poly, num);
  }

  /**
   * Test intersection between two triangles.
   * @param tri1 Vertices of triangle 1
   * @param tri2 Vertices of triangle 2
   * @return true if the triangles intersect, otherwise false
   */
  static bool TriangleTriangle (const csVector3 tri1[3],
			 const csVector3 tri2[3]);

  /**
   * Calculate intersection between two triangles and return it
   * in isectline.
   * @param tri1 Vertices of triangle 1
   * @param tri2 Vertices of triangle 2
   * @param[out] isectline The line segment where they intersect
   * @param[out] coplanar Returns whether the triangles are coplanar
   * @return true if the triangles intersect, otherwise false
   */
  static bool TriangleTriangle (const csVector3 tri1[3],
			 const csVector3 tri2[3],
			 csSegment3& isectline, bool& coplanar);
};

/**
 * This is a class that does unit testing (and other debug stuff) for most
 * of csgeom classes.
 */
class CS_CRYSTALSPACE_EXPORT csGeomDebugHelper : public iDebugHelper
{
public:
  csGeomDebugHelper ();
  virtual ~csGeomDebugHelper ();

  SCF_DECLARE_IBASE;
  virtual int GetSupportedTests () const
  {
    return CS_DBGHELP_UNITTEST;
  }
  virtual csPtr<iString> UnitTest ();
  virtual csPtr<iString> StateTest ()
  {
    return 0;
  }
  virtual csTicks Benchmark (int /*num_iterations*/)
  {
    return 0;
  }
  virtual csPtr<iString> Dump ()
  {
    return 0;
  }
  virtual void Dump (iGraphics3D* /*g3d*/)
  {
  }
  virtual bool DebugCommand (const char*)
  {
    return false;
  }
};

/** @} */

#endif // __CS_MATH3D_H__

