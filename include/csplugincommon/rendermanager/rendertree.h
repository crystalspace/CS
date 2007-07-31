/*
    Copyright (C) 2007 by Marten Svanfeldt

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_RENDERTREE_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_RENDERTREE_H__

#include "csplugincommon/rendermanager/standardtreetraits.h"
#include "iengine/viscull.h"
#include "csutil/redblacktree.h"

namespace CS
{
namespace RenderManager
{
  
  template<typename T>
  struct EBOptHelper
  {
    T customData;
  };

  template<>
  struct EBOptHelper<void>
  {};

  /**
   * 
   */
  template<typename TreeTraits = RenderTreeStandardTraits>
  class RenderTree
  {
  public:
    //---- Forward declarations
    struct MeshNode;
    struct ContextNode;

    //---- Type definitions
    typedef TreeTraits TreeTraitsType;
    /**
     * 
     */
    struct PersistentData
    {
      csBlockAllocator<MeshNode> meshNodeAllocator;
      csBlockAllocator<ContextNode> contextNodeAllocator;
    };

    /**
     * 
     */
    struct MeshNode : public EBOptHelper<typename TreeTraits::MeshNodeExtraDataType>
    {
      /**
       * 
       */
      struct SingleMesh : public EBOptHelper<typename TreeTraits::MeshExtraDataType>
      {
        csRenderMesh* renderMesh;
      };

      csArray<SingleMesh> meshes;
    };

    /**
     * 
     */
    struct ContextNode : public EBOptHelper<typename TreeTraits::MeshNodeExtraDataType>
    {
      // A sub-tree of mesh nodes
      csRedBlackTreeMap<typename TreeTraits::MeshNodeKeyType, MeshNode*>   meshNodes;
    };


    typedef RenderTree<TreeTraits> ThisType;

    //---- Methods
    RenderTree (PersistentData& dataStorage)
      : persistentData (dataStorage), totalMeshNodes (0), totalRenderMeshes (0)
    {
    }

    ~RenderTree ()
    {
      // Clean up the persistent data
      persistentData.contextNodeAllocator.Empty ();
      persistentData.meshNodeAllocator.Empty ();
    }

    /**
     * 
     */
    void PrepareViscull (iRenderView* rw)
    {
      // Create an initial context
      ContextNode* newCtx = persistentData.contextNodeAllocator.Alloc ();
      contextNodeList.Push (newCtx);

      currentContextNode = newCtx;
    }

    /**
     * 
     */
    bool Viscull (iRenderView* rw, iVisibilityCuller* culler)
    {
      ViscullCallback cb (*this, rw);

      culler->VisTest (rw, &cb);
      return true;
    }

    /**
     * 
     */
    void FinishViscull ()
    {
    }

    /**
     * 
     */
    template<typename Fn>
    void TraverseContexts (Fn& contextFunction)
    {
      CS::ForEach (contextNodeList.GetIterator (), contextFunction, *this);
    }

    /**
     * 
     */
    template<typename Fn>
    void TraverseMeshNodes (Fn& meshNodeFunction)
    {
      TraverseMeshNodesWalker<Fn> walker (meshNodeFunction);
      TraverseContexts (walker);
    }


    // Some helpers
    ///
    inline size_t GetTotalMeshNodes () const
    {
      return totalMeshNodes;
    }

    inline size_t GetTotalRenderMeshes () const
    {
      return totalRenderMeshes;
    }

  protected:    
    PersistentData&         persistentData;
    csArray<ContextNode*>   contextNodeList;
    
    ContextNode*            currentContextNode;

    size_t                  totalMeshNodes;
    size_t                  totalRenderMeshes;

    /**
     * 
     */
    class ViscullCallback : public scfImplementation1<ViscullCallback, iVisibilityCullerListener>
    {
    public:
      ViscullCallback (ThisType& ownerTree, iRenderView* rw)
        : scfImplementation1<ViscullCallback, iVisibilityCullerListener> (this), ownerTree (ownerTree), currentRenderView (rw)
      {}


      virtual void ObjectVisible (iVisibilityObject *visobject, 
        iMeshWrapper *mesh, uint32 frustum_mask)
      {
        // Call uppwards
        ownerTree.ViscullObjectVisible (visobject, mesh, frustum_mask, currentRenderView);
      }

    private:
      ThisType& ownerTree;
      iRenderView* currentRenderView;
    };

    void ViscullObjectVisible (iVisibilityObject *visobject, 
      iMeshWrapper *imesh, uint32 frustum_mask, iRenderView* renderView)
    {
      // Todo: Handle static lod & draw distance

      // Get the meshes
      int numMeshes;
      csRenderMesh** meshList = imesh->GetRenderMeshes (numMeshes, renderView, frustum_mask);

      // Add it to the appropriate meshnode
      for (int i = 0; i < numMeshes; ++i)
      {
        typename TreeTraits::MeshNodeKeyType meshKey = 
          TreeTraits::GetMeshNodeKey (imesh, *meshList[i]);

        // Get the mesh node
        MeshNode* meshNode = currentContextNode->meshNodes.Get (meshKey, 0);
        if (!meshNode)
        {
          // Get a new one
          meshNode = persistentData.meshNodeAllocator.Alloc ();

          TreeTraits::SetupMeshNode(*meshNode, imesh, *meshList[i]);
          currentContextNode->meshNodes.Put (meshKey, meshNode);

          totalMeshNodes++;
        }

        typename MeshNode::SingleMesh sm;
        sm.renderMesh = meshList[i];

        meshNode->meshes.Push (sm);

        totalRenderMeshes++;
      }
    }
    
    template<typename Fn>
    struct TraverseMeshNodesWalker
    {
      TraverseMeshNodesWalker (Fn& meshNodeFunction)
        : meshNodeFunction (meshNodeFunction)
      {}

      void operator() (ContextNode* node, ThisType& tree)
      {        
        MeshNodeCB<Fn> cb (meshNodeFunction, node, tree);
        node->meshNodes.TraverseInOrder (cb);
      }

      Fn& meshNodeFunction;
    };

    private:

      template<typename Fn>
      struct MeshNodeCB
      {
        MeshNodeCB(Fn& meshNodeFunction, ContextNode* node, ThisType& tree)
          : meshNodeFunction (meshNodeFunction), node (node), tree (tree)
        {}

        void operator() (const typename TreeTraits::MeshNodeKeyType& key, MeshNode* meshNode)
        {
          meshNodeFunction (key, meshNode, *node, tree);
        }

        Fn& meshNodeFunction;
        ContextNode* node;
        ThisType& tree;
      };
  };

  template<typename Tree, typename Fn>
  class NumberedMeshTraverser
  {
  public:
    typedef NumberedMeshTraverser<Tree, Fn> NumberedMeshTraverserType;

    NumberedMeshTraverser (Fn& fun)
      : fun(fun), meshOffset (0)
    {}

    void operator() (const typename Tree::TreeTraitsType::MeshNodeKeyType& key,
      typename Tree::MeshNode* node, typename Tree::ContextNode& ctxNode, Tree& tree)
    {
      for (size_t i = 0; i < node->meshes.GetSize(); ++i)
      {
        fun (node->meshes[i], meshOffset++, ctxNode, tree);
      }
    }

  private:
    Fn& fun;
    size_t meshOffset;
  };
 

}
}

#endif
