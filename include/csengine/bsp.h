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

#ifndef BSP_H
#define BSP_H

#include "csgeom/math3d.h"
#include "csengine/polytree.h"
#include "csengine/polyint.h"

class csBspTree;
class Dumper;

// The BSP tree can be build using the following criteria:
#define BSP_MINIMIZE_SPLITS 1		// Minimize the number of polygon splits
#define BSP_MOST_ON_SPLITTER 2		// Splitter with most coplanar polygons
#define BSP_RANDOM 3			// Choose a random splitter
#define BSP_ALMOST_MINIMIZE_SPLITS 4	// Select a number of polygons and choose minimal split
#define BSP_BALANCED 5			// Try to create a balanced tree
#define BSP_ALMOST_BALANCED 6		// Try to create an approximation of a balanced tree

/**
 * A BSP node.
 */
class csBspNode : public csPolygonTreeNode
{
  friend class csBspTree;
  friend class Dumper;

private:
  /**
   * All the polygons in this node.
   * These polygons are all on the same plane.
   * The 'front' and 'back' children in this node are seperated
   * by that plane.
   */
  csPolygonIntArray polygons;
  /**
   * If not -1 then this is the index of the first dynamic
   * polygon in the list of polygons.
   */
  int dynamic_idx;

  /// The front node.
  csBspNode* front;
  /// The back node.
  csBspNode* back;

private:
  /// Make an empty BSP node.
  csBspNode ();

  /**
   * Destroy this BSP node. The list of polygons
   * will be deleted but not the polygons themselves.
   */
  virtual ~csBspNode ();

  /**
   * Add a polygon to this BSP node.
   * If 'dynamic' is true it will be a dynamic polygon.
   * Dynamic polygons can be removed all at once with RemoveDynamicPolygons().
   */
  void AddPolygon (csPolygonInt* poly, bool dynamic = false);

  /**
   * Remove all dynamic polygons.
   */
  void RemoveDynamicPolygons ();

  /**
   * Count all vertices used by polygons in this bsp node
   * and children. Vertices used multiple times will be counted
   * multiple times as well. So really it counts all the corners
   * of all polygons used in the bsp node.
   */
  int CountVertices ();

  /**
   * Fetch all vertex indices used by polygons in this bsp node
   * and children. The given array must be at least CountVertices()
   * big.
   */
  void FetchVertices (int* array, int& cur_idx);

public:
  /// Return true if node is empty.
  bool IsEmpty ()
  {
    return polygons.GetNumPolygons () == 0 &&
    	(!front || front->IsEmpty ()) &&
	(!back || back->IsEmpty ());
  }

  /// Return type (NODE_???).
  int Type () { return NODE_BSPTREE; }
};

/**
 * The BSP tree.
 */
class csBspTree : public csPolygonTree
{
  friend class Dumper;

private:
  /// The mode.
  int mode;

private:
  /// Build the tree from the given node and number of polygons.
  void Build (csBspNode* node, csPolygonInt** polygons, int num);

  /**
   * Build the tree from the given node and number of polygons.
   * This is a dynamic version. It will add the polygons to an already built
   * BSP tree and add the polygons so that they can easily be removed later.
   */
  void BuildDynamic (csBspNode* node, csPolygonInt** polygons, int num);

  /**
   * Select a splitter from a list of polygons and return the index.
   */
  int SelectSplitter (csPolygonInt** polygons, int num);

  /// Traverse the tree from back to front starting at 'node' and 'pos'.
  void* Back2Front (csBspNode* node, const csVector3& pos,
  	csTreeVisitFunc* func, void* data, csTreeCullFunc* cullfunc,
	void* culldata);
  /// Traverse the tree from front to back starting at 'node' and 'pos'.
  void* Front2Back (csBspNode* node, const csVector3& pos,
  	csTreeVisitFunc* func, void* data, csTreeCullFunc* cullfunc,
	void* culldata);

  /// Return statistics about this bsp tree.
  void Statistics (csBspNode* node, int depth, int* num_nodes,
  	int* num_leaves, int* max_depth,
  	int* tot_polygons, int* max_poly_in_node, int* min_poly_in_node);

  /// Handle all dynamic objects in this tree.
  void* HandleObjects (csBspNode* node, const csVector3& pos,
  	csTreeVisitFunc* func, void* data);

public:
  /**
   * Create an empty tree for a parent container.
   */
  csBspTree (csPolygonParentInt* pset, int mode = BSP_MINIMIZE_SPLITS);

  /**
   * Destroy the whole BSP tree (but not the actual polygons and parent
   * objects).
   */
  virtual ~csBspTree ();

  /**
   * Create the tree for the default parent set.
   */
  void Build ();

  /**
   * Create the tree with a given set of polygons.
   */
  void Build (csPolygonInt** polygons, int num);

  /**
   * Add a bunch of polygons to the BSP tree. They will be marked
   * as dynamic polygons so that you can remove them from the tree again
   * with RemoveDynamicPolygons(). Note that adding polygons dynamically
   * will not modify the existing tree and splits but instead continue
   * splitting in the leaves where the new polygons arrive.
   */
  void AddDynamicPolygons (csPolygonInt** polygons, int num);

  /**
   * Remove all dynamically added polygons from the node. Note that
   * the polygons are not really destroyed. Only unlinked from the BSP
   * tree.
   */
  void RemoveDynamicPolygons ();

  /// Traverse the tree from back to front starting at the root and 'pos'.
  void* Back2Front (const csVector3& pos, csTreeVisitFunc* func, void* data,
  	csTreeCullFunc* cullfunc = NULL, void* culldata = NULL);
  /// Traverse the tree from front to back starting at the root and 'pos'.
  void* Front2Back (const csVector3& pos, csTreeVisitFunc* func, void* data,
  	csTreeCullFunc* cullfunc = NULL, void* culldata = NULL);

  /// Return statistics about this bsp tree.
  void Statistics (int* num_nodes, int* num_leaves, int* max_depth,
  	int* tot_polygons, int* max_poly_in_node, int* min_poly_in_node);

  /**
   * Get a list of all vertices used by all the polygons in this
   * bsp tree. This list should be deleted with 'delete[]' after use.
   * The vertices are returned as indices into the parent object (pset).
   */
  int* GetVertices (int& count);

  /**
   * Return true if bsp tree is empty.
   */
  bool IsEmpty () { return root ? root->IsEmpty () : false; }
};

#endif /*BSP_H*/

