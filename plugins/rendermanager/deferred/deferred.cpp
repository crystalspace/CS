/*
    Copyright (C) 2010 by Joe Forte

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "csplugincommon/rendermanager/renderview.h"
#include "csplugincommon/rendermanager/dependenttarget.h"
#include "csplugincommon/rendermanager/hdrhelper.h"
#include "csplugincommon/rendermanager/lightsetup.h"
#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/portalsetup.h"
#include "csplugincommon/rendermanager/posteffects.h"
#include "csplugincommon/rendermanager/render.h"
#include "csplugincommon/rendermanager/renderlayers.h"
#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/shadersetup.h"
#include "csplugincommon/rendermanager/standardsorter.h"
#include "csplugincommon/rendermanager/svsetup.h"
#include "csplugincommon/rendermanager/viscull.h"

#include "iengine.h"
#include "ivideo.h"
#include "ivaria/reporter.h"
#include "csutil/cfgacc.h"

#include "deferred.h"

using namespace CS::RenderManager;

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{

SCF_IMPLEMENT_FACTORY(RMDeferred);

/**
 * Allocates a RenderView using placement new with the given memory pool.
 */
CS::RenderManager::RenderView *CreateNewRenderView(CS::RenderManager::RenderView::Pool &pool, iView *view)
{
  #include "csutil/custom_new_disable.h"

  RenderView *rview = new(pool) CS::RenderManager::RenderView (view);

  #include "csutil/custom_new_enable.h"

  return rview;
}

/**
 * Draws the given texture over the contents of the entire screen.
 */
void DrawFullscreenTexture(iTextureHandle *tex, iGraphics3D *graphics3D)
{
  iGraphics2D *graphics2D = graphics3D->GetDriver2D ();

  int w, h;
  tex->GetRendererDimensions (w, h);

  graphics3D->SetZMode (CS_ZBUF_NONE);
  graphics3D->BeginDraw (CSDRAW_2DGRAPHICS | CSDRAW_CLEARSCREEN);
  graphics3D->DrawPixmap (tex, 
                          0, 
                          0,
                          graphics2D->GetWidth (),
                          graphics2D->GetHeight (),
                          0,
                          0,
                          w,
                          h,
                          0);
}

/**
 * Iterate over all lights within a context, call functor for each one.
 * Does not use any blocking at all.
 */
template<typename ContextType, typename Fn>
void ForEachLight(ContextType& context, Fn& fn)
{
  iLightList *list = context.sector->GetLights ();

  const int count = list->GetCount ();
  for (int i = 0; i < count; i++)
  {
    fn (list->Get (i));
  }
}

/**
 * Iterate over all mesh nodes within context, call functor for each one
 * not marked to use forward rendering. Does not use any blocking at all.
 */
template<typename ContextType, typename Fn>
void ForEachDeferredMeshNode(ContextType &context, Fn &fn)
{
  typedef CS::RenderManager::Implementation::NoOperationBlock<
    typename ContextType::TreeType::MeshNode*
  > noBlockType;

  typename ContextType::TreeType::MeshNodeTreeIteratorType it = 
    context.meshNodes.GetIterator ();

  noBlockType noBlock;
  // Helper object for calling function
  CS::RenderManager::Implementation::OperationCaller<
    Fn, 
    noBlockType,
    typename OperationTraits<Fn>::Ordering
  > caller (fn, noBlock);

  while (it.HasNext ())
  {
    typename ContextType::TreeType::MeshNode *node = it.Next ();
    CS_ASSERT_MSG("Null node encountered, should not be possible", node);

    if (!node->useForwardRendering)
      caller (node);
  }
}

/**
 * Iterate over all mesh nodes within context, call functor for each one 
 * marked to use forward rendering. Does not use any blocking at all.
 */
template<typename ContextType, typename Fn>
void ForEachForwardMeshNode(ContextType &context, Fn &fn)
{
  typedef CS::RenderManager::Implementation::NoOperationBlock<
    typename ContextType::TreeType::MeshNode*
  > noBlockType;

  typename ContextType::TreeType::MeshNodeTreeIteratorType it = 
    context.meshNodes.GetIterator ();

  noBlockType noBlock;
  // Helper object for calling function
  CS::RenderManager::Implementation::OperationCaller<
    Fn, 
    noBlockType,
    typename OperationTraits<Fn>::Ordering
  > caller (fn, noBlock);

  while (it.HasNext ())
  {
    typename ContextType::TreeType::MeshNode *node = it.Next ();
    CS_ASSERT_MSG("Null node encountered, should not be possible", node);

    if (node->useForwardRendering)
      caller (node);
  }
}

/**
 * Sets up shaders for objects that can choose to use deferred or forward shading. 
 */
template<typename RenderTree, typename LayerConfigType>
class DeferredShaderSetup
{
public:
  DeferredShaderSetup(ShaderArrayType &shaderArray, 
                      const LayerConfigType &layerConfig,
                      int deferredLayer)
    : 
  shaderArray(shaderArray), 
  layerConfig(layerConfig), 
  deferredLayer(deferredLayer)
  {}

  void operator() (typename RenderTree::MeshNode *node)
  {
    if (node->useForwardRendering)
      ForwardSetup (node);
    else
      DeferredSetup (node);
  }

   /**
   * Sets up a mesh node to not use the deferred layer.
   */
  void ForwardSetup(typename RenderTree::MeshNode *node)
  {
    // Get the shader
    const size_t totalMeshes = node->owner.totalRenderMeshes;

    const size_t meshCount = node->meshes.GetSize ();
    for (size_t i = 0; i < meshCount; i++)
    {
      typename RenderTree::MeshNode::SingleMesh &mesh = node->meshes[i];
      csRenderMesh *rm = mesh.renderMesh;

      // Setup the deferred layer.
      size_t layerOffset = deferredLayer * totalMeshes;
      shaderArray[mesh.contextLocalId + layerOffset] = nullptr;

      // Setup the forward rendering layers.
      const size_t layerCount = layerConfig.GetLayerCount ();
      for (size_t layer = 0; layer < layerCount; layer++)
      {
        if ((int)layer == deferredLayer)
          continue;

        size_t layerOffset = layer * totalMeshes;

        iShader *shader = nullptr;
        if (rm->material)
        {
          size_t layerShaderNum;
          const csStringID *layerShaders = layerConfig.GetShaderTypes (layer, layerShaderNum);

          shader = rm->material->GetMaterial ()->GetFirstShader (layerShaders, layerShaderNum);
        }
        
        if (shader)
          shaderArray[mesh.contextLocalId + layerOffset] = shader;
        else
          shaderArray[mesh.contextLocalId + layerOffset] = layerConfig.GetDefaultShader (layer);
      }
    }
  }

  /**
   * Sets up a mesh node to only use the deferred layer.
   */
  void DeferredSetup(typename RenderTree::MeshNode *node)
  {
    // Get the shader
    const size_t totalMeshes = node->owner.totalRenderMeshes;

    const size_t meshCount = node->meshes.GetSize ();
    for (size_t i = 0; i < meshCount; i++)
    {
      typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes[i];
      csRenderMesh *rm = mesh.renderMesh;

      // Setup the deferred layer.
      iShader *shader = nullptr;
      if (rm->material)
      {
        size_t layerShaderNum;
        const csStringID* layerShaders = layerConfig.GetShaderTypes (deferredLayer, layerShaderNum);

        shader = rm->material->GetMaterial ()->GetFirstShader (layerShaders, layerShaderNum);
      }

      size_t layerOffset = deferredLayer * totalMeshes;
      if (shader)
        shaderArray[mesh.contextLocalId + layerOffset] = shader;
      else
        shaderArray[mesh.contextLocalId + layerOffset] = layerConfig.GetDefaultShader (deferredLayer);

      // Setup the forward rendering layers.
      const size_t layerCount = layerConfig.GetLayerCount ();
      for (size_t layer = 0; layer < layerCount; layer++)
      {
        if ((int)layer == deferredLayer)
          continue;

        size_t layerOffset = layer * totalMeshes;
        shaderArray[mesh.contextLocalId + layerOffset] = nullptr;
      }
    }
  }

private:
  ShaderArrayType &shaderArray;
  const LayerConfigType &layerConfig;
  int deferredLayer;
};

/**
 * Iterates through the mesh nodes executing the DeferredShaderSetup functor.
 */
template<typename ContextNodeType, typename LayerConfigType>
void DeferredSetupShader(ContextNodeType &context, 
                         iShaderManager *shaderManager,
                         const LayerConfigType &layerConfig,
                         int deferredLayer)
{
  context.shaderArray.SetSize (context.totalRenderMeshes * layerConfig.GetLayerCount ());

  // Shader setup
  typedef typename ContextNodeType::TreeType Tree;
  
  DeferredShaderSetup<Tree, LayerConfigType>
    shaderSetup (context.shaderArray, layerConfig, deferredLayer);

  ForEachMeshNode (context, shaderSetup);
}

/**
 * Renderer for multiple contexts where all forward rendering objects are drawn.
 */
template<typename RenderTree>
class ForwardMeshTreeRenderer
{
public:

  ForwardMeshTreeRenderer(iGraphics3D* g3d, iShaderManager *shaderMgr, int deferredLayer)
    : 
  meshRender(g3d, shaderMgr),
  graphics3D(g3d),
  shaderMgr(shaderMgr),
  deferredLayer(deferredLayer)
  {}

  ~ForwardMeshTreeRenderer() {}

  /**
   * Render all contexts.
   */
  void operator()(typename RenderTree::ContextNode *context)
  {
    RenderView *rview = context->renderView;
    
    iCamera *cam = rview->GetCamera ();
    iClipper2D *clipper = rview->GetClipper ();
    
    // Setup the camera etc.. @@should be delayed as well
    graphics3D->SetProjectionMatrix (context->perspectiveFixup * cam->GetProjectionMatrix ());
    graphics3D->SetClipper (clipper, CS_CLIPPER_TOPLEVEL);

    graphics3D->SetWorldToCamera (context->cameraTransform.GetInverse ());

    size_t layerCount = context->svArrays.GetNumLayers ();
    for (size_t layer = 0; layer < layerCount; ++layer)
    {
      if ((int)layer == deferredLayer)
        continue;

      meshRender.SetLayer (layer);
      ForEachForwardMeshNode (*context, meshRender);
    }
  }
 
private:

  SimpleContextRender<RenderTree> meshRender;

  iGraphics3D *graphics3D;
  iShaderManager *shaderMgr;

  int deferredLayer;
};

/**
 * Deferred renderer for multiple contexts.
 *
 * Example:
 * \code
 * // ... contexts setup etc. ...
 *
 * {
 *   DeferredTreeRenderer<RenderTree> 
 *     render (graphics3D, shaderManager, stringSet, 
 *             gbuffer, lightRenderPersistent, deferredLayer);
 *
 *   ForEachContextReverse (renderTree, render);
 * }
 *
 * // ... apply post processing ...
 * \endcode
 */
template<typename RenderTree>
class DeferredTreeRenderer
{
public:

  DeferredTreeRenderer(iGraphics3D *g3d, 
                       iShaderManager *shaderMgr,
                       iStringSet *stringSet,
                       GBuffer &gbuffer,
                       DeferredLightRenderer::PersistentData &lightRenderPersistent,
                       int deferredLayer)
    : 
  meshRender(g3d, shaderMgr),
  graphics3D(g3d),
  shaderMgr(shaderMgr),
  stringSet(stringSet),
  gbuffer(gbuffer),
  lightRenderPersistent(lightRenderPersistent),
  deferredLayer(deferredLayer),
  lastAccumBuf(nullptr),
  lastSubTex(-1),
  lastRenderView(nullptr)
  {}

  ~DeferredTreeRenderer() 
  {
    RenderContextStack ();
  }

  /**
   * Render all contexts.
   */
  void operator()(typename RenderTree::ContextNode *context)
  {
    if (IsNew (*context))
    {
      // New context, render out the old ones
      RenderContextStack ();

      lastAccumBuf = context->renderTargets[rtaColor0].texHandle;
      lastSubTex = context->renderTargets[rtaColor0].subtexture;
      lastRenderView = context->renderView;
    }
    
    contextStack.Push (context);
  }

protected:

  void RenderContextStack()
  {
    const size_t ctxCount = contextStack.GetSize ();
/*
    for (size_t i = 0; i < ctxCount; i++)
    {
      Render (contextStack[i], i == 0);
    }*/
    
    if (ctxCount == 0)
      return;
    
    typename RenderTree::ContextNode *context = contextStack[0];

    RenderView *rview = context->renderView;

    iEngine *engine = rview->GetEngine ();
    iCamera *cam = rview->GetCamera ();
    iClipper2D *clipper = rview->GetClipper ();

    iGraphics2D *graphics2D = graphics3D->GetDriver2D ();

    for (size_t i = 0; i < ctxCount; i++)
    {
      typename RenderTree::ContextNode *context = contextStack[i];

      // Fill the gbuffer
      gbuffer.Attach ();
      {
        graphics3D->SetZMode (CS_ZBUF_MESH);

        // Setup the camera etc.
        graphics3D->SetProjectionMatrix (context->perspectiveFixup * cam->GetProjectionMatrix ());
        graphics3D->SetClipper (clipper, CS_CLIPPER_TOPLEVEL);

        int drawFlags = engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS | context->drawFlags;
        graphics3D->BeginDraw (drawFlags);
        graphics3D->SetWorldToCamera (context->cameraTransform.GetInverse ());

        meshRender.SetLayer (deferredLayer);
        ForEachDeferredMeshNode (*context, meshRender);
        
        graphics3D->FinishDraw ();
      }
      gbuffer.Detach ();

      // Fills the accumulation buffer
      AttachAccumBuffer (context, false);
      {
        int drawFlags = engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS | context->drawFlags;
        drawFlags &= ~CSDRAW_CLEARSCREEN;

        if (i == 0)
          graphics2D->Clear (graphics2D->FindRGB (0, 0, 0));

        graphics3D->BeginDraw (drawFlags);
        graphics3D->SetWorldToCamera (context->cameraTransform.GetInverse ());

        // Iterate through lights adding results into accumulation buffer.
        DeferredLightRenderer render (graphics3D,
                                      shaderMgr,
                                      stringSet,
                                      rview,
                                      gbuffer,
                                      lightRenderPersistent);

        ForEachLight (*context, render);
      }
      DetachAccumBuffer ();
    }

    // Draws the forward shaded objects.
    AttachAccumBuffer (context, true);
    {
      graphics3D->SetZMode (CS_ZBUF_MESH);

      for (size_t i = 0; i < ctxCount; i++)
      {
        typename RenderTree::ContextNode *context = contextStack[i];

        ForwardMeshTreeRenderer<RenderTreeType> render (graphics3D, shaderMgr, deferredLayer);
        render (context);
      }

      graphics3D->FinishDraw ();
    }
    DetachAccumBuffer ();

    contextStack.Empty ();
  }

  void Render(typename RenderTree::ContextNode *context, bool first)
  {
    RenderView *rview = context->renderView;

    iEngine *engine = rview->GetEngine ();
    iCamera *cam = rview->GetCamera ();
    iClipper2D *clipper = rview->GetClipper ();

    iGraphics2D *graphics2D = graphics3D->GetDriver2D ();

    // Fill the gbuffer
    gbuffer.Attach ();
    {
      graphics3D->SetZMode (CS_ZBUF_MESH);

      // Setup the camera etc.
      graphics3D->SetProjectionMatrix (context->perspectiveFixup * cam->GetProjectionMatrix ());
      graphics3D->SetClipper (clipper, CS_CLIPPER_TOPLEVEL);

      int drawFlags = engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS | context->drawFlags;

      if (!first)
        drawFlags &= ~CSDRAW_CLEARSCREEN;
      
      graphics3D->BeginDraw (drawFlags);
      graphics3D->SetWorldToCamera (context->cameraTransform.GetInverse ());

      meshRender.SetLayer (deferredLayer);
      ForEachDeferredMeshNode (*context, meshRender);

      graphics3D->FinishDraw ();
    }
    gbuffer.Detach ();

    // Fills the accumulation buffer
    AttachAccumBuffer (context, false);
    {
      int drawFlags = engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS | context->drawFlags;
      drawFlags &= ~CSDRAW_CLEARSCREEN;

      if (first)
        graphics2D->Clear (graphics2D->FindRGB (0, 0, 0));

      graphics3D->BeginDraw (drawFlags);
      graphics3D->SetWorldToCamera (context->cameraTransform.GetInverse ());

      // Iterate through lights adding results into accumulation buffer.
      DeferredLightRenderer render (graphics3D,
                                    shaderMgr,
                                    stringSet,
                                    rview,
                                    gbuffer,
                                    lightRenderPersistent);

      //render.OutputAmbientLight ();

      ForEachLight (*context, render);
    }
    DetachAccumBuffer ();

    // Draws the forward shaded objects.
    AttachAccumBuffer (context, true);
    {
      graphics3D->SetZMode (CS_ZBUF_MESH);

      ForwardMeshTreeRenderer<RenderTreeType> render (graphics3D, shaderMgr, deferredLayer);
      render (context);

      graphics3D->FinishDraw ();
    }
    DetachAccumBuffer ();
  }

  /**
   * Attaches the accumulation buffer stored with the given context.
   */
  bool AttachAccumBuffer(typename RenderTree::ContextNode *context, bool useGbufferDepth)
  {
    int subTex;
    iTextureHandle *buf = GetAccumBuffer (context, subTex);
    
    CS_ASSERT (buf);

    return AttachAccumBuffer (buf, subTex, useGbufferDepth);
  }

  /**
   * Attaches the given accumulation buffer and uses the gbuffers depth if requested.
   */
  bool AttachAccumBuffer(iTextureHandle *buf, int subTex, bool useGbufferDepth)
  {
    CS_ASSERT (buf);

    // Assume that the accumulation texture matches the gbuffer dimensions.
#if CS_DEBUG
    int w, h;
    int bufW, bufH;
    gbuffer.GetDimensions (w, h);
    buf->GetRendererDimensions (bufW, bufH);

    CS_ASSERT (w == bufW && h == bufH);
#endif

    if (!graphics3D->SetRenderTarget (buf, false, subTex, rtaColor0))
      return false;

    if (useGbufferDepth && gbuffer.HasDepthBuffer ())
    {
      if (!graphics3D->SetRenderTarget (gbuffer.GetDepthBuffer (), false, 0, rtaDepth))
        return false; 
    }

    if (!graphics3D->ValidateRenderTargets ())
      return false;

    return true;
  }

  /**
   * Detaches the accumulation buffer.
   */
  void DetachAccumBuffer()
  {
    graphics3D->UnsetRenderTargets ();
  }

   /**
    * Returns the contexts accumulation buffer or NULL if no such buffer exists.
    */
  iTextureHandle *GetAccumBuffer(typename RenderTree::ContextNode *context, int &subTex)
  {
    subTex = context->renderTargets[rtaColor0].subtexture;
    return context->renderTargets[rtaColor0].texHandle;
  }

  /**
   * Returns true if the given context has a valid accumulation buffer.
   */
  bool HasAccumBuffer(typename RenderTree::ContextNode *context)
  {
    int subTex;
    iTextureHandle *buf = GetAccumBuffer (context, subTex);
    return buf != nullptr; 
  }

  /**
   * Returns true if the given context has the same accumulation buffer 
   * as the last context.
   */
  bool HasSameAccumBuffer(typename RenderTree::ContextNode &context)
  {
    int subTex;
    iTextureHandle *buf = GetAccumBuffer (&context, subTex);

    return buf == lastAccumBuf && subTex == lastSubTex;
  }

  /**
   * Returns true if the given context has the same render view as the 
   * last context.
   */
  bool HasSameRenderView(typename RenderTree::ContextNode &context)
  {
    return context.renderView == lastRenderView;
  }

  /**
   * Returns true if the given context is different from the last context.
   */
  bool IsNew(typename RenderTree::ContextNode &context)
  {
    return !HasSameAccumBuffer (context) || !HasSameRenderView (context);
  }

private:

  SimpleContextRender<RenderTree> meshRender;

  iGraphics3D *graphics3D;
  iShaderManager *shaderMgr;
  iStringSet *stringSet;

  GBuffer &gbuffer;
  DeferredLightRenderer::PersistentData &lightRenderPersistent;

  csArray<typename RenderTree::ContextNode*> contextStack;

  int deferredLayer;

  iTextureHandle *lastAccumBuf;
  int lastSubTex;
  RenderView *lastRenderView;
};

//----------------------------------------------------------------------
template<typename RenderTreeType, typename LayerConfigType>
class StandardContextSetup
{
public:
  typedef StandardContextSetup<RenderTreeType, LayerConfigType> ThisType;
  typedef StandardPortalSetup<RenderTreeType, ThisType> PortalSetupType;

  StandardContextSetup(RMDeferred *rmanager, const LayerConfigType &layerConfig)
    : 
  rmanager(rmanager), 
  layerConfig(layerConfig),
  recurseCount(0), 
  deferredLayer(rmanager->deferredLayer),
  maxPortalRecurse(rmanager->maxPortalRecurse)
  {}
  
  StandardContextSetup (const StandardContextSetup &other, const LayerConfigType &layerConfig)
    :
  rmanager(other.rmanager), 
  layerConfig(layerConfig),
  recurseCount(other.recurseCount),
  deferredLayer(other.deferredLayer),
  maxPortalRecurse(other.maxPortalRecurse)
  {}

  void operator()(typename RenderTreeType::ContextNode &context, 
                  typename PortalSetupType::ContextSetupData &portalSetupData)
  {
    CS::RenderManager::RenderView* rview = context.renderView;
    iSector* sector = rview->GetThisSector ();

    if (recurseCount > maxPortalRecurse) return;
    
    iShaderManager* shaderManager = rmanager->shaderManager;

    // @@@ This is somewhat "boilerplate" sector/rview setup.
    sector->PrepareDraw (rview);
    // Make sure the clip-planes are ok
    CS::RenderViewClipper::SetupClipPlanes (rview->GetRenderContext ());

    // Do the culling
    iVisibilityCuller *culler = sector->GetVisibilityCuller ();
    Viscull<RenderTreeType> (context, rview, culler);

    // Set up all portals
    {
      recurseCount++;
      PortalSetupType portalSetup (rmanager->portalPersistent, *this);      
      portalSetup (context, portalSetupData);
      recurseCount--;
    }
    
    // Sort the mesh lists  
    {
      StandardMeshSorter<RenderTreeType> mySorter (rview->GetEngine ());
      mySorter.SetupCameraLocation (rview->GetCamera ()->GetTransform ().GetOrigin ());
      ForEachMeshNode (context, mySorter);
    }

    // After sorting, assign in-context per-mesh indices
    {
      SingleMeshContextNumbering<RenderTreeType> numbering;
      ForEachMeshNode (context, numbering);
    }

    // Setup the SV arrays
    // Push the default stuff
    SetupStandardSVs (context, layerConfig, shaderManager, sector);

    // Setup the material&mesh SVs
    {
      StandardSVSetup<RenderTreeType, MultipleRenderLayer> svSetup (context.svArrays, layerConfig);
      ForEachMeshNode (context, svSetup);
    }

    // Setup shaders and tickets
    DeferredSetupShader (context, shaderManager, layerConfig, deferredLayer);

    // Setup lighting (only needed for transparent objects)
    RMDeferred::LightSetupType::ShadowParamType shadowParam;
    RMDeferred::LightSetupType lightSetup (rmanager->lightPersistent, 
                                           rmanager->lightManager,
                                           context.svArrays, 
                                           layerConfig, 
                                           shadowParam);

    ForEachForwardMeshNode (context, lightSetup);

    SetupStandardTicket (context, shaderManager, lightSetup.GetPostLightingLayers ());
  }

private:

  RMDeferred *rmanager;

  const LayerConfigType &layerConfig;

  int recurseCount;
  int deferredLayer;
  int maxPortalRecurse;
};

//----------------------------------------------------------------------
RMDeferred::RMDeferred(iBase *parent) 
  : 
scfImplementationType(this, parent),
portalPersistent(CS::RenderManager::TextureCache::tcacheExactSizeMatch)
{
  SetTreePersistent (treePersistent);
}

//----------------------------------------------------------------------
bool RMDeferred::Initialize(iObjectRegistry *registry)
{
  const char *messageID = "crystalspace.rendermanager.deferred";

  objRegistry = registry;

  csRef<iGraphics3D> graphics3D = csQueryRegistry<iGraphics3D> (objRegistry);
  iGraphics2D *graphics2D = graphics3D->GetDriver2D ();
   
  shaderManager = csQueryRegistry<iShaderManager> (objRegistry);
  lightManager = csQueryRegistry<iLightManager> (objRegistry);
  stringSet = csQueryRegistryTagInterface<iStringSet> (objRegistry, "crystalspace.shared.stringset");

  treePersistent.Initialize (shaderManager);
  portalPersistent.Initialize (shaderManager, graphics3D, treePersistent.debugPersist);
  lightPersistent.Initialize (registry, treePersistent.debugPersist);
  lightRenderPersistent.Initialize (registry);

  // Initialize the extra data in the persistent tree data.
  RenderTreeType::TreeTraitsType::Initialize (treePersistent, registry);
  
  // Read Config settings.
  csConfigAccess cfg (objRegistry);
  maxPortalRecurse = cfg->GetInt ("RenderManager.Deferred.MaxPortalRecurse", 30);
  showGBuffer = true;

  bool layersValid = false;
  const char *layersFile = cfg->GetStr ("RenderManager.Deferred.Layers", nullptr);
  if (layersFile)
  {
    csReport (objRegistry, CS_REPORTER_SEVERITY_NOTIFY, messageID, 
      "Reading render layers from '%s'", layersFile);

    layersValid = CS::RenderManager::AddLayersFromFile (objRegistry, layersFile, renderLayer);
    
    if (!layersValid) 
    {
      renderLayer.Clear();
    }
    else
    {
      // Locates the deferred shading layer.
      deferredLayer = LocateDeferredLayer (renderLayer);
      if (deferredLayer <= 0)
      {
        csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
          messageID, "The render layers file '%s' does not contain a 'gbuffer fill' layer.", layersFile);

        AddDeferredLayer (renderLayer, deferredLayer);
      }
    }
  }
  
  csRef<iLoader> loader = csQueryRegistry<iLoader> (objRegistry);
  if (!layersValid)
  {
    csReport (objRegistry, CS_REPORTER_SEVERITY_NOTIFY, messageID,
      "Using default render layers");

    AddDeferredLayer (renderLayer, deferredLayer);

    if (!loader->LoadShader ("/shader/lighting/lighting_default.xml"))
    {
      csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
        messageID, "Could not load lighting_default shader");
    }

    CS::RenderManager::AddDefaultBaseLayers (objRegistry, renderLayer);
  }

  // Creates the accumulation buffer.
  int flags = CS_TEXTURE_2D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP | CS_TEXTURE_NPOTS;
  
  scfString errStr;
  accumBuffer = graphics3D->GetTextureManager ()->CreateTexture (graphics2D->GetWidth (),
    graphics2D->GetHeight (),
    csimg2D,
    "rgb16_f",
    flags,
    &errStr);

  if (!accumBuffer)
  {
    csReport(objRegistry, CS_REPORTER_SEVERITY_ERROR, messageID, 
      "Could not create accumulation buffer: %s!", errStr.GetCsString ().GetDataSafe ());
    return false;
  }

  // Create GBuffer
  GBuffer::Description desc;
  desc.colorBufferCount = 3;
  desc.hasDepthBuffer = true;
  desc.width = graphics2D->GetWidth ();
  desc.height = graphics2D->GetHeight ();
  desc.colorBufferFormat = nullptr;

  if (!gbuffer.Initialize (desc, 
                           graphics3D, 
                           shaderManager->GetSVNameStringset (), 
                           objRegistry))
  {
    return false;
  }

  // Fix portal texture cache to only query textures that match the gbuffer dimensions.
  portalPersistent.fixedTexCacheWidth = desc.width;
  portalPersistent.fixedTexCacheHeight = desc.height;

  // Make sure the texture cache creates matching texture buffers.
  portalPersistent.texCache.SetFormat ("rgb16_f");
  portalPersistent.texCache.SetFlags (flags);

  return true;
}

//----------------------------------------------------------------------
bool RMDeferred::RenderView(iView *view)
{
  iGraphics3D *graphics3D = view->GetContext ();

  view->UpdateClipper ();

  int frameWidth = graphics3D->GetWidth ();
  int frameHeight = graphics3D->GetHeight ();
  view->GetCamera ()->SetViewportSize (frameWidth, frameHeight);

  // Setup renderview
  csRef<CS::RenderManager::RenderView> rview;
  rview.AttachNew (CreateNewRenderView (treePersistent.renderViewPool, view));

  // Computes the left, right, top, and bottom of the view frustum.
  iPerspectiveCamera *camera = view->GetPerspectiveCamera ();
  float invFov = camera->GetInvFOV ();
  float l = -invFov * camera->GetShiftX ();
  float r =  invFov * (frameWidth - camera->GetShiftX ());
  float t = -invFov * camera->GetShiftY ();
  float b =  invFov * (frameHeight - camera->GetShiftY ());
  rview->SetFrustum (l, r, t, b);

  portalPersistent.UpdateNewFrame ();
  lightPersistent.UpdateNewFrame ();

  iEngine *engine = view->GetEngine ();
  engine->UpdateNewFrame ();  
  engine->FireStartFrame (rview);

  iSector *startSector = rview->GetThisSector ();
  if (!startSector)
    return false;

  RenderTreeType renderTree (treePersistent);
  RenderTreeType::ContextNode *startContext = renderTree.CreateContext (rview);

  // Add gbuffer textures to be visualized.
  if (showGBuffer)
    ShowGBuffer (renderTree);

  // Setup the main context
  {
    ContextSetupType contextSetup (this, renderLayer);
    ContextSetupType::PortalSetupType::ContextSetupData portalData (startContext);

    startContext->renderTargets[rtaColor0].texHandle = accumBuffer;
    startContext->renderTargets[rtaColor0].subtexture = 0;

    contextSetup (*startContext, portalData);
  }

  // Render all contexts.
  {
    DeferredTreeRenderer<RenderTreeType> render (graphics3D,
                                                 shaderManager,
                                                 stringSet,
                                                 gbuffer,
                                                 lightRenderPersistent,
                                                 deferredLayer);
    ForEachContextReverse (renderTree, render);
  }

  // Output the final result to the backbuffer.
  {
    DrawFullscreenTexture (accumBuffer, graphics3D);

    graphics3D->FinishDraw ();
  }

  DebugFrameRender (rview, renderTree);

  return true;
}

//----------------------------------------------------------------------
bool RMDeferred::PrecacheView(iView *view)
{
  return RenderView (view);
}

//----------------------------------------------------------------------
void RMDeferred::AddDeferredLayer(CS::RenderManager::MultipleRenderLayer &layers, int &addedLayer)
{
  const char *messageID = "crystalspace.rendermanager.deferred";

  csRef<iLoader> loader = csQueryRegistry<iLoader> (objRegistry);

  if (!loader->LoadShader ("/shader/deferred/fill_gbuffer.xml"))
  {
    csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
      messageID, "Could not load fill_gbuffer shader");
  }

  iShader *shader = shaderManager->GetShader ("fill_gbuffer");

  SingleRenderLayer baseLayer (shader, 0, 0);
  baseLayer.AddShaderType (stringSet->Request("gbuffer fill"));

  renderLayer.AddLayers (baseLayer);

  addedLayer = renderLayer.GetLayerCount () - 1;
}

//----------------------------------------------------------------------
int RMDeferred::LocateDeferredLayer(const CS::RenderManager::MultipleRenderLayer &layers)
{
  csStringID deferredShaderType = stringSet->Request("gbuffer fill");

  size_t count = renderLayer.GetLayerCount ();
  for (size_t i = 0; i < count; i++)
  {
    size_t num;
    const csStringID *strID = renderLayer.GetShaderTypes (i, num);
    for (size_t j = 0; j < num; j++)
    {
      if (strID[j] == deferredShaderType)
      {
        return i;
      }
    }
  }

  return -1;
}

//----------------------------------------------------------------------
void RMDeferred::ShowGBuffer(RenderTreeType &tree)
{
  size_t count = gbuffer.GetColorBufferCount ();
  if (count > 0)
  {
    int w, h;
    gbuffer.GetColorBuffer (0)->GetRendererDimensions (w, h);
    float aspect = (float)w / h;

    for (size_t i = 0; i < count; i++)
    {
      tree.AddDebugTexture (gbuffer.GetColorBuffer (i), aspect);
    }

    if (gbuffer.GetDepthBuffer ())
    {
      tree.AddDebugTexture (gbuffer.GetDepthBuffer (), aspect);
    }
  }
}

//----------------------------------------------------------------------
bool RMDeferred::DebugCommand(const char *cmd)
{
  if (strcmp (cmd, "toggle_visualize_gbuffer") == 0)
  {
    showGBuffer = !showGBuffer;
    return true;
  }

  return false;
}

}
CS_PLUGIN_NAMESPACE_END(RMDeferred)
