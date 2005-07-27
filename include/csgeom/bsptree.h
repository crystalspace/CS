/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __CS_BSPTREE_H__
#define __CS_BSPTREE_H__

#include "csextern.h"

#include "csgeom/plane3.h"
#include "csutil/array.h"
#include "csutil/blockallocator.h"
#include "csutil/dirtyaccessarray.h"

struct iGraphics3D;

template<class T>
class csSet;
struct csTriangle;

/**
 * This BSP-tree is a binary tree that organizes a triangle mesh.
 * This tree will not split triangles. If a triangle needs to be split
 * then it will be put in the two nodes.
 */
class CS_CRYSTALSPACE_EXPORT csBSPTree
{
private:
  static csBlockAllocator<csBSPTree> tree_nodes;
  static csDirtyAccessArray<int> b2f_array;

  csBSPTree* child1;		// If child1 is not 0 then child2 will
  csBSPTree* child2;		// also be not 0.

  csPlane3 split_plane;
  csArray<int> splitters;

  size_t FindBestSplitter (csTriangle* triangles, csPlane3* planes,
	int num_triangles, csVector3* vertices,
	const csArray<int>& triidx);
  void Build (csTriangle* triangles, csPlane3* planes,
	int num_triangles, csVector3* vertices,
	const csArray<int>& triidx);
  void Back2Front (const csVector3& pos, csDirtyAccessArray<int>& arr,
  	csSet<int>& used_indices);
public:
  /// Create a new empty BSP-tree.
  csBSPTree ();
  /// Destroy the BSP-tree.
  ~csBSPTree ();

  /// Clear the BSP-tree.
  void Clear ();

  /**
   * Build the BSP tree given the set of triangles.
   */
  void Build (csTriangle* triangles, int num_triangles,
  	csVector3* vertices);

  /**
   * Traverse the tree from back to front. This will return an array
   * containing the triangle indices in back2front order. The array
   * will not contain double elements.
   */
  const csDirtyAccessArray<int>& Back2Front (const csVector3& pos);
};

#endif // __CS_BSPTREE_H__

