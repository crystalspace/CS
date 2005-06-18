/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#include "iengine/camera.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/rview.h"
#include "iutil/objreg.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/rendermesh.h"
#include "ivideo/shader/shader.h"

#include "meshnode.h"

csMeshRenderNode::csMeshRenderNode (csMeshRenderNodeFactory* factory, 
                                    csStringID shaderType,
                                    iShader* defShader, 
				    csShaderVariableContext& shadervars) : 
  shadervars(shadervars), factory(factory), shaderType(shaderType), 
  defShader(defShader)
{
  shadervars.GetVariableAdd (factory->string_object2world);
}

void csMeshRenderNode::RenderMeshes (iGraphics3D* g3d,
                                     iShader* shader, size_t ticket,
                                     csRenderMesh** meshes, size_t num,
                                     const csShaderVarStack* Stacks)
{
  if (num == 0) return;
  //ToggleStepSettings (g3d, true);
  csRef<csShaderVariable> svO2W = 
    shadervars.GetVariable (factory->string_object2world);

  size_t numPasses = shader->GetNumberOfPasses (ticket);
  for (size_t p = 0; p < numPasses; p++)
  {
    shader->ActivatePass (ticket, p);

    size_t j;
    for (j = 0; j < num; j++)
    {
      csRenderMesh* mesh = meshes[j];
      svO2W->SetValue (mesh->object2world);

      const csShaderVarStack& stacks = Stacks[j];
      csRenderMeshModes modes (*mesh);
      shader->SetupPass (ticket, mesh, modes, stacks);
      g3d->DrawMesh (mesh, modes, stacks);
      shader->TeardownPass (ticket);
    }
    shader->DeactivatePass (ticket);
  }
}

void csMeshRenderNode::FillStacks (csShaderVarStack& stacks, 
                                   csRenderMesh* rm, iMeshWrapper* mw, 
                                   iMaterial* hdl, iShader* shader)
{
  iShaderVariableContext* svc = mw->GetSVContext();
  if (svc->IsEmpty())
    svc = 0;

  stacks.Empty();
  factory->shaderManager->PushVariables (stacks);
  shadervars.PushVariables (stacks);
  if (rm->variablecontext)
    rm->variablecontext->PushVariables (stacks);
  if (svc)
    svc->PushVariables (stacks);
  shader->PushVariables (stacks);
  hdl->PushVariables (stacks);
}

size_t csMeshRenderNode::GetTicket (const csShaderVarStack& stacks, csRenderMesh* rm,
                                    iShader* shader)
{
  csRenderMeshModes modes (*rm);
  return shader->GetTicket (modes, stacks);
}

void csMeshRenderNode::AddMesh (csRenderMesh* rm, iMeshWrapper* mw, 
                                long prio, bool keepOrder)
{
  csShaderVarStack stacks;

  iShader* shader;
  size_t ticket;

  iMaterial* hdl = rm->material->GetMaterial ();
  shader = hdl->GetShader (shaderType);
  if (shader == 0) shader = defShader;
  if ((shader == 0) || (shader == factory->nullShader))
  {
    return; // @@@ Perhaps an Assert? Or some Error?
  }
  FillStacks (stacks, rm, mw, hdl, shader);

  if (keepOrder)
  {
    ticket = (size_t)~0;
  }
  else
  {
    ticket = GetTicket (stacks, rm, shader);
  }

  ShaderTicketKey key;
  key.prio = prio;
  key.shader = shader;
  key.ticket = ticket;

  MeshBucket* bucket = buckets.GetElementPointer (key);
  if (bucket == 0)
  {
    bucket = buckets.Put (key, MeshBucket());
  }
  bucket->rendermeshes.Push (rm);
  bucket->stacks.Push (stacks);
}

bool csMeshRenderNode::Preprocess (iRenderView* rview)
{
  iGraphics3D* g3d = rview->GetGraphics3D();
  const csReversibleTransform& camt = rview->GetCamera ()->GetTransform ();
  g3d->SetWorldToCamera (camt.GetInverse ());
  g3d->SetZMode (CS_ZBUF_MESH);
  TraverseShaderBuckets traverser (*this, g3d);
  buckets.TraverseInOrder (traverser);

  return true;
}

//---------------------------------------------------------------------------

void csMeshRenderNode::TraverseShaderBuckets::Process (
  const ShaderTicketKey& key, csMeshRenderNode::MeshBucket &bucket)
{
  size_t startMesh = 0;
  iShader* lastShader = 0;
  size_t lastTicket = (size_t)~0;
  iShader* meshShader = key.shader;
  for (size_t i = 0; i < bucket.rendermeshes.Length(); i++)
  {
    size_t newTicket = (key.ticket != (size_t)~0) ? key.ticket :
      node.GetTicket (*(bucket.stacks.GetArray() + i), 
        bucket.rendermeshes[i], meshShader);
    if ((meshShader != lastShader) || (newTicket != lastTicket))
    {
      // @@@ Need error reporter
      if ((lastShader != 0) && (lastShader != node.factory->nullShader))
      {
        //g3d->SetWorldToCamera (camt);
        node.RenderMeshes (g3d, lastShader, lastTicket, 
          bucket.rendermeshes.GetArray() + startMesh, i - startMesh,
          bucket.stacks.GetArray() + startMesh);
      }
      startMesh = i;
      lastShader = meshShader;
      lastTicket = newTicket;
    }
  }
  if ((lastShader != 0) && (lastShader != node.factory->nullShader))
  {
    //g3d->SetWorldToCamera (camt);
    node.RenderMeshes (g3d, lastShader, lastTicket, 
      bucket.rendermeshes.GetArray() + startMesh, 
      bucket.rendermeshes.Length() - startMesh,
      bucket.stacks.GetArray() + startMesh);
  }
}

//---------------------------------------------------------------------------

csStringID csMeshRenderNodeFactory::string_object2world = (csStringID)-1;

csMeshRenderNodeFactory::csMeshRenderNodeFactory (iObjectRegistry* object_reg)
{
  shaderManager = CS_QUERY_REGISTRY (object_reg, iShaderManager);
  nullShader = shaderManager->GetShader ("*null");

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.shared.stringset", iStringSet);
  string_object2world = strings->Request ("object2world transform");
}

csMeshRenderNode* csMeshRenderNodeFactory::CreateMeshNode (
  csStringID shaderType, iShader* defShader, 
  csShaderVariableContext& shadervars)
{
  return new csMeshRenderNode (this, shaderType, defShader, shadervars);
}
