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
   * Split this polygon with the given plane (A,B,C,D) and return the
   * two resulting new polygons in 'front' and 'back'. The new polygons will
   * mimic the behaviour of the parent polygon as good as possible.
   * This function is mainly used by the BSP splitter.
   */
  virtual void SplitWithPlane (csPolygonInt** front, csPolygonInt** back,
  	csPlane& plane) = 0;

  /**
   * Return true if this polygon and the given polygon are on the same
   * plane. If their planes are shared this is automatically the case.
   * Otherwise this function will check their respective plane equations
   * to test for equality.
   */
  virtual bool SamePlane (csPolygonInt* p) = 0;

  /**
   * Return some type-id which BSP visitors can use for their
   * own purpose. The purpose of this is to allow several different
   * types of polygons to be added to the same tree. With this number
   * you can recognize them.
   */
  virtual int GetType () = 0;
};

#endif /*POLYINT_H*/
