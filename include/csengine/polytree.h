/*
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

#ifndef POLYTREE_H
#define POLYTREE_H

#include "csgeom/math3d.h"

class csPolygonInt;
class csPolygonParentInt;
class csPolygonTree;
class csPolygonStub;
class csPolyTreeObject;


#define NODE_OCTREE 1
#define NODE_BSPTREE 2

/**
 * A general node in a polygon tree.
 */
class csPolygonTreeNode
{
protected:
  /**
   * A linked list for all polygons stubs that are added
   * to this node. These stubs represents sets of polygons
   * from this object that belong to the same plane as the
   * plane of the splitter in the tree node.
   */
  csPolygonStub* first_stub;

  /**
   * A linked list of all polygons stubs that still need to
   * be processed whenever this node becomse visible.
   */
  csPolygonStub* todo_stubs;

public:
  /**
   * Constructor.
   */
  csPolygonTreeNode () : first_stub (NULL), todo_stubs (NULL) { }

  /**
   * Destructor.
   */
  virtual ~csPolygonTreeNode ();

  /// Remove all dynamic added polygons.
  virtual void RemoveDynamicPolygons () = 0;

  /// Return true if node is empty.
  virtual bool IsEmpty () = 0;

  /// Return type (NODE_???).
  virtual int Type () = 0;

  /**
   * Unlink a stub from the stub list.
   * Warning! This function does not test if the stub
   * is really on the list!
   */
  void UnlinkStub (csPolygonStub* ps);

  /**
   * Link a stub to the todo list.
   */
  void LinkStubTodo (csPolygonStub* ps);

  /**
   * Link a stub to the stub list.
   */
  void LinkStub (csPolygonStub* ps);
};

/**
 * Visit a node in a polygon tree. If this function returns non-NULL
 * the scanning will stop and the pointer will be returned.
 */
typedef void* (csTreeVisitFunc)(csPolygonParentInt*, csPolygonInt**,
	int num, void*);

/**
 * Potentially cull a node from the tree just before it would otherwise
 * have been traversed in Back2Front() or Front2Back().
 * If this function returns true then the node is potentially visible.
 */
typedef bool (csTreeCullFunc)(csPolygonTree* tree, csPolygonTreeNode* node,
	const csVector3& pos, void* data);

/**
 * A general polygon tree. This is an abstract data type.
 * Concrete implementations like csBspTree or csOctree inherit
 * from this class.
 */
class csPolygonTree
{
protected:
  /// The root of the tree.
  csPolygonTreeNode* root;

  /// The parent that this tree is made for.
  csPolygonParentInt* pset;

  /// Clear the nodes.
  void Clear () { CHK (delete root); }

public:
  /**
   * Constructor.
   */
  csPolygonTree (csPolygonParentInt* ps) : root (NULL), pset (ps) { }

  /**
   * Destructor.
   */
  virtual ~csPolygonTree () { }

  /// Get the polygonset for this tree.
  csPolygonParentInt* GetParent () { return pset; }

  /**
   * Create the tree for the default parent set.
   */
  virtual void Build () = 0;

  /**
   * Create the tree with a given set of polygons.
   */
  virtual void Build (csPolygonInt** polygons, int num) = 0;

  /**
   * Add a bunch of polygons to the tree. They will be marked
   * as dynamic polygons so that you can remove them from the tree again
   * with RemoveDynamicPolygons(). Note that adding polygons dynamically
   * will not modify the existing tree and splits but instead continue
   * splitting in the leaves where the new polygons arrive.
   */
  virtual void AddDynamicPolygons (csPolygonInt** polygons, int num) = 0;

  /**
   * Remove all dynamically added polygons from the node. Note that
   * the polygons are not really destroyed. Only unlinked from the BSP
   * tree.
   */
  virtual void RemoveDynamicPolygons () = 0;

  /**
   * Add a dynamic object to the tree.
   */
  void AddObject (csPolyTreeObject* obj);

  /// Traverse the tree from back to front starting at the root and 'pos'.
  virtual void* Back2Front (const csVector3& pos, csTreeVisitFunc* func,
  	void* data, csTreeCullFunc* cullfunc = NULL, void* culldata = NULL) = 0;
  /// Traverse the tree from front to back starting at the root and 'pos'.
  virtual void* Front2Back (const csVector3& pos, csTreeVisitFunc* func,
  	void* data, csTreeCullFunc* cullfunc = NULL, void* culldata = NULL) = 0;

  /// Return statistics about this tree.
  virtual void Statistics (int* num_nodes, int* num_leaves, int* max_depth,
  	int* tot_polygons, int* max_poly_in_node, int* min_poly_in_node) = 0;
};

#endif /*POLYTREE_H*/

