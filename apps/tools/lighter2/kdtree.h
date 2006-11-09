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


  /// Optimized KD-tree primitive
  struct KDTreePrimitive_Opt
  {
    /// Normal u, v and d components. Normalized.
    float normal_U, normal_V, normal_D;

    /// Index of biggest normal component
    int32 normal_K;

    /// Edge A u, v and d components
    float edgeA_U, edgeA_V, edgeA_D;

    /// The RadPrimitive we belong to
    RadPrimitive *primPointer; 

    /// Edge B u, v and d components
    float edgeB_U, edgeB_V, edgeB_D;

    /// Padding
    float pad0;
  };

  /// Optimized KD-tree node
  union KDTreeNode_Opt
  {
    struct
    {
      /**
       * Combined flag, dimension and pointer
       *  Bit 2            Flag indicating node/inner leaf (leaf=0,inner=1)
       *  Bit 3..length    Pointer to list of primitives
       */
      uintptr_t flagAndOffset;

      /**
       * Number of primitives in node
       */
      size_t numberOfPrimitives; 
    } leaf;

    struct 
    {
      /**
       * Combined flag, dimension and pointer
       *  Bit 0..1         Split dimension (0=x, 1=y, 2=z)
       *  Bit 2            Flag indicating node/inner leaf (leaf=0,inner=1)
       *  Bit 3..length-1  Offset to first child
       */
      uintptr_t flagDimensionAndOffset;

      /// Split location
      float splitLocation;
    } inner;
  };

  /// Optimized KD-tree
  struct KDTree_Opt
  {
    /// Constructor
    KDTree_Opt ()
      : nodeList (0), primitives (0)
    {
    }

    /// Destructor
    ~KDTree_Opt ();

    /// Bounding box
    csBox3 boundingBox;

    /// Nodes, nodeList[0] is root
    KDTreeNode_Opt* nodeList;

    /// Primitives
    KDTreePrimitive_Opt* primitives;
  };


  class KDTreeBuilder
  {
  public:
    /// Build a tree from all the objects returned by iterator
    static KDTree* BuildTree (const RadObjectHash::GlobalIterator& objectIt);

    /// Build an optimized tree from a normal one
    static KDTree_Opt* OptimizeTree (KDTree* tree);

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
    static bool CollectPrimitives (const KDTree_Opt *tree, 
      RadPrimitivePtrArray& primArray, const csBox3& overlapAABB);
  
  private:
    KDTreeHelper ();
    KDTreeHelper (const KDTreeHelper& o);

    // Traverse a node, collect any prims within AABB
    static void CollectPrimitives (const KDTree_Opt *tree, const KDTreeNode_Opt* node, 
      csBox3 currentBox, RadPrimitivePtrSet& outPrims, const csBox3& overlapAABB);
  };

}

#endif

