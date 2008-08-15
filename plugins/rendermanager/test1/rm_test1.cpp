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
#include "csplugincommon/rendermanager/lightsetup.h"
#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/portalsetup.h"
#include "csplugincommon/rendermanager/posteffects.h"
#include "csplugincommon/rendermanager/render.h"
#include "csplugincommon/rendermanager/renderlayers.h"
#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/shadersetup.h"
#include "csplugincommon/rendermanager/shadow_shadowmap.h"
#include "csplugincommon/rendermanager/standardsorter.h"
#include "csplugincommon/rendermanager/svsetup.h"
#include "csplugincommon/rendermanager/viscull.h"

#include "rm_test1.h"

CS_IMPLEMENT_PLUGIN

using namespace CS::RenderManager;

CS_PLUGIN_NAMESPACE_BEGIN(RMTest1)
{

SCF_IMPLEMENT_FACTORY(RMTest1)


template<typename RenderTreeType, typename LayerConfigType>
class StandardContextSetup
{
public:
  typedef StandardContextSetup<RenderTreeType, LayerConfigType> ThisType;
  typedef StandardPortalSetup<RenderTreeType, ThisType> PortalSetupType;

  StandardContextSetup (RMTest1* rmanager, const LayerConfigType& layerConfig)
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

    RMTest1::LightSetupType lightSetup (
      rmanager->lightPersistent, rmanager->lightManager,
      context.svArrays, layerConfig);

    ForEachMeshNode (context, lightSetup);

    // Setup shaders and tickets
    SetupStandardTicket (context, shaderManager,
      lightSetup.GetPostLightingLayers());
  }


private:
  RMTest1* rmanager;
  const LayerConfigType& layerConfig;
  int recurseCount;
};



RMTest1::RMTest1 (iBase* parent)
  : scfImplementationType (this, parent), targets (*this)
{

}

bool RMTest1::RenderView (iView* view)
{
  // Setup a rendering view
  view->UpdateClipper ();
  csRef<CS::RenderManager::RenderView> rview;
#include "csutil/custom_new_disable.h"
  rview.AttachNew (new (treePersistent.renderViewPool) 
    CS::RenderManager::RenderView(view));
#include "csutil/custom_new_enable.h"
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

  return true;
}


bool RMTest1::HandleTarget (RenderTreeType& renderTree,
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


bool RMTest1::Initialize(iObjectRegistry* objectReg)
{
  svNameStringSet = csQueryRegistryTagInterface<iShaderVarStringSet> (objectReg,
    "crystalspace.shader.variablenameset");

  stringSet = csQueryRegistryTagInterface<iStringSet> (objectReg,
    "crystalspace.shared.stringset");
    
  shaderManager = csQueryRegistry<iShaderManager> (objectReg);
  
  lightManager = csQueryRegistry<iLightManager> (objectReg);
  
  /*CS::RenderManager::SingleRenderLayer renderLayer = 
    CS::RenderManager::SingleRenderLayer (stringSet->Request("standard"),
      shaderManager->GetShader ("std_lighting"));
  this->renderLayer.AddLayers (renderLayer);*/
  
  //CS::RenderManager::AddDefaultBaseLayers (objectReg, renderLayer);

  csRef<iLoader> loader (csQueryRegistry<iLoader> (objectReg));
  if (!loader->LoadShader ("/shader/lighting/lighting_default.xml"))
  {
    csReport (objectReg, CS_REPORTER_SEVERITY_WARNING,
      "crystalspace.rendermanager.test1",
      "Could not load lighting_vproc_fixed shader");
  }

  CS::RenderManager::SingleRenderLayer lightLayer = 
    CS::RenderManager::SingleRenderLayer (stringSet->Request("standard"),
      shaderManager->GetShader ("lighting_default"));
  this->renderLayer.AddLayers (lightLayer);

/*
  CS::RenderManager::SingleRenderLayer ambientLayer = 
    CS::RenderManager::SingleRenderLayer (stringSet->Request("ambient"));
  this->renderLayer.AddLayers (ambientLayer);
  CS::RenderManager::SingleRenderLayer diffuseLayer = 
    CS::RenderManager::SingleRenderLayer (stringSet->Request("diffuse"));
  this->renderLayer.AddLayers (diffuseLayer);
*/
  
  csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (objectReg);
  treePersistent.Initialize (shaderManager);
  postEffects.Initialize (objectReg);
  
  portalPersistent.Initialize (shaderManager, g3d);
  lightPersistent.Initialize (shaderManager);

  /*csRef<iLoader> loader = csQueryRegistry<iLoader> (objectReg);  
  csRef<iShader> desatShader = loader->LoadShader ("/shader/desaturate.xml");
  postEffects.AddLayer (desatShader);*/

  return true;
}

}
CS_PLUGIN_NAMESPACE_END(RMTest1)
