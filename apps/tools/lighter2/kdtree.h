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


  /// Single primitive in KD-tree. A single triangle
  struct KDTreePrimitive
  {
    /// The three vertices
    csVector3 vertices[3];

    /// Normal
    csVector3 normal;

    /// The RadPrimitive we belong to
    RadPrimitive *primPointer;
  };

  
  /// Node in non-optimized KD-tree
  struct KDTreeNode
  {
    // CS_AXIS_* 
    uint splitDimension;

    // Split plane location (world space)
    float splitLocation;

    // Left-right pointer
    KDTreeNode *leftChild, *rightChild;

    // Triangle indices into triangle lists
    csArray<size_t> triangleIndices;

    KDTreeNode()
      : splitDimension(0), splitLocation(0.0f), leftChild(0), rightChild(0)
    {}

    ~KDTreeNode()
    { 
      delete leftChild;
      delete rightChild;
    }
  };

  /// KD-tree
  struct KDTree
  {
    // Bounding box
    csBox3 boundingBox;

    // Root node
    KDTreeNode *rootNode;

    // All triangles in tree
    csArray<KDTreePrimitive> allTriangles;

    KDTree()
      : rootNode (0)
    {}
  };


  class KDTreeBuilder
  {
  public:
    // Build a tree from all the objects returned by iterator
    static KDTree* BuildTree (const RadObjectHash::GlobalIterator& objectIt);

  protected:
    static void Subdivide(KDTree *tree, KDTreeNode *node, 
      const csBox3& currentAABB, unsigned int depth = 0, unsigned int lastAxis = ~0,
      float lastPosition = 0);
  };

  // Helper to do operations on a kd-tree
  class KDTreeHelper
  {
  public:

    // Collect all primitives within given AABB
    static bool CollectPrimitives (const KDTree *tree, RadPrimitivePtrArray& primArray,
      const csBox3& overlapAABB);
  
  private:
    KDTreeHelper ();
    KDTreeHelper (const KDTreeHelper& o);

    // Traverse a node, collect any prims within AABB
    static void CollectPrimitives (const KDTree *tree, const KDTreeNode* node, 
      csBox3 currentBox, RadPrimitivePtrSet& outPrims, const csBox3& overlapAABB);
  };

}

#endif

