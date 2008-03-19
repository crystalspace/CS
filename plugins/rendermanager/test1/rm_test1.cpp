/*
    Copyright (C) 2007 by Marten Svanfeldt

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation; 

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
  
  

  // Setup SV arrays
/*  size_t numSVs = svNameStringSet->GetSize ();
  size_t numRMs = renderTree.GetTotalRenderMeshes ();

  // Allocate the temporary storage for all render meshes
  csShaderVariable **shaderVarStacks = new csShaderVariable* [numSVs * numRMs];
  memset(shaderVarStacks, 0, sizeof (csShaderVariable*)*numSVs);
  size_t rmOffset = 0;

  // Shader and tickets list
  iShader** shaderList = new iShader*[numRMs];
  size_t* shaderTicket = new size_t[numRMs];

  // Setup the shadermanager + sector SVs
  shaderManager->PushVariables (shaderVarStacks);
  startSector->GetSVContext ()->PushVariables (shaderVarStacks);

  // Replicate to all meshes
  for (size_t i = 1; i < numRMs; ++i)
  {
    memcpy (shaderVarStacks+numSVs*i, shaderVarStacks, sizeof(csShaderVariable*)*numSVs);
  }

  struct SVSetup
  {
    SVSetup (csShaderVariable** svStacks, iShader** shaderList, size_t* shaderTickets,
      csStringID shaderType, size_t& rmOffset, size_t numSVs)
      : svStacks (svStacks), shaderList (shaderList), shaderTickets (shaderTickets), 
      shaderType(shaderType), rmOffset (rmOffset), numSVs (numSVs)
    {
    }

    void operator() (const RenderTreeType::MeshNode* mn, const RenderTreeType::ContextNode& ctx, 
      const RenderTreeType& tree)
    {
      iMeshWrapper* mw = mn->meshWrapper;
      
      for (size_t i = 0; i < mn->renderMeshes.GetSize (); ++i)
      {
        csRenderMesh* rm = mn->renderMeshes[i];
        csShaderVariable** localStack = svStacks + rmOffset*numSVs;

        shaderList[rmOffset] = rm->material->GetMaterial ()->GetShader (shaderType);
        
        shaderList[rmOffset]->PushVariables (localStack);
        rm->material->GetMaterial ()->PushVariables (localStack);
        rm->variablecontext->PushVariables (localStack);
        mw->GetSVContext ()->PushVariables (localStack);

        // Setup shader tickets
        shaderTickets[rmOffset] = 0; //shaderList[rmOffset]->GetTicket (*rm, localStack);
        

        rmOffset++;
      }  
    }

    csShaderVariable** svStacks;
    iShader** shaderList;
    size_t* shaderTickets;
    csStringID shaderType;

    size_t& rmOffset;
    size_t numSVs;
  };
  renderTree.TraverseMeshNodes (SVSetup(shaderVarStacks, shaderList, shaderTicket, defaultShaderName, 
    rmOffset, numSVs));

  // Render to screen

  // Clean up
  delete shaderVarStacks;
  delete shaderList;
  delete shaderTicket;
*/
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

  return true;
}