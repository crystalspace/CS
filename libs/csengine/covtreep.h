/*
    Copyright (C) 1999 by Jorrit Tyberghein
  
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

#ifndef COVTREEP_H
#define COVTREEP_H

// !!! NOTE !!! DON'T MOVE THIS INCLUDE FILE TO 'include'!!!
// IT IS MENT TO BE PRIVATE.

struct iGraphics2D;

#include "csengine/covmask.h"

/**
 * This templated class represents a node in the coverage
 * mask tree. It is templated because we don't want to use
 * any pointers. So we need to be able to nest classes and
 * this be done comfortably with the template approach
 * used below. Note that this is a private include file which
 * can only be included by libs/csengine/covtree.cpp.
 */
template <class Child>
class csCovTreeNode : public csCovMaskTriage
{
private:
  /**
   * The children of this node.
   * For every bit in the mask there is one child.
   */
  Child children[CS_CM_BITS];

public:
  /**
   * Test a polygon against this node (and children).
   * Returns true if polygon is visible.
   * hor_offs, and ver_offs are the offset for the (0,0) corner
   * of the node.
   */
  bool TestPolygon (csVector2* poly, int num_verts,
	csCovEdgeInfo* edges,
  	int hor_offs, int ver_offs) const;

  /**
   * Insert a polygon in this node (and children).
   * Returns true if polygon was visible (i.e. tree is modified).
   * hor_offs, and ver_offs are the offset for the (0,0) corner
   * of the node.
   */
  bool InsertPolygon (csVector2* poly, int num_verts,
	csCovEdgeInfo* edges,
  	int hor_offs, int ver_offs);

  /**
   * Update this node and all children to a polygon.
   * This function is similar to InsertPolygon() but it does
   * not look at the old contents of the node and children.
   * Instead it assumes the old contents was empty.
   * This function returns false if the polygon mask for
   * the top-level node was empty and true otherwise.
   */
  bool UpdatePolygon (csVector2* poly, int num_verts,
	csCovEdgeInfo* edges,
  	int hor_offs, int ver_offs);

  /**
   * This function is similar to UpdatePolygon() but it
   * inverts the polygon mask first. So it has the effect
   * of updating for the inverted polygon.
   */
  bool UpdatePolygonInverted (csVector2* poly, int num_verts,
	csCovEdgeInfo* edges,
  	int hor_offs, int ver_offs);

  /**
   * Test if a polygon is not empty for the given area on the
   * coverage mask tree without actually looking at the contents
   * of the tree. This is useful for testing polygon in empty
   * parts of the tree which are not defined.
   */
  bool TestPolygonNotEmpty (csVector2* poly, int num_verts,
	csCovEdgeInfo* edges,
  	int hor_offs, int ver_offs) const;

  /**
   * Make this tree invalid. This will set the state of
   * this node to 'invalid' and also the state of all children.
   * This can be used for debugging purposes but serves no other
   * useful purpose.
   */
  void MakeInvalid ();

  /**
   * Test consistancy for this node. Consistance can be broken
   * in the following cases:<br>
   * <ul>
   * <li>Mask is invalid.
   * <li>A bit is partial while the corresponding child isn't.
   * </ul>
   * The results are printed on standard output.
   * This function returns false if it found an error. In that case
   * it will stop searching for further errors.
   */
  bool TestConsistency (int hor_offs, int ver_offs) const;

  /// Return the horizontal/vertical number of pixels for this node.
  static int GetPixelSize ()
  {
    return Child::GetPixelSize ()*csCovMaskTriage::GetPixelSize ();
  }

  /// Return the horizontal/vertical number of pixels for this node (shift).
  static int GetPixelShift ()
  {
    return Child::GetPixelShift ()+csCovMaskTriage::GetPixelShift ();
  }

  /**
   * Do a graphical dump of the coverage mask tree
   * upto the specified level.
   */
  void GfxDump (iGraphics2D* ig2d, int level, int hor_offs, int ver_offs);
};

/**
 * A subclass of csCovMask also implementing
 * TestPolygon/InsertPolygon.
 */
class csCovTreeNode0 : public csCovMask
{
public:
  /**
   * Test a polygon against this node.
   * Returns true if polygon is visible.
   * hor_offs, and ver_offs are the offset for the (0,0) corner
   * of the node.
   */
  bool TestPolygon (csVector2* poly, int num_verts,
	csCovEdgeInfo* edges,
  	int hor_offs, int ver_offs) const;

  /**
   * Insert a polygon in this node.
   * Returns true if polygon was visible (i.e. mask is modified).
   * hor_offs, and ver_offs are the offset for the (0,0) corner
   * of the node.
   */
  bool InsertPolygon (csVector2* poly, int num_verts,
	csCovEdgeInfo* edges,
  	int hor_offs, int ver_offs);

  /**
   * Update this node to a mask.
   * This function is similar to InsertPolygon() but it does
   * not look at the old contents of the node.
   * Instead it assumes the old contents was empty.
   * This function returns false if the polygon mask for
   * the top-level node was empty and true otherwise.
   */
  bool UpdatePolygon (csVector2* poly, int num_verts,
	csCovEdgeInfo* edges,
  	int hor_offs, int ver_offs);

  /**
   * This function is similar to UpdatePolygon() but it
   * inverts the polygon mask first. So it has the effect
   * of updating for the inverted polygon.
   */
  bool UpdatePolygonInverted (csVector2* poly, int num_verts,
	csCovEdgeInfo* edges,
  	int hor_offs, int ver_offs);
  /**
   * Test if a polygon is not empty for the given area on the
   * coverage mask tree without actually looking at the contents
   * of the tree. This is useful for testing polygon in empty
   * parts of the tree which are not defined.
   */
  bool TestPolygonNotEmpty (csVector2* poly, int num_verts,
	csCovEdgeInfo* edges,
  	int hor_offs, int ver_offs) const;

  /**
   * Make this node invalid. This is a do-nothing function
   * here because a csCovMask does not have an invalid state.
   */
  void MakeInvalid () { }

  /**
   * Test consistancy for this node. This is an empty function
   * as a csCovMask cannot be inconsistant.
   */
  bool TestConsistency (int, int) const { return true; }

  /**
   * Do a graphical dump of the coverage mask tree
   * upto the specified level.
   */
  void GfxDump (iGraphics2D* ig2d, int level, int hor_offs, int ver_offs);
};

typedef csCovTreeNode<csCovTreeNode0> csCovTreeNode1;
typedef csCovTreeNode<csCovTreeNode1> csCovTreeNode2;
typedef csCovTreeNode<csCovTreeNode2> csCovTreeNode3;
typedef csCovTreeNode<csCovTreeNode3> csCovTreeNode4;
typedef csCovTreeNode<csCovTreeNode4> csCovTreeNode5;

#endif /*COVTREEP_H*/

