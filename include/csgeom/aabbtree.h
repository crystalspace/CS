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
#include "csutil/dirtyaccessarray.h"

namespace CS
{
namespace Geometry //@@Right?
{
  template<typename ObjectType>
  struct AABBTreeNodeExtraDataNone
  {
    void LeafAddObject (ObjectType*) {}
    void LeafUpdateObjects (ObjectType**, uint) {}
    void NodeUpdate (const AABBTreeNodeExtraDataNone& child1,
      const AABBTreeNodeExtraDataNone& child2) {}
  };

  /**
   * 
   */
  template<
    typename ObjectType, 
    unsigned int objectsPerLeaf = 1,
    typename NodeExtraData = AABBTreeNodeExtraDataNone<ObjectType>
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

      AABB_NODE_FLAG_SHIFT = 0x08,
      AABB_NODE_FLAG_MASK = 0xFF00
    };

    ///
    class Node;
    
    ///
    AABBTree ()
      : rootNode (0)
    {
      rootNode = AllocNode ();
      rootNode->SetLeaf (true);
    }

    ///
    ~AABBTree ()
    {
    #ifdef CS_DEBUG
      // Destroy only pointers "in use" to track down leaking nodes
      DeleteNodeRecursive (rootNode);
    #else
      // Don't bother with tree traversal, just free all nodes
      nodeAllocator.DeleteAll ();
    #endif
    }

    /**
     * 
     */
    void AddObject (ObjectType* object)
    {
      AddObjectRecursive (rootNode, object);
    }

    /**
     * 
     */
    bool RemoveObject (const ObjectType* object)
    {
      return RemoveObjectRec (object, rootNode);
    }

    /**
     * 
     */
//     void AddObjects (csDirtyAccessArray<ObjectType*> objects)
//     {
//       // Collect any existing objects into the objects array     
//       {
//         ObjectCollectFn collect (objects);
//         TraverseLeafs (collect);
//       }
// 
//       // Build a new tree
//       DeleteNodeRecursive (rootNode);
// 
//       // New root
//       rootNode = AllocNode ();
//       rootNode->SetLeaf (true);
// 
//       // Build
//       BuildTree (rootNode, objects, 0, objects.GetSize ());
//     }

    /**
     * 
     */
    bool MoveObject (ObjectType* object, const csBox3& oldBox)
    {
      // Traverse down the tree, recursively updating BB if we have an update
      return MoveObjectRec (object, rootNode, oldBox); 
    }

    /**
     * 
     */
    template<typename InnerFn, typename LeafFn>
    void Traverse (InnerFn& inner, LeafFn& leaf)
    {
      if (rootNode)
        TraverseRec (inner, leaf, rootNode);
    }

    /**
     * 
     */
    template<typename InnerFn, typename LeafFn>
    void Traverse (InnerFn& inner, LeafFn& leaf) const
    {
      if (rootNode)
        TraverseRec (inner, leaf, rootNode);
    }

    /**
     * 
     */
    template<typename InnerFn, typename LeafFn>
    void TraverseF2B (InnerFn& inner, LeafFn& leaf, const csVector3& direction)
    {
      if (rootNode)
        TraverseRecF2B (inner, leaf, direction, rootNode);
    }

    /**
     * 
     */
    template<typename InnerFn, typename LeafFn>
    void TraverseF2B (InnerFn& inner, LeafFn& leaf, const csVector3& direction) const
    {
      if (rootNode)
        TraverseRecF2B (inner, leaf, direction, rootNode);
    }

    /**
     * 
     */
    template<typename InnerFn, typename LeafFn>
    void TraverseOut (InnerFn& inner, LeafFn& leaf, const csVector3& point)
    {
      if (rootNode)
        TraverseRecOut (inner, leaf, point, rootNode);
    }

    /**
     * 
     */
    template<typename InnerFn, typename LeafFn>
    void TraverseOut (InnerFn& inner, LeafFn& leaf, const csVector3& point) const
    {
      if (rootNode)
        TraverseRecOut (inner, leaf, point, rootNode);
    }

    /**
     * 
     */
    struct InnerNodeNoOp
    {
      bool operator() (const Node* n)
      {
        CS_ASSERT_MSG("Invalid AABB-tree", !n->IsLeaf ());
        return true;
      }
    };

    /**
     * 
     */
    struct LeafNodeNoOp
    {
      bool operator() (const Node* n)
      { 
        CS_ASSERT_MSG("Invalid AABB-tree", n->IsLeaf ());
        return true;
      }
    };

  protected:
    
    /**
     * 
     */
    struct ObjectTypeSortByCenter
    {
      ObjectTypeSortByCenter (size_t axis)
        : axis (axis)
      {}

      bool operator() (ObjectType* o1, ObjectType* o2)
      {
        return o1->GetBBox ().GetCenter ()[axis] < 
          o2->GetBBox ().GetCenter ()[axis];
      }

      size_t axis;
    };

    /**
     * 
     */
    struct ObjectCollectFn
    {
      ObjectCollectFn (csDirtyAccessArray<ObjectType*>& objects)
        : objects (objects)
      {}

      void operator() (Node* child)
      {
        CS_ASSERT(child->IsLeaf ());
        for (size_t i = 0; child->GetObjectCount (); ++i)
        {
          objects.Push (child->GetLeafData (i));
        }
      }

      csDirtyAccessArray<ObjectType*>& objects;
    };


    /**
     * 
     */
    void AddObjectRecursive (Node* node, ObjectType* object)
    {
      if (node->IsLeaf ())
      {
        if (node->IsObjectSlotFree ())
        {
          node->AddLeafData (object);
          static_cast<NodeExtraData*> (node)->LeafAddObject (object);
        }
        else
        {
          // Need to split node in two

          // Split according to longest axis
          const size_t axis = node->GetBBox ().GetSize ().DominantAxis ();

          // Save old info          
          ObjectType* oldNodeI[objectsPerLeaf+1];

          oldNodeI[0] = object;

          size_t oldNodeCount = node->GetObjectCount ();
          for (size_t i = 0; i < oldNodeCount; ++i)
          {
            oldNodeI[i+1] = node->GetLeafData (i);            
          }

          {
            ObjectTypeSortByCenter sorter (axis);
            std::sort (oldNodeI, oldNodeI+oldNodeCount+1, sorter);
          }
        
          Node* node1 = AllocNode ();
          node1->SetLeaf (true);
          Node* node2 = AllocNode ();
          node2->SetLeaf (true);

          // Assign first
          {
            size_t i = 0;
            for (i = 0; i < (oldNodeCount+1)/2; ++i)
            {
              AddObjectRecursive (node1, oldNodeI[i]);
            }

            for (; i < oldNodeCount+1; ++i)
            {
              AddObjectRecursive (node2, oldNodeI[i]);
            }
          }

          // Setup new
          node->SetLeaf (false);
          node->SetChild1 (node1);
          node->SetChild2 (node2);
          static_cast<NodeExtraData*> (node)->NodeUpdate (*node1, *node2);
          
          // update bbox
          node->GetBBox() += object->GetBBox();
        }
      }
      else
      {
        // Select left or right depending on closeness to center (find better)
        const csVector3 objBoxCenter = object->GetBBox ().GetCenter ();
        const size_t axis = node->GetBBox ().GetSize ().DominantAxis ();

        if (objBoxCenter[axis] < node->GetBBox ().GetCenter ()[axis])
        {
          AddObjectRecursive (node->GetChild1 (), object);
          node->GetBBox ().AddBoundingBox (node->GetChild1 ()->GetBBox ());
        }
        else
        {
          AddObjectRecursive (node->GetChild2 (), object);
          node->GetBBox ().AddBoundingBox (node->GetChild2 ()->GetBBox ());
        }
        static_cast<NodeExtraData*> (node)->NodeUpdate (*node->GetChild1(), *node->GetChild2());
      }
    }

    /**
     * 
     */
    template<typename InnerFn, typename LeafFn>
    bool TraverseRec (InnerFn& inner, LeafFn& leaf, Node* node)
    {
      bool ret = true;
      if (!node) 
        return ret;

      if (node->IsLeaf ())
      {
        ret = leaf (node);
      }
      else
      {
        if (inner (node))
        {
          ret = TraverseRec (inner, leaf, node->GetChild1 ());
          if (ret) ret = TraverseRec (inner, leaf, node->GetChild2 ());
        }
      }
      return ret;
    }

    /**
     * 
     */
    template<typename InnerFn, typename LeafFn>
    bool TraverseRec (InnerFn& inner, LeafFn& leaf, const Node* node) const
    {
      bool ret = true;
      if (!node) 
        return ret;

      if (node->IsLeaf ())
      {
        ret = leaf (node);
      }
      else
      {
        if (inner (node))
        {
          ret = TraverseRec (inner, leaf, node->GetChild1 ());
          if (ret) ret = TraverseRec (inner, leaf, node->GetChild2 ());
        }
      }
      return ret;
    }

    /**
     * 
     */
    template<typename InnerFn, typename LeafFn>
    bool TraverseRecF2B (InnerFn& inner, LeafFn& leaf, const csVector3& direction, Node* node)
    {
      bool ret = true;
      if (!node) 
        return ret;

      if (node->IsLeaf ())
      {
        ret = leaf (node);
      }
      else
      {
        if (inner (node))
        {
          const csVector3 centerDiff = node->GetChild2 ()->GetBBox ().GetCenter () -
            node->GetChild1 ()->GetBBox ().GetCenter ();

          const size_t firstIdx = (centerDiff * direction > 0) ? 0 : 1;

          ret = TraverseRecF2B (inner, leaf, direction, node->GetChild (firstIdx));
          ret &= TraverseRecF2B (inner, leaf, direction, node->GetChild (1-firstIdx));
        }
      }
      return ret;
    }


    /**
     * 
     */
    template<typename InnerFn, typename LeafFn>
    bool TraverseRecF2B (InnerFn& inner, LeafFn& leaf, const csVector3& direction, const Node* node) const 
    {
      bool ret = true;
      if (!node) 
        return ret;

      if (node->IsLeaf ())
      {
        ret = leaf (node);
      }
      else
      {
        if (inner (node))
        {
          const csVector3 centerDiff = node->GetChild2 ()->GetBBox ().GetCenter () -
            node->GetChild1 ()->GetBBox ().GetCenter ();

          const size_t firstIdx = (centerDiff * direction > 0) ? 0 : 1;

          ret = TraverseRecF2B (inner, leaf, direction, node->GetChild (firstIdx));
          ret &= TraverseRecF2B (inner, leaf, direction, node->GetChild (1-firstIdx));
        }
      }
      return ret;
    }


    /**
     * 
     */
    template<typename InnerFn, typename LeafFn>
    bool TraverseRecOut (InnerFn& inner, LeafFn& leaf, const csVector3& point, Node* node)
    {
      bool ret = true;
      if (!node) 
        return ret;

      if (node->IsLeaf ())
      {
        ret = leaf (node);
      }
      else
      {
        if (inner (node))
        {
          const float ch1LenSq = (node->GetChild1 ()->GetBBox ().GetCenter () - point).SquaredNorm ();
          const float ch2LenSq = (node->GetChild2 ()->GetBBox ().GetCenter () - point).SquaredNorm ();

          const size_t firstIdx = (ch1LenSq > ch2LenSq) ? 0 : 1;

          ret = TraverseRecOut (inner, leaf, point, node->GetChild (firstIdx));
          if (ret) ret = TraverseRecOut (inner, leaf, point, node->GetChild (1-firstIdx));
        }
      }
      return ret;
    }

    /**
     * 
     */
    template<typename InnerFn, typename LeafFn>
    bool TraverseRecOut (InnerFn& inner, LeafFn& leaf, const csVector3& point, const Node* node) const
    {
      bool ret = true;
      if (!node) 
        return ret;

      if (node->IsLeaf ())
      {
        ret = leaf (node);
      }
      else
      {
        if (inner (node))
        {
          const float ch1LenSq = (node->GetChild1 ()->GetBBox ().GetCenter () - point).SquaredNorm ();
          const float ch2LenSq = (node->GetChild2 ()->GetBBox ().GetCenter () - point).SquaredNorm ();

          const size_t firstIdx = (ch1LenSq > ch2LenSq) ? 0 : 1;

          ret = TraverseRecOut (inner, leaf, point, node->GetChild (firstIdx));
          if (ret) ret = TraverseRecOut (inner, leaf, point, node->GetChild (1-firstIdx));
        }
      }
      return ret;
    }
  
    /**
     * 
     */
    void BuildTree (Node* root, csDirtyAccessArray<ObjectType*>& objects, 
      size_t objectStart, size_t objectEnd)
    {
      if (objectEnd <= objectStart)
        return;

      const size_t numObjects = objectEnd - objectStart;
      const bool fewEnough = numObjects <= objectsPerLeaf;

      if (fewEnough)
      {
        // Assign to a leaf
        root->SetLeaf (true);
        for (size_t i = objectStart; i < objectEnd; ++i)
        {
          root->AddLeafData (objects[i]);
        }
        static_cast<NodeExtraData*> (root)->LeafUpdateObjects (
          root->GetLeafObjects (), root->GetObjectCount());
      }
      else
      {
        // Very dumb, sort by center, split at median
        const size_t axis = root->GetBBox ().GetSize ().DominantAxis ();

        {
          ObjectTypeSortByCenter sorter (axis);
          ObjectType** arr = objects.GetArray ();
          std::sort (arr + objectStart, arr + objectEnd, sorter);

          const size_t median = objectStart + numObjects / 2;

          Node* left = AllocNode ();
          Node* right = AllocNode ();

          root->SetChild1 (left);
          root->SetChild2 (right);
          
          BuildTree (left, objects, objectStart, median);
          BuildTree (right, objects, median + 1, objectEnd);
          static_cast<NodeExtraData*> (root)->NodeUpdate (*left, *right);
        }

      }

    }

    /**
     * 
     */
    Node* FindObjectNodeRec (Node* node, ObjectType* object)
    {
      {
        const csBox3& objBox = object->GetBBox ();
        if (!node->GetBBox ().Overlap (objBox))
        {
          return 0;
        }
      }

      if (node->IsLeaf ())
      {
        for (size_t i = 0; i < node->GetObjectCount (); ++i)
        {
          if (node->GetLeafData (i) == object)
          {
            return node;
          }
        }
      }
      else
      {        
        Node* result = FindObjectNodeRec (node->GetChild1 (), object);        
        
        if (!result)
          result = FindObjectNodeRec (node->GetChild2 (), object);

        return result;
      }

      return 0;
    }

    /**
     * 
     */
    bool MoveObjectRec (ObjectType* object, Node* node, const csBox3& oldBox)
    {
      if (node && oldBox.Overlap (node->GetBBox ()))
      {
        if (node->IsLeaf ())
        {
          for (size_t i = 0; i < node->GetObjectCount (); ++i)
          {
            if (node->GetLeafData (i) == object)
            {
              // Found node, update the node BB
              csBox3 newNodeBB = node->GetLeafData (0)->GetBBox ();
              for (size_t i = 1; i < node->GetObjectCount (); ++i)
              {
                newNodeBB += node->GetLeafData (i)->GetBBox ();
              }
              node->SetBBox (newNodeBB);

              return true; // Found it
            }
          }
        }
        else
        {
          Node* left = node->GetChild1 ();
          Node* right = node->GetChild2 ();
          
          if (left && MoveObjectRec (object, left, oldBox))
          {
            // Tree was updated, update our bb
            csBox3 newNodeBB = left->GetBBox ();
            if (right)
            {
              newNodeBB += right->GetBBox ();
            }
            node->SetBBox (newNodeBB);

            return true;
          }
          
          if (right && MoveObjectRec (object, right, oldBox))
          {
            // Tree was updated, update our bb
            csBox3 newNodeBB = right->GetBBox ();
            if (left)
            {
              newNodeBB += left->GetBBox ();
            }
            node->SetBBox (newNodeBB);

            return true;
          }
        }
      }
      
      return false; // Don't overlap in this node
    }

    /**
     * 
     */
    bool RemoveObjectRec (const ObjectType* object, Node* node)
    {
      const csBox3& objBox = object->GetBBox ();

      if (node && objBox.Overlap (node->GetBBox ()))
      {
        if (node->IsLeaf ())
        {
          for (size_t i = 0; i < node->GetObjectCount (); ++i)
          {
            if (node->GetLeafData (i) == object)
            {
              // Found node, update the node BB
              csBox3 newNodeBB;
              for (size_t j = 0; j < node->GetObjectCount (); ++j)
              {
                if (i != j)
                {
                  newNodeBB += node->GetLeafData (j)->GetBBox ();
                }
              }
              node->SetBBox (newNodeBB);
	      node->RemoveLeafData (i);
	      static_cast<NodeExtraData*> (node)->LeafUpdateObjects (
	        node->GetLeafObjects(), node->GetObjectCount());

              return true; // Found it
            }
          }
        }
        else
        {
          Node* left = node->GetChild1 ();
          Node* right = node->GetChild2 ();

          if (left && RemoveObjectRec (object, left))
          {
            csBox3 newNodeBB;

            if (right)
            {
              newNodeBB = right->GetBBox ();
            }

            // Tree was updated, update our bb
            if (!left->IsLeaf() || left->GetObjectCount () > 0)
            {
              newNodeBB += left->GetBBox ();
              static_cast<NodeExtraData*> (node)->NodeUpdate (*left, *right);
            }
            else
            {
	      // We have to delete the left node. We do that by moving the children
	      // of the right node down.
	      if (right)
	      {
	        node->Copy (right);
		nodeAllocator.Free (left);
		nodeAllocator.Free (right);
                newNodeBB = node->GetBBox ();
	      }
	      else
	      {
                node->SetChild1 (0);
		node->SetLeaf (true);
                static_cast<NodeExtraData*> (node)->LeafUpdateObjects (0, 0);
		nodeAllocator.Free (left);
	      }
            }

            node->SetBBox (newNodeBB);

            return true;
          }

          if (right && RemoveObjectRec (object, right))
          {
            csBox3 newNodeBB;

            if (left)
            {
              newNodeBB = left->GetBBox ();
            }

            // Tree was updated, update our bb
            if (!right->IsLeaf() || right->GetObjectCount () > 0)
            {
              newNodeBB += right->GetBBox ();
              static_cast<NodeExtraData*> (node)->NodeUpdate (*left, *right);
            }
            else
            {
	      // We have to delete the right node. We do that by moving the children
	      // of the left node down.
	      if (left)
	      {
		node->Copy (left);
		nodeAllocator.Free (left);
		nodeAllocator.Free (right);
                newNodeBB = node->GetBBox ();
	      }
	      else
	      {
                node->SetChild2 (0);
		node->SetLeaf (true);
                static_cast<NodeExtraData*> (node)->LeafUpdateObjects (0, 0);
		nodeAllocator.Free (right);
	      }
            }

            node->SetBBox (newNodeBB);

            return true;
          }
        }
      }

      return false;
    }

    /**
     * 
     */
    Node* AllocNode ()
    {
      return nodeAllocator.Alloc ();
    }
    
    /**
     * 
     */
    void DeleteNodeRecursive (Node* node)
    {
      if (!node)
        return;

      if (!node->IsLeaf ())
      {
        DeleteNodeRecursive (node->GetChild1 ());
        DeleteNodeRecursive (node->GetChild2 ());
      }

      nodeAllocator.Free (node);
    }

    typedef csBlockAllocator<
      Node, 
      CS::Memory::AllocatorAlign<32>,
      csBlockAllocatorDisposeLeaky<Node>,
      csBlockAllocatorSizeObjectAlign<Node, 32>
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
    unsigned int objectsPerLeaf,
    typename NodeExtraData
  >
  class AABBTree<ObjectType, objectsPerLeaf, NodeExtraData>::Node : public NodeExtraData
  {
  public:
    Node ()
      : typeAndFlags (AABB_NODE_INNER), leafObjCount (0)  
    {
      children[0] = children[1] = 0;
    }

    // General accessors
    ///
    bool IsLeaf () const
    {
      return (typeAndFlags & AABB_NODE_TYPE_MASK) == AABB_NODE_LEAF;
    }

    ///
    void SetLeaf (bool isLeaf) 
    {
      if (isLeaf && !IsLeaf ())
      {
        typeAndFlags |= AABB_NODE_LEAF;
	// Ensure no children are 'lost'
	CS_ASSERT(children[0] == 0);
	CS_ASSERT(children[1] == 0);
        leafObjCount = 0;
      }
      else if (!isLeaf && IsLeaf ())
      {
        typeAndFlags &= ~AABB_NODE_LEAF;
        leafObjCount = 0;
        children[0] = children[1] = 0;
      }
    }

    ///
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
    uint GetObjectCount () const
    {
      CS_ASSERT(IsLeaf ()); // object count is only sensible for leaves
      return leafObjCount;
    }

    ///
    bool IsObjectSlotFree () const
    {
      CS_ASSERT(IsLeaf ()); // object count is only sensible for leaves
      return leafObjCount < objectsPerLeaf;
    }

    ///
    const csBox3& GetBBox () const
    {
      return boundingBox;
    }

    ///
    csBox3& GetBBox ()
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
      CS_ASSERT(!IsLeaf ());
      return children[0];
    }

    /// 
    Node* GetChild2 () const
    {
      CS_ASSERT(!IsLeaf ());
      return children[1];
    }

    ///
    Node* GetChild (size_t i) const
    {
      CS_ASSERT(!IsLeaf () && i < 2);
      return children[i];
    }

    /// 
    void SetChild1 (Node* child)
    {
      CS_ASSERT(!IsLeaf ());
      children[0] = child;
    }

    /// 
    void SetChild2 (Node* child)
    {
      CS_ASSERT(!IsLeaf ());
      children[1] = child;
    }

    /// Copy the node contents to this one.
    void Copy (Node* source)
    {
      typeAndFlags = source->typeAndFlags;
      if (IsLeaf ())
      {
	memcpy (leafStorage, source->leafStorage, sizeof (ObjectType*) * objectsPerLeaf);
      }
      else
      {
        SetChild1 (source->GetChild1 ());
        SetChild2 (source->GetChild2 ());
      }
      leafObjCount = source->leafObjCount;
      SetBBox (source->GetBBox ());
      NodeExtraData::operator= (*source);
    }

    // Accessor for leaf node data
    ///
    ObjectType* GetLeafData (size_t index) const
    {
      CS_ASSERT(IsLeaf ());
      CS_ASSERT(index < objectsPerLeaf);

      return leafStorage[index];
    }

    ObjectType** GetLeafObjects ()
    {
      CS_ASSERT(IsLeaf ());
      return leafStorage;
    }

    ///
    void AddLeafData (ObjectType* object)
    {
      CS_ASSERT(IsLeaf ());
      CS_ASSERT(leafObjCount < objectsPerLeaf);
      leafStorage[leafObjCount++] = object;

      boundingBox.AddBoundingBox (object->GetBBox ());
    }

    void RemoveLeafData (size_t index)
    {
      CS_ASSERT(IsLeaf ());
      CS_ASSERT(leafObjCount > 0);
      leafStorage[index] = leafStorage[--leafObjCount];
    }

  private:
    ///
    uint16 typeAndFlags;

    ///
    uint16 leafObjCount;

    ///
    csBox3 boundingBox;

    ///
    union
    {
      ObjectType* leafStorage[objectsPerLeaf];
      Node* children[2];
    };
  };


}
}

#endif
