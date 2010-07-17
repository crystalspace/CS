/*
    Copyright (C) 2007-2008 by Marten Svanfeldt
    Copyright (C) 2010 by Joe Forte

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

#ifndef __CS_DEFERREDRENDERTREE_H__
#define __CS_DEFERREDRENDERTREE_H__

#include "csplugincommon/rendermanager/rendertree.h"

struct iMeshWrapper;
struct iPortalContainer;

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{
  /**
   * DeferredRenderTree is the main data-structure for the deferred render manager.
   * It contains the entire setup of meshes and where to render those meshes,
   * as well as basic operations regarding those meshes.
   *
   * The \a TreeTraits template argument specifies additional data stored with
   * meshes, contexts and others in the tree. See the subclasses in
   * RenderTreeStandardTraits for a list of what can be customized.
   * To provide custom traits, create a class and either provide a new, custom
   * type for a trait or typedef in the respective type from
   * RenderTreeStandardTraits.
   */
  template<typename TreeTraits = DeferredRenderTreeTraits>
  class DeferredRenderTree : public CS::RenderManager::RenderTreeBase
  {
  public:

    //---- Forward declarations
    struct MeshNode;
    struct ContextNode;
    struct ContextsContainer;

    //---- Type definitions
    typedef TreeTraits TreeTraitsType;
    typedef DeferredRenderTree<TreeTraitsType> ThisType;
    typedef DeferredRenderTree<TreeTraitsType> RealTreeType;
    typedef CS::RenderManager::RenderView RenderView;
    typedef CS::RenderManager::PostEffectManager PostEffectManager;
    typedef CS::RenderManager::SVArrayHolder SVArrayHolder;

    //---- Internal types
    typedef csFixedSizeAllocator<
      csRedBlackTreeMap<typename TreeTraitsType::MeshNodeKeyType,
        MeshNode*>::allocationUnitSize> MeshNodeTreeBlockAlloc;
    typedef CS::Memory::AllocatorRef<MeshNodeTreeBlockAlloc>
      MeshNodeTreeBlockRefAlloc;
    typedef csRedBlackTreeMap<typename TreeTraitsType::MeshNodeKeyType, MeshNode*,
      MeshNodeTreeBlockRefAlloc> MeshNodeTreeType;
    typedef typename MeshNodeTreeType::Iterator MeshNodeTreeIteratorType;
    typedef csArray<ContextNode*> ContextNodeArrayType;
    typedef typename ContextNodeArrayType::Iterator ContextNodeArrayIteratorType;
    typedef typename ContextNodeArrayType::ReverseIterator ContextNodeArrayReverseIteratorType;

    /**
     * Data used by the render tree that needs to persist over multiple frames.
     * Render managers must store an instance of this class and provide
     * it to the render tree upon instantiation.
     */
    struct PersistentData
    {
      /**
       * Initialize data. Fetches various required values from objects in
       * the object registry. Must be called when the render manager plugin is 
       * initialized.
       */
      void Initialize (iShaderManager* shmgr)
      {
        svObjectToWorldName = 
          shmgr->GetSVNameStringset()->Request ("object2world transform");
        svObjectToWorldInvName = 
          shmgr->GetSVNameStringset()->Request ("object2world transform inverse");
        svFogplaneName = 
          shmgr->GetSVNameStringset()->Request ("fogplane");
          
        dbgDebugClearScreen = debugPersist.RegisterDebugFlag ("debugclear");
      }

      void Clear ()
      {
        // Clean up the persistent data
        contextNodeAllocator.Empty ();
        meshNodeAllocator.Empty ();
      }

      csBlockAllocator<MeshNode> meshNodeAllocator;
      csBlockAllocator<ContextNode> contextNodeAllocator;
      MeshNodeTreeBlockAlloc meshNodeTreeAlloc;

      CS::ShaderVarStringID svObjectToWorldName;
      CS::ShaderVarStringID svObjectToWorldInvName;
      CS::ShaderVarStringID svFogplaneName;
    
      RenderView::Pool renderViewPool;
      csRenderMeshHolder rmHolder;
      
      DebugPersistent debugPersist;
      uint dbgDebugClearScreen;
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
        /// Originating mesh wrapper
        iMeshWrapper* meshWrapper;
        /// Render mesh
        csRenderMesh* renderMesh;
        /// Mesh Z buffer mode
        csZBufMode zmode;
        /// Mesh object wrapper shader variables
        iShaderVariableContext* meshObjSVs;
        /// Mesh object to world transformation
        csRef<csShaderVariable> svObjectToWorld;
        /// Mesh object to world inverse transformation
        csRef<csShaderVariable> svObjectToWorldInv;
        /// Mesh flags
        csFlags meshFlags;

        /// "Local ID" in the context; used for array indexing
        size_t contextLocalId;
        
        /**\name Copying render target contents before rendering the mesh.
         * Setting these fields has the render target contents copied to the
         * given texture just before the mesh is rendered.
         * @{ */
        /// Number of attachment/texture pairs.
        size_t preCopyNum;
        /// Array of attachments to be copied.
        csRenderTargetAttachment preCopyAttachments[rtaNumAttachments];
        /// Array of textures to be copied to.
        iTextureHandle* preCopyTextures[rtaNumAttachments];
        /** @} */
        
        SingleMesh () : preCopyNum (0) {}
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

      /// Sorting
      int sorting;

      /// All the meshes within the meshnode
      MeshArrayType meshes;

      MeshNode (ContextNode& owner)
        : owner (owner)
      {}
    };

    /**
     * A single context node, Groups meshes which should be rendered from the
     * same view to the same target.
     *
     * Create instances of this structure by using RenderTree::CreateContext.
     */
    struct ContextNode : public CS::Meta::EBOptHelper<typename TreeTraits::ContextNodeExtraDataType>
    {
      /**
       * Information for a portal
       */
      struct PortalHolder
      {
      #ifdef CS_DEBUG
        /// Debugging: name of mesh
        const char* db_mesh_name;
      #endif
        /// Portal container interface
        iPortalContainer* portalContainer;
        /// Originating mesh wrapper
        iMeshWrapper* meshWrapper;
      };

      //-- Types
      typedef RealTreeType TreeType;

      /// Owner of context node
      TreeType& owner;

      /// View rendered to
      csRef<RenderView> renderView;
      /// A single render target
      struct TargetTexture
      {
        /// Texture handle
        iTextureHandle* texHandle;
        /// Subtexture
        int subtexture;

        TargetTexture() : texHandle (0), subtexture (0) {}
      };
      /// All render targets to be used when rendering the context node
      TargetTexture renderTargets[rtaNumAttachments];
      /**
       * Matrix to be applied after projection from camera (usually used for
       * post processing manager targets)
       */
      CS::Math::Matrix4 perspectiveFixup;
      /// Camera transformation
      csReversibleTransform cameraTransform;
      /// Flags for iGraphics3D::BeginDraw()
      int drawFlags;
      /// Sector to render
      iSector* sector;
      /// Holds fog plane for sector+view
      csRef<csShaderVariable> svFogplane;
      /// Context-specific shader variables
      csRef<iShaderVariableContext> shadervars;
      
      /// Post processing effects to apply after rendering the context
      csRef<PostEffectManager> postEffects;

      /// Sub-tree of mesh nodes
      MeshNodeTreeType meshNodes;

      /// All portals within context
      csArray<PortalHolder> allPortals;

      /// The SVs themselves
      SVArrayHolder svArrays;

      /// Arrays of per-mesh shader
      csDirtyAccessArray<iShader*> shaderArray;
      /// Arrays of per-mesh ticket info
      csArray<size_t> ticketArray;

      /// Total number of render meshes within the context
      size_t totalRenderMeshes;
      
      ContextNode(TreeType& owner, MeshNodeTreeBlockAlloc& meshNodeAlloc) 
        : owner (owner), drawFlags (0), 
          meshNodes (MeshNodeTreeBlockRefAlloc (meshNodeAlloc)),
          totalRenderMeshes (0) 
      {}
      
      /**
       * Add a rendermesh to context, putting it in the right meshnode etc.
       */
      void AddRenderMesh (csRenderMesh* rm, 
			  CS::Graphics::RenderPriority renderPrio,
			  typename MeshNode::SingleMesh& singleMeshTemplate)
      {
        bool isTransparent = IsTransparent(rm, renderPrio);

	typename TreeTraits::MeshNodeKeyType meshKey = 
	  TreeTraits::GetMeshNodeKey (renderPrio, *rm, isTransparent);
	
	// Get the mesh node
	MeshNode* meshNode = meshNodes.Get (meshKey, 0);
	if (!meshNode)
	{
	  // Get a new one
	  meshNode = owner.CreateMeshNode (*this, meshKey);
    
	  TreeTraitsType::SetupMeshNode(*meshNode, renderPrio, *rm, isTransparent);
	  meshNodes.Put (meshKey, meshNode);
	}
    
	csRef<csShaderVariable> svObjectToWorld;
	svObjectToWorld.AttachNew (new csShaderVariable (
	  owner.GetPersistentData ().svObjectToWorldName));
	svObjectToWorld->SetValue (rm->object2world);
	csRef<csShaderVariable> svObjectToWorldInv;
	svObjectToWorldInv.AttachNew (new csShaderVariable (
	  owner.GetPersistentData ().svObjectToWorldInvName));
	svObjectToWorldInv->SetValue (rm->object2world.GetInverse());
    
	typename MeshNode::SingleMesh sm (singleMeshTemplate);
	sm.renderMesh = rm;
	sm.svObjectToWorld = svObjectToWorld;
	sm.svObjectToWorldInv = svObjectToWorldInv;
	if (rm->z_buf_mode != (csZBufMode)~0) 
	  sm.zmode = rm->z_buf_mode;
    
	meshNode->meshes.Push (sm);
	totalRenderMeshes++;
      }

      /// Add a new render layer after \a layer
      void InsertLayer (size_t after)
      {
        const size_t layerCount = shaderArray.GetSize() / totalRenderMeshes;
        shaderArray.SetSize (shaderArray.GetSize() + totalRenderMeshes);
        const size_t layerOffsetNew = (after + 1) * totalRenderMeshes;
        memmove (shaderArray.GetArray() + layerOffsetNew + totalRenderMeshes,
          shaderArray.GetArray() + layerOffsetNew,
          totalRenderMeshes * (layerCount - after - 1) * sizeof(csShaderVariable*));
        memset (shaderArray.GetArray() + layerOffsetNew, 0,
          totalRenderMeshes * sizeof(csShaderVariable*));

        svArrays.InsertLayer (after, after);
      }

      /**
       * Copy the shader for mesh \a meshId from layer \a fromLayer to layer
       * \a toLayer.
       */
      void CopyLayerShader (size_t meshId, size_t fromLayer, size_t toLayer)
      {
        const size_t fromLayerOffset = fromLayer * totalRenderMeshes;
        const size_t toLayerOffset = toLayer * totalRenderMeshes;
        shaderArray[toLayerOffset + meshId] =
          shaderArray[fromLayerOffset + meshId];
      }
      
      /**
       * Get the dimension of the render target set for this context.
       */
      bool GetTargetDimensions (int& renderW, int& renderH)
      {
	for (int a = 0; a < rtaNumAttachments; a++)
	{
	  if (renderTargets[a].texHandle != 0)
	  {
	    renderTargets[a].texHandle->GetRendererDimensions (
	      renderW, renderH);
	    return true;
	  }
        }
        if (renderView.IsValid())
        {
          iGraphics3D* g3d = renderView->GetGraphics3D();
          if (g3d)
          {
            renderW = g3d->GetWidth();
            renderH = g3d->GetHeight();
          }
        }
        return false;
      }

      /**
       * Returns true if the given render mesh is considered transparent.
       */
      bool IsTransparent (csRenderMesh* rm, 
                          CS::Graphics::RenderPriority renderPrio)
      {
        // TODO: make customizable.
        return renderPrio == 9 || renderPrio == 10;
      }

    };


    //---- Methods
    DeferredRenderTree (PersistentData &dataStorage)
      : RenderTreeBase (dataStorage.debugPersist), persistentData (dataStorage)
    {}

    ~DeferredRenderTree ()
    {
      persistentData.Clear ();
    }

    /**
     * Create a new context
     * \param rw Render view to associate the new context with. Should be 
     * initialized before calling method.
     * \param insertAfter Context to insert this one after. Must be a valid
     * context in current render tree.
     */
    ContextNode* CreateContext (RenderView* rw, ContextNode* insertAfter = 0)
    {
      // Create an initial context
      ContextNode* newCtx = persistentData.contextNodeAllocator.Alloc (*this,
        persistentData.meshNodeTreeAlloc);
      newCtx->renderView = rw;
      newCtx->cameraTransform = rw->GetCamera ()->GetTransform ();
      newCtx->sector = rw->GetThisSector();

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
    void DestroyContext (ContextNode* context)
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

    /// Debugging helper: whether debug screen clearing is enabled
    bool IsDebugClearEnabled () const
    {
      return IsDebugFlagEnabled (persistentData.dbgDebugClearScreen);
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
    PersistentData &persistentData;
    ContextNodeArrayType contexts; 
  };

}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

#endif // __CS_DEFERREDRENDERTREE_H__
