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

#ifndef __FRUSTRUM_H__
#define __FRUSTRUM_H__

#include "types.h"
#include "csgeom/math3d.h"

class csTransform;

/**
 * A frustrum. This consist of a center point (origin),
 * a frustrum polygon in 3D space (relative to center point)
 * and a plane. The planes which go through the center and
 * every edge of the polygon form the frustrum. The plane
 * is the back plane of the frustrum.
 * It is also possible to have an infinite frustrum in which
 * case the polygon will be NULL (not specified). The back
 * plane can also be NULL.
 */
class csFrustrum
{
private:
  csVector3 origin;

  csVector3* vertices;
  int num_vertices;
  int max_vertices;

  csPlane* backplane;

  /**
   * If true we have a total wide frustrum. A frustrum like
   * this does not have a polygon defining the planes but it can have
   * a back plane. The value of this boolean is only used if there
   * is no polygon. In other words, if the polygon is present in this
   * frustrum then 'wide' is simply ignored and can have any value.
   */
  bool wide;

  /**
   * If true we have a mirrored frustrum where the direction of
   * the polygon is reversed.
   */
  bool mirrored;

  void Clear ();

  void ExtendVertexArray (int num);

public:
  /// Create a new empty frustrum.
  csFrustrum (csVector3& o) : origin (o), vertices (NULL), num_vertices (0),
  	max_vertices (0), backplane (NULL), wide (false), mirrored (false) { }

  /**
   * Create a frustrum given a polygon and a backplane.
   * The polygon is given relative to the origin 'o'.
   * If the given polygon is NULL then we create an empty frustrum.
   */
  csFrustrum (csVector3& o, csVector3* verts, int num_verts, csPlane* backp = NULL);

  /// Copy constructor.
  csFrustrum (const csFrustrum &copy);

  ///
  virtual ~csFrustrum () { Clear (); }

  /// Set the origin of this frustrum.
  void SetOrigin (csVector3& o) { origin = o; }

  /// Get the origin of this frustrum.
  csVector3& GetOrigin () { return origin; }

  /// Enable/disable mirroring.
  void SetMirrored (bool m) { mirrored = m; }

  /// Is this frustrum mirrored?
  bool IsMirrored () { return mirrored; }

  /**
   * Set the back plane of this frustrum.
   * The given plane is copied to this structure and can thus
   * be reused/freed later. The plane should be specified relative
   * to the origin point.
   */
  void SetBackPlane (csPlane& plane);

  /**
   * Get the back plane.
   */
  csPlane* GetBackPlane () { return backplane; }

  /**
   * Remove the back plane of this frustrum.
   */
  void RemoveBackPlane ();

  /**
   * Add a vertex to the frustrum polygon.
   */
  void AddVertex (csVector3& v);

  /**
   * Get the number of vertices.
   */
  int GetNumVertices () { return num_vertices; }

  /**
   * Get a vertex.
   */
  csVector3& GetVertex (int idx) { return vertices[idx]; }

  /**
   * Get the array of vertices.
   */
  csVector3* GetVertices () { return vertices; }

  /**
   * Apply a transformation to this frustrum.
   */
  void Transform (csTransform* trans);

  /**
   * Clip this frustrum to the positive side of a plane
   * formed by the origin of this frustum, and the two given vertices.
   * 'v1' and 'v2' are given relative to that origin.
   */
  void ClipToPlane (csVector3& v1, csVector3& v2);

  /**
   * Clip the polygon of this frustrum to the postive side of an arbitrary plane
   * (which should be specified relative to the origin of the frustrum).
   * Note that this clips the polygon which forms the frustrum. It does
   * not clip the frustrum itself.
   */
  void ClipPolyToPlane (csPlane* plane);

  /**
   * Intersect with another frustrum. The other frustrum
   * must have the same origin as this one. Otherwise the
   * result is undefined.
   * Returns new frustrum which you should delete
   * after usage. If there is no intersection this function
   * returns NULL.
   */
  csFrustrum* Intersect (const csFrustrum& other);

  /**
   * Intersect a convex polygon with this volume. The convex polygon
   * is given relative to the center point (origin) of this frustrum.<p>
   *
   * Returns a new frustrum which exactly covers the intersection
   * of the polygon with the frustrum (i.e. the smallest frustrum
   * which is part of this frustrum and which 'sees' exactly
   * the same of the given polygon as this frustrum).<p>
   *
   * This function returns NULL if there is no intersection.<p>
   *
   * Note that the frustrum polygon of the returned csFrustrum is
   * guaranteed to be coplanar with the given polygon.
   */
  csFrustrum* Intersect (csVector3* poly, int num);

  /// Return true if frustrum is empty.
  bool IsEmpty () const { return !wide && vertices == NULL; }

  /// Return true if frustrum is infinite.
  bool IsInfinite () const { return wide && vertices == NULL && backplane == NULL; }

  /// Return true if frustrum is infinitely wide but it can still have a back plane.
  bool IsWide () const { return wide && vertices == NULL; }

  /**
   * Make the frustrum infinite (i.e. clear the polygon and
   * the back plane).
   */
  void MakeInfinite ();

  /**
   * Make the frustrum empty.
   */
  void MakeEmpty ();
};

#endif // __CSFRUSTRUM_H__
