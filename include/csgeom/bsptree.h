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
#include "csutil/blockallocator.h"

struct iGraphics3D;
class csBSPTree;

/**
 * A callback function for visiting a BSP-tree node. If this function
 * returns true the traversal will continue. Otherwise Front2Back()
 * will stop.
 */
typedef bool (csBSPTreeVisitFunc)(csBSPTree* treenode, void* userdata);

/**
 * A BSP-tree is a binary tree that organizes 3D space.
 * This tree will not split triangles. If a triangle needs to be split
 * then it will be put in the two nodes.
 */
class CS_CSGEOM_EXPORT csBSPTree
{
private:
  static csBlockAllocator<csBSPTree> tree_nodes;

  csBSPTree* child1;		// If child1 is not 0 then child2 will
  csBSPTree* child2;		// also be not 0.

  int split_poly;

  int FindBestSplitter (csTriangle* triangles, csPlane3* planes,
	int num_triangles, csVector3* vertices);

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
   * Traverse the tree from front to back. Every node of the
   * tree will be encountered.
   */
  void Front2Back (const csVector3& pos, csBSPTreeVisitFunc* func);
};

#endif // __CS_BSPTREE_H__

