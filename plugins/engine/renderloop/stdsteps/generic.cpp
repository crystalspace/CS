/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2003 by Marten Svanfeldt

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

#include "iutil/document.h"
#include "ivideo/rndbuf.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"
#include "ivideo/material.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iengine/mesh.h"
#include "iengine/material.h"
#include "ivideo/material.h"
#include "imesh/object.h"
#include "iengine/portalcontainer.h"

#include "generic.h"

SCF_IMPLEMENT_FACTORY(csGenericRSType);
SCF_IMPLEMENT_FACTORY(csGenericRSLoader);

//---------------------------------------------------------------------------

csGenericRSType::csGenericRSType (iBase* p) : csBaseRenderStepType (p)
{
}

csPtr<iRenderStepFactory> csGenericRSType::NewFactory()
{
  return csPtr<iRenderStepFactory> 
    (new csGenericRenderStepFactory (object_reg));
}

//---------------------------------------------------------------------------

csGenericRSLoader::csGenericRSLoader (iBase* p) : csBaseRenderStepLoader (p)
{
  init_token_table (tokens);
}

csPtr<iBase> csGenericRSLoader::Parse (iDocumentNode* node, 
				       iLoaderContext* ldr_context,      
				       iBase* context)
{
  csRef<iGenericRenderStep> step = 
    new csGenericRenderStep (object_reg);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request (child->GetValue ());
    switch (id)
    {
      case XMLTOKEN_ZOFFSET:
	{
	  bool result;
	  if (!synldr->ParseBool (child, result, false))
	  {
	    return 0;
	  }
	  step->SetZOffset (result);
	}
	break;
      case XMLTOKEN_ZUSE:
	step->SetZBufMode (CS_ZBUF_USE);
	break;
      case XMLTOKEN_ZTEST:
	step->SetZBufMode (CS_ZBUF_TEST);
	break;
      case XMLTOKEN_ZFILL:
	step->SetZBufMode (CS_ZBUF_FILL);
	break;
      case XMLTOKEN_ZNONE:
	step->SetZBufMode (CS_ZBUF_NONE);
	break;
      case XMLTOKEN_ZMESH:
        step->SetZBufMode (CS_ZBUF_MESH);
        break;
      case XMLTOKEN_SHADERTYPE:
	step->SetShaderType (child->GetContentsValue ());
	break;
      default:
	if (synldr) synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (step);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csGenericRenderStepFactory);
  SCF_IMPLEMENTS_INTERFACE(iRenderStepFactory);
SCF_IMPLEMENT_IBASE_END;

csGenericRenderStepFactory::csGenericRenderStepFactory (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);
  csGenericRenderStepFactory::object_reg = object_reg;
}

csGenericRenderStepFactory::~csGenericRenderStepFactory ()
{
  SCF_DESTRUCT_IBASE();
}

csPtr<iRenderStep> csGenericRenderStepFactory::Create ()
{
  return csPtr<iRenderStep> 
    (new csGenericRenderStep (object_reg));
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csGenericRenderStep)
  SCF_IMPLEMENTS_INTERFACE(iRenderStep)
  SCF_IMPLEMENTS_INTERFACE(iGenericRenderStep)
  SCF_IMPLEMENTS_INTERFACE(iLightRenderStep)
SCF_IMPLEMENT_IBASE_END

csGenericRenderStep::csGenericRenderStep (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);

  objreg = object_reg;

  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.shared.stringset", iStringSet);
  shaderManager = CS_QUERY_REGISTRY (object_reg, iShaderManager);

  shadertype = 0;
  zOffset = false;
  zmode = CS_ZBUF_USE;
  currentSettings = false;
}

csGenericRenderStep::~csGenericRenderStep ()
{
  SCF_DESTRUCT_IBASE();
}

void csGenericRenderStep::RenderMeshes (iGraphics3D* g3d,
                                        iShader* shader, 
                                        csRenderMesh** meshes, 
                                        int num,
                                        CS_SHADERVAR_STACK &stacks)
{
  if (num == 0) return;
  ToggleStepSettings (g3d, true);
  if (!shaderManager)
  {
    shaderManager = CS_QUERY_REGISTRY (objreg, iShaderManager);
  }

  iMaterial *material = 0;

  int numPasses = shader->GetNumberOfPasses ();
  for (int p=0; p < numPasses; p++)
  {
    shader->ActivatePass (p);

    int j;
    for (j = 0; j < num; j++)
    {
      csRenderMesh* mesh = meshes[j];
      if (mesh->material->GetMaterial () != material)
      {
        if (material != 0)
        {
          shader->PopVariables (stacks);
          material->PopVariables (stacks);
        }
        material = mesh->material->GetMaterial ();
        material->PushVariables (stacks);
        shader->PushVariables (stacks);
      }
      if (mesh->variablecontext)
        mesh->variablecontext->PushVariables (stacks);
      shader->SetupPass (mesh, stacks);
      g3d->DrawMesh (mesh, stacks);
      shader->TeardownPass ();
      if (mesh->variablecontext)
        mesh->variablecontext->PopVariables (stacks);
    }
    shader->DeactivatePass ();
  }

  if (material != 0)
  {
    shader->PopVariables (stacks);
    material->PopVariables (stacks);
  }
}

void csGenericRenderStep::Perform (iRenderView* rview, iSector* sector,
  CS_SHADERVAR_STACK &stacks)
{
  iGraphics3D* g3d = rview->GetGraphics3D();

  csRenderMeshList* meshlist = sector->GetVisibleMeshes (rview);
  int num = meshlist->SortMeshLists ();
  CS_ALLOC_STACK_ARRAY (csRenderMesh*, sameShaderMeshes, num);
  csArray<csRenderMesh> saveMeshes;
  meshlist->GetSortedMeshes (sameShaderMeshes);
 
  int lastidx = 0;
  int numSSM = 0;
  iShader* shader = 0;

  for (int n = 0; n < num; n++)
  {
    csRenderMesh* mesh = sameShaderMeshes[n];

    if (mesh->portal) 
    {
      if (numSSM > 0)
      {
        if (shader != 0)
	{
          RenderMeshes (g3d, shader, sameShaderMeshes+lastidx, 
            numSSM, stacks);
          shader = 0;
	}
        numSSM = 0;
      }

      ToggleStepSettings (g3d, false);
      /*
        When drawing the contents of a portal, the transforms in the
	rendermeshes will be changed to reflect the portal transform.
	However, as meshes usually keep just 1 copy RM, it'll affect
	the original mesh as well.
	@@@ Uuuugly workaround: copy the RMs before drawing behind the
	portal and copy them back afterwards.
       */
      saveMeshes.SetLength (num - n);
      for (int i = n; i < num; i++)
	saveMeshes[i - n] = *sameShaderMeshes[i - n];
      mesh->portal->Draw (rview);
      for (int i = n; i < num; i++)
	*sameShaderMeshes[i - n] = saveMeshes[i - n];
    }
    else 
    {
      iShader* meshShader = 
        mesh->material->GetMaterialHandle()->GetShader(shadertype);
      if (meshShader != shader)
      {
        // @@@ Need error reporter
        if (shader != 0)
	{
          RenderMeshes (g3d, shader, sameShaderMeshes+lastidx, 
            numSSM, stacks);
	}
	lastidx = n;
        shader = meshShader;
        numSSM = 0;
      }
      numSSM++;
    }
  }
  
  if (numSSM != 0)
  {
    // @@@ Need error reporter
    if (shader != 0)
      RenderMeshes (g3d, shader, sameShaderMeshes+lastidx, 
        numSSM, stacks);
  }

  ToggleStepSettings (g3d, false);
}

void csGenericRenderStep::ToggleStepSettings (iGraphics3D* g3d, 
					      bool settings)
{
  if (settings != currentSettings)
  {
    if (settings)
    {
      if (zOffset)
	g3d->EnableZOffset ();
      g3d->SetZMode (zmode);
    }
    else
    {
      if (zOffset)
	g3d->DisableZOffset ();
    }
    currentSettings = settings;
  }
}

void csGenericRenderStep::Perform (iRenderView* rview, iSector* sector,
				   iLight* light,
                                   CS_SHADERVAR_STACK &stacks)
{
  Perform (rview, sector, stacks);
}

void csGenericRenderStep::SetShaderType (const char* type)
{
  shadertype = strings->Request (type);
}

const char* csGenericRenderStep::GetShaderType ()
{
  return strings->Request (shadertype);
}

void csGenericRenderStep::SetZOffset (bool zOffset)
{
  csGenericRenderStep::zOffset = zOffset;
}

bool csGenericRenderStep::GetZOffset ()
{
  return zOffset;
}

void csGenericRenderStep::SetZBufMode (csZBufMode zmode)
{
  csGenericRenderStep::zmode = zmode;
}

csZBufMode csGenericRenderStep::GetZBufMode ()
{
  return zmode;
}

