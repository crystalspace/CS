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
SCF_IMPLEMENT_EMBEDDED_IBASE_END;

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

SCF_IMPLEMENT_IBASE(csGenericRenderStep::ViscullCallback)
  SCF_IMPLEMENTS_INTERFACE(iVisibilityCullerListener)
SCF_IMPLEMENT_IBASE_END

csGenericRenderStep::csGenericRenderStep (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);

  objreg = object_reg;

  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.renderer.stringset", iStringSet);
  shaderManager = CS_QUERY_REGISTRY (object_reg, iShaderManager);

  shadertype = 0;
  zOffset = false;
  zmode = CS_ZBUF_USE;
}

csGenericRenderStep::~csGenericRenderStep ()
{
  SCF_DESTRUCT_IBASE();
}

void csGenericRenderStep::RenderMeshes (iGraphics3D* g3d,
                                        iShader* shader, 
                                        csRenderMesh** meshes, 
                                        int num)
{
  if (num == 0) return;
  csArray<iShaderVariableContext*> dynDomain;
  if (!shaderManager)
  {
    shaderManager = CS_QUERY_REGISTRY (objreg, iShaderManager);
  }

  int numPasses = shader->GetNumberOfPasses ();
  for (int p=0; p < numPasses; p++)
  {
    shader->ActivatePass (p);

    int j;
    for (j = 0; j < num; j++)
    {
      dynDomain.Empty ();
      csRenderMesh* mesh = meshes[j];
      dynDomain.Push (shaderManager);
      dynDomain.Push (mesh->material->GetMaterial ());

      shader->SetupPass (mesh, dynDomain);
      g3d->DrawMesh (mesh);
      shader->TeardownPass ();
    }
    shader->DeactivatePass ();
  }
}

void csGenericRenderStep::Perform (iRenderView* rview, iSector* sector)
{
  iGraphics3D* g3d = rview->GetGraphics3D();

  if (zOffset)
    g3d->EnableZOffset ();

  g3d->SetZMode (zmode);

  //g3d->SetShadowState (CS_SHADOW_VOLUME_USE);
/*
  iVisibilityCuller* viscull = sector->GetVisibilityCuller ();
  ViscullCallback callback (rview, objreg);
  viscull->VisTest (rview, &callback);
  
  //draw
  callback.meshList.GetSortedMeshList (meshes);*/

  sector->GetVisibleMeshes (rview)->GetSortedMeshList (meshes);
  int num = meshes.Length ();
 
  CS_ALLOC_STACK_ARRAY (csRenderMesh*, sameShaderMeshes, num);
  int numSSM = 0;
  iShader* shader = 0;

  for (int n = 0; n < num; n++)
  {
    csRenderMesh* mesh = meshes[n];

    iShader* meshShader = 
      mesh->material->GetMaterialHandle()->GetShader(shadertype);
    if (meshShader != shader)
    {
      // @@@ Need error reporter
      if (shader != 0)
      {
        RenderMeshes (g3d, shader, sameShaderMeshes, 
          numSSM);
      }

      shader = meshShader;
      numSSM = 0;
    }
    sameShaderMeshes[numSSM++] = mesh;
  }
  
  if (numSSM != 0)
  {
    // @@@ Need error reporter
    if (shader != 0)
    {
      RenderMeshes (g3d, shader, sameShaderMeshes, 
        numSSM);
    }
  }
   
  //g3d->SetShadowState (CS_SHADOW_VOLUME_FINISH);

  if (zOffset)
    g3d->DisableZOffset ();
};

void csGenericRenderStep::Perform (iRenderView* rview, iSector* sector,
				   iLight* light)
{
  Perform (rview, sector);
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

csGenericRenderStep::ViscullCallback::ViscullCallback (iRenderView *rview, 
                                                       iObjectRegistry *objreg)
                                                       : meshList (objreg)
{
  SCF_CONSTRUCT_IBASE(0);
  ViscullCallback::rview = rview;
}

csGenericRenderStep::ViscullCallback::~ViscullCallback()
{
  SCF_DESTRUCT_IBASE();
}

void csGenericRenderStep::ViscullCallback::ObjectVisible (
  iVisibilityObject *visobject, iMeshWrapper *mesh)
{
  if (!mesh->GetMeshObject ()->DrawTest (rview, mesh->GetMovable ())) return;

  int num;
  csRenderMesh** meshes = mesh->GetRenderMeshes (num);
  meshList.AddRenderMeshes (meshes, num, mesh->GetRenderPriority ());
}
