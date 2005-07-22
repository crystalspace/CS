/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2003 by Anders Stenberg

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
#include "iutil/strset.h"
#include "ivideo/rndbuf.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iengine/mesh.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "ivideo/material.h"
#include "csgeom/transfrm.h"
#include "csgfx/shadervar.h"
#include "csgfx/shadervarcontext.h"

#include "fullquad.h"

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY(csFullScreenQuadRSType)
SCF_IMPLEMENT_FACTORY(csFullScreenQuadRSLoader)

//---------------------------------------------------------------------------

csFullScreenQuadRSType::csFullScreenQuadRSType (iBase* p)
	: csBaseRenderStepType (p)
{
}

csPtr<iRenderStepFactory> csFullScreenQuadRSType::NewFactory()
{
  return csPtr<iRenderStepFactory> 
    (new csFullScreenQuadRenderStepFactory (object_reg));
}

//---------------------------------------------------------------------------

csFullScreenQuadRSLoader::csFullScreenQuadRSLoader (iBase* p)
	: csBaseRenderStepLoader (p)
{
  InitTokenTable (tokens);
}

csPtr<iBase> csFullScreenQuadRSLoader::Parse (iDocumentNode* node, 
				       iLoaderContext* ldr_context,      
				       iBase* context)
{
  csFullScreenQuadRenderStep* newstep = 
    new csFullScreenQuadRenderStep (object_reg);
  csRef<iRenderStep> step;
  step.AttachNew (newstep);    

  if (!ParseStep (node, newstep, newstep->GetOtherSettings(),
    false))
    return 0;

  if (newstep->GetDistinguishFirstPass () &&
    newstep->GetFirstSettings().shader.IsEmpty() &&
    (newstep->GetFirstSettings().material.IsEmpty() ||
    (newstep->GetFirstSettings().shadertype == csInvalidStringID)))
  {
    synldr->Report ("crystalspace.renderloop.step.fullscreenquad",
      CS_REPORTER_SEVERITY_WARNING, node,
      "Neither a shader nor a material & shadertype was set for first pass");
  }

  if (newstep->GetOtherSettings().shader.IsEmpty() &&
    (newstep->GetOtherSettings().material.IsEmpty() ||
    (newstep->GetOtherSettings().shadertype == csInvalidStringID)))
  {
    synldr->Report ("crystalspace.renderloop.step.fullscreenquad",
      CS_REPORTER_SEVERITY_WARNING, node,
      "Neither a shader nor a material & shadertype was set for other passes");
  }

  return csPtr<iBase> (step);
}

bool csFullScreenQuadRSLoader::ParseStep (iDocumentNode* node,
  csFullScreenQuadRenderStep* step, 
  csFullScreenQuadRenderStep::DrawSettings& settings, bool firstPass)
{
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request (child->GetValue ());
    switch (id)
    {
      case XMLTOKEN_MATERIAL:
	{
	  settings.material = child->GetContentsValue ();
	}
	break;
      case XMLTOKEN_SHADERTYPE:
        {
	  settings.shadertype = 
	    strings->Request (child->GetContentsValue ());
          //((csFullScreenQuadRenderStep*)(void*)step)->
          //  SetShaderType (strings->Request (child->GetContentsValue ()));
        }
        break;
      case XMLTOKEN_SHADER:
	{
	  settings.shader = child->GetContentsValue ();
	}
	break;
      case XMLTOKEN_MIXMODE:
	if (!synldr->ParseMixmode (child, settings.mixmode))
	{
	  return false;
	}
	break;
      case XMLTOKEN_ALPHAMODE:
	if (!synldr->ParseAlphaMode (child, strings, settings.alphaMode))
	{
	  return false;
	}
	break;
      case XMLTOKEN_FIRSTPASS:
	{
	  if (firstPass)
	  {
	    synldr->Report ("crystalspace.renderloop.step.fullscreenquad",
	      CS_REPORTER_SEVERITY_WARNING, child, 
	      "Can't nest <firstpass> tokens");
	    return false;
	  }
	  csFullScreenQuadRenderStep::DrawSettings& firstSettings =
	    step->GetFirstSettings();
	  firstSettings = settings;
	  if (!ParseStep (child, step, firstSettings, true))
	    return false;
	}
	break;
      case XMLTOKEN_SHADERVAR:
	{
	  const char* varname = child->GetAttributeValue ("name");
	  if (!varname)
	  {
	    synldr->Report ("crystalspace.renderloop.step.fullscreenquad",
	      CS_REPORTER_SEVERITY_WARNING, child,
	      "<shadervar> without name");
	    return false;
	  }
	  if (!settings.svContext.IsValid())
	    settings.svContext.AttachNew (new csShaderVariableContext ());

	  csRef<csShaderVariable> var;
	  var.AttachNew (new csShaderVariable (strings->Request (varname)));

	  if (!synldr->ParseShaderVar (child, *var))
	  {
	    return false;
	  }
	  settings.svContext->AddVariable (var);
	}
	break;
      case XMLTOKEN_TEXTURE:
	{
	  settings.texture = child->GetContentsValue ();
	}
	break;
      default:
	if (synldr) synldr->ReportBadToken (child);
	return 0;
    }
  }
  return true;
}


//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csFullScreenQuadRenderStepFactory);
  SCF_IMPLEMENTS_INTERFACE(iRenderStepFactory);
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csFullScreenQuadRenderStepFactory::csFullScreenQuadRenderStepFactory (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);
  csFullScreenQuadRenderStepFactory::object_reg = object_reg;
}

csFullScreenQuadRenderStepFactory::~csFullScreenQuadRenderStepFactory ()
{
  SCF_DESTRUCT_IBASE();
}

csPtr<iRenderStep> csFullScreenQuadRenderStepFactory::Create ()
{
  return csPtr<iRenderStep> 
    (new csFullScreenQuadRenderStep (object_reg));
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csFullScreenQuadRenderStep);
  SCF_IMPLEMENTS_INTERFACE(iRenderStep);
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csFullScreenQuadRenderStep::csFullScreenQuadRenderStep (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);

  csRef<iGraphics3D> g3d = 
    CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.shared.stringset", iStringSet);
  csFullScreenQuadRenderStep::object_reg = object_reg;

  engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  firstPass.material = "";
  firstPass.shader = "";
  firstPass.texture = "";
  firstPass.shadertype = csInvalidStringID;
  firstPass.mixmode = CS_FX_COPY;
  firstPass.alphaMode.autoAlphaMode = true;
  firstPass.alphaMode.autoModeTexture = strings->Request (
    CS_MATERIAL_TEXTURE_DIFFUSE);

  otherPasses = firstPass;
  distinguishFirstPass = false;
  isFirstPass = true;
}

csFullScreenQuadRenderStep::~csFullScreenQuadRenderStep ()
{
  SCF_DESTRUCT_IBASE();
}

void csFullScreenQuadRenderStep::Perform (iRenderView* rview, iSector* sector,
  csShaderVarStack &stacks)
{
  csRef<iGraphics3D> g3d = rview->GetGraphics3D();
  if (!shaderMgr.IsValid())
    shaderMgr = CS_QUERY_REGISTRY (object_reg, iShaderManager);

  const DrawSettings& settings = 
    (distinguishFirstPass && isFirstPass) ? firstPass : otherPasses;
  isFirstPass = false;

  iShader* shader = 0;

  if (!settings.shader.IsEmpty())
  {
    shader = shaderMgr->GetShader (settings.shader);
  }

  if ((shader == 0) && (!settings.material.IsEmpty() &&
    (settings.shadertype != csInvalidStringID)))
  {
    iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName (
      settings.material);
    if (mat != 0)
    {
      mat->Visit(); // @@@ here?
      shader = mat->GetMaterial()->GetShader (settings.shadertype);
    }
  }

  if (shader != 0)
  {
    static uint indices[4] = {0, 1, 2, 3};
    csVector3 verts[4];
    csVector2 texels[4];

    csSimpleRenderMesh mesh;
    mesh.meshtype = CS_MESHTYPE_QUADS;
    mesh.indexCount = 4;
    mesh.indices = indices;
    mesh.vertexCount = 4;

    float hw = float (g3d->GetWidth () / 2);
    float hh = float (g3d->GetHeight () / 2);
    float asp = hw / hh;

    verts[0].Set (-asp, -1.0f, 2.0f);
    texels[0].Set (0.0f, 1.0f);
    verts[1].Set (-asp,  1.0f, 2.0f);
    texels[1].Set (0.0f, 0.0f);
    verts[2].Set ( asp,  1.0f, 2.0f);
    texels[2].Set (1.0f, 0.0f);
    verts[3].Set ( asp, -1.0f, 2.0f);
    texels[3].Set (1.0f, 1.0f);

    mesh.vertices = verts;
    mesh.texcoords = texels;
    mesh.shader = shader;
    mesh.dynDomain = settings.svContext;
    mesh.alphaType = settings.alphaMode;
    mesh.mixmode = settings.mixmode;

    if (!settings.texture.IsEmpty())
    {
      iTextureWrapper* tex = engine->GetTextureList()->FindByName (
	settings.texture);
      if (tex != 0)
	mesh.texture = tex->GetTextureHandle();
    }

    g3d->DrawSimpleMesh (mesh);
  }
}

