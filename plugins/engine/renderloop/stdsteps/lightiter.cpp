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
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iengine/mesh.h"
#include "iengine/material.h"

#include "lightiter.h"

SCF_IMPLEMENT_FACTORY(csLightIterRSType);
SCF_IMPLEMENT_FACTORY(csLightIterRSLoader);

//---------------------------------------------------------------------------

csLightIterRSType::csLightIterRSType (iBase* p) : csBaseRenderStepType (p)
{
}

csPtr<iRenderStepFactory> csLightIterRSType::NewFactory()
{
  return csPtr<iRenderStepFactory> 
    (new csLightIterRenderStepFactory (object_reg));
}

//---------------------------------------------------------------------------

csLightIterRSLoader::csLightIterRSLoader (iBase* p) : csBaseRenderStepLoader (p)
{
  init_token_table (tokens);
}

bool csLightIterRSLoader::Initialize (iObjectRegistry* object_reg)
{
  if (csBaseRenderStepLoader::Initialize (object_reg))
  {
    return rsp.Initialize (object_reg);
  }
  else
  {
    return false;
  }
}

csPtr<iBase> csLightIterRSLoader::Parse (iDocumentNode* node, 
				       iLoaderContext* ldr_context,      
				       iBase* context)
{
  csRef<iLightIterRenderStep> step = 
    new csLightIterRenderStep (object_reg);
  csRef<iRenderStepContainer> steps =
    SCF_QUERY_INTERFACE (step, iRenderStepContainer);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request (child->GetValue ());
    switch (id)
    {
      case XMLTOKEN_STEPS:
	{
	  if (!rsp.ParseRenderSteps (steps, child))
	    return 0;
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

SCF_IMPLEMENT_IBASE(csLightIterRenderStepFactory);
  SCF_IMPLEMENTS_INTERFACE(iRenderStepFactory);
SCF_IMPLEMENT_EMBEDDED_IBASE_END;

csLightIterRenderStepFactory::csLightIterRenderStepFactory (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);

  csLightIterRenderStepFactory::object_reg = object_reg;
}

csLightIterRenderStepFactory::~csLightIterRenderStepFactory ()
{
}

csPtr<iRenderStep> csLightIterRenderStepFactory::Create ()
{
  return csPtr<iRenderStep> 
    (new csLightIterRenderStep (object_reg));
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csLightIterRenderStep);
  SCF_IMPLEMENTS_INTERFACE(iRenderStep);
  SCF_IMPLEMENTS_INTERFACE(iRenderStepContainer);
  SCF_IMPLEMENTS_INTERFACE(iLightIterRenderStep);
SCF_IMPLEMENT_EMBEDDED_IBASE_END;

csLightIterRenderStep::csLightIterRenderStep (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);

  csLightIterRenderStep::object_reg = object_reg;

  initialized = false;
}

csLightIterRenderStep::~csLightIterRenderStep ()
{
}

void csLightIterRenderStep::InitVariables ()
{
  if (!initialized)
  {
    initialized = true;

    csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
      object_reg, "crystalspace.renderer.stringset", iStringSet);

    csStringID posname = strings->Request ("STANDARD_LIGHT_0_POSITION");
    csStringID difname = strings->Request ("STANDARD_LIGHT_0_DIFFUSE");
    csStringID spcname = strings->Request ("STANDARD_LIGHT_0_SPECULAR");
    csStringID attname = strings->Request ("STANDARD_LIGHT_0_ATTENUATION");

    csRef<iShaderManager> shadermgr = CS_QUERY_REGISTRY (
    	object_reg, iShaderManager);

    shvar_light_0_position = shadermgr->GetVariable (posname);
    if (!shvar_light_0_position)
    {
      shvar_light_0_position = shadermgr->CreateVariable(posname);
      shvar_light_0_position->SetType(csShaderVariable::VECTOR4);
      shadermgr->AddVariable(shvar_light_0_position);
    }

    shvar_light_0_diffuse = shadermgr->GetVariable (difname);
    if (!shvar_light_0_diffuse)
    {
      shvar_light_0_diffuse = shadermgr->CreateVariable(difname);
      shvar_light_0_diffuse->SetType(csShaderVariable::VECTOR4);
      shadermgr->AddVariable(shvar_light_0_diffuse);
    }

    shvar_light_0_specular = shadermgr->GetVariable (spcname);
    if (!shvar_light_0_specular)
    {
      shvar_light_0_specular = shadermgr->CreateVariable(spcname);
      shvar_light_0_specular->SetType(csShaderVariable::VECTOR4);
      shadermgr->AddVariable(shvar_light_0_specular);
    }

    shvar_light_0_attenuation = shadermgr->GetVariable (attname);
    if (!shvar_light_0_attenuation)
    {
      shvar_light_0_attenuation = shadermgr->CreateVariable(attname);
      shvar_light_0_attenuation->SetType(csShaderVariable::VECTOR4);
      shadermgr->AddVariable(shvar_light_0_attenuation);
    }
  }
}

void csLightIterRenderStep::Perform (iRenderView* rview, iSector* sector)
{
  InitVariables ();

  // @@@ This code is ignoring dynamic lights. Perhaps we need a better
  // way to represent those.
  iLightList* lights = sector->GetLights();
  int nlights = lights->GetCount();

  while (nlights-- > 0)
  {
    iLight* light = lights->Get (nlights);
    const csVector3 lightPos = light->GetCenter ();

    /* 
    @@@ material specific diffuse/specular/ambient.
    Realized as shader variables maybe?
    */
    csReversibleTransform camTransR = rview->GetCamera()->GetTransform();

    const csColor& color = light->GetColor ();
    shvar_light_0_diffuse->SetValue (
      csVector3 (color.red, color.green, color.blue));

    shvar_light_0_specular->SetValue (csVector3 (1));
    shvar_light_0_attenuation->SetValue (light->GetAttenuationVector ());
    shvar_light_0_position->SetValue (lightPos * camTransR);

    csSphere lightSphere (lightPos, light->GetInfluenceRadius ());
    if (rview->TestBSphere (camTransR, lightSphere))
    {
      int i;
      for (i = 0; i < steps.Length(); i++)
      {
        steps[i]->Perform (rview, sector, light);
      }
    }
  }
}

int csLightIterRenderStep::AddStep (iRenderStep* step)
{
  csRef<iLightRenderStep> lrs = 
    SCF_QUERY_INTERFACE (step, iLightRenderStep);
  if (!lrs) return -1;
  return steps.Push (lrs);
}

int csLightIterRenderStep::GetStepCount ()
{
  return steps.Length();
}


