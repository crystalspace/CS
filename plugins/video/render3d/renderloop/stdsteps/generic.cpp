/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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
	{
	  step->SetZBufMode (CS_ZBUF_USE);
	}
	break;
      case XMLTOKEN_ZTEST:
	{
	  step->SetZBufMode (CS_ZBUF_TEST);
	}
	break;
      case XMLTOKEN_ZFILL:
	{
	  step->SetZBufMode (CS_ZBUF_FILL);
	}
	break;
      case XMLTOKEN_ZNONE:
	{
	  step->SetZBufMode (CS_ZBUF_NONE);
	}
	break;
      case XMLTOKEN_SHADERTYPE:
	{
	  step->SetShaderType (child->GetContentsValue ());
	}
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
  SCF_IMPLEMENTS_INTERFACE(iVisibilityCullerListner)
SCF_IMPLEMENT_IBASE_END

csGenericRenderStep::csGenericRenderStep (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);

  strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.renderer.stringset", iStringSet);

  shadertype = 0;
  zOffset = false;
  zmode = CS_ZBUF_USE;
}

csGenericRenderStep::~csGenericRenderStep ()
{
}

void csGenericRenderStep::RenderMeshes (iGraphics3D* g3d,
                                         iShaderWrapper* shader, 
                                         csRenderMesh** meshes, 
                                         int num)
{
  if (num == 0) return;

  iShaderTechnique *tech = shader->GetShader()->GetBestTechnique ();

  for (int p=0; p<tech->GetPassCount (); p++)
  {
    iShaderPass *pass = tech->GetPass (p);
    pass->Activate (0);

    int j;
    for (j = 0; j < num; j++)
    {
      csRenderMesh* mesh = meshes[j];

      shader->SelectMaterial (mesh->material->GetMaterial ());

      pass->SetupState (mesh);

      uint mixsave = mesh->mixmode;
      uint mixmode = pass->GetMixmodeOverride ();
      if (mixmode != 0)
        mesh->mixmode = mixmode;

      g3d->DrawMesh (mesh);
      mesh->mixmode = mixsave;

      pass->ResetState ();
    }
    pass->Deactivate ();
  }
}

void csGenericRenderStep::Perform (iRenderView* rview, iSector* sector)
{
  iGraphics3D* g3d = rview->GetGraphics3D();

  if (zOffset)
    g3d->EnableZOffset ();

  g3d->SetZMode (zmode);

  iVisibilityCuller* viscull = sector->GetVisibilityCuller ();
  // @@@ Don't alloc/dealloc every frame!
  ViscullCallback* callback = new ViscullCallback (g3d, shadertype, rview);
  viscull->VisTest (rview, callback);
  delete callback;

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

csGenericRenderStep::ViscullCallback::ViscullCallback (iGraphics3D* g3d, 
						       csStringID shadertype,
						       iRenderView* rview)
{
  SCF_CONSTRUCT_IBASE(0);
  ViscullCallback::g3d = g3d;
  ViscullCallback::shadertype = shadertype;
  ViscullCallback::rview = rview;
}

void csGenericRenderStep::ViscullCallback::ObjectVisible (
  iVisibilityObject *visobject, iMeshWrapper *mesh)
{
  if (!mesh->GetMeshObject ()->DrawTest (rview, mesh->GetMovable ())) return;

  int num;
  csRenderMesh** meshes = mesh->GetRenderMeshes (num);
  CS_ALLOC_STACK_ARRAY (csRenderMesh*, sameShaderMeshes, num);
  int numSSM = 0;
  iShaderWrapper* shader = 0;

  for (int n = 0; n < num; n++)
  {
    csRenderMesh* mesh = meshes[n];

    iShaderWrapper* meshShader = 
      mesh->material->GetMaterialHandle()->GetShader(shadertype);
    if (meshShader != shader)
    {
      // @@@ Need error reporter
      if (shader != 0)
      {
        csGenericRenderStep::RenderMeshes (g3d, shader, sameShaderMeshes, 
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
      csGenericRenderStep::RenderMeshes (g3d, shader, sameShaderMeshes, 
        numSSM);
    }
  }

  
}
