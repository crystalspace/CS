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
   * dxdy and dydx are an array of edge gradients for the polygon.
   * hor_offs, and ver_offs are the offset for the (0,0) corner
   * of the node.
   */
  bool TestPolygon (csVector2* poly, int num_verts,
  	float* dxdy, float* dydx,
  	int hor_offs, int ver_offs) const;

  /**
   * Insert a polygon in this node (and children).
   * Returns true if polygon was visible (i.e. tree is modified).
   * dxdy and dydx are an array of edge gradients for the polygon.
   * hor_offs, and ver_offs are the offset for the (0,0) corner
   * of the node.
   */
  bool InsertPolygon (csVector2* poly, int num_verts,
  	float* dxdy, float* dydx,
  	int hor_offs, int ver_offs);

  /// Return the horizontal number of pixels for this node.
  static int GetHorizontalSize ()
  {
    return Child::GetHorizontalSize ()*csCovMaskTriage::GetHorizontalSize ();
  }

  /**
   * Update this node and all children to a polygon.
   * This function is similar to InsertPolygon() but it does
   * not look at the old contents of the node and children.
   * Instead it assumes the old contents was empty.
   * This function returns false if the polygon mask for
   * the top-level node was empty and true otherwise.
   */
  bool UpdatePolygon (csVector2* poly, int num_verts,
  	float* dxdy, float* dydx,
  	int hor_offs, int ver_offs);

  /// Return the vertical number of pixels for this node.
  static int GetVerticalSize ()
  {
    return Child::GetVerticalSize ()*csCovMaskTriage::GetVerticalSize ();
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
   * dxdy and dydx are an array of edge gradients for the polygon.
   * hor_offs, and ver_offs are the offset for the (0,0) corner
   * of the node.
   */
  bool TestPolygon (csVector2* poly, int num_verts,
  	float* dxdy, float* dydx,
  	int hor_offs, int ver_offs) const;

  /**
   * Insert a polygon in this node.
   * Returns true if polygon was visible (i.e. mask is modified).
   * dxdy and dydx are an array of edge gradients for the polygon.
   * hor_offs, and ver_offs are the offset for the (0,0) corner
   * of the node.
   */
  bool InsertPolygon (csVector2* poly, int num_verts,
  	float* dxdy, float* dydx,
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
  	float* dxdy, float* dydx,
  	int hor_offs, int ver_offs);

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

