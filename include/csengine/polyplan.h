/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef __CS_POLYPLANE_H__
#define __CS_POLYPLANE_H__

#include "csgeom/math3d.h"
#include "csgeom/transfrm.h"

class Dumper;

/**
 * This class represents a polygon plane.
 * Never 'delete' an instance of this class directly
 * but instead call 'DecRef()'.
 */
class csPolyPlane
{
  ///
  friend class csPolygon2D;
  ///
  friend class csPolyTexture;
  ///
  friend class Dumper;

private:
  /// The object space plane equation (this is fixed).
  csPlane3 plane_obj;
  /// The world space plane equation.
  csPlane3 plane_wor;
  /// The camera space plane equation.
  csPlane3 plane_cam;
  /// Reference count.
  int ref_count;

  /// Destructor is private.
  virtual ~csPolyPlane ();

public:
  /// Constructor. Reference count is initialized to 1.
  csPolyPlane ();

  /// Maintain a reference counter for texture type objects
  void IncRef () { ref_count++; }
  /// Decrement usage counter
  void DecRef () { if (!--ref_count) delete this; }

  /**
   * Transform this plane from object space to world space using
   * the given matrices. One vertex on the plane is also given so
   * that we can more easily recompute the 'D' component of the plane.
   * The given vertex should be in world space.
   */
  void ObjectToWorld (
  	const csReversibleTransform& obj,
  	const csVector3& vertex1);

  /**
   * Transform this plane from world space to camera space using
   * the given matrices. One vertex on the plane is also given so
   * that we can more easily recompute the 'D' component of the plane.
   * The given vertex should be in camera space.
   */
  void WorldToCamera (
  	const csReversibleTransform& t,
	const csVector3& vertex1);

  /**
   * Return the minimum squared distance from the plane to
   * a point in 3D space (using world coordinates).
   */
  float SquaredDistance (csVector3& v) const
  {
    float d = plane_wor.Distance (v);
    return d*d;
  }

  /**
   * Check if this plane is nearly equal to another one in
   * world space coordinates (it only checks on the component
   * values. The planes are not normalized).
   */
  bool NearlyEqual (csPolyPlane* plane) const
  {
    return csMath3::PlanesEqual (plane_wor, plane->plane_wor);
  }

  /**
   * Return the closest point on the plane to a point
   * in 3D space (using world coordinates).
   */
  void ClosestPoint (csVector3& v, csVector3& isect) const;

  /**
   * Run a segment through this plane (in world space) and
   * see where it intersects. Put value in 'pr' between 0 and 1 for
   * position on segment (0 is at start, 1 is at end) and also
   * put intersection point in 'isect'. Return false if there is
   * no intersection.
   */
  bool IntersectSegment (const csVector3& start, const csVector3& end, 
                          csVector3& isect, float* pr) const;

  /**
   * Get the object version of the plane.
   */
  csPlane3& GetObjectPlane () { return plane_obj; }

  /**
   * Get the object version of the plane.
   */
  const csPlane3& GetObjectPlane () const { return plane_obj; }

  /**
   * Get the world version of the plane.
   */
  csPlane3& GetWorldPlane () { return plane_wor; }

  /**
   * Get the world version of the plane.
   */
  const csPlane3& GetWorldPlane () const { return plane_wor; }

  /**
   * Get the camera version of the plane.
   */
  csPlane3& GetCameraPlane () { return plane_cam; }

  /**
   * Get the camera version of the plane.
   */
  const csPlane3& GetCameraPlane () const { return plane_cam; }

  /**
   * Get the normal in object space.
   */
  void GetObjectNormal (float* p_A, float* p_B, float* p_C, float* p_D) const;

  /**
   * Get the normal in world space.
   */
  void GetWorldNormal (float* p_A, float* p_B, float* p_C, float* p_D) const;

  /**
   * Get the normal in camera space.
   */
  void GetCameraNormal (float* p_A, float* p_B, float* p_C, float* p_D) const;
};

#endif // __CS_POLYPLANE_H__
