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
class csCovTreeNode
{
private:
  /**
   * The children of this node.
   * For every bit in the mask there is one child.
   */
  Child children[CS_CM_BITS];

  /// The visibility mask for this node.
  csCovMaskTriage mask;

public:
  /**
   * Test a polygon against this node (and children).
   * Returns true if polygon is visible.
   */
  bool TestPolygon (csVector2* poly, int num_verts) const;

  /**
   * Insert a polygon in this node (and children).
   * Returns true if polygon was visible (i.e. tree is modified).
   */
  bool InsertPolygon (csVector2* poly, int num_verts);
};

/**
 * A subclass of csCovMaskSingle also implementing
 * TestPolygon/InsertPolygon.
 */
class csCovTreeNode0 : public csCovMask
{
public:
  /**
   * Test a polygon against this node.
   * Returns true if polygon is visible.
   */
  bool TestPolygon (csVector2* poly, int num_verts) const;

  /**
   * Insert a polygon in this node.
   * Returns true if polygon was visible (i.e. mask is modified).
   */
  bool InsertPolygon (csVector2* poly, int num_verts);
};

typedef csCovTreeNode<csCovTreeNode0> csCovTreeNode1;
typedef csCovTreeNode<csCovTreeNode1> csCovTreeNode2;
typedef csCovTreeNode<csCovTreeNode2> csCovTreeNode3;
typedef csCovTreeNode<csCovTreeNode3> csCovTreeNode4;
typedef csCovTreeNode<csCovTreeNode4> csCovTreeNode5;

#endif /*COVTREEP_H*/

