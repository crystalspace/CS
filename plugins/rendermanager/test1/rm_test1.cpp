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
  rview.AttachNew (new(renderViewPool) CS::RenderManager::RenderView(view));

  view->GetEngine ()->UpdateNewFrame ();

  iSector* startSector = rview->GetThisSector ();

  // Pre-setup culling graph
  RenderTreeType renderTree (treePersistent);
  RenderTreeType::ContextNode* mainContext = renderTree.PrepareViscull (rview);

  // Do the culling
  iVisibilityCuller* culler = startSector->GetVisibilityCuller ();
  renderTree.Viscull (mainContext, rview, culler);


  // Sort the mesh lists  
  {
    StandardMeshSorter<RenderTreeType> mySorter (view->GetEngine (), view->GetCamera ());
    renderTree.TraverseMeshNodes (mySorter, mainContext);
  }

  // Setup the SV arrays
  // Push the default stuff
  SetupStandardSVs<RenderTreeType> (*mainContext, shaderManager, startSector);

  // Setup the material&mesh SVs
  {
    StandardSVSetup<RenderTreeType> svSetup (mainContext->svArrays);
    renderTree.TraverseMeshNodes (svSetup, mainContext);
  }

  // Render all meshes, using only default shader
  {
    SetupStandarShaderAndTicket (renderTree, *mainContext, shaderManager, defaultShaderName, defaultShader);

    // Render
    {
      iGraphics3D* g3d = view->GetContext ();

      BeginFinishDrawScope bd (g3d, view->GetEngine ()->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS);

      g3d->SetWorldToCamera (view->GetCamera ()->GetTransform ().GetInverse ());

      SimpleRender<RenderTreeType> render (g3d, shaderManager->GetShaderVariableStack ());

      renderTree.TraverseMeshNodes (render, mainContext);
    }
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
  return true;
}
