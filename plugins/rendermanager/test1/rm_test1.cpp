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

  view->GetEngine ()->IncrementCurrentFrameNumber ();

  iSector* startSector = rview->GetThisSector ();

  // Pre-setup culling graph
  RenderTreeType renderTree (treePersistent);
  renderTree.PrepareViscull (rview);

  // Do the culling
  iVisibilityCuller* culler = startSector->GetVisibilityCuller ();
  renderTree.Viscull (rview, culler);

  // Finalize the tree
  renderTree.FinishViscull ();


  // Sort the mesh lists  
  {
    StandardMeshSorter<RenderTreeType> mySorter (view->GetEngine (), view->GetCamera ());
    renderTree.TraverseMeshNodes (mySorter);
  }

  // Setup the SV arrays
  SVArrayHolder svArrays (svNameStringSet->GetSize (), renderTree.GetTotalRenderMeshes ());

  // Push the default stuff
  csShaderVariableStack& svStack = shaderManager->GetShaderVariableStack ();
  svArrays.SetupSVStck (svStack, 0);

  shaderManager->PushVariables (svStack);
  startSector->GetSVContext ()->PushVariables (svStack);

  // Replicate
  svArrays.ReplicateSet (0, 1);
  
  // Setup the material&mesh SVs
  {
    StandardSVSetup<RenderTreeType> svSetup (svArrays);
    renderTree.TraverseMeshNodes (svSetup);
  }

  // Render all meshes, using only default shader
  {
    csArray<iShader*> shaderArray; shaderArray.SetSize (renderTree.GetTotalRenderMeshes ());
    csArray<size_t> ticketArray; ticketArray.SetSize (renderTree.GetTotalRenderMeshes ());
    // Shader setup
    {
      ShaderSetup<RenderTreeType> shaderSetup (shaderArray, defaultShaderName, defaultShader);

      renderTree.TraverseMeshNodes (shaderSetup);
    }

    // Ticket setup
    {
      TicketSetup<RenderTreeType> ticketSetup (svArrays, shaderManager->GetShaderVariableStack (),
        shaderArray, ticketArray);

      renderTree.TraverseMeshNodes (ticketSetup);
    }

    // Render
    {
      iGraphics3D* g3d = view->GetContext ();

      BeginFinishDrawScope bd (g3d, view->GetEngine ()->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS);

      g3d->SetWorldToCamera (view->GetCamera ()->GetTransform ().GetInverse ());

      SimpleRender<RenderTreeType> render (g3d, svArrays, 
        shaderManager->GetShaderVariableStack (), shaderArray, ticketArray);

      renderTree.TraverseMeshNodes (render);
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