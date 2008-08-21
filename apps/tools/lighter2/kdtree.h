/*
  Copyright (C) 2005-2006 by Marten Svanfeldt

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

// For debugging:
//#define KDTREE_ASSERT(x)    CS_ASSERT(x)
// For a tad more speed:
#define KDTREE_ASSERT(x)  (void)0

#include "csutil/compileassert.h"

#include "statistics.h"

namespace lighter
{
  class Primitive;
  class Object;

  enum
  {
    KDPRIM_FLAG_NOSHADOW = 0x04,
    KDPRIM_FLAG_TRANSPARENT = 0x08,
    KDPRIM_FLAG_MASK = 0xFFFFFFFC
  };

  /**
  * Optimized KD-tree primitive
  * \todo
  * Make more efficient on 64 bit platforms!
  */
  struct KDTreePrimitive
  {
    /// Normal u, v and d components. Normalized.
    float normal_U, normal_V, normal_D;

    /// Index of biggest normal component, and flags
    int32 normal_K;

    /// Edge A u, v and d components
    float edgeA_U, edgeA_V, edgeA_D;

    /// The Primitive we belong to
    Primitive *primPointer; 

    /// Edge B u, v and d components
    float edgeB_U, edgeB_V, edgeB_D;

#if (CS_PROCESSOR_SIZE == 32)
    /// Padding
    float pad0;
#endif
  };

  /// Optimized KD-tree node
  union KDTreeNode
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
      *  Bit 3..length-1  Pointer to first child
      */
      uintptr_t flagDimensionAndOffset;

      /// Split location
      float splitLocation;
    } inner;
  };

  /// Optimized KD-tree
  class KDTree
  {
  public:
    /// Constructor
    KDTree ()
      : nodeList (0), primitives (0)
    {
    }

    /// Destructor
    ~KDTree ();

    /// Bounding box
    csBox3 boundingBox;

    /// Nodes, nodeList[0] is root
    KDTreeNode* nodeList;

    /// Primitives
    KDTreePrimitive* primitives;
  };

  /// Helper to manipulate KDTreeNode
  class KDTreeNode_Op
  {
  public:
    static CS_FORCEINLINE void SetLeaf (KDTreeNode* node, bool leaf)
    {
      if (leaf)
        node->inner.flagDimensionAndOffset &= ~0x04;
      else
        node->inner.flagDimensionAndOffset |= 0x04;
    }

    static CS_FORCEINLINE bool IsLeaf (const KDTreeNode* node)
    {
      return (node->inner.flagDimensionAndOffset & 0x04) == 0;
    }

    static CS_FORCEINLINE void SetPrimitiveList (KDTreeNode* node, 
      KDTreePrimitive* primList)
    {
      KDTREE_ASSERT (IsLeaf (node));
      const uintptr_t primListUI = reinterpret_cast<uintptr_t> (primList);
      KDTREE_ASSERT ((primListUI & 0x07) == 0);
      node->leaf.flagAndOffset = (node->leaf.flagAndOffset & 0x07) |
        primListUI;
    }

    static CS_FORCEINLINE KDTreePrimitive* GetPrimitiveList 
      (const KDTreeNode* node)
    {
      return reinterpret_cast<KDTreePrimitive*>
        (node->leaf.flagAndOffset & ~0x07);
    }

    static CS_FORCEINLINE void SetPrimitiveListSize (KDTreeNode* node, 
      size_t size)
    {
      KDTREE_ASSERT (IsLeaf (node));
      node->leaf.numberOfPrimitives = size;
    }

    static CS_FORCEINLINE size_t GetPrimitiveListSize (const KDTreeNode* node)
    {
      return node->leaf.numberOfPrimitives;
    }

    static CS_FORCEINLINE void SetDimension (KDTreeNode* node, uint dim)
    {
      KDTREE_ASSERT (!IsLeaf (node));
      node->inner.flagDimensionAndOffset =
        (node->inner.flagDimensionAndOffset & ~0x03) | dim;
    }

    static CS_FORCEINLINE uint GetDimension (const KDTreeNode* node)
    {
      return node->inner.flagDimensionAndOffset & 0x03;
    }

    static CS_FORCEINLINE void SetLeft (KDTreeNode* node, KDTreeNode* left)
    {
      KDTREE_ASSERT (!IsLeaf (node));
      const uintptr_t leftUI = reinterpret_cast<uintptr_t> (left);
      KDTREE_ASSERT ((leftUI & 0x07) == 0);
      node->inner.flagDimensionAndOffset =
        (node->inner.flagDimensionAndOffset & 0x07) | leftUI;
    }

    static CS_FORCEINLINE KDTreeNode* GetLeft (const KDTreeNode* node)
    {
      return reinterpret_cast<KDTreeNode*>
        (node->inner.flagDimensionAndOffset & ~0x07); 
    }

    static CS_FORCEINLINE void SetLocation (KDTreeNode* node, float loc)
    {
      KDTREE_ASSERT (!IsLeaf (node));
      node->inner.splitLocation = loc;
    }

    static CS_FORCEINLINE float GetLocation (const KDTreeNode* node)
    {
      return node->inner.splitLocation;
    }

  };

  class KDTreeBuilder
  {
  public:
    KDTreeBuilder ();

    /*
    Take an object iterator and build a kd-tree from that
    */
    KDTree* BuildTree (csHash<csRef<Object>, csString>::GlobalIterator& objects,
      Statistics::Progress& progress);

  private:

    // Building constants
    enum
    {
      TRAVERSAL_COST = 1,
      INTERSECTION_CONST = 6,
      MAX_DEPTH = 60,
      PRIMS_PER_LEAF = 4,
      NODE_SIZE_EPSILON = 1000 //*1e-9
    };

    //Internal node representing a kd-tree node (inner or leaf) while building
    struct KDNode
    {
      // CS_AXIS_* 
      uint splitDimension;

      // Split plane location (world space)
      float splitLocation;

      // Left-right pointer
      KDNode *leftChild, *rightChild;

      // Primitives
      csArray<Primitive*> primitives;

      KDNode()
        : splitDimension(0), splitLocation(0.0f), leftChild(0), rightChild(0)
      {}
    };

    class PrimBox;
    // Represent an end-point of triangle along given axis
    class EndPoint
    {
    public:
      // Get/Set helpers
      EndPoint* GetNext (size_t a) const
      {
        return reinterpret_cast<EndPoint*> (data[a] & ~((size_t)0x03));
      }

      void SetNext (size_t a, EndPoint* n)
      {
        data[a] = GetSide (a) | reinterpret_cast<size_t> (n);
      }

      size_t GetSide (size_t a) const
      {
        return data[a] & 0x03;
      }

      void SetSide (size_t a, size_t s)
      {
        data[a] = s | reinterpret_cast<size_t> (GetNext (a));
      }

      float GetPosition (size_t a) const
      {
        return position[a];
      }

      void SetPosition (size_t a, float pos)
      {
        position[a] = pos;
      }

      /*
      Get the (probable) location of the primbox by realigning the this pointer
      to an even 64/128 byte boundary.
      Can be dangerous, use with care!
      */
      PrimBox* GetBox ()
      {
        return reinterpret_cast<PrimBox*> ( reinterpret_cast<size_t> (this) &
          BOX_ALIGN);
      }

      enum
      {
        SIDE_END = 0,
        SIDE_PLANAR = 1,
        SIDE_INVALID = 2,
        SIDE_START = 3
      };

    private:

      // Data
      size_t data[3]; //Side and pointer to next endpoint. Side use two lowest bits
      float position[3];

#if (CS_PROCESSOR_SIZE == 64)
      enum {BOX_ALIGN = ~0x7f};      
#else
      enum {BOX_ALIGN = ~0x3f};
#endif
    }; // 

    //Represent a primitive as an AABB
    class PrimBox
    {
    public:
      PrimBox ()
        : primitive (0), clone (0), flags (STATE_STRADDLING)
      {
        //Setup side cross references
        for (size_t i = 0; i < 3; ++i)
        {
          side[0].SetNext (i, &side[1]);
          side[0].SetSide (i, EndPoint::SIDE_START);

          side[1].SetNext (i, 0);
          side[1].SetSide (i, EndPoint::SIDE_END);
        } 
      }

      enum
      {
        STATE_LEFT = 0,
        STATE_RIGHT,
        STATE_STRADDLING,
        STATE_PROCESSED
      };

      //End-point [min/max]
      EndPoint side[2];             //48 / 80
      Primitive* primitive;         // 4 /  8     
      PrimBox* clone;               // 4 /  8 
      uint8 pad[4];                 // 4 /  4

      int32 flags;                  // 4 /  4
#if (CS_PROCESSOR_SIZE == 64)
      //Bit unfortunate, but we need an even pot-2 alignment
      uint8 pad0[24];               // 0 / 24
#endif
    }; //sizeof = 64 / 128
#if (CS_PROCESSOR_SIZE == 64)
    CS_COMPILE_ASSERT(sizeof (PrimBox) == 128);
#else
    CS_COMPILE_ASSERT(sizeof (PrimBox) == 64);
#endif


    // Helper class for clipping primitives and computing primitive AABB
    struct PrimHelper
    {
      PrimHelper ()
      {
      }

      //Init with a Prrimitive
      void Init (const Primitive* prim);

      // Clip to an AABB
      void Clip (const csBox3& box);
      bool ClipAxis (float pos, float direction, size_t axis);

      void UpdateBB ();

      csBox3 aabb;
      csVector3 vertices[10];
      csVector3 tempVertices[10];
      size_t numVerts, numTempVerts;
    } primHelper;

    /**
    * Holder for the sorted end point lists.
    * The lists are sorted by position followed by side.
    */
    struct EndPointList
    {
      void Insert (size_t axis, EndPoint* ep);
      void Remove (size_t axis, EndPoint* ep);
      void SortList (size_t axis);

      EndPoint* head[3];
      EndPoint* tail[3];

      EndPointList ()
      {
        head[0] = head[1] = head[2] = 0;
        tail[0] = tail[1] = tail[2] = 0;
      }
    } endPointList;

    //Allocator for boxes
    typedef csBlockAllocator<PrimBox, CS::Memory::AllocatorAlign<128> >
      BoxAllocatorType;

    BoxAllocatorType boxAllocator;

    //Allocator for internal kdtree-nodes
    typedef csBlockAllocator<KDNode> NodeAllocatorType;
    NodeAllocatorType nodeAllocator;

    //Object extents, primitive count etc collected on first pass
    csBox3 objectExtents;
    size_t numPrimitives;

    //Private functions
    bool SetupEndpoints (csHash<csRef<Object>, csString>::GlobalIterator& objects);
    bool BuildKDNodeRecursive (EndPointList* epList, KDNode* node, 
      csBox3 aabb, size_t numPrim, size_t treeDepth);
    KDTree* SetupRealTree (KDNode* rootNode);

    friend struct CopyFunctor;
    friend struct CountFunctor;
  };

  // Helper to do operations on a kd-tree
  class KDTreeHelper
  {
  public:

    // Collect all primitives within given AABB
    static bool CollectPrimitives (const KDTree *tree, 
      csArray<Primitive*>& primArray, const csBox3& overlapAABB);

  private:
    KDTreeHelper ();
    KDTreeHelper (const KDTreeHelper& o);

    // Traverse a node, collect any prims within AABB
    static void CollectPrimitives (const KDTree *tree, const KDTreeNode* node, 
      csBox3 currentBox, csSet<Primitive*>& outPrims, const csBox3& overlapAABB);
  };

}

#undef KDTREE_ASSERT

#endif

