/*
    Copyright (C) 2007 by Marten Svanfeldt
	      (C) 2008 by Frank Richter

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

#include "cssysdef.h"

#include "iutil/objreg.h"
#include "iutil/plugin.h"

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

#include "frustvis.h"
#include "plugins/engine/3d/light.h"
#include "plugins/engine/3d/meshobj.h"
#include "plugins/engine/3d/meshgen.h"
#include "plugins/engine/3d/sector.h"
#include "plugins/engine/3d/engine.h"

#include "rm_culler.h"

#include "iutil/verbositymanager.h"
#include "ivaria/reporter.h"
#include "csutil/cfgacc.h"

using namespace CS::RenderManager;

CS_PLUGIN_NAMESPACE_BEGIN(RMCuller)
{

SCF_IMPLEMENT_FACTORY(RMCuller)


template<typename RenderTreeType, typename LayerConfigType>
class StandardContextSetup
{
	friend class csEngine;
public:
	typedef StandardContextSetup<RenderTreeType, LayerConfigType> ThisType;
	typedef StandardPortalSetup<RenderTreeType, ThisType> PortalSetupType;

  StandardContextSetup (RMCuller* rmanager, const LayerConfigType& layerConfig)
    : rmanager (rmanager), layerConfig (layerConfig),
    recurseCount (0), maxPortalRecurse (rmanager->maxPortalRecurse)
  {
  }

  StandardContextSetup (const StandardContextSetup& other,
      const LayerConfigType& layerConfig)
    : rmanager (other.rmanager), layerConfig (layerConfig),
      recurseCount (other.recurseCount), maxPortalRecurse(other.maxPortalRecurse)
  {
  }


  void operator() (typename RenderTreeType::ContextNode& context,
    typename PortalSetupType::ContextSetupData& portalSetupData)
  {
    CS::RenderManager::RenderView* rview = context.renderView;
    csSector* sector = (csSector*)rview->GetThisSector ();

    if (recurseCount > maxPortalRecurse) return;
    
    iShaderManager* shaderManager = rmanager->shaderManager;

    // @@@ This is somewhat "boilerplate" sector/rview setup.
    sector->PrepareDraw (rview);
    // Make sure the clip-planes are ok
    CS::RenderViewClipper::SetupClipPlanes (rview->GetRenderContext ());
    
    if (context.owner.IsDebugFlagEnabled (rmanager->dbgFlagClipPlanes))
      context.owner.AddDebugClipPlanes (rview);

    /**
     * Do the culling.
     * If the culler is external, call the generic Viscull.
     * Else call our own culler.
     */
    iVisibilityCuller* culler = sector->GetVisibilityCuller ();
    if (rmanager->visCullers.Contains (culler))
    {
      // Implement awesome culler here.
      csFrustumVis* visCuller = static_cast<csFrustumVis*> (culler);

      const CS::Utility::MeshFilter* filter = &rview->GetMeshFilter();
      CS::RenderManager::Implementation::ViscullCallback<RenderTreeType> cb (context, rview, filter);

      int renderW = 0, renderH = 0;
      context.GetTargetDimensions (renderW, renderH);
      visCuller->VisTest (rview, &cb, renderW, renderH);
    }
    else
    {
      Viscull<RenderTreeType> (context, rview, culler);
    }

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
      StandardSVSetup<RenderTreeType, MultipleRenderLayer> svSetup (
        context.svArrays, layerConfig);

      ForEachMeshNode (context, svSetup);
    }

    SetupStandardShader (context, shaderManager, layerConfig);
    
    RMCuller::LightSetupType::ShadowParamType shadowParam;
    RMCuller::LightSetupType lightSetup (
      rmanager->lightPersistent, rmanager->lightManager,
      context.svArrays, layerConfig, shadowParam);

    ForEachMeshNode (context, lightSetup);

    // Setup shaders and tickets
    SetupStandardTicket (context, shaderManager,
      lightSetup.GetPostLightingLayers());
  
    {
      ThisType ctxRefl (*this,
        rmanager->renderLayerReflect);
      ThisType ctxRefr (*this,
        rmanager->renderLayerRefract);
      RMCuller::AutoReflectRefractType fxRR (
        rmanager->reflectRefractPersistent, ctxRefl, ctxRefr);
        
      RMCuller::AutoFramebufferTexType fxFB (
        rmanager->framebufferTexPersistent);
      
      // Set up a functor that combines the AutoFX functors
      typedef CS::Meta::CompositeFunctorType2<
        RMCuller::AutoReflectRefractType,
        RMCuller::AutoFramebufferTexType> FXFunctor;
      FXFunctor fxf (fxRR, fxFB);
      
      typedef TraverseUsedSVSets<RenderTreeType,
        FXFunctor> SVTraverseType;
      SVTraverseType svTraverser
        (fxf, shaderManager->GetSVNameStringset ()->GetSize (),
	 fxRR.svUserFlags | fxFB.svUserFlags);
      // And do the iteration
      ForEachMeshNode (context, svTraverser);
    }
  }

  // Called by RMCuller::AutoReflectRefractType
  void operator() (typename RenderTreeType::ContextNode& context)
  {
    typename PortalSetupType::ContextSetupData portalData (&context);

    operator() (context, portalData);
  }
private:
  RMCuller* rmanager;
  const LayerConfigType& layerConfig;
  int recurseCount;
  int maxPortalRecurse;
};



RMCuller::RMCuller (iBase* parent)
  : scfImplementationType (this, parent),
  doHDRExposure (false), targets (*this)
{
  SetTreePersistent (treePersistent);
}

bool RMCuller::RenderView (iView* view)
{
  // Setup a rendering view
  view->UpdateClipper ();
  csRef<CS::RenderManager::RenderView> rview;
#include "csutil/custom_new_disable.h"
  rview.AttachNew (new (treePersistent.renderViewPool) 
    CS::RenderManager::RenderView(view));
#include "csutil/custom_new_enable.h"
  iCamera* c = view->GetCamera ();
  iGraphics3D* G3D = rview->GetGraphics3D ();
  int frameWidth = G3D->GetWidth ();
  int frameHeight = G3D->GetHeight ();
  c->SetViewportSize (frameWidth, frameHeight);
  view->GetEngine ()->UpdateNewFrame ();  
  view->GetEngine ()->FireStartFrame (rview);

  float leftx = -c->GetShiftX () * c->GetInvFOV ();
  float rightx = (frameWidth - c->GetShiftX ()) * c->GetInvFOV ();
  float topy = -c->GetShiftY () * c->GetInvFOV ();
  float boty = (frameHeight - c->GetShiftY ()) * c->GetInvFOV ();
  rview->SetFrustum (leftx, rightx, topy, boty);

  contextsScannedForTargets.Empty ();
  portalPersistent.UpdateNewFrame ();
  lightPersistent.UpdateNewFrame ();
  reflectRefractPersistent.UpdateNewFrame ();
  framebufferTexPersistent.UpdateNewFrame ();

  iSector* startSector = rview->GetThisSector ();

  if (!startSector)
    return false;

  CS::Math::Matrix4 perspectiveFixup;
  postEffects.SetupView (view, perspectiveFixup);

  // Pre-setup culling graph
  RenderTreeType renderTree (treePersistent);

  RenderTreeType::ContextNode* startContext = renderTree.CreateContext (rview);
  startContext->renderTargets[rtaColor0].texHandle = postEffects.GetScreenTarget ();
  startContext->perspectiveFixup = perspectiveFixup;

  // Setup the main context
  {
    ContextSetupType contextSetup (this, renderLayer);
    ContextSetupType::PortalSetupType::ContextSetupData portalData (startContext);

    contextSetup (*startContext, portalData);
  
    targets.StartRendering (shaderManager);
    targets.EnqueueTargets (renderTree, shaderManager, renderLayer, contextsScannedForTargets);
  }

  // Setup all dependent targets
  while (targets.HaveMoreTargets ())
  {
    TargetManagerType::TargetSettings ts;
    targets.GetNextTarget (ts);

    HandleTarget (renderTree, ts);
  }

  targets.FinishRendering ();

  // Render all contexts, back to front
  {
    view->GetContext()->SetZMode (CS_ZBUF_MESH);

    SimpleTreeRenderer<RenderTreeType> render (rview->GetGraphics3D (),
      shaderManager);
    ForEachContextReverse (renderTree, render);
  }

  postEffects.DrawPostEffects (renderTree);
  
  if (doHDRExposure) hdrExposure.ApplyExposure (renderTree, view);
  
  DebugFrameRender (rview, renderTree);

  return true;
}

bool RMCuller::PrecacheView(iView* view)
{
  if (!RenderView (view)) return false;

  postEffects.ClearIntermediates();
  hdr.GetHDRPostEffects().ClearIntermediates();

  /* @@@ Other ideas for precache drawing:
    - No frame advancement?
   */

  return true;
}

bool RMCuller::HandleTarget (RenderTreeType& renderTree,
                                 const TargetManagerType::TargetSettings& settings)
{
  // Prepare
  csRef<CS::RenderManager::RenderView> rview;
#include "csutil/custom_new_disable.h"
  rview.AttachNew (new (treePersistent.renderViewPool) 
    CS::RenderManager::RenderView(settings.view));
#include "csutil/custom_new_enable.h"

  iSector* startSector = rview->GetThisSector ();

  if (!startSector)
    return false;

  RenderTreeType::ContextNode* startContext = renderTree.CreateContext (rview);
  startContext->renderTargets[rtaColor0].texHandle = settings.target;
  startContext->renderTargets[rtaColor0].subtexture = settings.targetSubTexture;
  startContext->drawFlags = settings.drawFlags;

  ContextSetupType contextSetup (this, renderLayer);
  ContextSetupType::PortalSetupType::ContextSetupData portalData (startContext);

  contextSetup (*startContext, portalData);
  
  targets.EnqueueTargets (renderTree, shaderManager, renderLayer, contextsScannedForTargets);

  return true;
}


bool RMCuller::Initialize(iObjectRegistry* objectReg)
{
  const char messageID[] = "crystalspace.rendermanager.culler";
  
  this->objectReg = objectReg;

  svNameStringSet = csQueryRegistryTagInterface<iShaderVarStringSet> (objectReg,
    "crystalspace.shader.variablenameset");

  stringSet = csQueryRegistryTagInterface<iStringSet> (objectReg,
    "crystalspace.shared.stringset");
    
  shaderManager = csQueryRegistry<iShaderManager> (objectReg);
  lightManager = csQueryRegistry<iLightManager> (objectReg);
  
  csRef<iVerbosityManager> verbosity = csQueryRegistry<iVerbosityManager> (
    objectReg);
  bool doVerbose = verbosity && verbosity->Enabled ("rendermanager.culler");
  
  csConfigAccess cfg (objectReg);
  bool layersValid = false;
  const char* layersFile = cfg->GetStr ("RenderManager.Culler.Layers", 0);
  if (layersFile)
  {
    if (doVerbose)
      csReport (objectReg, CS_REPORTER_SEVERITY_NOTIFY, messageID,
	"Reading render layers from '%s'", layersFile);
    layersValid = CS::RenderManager::AddLayersFromFile (objectReg, layersFile,
      renderLayer);
    if (!layersValid) renderLayer.Clear();
  }
  layersFile = cfg->GetStr (
    "RenderManager.Culler.Layers.Reflections", 0);
  if (layersFile)
  {
    if (doVerbose)
      csReport (objectReg, CS_REPORTER_SEVERITY_NOTIFY, messageID,
	"Reading reflection render layers from '%s'", layersFile);
    layersValid = CS::RenderManager::AddLayersFromFile (objectReg, layersFile,
      renderLayerReflect);
    if (!layersValid) renderLayerReflect = renderLayer;
  }
  else
    renderLayerReflect = renderLayer;
  layersFile = cfg->GetStr (
    "RenderManager.Culler.Layers.Refractions", 0);
  if (layersFile)
  {
    if (doVerbose)
      csReport (objectReg, CS_REPORTER_SEVERITY_NOTIFY, messageID,
	"Reading refraction render layers from '%s'", layersFile);
    layersValid = CS::RenderManager::AddLayersFromFile (objectReg, layersFile,
      renderLayerRefract);
    if (!layersValid) renderLayerRefract = renderLayer;
  }
  else
    renderLayerRefract = renderLayer;
  
  csRef<iLoader> loader (csQueryRegistry<iLoader> (objectReg));
  if (!layersValid)
  {
    if (!loader->LoadShader ("/shader/lighting/lighting_default.xml"))
    {
      csReport (objectReg, CS_REPORTER_SEVERITY_WARNING,
	messageID, "Could not load lighting_default shader");
    }
  
    if (doVerbose)
      csReport (objectReg, CS_REPORTER_SEVERITY_NOTIFY, messageID,
	"Using default render layers");
    CS::RenderManager::AddDefaultBaseLayers (objectReg, renderLayer);
  }
  
  maxPortalRecurse = cfg->GetInt("RenderManager.Culler.MaxPortalRecurse", 30);
  
  csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (objectReg);
  treePersistent.Initialize (shaderManager);
  dbgFlagClipPlanes =
    treePersistent.debugPersist.RegisterDebugFlag ("draw.clipplanes.view");
  postEffects.Initialize (objectReg);
  
  const char* effectsFile = cfg->GetStr ("RenderManager.Culler.Effects", 0);
  if (effectsFile)
  {
    PostEffectLayersParser postEffectsParser (objectReg);
    postEffectsParser.AddLayersFromFile (effectsFile, postEffects);
  }
  
  HDRSettings hdrSettings (cfg, "RenderManager.Culler");
  if (hdrSettings.IsEnabled())
  {
    doHDRExposure = true;
    
    hdr.Setup (objectReg, 
      hdrSettings.GetQuality(), 
      hdrSettings.GetColorRange());
    postEffects.SetChainedOutput (hdr.GetHDRPostEffects());
  
    hdrExposure.Initialize (objectReg, hdr, hdrSettings);
  }
  
  portalPersistent.Initialize (shaderManager, g3d,
    treePersistent.debugPersist);
  lightPersistent.Initialize (objectReg, treePersistent.debugPersist);
  reflectRefractPersistent.Initialize (objectReg, treePersistent.debugPersist,
    &postEffects);
  framebufferTexPersistent.Initialize (objectReg,
    &postEffects);
  
  return true;
}

iVisibilityCuller* RMCuller::GetVisCuller ()
{
  csFrustumVis* fvCuller = new csFrustumVis();
  fvCuller->Initialize (objectReg);
  visCullers.AddNoTest (fvCuller);
  return fvCuller;
}

}
CS_PLUGIN_NAMESPACE_END(RMCuller)
