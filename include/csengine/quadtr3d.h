/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Copyright (C) 2000 by Wouter Wijngaards

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

#ifndef __CS_QUADTR3D_H__
#define __CS_QUADTR3D_H__

#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/box.h"
class q3d_node_pos_info;

/**
 * A quadtree node can be in three states: empty, full, or partial.
 * If empty or full the state of the children does not matter.
 * The unknown state is an error.
 *  4 child node states are stored in one byte.
 *  2 bits per node, in sequence.
 */
#define CS_QUAD3D_EMPTY 0
#define CS_QUAD3D_PARTIAL 1
#define CS_QUAD3D_UNKNOWN 2
#define CS_QUAD3D_FULL 3
#define CS_QUAD3D_ALL_EMPTY 0x0 /* because CS_QUAD3D_EMPTY == 0 */
#define CS_QUAD3D_ALL_FULL 0xFF /* because CS_QUAD3D_FULL == 3 */
#define CS_QUAD3D_ALL_UNKNOWN 0xAA /* because CS_QUAD3D_FULL == 2 */

/**
 * Values denoting if the tree changed.
 * Note that false means no change, and a true value a change.
 */
#define CS_QUAD3D_NOCHANGE 0
#define CS_QUAD3D_POSSIBLECHANGE 1
#define CS_QUAD3D_CERTAINCHANGE 2

/**
 *  QuadTree3D
 */
class csQuadTree3D
{
  friend class Dumper;

private:
  /// Bounding box of the quadtree
  csBox3 bbox;
  /// Four corners of a rectangle of the root rectangle in 3d space.
  csVector3 corners[4];
  /// Center of frustums, the viewpoint for culling.
  csVector3 center;
  /// Depth of the tree, 1 == 1 node only, a root.
  int max_depth;
  /// State of the root node.
  int root_state;

  /**
   * A quick lookup array to divide the rectangles...
   * for vertex n to n+1, in [n][d] are the csvector3s
   * of the size of a rectangle at that depth. d at 0==root;
   * Thus in [0][0] is stored corners[1] - corners[0];
   *   (adding this value to corners[0] will give the next corner)
   * and in [0][1] is stored (corners[1] - corners[0])/2.0,
   *   (adding this value to corners[0] will give the midpoint)
   * Thus at [n][current_depth] is the size of children
   * Having this table avoids having to divide a lot later.
   */
  csVector3* sizes[4];

  /**
   * State of all children, and their children.
   * They are ordered like this:
   * /// Old style:
   * ///root has children in byte 0.
   * ///nodes in byte 0 have children in byte 0+node_nr+1(1,2,3,4).
   * ///nodes in byte 1 have children in byte 4+node_nr+1.
   * ///nodes in byte n have children in byte 4*n+node_nr+1
   * ///So for byte n, take 4*n + node_nr+1 as the new byte
   * New style:
   * nodes of depth 2, then all nodes of depth 3, etc.
   * nodes are sequentially stored (bytepacked) in the ordering
   * y*plane_size+x. The root node in depth 1 is stored in variable
   * root_state.
   */
  unsigned char* states;

  /// Convenience variable: how many bytes alloced in states
  int state_size;

  /**
   * This function is used for traversing the quadtree.
   * it will get the nodes bounding box, state, and node_pos
   * all these are this node's values.
   * and custom clientdata.
   * It can return an integer.
   * node_depth==1 means you are the root, and so on,
   * if the node_depth == max_depth you are a leaf.
   */
  typedef int (csQuadTree3D::*quad_traverse_func)(csQuadTree3D*,
    int node_state, q3d_node_pos_info*, void* data);

  /**
   * Private functions to help dealing with quadtree.
   * Call all four children. Of the node at parent_pos,
   * box is the bounding box of the parent node.
   * each child will be passed the data.
   * returns return values of children in retval.
   * in order 0=topleft, 1=topright, 2=bottomleft, 3=bottomright.
   * note that theoretically the state of the caller could be changed.
   * func is of type quad_traverse_func.
   */
  void CallChildren(quad_traverse_func func, csQuadTree3D*,
    q3d_node_pos_info *parent_pos, void *data, int retval[4]);

  /**
   * Convenience version, the retval argument is omitted, return values
   * are discarded.
   */
  void CallChildren(quad_traverse_func func, csQuadTree3D*,
    q3d_node_pos_info *parent_pos, void *data);

  /**
   * Get the state of a node, in byte offset , nodenr given.
   * offset -1 means root node.
   */
  int GetNodeState(int offset, int nodenr) const;

  /// Convenience function to GetNodeState at q3d_node_pos_info
  int GetNodeState(q3d_node_pos_info *pos) const;

  /**
   * Set the state of a node, in byte offset , nodenr given.
   * offset -1 means root node.
   */
  void SetNodeState(int offset, int nodenr, int newstate);

  /// Convenience function to set a node state
  void SetNodeState(q3d_node_pos_info *pos, int newstate);

  /**
   * For a node, check if data (a csVector3 * relative to center)
   * is in this node, possibly recursing.
   * returning the QUAD3D_FULL/PARTIAL/EMPTY if it hits.
   * returning QUAD3D_UNKOWN if the point is not in the node.
   */
  int test_point_func(csQuadTree3D*, int node_state, q3d_node_pos_info*,
    void* data);

  /// Gather result from retval for testpoint
  int GetTestPointResult(int retval[4]) const;

  /// Struct with info for polygon being inserted
  struct poly_info
  {
    csVector3* verts;
    int num_verts;
    const csBox3* pol_bbox;
    bool test_only;
  };

  /**
   * For a node, insert polygon into it (possibly in its children too)
   * expects data to be a struct poly_info*
   * returns true if the polygon covered previously empty space.
   * Thus, if the tree is modified. or if PARTIAL->PARTIAL.
   * Returns CS_QUAD3D_NOCHANGE if no change to the tree.
   *  CS_QUAD3D_POSSIBLECHANGE if it probably changed, and
   *  CS_QUAD3D_CERTAINCHANGE if it changed for certain.
   */
  int insert_polygon_func(csQuadTree3D*, int node_state, q3d_node_pos_info*,
    void* data);

  /**
   * Gather result from retval for insertpolygon_func
   * NOCHANGE < POSSIBLECHANGE < CERTAINCHANGE
   */
  int GetPolygonResult(int retval[4]) const;

  /**
   * For a node, mark it by casting (void*)data to an int.
   * that is the new state.
   */
  int mark_node_func (csQuadTree3D*, int node_state, q3d_node_pos_info*,
    void* data);

  /**
   * For a node, give it the (int) cast of data as value and
   * give children a good value. CS_QUAD3D_UNKNOWN means the
   * child's value is unchanged.
   */
  int propagate_func(csQuadTree3D*, int node_state, q3d_node_pos_info*,
    void* data);

  /**
   * Set the state of the node based on states of child nodes.
   * returns new node state, leaves only return their state.
   */
  int sift_func(csQuadTree3D*, int node_state, q3d_node_pos_info*, void* data);

public:
  /**
   * Create a quadtree of depth, using about (4**depth-1)/3-1/3 bytes.
   *  depth must be >= 1
   *  depth=1 is only the root.
   *  startcenter is the origin of the frustums, the viewpoint for culling.
   *  bounding box is a bounding box for the culler (unused now)
   *  the 4 corners state the root rectangle in 3d coordinates.
   *     in a circular ordering, as for a 3d polygon.
   *     and must be relative to the center point.
   */
  csQuadTree3D(const csVector3& start_center, const csVector3 startcorners[4],
    const csBox3& bounding_box, int the_depth);

  /// Destroy quadtree
  ~csQuadTree3D();

  /**
   * Set the main frustum.
   */
  void SetMainFrustum (const csVector3& start_center,
  	const csVector3 startcorners[4]);

  /// Is the tree full?
  bool IsFull () const { return root_state == CS_QUAD3D_FULL; }

  /// Get the center of the frustums, the origin of them.
  const csVector3& GetCenter() const { return center;}

  /// Get the bounding box of the culler
  const csBox3& GetBoundingBox() const { return bbox;}

  /// Get the root rectangle vertices, array of 4.
  csVector3* GetCorners() {return corners;}

  /// Make the tree empty again
  void MakeEmpty();

  /**
   * Insert a polygon into the quad-tree.
   *   polygon must be relative to the center point.
   * Return true if the tree was modified (i.e. if parts of the
   * polygon were visible. More precisely:
   * Returns CS_QUAD3D_NOCHANGE if no change to the tree.
   *  CS_QUAD3D_POSSIBLECHANGE if it probably changed, and
   *  CS_QUAD3D_CERTAINCHANGE if it changed for certain.
   */
  int InsertPolygon (csVector3* verts, int num_verts,
  	const csBox3& pol_bbox);

  /**
   * Test for polygon visibility with the quad-tree.
   *   polygon must be relative to the center point.
   * Returns CS3D_QUAD_NOCHANGE if no change to the tree.
   *  CS_QUAD3D_POSSIBLECHANGE if it probably changed, and
   *  CS_QUAD3D_CERTAINCHANGE if it changed for certain.
   */
  int TestPolygon (csVector3* verts, int num_verts,
  	const csBox3& pol_bbox);

  /**
   * Test if a given point is visible in the quad-tree.
   *   point must be relative to the center point.
   * Returns CS_QUAD3D_FULL if not visible, CS_QUAD3D_EMPTY
   * if visible and CS_QUAD3D_PARTIAL if undecided.
   * This function returns CS_QUAD3D_UNKNOWN if the point is not
   * in the quadtree.
   */
  int TestPoint (const csVector3& point);

  /**
   * A TestRectangle function that will test on leaf-coordinates, for full.
   * Give depth of the node-plane to test on, the x and y integer node coords
   * of the topleft corner of the rectangle, and it's width and height.
   * Returns true if and only if all nodes at that spot are CS_QUAD3D_FULL.
   * Note: You must have called Propagate() before using this method.
   */
  bool TestRectangle(int depth, int x, int y, int w, int h);

  /**
   * Propagate, makes sure that all nodes in the tree get a value
   * repesenting their state. For speed reasons this is not always the case
   * after an InsertPolygon() call.
   */
  void Propagate();

  /**
   * The leaves must have valid values. The other nodes are
   * given correct values.
   */
  void Sift();

  /// This function will print the quadtree.
  void Print();
};

#endif // __CS_QUADTR3D_H__
