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

#ifndef QUADTREE_H
#define QUADTREE_H

#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/box.h"

// A quadtree node can be in three states: empty, full, or partial.
// If empty or full the state of the children does not matter.
#define CS_QUAD_EMPTY 0
#define CS_QUAD_FULL 1
#define CS_QUAD_PARTIAL 2
#define CS_QUAD_UNKNOWN 3

class csQuadtree;
class Dumper;

/**
 * A quadtree node.
 */
class csQuadtreeNode
{
  friend class csQuadtree;
  friend class Dumper;

private:
  /**
   * Children.
   * 0 = topleft, 1=topright, 2=bottomright, 3=bottomleft.
   */
  csQuadtreeNode* children;

  /// Center point for this node.
  csVector2 center;
  /// Contents state of this node.
  int state;

private:
  /// Make an empty quadtree node.
  csQuadtreeNode ();

  /**
   * Destroy this quadtree node.
   */
  ~csQuadtreeNode ();

  /// Set center.
  void SetCenter (const csVector2& cen) { center = cen; }

public:
  /// Get center.
  const csVector2& GetCenter () { return center; }

  /// Get contents state.
  int GetState () { return state; }

  /// Set state.
  void SetState (int st) { state = st; }
};

/**
 * The quadtree.
 */
class csQuadtree
{
  friend class Dumper;

private:
  /// The root of the tree.
  csQuadtreeNode* root;
  /// The bounding box of the entire tree.
  csBox bbox;

private:
  /// Build the tree with a given depth.
  void Build (csQuadtreeNode* node, const csBox& box, int depth);

  /// Insert a polygon in the node.
  bool InsertPolygon (csQuadtreeNode* node,
	csVector2* verts, int num_verts,
	const csBox& cur_bbox, const csBox& pol_bbox);

  /// Test a polygon in the node.
  bool TestPolygon (csQuadtreeNode* node,
	csVector2* verts, int num_verts,
	const csBox& cur_bbox, const csBox& pol_bbox);

  /// Test a point in the node.
  int TestPoint (csQuadtreeNode* node, const csVector2& point);

public:
  /**
   * Create an empty tree with the given box.
   */
  csQuadtree (const csBox& box, int depth);

  /**
   * Destroy the whole quadtree.
   */
  virtual ~csQuadtree ();

  /**
   * Make tree empty.
   */
  void MakeEmpty () { root->SetState (CS_QUAD_EMPTY); }

  /**
   * Is the tree full?
   */
  bool IsFull () { return root->GetState () == CS_QUAD_FULL; }

  /**
   * Insert a polygon into the quad-tree.
   * Return true if the tree was modified (i.e. if parts of the
   * polygon were visible.
   */
  bool InsertPolygon (csVector2* verts, int num_verts,
  	const csBox& pol_bbox);

  /**
   * Test for polygon visibility with the quad-tree.
   * Return true if polygon is visible.
   */
  bool TestPolygon (csVector2* verts, int num_verts,
  	const csBox& pol_bbox);

  /**
   * Test if a given point is visible in the quad-tree.
   * Returns CS_QUAD_FULL if not visible, CS_QUAD_EMPTY
   * if visible and CS_QUAD_PARTIAL if undecided.
   * This function returns CS_QUAD_UNKNOWN if the point is not
   * in the quadtree.
   */
  int TestPoint (const csVector2& point);
};

#endif /*QUADTREE_H*/

