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
#include "csgeom/box.h"
#include "csutil/garray.h"
#include "csengine/polytree.h"
#include "csengine/bsp.h"
#include "csengine/arrays.h"

class csPolygonInt;
class csOctree;
class csOctreeNode;
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
 * A visibility info node for one octree node.
 * This node represents a visible octree node and possibly
 * all visible polygons (if the node is a leaf).
 */
class csOctreeVisible
{
private:
  // The visible node.
  csOctreeNode* node;
  // The visible polygons.
  csPolygonArray polygons;
  
public:
  /// Set octree node.
  void SetOctreeNode (csOctreeNode* onode) { node = onode; }
  /// Get octree node.
  csOctreeNode* GetOctreeNode () { return node; }
  /// Get the polygon array.
  csPolygonArray& GetPolygons () { return polygons; }
};

/// A growing array for the PVS information (visible octree/polygons).
TYPEDEF_GROWING_ARRAY (csPVS, csOctreeVisible);

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
  /// Bounding box;
  csBox3 bbox;
  /// Center point for this node.
  csVector3 center;
  /**
   * If true then this is a leaf.
   * If a node has no polygons then it will also be a leaf
   * but there will be no mini-bsp.
   */
  bool leaf;
  /// Mini-bsp tree (in this case there are no children).
  csBspTree* minibsp;
  /**
   * If there is a mini-bsp tree this array contains the indices
   * of all vertices that are used by the polygons in the tree.
   * This can be used to optimize the world->camera transformation
   * process because only the minibsp nodes that are really used
   * need to be traversed.
   */
  int* minibsp_verts;
  /// Number of vertices in minibsp_verts.
  int minibsp_numverts;

  /**
   * A growing array for the PVS information.
   * This list contains all octree nodes and polygons that
   * are visible from this node.
   */
  csPVS pvs;
  /**
   * Visibility number. If equal to csOctreeNode::pvs_cur_vis_nr then
   * this object is visible.
   */
  ULong pvs_vis_nr;

public:
  /**
   * Current visibility number. All objects (i.e. octree
   * nodes and polygons) which have the same number as this one
   * are visible. All others are not.
   */
  static ULong pvs_cur_vis_nr;

private:
  /// Make an empty octree node.
  csOctreeNode ();

  /**
   * Destroy this octree node.
   */
  virtual ~csOctreeNode ();

  /// Set box.
  void SetBox (const csVector3& bmin, const csVector3& bmax)
  {
    bbox.Set (bmin, bmax);
    center = (bmin + bmax) / 2;
  }

  /// Set mini-bsp tree.
  void SetMiniBsp (csBspTree* mbsp);

  /// Build vertex tables.
  void BuildVertexTables ();

public:
  /// Return true if node is empty.
  bool IsEmpty ();

  /// Return true if node is visible according to PVS.
  bool IsVisible () { return pvs_vis_nr == pvs_cur_vis_nr; }

  /// Mark visible (used by PVS).
  void MarkVisible () { pvs_vis_nr = pvs_cur_vis_nr; }

  /// Return true if node is leaf.
  bool IsLeaf () { return leaf; }

  /// Get center.
  const csVector3& GetCenter () const { return center; }

  /// Get minimum coordinate of box.
  const csVector3& GetMinCorner () const { return bbox.Min (); }

  /// Get maximum coordinate of box.
  const csVector3& GetMaxCorner () const { return bbox.Max (); }

  /// Get mini-bsp tree.
  csBspTree* GetMiniBsp () const { return minibsp; }

  /// Get indices of vertices used in the mini-bsp of this leaf.
  int* GetMiniBspVerts () const { return minibsp_verts; }

  /// Get number of vertices.
  int GetMiniBspNumVerts () const { return minibsp_numverts; }

  /// Return type (NODE_???).
  int Type () { return NODE_OCTREE; }

  /// Get the PVS.
  csPVS& GetPVS () { return pvs; }
};

/**
 * The octree.
 */
class csOctree : public csPolygonTree
{
  friend class Dumper;

private:
  /// The main bounding box for the octree.
  csBox3 bbox;
  /// The number of polygons at which we revert to a bsp tree.
  int bsp_num;
  /// The mode for the mini-bsp trees.
  int mode;

private:
  /// Build the tree from the given node and number of polygons.
  void Build (csOctreeNode* node, const csVector3& bmin, const csVector3& bmax,
  	csPolygonInt** polygons, int num);

  /// Traverse the tree from back to front starting at 'node' and 'pos'.
  void* Back2Front (csOctreeNode* node, const csVector3& pos,
  	csTreeVisitFunc* func, void* data, csTreeCullFunc* cullfunc,
	void* culldata);
  /// Traverse the tree from front to back starting at 'node' and 'pos'.
  void* Front2Back (csOctreeNode* node, const csVector3& pos,
  	csTreeVisitFunc* func, void* data, csTreeCullFunc* cullfunc,
	void* culldata);

  /**
   * Process all todo stubs in a node and add new
   * todo stubs to the children of this node.
   */
  void ProcessTodo (csOctreeNode* node);

  /**
   * Try to find the best center possible and update the node.
   */
  void ChooseBestCenter (csOctreeNode* node, csPolygonInt** polygons, int num);

  /**
   * Gather statistics info about this tree.
   */
  void Statistics (csOctreeNode* node, int depth,
  	int* num_oct_nodes, int* max_oct_depth, int* num_bsp_trees,
  	int* tot_bsp_nodes, int* min_bsp_nodes, int* max_bsp_nodes,
	int* tot_bsp_leaves, int* min_bsp_leaves, int* max_bsp_leaves,
	int* tot_max_depth, int* min_max_depth, int* max_max_depth,
	int* tot_tot_poly, int* min_tot_poly, int* max_tot_poly);

public:
  /**
   * Create an empty tree for the given parent, a bounding box defining the
   * outer limits of the octree, and the number of polygons at which we
   * revert to a BSP tree.
   */
  csOctree (csSector* sect, const csVector3& min_bbox,
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
   * Create the tree with a given set of polygons.
   */
  void Build (const csPolygonArray& polygons);

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
  	csVector3* array, int& num_array)
  {
    node->bbox.GetConvexOutline (pos, array, num_array);
  }

  /**
   * Update with PVS. This routine will take the given position
   * and locate the octree leaf node for that position. Then it
   * will get the PVS for that node and mark all visible octree
   * nodes and polygons.
   */
  void MarkVisibleFromPVS (const csVector3& pos);

  /**
   * Build vertex tables for minibsp leaves. These tables are
   * used to optimize the world to camera transformation so that only
   * the needed vertices are transformed.
   */
  void BuildVertexTables () { if (root) ((csOctreeNode*)root)->BuildVertexTables (); }

  /// Print statistics about this octree.
  void Statistics ();
};

#endif /*OCTREE_H*/

