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
 * 3D or 2D ones). The csOctree class also uses csPolygonInt.
 */
class csPolygonInt
{
public:
  /**
   * Return the plane of this polygon.
   */
  virtual csPlane* GetPolyPlane () = 0;

  /**
   * Classify this polygon with regards to a plane (in world space). If this poly
   * is on same plane it returns POL_SAME_PLANE. If this poly is
   * completely in front of the given plane it returnes POL_FRONT. If this poly
   * is completely back of the given plane it returnes POL_BACK. Otherwise it
   * returns POL_SPLIT_NEEDED.
   */
  virtual int Classify (const csPlane& pl) = 0;

  /**
   * Classify to X plane. The default implementation just calls Classify()
   * above with a constructed plane but this function can be overridden
   * for more efficiency.
   */
  virtual int ClassifyX (float x) { return Classify (csPlane (1, 0, 0, -x)); }

  /**
   * Classify to Y plane. The default implementation just calls Classify()
   * above with a constructed plane but this function can be overridden
   * for more efficiency.
   */
  virtual int ClassifyY (float y) { return Classify (csPlane (0, 1, 0, -y)); }

  /**
   * Classify to Z plane. The default implementation just calls Classify()
   * above with a constructed plane but this function can be overridden
   * for more efficiency.
   */
  virtual int ClassifyZ (float z) { return Classify (csPlane (0, 0, 1, -z)); }

  /**
   * Split this polygon with the given plane (A,B,C,D) and return the
   * two resulting new polygons in 'front' and 'back'. The new polygons will
   * mimic the behaviour of the parent polygon as good as possible.
   * Note that the 'front' should be the negative side of the plane
   * and 'back' the positive side.
   */
  virtual void SplitWithPlane (csPolygonInt** front, csPolygonInt** back,
  	const csPlane& plane) = 0;

  /**
   * Split this polygon with the X plane. Default implementation just
   * calls SplitWithPlane() with a constructed plane but this function
   * can be overridden for more efficiency.
   */
  virtual void SplitWithPlaneX (csPolygonInt** front, csPolygonInt** back,
  	float x)
  {
    SplitWithPlane (front, back, csPlane (1, 0, 0, -x));
  }

  /**
   * Split this polygon with the Y plane. Default implementation just
   * calls SplitWithPlane() with a constructed plane but this function
   * can be overridden for more efficiency.
   */
  virtual void SplitWithPlaneY (csPolygonInt** front, csPolygonInt** back,
  	float y)
  {
    SplitWithPlane (front, back, csPlane (0, 1, 0, -y));
  }

  /**
   * Split this polygon with the Z plane. Default implementation just
   * calls SplitWithPlane() with a constructed plane but this function
   * can be overridden for more efficiency.
   */
  virtual void SplitWithPlaneZ (csPolygonInt** front, csPolygonInt** back,
  	float z)
  {
    SplitWithPlane (front, back, csPlane (0, 0, 1, -z));
  }

  /**
   * Return some type-id which BSP visitors can use for their
   * own purpose. The purpose of this is to allow several different
   * types of polygons to be added to the same tree. With this number
   * you can recognize them.
   */
  virtual int GetType () = 0;
};

#endif /*POLYINT_H*/
