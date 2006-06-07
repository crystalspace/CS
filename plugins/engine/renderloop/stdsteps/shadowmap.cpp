/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2003 by Anders Stenberg
              (C) 2006 by Hristo Hristov

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

#include "csgeom/transfrm.h"
#include "csgfx/shadervar.h"
#include "csgfx/shadervarcontext.h"

#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iutil/document.h"
#include "iutil/strset.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/rendermesh.h"
#include "ivideo/rndbuf.h"
#include <iengine/camera.h>
#include <iutil/event.h>
#include <iutil/eventq.h>
#include "csutil/event.h"
#include "csgfx/renderbuffer.h"
#include <cstool/rendermeshlist.h>

#include <imesh/genmesh.h>
#include <imesh/object.h>
#include <iengine/movable.h>
#include "csgfx/imagecubemapmaker.h"
#include "igraphic/image.h"
#include "ivideo/txtmgr.h"
#include <iutil/object.h>
#include "csgfx/memimage.h"

#include "shadowmap.h"

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY(csShadowmapRSType)
SCF_IMPLEMENT_FACTORY(csShadowmapRSLoader)

//---------------------------------------------------------------------------

csShadowmapRSType::csShadowmapRSType (iBase* p)
	: csBaseRenderStepType (p)
{
}

csPtr<iRenderStepFactory> csShadowmapRSType::NewFactory()
{
  return csPtr<iRenderStepFactory> 
    (new csShadowmapRenderStepFactory (object_reg));
}

//---------------------------------------------------------------------------

csShadowmapRSLoader::csShadowmapRSLoader (iBase* p)
	: csBaseRenderStepLoader (p)
{
  InitTokenTable (tokens);
}

bool csShadowmapRSLoader::Initialize (iObjectRegistry* object_reg)
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


csPtr<iBase> csShadowmapRSLoader::Parse (iDocumentNode* node, 
				       iStreamSource*,
				       iLoaderContext* ldr_context,      
				       iBase* context)
{
  csShadowmapRenderStep* newstep = 
    new csShadowmapRenderStep (object_reg);
  csRef<iRenderStep> step;
  step.AttachNew (newstep);    

  if (!ParseStep (node, newstep, newstep->GetSettings()))
    return 0;

  if (newstep->GetSettings().shader.IsEmpty() &&
    ((newstep->GetSettings().shadertype == csInvalidStringID)))
  {
    synldr->Report ("di.renderloop.step.rendertotexture",
      CS_REPORTER_SEVERITY_WARNING, node,
      "Neither a shader nor a shadertype was set");
  }

  return csPtr<iBase> (step);
}

bool csShadowmapRSLoader::ParseStep (iDocumentNode* node,
  csShadowmapRenderStep* step, 
  csShadowmapRenderStep::DrawSettings& settings)
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
      case XMLTOKEN_SHADERTYPE:
        {
	  settings.shadertype = 
	    strings->Request (child->GetContentsValue ());
        }
        break;
      case XMLTOKEN_SHADER:
	{
	  settings.shader = child->GetContentsValue ();
	}
	break;
      case XMLTOKEN_DEFAULTSHADER:
	{
	  csRef<iShader> defshader = synldr->ParseShaderRef (child);
	  step->SetDefaultShader(defshader);
	}
	break;
      default:
	if (synldr) synldr->ReportBadToken (child);
	return false;
    }
  }
  return true;
}


//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csShadowmapRenderStepFactory);
  SCF_IMPLEMENTS_INTERFACE(iRenderStepFactory);
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csShadowmapRenderStepFactory::csShadowmapRenderStepFactory (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);
  csShadowmapRenderStepFactory::object_reg = object_reg;
}

csShadowmapRenderStepFactory::~csShadowmapRenderStepFactory ()
{
  SCF_DESTRUCT_IBASE();
}

csPtr<iRenderStep> csShadowmapRenderStepFactory::Create ()
{
  return csPtr<iRenderStep> 
    (new csShadowmapRenderStep (object_reg));
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csShadowmapRenderStep);
  SCF_IMPLEMENTS_INTERFACE(iRenderStep);
  SCF_IMPLEMENTS_INTERFACE(iLightIterRenderStep)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csShadowmapRenderStep::csShadowmapRenderStep (
  iObjectRegistry* object_reg) : r2tVisCallback ()
{
  SCF_CONSTRUCT_IBASE(0);

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.shared.stringset", iStringSet);
  csShadowmapRenderStep::object_reg = object_reg;
  bones_name = strings->Request("bones");
  shader_name = strings->Request("distance_animated");
  depth_cubemap_name = strings->Request("cubemap depth");
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  context = 0;
  defShader = 0;

  settings = DrawSettings();

  r2tVisCallback.parent = this;
  mesh_list = new csRenderMeshList(engine);

	/*
	csRGBpixel *depth_texture_data = new csRGBpixel[CS_DEPTH_TEXTURE_WIDTH * CS_DEPTH_TEXTURE_HEIGHT];
	csRef<iImage> img = csPtr<iImage> (new csImageMemory (
		CS_DEPTH_TEXTURE_WIDTH, CS_DEPTH_TEXTURE_HEIGHT, depth_texture_data, true, 
		CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));
	depth_texture_1 = g3d->GetTextureManager()->RegisterTexture (
		img, CS_TEXTURE_2D | CS_TEXTURE_CLAMP | CS_TEXTURE_NOMIPMAPS);
	*/

  for (size_t i = 0; i < 5 ; i++ )
  {
    static const int depthTextureWidth = 256;
    static const int depthTextureHeight = 256;

    csRef<csImageCubeMapMaker> cubeMaker;
    cubeMaker.AttachNew (new csImageCubeMapMaker ());

    csRef<iImage> cube_side_img;
    cube_side_img.AttachNew (new csImageMemory (
      depthTextureWidth, depthTextureHeight, 
      CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));

    cubeMaker->SetSubImage (0, cube_side_img);
    cubeMaker->SetSubImage (1, cube_side_img);
    cubeMaker->SetSubImage (2, cube_side_img);
    cubeMaker->SetSubImage (3, cube_side_img);
    cubeMaker->SetSubImage (4, cube_side_img);
    cubeMaker->SetSubImage (5, cube_side_img);

    csRef<iTextureHandle> depth_texture = g3d->GetTextureManager ()->RegisterTexture (
      cubeMaker, CS_TEXTURE_2D | CS_TEXTURE_CLAMP | CS_TEXTURE_NOMIPMAPS);

    depth_texture->SetTextureClass("nocompress");
    depth_textures.Push(depth_texture);
  }

  shaderMgr = csQueryRegistry<iShaderManager> (object_reg);
}

csShadowmapRenderStep::~csShadowmapRenderStep ()
{
  SCF_DESTRUCT_IBASE();
  delete mesh_list;
}

void csShadowmapRenderStep::Perform (iRenderView* rview, iSector* sector,
  iShaderVarStack* stacks)
{
  csOrthoTransform old_transform = rview->GetCamera()->GetTransform();;
  context = engine->GetContext();
  g3d->FinishDraw();

  iLightList* lights = sector->GetLights();
  int nlights = lights->GetCount();

  csArray<iLight*> lightList (16);
  int curr_depth_cubemap = 0;

  csRef<iVisibilityCuller> culler = sector->GetVisibilityCuller ();

  while (nlights-- > 0)
  {
    iLight* light = lights->Get (nlights);
    csShaderVariable *sv;
    sv = light->GetSVContext()->GetVariableAdd(depth_cubemap_name);
    sv->SetValue (depth_textures[curr_depth_cubemap]);
    const float lcod = light->GetCutoffDistance();

    for (int j = 0; j < 6 ; j++)
    {
      g3d->SetRenderTarget(depth_textures[curr_depth_cubemap], false, j);

      csOrthoTransform new_transform = light->GetMovable()->GetFullTransform();

      switch (j)
      {
        case 0:
          new_transform.RotateThis(csVector3(0, 1, 0), HALF_PI);
          break;
        case 1:
	  new_transform.RotateThis(csVector3(0, 1, 0), -HALF_PI);
          break;
        case 2:
	  new_transform.RotateThis(csVector3(1, 0, 0), -HALF_PI);
          break;
        case 3:
	  new_transform.RotateThis(csVector3(1, 0, 0), HALF_PI);
          break;
        case 4:
          /* nothing */
          break;
        case 5:
	  new_transform.RotateThis(csVector3(0, 1, 0), -PI);
          break;
      }

      rview->GetCamera()->SetTransform  (new_transform);
      g3d->BeginDraw (CSDRAW_3DGRAPHICS | CSDRAW_CLEARZBUFFER
        | CSDRAW_CLEARSCREEN);

      //csVector3 nll = rview->GetCamera()->GetTransform().This2Other(csVector3(-1, -1, 1));
      //csVector3 nlr = rview->GetCamera()->GetTransform().This2Other(csVector3(1, -1, 1));
      //csVector3 nul = rview->GetCamera()->GetTransform().This2Other(csVector3(-1, 1, 1));
      //csVector3 nur = rview->GetCamera()->GetTransform().This2Other(csVector3(1, 1, 1));

      csVector3 fll = rview->GetCamera()->GetTransform().This2Other (
        csVector3 (-lcod, -lcod, lcod));
      csVector3 flr = rview->GetCamera()->GetTransform().This2Other (
	csVector3 (lcod, -lcod, lcod));
      csVector3 ful = rview->GetCamera()->GetTransform().This2Other (
	csVector3 (-lcod, lcod, lcod));
      csVector3 fur = rview->GetCamera()->GetTransform().This2Other (
	csVector3 (lcod, lcod, lcod));

      csVector3 view_pos = rview->GetCamera()->GetTransform().GetOrigin();
      csPlane3 planes[5];
      planes[0] = csPlane3(view_pos, fur, ful);
      planes[1] = csPlane3(view_pos, flr, fur);
      planes[2] = csPlane3(view_pos, fll, flr);
      planes[3] = csPlane3(view_pos, ful, fll);
      planes[4] = csPlane3(fur, fll, ful);

      lightMeshes.Truncate(0);

      culler->VisTest (planes, 5, &r2tVisCallback);

      mesh_list->Empty();

      for (size_t i = 0; i < lightMeshes.Length() ;i++)
      {
	int num;
	uint32 frustum_mask = 0;
	csRenderMesh** meshes = lightMeshes[i]->GetMeshObject()->GetRenderMeshes 
	  (num, rview, lightMeshes[i]->GetMovable(), frustum_mask);
	CS_ASSERT(!((num != 0) && (meshes == 0)));
	if (num > 0)
	{
  	  mesh_list->AddRenderMeshes (meshes, num, lightMeshes[i]->GetRenderPriority (),
	    lightMeshes[i]->GetZBufMode (), lightMeshes[i]);
        }
      }

      size_t num = mesh_list->SortMeshLists (rview);

      render_meshes.SetLength(num);
      mesh_wrappers.SetLength(num);

      mesh_list->GetSortedMeshes (render_meshes.GetArray(), mesh_wrappers.GetArray());

      for (size_t i = 0; i < num ; i++ )
      {
  	iShader* shader = defShader;
	csRenderMesh *rmesh = render_meshes[i];
	csRenderMeshModes modes = csRenderMeshModes(*rmesh);
	if (rmesh->material)
	{
          iMaterial* mat = rmesh->material->GetMaterial ();
	  iShader* meshShader = mat->GetShader (settings.shadertype);
	  if (meshShader)
	  {
	    shader = meshShader;
	  }
	}

	if (!shader)
	{
	  continue;
	}
	size_t ticket = shader->GetTicket (modes, stacks);
	for (size_t p = 0; p < shader->GetNumberOfPasses (ticket); p ++) 
	{
	  shader->ActivatePass (ticket, p);
	  stacks->Empty ();
	  mesh_wrappers[i]->GetSVContext ()->PushVariables (stacks);
	  rmesh->variablecontext->PushVariables (stacks);
	  shaderMgr->PushVariables (stacks);
	  shader->PushVariables (stacks);
	  g3d->SetWorldToCamera (rview->GetCamera()->GetTransform ().GetInverse ());
	  shader->SetupPass (ticket, rmesh, modes, stacks);
	  g3d->DrawMesh (rmesh, modes, stacks);
	  shader->TeardownPass (ticket);
	  shader->DeactivatePass (ticket);
        }
      }
      g3d->FinishDraw();
    }
    curr_depth_cubemap++;
  }
  rview->GetCamera()->SetTransform(old_transform);
  engine->SetContext(context);
  //rview->GetCamera()->SetFOVAngle(60, 512);
  g3d->BeginDraw (CSDRAW_3DGRAPHICS | CSDRAW_CLEARZBUFFER 
    | CSDRAW_CLEARSCREEN);
}

SCF_IMPLEMENT_IBASE(csShadowmapRenderStep::R2TVisCallback)
SCF_IMPLEMENTS_INTERFACE(iVisibilityCullerListener)
SCF_IMPLEMENT_IBASE_END

csShadowmapRenderStep::R2TVisCallback::R2TVisCallback ()
{
  SCF_CONSTRUCT_IBASE(0);
}

csShadowmapRenderStep::R2TVisCallback::~R2TVisCallback ()
{
  SCF_DESTRUCT_IBASE();
}

void csShadowmapRenderStep::R2TVisCallback::ObjectVisible (
  iVisibilityObject *visobject, iMeshWrapper *mesh, uint32 /*frustum_mask*/)
{
  //printf("adding %s\n", mesh->QueryObject()->GetName());
  parent->lightMeshes.Push (mesh);
}
