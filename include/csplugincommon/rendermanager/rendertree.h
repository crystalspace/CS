/*
    Copyright (C) 2007-2008 by Marten Svanfeldt

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
#include "csutil/redblacktree.h"
#include "csutil/metautils.h"

struct iMeshWrapper;
struct iPortalContainer;

namespace CS
{
namespace RenderManager
{   

  /**
   * RenderTree is the main data-structure for the rendermanagers.
   * It contains the entire setup of meshes and where to render those meshes,
   * as well as basic operations regarding those meshes.
   */
  template<typename TreeTraits = RenderTreeStandardTraits>
  class RenderTree
  {
  public:
    //---- Forward declarations
    struct MeshNode;
    struct ContextNode;
    struct ContextsContainer;

    //---- Type definitions
    typedef TreeTraits TreeTraitsType;
    typedef RenderTree<TreeTraitsType> ThisType;
    typedef RenderTree<TreeTraitsType> RealTreeType;

    //---- Internal types
    typedef csRedBlackTreeMap<typename TreeTraitsType::MeshNodeKeyType, MeshNode*> MeshNodeTreeType;
    typedef typename MeshNodeTreeType::Iterator MeshNodeTreeIteratorType;
    typedef csArray<ContextNode*> ContextNodeArrayType;
    typedef typename ContextNodeArrayType::Iterator ContextNodeArrayIteratorType;
    typedef typename ContextNodeArrayType::ReverseIterator ContextNodeArrayReverseIteratorType;


    /**
     * Persistent data for the tree, needs to be stored between frames by the RM
     */
    struct PersistentData
    {
      void Initialize (iShaderManager* shmgr)
      {
        svObjectToWorldName = 
          shmgr->GetSVNameStringset()->Request ("object2world transform");
      }

      void Clear ()
      {
        // Clean up the persistent data
        contextNodeAllocator.Empty ();
        meshNodeAllocator.Empty ();
      }

      csBlockAllocator<MeshNode> meshNodeAllocator;
      csBlockAllocator<ContextNode> contextNodeAllocator;      
      

      CS::ShaderVarStringID svObjectToWorldName;
    
      RenderView::Pool renderViewPool;
      csRenderMeshHolder rmHolder;           
    };

    /**
     * A mesh node is a single list of meshes that can be rendered in one go
     * without any explicit order considerations.
     */
    struct MeshNode : 
      public CS::Meta::EBOptHelper<typename TreeTraitsType::MeshNodeExtraDataType>
    {
      /**
       * A single mesh within the tree to be rendered.
       */
      struct SingleMesh : 
        public CS::Meta::EBOptHelper<typename TreeTraitsType::MeshExtraDataType>
      {
        csRenderMesh* renderMesh;
        csZBufMode zmode;
        iShaderVariableContext* meshObjSVs;
        csRef<csShaderVariable> svObjectToWorld;

        size_t contextLocalId;
      };

      //-- Types
      typedef RealTreeType TreeType;
      typedef typename TreeType::ContextNode ContextNodeType;

      //-- Some local types
      typedef csArray<SingleMesh> MeshArrayType;
      typedef typename MeshArrayType::Iterator MeshArrayIteratorType;

      /// Owner
      ContextNode& owner;
      
      /// Our own key
      typename TreeTraitsType::MeshNodeKeyType key;

      /// All the meshes within the meshnode
      MeshArrayType meshes;

      MeshNode (ContextNode& owner)
        : owner (owner)
      {}
    };

    /**
     * 
     */
    struct ContextNode : public CS::Meta::EBOptHelper<typename TreeTraits::ContextNodeExtraDataType>
    {
      /**
       * 
       */
      struct PortalHolder
      {
      #ifdef CS_DEBUG
        const char* db_mesh_name;
      #endif
        iPortalContainer* portalContainer;
        iMeshWrapper* meshWrapper;
      };

      //-- Types
      typedef RealTreeType TreeType;

      // Owner
      TreeType& owner;

      // Target data
      csRef<RenderView> renderView;
      iTextureHandle* renderTarget;
      int subtexture;
      csReversibleTransform cameraTransform;

      // A sub-tree of mesh nodes
      MeshNodeTreeType meshNodes;

      // All portals within context
      csArray<PortalHolder> allPortals;

      // The SVs themselves
      SVArrayHolder svArrays;

      // Arrays of per-mesh shader and ticket info
      csArray<iShader*> shaderArray;
      csArray<size_t> ticketArray;

      // Total number of render meshes within the context, just for statistics
      size_t totalRenderMeshes;
      
      ContextNode(TreeType& owner) 
        : owner (owner), renderTarget (0), subtexture (0), totalRenderMeshes (0) 
      {}
    };


    //---- Methods
    RenderTree (PersistentData& dataStorage)
      : persistentData (dataStorage)
    {
    }

    ~RenderTree ()
    {
      persistentData.Clear ();
    }

    /**
     * Create a new context
     * \param rw Render view to associate the new context with. Should be 
     * initalized before calling method.
     * \param insertAfter Context to insert this one after. Must be a valid
     * context in current render tree.
     */
    ContextNode* CreateContext (RenderView* rw, ContextNode* insertAfter = 0)
    {
      // Create an initial context
      ContextNode* newCtx = persistentData.contextNodeAllocator.Alloc (*this);
      newCtx->renderView = rw;
      newCtx->cameraTransform = rw->GetCamera ()->GetTransform ();

      if (insertAfter)
      {
        size_t n = contexts.Find (insertAfter);
        CS_ASSERT(n != csArrayItemNotFound);
        contexts.Insert (n+1, newCtx);
      }
      else
      {
        contexts.Push (newCtx);      
      }

      return newCtx;
    }
    
    /**
     * Destroy a context and return it to the allocation pool.
     */
    void DestoryContext (ContextNode* context)
    {
      CS_ASSERT(contexts.Find (context) != csArrayItemNotFound);
      contexts.Delete (context);

      persistentData.contextNodeAllocator.Free (context);
    }

    /**
     * Get an iterator for iterating forward over the contexts.
     */
    ContextNodeArrayIteratorType GetContextIterator ()
    {
      return contexts.GetIterator ();
    }

    /**
     * Get an iterator for iterating backward over the contexts.
     */
    ContextNodeArrayReverseIteratorType GetReverseContextIterator ()
    {
      return contexts.GetReverseIterator ();
    }

    /**
     * Create a new mesh node associated with the given context.
     */
    MeshNode* CreateMeshNode (ContextNode& context, 
      const typename TreeTraitsType::MeshNodeKeyType& key)
    {
      MeshNode* newNode = persistentData.meshNodeAllocator.Alloc (context);      
      newNode->key = key;
    
      return newNode;
    }


    /**
     * Destroy given mesh node.
     */
    void DestroyMeshNode (MeshNode* meshNode)
    {
      meshNode->owner.meshNodes.Delete (meshNode->key);
      persistentData.meshNodeAllocator.Free (meshNode);
    }



    PersistentData& GetPersistentData()
    {
      return persistentData;
    }
    const PersistentData& GetPersistentData() const
    {
      return persistentData;
    }

  protected:    
    PersistentData&         persistentData;
    ContextNodeArrayType    contexts; 
  };

}
}



#endif
