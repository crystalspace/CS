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

#ifndef OCTREE_H
#define OCTREE_H

#include "csgeom/math3d.h"
#include "csgeom/polytree.h"
#include "csgeom/bsp.h"

class csPolygonInt;
class csPolygonParentInt;
class csOctree;
class csBspTree;
class Dumper;

#define OCTREE_FFF 0
#define OCTREE_FFB 1
#define OCTREE_FBF 2
#define OCTREE_FBB 3
#define OCTREE_BFF 4
#define OCTREE_BFB 5
#define OCTREE_BBF 6
#define OCTREE_BBB 7

/**
 * An octree node.
 */
class csOctreeNode : public csPolygonTreeNode
{
  friend class csOctree;
  friend class Dumper;

private:
  /// Children.
  csPolygonTreeNode* children[8];
  /// Center point for this node.
  csVector3 center;
  /// Mini-bsp tree (in this case there are no children).
  csBspTree* minibsp;

private:
  /// Make an empty octree node.
  csOctreeNode ();

  /**
   * Destroy this octree node.
   */
  virtual ~csOctreeNode ();

  /// Remove all dynamically added polygons from the node.
  void RemoveDynamicPolygons ();

  /// Return true if node is empty.
  bool IsEmpty ();

  /// Set center.
  void SetCenter (const csVector3& c) { center = c; }

  /// Get center.
  const csVector3& GetCenter () { return center; }

  /// Set mini-bsp tree.
  void SetMiniBsp (csBspTree* mbsp) { minibsp = mbsp; }

  /// Get mini-bsp tree.
  csBspTree* GetMiniBsp () { return minibsp; }
};

/**
 * The octree.
 */
class csOctree : public csPolygonTree
{
  friend class Dumper;

private:
  /// The main bounding box for the octree (min).
  csVector3 min_bbox;
  /// The main bounding box for the octree (max).
  csVector3 max_bbox;
  /// The number of polygons at which we revert to a bsp tree.
  int bsp_num;
  /// The mode for the mini-bsp trees.
  int mode;

private:
  /// Build the tree from the given node and number of polygons.
  void Build (csOctreeNode* node, const csVector3& bmin, const csVector3& bmax,
  	csPolygonInt** polygons, int num);

  /**
   * Build the tree from the given node and number of polygons.
   * This is a dynamic version. It will add the polygons to an already built
   * octree and add the polygons so that they can easily be removed later.
   */
  void BuildDynamic (csOctreeNode* node, const csVector3& bmin, const csVector3& bmax,
  	csPolygonInt** polygons, int num);

  /**
   * Remove all dynamically added polygons from the node.
   */
  void RemoveDynamicPolygons (csOctreeNode* node);

  /// Traverse the tree from back to front starting at 'node' and 'pos'.
  void* Back2Front (csOctreeNode* node, const csVector3& pos,
  	csTreeVisitFunc* func, void* data);
  /// Traverse the tree from front to back starting at 'node' and 'pos'.
  void* Front2Back (csOctreeNode* node, const csVector3& pos,
  	csTreeVisitFunc* func, void* data);

public:
  /**
   * Create an empty tree for the given parent, a bounding box defining the
   * outer limits of the octree, and the number of polygons at which we revert to
   * a BSP tree.
   */
  csOctree (csPolygonParentInt* pset, const csVector3& min_bbox,
  	const csVector3& max_bbox, int bsp_num, int mode = BSP_MINIMIZE_SPLITS);

  /**
   * Destroy the whole octree (but not the actual polygons and parent
   * objects).
   */
  virtual ~csOctree ();

  /**
   * Create the tree for the default parent set.
   */
  void Build ();

  /**
   * Create the tree with a given set of polygons.
   */
  void Build (csPolygonInt** polygons, int num);

  /**
   * Add a bunch of polygons to the octree. They will be marked
   * as dynamic polygons so that you can remove them from the tree again
   * with RemoveDynamicPolygons(). Note that adding polygons dynamically
   * will not modify the existing tree and splits but instead continue
   * splitting in the leaves where the new polygons arrive.
   */
  void AddDynamicPolygons (csPolygonInt** polygons, int num);

  /**
   * Remove all dynamically added polygons from the node. Note that
   * the polygons are not really destroyed. Only unlinked from the
   * tree.
   */
  void RemoveDynamicPolygons ();

  /// Traverse the tree from back to front starting at the root and 'pos'.
  void* Back2Front (const csVector3& pos, csTreeVisitFunc* func, void* data);
  /// Traverse the tree from front to back starting at the root and 'pos'.
  void* Front2Back (const csVector3& pos, csTreeVisitFunc* func, void* data);
};

#endif /*OCTREE_H*/

