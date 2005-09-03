/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __KDTREE_H__
#define __KDTREE_H__

#include "radobject.h"

namespace lighter
{
  class RadPrimitive;

  /// Single node in the KD tree
  struct KDTreeNode
  {
    // CS_AXIS_* 
    uint splitDimension;

    // Splitplane location (worldspace)
    float splitLocation;

    // Left-right pointer
    KDTreeNode *leftChild, *rightChild;

    // BBox
    csBox3 boundingBox;

    // All our primitives (or well, pointers to them)
    RadPrimitivePtrArray radPrimitives;

    KDTreeNode ()
      : splitDimension (CS_AXIS_NONE), splitLocation (0.0f),
      leftChild (0), rightChild (0)
    {}

    ~KDTreeNode ()
    {
      delete leftChild;
      delete rightChild;
    }

    // Subdivide node
    void Subdivide ();
  };


  class KDTree
  {
  public:
    KDTree () 
      : rootNode (0)
    {
    }

    ~KDTree ()
    {
      delete rootNode;
    }

    // Build a tree from all the objects returned by iterator
    void BuildTree (const RadObjectHash::GlobalIterator& objectIt);

    /* Return all primitives on positive side of plane. The returned array is only
    valid at most until next call to GetPrimitives */
    RadPrimitivePtrArray& GetPrimitives (const csPlane3 &plane);

    // Root node of the tree
    KDTreeNode *rootNode;
  private:
    RadPrimitivePtrArray tempPrimitiveArray;
  };

}

#endif

