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
#include "csplugincommon/rendermanager/occluvis.h"
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

#include "deferredoperations.h"
#include "deferredshadersetup.h"
#include "deferredrender.h"
#include "deferred.h"

using namespace CS::RenderManager;

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{

SCF_IMPLEMENT_FACTORY(RMDeferred);

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
  zonlyLayer(rmanager->zonlyLayer),
  maxPortalRecurse(rmanager->maxPortalRecurse)
  {}
  
  StandardContextSetup (const StandardContextSetup &other, const LayerConfigType &layerConfig)
    :
  rmanager(other.rmanager), 
  layerConfig(layerConfig),
  recurseCount(other.recurseCount),
  deferredLayer(other.deferredLayer),
  zonlyLayer(other.zonlyLayer),
  maxPortalRecurse(other.maxPortalRecurse)
  {}

  void operator()(typename RenderTreeType::ContextNode &context, 
    typename PortalSetupType::ContextSetupData &portalSetupData,
    bool recursePortals = true)
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
    if (recursePortals)
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
    DeferredSetupShader (context, shaderManager, layerConfig, deferredLayer, zonlyLayer);

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
  int zonlyLayer;
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
  messageID = "crystalspace.rendermanager.deferred";

  objRegistry = registry;

  graphics3D = csQueryRegistry<iGraphics3D> (objRegistry);
  iGraphics2D *graphics2D = graphics3D->GetDriver2D ();
   
  shaderManager = csQueryRegistry<iShaderManager> (objRegistry);
  lightManager = csQueryRegistry<iLightManager> (objRegistry);
  stringSet = csQueryRegistryTagInterface<iStringSet> (objRegistry, "crystalspace.shared.stringset");
  svStringSet = shaderManager->GetSVNameStringset();

  treePersistent.Initialize (shaderManager);
  portalPersistent.Initialize (shaderManager, graphics3D, treePersistent.debugPersist);
  lightPersistent.Initialize (registry, treePersistent.debugPersist);
  lightRenderPersistent.Initialize (registry);
  
  PostEffectsSupport::Initialize (registry, "RenderManager.Deferred");
  //AddPostEffectLayer();

  // Initialize the extra data in the persistent tree data.
  RenderTreeType::TreeTraitsType::Initialize (treePersistent, registry);
  
  // Read Config settings.
  csConfigAccess cfg (objRegistry);
  maxPortalRecurse = cfg->GetInt ("RenderManager.Deferred.MaxPortalRecurse", 30);
  showGBuffer = false;
  drawLightVolumes = false;
  debugBuffer = ColorBuffer;
  idPostEffectOptionSV = csInvalidStringID;

  bool layersValid = false;
  const char *layersFile = cfg->GetStr ("RenderManager.Deferred.Layers", nullptr);
  if (layersFile)
  {
    csReport (objRegistry, CS_REPORTER_SEVERITY_NOTIFY, messageID, 
      "Reading render layers from %s", CS::Quote::Single (layersFile));

    layersValid = CS::RenderManager::AddLayersFromFile (objRegistry, layersFile, renderLayer);
    
    if (!layersValid) 
    {
      renderLayer.Clear();
    }
    else
    {
      // Locates the deferred shading layer.
      deferredLayer = LocateDeferredLayer (renderLayer);
      if (deferredLayer < 0)
      {
        csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
          messageID, "The render layers file %s does not contain a %s layer.",
	  CS::Quote::Single (layersFile),
	  CS::Quote::Single ("gbuffer fill"));

        AddDeferredLayer (renderLayer, deferredLayer);
      }

      // Locates the zonly shading layer.
      zonlyLayer = LocateZOnlyLayer (renderLayer);
      if (zonlyLayer < 0)
      {
        csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
          messageID, "The render layers file %s does not contain a %s layer.",
	  CS::Quote::Single (layersFile),
	  CS::Quote::Single ("depthwrite"));

        AddZOnlyLayer (renderLayer, zonlyLayer);
      }
    }
  }
  
  csRef<iLoader> loader = csQueryRegistry<iLoader> (objRegistry);
  if (!layersValid)
  {
    csReport (objRegistry, CS_REPORTER_SEVERITY_NOTIFY, messageID,
      "Using default render layers");

    AddZOnlyLayer (renderLayer, zonlyLayer);
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
  accumFmt = cfg->GetStr ("RenderManager.Deferred.AccumBufferFormat", "rgb16_f");

  scfString errStr;
  accumBuffer = graphics3D->GetTextureManager ()->CreateTexture (graphics2D->GetWidth (),
    graphics2D->GetHeight (),
    csimg2D,
    accumFmt,
    flags,
    &errStr);

  if (!accumBuffer)
  {
    csReport(objRegistry, CS_REPORTER_SEVERITY_ERROR, messageID, 
      "Could not create accumulation buffer: %s!", errStr.GetCsString ().GetDataSafe ());
    return false;
  }

  // Create GBuffer
  const char *gbufferFmt = cfg->GetStr ("RenderManager.Deferred.GBuffer.BufferFormat", "rgba16_f");
  int bufferCount = cfg->GetInt ("RenderManager.Deferred.GBuffer.BufferCount", 3);
  bool hasDepthBuffer = cfg->GetBool ("RenderManager.Deferred.GBuffer.DepthBuffer", true);

  GBuffer::Description desc;
  desc.colorBufferCount = bufferCount;
  desc.hasDepthBuffer = hasDepthBuffer;
  desc.width = graphics2D->GetWidth ();
  desc.height = graphics2D->GetHeight ();
  desc.colorBufferFormat = gbufferFmt;

  if (!gbuffer.Initialize (desc, graphics3D, shaderManager->GetSVNameStringset(), objRegistry))
  {
    return false;
  }

  if (!globalIllum.Initialize (graphics3D, objRegistry, &gbuffer) || !CreateDirectLightBuffer())
  {
    csReport (objRegistry, CS_REPORTER_SEVERITY_NOTIFY, messageID,
          "Dynamic global illumination is disabled");
  }
  else
  {
    csReport (objRegistry, CS_REPORTER_SEVERITY_NOTIFY, messageID, 
          "Dynamic global illumination is enabled");
  }

  // Fix portal texture cache to only query textures that match the gbuffer dimensions.
  portalPersistent.fixedTexCacheWidth = desc.width;
  portalPersistent.fixedTexCacheHeight = desc.height;

  // Make sure the texture cache creates matching texture buffers.
  portalPersistent.texCache.SetFormat (accumFmt);
  portalPersistent.texCache.SetFlags (flags);

  RMViscullCommon::Initialize (objRegistry, "RenderManager.Deferred");
  
  return true;
}

//----------------------------------------------------------------------
bool RMDeferred::CreateDirectLightBuffer()
{
  scfString errStr;
  iGraphics2D *graphics2D = graphics3D->GetDriver2D();
  int flags = CS_TEXTURE_2D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP | CS_TEXTURE_NPOTS;
  directLightBuffer = graphics3D->GetTextureManager()->CreateTexture (graphics2D->GetWidth(),
      graphics2D->GetHeight(), csimg2D, accumFmt, flags, &errStr);

  if (!directLightBuffer)
  {
    csReport (objRegistry, CS_REPORTER_SEVERITY_ERROR, messageID, 
      "Could not create direct light buffer: %s!", errStr.GetCsString ().GetDataSafe ());
    return false;
  }

  return true;
}

//----------------------------------------------------------------------
bool RMDeferred::RenderView(iView *view, bool recursePortals)
{
  iGraphics3D *graphics3D = view->GetContext ();
  iGraphics2D *graphics2D = graphics3D->GetDriver2D ();

  view->UpdateClipper ();

  int frameWidth = graphics3D->GetWidth ();
  int frameHeight = graphics3D->GetHeight ();
  view->GetCamera ()->SetViewportSize (frameWidth, frameHeight);

  // Setup renderview
  csRef<CS::RenderManager::RenderView> rview;
  rview = treePersistent.renderViews.GetRenderView (view);

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
  startContext->drawFlags |= (CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER);

  // Add gbuffer textures to be visualized.
  if (showGBuffer)
    ShowGBuffer (renderTree);

  //UpdatePostEffectsSV ();

  // Add global shader variable with viewport dimensions
  csShaderVariable *viewportSizeSV = shaderManager->GetVariableAdd (svStringSet->Request("viewport size"));
  viewportSizeSV->SetValue (csVector4 (graphics2D->GetWidth(), graphics2D->GetHeight(), 
    1.0f / graphics2D->GetWidth(), 1.0f / graphics2D->GetHeight()));
    
  // Add global shader variable with distance from near to far clip plane
  csShaderVariable *farPlaneSV = shaderManager->GetVariableAdd (svStringSet->Request("far clip distance"));
  float farPlaneDistance = rview->GetCamera()->GetFarPlane()->D();
  float nearPlaneDistance = camera->GetNearClipDistance();
  farPlaneSV->SetValue (fabs (farPlaneDistance - nearPlaneDistance));

  CS::Math::Matrix4 perspectiveFixup;
  postEffects.SetupView (view, perspectiveFixup);

  bool hasPostEffects = (postEffects.GetScreenTarget () != (iTextureHandle*)nullptr);

  // Setup the main context
  {
    ContextSetupType contextSetup (this, renderLayer);
    ContextSetupType::PortalSetupType::ContextSetupData portalData (startContext);

    if (hasPostEffects)
    {
      startContext->renderTargets[rtaColor0].texHandle = postEffects.GetScreenTarget ();
      startContext->perspectiveFixup = perspectiveFixup;
    }
    else
    {
      startContext->renderTargets[rtaColor0].texHandle = accumBuffer;
      startContext->renderTargets[rtaColor0].subtexture = 0;

      /*if (directLightBuffer)
      {
        startContext->renderTargets[rtaColor1].texHandle = directLightBuffer;
        startContext->renderTargets[rtaColor1].subtexture = 0;
      }*/
    }

    contextSetup (*startContext, portalData, recursePortals);
  }

  // Render all contexts.
  {
    DeferredTreeRenderer<RenderTreeType> render (graphics3D,
                                                 shaderManager,
                                                 stringSet,
                                                 gbuffer,
                                                 globalIllum,
                                                 directLightBuffer,
                                                 lightRenderPersistent,
                                                 deferredLayer,
                                                 zonlyLayer,
                                                 drawLightVolumes);

    ForEachContextReverse (renderTree, render);
  }

  if (hasPostEffects)
  {    
    //UpdatePostEffectsSV ();
    SetPostEffectShaderOption ();
    postEffects.DrawPostEffects (renderTree);
  }
  else
  {  
    // Output the final result to the backbuffer.
    DrawFullscreenTexture (accumBuffer, graphics3D);
  }

  DebugFrameRender (rview, renderTree);

  return true;
}

bool RMDeferred::RenderView(iView *view)
{
  return RenderView (view, true);
}

//----------------------------------------------------------------------
bool RMDeferred::PrecacheView(iView *view)
{
  return RenderView (view, false);

  postEffects.ClearIntermediates ();
}

//----------------------------------------------------------------------
void RMDeferred::AddPostEffectLayer()
{
  const char *messageID = "crystalspace.rendermanager.deferred";

  csRef<iLoader> loader = csQueryRegistry<iLoader> (objRegistry);
  if (!loader->LoadShader("shader/postproc/fx.xml"))
  {
    csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
      messageID, "Could not load post effect shader");
  }
  iShader *postEffectShader = shaderManager->GetShader ("fx");
  
  postEffects.AddLayer (postEffectShader);
}

//----------------------------------------------------------------------
void RMDeferred::SetPostEffectShaderOption()
{
  if (idPostEffectOptionSV == csInvalidStringID) 
  {
    iShaderVarStringSet *svStringSet = shaderManager->GetSVNameStringset ();
    idPostEffectOptionSV = svStringSet->Request ("option");
  }
    
  csShaderVariable *postEffectOptionSV = postEffects.GetLastLayer()->GetSVContext()->
      GetVariableAdd (idPostEffectOptionSV);

  postEffectOptionSV->SetValue((int)debugBuffer);
}

//----------------------------------------------------------------------
/*void RMDeferred::UpdatePostEffectsSV()
{
  iShaderVarStringSet *svStringSet = shaderManager->GetSVNameStringset ();
  
	postEffectOptionSV.AttachNew (new csShaderVariable (svStringSet->Request ("option")));
	postEffectOptionSV->SetValue ((int)debugBuffer);
  postEffects.GetLastLayer()->GetSVContext()->AddVariable (postEffectOptionSV);  
  
  /*postEffectOptionSV->SetValue ((int)debugBuffer);

  csShaderVariableStack svStack;
  postEffects.GetLayerRenderSVs (postEffects.GetLastLayer(), svStack);
  
  iShader *postEffectShader = shaderManager->GetShader ("effect");
  postEffectShader->PushVariables (svStack);*/
//}

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
void RMDeferred::AddZOnlyLayer(CS::RenderManager::MultipleRenderLayer &layers, int &addedLayer)
{
  const char *messageID = "crystalspace.rendermanager.deferred";

  csRef<iLoader> loader = csQueryRegistry<iLoader> (objRegistry);

  if (!loader->LoadShader ("/shader/early_z/z_only.xml"))
  {
    csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
      messageID, "Could not load z_only shader");
  }

  iShader *shader = shaderManager->GetShader ("z_only");

  SingleRenderLayer baseLayer (shader, 0, 0);
  baseLayer.AddShaderType (stringSet->Request("depthwrite"));

  renderLayer.AddLayers (baseLayer);

  addedLayer = renderLayer.GetLayerCount () - 1;
}

//----------------------------------------------------------------------
int RMDeferred::LocateDeferredLayer(const CS::RenderManager::MultipleRenderLayer &layers)
{
  return LocateLayer (layers, stringSet->Request("gbuffer fill"));
}

//----------------------------------------------------------------------
int RMDeferred::LocateZOnlyLayer(const CS::RenderManager::MultipleRenderLayer &layers)
{
  return LocateLayer (layers, stringSet->Request("depthwrite"));
}

//----------------------------------------------------------------------
int RMDeferred::LocateLayer(const CS::RenderManager::MultipleRenderLayer &layers,
                            csStringID shaderType)
{
  size_t count = renderLayer.GetLayerCount ();
  for (size_t i = 0; i < count; i++)
  {
    size_t num;
    const csStringID *strID = renderLayer.GetShaderTypes (i, num);
    for (size_t j = 0; j < num; j++)
    {
      if (strID[j] == shaderType)
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
  else if (strcmp (cmd, "toggle_visualize_lightvolumes") == 0)
  {
    drawLightVolumes = !drawLightVolumes;
    return true;
  }
  else if (strcmp (cmd, "toggle_visualize_ambient_occlusion") == 0)
  {
    globalIllum.showAmbientOcclusion = !globalIllum.showAmbientOcclusion;
    if (globalIllum.showAmbientOcclusion)
      globalIllum.showGlobalIllumination = false;

    return true;
  }
  else if (strcmp (cmd, "toggle_visualize_global_illumination") == 0)
  {
    globalIllum.showGlobalIllumination = !globalIllum.showGlobalIllumination;
    if (globalIllum.showGlobalIllumination)
      globalIllum.showAmbientOcclusion = false;

    return true;
  }
  /*else if (strcmp (cmd, "toggle_visualize_diffusebuffer") == 0)
  {
    debugBuffer = DiffuseBuffer;
    return true;
  }
  else if (strcmp (cmd, "toggle_visualize_normalbuffer") == 0)
  {
    debugBuffer = NormalBuffer;
    return true;
  }
  else if (strcmp (cmd, "toggle_visualize_ambientbuffer") == 0)
  {
    debugBuffer = AmbientBuffer;
    return true;
  }
  else if (strcmp (cmd, "toggle_visualize_depthbuffer") == 0)
  {
    debugBuffer = DepthBuffer;
    return true;
  }
  else if (strcmp (cmd, "toggle_visualize_specularbuffer") == 0)
  {
    debugBuffer = SpecularBuffer;
    return true;
  }
  else if (strcmp (cmd, "toggle_visualize_colorbuffer") == 0)
  {
    debugBuffer = ColorBuffer;
    return true;
  }*/

  return false;
}

//-----------------------------------------------------------------------------
void RMDeferred::EnableGlobalIllumination (bool enable)
{
  if (enable)
  {
    if (globalIllum.IsEnabled())
      return;

    globalIllum.SetEnabled (true);

    if (globalIllum.IsInitialized())
      return;
  
    csRef<iGraphics3D> graphics3D = csQueryRegistry<iGraphics3D> (objRegistry);
    globalIllum.Initialize (graphics3D, objRegistry, &gbuffer, false);

    if (!directLightBuffer)
      CreateDirectLightBuffer();    
  }
  else
  {
    if (globalIllum.IsEnabled())
      globalIllum.SetEnabled (false);
  }
}

void RMDeferred::ChangeBufferResolution(const char *bufferResolution)
{
  globalIllum.ChangeBufferResolution (bufferResolution);
}

void RMDeferred::EnableBlurPass (bool enableBlur)
{
  globalIllum.applyBlur = enableBlur;
}

csShaderVariable* RMDeferred::GetGlobalIllumVariableAdd(const char *svName)
{
  if (!svName) return nullptr;
  return globalIllum.globalIllumShader->GetVariableAdd (svStringSet->Request (svName));
}

csShaderVariable* RMDeferred::GetBlurVariableAdd(const char *svName)
{
  if (!svName) return nullptr;
  return shaderManager->GetVariableAdd (svStringSet->Request (svName));
}

csShaderVariable* RMDeferred::GetCompositionVariableAdd(const char *svName)
{
  if (!svName) return nullptr;
  return globalIllum.lightCompositionShader->GetVariableAdd (svStringSet->Request (svName));
}

}
CS_PLUGIN_NAMESPACE_END(RMDeferred)
