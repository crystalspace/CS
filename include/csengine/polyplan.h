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

#ifndef POLYPLANE_H
#define POLYPLANE_H

#include "csgeom/transfrm.h"
#include "csobject/csobject.h"

class Dumper;

/**
 * This class represents a texture plane. This is a plane
 * that defines the orientation and offset of a texture. It can
 * be used by several polygons to let the textures fit perfectly.
 */
class csPolyPlane : public csObject
{
  ///
  friend class csPolygon2D;
  ///
  friend class csPolyTexture;
  ///
  friend class Dumper;

private:
  /// The object space plane equation (this is fixed).
  csPlane plane_obj;
  /// The world space plane equation.
  csPlane plane_wor;
  /// The camera space plane equation.
  csPlane plane_cam;

  /// Transformation from object to texture space.
  csMatrix3 m_obj2tex;
  /// Translation from object to texture space.
  csVector3 v_obj2tex;

  /// Transformation from world to texture space.
  csMatrix3 m_world2tex;
  /// Translation from world to texture space.
  csVector3 v_world2tex;

  /**
   * Transformed texture space transformation. This transforms a
   * coordinate from camera space to texture space. This transformation
   * is obtained by using m_world2tex and v_world2tex with the camera transformation.
   */
  csMatrix3 m_cam2tex;
  ///
  csVector3 v_cam2tex;

public:
  ///
  csPolyPlane ();
  ///
  virtual ~csPolyPlane ();

  /**
   * Transform this plane from object space to world space using
   * the given matrices. One vertex on the plane is also given so
   * that we can more easily recompute the 'D' component of the plane.
   * The given vertex should be in world space.
   */
  void ObjectToWorld (const csReversibleTransform& obj, csVector3& vertex1);

  /**
   * Transform the same way from world space to camera space.
   * The given vertex should be in camera space.
   */
  void WorldToCamera (const csReversibleTransform& t, csVector3& vertex1);

  ///
  void SetTextureSpace (csVector3& v_orig,
  			  csVector3& v1, float len1,
  			  csVector3& v2, float len2);
  ///
  void SetTextureSpace (float xo, float yo, float zo,
  			  float x1, float y1, float z1,
  			  float len);
  ///
  void SetTextureSpace (csVector3& v_orig, csVector3& v1, float len);
  ///
  void SetTextureSpace (csVector3& v_orig, csVector3& v_u, csVector3& v_v);
  ///
  void SetTextureSpace (float xo, float yo, float zo,
  			  float xu, float yu, float zu,
  			  float xv, float yv, float zv);
  ///
  void SetTextureSpace (float xo, float yo, float zo,
  			  float xu, float yu, float zu,
  			  float xv, float yv, float zv,
  			  float xw, float yw, float zw);
  ///
  void SetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector);

  /// Get the transformation from object to texture space.
  void GetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector);

  /**
   * Check if the plane in world space is visible from the given point.
   * To do this it will only do back-face culling.
   */
  bool VisibleFromPoint (const csVector3& p)
  {
    return csMath3::Visible (p, plane_wor);
  }

  /**
   * Return the minimum squared distance from the plane to
   * a point in 3D space (using world coordinates).
   */
  float SquaredDistance (csVector3& v) { float d = Distance (v); return d*d; }

  /**
   * Return the minimum distance from the plane to
   * a point in 3D space (using world coordinates).
   */
  float Distance (csVector3& v)
  {
    // The normal is normalized so...
    return plane_wor.Distance (v);
  }

  /**
   * Check if this plane is nearly equal to another one in
   * world space coordinates (it only checks on the component
   * values. The planes are not normalized).
   */
  bool NearlyEqual (csPolyPlane* plane)
  {
    return csMath3::PlanesEqual (plane_wor, plane->plane_wor);
  }

  /**
   * Classify a vector with regards to this plane in world space.
   */
  float Classify (const csVector3& pt)
  {
    return plane_wor.Classify (pt);
  }

  /**
   * Return the closest point on the plane to a point
   * in 3D space (using world coordinates).
   */
  void ClosestPoint (csVector3& v, csVector3& isect);

  /**
   * Run a segment through this plane (in world space) and
   * see where it intersects. Put value in 'pr' between 0 and 1 for
   * position on segment (0 is at start, 1 is at end) and also
   * put intersection point in 'isect'. Return false if there is
   * no intersection.
   */
  bool IntersectSegment (const csVector3& start, const csVector3& end, 
                          csVector3& isect, float* pr);

  /**
   * Get the object version of the plane.
   */
  csPlane& GetObjectPlane () { return plane_obj; }

  /**
   * Get the world version of the plane.
   */
  csPlane& GetWorldPlane () { return plane_wor; }

  /**
   * Get the camera version of the plane.
   */
  csPlane& GetCameraPlane () { return plane_cam; }

  /**
   * Get the normal in object space.
   */
  void GetObjectNormal (float* p_A, float* p_B, float* p_C, float* p_D);

  /**
   * Get the normal in world space.
   */
  void GetWorldNormal (float* p_A, float* p_B, float* p_C, float* p_D);

  /**
   * Get the normal in camera space.
   */
  void GetCameraNormal (float* p_A, float* p_B, float* p_C, float* p_D);

  CSOBJTYPE;
};

#endif /*POLYPLANE_H*/
