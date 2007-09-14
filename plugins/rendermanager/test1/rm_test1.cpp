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

  // Pre-setup culling graph
  RenderTreeType renderTree (treePersistent);
  RenderTreeType::ContextsContainer* screenContexts = 
    renderTree.CreateContextContainer ();
  screenContexts->view = view;
  RenderTreeType::ContextNode* startContext = renderTree.CreateContext (screenContexts,
    rview);

  // Setup the main context
  {
    StandardContextSetup<RenderTreeType> contextSetup (shaderManager, 
      defaultShader, defaultShaderName);
    contextSetup (renderTree, startContext, screenContexts, startSector, rview);
  
    targets.AddDependentTargetsToTree (screenContexts, renderTree, 
      contextSetup, shaderManager);
  }

  // Render all contexts, back to front
  {
    view->GetContext()->SetZMode (CS_ZBUF_MESH);

    ContextRender<RenderTreeType> render (shaderManager);
    renderTree.TraverseContextContainersReverse (render);
    //renderTree.TraverseMeshNodes (render, mainContext);
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
