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

#include "cstool/fogmath.h"
#include "csgeom/math3d.h"
#include "csutil/flags.h"
#include "csutil/stringquote.h"
#include "csutil/sysfunc.h"
#include "iengine/camera.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/portal.h"
#include "iengine/portalcontainer.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "igeom/clip2d.h"
#include "imesh/objmodel.h"
#include "imesh/object.h"
#include "iutil/document.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/material.h"
#include "ivideo/rendermesh.h"
#include "ivideo/rndbuf.h"

#include "generic.h"

SCF_IMPLEMENT_FACTORY(csGenericRSType)
SCF_IMPLEMENT_FACTORY(csGenericRSLoader)

//---------------------------------------------------------------------------

csGenericRSType::csGenericRSType (iBase* p) :
  scfImplementationType (this, p)
{
}

csPtr<iRenderStepFactory> csGenericRSType::NewFactory()
{
  return csPtr<iRenderStepFactory> 
    (new csGenericRenderStepFactory (object_reg));
}

//---------------------------------------------------------------------------

csGenericRSLoader::csGenericRSLoader (iBase* p) :
  scfImplementationType (this, p)
{
  InitTokenTable (tokens);
}

csPtr<iBase> csGenericRSLoader::Parse (iDocumentNode* node, 
				       iStreamSource*,
				       iLoaderContext* ldr_context,
				       iBase* /*context*/)
{
  csRef<iGenericRenderStep> step;
  step.AttachNew (new csGenericRenderStep (object_reg));

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request (child->GetValue ());
    switch (id)
    {
      case XMLTOKEN_PORTALTRAVERSAL:
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
	break;
      case XMLTOKEN_SHADERTYPE:
	step->SetShaderType (child->GetContentsValue ());
	break;
      case XMLTOKEN_DEFAULTSHADER:
	{
	  csRef<iShader> defshader = synldr->ParseShaderRef (ldr_context,
	      child);
	  step->SetDefaultShader (defshader);
	}
	break;
      case XMLTOKEN_NODEFAULTTRIGGER:
	step->AddDisableDefaultTriggerType (child->GetContentsValue ());
	break;
      default:
	{
	  csZBufMode zmode;
	  if (synldr->ParseZMode (child, zmode, true))
	  {
	    step->SetZBufMode (zmode);
	    break;
	  }
	  synldr->ReportBadToken (child);
	}
	return 0;
    }
  }

  return csPtr<iBase> (step);
}

//---------------------------------------------------------------------------

csGenericRenderStepFactory::csGenericRenderStepFactory (
  iObjectRegistry* object_reg) :
  scfImplementationType (this)
{
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

CS::ShaderVarStringID csGenericRenderStep::fogplane_name;
CS::ShaderVarStringID csGenericRenderStep::string_object2world;
CS::ShaderVarStringID csGenericRenderStep::string_object2worldInv;
CS::ShaderVarStringID csGenericRenderStep::light_ambient;

csGenericRenderStep::csGenericRenderStep (
  iObjectRegistry* object_reg) :
  scfImplementationType (this)
{
  objreg = object_reg;

  strings = csQueryRegistryTagInterface<iStringSet> 
    (object_reg, "crystalspace.shared.stringset");
  stringsSvName = csQueryRegistryTagInterface<iShaderVarStringSet> 
    (object_reg, "crystalspace.shader.variablenameset");
  shaderManager = csQueryRegistry<iShaderManager> (object_reg);

  shadertype = 0;
  zOffset = false;
  portalTraversal = false;
  zmode = CS_ZBUF_USE;
  currentSettings = false;
  fogplane_name = stringsSvName->Request ("fogplane");
  string_object2world = stringsSvName->Request ("object2world transform");
  string_object2worldInv = stringsSvName->Request ("object2world transform inverse");
  light_ambient = stringsSvName->Request ("light ambient");

  visible_meshes_index = 0;
}

csGenericRenderStep::~csGenericRenderStep ()
{
}

/* @@@ Name could prolly be better.
 * This struct really contains the various contexts and data sources that
 * contribute to the shader variables used for rendering.
 */
struct ShaderVarPusher
{
  iShaderVariableContext* sectorContext;
  iLight *light;
  csRenderMesh* mesh;
  iShaderVariableContext* meshContext;
  iShader* shader;

  ShaderVarPusher () : sectorContext (0), light (0), mesh (0), meshContext (0),
    shader (0)
  { }
  void PushVariables (csShaderVariableStack& stack) const
  {
    if (sectorContext)
      sectorContext->PushVariables (stack);
    if (mesh->variablecontext)
      mesh->variablecontext->PushVariables (stack);
    if (meshContext)
      meshContext->PushVariables (stack);
    shader->PushVariables (stack);
    if (mesh->material)
      mesh->material->GetMaterial()->PushVariables (stack);
  }
};

void csGenericRenderStep::RenderMeshes (iRenderView* rview, iGraphics3D* g3d,
                                        const ShaderVarPusher& Pusher,
					size_t ticket,
					meshInfo* meshContexts,
                                        csRenderMesh** meshes, 
                                        size_t num,
                                        csShaderVariableStack& _stack)
{
  if (num == 0) return;
  ToggleStepSettings (g3d, true);
  if (!shaderManager)
  {
    shaderManager = csQueryRegistry<iShaderManager> (objreg);
  }
  csRef<csShaderVariable> svO2W = 
    shadervars.Top ().GetVariable(string_object2world);
  csRef<csShaderVariable> svO2WI = 
    shadervars.Top ().GetVariable(string_object2worldInv);

  ShaderVarPusher pusher (Pusher);
  iShader* shader = pusher.shader;

  bool noclip = false;
  csRef<iClipper2D> old_clipper;
  int old_cliptype = g3d->GetClipType ();
  
  size_t numPasses = shader->GetNumberOfPasses (ticket);
  for (size_t p = 0; p < numPasses; p++)
  {
    shader->ActivatePass (ticket, p);

    size_t j;
    for (j = 0; j < num; j++)
    {
      if (!meshContexts[j].render) continue;
      csRenderMesh* mesh = meshes[j];
      iShaderVariableContext* meshContext = meshContexts[j].svc;
      if (meshContext->IsEmpty())
	meshContext = 0;
      if ((!portalTraversal) && mesh->portal != 0) continue;

      svO2W->SetValue (mesh->object2world);
      svO2WI->SetValue (mesh->object2world.GetInverse());

      pusher.meshContext = meshContext;
      pusher.mesh = mesh;

      csShaderVariableStack stack;
      stack.Setup (shaderManager->GetShaderVariableStack ());
      stack.Copy (_stack);
      shaderManager->PushVariables (stack);
      shadervars.Top ().PushVariables (stack);
      pusher.PushVariables (stack);

      csRenderMeshModes modes (*mesh);
      shader->SetupPass (ticket, mesh, modes, stack);

      if (meshContexts[j].noclip && !noclip)
      {
        // This mesh doesn't want clipping and the clip was not
	// already disabled.
	noclip = true;
	// First search for the top level clipper.
	csRenderContext* ctxt = rview->GetRenderContext ();
	while (ctxt->previous) ctxt = ctxt->previous;
	old_clipper = g3d->GetClipper ();
	old_cliptype = g3d->GetClipType ();
	g3d->SetClipper (ctxt->iview, ctxt->do_clip_frustum ?
		CS_CLIPPER_REQUIRED : CS_CLIPPER_OPTIONAL);
      }
      else if (!meshContexts[j].noclip && noclip)
      {
        // Restore clipper.
	g3d->SetClipper (old_clipper, old_cliptype);
	old_clipper = 0;
      }
      
      g3d->DrawMesh (mesh, modes, stack);
      shader->TeardownPass (ticket);
      
    }
    shader->DeactivatePass (ticket);
  }

  // Restore clipper if needed.
  if (noclip)
  {
    g3d->SetClipper (old_clipper, old_cliptype);
  }

}

void csGenericRenderStep::Perform (iRenderView* rview, iSector* sector,
  csShaderVariableStack& stack)
{
  Perform (rview, sector, 0, stack);
}

void csGenericRenderStep::ToggleStepSettings (iGraphics3D* g3d, 
					      bool settings)
{
  if (settings != currentSettings)
  {
    if (settings)
    {
      g3d->SetZMode (zmode);
    }
    currentSettings = settings;
  }
}

class ShaderTicketHelper
{
private:
  csShaderVariableStack origStack;
  csShaderVariableStack stack;
  const csArray<csShaderVariableContext>& shadervars;
  size_t shadervars_idx;
  //csShaderVariableContext& shadervars;

  iMaterialWrapper* lastMat;
  iShader* lastShader;
  iShaderVariableContext* lastMeshContext;
  iShaderVariableContext* lastSectorContext;
  size_t matShadMeshTicket;

  void Reset ()
  {
    matShadMeshTicket = (size_t)~0;
  }

public:
  ShaderTicketHelper (csShaderVariableStack& _stack,
    const csArray<csShaderVariableContext>& sv,
    size_t sv_idx) : origStack (_stack), stack (_stack), shadervars (sv),
      shadervars_idx (sv_idx),
      lastMat (0), lastShader (0), lastMeshContext (0), lastSectorContext (0)
  {
    Reset ();
    stack.MakeOwnArray();
  }

  size_t GetTicket (const ShaderVarPusher& pusher)
  {
    if ((pusher.mesh->material != lastMat) 
      || (pusher.shader != lastShader)
      || (pusher.meshContext != lastMeshContext)
      || (pusher.sectorContext != lastSectorContext))
    {
      Reset ();
      stack.Copy (origStack);
      lastMat = pusher.mesh->material;
      lastShader = pusher.shader;
      lastMeshContext = pusher.meshContext;
      lastSectorContext = pusher.sectorContext;
    }
    bool materialShaderOnly = !(pusher.mesh->variablecontext.IsValid () 
      && !pusher.mesh->variablecontext->IsEmpty());
    size_t newTicket = matShadMeshTicket;
    if (!materialShaderOnly || (matShadMeshTicket == (size_t)~0))
    {
      shadervars[shadervars_idx].PushVariables (stack);
      pusher.PushVariables (stack);

      csRenderMeshModes modes (*pusher.mesh);
      newTicket = pusher.shader->GetTicket (modes, stack);
    }
    if (materialShaderOnly) matShadMeshTicket = newTicket;
    return newTicket;
  }
};

void csGenericRenderStep::Perform (iRenderView* rview, iSector* sector,
				   iLight* light,
                                   csShaderVariableStack& stack)
{
  iGraphics3D* g3d = rview->GetGraphics3D();

  csRenderMeshList* meshlist = sector->GetVisibleMeshes (rview);
  size_t num = meshlist->SortMeshLists (rview);
  visible_meshes.SetSize (visible_meshes_index+num);
  imeshes_scratch.SetSize (num);
  mesh_info.SetSize (visible_meshes_index+num);
  csRenderMesh** sameShaderMeshes = visible_meshes.GetArray ()
  	+ visible_meshes_index;
  meshInfo* sameShaderMeshInfo = mesh_info.GetArray ()
  	+ visible_meshes_index;
  size_t prev_visible_meshes_index = visible_meshes_index;
  visible_meshes_index += num;
  meshlist->GetSortedMeshes (sameShaderMeshes, imeshes_scratch.GetArray());
  for (size_t i = 0; i < num; i++)
  {
    iMeshWrapper* m = imeshes_scratch[i];
    sameShaderMeshInfo[i].svc = m->GetSVContext();
    sameShaderMeshInfo[i].noclip = m->GetFlags ().Check (CS_ENTITY_NOCLIP);
    sameShaderMeshInfo[i].render = true;
    // Only get this information if we have a light. Otherwise it is not
    // useful.
    if (light)
    {
      iObjectModel* objmodel = m->GetMeshObject ()->GetObjectModel ();
      iMovable* mov = m->GetMovable ();
#if USE_BOX
      sameShaderMeshInfo[i].obj_model = objmodel;
      sameShaderMeshInfo[i].movable = mov;
#else
      csVector3 obj_center;
      objmodel->GetRadius (sameShaderMeshInfo[i].radius, obj_center);
      if (mov->IsFullTransformIdentity ())
      {
        sameShaderMeshInfo[i].wor_center = obj_center;
      }
      else
      {
	csReversibleTransform trans = mov->GetFullTransform ();
        sameShaderMeshInfo[i].wor_center = trans.This2Other (obj_center);
      }
#endif
    }
  }
 
  size_t lastidx = 0;
  size_t numSSM = 0;
  iShader* shader = 0;
  size_t currentTicket = (size_t)~0;

  shadervars.Push (csShaderVariableContext ());
  shadervars.Top ().GetVariableAdd (string_object2world);
  shadervars.Top ().GetVariableAdd (string_object2worldInv);

  csRef<csShaderVariable> sv;
  sv = shadervars.Top ().GetVariableAdd (light_ambient);
  iEngine* engine = rview->GetEngine ();
  csColor ambient;
  engine->GetAmbientLight (ambient);
  sv->SetValue (ambient + sector->GetDynamicAmbientLight());

  if (sector->HasFog())
  {
    //construct a cameraplane
    csVector4 fogPlane;
    iPortal *lastPortal = rview->GetLastPortal();
    if(lastPortal)
    {
      csPlane3 plane;
      lastPortal->ComputeCameraPlane(rview->GetCamera()->GetTransform(), plane);
      fogPlane = plane.norm;
      fogPlane.w = plane.DD;
    }
    else
    {
      fogPlane = csVector4(0.0,0.0,1.0,0.0);
    }
    sv = shadervars.Top ().GetVariableAdd (fogplane_name);
    sv->SetValue (fogPlane);
  }

  ShaderVarPusher pusher;
  pusher.sectorContext = sector->GetSVContext();
  pusher.light = light;
  ShaderTicketHelper ticketHelper (stack, shadervars, shadervars.GetSize ()-1);
  const csReversibleTransform& camt = rview->GetCamera ()->GetTransform ();

  csLightType light_type;
  float cutoff_distance = 0;
  csVector3 light_center;
  if (light)
  {
    light_type = light->GetType ();
    cutoff_distance = light->GetCutoffDistance ();
    light_center = light->GetMovable ()->GetFullPosition ();
  }

  for (size_t n = 0; n < num; n++)
  {
    csRenderMesh* mesh = sameShaderMeshes[n];
    if (light)
    {
      // @@@ TODO: Better test for DIRECTIONAL and SPOTLIGHT
#if USE_BOX
      // We transform the light center to object space and then
      // we test if the transformed light affects the bounding box.
      iMovable* mov = sameShaderMeshInfo[n].movable;
      iObjectModel* obj_model = sameShaderMeshInfo[n].obj_model;
      const csBox3& obj_bbox = obj_model->GetObjectBoundingBox ();
      bool isect;
      if (mov->IsFullTransformIdentity ())
      {
        isect = csIntersect3::BoxSphere (obj_bbox,
	    light_center, cutoff_distance * cutoff_distance);
      }
      else
      {
        csReversibleTransform trans = mov->GetFullTransform ();
        csVector3 obj_light_center = trans.Other2This (light_center);
        isect = csIntersect3::BoxSphere (obj_bbox,
	    obj_light_center, cutoff_distance * cutoff_distance);
      }
      if (!isect)
	sameShaderMeshInfo[n].render = false;
#else
      float dist = sqrt (csSquaredDist::PointPoint (
	sameShaderMeshInfo[n].wor_center, //mesh->worldspace_origin,
      	light_center));
      if (dist-sameShaderMeshInfo[n].radius > cutoff_distance)
	sameShaderMeshInfo[n].render = false;
#endif
    }
    pusher.mesh = mesh;

    if (mesh->portal) 
    {
      if (numSSM > 0)
      {
        if (shader != 0)
	{
          pusher.shader = shader;
          g3d->SetWorldToCamera (camt.GetInverse ());
	  RenderMeshes (rview, g3d, pusher, currentTicket,
	  	sameShaderMeshInfo + lastidx,
		sameShaderMeshes+lastidx, numSSM, stack);
          shader = 0;
	}
        numSSM = 0;
      }

      if (portalTraversal)
      {
        ToggleStepSettings (g3d, false);
        mesh->portal->Draw (rview);
      }

      // Portal traversal can relocate the visible_meshes
      // growing array. So after portal traversal we have to fix
      // the sameShaderMeshes pointer because it may now point
      // to an invalid area.
      sameShaderMeshes = visible_meshes.GetArray () + prev_visible_meshes_index;
      sameShaderMeshInfo = mesh_info.GetArray () + prev_visible_meshes_index;
    }
    else 
    {
#ifdef CS_DEBUG
      if (!mesh->material)
      {
	csPrintfErr ("INTERNAL ERROR: mesh %s is missing a material!\n",
	  CS::Quote::Single (mesh->db_mesh_name));
	exit (-1);
      }
#endif
      iMaterial* hdl = mesh->material->GetMaterial ();
#ifdef CS_DEBUG
      if (!hdl)
      {
        csPrintfErr ("INTERNAL ERROR: mesh %s is missing a material!\n",
	  CS::Quote::Single (mesh->db_mesh_name));
	exit (-1);
      }
#endif
      iShader* meshShader = hdl->GetShader (shadertype);
      if (meshShader == 0) 
      {
	bool doDefault = true;
	for (size_t i = 0; i < disableDefaultTypes.GetSize (); i++)
	{
	  if (hdl->GetShader (disableDefaultTypes[i]) != 0)
	  {
	    doDefault = false;
	    break;
	  }
	}
	if (doDefault) meshShader = defShader;
      }
      pusher.shader = meshShader;
      pusher.mesh = mesh;
      pusher.meshContext = sameShaderMeshInfo[n].svc;
      size_t newTicket = meshShader ? ticketHelper.GetTicket (pusher) : (size_t)~0;
      if ((meshShader != shader) || (newTicket != currentTicket))
      {
        pusher.shader = shader;
        // @@@ Need error reporter
        if (shader != 0)
	{
          g3d->SetWorldToCamera (camt.GetInverse ());
          RenderMeshes (rview, g3d, pusher, currentTicket,
	  	sameShaderMeshInfo + lastidx, 
		sameShaderMeshes + lastidx, numSSM, stack);
	}
	lastidx = n;
        shader = meshShader;
	currentTicket = newTicket;
        numSSM = 0;
      }
      numSSM++;
    }
  }
  
  if (numSSM != 0)
  {
    // @@@ Need error reporter
    pusher.shader = shader;
    if (shader != 0)
    {
      g3d->SetWorldToCamera (camt.GetInverse ());
      RenderMeshes (rview, g3d, pusher, currentTicket,
      	sameShaderMeshInfo + lastidx,
        sameShaderMeshes + lastidx, numSSM, stack);
    }
  }

  shadervars.Pop ();

  ToggleStepSettings (g3d, false);

  visible_meshes_index = prev_visible_meshes_index;
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

bool csGenericRenderStep::GetZOffset () const
{
  return zOffset;
}

void csGenericRenderStep::SetZBufMode (csZBufMode zmode)
{
  csGenericRenderStep::zmode = zmode;
}

csZBufMode csGenericRenderStep::GetZBufMode () const
{
  return zmode;
}

void csGenericRenderStep::AddDisableDefaultTriggerType (const char* type)
{
  csStringID shadertype = strings->Request (type);
  if (shadertype == csInvalidStringID) return;
  disableDefaultTypes.Push (shadertype);
}

void csGenericRenderStep::RemoveDisableDefaultTriggerType (const char* type)
{
  csStringID shadertype = strings->Request (type);
  if (shadertype == csInvalidStringID) return;
  disableDefaultTypes.Delete (shadertype);
}
