/*
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_POLYTREE_H__
#define __CS_POLYTREE_H__

#include "csextern.h"

#include "csgeom/box.h"
#include "csgeom/vector3.h"
#include "csutil/array.h"

#define CS_POLYTREE_AXISINVALID -1
#define CS_POLYTREE_AXISX 0
#define CS_POLYTREE_AXISY 1
#define CS_POLYTREE_AXISZ 2

struct iPolygonMesh;
struct csMeshedPolygon;

/**
 * A polygon tree. This is basically a simpler version of the kdtree.
 */
class CS_CSGEOM_EXPORT csPolygonTree
{
private:
  csBox3 bbox;			// Bounding box of all polygons in this node.
  csArray<int> polygons;	// Indices of polygons in this node.

  csPolygonTree* child1;	// If child1 is not 0 then child2 will
  csPolygonTree* child2;	// also be not 0.

  int split_axis;		// One of CS_POLYTREE_AXIS?
  float split_location;		// Where is the split?

  void MakeLeaf (csArray<int>& polyidx);
  void CalculateBBox (csArray<int>& polyidx, iPolygonMesh* mesh);
  void Build (csArray<int>& polyidx, iPolygonMesh* mesh);

public:
  /// Create a new empty polygon-tree.
  csPolygonTree ();
  /// Destroy the polygon-tree.
  ~csPolygonTree ();

  /// Make the tree empty.
  void Clear ();

  /// Build the tree from the given polygon mesh.
  void Build (iPolygonMesh* mesh);

  /// Get the bounding box.
  const csBox3& GetBoundingBox () const { return bbox; }

  /// Get the array of polygon indices.
  const csArray<int>& GetPolygons () const { return polygons; }

  /// Hit a box with this tree and return the polygons that intersect.
  void IntersectBox (csArray<int>& polyidx, const csBox3& box);

  /// Hit a sphere with this tree and return the polygons that intersect.
  void IntersectSphere (csArray<int>& polyidx, const csVector3& center,
  	float sqradius);

  /// Remove doubles in the list of polygons.
  void RemoveDoubles (csArray<int>& polyidx);
};

#endif // __CS_POLYTREE_H__

