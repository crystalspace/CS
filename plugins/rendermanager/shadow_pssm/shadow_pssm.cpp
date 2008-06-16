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

#include "crystalspace.h"

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

#include "shadow_pssm.h"

CS_IMPLEMENT_PLUGIN

using namespace CS::RenderManager;

CS_PLUGIN_NAMESPACE_BEGIN(RMShadowedPSSM)
{

SCF_IMPLEMENT_FACTORY(RMShadowedPSSM)


template<typename RenderTreeType, typename LayerConfigType>
class StandardContextSetup
{
public:
  typedef StandardContextSetup<RenderTreeType, LayerConfigType> ThisType;
  typedef StandardPortalSetup<RenderTreeType, ThisType> PortalSetupType;

  StandardContextSetup (RMShadowedPSSM* rmanager, const LayerConfigType& layerConfig)
    : rmanager (rmanager), layerConfig (layerConfig),
    recurseCount (0)
  {

  }

  void operator() (typename RenderTreeType::ContextNode& context,
    typename PortalSetupType::ContextSetupData& portalSetupData)
  {
    CS::RenderManager::RenderView* rview = context.renderView;
    iSector* sector = rview->GetThisSector ();

    // @@@ FIXME: Of course, don't hardcode.
    if (recurseCount > 30) return;
    
    RMShadowedPSSM::ShadowType::ViewSetup shadowViewSetup (
      rmanager->lightPersistent.shadowPersist, rview);
    
    iShaderManager* shaderManager = rmanager->shaderManager;

    // @@@ This is somewhat "boilerplate" sector/rview setup.
    rview->SetThisSector (sector);
    sector->CallSectorCallbacks (rview);
    // Make sure the clip-planes are ok
    CS::RenderViewClipper::SetupClipPlanes (rview->GetRenderContext ());

    // Do the culling
    iVisibilityCuller* culler = sector->GetVisibilityCuller ();
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
      StandardSVSetup<RenderTreeType, MultipleRenderLayer> svSetup (
        context.svArrays, layerConfig);

      ForEachMeshNode (context, svSetup);
    }

    SetupStandardShader (context, shaderManager, layerConfig);

    RMShadowedPSSM::LightSetupType lightSetup (
      rmanager->lightPersistent, rmanager->lightManager,
      context.svArrays, layerConfig, shadowViewSetup);

    ForEachMeshNode (context, lightSetup);
    shadowViewSetup.PostLightSetup (context, layerConfig);

    // Setup shaders and tickets
    SetupStandardTicket (context, shaderManager,
      lightSetup.GetPostLightingLayers());
  }


private:
  RMShadowedPSSM* rmanager;
  const LayerConfigType& layerConfig;
  int recurseCount;
};



RMShadowedPSSM::RMShadowedPSSM (iBase* parent)
  : scfImplementationType (this, parent), doHDRExposure (false), targets (*this),
    wantDebugLockLines (false), lockedDebugLines (0)
{
}

bool RMShadowedPSSM::RenderView (iView* view)
{
  // Setup a rendering view
  view->UpdateClipper ();
  csRef<CS::RenderManager::RenderView> rview;
#include "csutil/custom_new_disable.h"
  rview.AttachNew (new (treePersistent.renderViewPool) 
    CS::RenderManager::RenderView(view));
#include "csutil/custom_new_enable.h"
  view->GetCamera()->SetViewportSize (rview->GetGraphics3D()->GetWidth(),
    rview->GetGraphics3D()->GetHeight());
  view->GetEngine ()->UpdateNewFrame ();  
  view->GetEngine ()->FireStartFrame (rview);

  contextsScannedForTargets.Empty ();
  portalPersistent.UpdateNewFrame ();
  lightPersistent.UpdateNewFrame ();

  iSector* startSector = rview->GetThisSector ();

  if (!startSector)
    return false;

  postEffects.SetupView (view);

  // Pre-setup culling graph
  RenderTreeType renderTree (treePersistent);

  RenderTreeType::ContextNode* startContext = renderTree.CreateContext (rview);
  startContext->renderTargets[rtaColor0].texHandle = postEffects.GetScreenTarget ();

  // Setup the main context
  {
    ContextSetupType contextSetup (this, renderLayer);
    ContextSetupType::PortalSetupType::ContextSetupData portalData (startContext);

    contextSetup (*startContext, portalData);
  
    targets.PrepareQueues (shaderManager);
    targets.EnqueueTargets (renderTree, shaderManager, renderLayer, contextsScannedForTargets);  
  }

  // Setup all dependent targets
  while (targets.HaveMoreTargets ())
  {
    TargetManagerType::TargetSettings ts;
    targets.GetNextTarget (ts);

    HandleTarget (renderTree, ts);
  }


  targets.PostCleanupQueues ();
  // Render all contexts, back to front
  {
    view->GetContext()->SetZMode (CS_ZBUF_MESH);

    SimpleTreeRenderer<RenderTreeType> render (rview->GetGraphics3D (),
      shaderManager);
    ForEachContextReverse (renderTree, render);
  }

  postEffects.DrawPostEffects ();
  
  
  if (doHDRExposure) hdrExposure.ApplyExposure (postEffects);
  
  if (wantDebugLockLines)
  {
    lockedDebugLines =
      new RenderTreeType::DebugLines (renderTree.GetDebugLines());
    wantDebugLockLines = false;
  }
  else if (lockedDebugLines)
    renderTree.SetDebugLines (*lockedDebugLines);
  renderTree.DrawDebugLines (rview->GetGraphics3D (), rview);
  //renderTree.RenderDebugTextures (rview->GetGraphics3D ());

  return true;
}


bool RMShadowedPSSM::HandleTarget (RenderTreeType& renderTree,
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

  ContextSetupType contextSetup (this, renderLayer);
  ContextSetupType::PortalSetupType::ContextSetupData portalData (startContext);

  contextSetup (*startContext, portalData);
  
  targets.EnqueueTargets (renderTree, shaderManager, renderLayer, contextsScannedForTargets);

  return true;
}

bool RMShadowedPSSM::DebugCommand (const char *cmd)
{
  if (strcmp (cmd, "toggle_debug_lines_lock") == 0)
  {
    csPrintf ("%p got toggle_debug_lines_lock: ", this);
    if (lockedDebugLines)
    {
      delete lockedDebugLines; lockedDebugLines = 0;
      csPrintf ("unlocked\n");
    }
    else
    {
      wantDebugLockLines = !wantDebugLockLines;
      csPrintf ("%slocked\n", wantDebugLockLines ? "" : "un");
    }
    return true;
  }
  return false;
}


bool RMShadowedPSSM::Initialize(iObjectRegistry* objectReg)
{
  const char messageID[] = "crystalspace.rendermanager.shadow_pssm";
  
  this->objectReg = objectReg;

  svNameStringSet = csQueryRegistryTagInterface<iShaderVarStringSet> (objectReg,
    "crystalspace.shader.variablenameset");

  stringSet = csQueryRegistryTagInterface<iStringSet> (objectReg,
    "crystalspace.shared.stringset");
    
  shaderManager = csQueryRegistry<iShaderManager> (objectReg);
  lightManager = csQueryRegistry<iLightManager> (objectReg);
  
  csRef<iVerbosityManager> verbosity = csQueryRegistry<iVerbosityManager> (
    objectReg);
  bool doVerbose = verbosity && verbosity->Enabled ("rendermanager.shadow_pssm");
  
  csConfigAccess cfg (objectReg);
  bool layersValid = false;
  const char* layersFile = cfg->GetStr ("RenderManager.ShadowPSSM.Layers", 0);
  if (layersFile)
  {
    if (doVerbose)
      csReport (objectReg, CS_REPORTER_SEVERITY_NOTIFY, messageID,
	"Reading render layers from '%s'", layersFile);
    layersValid = CS::RenderManager::AddLayersFromFile (objectReg, layersFile,
      renderLayer);
    if (!layersValid) renderLayer.Clear();
  }
  csRef<iLoader> loader (csQueryRegistry<iLoader> (objectReg));
  if (!layersValid)
  {
    if (!loader->LoadShader ("/shader/lighting/lighting_default.xml"))
    {
      csReport (objectReg, CS_REPORTER_SEVERITY_WARNING,
	"crystalspace.rendermanager.test1",
	"Could not load lighting_default shader");
    }
  
    if (doVerbose)
      csReport (objectReg, CS_REPORTER_SEVERITY_NOTIFY, messageID,
	"Using default render layers");
    CS::RenderManager::AddDefaultBaseLayers (objectReg, renderLayer);
  }
  
  
  csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (objectReg);
  treePersistent.Initialize (shaderManager);
  postEffects.Initialize (objectReg);
  
  const char* effectsFile = cfg->GetStr ("RenderManager.ShadowPSSM.Effects", 0);
  if (effectsFile)
  {
    PostEffectLayersParser postEffectsParser (objectReg);
    postEffectsParser.AddLayersFromFile (effectsFile, postEffects);
  }
  
  HDRSettings hdrSettings (cfg, "RenderManager.ShadowPSSM");
  if (hdrSettings.IsEnabled())
  {
    doHDRExposure = true;
    
    HDRHelper hdr;
    hdr.Setup (objectReg, 
      hdrSettings.GetQuality(), 
      hdrSettings.GetColorRange(), 
      postEffects, !doHDRExposure);
  
    // @@@ Make configurable, too
    hdrExposure.Initialize (objectReg, postEffects);
  }
  
  portalPersistent.Initialize (shaderManager, g3d);
  lightPersistent.shadowPersist.SetConfigPrefix ("RenderManager.ShadowPSSM");
  lightPersistent.Initialize (objectReg);
  
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(RMShadowedPSSM)
