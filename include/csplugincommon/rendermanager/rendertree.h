/*
    Copyright (C) 2007 by Marten Svanfeldt

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation; 

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

#include "iengine/viscull.h"

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

  struct RenderTreeStandardTraits
  {

  };

  /**
   * 
   */
  template<typename MeshNodeDataType = void, typename ContextNodeDataType = void, 
    typename TreeTraits = RenderTreeStandardTraits>
  class RenderTree
  {
  public:
    //---- Forward declarations
    struct MeshNode;
    struct ContextNode;

    //---- Type definitions
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
    struct MeshNode : public EBOptHelper<MeshNodeDataType>
    {
      iMeshWrapper* meshWrapper;
      csArray<csRenderMesh*> renderMeshes;
    };

    /**
     * 
     */
    struct ContextNode : public EBOptHelper<ContextNodeDataType>
    {
      csArray<MeshNode*> meshNodes;
    };


    typedef RenderTree<MeshNodeDataType, ContextNodeDataType, TreeTraits> ThisType;

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
    void TraverseMeshNodes (Fn& meshFunction)
    {
      struct LocalWalk
      {
        LocalWalk (Fn& meshFunction)
          : meshFunction (meshFunction)
        {}

        void operator() (const ContextNode* node, const ThisType& tree)
        {
          CS::ForEach (node->meshNodes.GetIterator (), meshFunction, *node, tree);
        }

        Fn& meshFunction;
      };

      TraverseContexts (LocalWalk(meshFunction));
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
    PersistentData& persistentData;
    csArray<ContextNode*> contextNodeList;

    size_t    totalMeshNodes;
    size_t    totalRenderMeshes;

    /**
     * 
     */
    class ViscullCallback : public scfImplementation1<ViscullCallback, iVisibilityCullerListener>
    {
    public:
      ViscullCallback (ThisType& ownerTree, iRenderView* rw)
        : scfImplementationType (this), ownerTree (ownerTree), currentRenderView (rw)
      {}


      virtual void ObjectVisible (iVisibilityObject *visobject, 
        iMeshWrapper *mesh, uint32 frustum_mask)
      {
        // For now, just add them to first context, not that smart
        MeshNode* mn = ownerTree.persistentData.meshNodeAllocator.Alloc ();
        ownerTree.contextNodeList[0]->meshNodes.Push (mn);

        mn->meshWrapper = mesh;

        ownerTree.totalMeshNodes++;

        int count;
        csRenderMesh** rms = mesh->GetRenderMeshes (count, currentRenderView, frustum_mask);

        for (int i = 0; i < count; ++i)
        {
          mn->renderMeshes.Push (rms[i]);
          ownerTree.totalRenderMeshes++;
        }
      }

    private:
      ThisType& ownerTree;
      iRenderView* currentRenderView;
    };

  };



}
}

#endif
