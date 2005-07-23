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

#include "csgeom/box.h"
#include "csutil/array.h"
#include "csutil/blockallocator.h"


class csKDBuildTree;
struct csKDBuildTreeNode;
class csLightingMesh;
class csMeshPatch;


/**
 * KDTree node (either inner or leafnode).
 * This is a node in the highly optimized tree used for runtime traversal
 * and should therefor not be changed without performance taking into
 * concideration. See [Wald01], section 7.2.
 */
union csKDTreeNode
{
  /**
   * KDTree node for a leaf (child having primitives) 
   */
  struct
  {
    /**
     * Combined flag, dimension and pointer
     *  Bit 2            Flag indicating node/inner leaf (node=0,inner=1) 
     *  Bit 3..length    Pointer to list of primitives
     */
    uintptr_t flagAndOffset;

    /**
     * Number of primitives in node 
     */
    size_t numberOfPrimitives;
  } leaf;

  /**
   * KDTree node for a inner node
   */
  struct
  {
    /**
     * Combined flag, dimension and pointer
     *  Bit 0..1         Split dimension (0=x, 1=y, 2=z)
     *  Bit 2            Flag indicating node/inner leaf (node=0,inner=1)
     *  Bit 3..length-1  Offset to first child
     */
    uintptr_t flagDimensionAndOffset;

    /**
     * Split amount 
     */
    float splitLocation;
  } inner;
};

// Helper functions to get and set data in the kdtree-node
struct csKDTreeNodeH
{
  static void SetFlag (csKDTreeNode *node, bool inner=false)
  {
    if (inner)
      node->leaf.flagAndOffset |= 0x4;
    else
      node->leaf.flagAndOffset &= ~0x4;
  }

  static bool GetFlag (const csKDTreeNode *node)
  {
    return node->leaf.flagAndOffset & 0x4;
  }

  static void SetDimension (csKDTreeNode *node, uint dim)
  {
    node->leaf.flagAndOffset = (node->leaf.flagAndOffset & ~0x3) | dim;
  }

  static uint GetDimension (const csKDTreeNode *node)
  {
    return (node->leaf.flagAndOffset & 0x3);
  }

  static void SetPointer (csKDTreeNode *node, uintptr_t ptr)
  {
    node->leaf.flagAndOffset = (node->leaf.flagAndOffset & 0x7) | ptr;
  }

  static uintptr_t GetPointer (const csKDTreeNode *node)
  {
    return (node->leaf.flagAndOffset & ~0x7);
  }
};

/**
 * An optimized KD-tree for raytracing among triangles. 
 */
class csKDTree
{
public:
  /// Constructor
  csKDTree ();

  /// Destructor
  ~csKDTree ();

  /// Build our optimized tree from a "build tree"
  void BuildTree (csKDBuildTree *buildTree);

  /// Transfer one node from the build tree to the real one
  void TransferNode (csKDBuildTreeNode *fromNode, csKDTreeNode *tonode);

  /// Allocator for nodes.
  csBlockAllocator<csKDTreeNode, csBlockAllocatorAlignPolicy<32> > nodeAllocator;

  /// Pointer to root node
  csKDTreeNode *rootNode;

  /// Bounding box of entier tree
  csBox3 boundingBox;
};

// Helper used when trying to split a tree
struct SplitPositionFixer
{
  SplitPositionFixer (size_t num)
  {
    //Alloc num entries
    splitPoolDEL = splitPool = new SplitPosition[num*2+8];
    for (uint i=0; i<(num*2+6); i++)
    {
      splitPool[i].next = &splitPool[i+1];
    }
    splitPool[num*2+7].next = 0;
  }

  ~SplitPositionFixer ()
  {
    //dealloc all entries
    delete[] splitPoolDEL;
  }

  // Prepare for a new node
  void Reset () 
  {
    if (firstEntry)
    {
      SplitPosition *list = firstEntry;
      while (list->next) list = list->next;
      list->next = splitPool;
      splitPool = firstEntry;
    }
    firstEntry = 0;
  }

  // Add a new possible splitpoint
  void AddPosition (float pos)
  {
    SplitPosition* entry = splitPool;
    splitPool = splitPool->next;

    entry->next = 0;
    entry->position = pos;
    if (!firstEntry)
      firstEntry = entry;
    else
    {
      if (pos < firstEntry->position)
      {
        entry->next = firstEntry;
        firstEntry = entry;
      }
      else if (pos == firstEntry->position)
      {
        //recycle, no need to store this position
        entry->next = splitPool;
        splitPool = entry;
      }
      else
      {
        SplitPosition *list = firstEntry;
        while ((list->next) && (pos >= list->next->position))
        {
          if (pos == list->next->position)
          {
            //recycle, no need to store this position
            entry->next = splitPool;
            splitPool = entry;
            return;
          }
          list = list->next;
        }
        entry->next = list->next;
        list->next = entry;
      }
    }
  }

  // Holder for possible splitpoint
  struct SplitPosition
  {
    float position;
    SplitPosition* next;
  };
  SplitPosition  *splitPoolDEL, *splitPool, *firstEntry;
};

/// A node in the build-tree
struct csKDBuildTreeNode
{
  /// List of patches in this node.
  csArray<csMeshPatch*> patches;

  /// Split dimension
  uint splitDimension;

  /// Split location
  float splitLocation;

  /// Boundingbox
  csBox3 boundingBox;

  /// Child nodes
  csKDBuildTreeNode *left, *right;

  void Subdivide (SplitPositionFixer *sphelper);
};

/**
 * KD-tree used to build the optimized tree 
 */
class csKDBuildTree
{
public:
  /// Constructor
  csKDBuildTree ();

  /// Destructor
  ~csKDBuildTree ();

  /// Build KD-tree from a list of objects
  void BuildTree (const csArray<csLightingMesh*>& meshes);

private:
  csKDBuildTreeNode* rootNode;

  friend class csKDTree;
};

#endif
