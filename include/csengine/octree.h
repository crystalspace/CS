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
#include "csengine/polytree.h"
#include "csengine/bsp.h"

class csPolygonInt;
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
  /// Bounding box;
  csBox3 bbox;
  /// Center point for this node.
  csVector3 center;
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

  /**
   * Build the tree from the given node and number of polygons.
   * This is a dynamic version. It will add the polygons to an already built
   * octree and add the polygons so that they can easily be removed later.
   */
  void BuildDynamic (csOctreeNode* node, csPolygonInt** polygons, int num);

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

  /**
   * Process all todo stubs in a node and add new
   * todo stubs to the children of this node.
   */
  void ProcessTodo (csOctreeNode* node);

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
  	csVector3* array, int& num_array)
  {
    node->bbox.GetConvexOutline (pos, array, num_array);
  }

  /**
   * Build vertex tables for minibsp leaves. These tables are
   * used to optimize the world to camera transformation so that only
   * the needed vertices are transformed.
   */
  void BuildVertexTables () { if (root) ((csOctreeNode*)root)->BuildVertexTables (); }

  /// Return statistics about this octree.
  void Statistics (int* num_nodes, int* num_leaves, int* max_depth,
  	int* tot_polygons, int* max_poly_in_node, int* min_poly_in_node);
};

#endif /*OCTREE_H*/

