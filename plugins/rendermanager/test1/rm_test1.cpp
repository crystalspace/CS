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

using namespace CS::Plugin::RMTest1;
using namespace CS::RenderManager;

SCF_IMPLEMENT_FACTORY(RMTest1)

template<typename RenderTreeType, typename LayerConfigType>
class StandardContextSetup
{
public:
  typedef StandardContextSetup<RenderTreeType, LayerConfigType> ThisType;

  StandardContextSetup (iShaderManager* shaderManager, const LayerConfigType& layerConfig)
    : shaderManager (shaderManager), 
    layerConfig (layerConfig),
    recurseCount (0)
  {

  }

  void operator() (RenderTreeType& renderTree, 
    typename RenderTreeType::ContextNode* context, 
    typename RenderTreeType::ContextsContainer* container, 
    iSector* sector, CS::RenderManager::RenderView* rview)
  {
    // @@@ FIXME: Of course, don't hardcode.
    if (recurseCount > 5) return;

    sector->CallSectorCallbacks (rview);

    // Make sure the clip-planes are ok
    CS::RenderViewClipper::SetupClipPlanes (rview->GetRenderContext ());

    // Do the culling
    iVisibilityCuller* culler = sector->GetVisibilityCuller ();
    renderTree.Viscull (container, context, rview, culler);

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
      StandardSVSetup<RenderTreeType, SingleRenderLayer> svSetup (context->svArrays, layerConfig);
      renderTree.TraverseMeshNodes (svSetup, context);
    }

    // Setup shaders and tickets
    SetupStandarShaderAndTicket (renderTree, *context, shaderManager, 
      layerConfig);

    {
      recurseCount++;
      StandardPortalSetup<RenderTreeType, ThisType> portalSetup (*this);
      portalSetup (renderTree, context, container, sector, rview);
      recurseCount--;
    }

  }


private:


  iShaderManager* shaderManager;
  const LayerConfigType& layerConfig;
  int recurseCount;
};



RMTest1::RMTest1 (iBase* parent)
: scfImplementationType (this, parent)
{

}

bool RMTest1::RenderView (iView* view)
{
  // Setup a rendering view
  csRef<CS::RenderManager::RenderView> rview;
  rview.AttachNew (new (renderViewPool) CS::RenderManager::RenderView(view));
  view->GetEngine ()->UpdateNewFrame ();

  view->UpdateClipper ();

  iSector* startSector = rview->GetThisSector ();

  if (!startSector)
    return false;

  SingleRenderLayer renderLayer (defaultShaderName, defaultShader);

  // Pre-setup culling graph
  RenderTreeType renderTree (treePersistent);
  RenderTreeType::ContextsContainer* screenContexts = 
    renderTree.CreateContextContainer ();
  screenContexts->view = view;
  RenderTreeType::ContextNode* startContext = renderTree.CreateContext (screenContexts,
    rview);

  // Setup the main context
  {
    StandardContextSetup<RenderTreeType, SingleRenderLayer> contextSetup (shaderManager, 
      renderLayer);
    contextSetup (renderTree, startContext, screenContexts, startSector, rview);
  
    //targets.AddDependentTargetsToTree (screenContexts, renderTree, 
    //  contextSetup, shaderManager);
  }

  // Render all contexts, back to front
  {
    view->GetContext()->SetZMode (CS_ZBUF_MESH);

    ContextRender<RenderTreeType, SingleRenderLayer> render (shaderManager, renderLayer);
    renderTree.TraverseContextContainersReverse (render);
  }

  return true;
}

bool RMTest1::Initialize(iObjectRegistry* objectReg)
{
  svNameStringSet = csQueryRegistryTagInterface<iStringSet> (objectReg,
    "crystalspace.shader.variablenameset");

  stringSet = csQueryRegistryTagInterface<iStringSet> (objectReg,
    "crystalspace.shared.stringset");

  shaderManager = csQueryRegistry<iShaderManager> (objectReg);

  defaultShaderName = stringSet->Request("standard");  
  defaultShader = shaderManager->GetShader ("std_lighting");

  treePersistent.Initialize (shaderManager);

  return true;
}
