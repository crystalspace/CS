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

#ifndef POLYINT_H
#define POLYINT_H

#include "csgeom/math3d.h"

// Values returned by classify.
#define POL_SAME_PLANE 0
#define POL_FRONT 1
#define POL_BACK 2
#define POL_SPLIT_NEEDED 3

class csPolygonInt;

/**
 * The 'interface' class for the parent of a set of polygons.
 * This class is used by the BSP tree. If a class inherits from
 * this interface (using multiple inheritance if needed) then
 * it can be used by the BSP tree as a valid parent for polygons.
 */
class csPolygonParentInt
{
public:
  /// Add a polygon.
  virtual void AddPolygon (csPolygonInt* p) = 0;

  /// Get the number of polygons.
  virtual int GetNumPolygons () = 0;

  /// Get a polygon with the index.
  virtual csPolygonInt* GetPolygon (int num) = 0;
};

/**
 * This class indicates what methods a class should use in order
 * to be a 'polygon'. It acts as an 'interface' in JAVA terminology.
 * There is no data in this class and no method implementations.<p>
 *
 * The BSP tree implementation is an example of a class that
 * uses this csPolygonInt interface. The consequence of this is that
 * the BSP tree can be used for several sorts of polygons (even
 * 3D or 2D ones).<p>
 *
 * This class exports methods in three categories:
 * <ul>
 * <li>Polygon manipulation (clone, set_parent, get_parent)
 * <li>Vertex manipulation (reset, add_vertex, finish, get_num_vertices)
 * <li>Plane functions (classify, same_plane, get_poly_plane, split_with_plane)
 * </ul>
 */
class csPolygonInt
{
public:
  /**
   * Clone this polygon and return a new (almost)
   * identical copy. The vertices must not be cloned (the resulting
   * polygon should be empty) and the polygon should not be linked
   * to the parent, but all other attributes must be copied.
   */
  virtual csPolygonInt* Clone () = 0;

  /**
   * Set the parent of this polygon.
   */
  virtual void SetParent (csPolygonParentInt* parent) = 0;

  /**
   * Get the parent of this polygon.
   */
  virtual csPolygonParentInt* GetParent () = 0;

  /**
   * Reset the polygon (remove all vertices).
   */
  virtual void Reset () = 0;

  /**
   * Add a vertex from the container to the polygon and return
   * the index (starting with 0) of the added vertex.
   */
  virtual int AddVertex (const csVector3& v) = 0;

  /**
   * Finish adding vertices.
   */
  virtual void Finish () = 0;

  /**
   * Return the current number of vertices in this polygon.
   */
  virtual int GetNumVertices () = 0;

  /**
   * Return the plane of this polygon.
   */
  virtual csPlane* GetPolyPlane () = 0;

  /**
   * Classify a polygon with regards to this one. If the poly is on same
   * plane as this one it returns POL_SAME_PLANE. If this poly is
   * completely in front of the given poly it returnes POL_FRONT. If this poly
   * is completely back of the given poly it returnes POL_BACK. Otherwise it
   * returns POL_SPLIT_NEEDED.
   */
  virtual int Classify (csPolygonInt* poly) = 0;

  /**
   * Split this polygon with the given plane (A,B,C,D) and put the
   * 'front' polygon in the 'front' array and the 'back' polygon in the
   * 'back' array.
   */
  virtual void SplitWithPlane (csVector3* front, int& front_n, csVector3* back, int& back_n, csPlane& plane) = 0;

  /**
   * Return true if this polygon and the given polygon are on the same
   * plane. If their planes are shared this is automatically the case.
   * Otherwise this function will check their respective plane equations
   * to test for equality.
   */
  virtual bool SamePlane (csPolygonInt* p) = 0;
};

#endif /*POLYINT_H*/
