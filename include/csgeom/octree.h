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
  /// Minimum corner.
  csVector3 min_corner;
  /// Maximum corner.
  csVector3 max_corner;
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

  /// Set box.
  void SetBox (const csVector3& bmin, const csVector3& bmax)
  {
    min_corner = bmin;
    max_corner = bmax;
    center = (bmin + bmax) / 2;
  }

  /// Set mini-bsp tree.
  void SetMiniBsp (csBspTree* mbsp) { minibsp = mbsp; }

public:
  /// Return true if node is empty.
  bool IsEmpty ();

  /// Get center.
  const csVector3& GetCenter () { return center; }

  /// Get minimum coordinate of box.
  const csVector3& GetMinCorner () { return min_corner; }

  /// Get maximum coordinate of box.
  const csVector3& GetMaxCorner () { return max_corner; }

  /// Get mini-bsp tree.
  csBspTree* GetMiniBsp () { return minibsp; }

  /// Return type (NODE_???).
  int Type () { return NODE_OCTREE; }
};

// We have a coordinate system around our node which is
// divided into 27 regions. The center region at coordinate (1,1,1)
// is the node itself. Every one of the 26 remaining regions
// defines an number of vertices which are the convex outline
// as seen from a camera view point in that region.
// The numbers inside the outlines table are indices from 0 to
// 7 which describe the 8 vertices outlining the node:
//	0: left/down/front vertex
//	1: left/down/back
//	2: left/up/front
//	3: left/up/back
//	4: right/down/front
//	5: right/down/back
//	6: right/up/front
//	7: right/up/back
struct Outline
{
  int num;
  int vertices[6];
};
/// Outline lookup table.
static Outline outlines[27] =
{
  { 6, { 3, 2, 6, 4, 5, 1 } },		// 0,0,0
  { 6, { 3, 2, 0, 4, 5, 1 } },		// 0,0,1
  { 6, { 7, 3, 2, 0, 4, 5 } },		// 0,0,2
  { 6, { 3, 2, 6, 4, 0, 1 } },		// 0,1,0
  { 4, { 3, 2, 0, 1, -1, -1 } },	// 0,1,1
  { 6, { 7, 3, 2, 0, 1, 5 } },		// 0,1,2
  { 6, { 3, 7, 6, 4, 0, 1 } },		// 0,2,0
  { 6, { 3, 7, 6, 2, 0, 1 } },		// 0,2,1
  { 6, { 7, 6, 2, 0, 1, 5 } },		// 0,2,2
  { 6, { 2, 6, 4, 5, 1, 0 } },		// 1,0,0
  { 4, { 0, 4, 5, 1, -1, -1 } },	// 1,0,1
  { 6, { 3, 1, 0, 4, 5, 7 } },		// 1,0,2
  { 4, { 2, 6, 4, 0, -1, -1 } },	// 1,1,0
  { 0, { -1, -1, -1, -1, -1, -1 } },	// 1,1,1
  { 4, { 7, 3, 1, 5, -1, -1 } },	// 1,1,2
  { 6, { 3, 7, 5, 4, 0, 2 } },		// 1,2,0
  { 4, { 3, 7, 6, 2, -1, -1 } },	// 1,2,1
  { 6, { 2, 3, 1, 5, 7, 6 } },		// 1,2,2
  { 6, { 2, 6, 7, 5, 1, 0 } },		// 2,0,0
  { 6, { 6, 7, 5, 1, 0, 4 } },		// 2,0,1
  { 6, { 6, 7, 3, 1, 0, 4 } },		// 2,0,2
  { 6, { 2, 6, 7, 5, 4, 0 } },		// 2,1,0
  { 4, { 6, 7, 5, 4, -1, -1 } },	// 2,1,1
  { 6, { 6, 7, 3, 1, 5, 4 } },		// 2,1,2
  { 6, { 2, 3, 7, 5, 4, 0 } },		// 2,2,0
  { 6, { 2, 3, 7, 5, 4, 6 } },		// 2,2,1
  { 6, { 6, 2, 3, 1, 5, 4 } }		// 2,2,2
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
  	csTreeVisitFunc* func, void* data, csTreeCullFunc* cullfunc,
	void* culldata);
  /// Traverse the tree from front to back starting at 'node' and 'pos'.
  void* Front2Back (csOctreeNode* node, const csVector3& pos,
  	csTreeVisitFunc* func, void* data, csTreeCullFunc* cullfunc,
	void* culldata);

  /// Return statistics about this octree.
  void Statistics (csOctreeNode* node, int depth, int* num_nodes,
  	int* num_leaves, int* max_depth,
  	int* tot_polygons, int* max_poly_in_node, int* min_poly_in_node);

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
  void* Back2Front (const csVector3& pos, csTreeVisitFunc* func, void* data,
  	csTreeCullFunc* cullfunc = NULL, void* culldata = NULL);
  /// Traverse the tree from front to back starting at the root and 'pos'.
  void* Front2Back (const csVector3& pos, csTreeVisitFunc* func, void* data,
  	csTreeCullFunc* cullfunc = NULL, void* culldata = NULL);

  /**
   * Get a convex outline (not a polygon unless projected to 2D)
   * for for this octree node as seen from the given position.
   * The coordinates returned are world space coordinates.
   * Note that you need place for at least six vectors in the array.
   */
  void GetConvexOutline (csOctreeNode* node, const csVector3& pos,
  	csVector3* array, int& num_array);

  /// Return statistics about this octree.
  void Statistics (int* num_nodes, int* num_leaves, int* max_depth,
  	int* tot_polygons, int* max_poly_in_node, int* min_poly_in_node);
};

#endif /*OCTREE_H*/

