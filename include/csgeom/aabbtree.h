/*
  Copyright (C) 2008 by Marten Svanfeldt

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_CSGEOM_AABBTREE_H__
#define __CS_CSGEOM_AABBTREE_H__

#include "csutil/blockallocator.h"
#include "csgeom/box.h"

namespace CS
{
namespace Geometry //@@Right?
{

  /**
   * 
   */
  template<
    typename ObjectType, 
    unsigned int objectsPerLeaf
  >
  class AABBTree
  {
  public:
    ///
    enum 
    {
      AABB_NODE_INNER = 0x0,
      AABB_NODE_LEAF = 0x1,
      AABB_NODE_TYPE_MASK = 0x1,

      AABB_NODE_FLAG_SHIFT = 0x10,
      AABB_NODE_FLAG_MASK = 0xFFFF0000
    };

    ///
    class Node;
    
    ///
    AABBTree ()
      : rootNode (0)
    {}

    ///
    ~AABBTree ()
    {
      nodeAllocator.DeleteAll ();
    }

    /**
     * 
     */
    void AddObject (const ObjectType& object);

    /**
     * 
     */
    void RemoveObject (const ObjectType& object);

  private:
    typedef csBlockAllocator<
      Node, 
      CS::Memory::AllocatorAlign<32>, 
      csBlockAllocatorSizeObjectAlign<32>
    > NodeAllocatorType;

    ///
    NodeAllocatorType nodeAllocator;

    /// 
    Node* rootNode;
  };

  /**
   * 
   */
  template<
    typename ObjectType, 
    unsigned int objectsPerLeaf
  >
  class AABBTree<ObjectType, objectsPerLeaf>::Node
  {
  public:
    Node (bool isLeaf = false)
      : typeAndFlags (isLeaf ? AABB_NODE_LEAF : AABB_NODE_INNER)
    {}

    // General accessors
    ///
    uint GetType () const
    {
      return (typeAndFlags & AABB_NODE_TYPE_MASK);
    }

    uint GetFlags () const
    {
      return typeAndFlags >> AABB_NODE_FLAG_SHIFT;
    }

    ///
    void SetFlags (uint newFlags)
    {
      typeAndFlags = (typeAndFlags & ~AABB_NODE_FLAG_MASK) | 
        (newFlags << AABB_NODE_FLAG_SHIFT);
    }

    ///
    const csBox3& GetBBox () const
    {
      return boundingBox;
    }

    ///
    void SetBBox (const csBox3& box)
    {
      boundingBox = box;
    }

    // Accessor for inner node data
    /// 
    Node* GetChild1 () const
    {
      CS_ASSERT(GetType() == AABB_NODE_INNER);
      return children[0];
    }

    /// 
    Node* GetChild2 () const
    {
      CS_ASSERT(GetType() == AABB_NODE_INNER);
      return children[1];
    }

    // Accessor for leaf node data
    ///
    const ObjectType& GetLeafData (size_t index) const
    {
      CS_ASSERT(GetType() == AABB_NODE_INNER);
      CS_ASSERT(index < objectsPerLeaf);

      return leafStorage[index];
    }

    ///
    ObjectType& GetLeafData (size_t index)
    {
      CS_ASSERT(GetType() == AABB_NODE_INNER);
      CS_ASSERT(index < objectsPerLeaf);

      return leafStorage[index];
    }

  private:
    ///
    uint32 typeAndFlags;
    ///
    csBox3 boundingBox;

    ///
    union
    {
      ObjectType leafStorage[objectsPerLeaf];
      Node* children[2];
    };
  };


}
}

#endif
