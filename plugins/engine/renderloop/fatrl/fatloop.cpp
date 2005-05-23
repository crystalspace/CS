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
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iutil/document.h"
#include "ivideo/material.h"
#include "ivideo/rendermesh.h"

#include "csutil/bitarray.h"
#include "csutil/sysfunc.h"
#include "cstool/rendermeshlist.h"

#include "fatloop.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csFatLoopType);
SCF_IMPLEMENT_FACTORY(csFatLoopLoader);

//---------------------------------------------------------------------------

csFatLoopType::csFatLoopType (iBase* p) : csBaseRenderStepType (p)
{
}

csPtr<iRenderStepFactory> csFatLoopType::NewFactory()
{
  return csPtr<iRenderStepFactory> (new csFatLoopFactory (object_reg));
}

//---------------------------------------------------------------------------

csFatLoopLoader::csFatLoopLoader (iBase* p) : csBaseRenderStepLoader (p)
{
  InitTokenTable (tokens);
}

csPtr<iBase> csFatLoopLoader::Parse (iDocumentNode* node, 
                                     iLoaderContext* ldr_context,
                                     iBase* context)
{
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.shared.stringset", iStringSet);

  csRef<csFatLoopStep> step;
  step.AttachNew (new csFatLoopStep (object_reg));

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request (child->GetValue ());
    switch (id)
    {
      /*case XMLTOKEN_PORTALTRAVERSAL:
        {
	  bool result;
	  if (!synldr->ParseBool (child, result, true))
	    return 0;
	  step->SetPortalTraversal (result);
	}
	break;
      case XMLTOKEN_ZOFFSET:
	{
	  bool result;
	  if (!synldr->ParseBool (child, result, true))
	    return 0;
	  step->SetZOffset (result);
	}
	break;*/
      case XMLTOKEN_SHADERTYPE:
	step->shadertype = strings->Request (child->GetContentsValue ());
	break;
      case XMLTOKEN_DEFAULTSHADER:
	{
	  step->defShader = synldr->ParseShaderRef (child);
	}
	break;
      /*case XMLTOKEN_NODEFAULTTRIGGER:
	step->AddDisableDefaultTriggerType (child->GetContentsValue ());
	break;*/
      default:
	{
	  synldr->ReportBadToken (child);
	}
	return 0;
    }
  }

  //step->shadertype = strings->Request (type);
  //step->defShader = synldr->ParseShaderRef (child);
  return csPtr<iBase> (step);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csFatLoopFactory);
  SCF_IMPLEMENTS_INTERFACE(iRenderStepFactory);
SCF_IMPLEMENT_IBASE_END;

csFatLoopFactory::csFatLoopFactory (iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);
  csFatLoopFactory::object_reg = object_reg;
}

csFatLoopFactory::~csFatLoopFactory ()
{
  SCF_DESTRUCT_IBASE();
}

csPtr<iRenderStep> csFatLoopFactory::Create ()
{
  return csPtr<iRenderStep> (new csFatLoopStep (object_reg));
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csFatLoopStep);
  SCF_IMPLEMENTS_INTERFACE(iRenderStep);
SCF_IMPLEMENT_IBASE_END;

csStringID csFatLoopStep::string_object2world = (csStringID)-1;

csFatLoopStep::csFatLoopStep (iObjectRegistry* object_reg) :
  visible_meshes_index(0)
{
  SCF_CONSTRUCT_IBASE(0);
  this->object_reg = object_reg;

  shaderManager = CS_QUERY_REGISTRY (object_reg, iShaderManager);
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.shared.stringset", iStringSet);
  string_object2world = strings->Request ("object2world transform");
}

csFatLoopStep::~csFatLoopStep ()
{
  SCF_DESTRUCT_IBASE();
}

class ShaderTicketHelper
{
private:
  csShaderVarStack& stacks;
  const csArray<csShaderVariableContext>& shadervars;
  size_t shadervars_idx;
  //csShaderVariableContext& shadervars;

  iMaterial* lastMat;
  iShader* lastShader;
  iShaderVariableContext* lastMeshContext;
  size_t matShadMeshTicket;

  void Reset ()
  {
    matShadMeshTicket = (size_t)~0;
  }

public:
  ShaderTicketHelper (csShaderVarStack& Stacks,
    const csArray<csShaderVariableContext>& sv,
    size_t sv_idx) :
    	stacks (Stacks), 
    	shadervars (sv),
	shadervars_idx (sv_idx)
  {
    lastMat = 0;
    lastShader = 0;
    lastMeshContext = 0;
    Reset ();
  }

  size_t GetTicket (iMaterial* material, iShader* shader, 
    iShaderVariableContext* meshContext, csRenderMesh* mesh)
  {
    if ((material != lastMat) || (shader != lastShader)
      || (meshContext != lastMeshContext))
    {
      Reset ();
      lastMat = material;
      lastShader = shader;
      lastMeshContext = meshContext;
    }
    if (mesh->variablecontext.IsValid () 
      && !mesh->variablecontext->IsEmpty())
    {
      shader->PushVariables (stacks);
      material->PushVariables (stacks);
      if (meshContext) meshContext->PushVariables (stacks);
      mesh->variablecontext->PushVariables (stacks);
      shadervars[shadervars_idx].PushVariables (stacks);

      csRenderMeshModes modes (*mesh);
      size_t retTicket = shader->GetTicket (modes, stacks);

      shadervars[shadervars_idx].PopVariables (stacks);
      mesh->variablecontext->PopVariables (stacks);
      if (meshContext) meshContext->PopVariables (stacks);
      material->PopVariables (stacks);
      shader->PopVariables (stacks);
      return retTicket;
    }
    else
    {
      if (matShadMeshTicket == (size_t)~0)
      {
	shader->PushVariables (stacks);
	material->PushVariables (stacks);
	if (meshContext) meshContext->PushVariables (stacks);
	shadervars[shadervars_idx].PushVariables (stacks);

	csRenderMeshModes modes (*mesh);
	matShadMeshTicket = shader->GetTicket (modes, stacks);

	shadervars[shadervars_idx].PopVariables (stacks);
	if (meshContext) meshContext->PopVariables (stacks);
	material->PopVariables (stacks);
	shader->PopVariables (stacks);
      }
      return matShadMeshTicket;
    }
  }
};

void csFatLoopStep::RenderMeshes (iGraphics3D* g3d,
				  iShader* shader, size_t ticket,
				  iShaderVariableContext** meshContexts,
				  csRenderMesh** meshes, 
				  size_t num,
				  csShaderVarStack& stacks)
{
  if (num == 0) return;
  //ToggleStepSettings (g3d, true);
  if (!shaderManager)
  {
    shaderManager = CS_QUERY_REGISTRY (object_reg, iShaderManager);
  }
  csRef<csShaderVariable> svO2W = 
    shadervars.Top ().GetVariable(string_object2world);
  shaderManager->PushVariables (stacks);

  iMaterial *material = 0;
  iShaderVariableContext* lastMeshContext = 0;
  
  size_t numPasses = shader->GetNumberOfPasses (ticket);
  for (size_t p = 0; p < numPasses; p++)
  {
    shader->ActivatePass (ticket, p);

    size_t j;
    for (j = 0; j < num; j++)
    {
      csRenderMesh* mesh = meshes[j];
      iShaderVariableContext* meshContext = meshContexts[j];
      if (meshContext->IsEmpty())
	meshContext = 0;
      //if ((!portalTraversal) && mesh->portal != 0) continue;

      svO2W->SetValue (mesh->object2world);

      if ((mesh->material->GetMaterial () != material)
	|| (meshContext != lastMeshContext))
      {
	if (lastMeshContext) meshContext->PopVariables (stacks);
        if (material != 0)
        {
          material->PopVariables (stacks);
          shader->PopVariables (stacks);
        }
        material = mesh->material->GetMaterial ();
	lastMeshContext = meshContext;
        shader->PushVariables (stacks);
        material->PushVariables (stacks);
	if (meshContext) meshContext->PushVariables (stacks);
      }
      shadervars.Top ().PushVariables (stacks);
      if (mesh->variablecontext)
        mesh->variablecontext->PushVariables (stacks);
      
      csRenderMeshModes modes (*mesh);
      shader->SetupPass (ticket, mesh, modes, stacks);
      g3d->DrawMesh (mesh, modes, stacks);
      shader->TeardownPass (ticket);

      if (mesh->variablecontext)
        mesh->variablecontext->PopVariables (stacks);
      shadervars.Top ().PopVariables (stacks);
    }
    shader->DeactivatePass (ticket);
  }

  if (lastMeshContext) lastMeshContext->PopVariables (stacks);
  if (material != 0)
  {
    material->PopVariables (stacks);
    shader->PopVariables (stacks);
  }

  shaderManager->PopVariables (stacks);
}

class PriorityHelper
{
  iEngine* engine;
  csBitArray knownPrios;
  csBitArray prioSorted;
public:
  PriorityHelper (iEngine* engine): engine(engine) {}
  bool IsPrioSpecial (long priority)
  {
    if ((knownPrios.Length() <= (size_t)priority) || (!knownPrios.IsBitSet (priority)))
    {
      if (knownPrios.Length() <= (size_t)priority) 
      {
        knownPrios.SetLength (priority + 1);
        prioSorted.SetLength (priority + 1);
      }
      prioSorted.Set (priority, 
        engine->GetRenderPrioritySorting (priority) != CS_RENDPRI_NONE);
      knownPrios.Set (priority);
    }
    return prioSorted.IsBitSet (priority);
  }
};

void csFatLoopStep::Perform (iRenderView* rview, iSector* sector,
                             csShaderVarStack &stacks)
{
  iGraphics3D* g3d = rview->GetGraphics3D();

  csRenderMeshList* meshlist = sector->GetVisibleMeshes (rview);
  size_t num = meshlist->SortMeshLists (rview);
  visible_meshes.SetLength (visible_meshes_index+num);
  imeshes_scratch.SetLength (num);
  mesh_svc.SetLength (visible_meshes_index+num);
  csRenderMesh** sameShaderMeshes = visible_meshes.GetArray ()
  	+ visible_meshes_index;
  iShaderVariableContext** sameShaderMeshSvcs = mesh_svc.GetArray ()
  	+ visible_meshes_index;
  size_t prev_visible_meshes_index = visible_meshes_index;
  visible_meshes_index += num;
  meshlist->GetSortedMeshes (sameShaderMeshes, imeshes_scratch.GetArray());
  for (size_t i = 0; i < num; i++)
    sameShaderMeshSvcs[i] = imeshes_scratch[i]->GetSVContext();

  shadervars.Push (csShaderVariableContext ());
  shadervars.Top ().GetVariableAdd (string_object2world);

  ShaderTicketHelper ticketHelper (stacks, shadervars, shadervars.Length ()-1);
  const csReversibleTransform& camt = rview->GetCamera ()->GetTransform ();

  PriorityHelper ph (engine);
  buckets.Empty();
  for (size_t n = 0; n < num; n++)
  {
#if 0
    csRenderMesh* mesh = sameShaderMeshes[n];

    if (!mesh->portal) 
    {
#ifdef CS_DEBUG
      if (!mesh->material)
      {
	csPrintfErr ("INTERNAL ERROR: mesh '%s' is missing a material!\n",
	  mesh->db_mesh_name);
	exit (-1);
      }
#endif
      iMaterial* hdl = mesh->material->GetMaterial ();
#ifdef CS_DEBUG
      if (!hdl)
      {
        csPrintfErr ("INTERNAL ERROR: mesh '%s' is missing a material!\n",
	  mesh->db_mesh_name);
	exit (-1);
      }
#endif
      iShader* meshShader = hdl->GetShader (shadertype);
      if (meshShader == 0) 
      {
	/*bool doDefault = true;
	for (size_t i = 0; i < disableDefaultTypes.Length(); i++)
	{
	  if (hdl->GetShader (disableDefaultTypes[i]) != 0)
	  {
	    doDefault = false;
	    break;
	  }
	}
	if (doDefault)*/ meshShader = defShader;
      }
      size_t newTicket = meshShader ? ticketHelper.GetTicket (
	mesh->material->GetMaterial (), meshShader, 
	sameShaderMeshSvcs[n], mesh) : (size_t)~0;
      g3d->SetWorldToCamera (camt);
      RenderMeshes (g3d, meshShader, newTicket, sameShaderMeshSvcs + n, 
	sameShaderMeshes + n, 1, stacks);
    }
#endif
    csRenderMesh* mesh = sameShaderMeshes[n];
    if (mesh->portal) continue;
    iMeshWrapper* mw = imeshes_scratch[n];
    long prio = mw->GetRenderPriority();
    ShaderTicketKey key;
    key.prio = prio;
    if (ph.IsPrioSpecial (prio))
    {
      key.shader = 0; key.ticket = (size_t)~0;
    }
    else
    {
      iMaterial* hdl = mesh->material->GetMaterial ();
      iShader* sortBy = hdl->GetShader (shadertype);
      if (sortBy == 0) sortBy = defShader;
      if (sortBy == 0) continue; // @@@ Perhaps an Assert? Or some Error?
      key.shader = sortBy;
      key.ticket = ticketHelper.GetTicket (
        mesh->material->GetMaterial (), sortBy, 
        sameShaderMeshSvcs[n], mesh);
    }
    MeshBucket* bucket = buckets.GetElementPointer (key);
    if (bucket == 0)
    {
      bucket = buckets.Put (key, MeshBucket());
    }
    bucket->rendermeshes.Push (mesh);
    bucket->wrappers.Push (mw);
    bucket->contexts.Push (sameShaderMeshSvcs[n]);
  }
  g3d->SetWorldToCamera (camt);
  g3d->SetZMode (CS_ZBUF_MESH);
  buckets.TraverseInOrder (TraverseShaderBuckets (*this, g3d, stacks));

  shadervars.Pop ();

  visible_meshes_index = prev_visible_meshes_index;
}

void csFatLoopStep::TraverseShaderBuckets::Process (const ShaderTicketKey& key,
  csFatLoopStep::MeshBucket &bucket)
{
  ShaderTicketHelper ticketHelper (stacks, step.shadervars, step.shadervars.Length ()-1);
  size_t startMesh = 0;
  iShader* lastShader = 0;
  size_t lastTicket = ~0;
  iShader* meshShader = key.shader;
  for (size_t i = 0; i < bucket.rendermeshes.Length(); i++)
  {
    if (key.shader == 0)
    {
      iMaterial* hdl = bucket.rendermeshes[i]->material->GetMaterial ();
      meshShader = hdl->GetShader (step.shadertype);
      if (meshShader == 0) meshShader = step.defShader;
    }
    size_t newTicket = (key.ticket != (size_t)~0) ? key.ticket :
      (meshShader ? ticketHelper.GetTicket (
      bucket.rendermeshes[i]->material->GetMaterial (), meshShader, 
      bucket.contexts[i], bucket.rendermeshes[i]) : (size_t)~0);
    if ((meshShader != lastShader) || (newTicket != lastTicket))
    {
      // @@@ Need error reporter
      if (lastShader != 0)
      {
        //g3d->SetWorldToCamera (camt);
        step.RenderMeshes (g3d, lastShader, lastTicket, 
          bucket.contexts.GetArray() + startMesh, 
          bucket.rendermeshes.GetArray() + startMesh, i - startMesh, stacks);
      }
      startMesh = i;
      lastShader = meshShader;
      lastTicket = newTicket;
    }
  }
  if (lastShader != 0)
  {
    //g3d->SetWorldToCamera (camt);
    step.RenderMeshes (g3d, lastShader, lastTicket, 
      bucket.contexts.GetArray() + startMesh, 
      bucket.rendermeshes.GetArray() + startMesh, 
      bucket.rendermeshes.Length() - startMesh, stacks);
  }
}
