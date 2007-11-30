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

#include "csutil/compositefunctor.h"

#include "rm_test1.h"

CS_IMPLEMENT_PLUGIN

//using namespace CS::Plugin::RMTest1;
using namespace CS::RenderManager;

CS_PLUGIN_NAMESPACE_BEGIN(RMTest1)
{

SCF_IMPLEMENT_FACTORY(RMTest1)

template<typename RenderTreeType, typename LayerConfigType>
class StandardContextSetup
{
public:
  typedef StandardContextSetup<RenderTreeType, LayerConfigType> ThisType;

  StandardContextSetup (RMTest1* rmanager, const LayerConfigType& layerConfig)
    : rmanager (rmanager), layerConfig (layerConfig),
    recurseCount (0)
  {

  }

  void operator() (RenderTreeType& renderTree, 
    typename RenderTreeType::ContextNode* context, 
    typename RenderTreeType::ContextsContainer* container, 
    iSector* sector, CS::RenderManager::RenderView* rview)
  {
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
    renderTree.Viscull (container, context, rview, culler);

    // Set up all portals
    {
      recurseCount++;
      StandardPortalSetup<RenderTreeType, ThisType> portalSetup (
        rmanager->portalPersistent, *this);
      portalSetup (renderTree, context, container, sector, rview);
      recurseCount--;
    }
    
    // Sort the mesh lists  
    {
      StandardMeshSorter<RenderTreeType> mySorter (rview->GetEngine (), rview->GetCamera ());
      renderTree.TraverseMeshNodes (mySorter, context);
    }

    // Setup the SV arrays
    // Push the default stuff
    SetupStandardSVs<RenderTreeType> (layerConfig, *context, shaderManager, sector);

    // Setup the material&mesh SVs
    {
      StandardSVSetup<RenderTreeType, MultipleRenderLayer> svSetup (
        context->svArrays, layerConfig);
      renderTree.TraverseMeshNodes (svSetup, context);
    }

    // Setup shaders and tickets
    SetupStandarShaderAndTicket (renderTree, *context, shaderManager, 
      layerConfig);
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
  rview.AttachNew (new (treePersistent.renderViewPool) 
    CS::RenderManager::RenderView(view));
  view->GetEngine ()->UpdateNewFrame ();
  portalPersistent.UpdateNewFrame ();
  view->GetEngine ()->FireStartFrame (rview);

  iSector* startSector = rview->GetThisSector ();

  if (!startSector)
    return false;

  postEffects.SetupView (view);

  // Pre-setup culling graph
  RenderTreeType renderTree (treePersistent);
  RenderTreeType::ContextsContainer* screenContexts = 
    renderTree.CreateContextContainer ();
  screenContexts->rview = rview;
  screenContexts->renderTarget = postEffects.GetScreenTarget ();

  RenderTreeType::ContextNode* startContext = renderTree.CreateContext (screenContexts,
    rview);

  // Setup the main context
  {
    //StandardContextSetup<RenderTreeType, SingleRenderLayer>
    ContextSetupType contextSetup (this, 
      renderLayer);
    contextSetup (renderTree, startContext, screenContexts, startSector, rview);
  
    targets.PrepareQueues (shaderManager);
    targets.EnqueueTargetsInContext (screenContexts, renderTree, shaderManager, renderLayer);  
  }

  // Setup all dependent targets
  while (targets.HaveMoreTargets ())
  {
    csStringID svName;
    RenderTreeType::ContextsContainer* contexts;
    targets.GetNextTarget (svName, contexts);

    HandleTarget (renderTree, svName, contexts);    
  }


  targets.PostCleanupQueues ();
  // Render all contexts, back to front
  {
    view->GetContext()->SetZMode (CS_ZBUF_MESH);

    ContextRender<RenderTreeType, MultipleRenderLayer> render (shaderManager, renderLayer);
    renderTree.TraverseContextContainersReverse (render);
  }

  postEffects.DrawPostEffects ();

  return true;
}

bool RMTest1::HandleTarget (RenderTreeType& renderTree, csStringID svName, 
                            RenderTreeType::ContextsContainer* contexts)
{
  // Prepare
  csRef<CS::RenderManager::RenderView> rview = contexts->rview;

  iSector* startSector = rview->GetThisSector ();

  if (!startSector)
    return false;

  RenderTreeType::ContextNode* startContext = renderTree.CreateContext (contexts,
    rview);

  // Setup
  StandardContextSetup<RenderTreeType, MultipleRenderLayer> contextSetup (this,
    renderLayer);
  contextSetup (renderTree, startContext, contexts, startSector, rview);

  targets.EnqueueTargetsInContext (contexts, renderTree, shaderManager, renderLayer);

  return true;
}

bool RMTest1::Initialize(iObjectRegistry* objectReg)
{
  svNameStringSet = csQueryRegistryTagInterface<iStringSet> (objectReg,
    "crystalspace.shader.variablenameset");

  stringSet = csQueryRegistryTagInterface<iStringSet> (objectReg,
    "crystalspace.shared.stringset");

  shaderManager = csQueryRegistry<iShaderManager> (objectReg);
  

  /*CS::RenderManager::SingleRenderLayer renderLayer = 
    CS::RenderManager::SingleRenderLayer (stringSet->Request("standard"),
      shaderManager->GetShader ("std_lighting"));
  this->renderLayer.AddLayers (renderLayer);*/
  CS::RenderManager::AddDefaultBaseLayers (objectReg, renderLayer);

  csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (objectReg);
  treePersistent.Initialize (shaderManager);
  postEffects.Initialize (objectReg);
  portalPersistent.Initialize (shaderManager, g3d);

  csRef<iLoader> loader = csQueryRegistry<iLoader> (objectReg);
  
  /*csRef<iShader> desatShader = loader->LoadShader ("/shader/desaturate.xml");
  postEffects.AddLayer (desatShader);*/

  return true;
}

}
CS_PLUGIN_NAMESPACE_END(RMTest1)
