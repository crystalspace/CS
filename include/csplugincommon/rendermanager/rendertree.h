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

/**\file
 * Render tree
 */

#include "iengine/camera.h"
#include "csplugincommon/rendermanager/standardtreetraits.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/metautils.h"
#include "csutil/redblacktree.h"
#include "cstool/rendermeshholder.h"

struct iMeshWrapper;
struct iPortalContainer;

namespace CS
{
namespace RenderManager
{
  class PostEffectManager;
  
  /**
   * Helper class containing stuff which doesn't require any of the template
   * parameters to RenderTree
   */
  class CS_CRYSTALSPACE_EXPORT RenderTreeBase
  {
  public:
   struct CS_CRYSTALSPACE_EXPORT DebugPersistent
    {
      DebugPersistent ();
      
      uint RegisterDebugFlag (const char* string);
      uint QueryDebugFlag (const char* string);
    
      bool IsDebugFlagEnabled (uint flag);
      void EnableDebugFlag (uint flag, bool state);
    protected:
      uint nextDebugId;
      csHash<uint, csString> debugIdMappings;
      csHash<csArray<uint>, uint> debugIdChildren;
      csBitArray debugFlags;
    };
  
  protected:
    struct DebugTexture
    {
      csRef<iTextureHandle> texh;
      float aspect;
    };
    csArray<DebugTexture> debugTextures;
    DebugPersistent& debugPersist;
    
    RenderTreeBase (DebugPersistent& debugPersist)
    : debugPersist (debugPersist) {}
  public:
    /**\name Debugging helpers: toggling of debugging features
     * @{ */
    /**
     * Register a debug flag, returns a numeric ID.
     * \remark Flag names are hierarchical. The hierarchy levels are
     *   separated by dots. If a flag is set or unset, all flags below in the
     *   hierarchy are set or unset as well.
     */
    uint RegisterDebugFlag (const char* string)
    { return debugPersist.RegisterDebugFlag (string); }
    /**
     * Query whether a debug flag was registered and return its ID or
     * (uint)-1 if not registered.
     */
    uint QueryDebugFlag (const char* string)
    { return debugPersist.QueryDebugFlag (string); }
    
    /// Check whether a debug flag is enabled
    bool IsDebugFlagEnabled (uint flag) const
    { return debugPersist.IsDebugFlagEnabled (flag); }
    /**
     * Enable or disable a debug flag.
     * \remark Flag names are hierarchical. The hierarchy levels are
     *   separated by dots. If a flag is set or unset, all flags below in the
     *   hierarchy are set or unset as well.
     */
    void EnableDebugFlag (uint flag, bool state)
    { debugPersist.EnableDebugFlag (flag, state); }
    /**
     * Enable or disable a debug flag.
     * \remark Flag names are hierarchical. The hierarchy levels are
     *   separated by dots. If a flag is set or unset, all flags below in the
     *   hierarchy are set or unset as well.
     */
    void EnableDebugFlag (const char* flagStr, bool state)
    {
      uint flag = RegisterDebugFlag (flagStr);
      EnableDebugFlag (flag, state); 
    }
    /** @} */
 
    //@{
    /**\name Debugging helpers: debugging textures
     */
    /**
     * Add a texture to be rendered at the bottom of the view the next frame.
     */
    void AddDebugTexture (iTextureHandle* tex, float aspect = 1.0f);
    /**
     * Render out debug textures. To be called by the rendermanager at the end
     * of rendering a view.
     */
    void RenderDebugTextures (iGraphics3D* g3d);
    //@}
  
    //@{
    /**\name Debugging helpers: line drawing
     * \sa DrawDebugLines
     */
    /// Add debug line (world space)
    void AddDebugLine3D (const csVector3& v1, const csVector3& v2,
                         const csColor& color1, const csColor& color2);
    /**
     * Add debug line (arbitrary space, transformed to world space with 
     * \a toWorldSpace)
     */
    void AddDebugLine3DTF (const csVector3& v1, const csVector3& v2,
                           const csTransform& toWorldSpace,
                           const csColor& color1, const csColor& color2);
    /**
     * Add lines to visualize a bounding box (in arbitrary space, transformed 
     * to world space with \a toWorldSpace)
     */
    void AddDebugBBox (const csBox3& box,
                       const csTransform& toWorldSpace,
                       const csColor& col);
    /**
     * Add lines to visualize a plane (in arbitrary space, transformed 
     * to world space with \a toWorldSpace)
     */
    void AddDebugPlane (const csPlane3& _plane,
                        const csTransform& toWorldSpace,
                        const csColor& col,
                        const csVector3& linesOrg = csVector3 (0));
    /**
     * Visualize camera clip planes for the given view.
     */
    void AddDebugClipPlanes (RenderView* view);

    struct DebugLines
    {
      csDirtyAccessArray<csVector3> verts;
      csDirtyAccessArray<csVector4> colors;
    };
    /// Get all current debug lines. Useful to conserve the current lines
    const DebugLines& GetDebugLines () const { return debugLines; }
    /**
     * Set all current debug lines. Useful to e.g. set conserved lines from an
     * earlier frame.
     */
    void SetDebugLines (const DebugLines& lines) { debugLines = lines; }
    //@}
  
      
    //@{
    /**\name Debugging helpers: screen space line drawing
     * \sa DrawDebugLines
     */
    /**
     * Add a debug line (screen space - ie pixel coordinates!).
     */
    void AddDebugLineScreen (const csVector2& v1, const csVector2& v2,
      csRGBcolor color);
    //@}
    
    /**
     * Render out debug lines (world space and screen space). 
     * To be called by the rendermanager at the end of rendering a view.
     */
    void DrawDebugLines (iGraphics3D* g3d, RenderView* view);
  protected:
    DebugLines debugLines;
    struct DebugLineScreen
    {
      csVector2 v1, v2;
      csRGBcolor color;
    };
    csArray<DebugLineScreen> debugLinesScreen;
  };

  /**
   * RenderTree is the main data-structure for the rendermanagers.
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
  template<typename TreeTraits = RenderTreeStandardTraits>
  class RenderTree : public RenderTreeBase
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
    struct PersistentData : 
      public CS::Meta::EBOptHelper<typename TreeTraitsType::PersistentDataExtraDataType>
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
    
      CS::RenderManager::RenderViewCache renderViews;
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
      ContextNode& GetOwner() const { return *owner; }
      
      /// Our own key
      typename TreeTraitsType::MeshNodeKeyType key;

      /// Sorting
      int sorting;

      /// All the meshes within the meshnode
      MeshArrayType meshes;

      MeshNode (ContextNode& owner)
        : owner (&owner)
      {}
    protected:
      ContextNode* owner;

      friend struct ContextNode;
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
      /// Mesh render grouping
      CS::RenderPriorityGrouping renderGrouping;
      
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
	  renderGrouping (CS::rpgByLayer),
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
	typename TreeTraits::MeshNodeKeyType meshKey = 
	  TreeTraits::GetMeshNodeKey (renderPrio, *rm, owner.GetPersistentData ());
	
	// Get the mesh node
	MeshNode* meshNode = meshNodes.Get (meshKey, 0);
	if (!meshNode)
	{
	  // Get a new one
	  meshNode = owner.CreateMeshNode (*this, meshKey);
    
	  RenderTree::TreeTraitsType::SetupMeshNode(*meshNode, renderPrio, *rm, 
            owner.GetPersistentData ());
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

      void MoveRenderMeshes (MeshNodeTreeIteratorType& sourceMeshNode,
			     ContextNode* targetContext)
      {
	typename TreeTraits::MeshNodeKeyType srcKey;
	MeshNode* srcNode = sourceMeshNode.PeekNext (srcKey);

	CS_ASSERT(!targetContext->meshNodes.Get (srcKey, nullptr));
	size_t nodeMeshes = srcNode->meshes.GetSize ();
	srcNode->owner = targetContext;
	targetContext->meshNodes.Put (srcKey, srcNode);
	totalRenderMeshes -= nodeMeshes;
	targetContext->totalRenderMeshes += nodeMeshes;

	meshNodes.Delete (sourceMeshNode);
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
    };


    //---- Methods
    RenderTree (PersistentData& dataStorage)
      : RenderTreeBase (dataStorage.debugPersist), persistentData (dataStorage)
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
     * Clone a context. The new context is added <em>before</em> the
     * context to be cloned.
     */
    ContextNode* CloneContext (ContextNode* context)
    {
      // Create an initial context
      ContextNode* newCtx = persistentData.contextNodeAllocator.Alloc (*this,
        persistentData.meshNodeTreeAlloc);
      newCtx->renderView = context->renderView;
      memcpy (newCtx->renderTargets, context->renderTargets, sizeof (newCtx->renderTargets));
      newCtx->perspectiveFixup = context->perspectiveFixup;
      newCtx->cameraTransform = context->cameraTransform;
      newCtx->drawFlags = context->drawFlags;
      newCtx->renderGrouping = context->renderGrouping;
      newCtx->sector = context->sector;
      newCtx->svFogplane = context->svFogplane; // should that be copied?
      newCtx->shadervars = context->shadervars; // should that be copied?
      newCtx->postEffects = context->postEffects; // should that be copied?

      size_t n = contexts.Find (context);
      CS_ASSERT(n != csArrayItemNotFound);
      contexts.Insert (n, newCtx);

      return newCtx;
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
    PersistentData&         persistentData;
    ContextNodeArrayType    contexts; 
  };

}
}



#endif
