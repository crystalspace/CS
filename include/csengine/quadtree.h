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

#ifndef QUADTREE_H
#define QUADTREE_H

#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/box.h"

/**
 * A quadtree node can be in three states: empty, full, or partial.
 * If empty or full the state of the children does not matter.
 * The unknown state is an error.
 *  4 child node states are stored in one byte.
 *  2 bits per node, in sequence. 
 */
#define CS_QUAD_EMPTY 0
#define CS_QUAD_PARTIAL 1
#define CS_QUAD_UNKNOWN 2
#define CS_QUAD_FULL 3
#define CS_QUAD_ALL_EMPTY 0x0 /* because CS_QUAD_EMPTY == 0 */
#define CS_QUAD_ALL_FULL 0xFF /* because CS_QUAD_FULL == 3 */
#define CS_QUAD_ALL_UNKNOWN 0xAA /* because CS_QUAD_FULL == 2 */

class csQuadTree;
class Dumper;


/**
 * values denoting if the tree changed.
 * Note that false means no change, and a true value a change.
 */
#define CS_QUAD_NOCHANGE 0
#define CS_QUAD_POSSIBLECHANGE 1
#define CS_QUAD_CERTAINCHANGE 2

class csQuadTree;
class node_pos_info;


/**
 *  QuadTree
 */
class csQuadTree {
  friend class Dumper;

private:
  /// bounding box of the quadtree
  csBox2 bbox;
  /// depth of the tree, 1 == 1 node only, a root.
  int max_depth;
  /// the state of the root node.
  int root_state;

  /** state of all children, and their children.
   *  they are ordered like this:
   *  /// Old style:
   *  ///root has children in byte 0.
   *  ///nodes in byte 0 have children in byte 0+node_nr+1(1,2,3,4).
   *  ///nodes in byte 1 have children in byte 4+node_nr+1.
   *  ///nodes in byte n have children in byte 4*n+node_nr+1
   *  ///So for byte n, take 4*n + node_nr+1 as the new byte
   *  New style:
   *  nodes of depth 2, then all nodes of depth 3, etc.
   *  nodes are sequentially stored (bytepacked) in the ordering
   *  y*plane_size+x. The root node in depth 1 is stored in variable 
   *  root_state.
   */
  unsigned char* states;

  /// convenience variable: how many bytes alloced in states
  int state_size;

  /// This contains the info that designates thr position of a particular node.
  class node_pos_info { 
   public:
    int offset; // index into states array
                // the offset -1 denotes the root node.
    int bytepos; // index into byte. [0..3]
    int depth; // depth into tree 1==root
    int plane_size; // width and height of square plane of nodes.
    int plane_start; // index in array where the node's at depth are located
    int node_x, node_y; // coordinates of the node in it's plane.
                        // integer in the plane of leaves at node_depth.
    // note that above values are redundant.
    //  offset = plane_start + (node_y * plane_size + node_x) / 4
    //  bytepos = (node_y * plane_size + node_x) % 4
    //  plane_size = 2**(depth-1)
    //  plane_start = (4**(depth-1) -1 ) / 3
    // but it is easier to pass these values along.
  
    /// construct a pos info pointing to the root node.
    node_pos_info(void); 
    /// clear the node_pos_info, to point to root node.
    void set_root(void);
    /// copy constructor
    node_pos_info(const node_pos_info &copy);
  };

  /** this function is used for traversing the quadtree.
   *  it will get the nodes bounding box, state, and node_pos
   *  all these are this node's values.
   *  and custom clientdata.
   *  It can return an integer.
   *  node_depth==1 means you are the root, and so on,
   *  if the node_depth == max_depth you are a leaf.
   */
  typedef int (csQuadTree::*quad_traverse_func)(csQuadTree* pObj, 
    const csBox2& node_bbox, int node_state, node_pos_info *node_pos,
    void* data);

  /** private functions to help dealing with quadtree.
   *  Call all four children. Of the node at parent_pos,
   *  box is the bounding box of the parent node.
   *  each child will be passed the data.
   *  returns return values of children in retval.
   *  in order 0=topleft, 1=topright, 2=bottomleft, 3=bottomright.
   *  note that theoretically the state of the caller could be changed.
   *  func is of type quad_traverse_func.
   */
  void CallChildren(quad_traverse_func func, csQuadTree* pObj, 
    const csBox2& box, node_pos_info *parent_pos, void *data, int retval[4]);

  /** Convenience version, the retval argument is omitted, return values
   * are discarded.
   */
  void CallChildren(quad_traverse_func func, csQuadTree* pObj, 
    const csBox2& box, node_pos_info *parent_pos, void *data);

  /** Get the state of a node, in byte offset , nodenr given.
   *  offset -1 means root node.
   */
  int GetNodeState(int offset, int nodenr);

  /// convenience function to GetNodeState at node_pos_info
  inline int GetNodeState(node_pos_info *pos) 
    {return GetNodeState(pos->offset, pos->bytepos);}

  /** Set the state of a node, in byte offset , nodenr given.
   *  offset -1 means root node.
   */
  void SetNodeState(int offset, int nodenr, int newstate);

  /// convenience function to set a node state
  inline void SetNodeState(node_pos_info *pos, int newstate)
    {SetNodeState(pos->offset, pos->bytepos, newstate);}

  /** for a node, check if data (a csVector2 * )
   * is in this node, possibly recursing.
   * returning the QUAD_FULL/PARTIAL/EMPTY if it hits.
   * returning QUAD_UNKOWN if the point is not in the node.
   */
  int test_point_func(csQuadTree* pObj, const csBox2& node_bbox,
     int node_state, node_pos_info *node_pos, void* data);

  /** gather result from retval for testpoint
   */
  int GetTestPointResult(int retval[4]); 

  /** struct with info for polygon being inserted
   */
  struct poly_info {
    csVector2* verts;
    int num_verts;
    const csBox2* pol_bbox;
    bool test_only;
  };
  
  /** for a node, insert polygon into it (possibly in its children too)
   * expects data to be a struct poly_info*
   * returns true if the polygon covered previously empty space.
   *    thus, if the tree is modified. or if PARTIAL->PARTIAL.
   * Returns CS_QUAD_NOCHANGE if no change to the tree.
   *  CS_QUAD_POSSIBLECHANGE if it probably changed, and 
   *  CS_QUAD_CERTAINCHANGE if it changed for certain.
   */
  int insert_polygon_func(csQuadTree* pObj, const csBox2& node_bbox,
     int node_state, node_pos_info *node_pos, void* data);

  /** gather result from retval for insertpolygon_func
   *  NOCHANGE < POSSIBLECHANGE < CERTAINCHANGE
   */
  int GetPolygonResult(int retval[4]); 

  /** for a node, mark it by casting (void*)data to an int.
   *  that is the new state.
   */
  int mark_node_func (csQuadTree* pObj, const csBox2& node_bbox,
     int node_state, node_pos_info *node_pos, void* data);

  /** for a node, give it the (int) cast of data as value and
   *  give children a good value. CS_QUAD_UNKNOWN means the
   *  child's value is unchanged.
   */
  int propagate_func(csQuadTree* pObj, const csBox2& node_bbox,
     int node_state, node_pos_info *node_pos, void* data);

  /** Set the state of the node based on states of child nodes.
   * returns new node state, leaves only return their state.
   */
  int sift_func(csQuadTree* pObj, const csBox2& node_bbox,
     int node_state, node_pos_info *node_pos, void* data);

public:
  /** create a quadtree of depth, using about (4**depth-1)/3-1/3 bytes. 
   *  depth must be >= 1
   *  depth=1 is only the root.
   */
  csQuadTree(const csBox2& the_box, int the_depth);

  /// destroy quadtree
  ~csQuadTree();

  /**
   * Is the tree full?
   */
  bool IsFull () { return root_state == CS_QUAD_FULL; }

  /** 
   * Make the tree empty again
   */
  void MakeEmpty();

  /**
   * Insert a polygon into the quad-tree.
   * Return true if the tree was modified (i.e. if parts of the
   * polygon were visible. More precisely:
   * Returns CS_QUAD_NOCHANGE if no change to the tree.
   *  CS_QUAD_POSSIBLECHANGE if it probably changed, and 
   *  CS_QUAD_CERTAINCHANGE if it changed for certain.
   */
  int InsertPolygon (csVector2* verts, int num_verts,
  	const csBox2& pol_bbox);

  /**
   * Test for polygon visibility with the quad-tree.
   * Returns CS_QUAD_NOCHANGE if no change to the tree.
   *  CS_QUAD_POSSIBLECHANGE if it probably changed, and 
   *  CS_QUAD_CERTAINCHANGE if it changed for certain.
   */
  int TestPolygon (csVector2* verts, int num_verts,
  	const csBox2& pol_bbox);

  /**
   * Test if a given point is visible in the quad-tree.
   * Returns CS_QUAD_FULL if not visible, CS_QUAD_EMPTY
   * if visible and CS_QUAD_PARTIAL if undecided.
   * This function returns CS_QUAD_UNKNOWN if the point is not
   * in the quadtree.
   */
  int TestPoint (const csVector2& point);

  /**
   *  4x4 bit array (2bytes) -> Quadtree of depth 3.
   *  depth3 quadtree->4x4 array (partial -> empty)
   */

  /** a TestRectangle function that will test on leaf-coordinates, for full. 
   *  Give depth of the node-plane to test on, the x and y integer node coords
   *  of the topleft corner of the rectangle, and it's width and height.
   *  Returns true if and only if all nodes at that spot are CS_QUAD_FULL.
   *  Note: You must have called Propagate() before using this method.
   */
  bool TestRectangle(int depth, int x, int y, int w, int h);

  /** Propagate, makes sure that all nodes in the tree get a value
   * repesenting their state. For speed reasons this is not always the case
   * after an InsertPolygon() call.
   */
  void Propagate(void);

  /** The leaves must have valid values. The other nodes are
   *  given correct values.
   */
  void Sift(void);

  
  /** This function will print the quadtree...
   */
  void Print(void);
};


#endif /*QUADTREE_H*/

