/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2005 by Marten Svanfeldt

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
#include "csqint.h"

#include "csgeom/sphere.h"
#include "csgfx/csimgvec.h"
#include "csgfx/memimage.h"
#include "csutil/cscolor.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "igraphic/image.h"
#include "iutil/document.h"
#include "ivideo/rndbuf.h"
#include "ivideo/txtmgr.h"

#include "lightiter.h"

CS_LEAKGUARD_IMPLEMENT (csLightIterRSType);
CS_LEAKGUARD_IMPLEMENT (csLightIterRSLoader);
CS_LEAKGUARD_IMPLEMENT (csLightIterRenderStepFactory);
CS_LEAKGUARD_IMPLEMENT (csLightIterRenderStep::LightSVAccessor);

SCF_IMPLEMENT_FACTORY(csLightIterRSType)
SCF_IMPLEMENT_FACTORY(csLightIterRSLoader)

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
  InitTokenTable (tokens);
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
				       iStreamSource*,
				       iLoaderContext* ldr_context,      
				       iBase* context)
{
  csRef<iLightIterRenderStep> step;
  step.AttachNew (new csLightIterRenderStep (object_reg));
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
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csLightIterRenderStepFactory::csLightIterRenderStepFactory (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);
  csLightIterRenderStepFactory::object_reg = object_reg;
}

csLightIterRenderStepFactory::~csLightIterRenderStepFactory ()
{
  SCF_DESTRUCT_IBASE();
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
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csLightIterRenderStep::csLightIterRenderStep (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);
  csLightIterRenderStep::object_reg = object_reg;
  initialized = false;
}

csLightIterRenderStep::~csLightIterRenderStep ()
{
  csHash<LightSVAccessor*, csPtrKey<iLight> >::GlobalIterator it =
    knownLights.GetIterator();

  while (it.HasNext())
  {
    csPtrKey<iLight> light;
    LightSVAccessor* cb = it.Next (light);
    light->RemoveLightCallback (cb);
  }

  SCF_DESTRUCT_IBASE();
}

void csLightIterRenderStep::Init ()
{
  if (!initialized)
  {
    initialized = true;

    g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);

    csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
      object_reg, "crystalspace.shared.stringset", iStringSet);

    csStringID posname = strings->Request ("light 0 position");
    csStringID poswname = strings->Request ("light 0 position world");
    csStringID difname = strings->Request ("light 0 diffuse");
    csStringID spcname = strings->Request ("light 0 specular");
    csStringID attname = strings->Request ("light 0 attenuation");
    csStringID atxname = strings->Request ("light 0 attenuationtex");

    shadermgr = CS_QUERY_REGISTRY (
    	object_reg, iShaderManager);

    shvar_light_0_position = shadermgr->GetVariable (posname);
    if (!shvar_light_0_position)
    {
      shvar_light_0_position = new csShaderVariable (posname);
      shvar_light_0_position->SetType (csShaderVariable::VECTOR4);
      shadermgr->AddVariable (shvar_light_0_position);
    }

    shvar_light_0_position_world = shadermgr->GetVariable (poswname);
    if (!shvar_light_0_position_world)
    {
      shvar_light_0_position_world = new csShaderVariable (poswname);
      shvar_light_0_position_world->SetType (csShaderVariable::VECTOR4);
      shadermgr->AddVariable (shvar_light_0_position_world);
    }

    shvar_light_0_diffuse = shadermgr->GetVariable (difname);
    if (!shvar_light_0_diffuse)
    {
      shvar_light_0_diffuse = new csShaderVariable (difname);
      shvar_light_0_diffuse->SetType (csShaderVariable::VECTOR4);
      shadermgr->AddVariable (shvar_light_0_diffuse);
    }

    shvar_light_0_specular = shadermgr->GetVariable (spcname);
    if (!shvar_light_0_specular)
    {
      shvar_light_0_specular = new csShaderVariable (spcname);
      shvar_light_0_specular->SetType (csShaderVariable::VECTOR4);
      shadermgr->AddVariable (shvar_light_0_specular);
    }

    shvar_light_0_attenuation = shadermgr->GetVariable (attname);
    if (!shvar_light_0_attenuation)
    {
      shvar_light_0_attenuation = new csShaderVariable (attname);
      shvar_light_0_attenuation->SetType (csShaderVariable::VECTOR4);
      shadermgr->AddVariable (shvar_light_0_attenuation);
    }

    shvar_light_0_attenuationtex = shadermgr->GetVariable (atxname);
    if (!shvar_light_0_attenuationtex)
    {
      shvar_light_0_attenuationtex = new csShaderVariable (atxname);
      shvar_light_0_attenuationtex->SetType (csShaderVariable::TEXTURE);
      shadermgr->AddVariable (shvar_light_0_attenuationtex);
    }
  }
}

csLightIterRenderStep::LightSVAccessor* 
csLightIterRenderStep::GetLightAccessor (iLight* light)
{
  LightSVAccessor* acc = knownLights.Get (light, 0);
  if (acc == 0)
  {
    acc = new LightSVAccessor (light, this);
    knownLights.Put (light, acc);
    acc->DecRef();
  }
  return acc;
}

void csLightIterRenderStep::Perform (iRenderView* rview, iSector* sector,
  csShaderVarStack &stacks)
{
  Init ();

  // @@@ This code is ignoring dynamic lights. Perhaps we need a better
  // way to represent those.
  iLightList* lights = sector->GetLights();
  int nlights = lights->GetCount();

  csArray<iLight*> lightList;

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

    csLightAttenuationMode attnMode = light->GetAttenuationMode ();
    if (attnMode == CS_ATTN_LINEAR)
    {
      float r = light->GetAttenuationConstants ().x;
      shvar_light_0_attenuation->SetValue (csVector3(r, 1/r, 0));
    }
    else
    {
      shvar_light_0_attenuation->SetValue (light->GetAttenuationConstants ());
    }
    shvar_light_0_position->SetValue (lightPos * camTransR);
    shvar_light_0_position_world->SetValue (lightPos);

    shvar_light_0_attenuationtex->SetAccessor (GetLightAccessor (light));

    lightList.Push (light);
    shadermgr->SetActiveLights (lightList);
    lightList.Empty ();

    csSphere lightSphere (lightPos, light->GetCutoffDistance ());
    if (rview->TestBSphere (camTransR, lightSphere))
    {
      size_t i;
      for (i = 0; i < steps.Length(); i++)
      {
        steps[i]->Perform (rview, sector, light, stacks);
      }
    }
  }
}

size_t csLightIterRenderStep::AddStep (iRenderStep* step)
{
  csRef<iLightRenderStep> lrs = 
    SCF_QUERY_INTERFACE (step, iLightRenderStep);
  if (!lrs) return csArrayItemNotFound;
  return steps.Push (lrs);
}

bool csLightIterRenderStep::DeleteStep (iRenderStep* step)
{
  csRef<iLightRenderStep> lrs = 
    SCF_QUERY_INTERFACE (step, iLightRenderStep);
  if (!lrs) return false;
  return steps.Delete(lrs);
}

iRenderStep* csLightIterRenderStep::GetStep (size_t n) const
{
  return (iRenderStep*) steps.Get(n);
}

size_t csLightIterRenderStep::Find (iRenderStep* step) const
{
  csRef<iLightRenderStep> lrs = 
    SCF_QUERY_INTERFACE (step, iLightRenderStep);
  if (!lrs) return csArrayItemNotFound;
  return steps.Find(lrs);
}

size_t csLightIterRenderStep::GetStepCount () const
{
  return steps.Length();
}

csPtr<iTextureHandle> csLightIterRenderStep::GetAttenuationTexture (
  int attnType)
{
  if (!attTex.IsValid())
  {
  #define CS_ATTTABLE_SIZE	  128
  #define CS_HALF_ATTTABLE_SIZE	  ((float)CS_ATTTABLE_SIZE/2.0f)

    csRGBpixel *attenuationdata = 
      new csRGBpixel[CS_ATTTABLE_SIZE * CS_ATTTABLE_SIZE];
    csRGBpixel* data = attenuationdata;
    for (int y=0; y < CS_ATTTABLE_SIZE; y++)
    {
      for (int x=0; x < CS_ATTTABLE_SIZE; x++)
      {
	float yv = 3.0f * ((y + 0.5f)/CS_HALF_ATTTABLE_SIZE - 1.0f);
	float xv = 3.0f * ((x + 0.5f)/CS_HALF_ATTTABLE_SIZE - 1.0f);
	float i = exp (-0.7 * (xv*xv + yv*yv));
	unsigned char v = i>1.0f ? 255 : csQint (i*255.99f);
	(data++)->Set (v, v, v, v);
      }
    }

    csRef<iImage> img = csPtr<iImage> (new csImageMemory (
      CS_ATTTABLE_SIZE, CS_ATTTABLE_SIZE, attenuationdata, true, 
      CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));
    attTex = g3d->GetTextureManager()->RegisterTexture (
	img, CS_TEXTURE_3D | CS_TEXTURE_CLAMP | CS_TEXTURE_NOMIPMAPS);
    attTex->SetTextureClass ("lookup");
  }
  return csPtr<iTextureHandle> (attTex);
}

csPtr<iTextureHandle> csLightIterRenderStep::GetAttenuationTexture (
  const csVector3& attnVec)
{
  if (attnVec.z != 0)
    return GetAttenuationTexture (CS_ATTN_REALISTIC);
  else if (attnVec.y != 0)
    return GetAttenuationTexture (CS_ATTN_INVERSE);
  else 
    return GetAttenuationTexture (CS_ATTN_NONE);
}


//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csLightIterRenderStep::LightSVAccessor)
  SCF_IMPLEMENTS_INTERFACE(iLightCallback)
  SCF_IMPLEMENTS_INTERFACE(iShaderVariableAccessor)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csLightIterRenderStep::LightSVAccessor::LightSVAccessor (iLight* light,
  csLightIterRenderStep* parent)
{
  SCF_CONSTRUCT_IBASE(0);

  LightSVAccessor::light = light;
  LightSVAccessor::parent = parent;

  needUpdate = true;
  light->SetLightCallback (this);
}

csLightIterRenderStep::LightSVAccessor::~LightSVAccessor ()
{
  SCF_DESTRUCT_IBASE();
}

void csLightIterRenderStep::LightSVAccessor::OnColorChange (iLight* light, 
  const csColor& newcolor)
{
}

void csLightIterRenderStep::LightSVAccessor::OnPositionChange (iLight* light, 
  const csVector3& newpos)
{
}

void csLightIterRenderStep::LightSVAccessor::OnSectorChange (iLight* light, 
  iSector* newsector)
{
}

void csLightIterRenderStep::LightSVAccessor::OnRadiusChange (iLight* light, 
  float newradius)
{
}

void csLightIterRenderStep::LightSVAccessor::OnDestroy (iLight* light)
{
  parent->knownLights.Delete (this->light, this);
  //delete this;
}

void csLightIterRenderStep::LightSVAccessor::OnAttenuationChange (
  iLight* light, int newatt)
{
  needUpdate = true;
  attnType = newatt;
}

void csLightIterRenderStep::LightSVAccessor::PreGetValue (
  csShaderVariable *variable)
{
  if (needUpdate)
  {
    //CreateTexture ();
    if (attnType == CS_ATTN_CLQ)
    {
      const csVector3& attnVec = light->GetAttenuationConstants ();
      attTex = parent->GetAttenuationTexture (attnVec);
    }
    else
      attTex = parent->GetAttenuationTexture (attnType);

    needUpdate = false;
  }
  variable->SetValue (attTex);
}
