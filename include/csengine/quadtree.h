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
#define CS_QUAD_PARTIAL 1
#define CS_QUAD_UNKNOWN 2
#define CS_QUAD_FULL 3

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
  csBox2 bbox;

private:
  /// Build the tree with a given depth.
  void Build (csQuadtreeNode* node, const csBox2& box, int depth);

  /// Insert a polygon in the node.
  bool InsertPolygon (csQuadtreeNode* node,
	csVector2* verts, int num_verts,
	const csBox2& cur_bbox, const csBox2& pol_bbox);

  /// Test a polygon in the node.
  bool TestPolygon (csQuadtreeNode* node,
	csVector2* verts, int num_verts,
	const csBox2& cur_bbox, const csBox2& pol_bbox);

  /// Test a point in the node.
  int TestPoint (csQuadtreeNode* node, const csVector2& point);

public:
  /**
   * Create an empty tree with the given box.
   */
  csQuadtree (const csBox2& box, int depth);

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
  	const csBox2& pol_bbox);

  /**
   * Test for polygon visibility with the quad-tree.
   * Return true if polygon is visible.
   */
  bool TestPolygon (csVector2* verts, int num_verts,
  	const csBox2& pol_bbox);

  /**
   * Test if a given point is visible in the quad-tree.
   * Returns CS_QUAD_FULL if not visible, CS_QUAD_EMPTY
   * if visible and CS_QUAD_PARTIAL if undecided.
   * This function returns CS_QUAD_UNKNOWN if the point is not
   * in the quadtree.
   */
  int TestPoint (const csVector2& point);
};

/**
 *  4 child node states are stored in one byte.
 *  2 bits per node, in sequence. topleft, topright, botright, botleft
 */
#define CS_QUAD_TOPLEFT_MASK   0xC0
#define CS_QUAD_TOPLEFT_SHIFT  6
#define CS_QUAD_TOPRIGHT_MASK  0x30
#define CS_QUAD_TOPRIGHT_SHIFT 4
#define CS_QUAD_BOTRIGHT_MASK  0x0C
#define CS_QUAD_BOTRIGHT_SHIFT 2
#define CS_QUAD_BOTLEFT_MASK   0x03
#define CS_QUAD_BOTLEFT_SHIFT  0
#define CS_QUAD_ALL_EMPTY 0x0 /* because CS_QUAD_EMPTY == 0 */
#define CS_QUAD_ALL_FULL 0xFF /* because CS_QUAD_FULL == 3 */

class WWQuadTree;

/**
 *  Wouter's Wild QuadTree
 */
class WWQuadTree {
private:
  /// bounding box of the quadtree
  csBox2 bbox;
  /// depth of the tree, 1 == 1 node only.
  int depth;
  /// the state of the root node.
  int root_state;

  /** state of all children, and their children.
   *  they are ordered like this:
   *  root has children in byte 0.
   *  nodes in byte 0 have children in byte 0+node_nr(1,2,3,4).
   *  nodes in byte 1 have children in byte 4+node_nr.
   *  nodes in byte n have children in byte 4*n+node_nr
   *  So for byte 0, take 0+node_nr as the new byte
   *  for byte n, take 4*n + node_nr as the new byte
   */
  unsigned char* states;
  /// convenience variable: how many bytes alloced in states
  int state_size;


  /** this function is used for traversing the quadtree.
   *  it will get the nodes bounding box, state, byte offset and node_nr and 
   *  custom clientdata.
   */
  typedef int WWQuadTree::quad_traverse_func(csBox2& node_bbox, 
    int node_state, int offset, int node_nr, void* data);

  /** private functions to help dealing with quadtree
   *  call all four children of node at offset and node_nr
   *  each will be passed the data.
   *  returns return values of children in rc1,rc2,rc3,rc4
   *  note that theoretically the state of the caller could be changed.
   */
  void CallChildren(quad_traverse_func func, int offset, int node_nr, 
    void *data, int& rc1, int& rc2, int& rc3, int& rc4);

public:
  /** create a quadtree of depth, using about 2**twice depth bytes. depth >= 1
   *  depth=1 is only the root.
   */
  WWQuadTree(const csBox2& the_box, int the_depth);

  /// destroy quadtree
  ~WWQuadTree();

  /**
   * Is the tree full?
   */
  bool IsFull () { return root_state == CS_QUAD_FULL; }

  /**
   * Insert a polygon into the quad-tree.
   * Return true if the tree was modified (i.e. if parts of the
   * polygon were visible.
   */
  bool InsertPolygon (csVector2* verts, int num_verts,
  	const csBox2& pol_bbox);

  /**
   * Test for polygon visibility with the quad-tree.
   * Return true if polygon is visible.
   */
  bool TestPolygon (csVector2* verts, int num_verts,
  	const csBox2& pol_bbox);

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

