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
#include "csqsqrt.h"
#include "csqint.h"

#include "csgeom/math.h"
#include "csgeom/math3d.h"
#include "csgeom/poly2d.h"
#include "csgeom/poly3d.h"
#include "csgeom/polyclip.h"
#include "csgeom/sphere.h"
#include "csgfx/memimage.h"
#include "csutil/bitarray.h"
#include "csutil/cscolor.h"
#include "csutil/flags.h"
#include "csutil/sysfunc.h"
#include "cstool/rendermeshlist.h"

#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/lightmgr.h"
#include "iengine/material.h"
#include "iengine/movable.h"
#include "iengine/portal.h"
#include "iengine/portalcontainer.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iutil/document.h"
#include "ivideo/material.h"
#include "ivideo/rendermesh.h"
#include "ivideo/txtmgr.h"

#include "fatloop.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csFatLoopType)
SCF_IMPLEMENT_FACTORY(csFatLoopLoader)

static const char* MessageID = "crystalspace.renderloop.step.fatloop";

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
	break;
      case XMLTOKEN_SHADERTYPE:
	step->shadertype = strings->Request (child->GetContentsValue ());
	break;
      case XMLTOKEN_DEFAULTSHADER:
	{
	  step->defShader = synldr->ParseShaderRef (child);
	}
	break;
      case XMLTOKEN_NODEFAULTTRIGGER:
	step->AddDisableDefaultTriggerType (child->GetContentsValue ());
	break;*/
      case XMLTOKEN_PASS:
        {
          RenderPass pass;
          if (!ParsePass (child, pass))
            return 0;
          step->AddPass (pass);
        }
        break;
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

bool csFatLoopLoader::ParsePass (iDocumentNode* node, RenderPass& pass)
{
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.shared.stringset", iStringSet);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request (child->GetValue ());
    switch (id)
    {
      case XMLTOKEN_SHADERTYPE:
	pass.shadertype = strings->Request (child->GetContentsValue ());
	break;
      case XMLTOKEN_DEFAULTSHADER:
        pass.defShader = synldr->ParseShaderRef (child);
	break;
      case XMLTOKEN_MAXLIGHTS:
	pass.maxLights = child->GetContentsValueAsInt ();
	break;
      case XMLTOKEN_MAXPASSES:
	pass.maxPasses = child->GetContentsValueAsInt ();
	break;
      case XMLTOKEN_BASEPASS:
	if (!synldr->ParseBool (child, pass.basepass, true))
	  return false;
	break;
      case XMLTOKEN_ZOFFSET:
	if (!synldr->ParseBool (child, pass.zoffset, true))
	  return false;
	break;
      default:
	{
	  synldr->ReportBadToken (child);
	}
	return false;
    }
  }

  if (pass.shadertype == csInvalidStringID)
  {
    synldr->ReportError (MessageID, 
      node, "No 'shadertype' specified in pass");
    return false;
  }

  return true;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csFatLoopFactory);
  SCF_IMPLEMENTS_INTERFACE(iRenderStepFactory);
SCF_IMPLEMENT_IBASE_END

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
SCF_IMPLEMENT_IBASE_END

csFatLoopStep::csFatLoopStep (iObjectRegistry* object_reg) :
  /*buckets(2, 2), */passes(2, 2), meshNodeFact(object_reg), 
  portalNodeFact(object_reg)
{
  SCF_CONSTRUCT_IBASE(0);
  this->object_reg = object_reg;

  shaderManager = CS_QUERY_REGISTRY (object_reg, iShaderManager);
  nullShader = shaderManager->GetShader ("*null");
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  lightmgr = CS_QUERY_REGISTRY (object_reg, iLightManager);

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg,
    "crystalspace.shared.stringset", iStringSet);
  fogplane_name = strings->Request ("fogplane");
  fogdensity_name = strings->Request ("fog density");
  fogcolor_name = strings->Request ("fog color");

  lsvCache.SetStrings (strings);
}

csFatLoopStep::~csFatLoopStep ()
{
  SCF_DESTRUCT_IBASE();
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
    if ((knownPrios.GetSize() <= (size_t)priority) || 
      (!knownPrios.IsBitSet (priority)))
    {
      if (knownPrios.GetSize() <= (size_t)priority) 
      {
        knownPrios.SetSize (priority + 1);
        prioSorted.SetSize (priority + 1);
      }
      prioSorted.Set (priority, 
        engine->GetRenderPrioritySorting (priority) != CS_RENDPRI_SORT_NONE);
      knownPrios.Set (priority);
    }
    return prioSorted.IsBitSet (priority);
  }
};

void csFatLoopStep::Perform (iRenderView* rview, iSector* sector,
                             csShaderVarStack &stacks)
{
  shadervars.Clear();
  RenderNode* node = renderNodeAlloc.Alloc();
  BuildNodeGraph (node, rview, sector);
  ProcessNode (rview, node);
  renderNodeAlloc.Empty();
}

void csFatLoopStep::SetupFog (RenderNode* node)
{
  csRef<csShaderVariable> sv;
  sv = shadervars.GetVariableAdd (fogdensity_name);
  sv->SetValue (node->fog.density);
  sv = shadervars.GetVariableAdd (fogcolor_name);
  sv->SetValue (node->fog.color);
  sv = shadervars.GetVariableAdd (fogplane_name);
  sv->SetValue (node->fog.plane);
}

void csFatLoopStep::CleanEmptyMeshNodes (RenderNode* node, 
  const csArray<csMeshRenderNode*>& meshNodes)
{
  size_t nodeContainedIdx = node->containedNodes.GetSize() - meshNodes.GetSize();
  size_t meshNodeIdx = 0;
  while (nodeContainedIdx < node->containedNodes.GetSize())
  {
    if (meshNodes[meshNodeIdx]->HasMeshes())
      nodeContainedIdx++;
    else
    {
      renderNodeAlloc.Free (node->containedNodes[nodeContainedIdx]);
      node->containedNodes.DeleteIndex (nodeContainedIdx);
    }
    meshNodeIdx++;
  }
}

void csFatLoopStep::BuildNodeGraph (RenderNode* node, iRenderView* rview, 
                                    iSector* sector)
{
  if (!sector) return;

  rview->SetupClipPlanes ();
  rview->SetThisSector (sector);
  sector->IncRecLevel();
  sector->PrepareDraw (rview);

  RenderNode* newNode = renderNodeAlloc.Alloc();
  node->containedNodes.Push (newNode);

  if (sector->HasFog())
  {
    csFog* fog = sector->GetFog();
    node->fog.density = fog->density;
    node->fog.color.Set (fog->red, fog->green, fog->blue);

    //construct a cameraplane
    iPortal *lastPortal = rview->GetLastPortal();
    if(lastPortal)
    {
      csPlane3 plane;
      lastPortal->ComputeCameraPlane(rview->GetCamera()->GetTransform(), plane);
      node->fog.plane = plane.norm;
      node->fog.plane.w = plane.DD;
    }
    else
    {
      node->fog.plane.Set (0.0,0.0,1.0,0.0);
    }
  }
  else
  {
    node->fog.density = 0;
  }
  SetupFog (node);

  // Here go the render nodes we'll actually process
  csArray<csMeshRenderNode*> meshNodes;
  for (size_t p = 0; p < passes.GetSize(); p++)
  {
    meshNodes.Push (meshNodeFact.CreateMeshNode (passes[p].shadertype, 
      passes[p].defShader, shadervars, passes[p].zoffset));
    RenderNode* newNode = renderNodeAlloc.Alloc();
    newNode->renderNode = meshNodes[p];
    newNode->fog = node->fog;
    node->containedNodes.Push (newNode);
  }

  // This is a growing array of visible meshes. It will contain
  // the visible meshes from every recursion level appended. At
  // exit of this step the visible meshes from the current recursion
  // level are removed again.
  csDirtyAccessArray<csRenderMesh*> visible_meshes;
  csDirtyAccessArray<iMeshWrapper*> imeshes_scratch;

  csRenderMeshList* meshlist = sector->GetVisibleMeshes (rview);
  size_t num = meshlist->SortMeshLists (rview);
  visible_meshes.SetSize (num);
  imeshes_scratch.SetSize (num);
  csRenderMesh** sameShaderMeshes = visible_meshes.GetArray ();
  meshlist->GetSortedMeshes (sameShaderMeshes, imeshes_scratch.GetArray());

  uint framenr = rview->GetCurrentFrameNumber();

  csReversibleTransform camTransR = rview->GetCamera()->GetTransform();

  PriorityHelper ph (engine);
  for (size_t n = 0; n < num; n++)
  {
    csRenderMesh* mesh = sameShaderMeshes[n];
    if (mesh->portal) 
    {
      CleanEmptyMeshNodes (node, meshNodes);
      BuildPortalNodes (node, imeshes_scratch[n], mesh->portal, rview);
      SetupFog (node);

      for (size_t p = 0; p < passes.GetSize(); p++)
      {
        meshNodes[p] = meshNodeFact.CreateMeshNode (passes[p].shadertype, 
	  passes[p].defShader, shadervars, passes[p].zoffset);
        RenderNode* newNode = renderNodeAlloc.Alloc();
        newNode->renderNode = meshNodes[p];
	newNode->fog = node->fog;
        node->containedNodes.Push (newNode);
      }
      continue;
    }

    iMeshWrapper* mw = imeshes_scratch[n];
    long prio = mw->GetRenderPriority();

    // @@@ FIXME: only fetch when needed
    const csArray<iLight*>& relevantLights = 
      lightmgr->GetRelevantLights (mw, -1, true);
    size_t lightOfs = 0;

    size_t c = 0;
    while (c < passes.GetSize())
    {
      csMeshRenderNode* meshNode = meshNodes[c];
      const RenderPass& pass = passes[c++];
      size_t i;
      int lightCount = 0;
      int passCount = 0;

      iShader* shader;

      // Grab the shader from the material...
      iMaterial* hdl = mesh->material->GetMaterial ();
      shader = hdl->GetShader (pass.shadertype);
      if (shader == 0) shader = pass.defShader;
      if ((shader == 0) || (shader == nullShader))
	continue;

      // Render passes untik the limit is hit
      while (passCount < pass.maxPasses)
      {
	passCount++;
	if (!pass.basepass 
	  && lightOfs >= relevantLights.GetSize())
	  break;
	// Set up SV with light count
	csRef<csShaderVariable> sv;
	sv = GetFrameUniqueSV (framenr, shadervars, 
	  lsvCache.GetDefaultSVId (csLightShaderVarCache::varLightCount));
	shadervars.ReplaceVariable (sv);
	sv->SetValue ((int)(relevantLights.GetSize() - lightOfs));

	// ...and fill light SVs.
	for (i = lightOfs; i < relevantLights.GetSize(); i++)
	{
	  if (((int)lightOfs + lightCount) >= pass.maxLights) break;
	  lightCount++;
	  SetLightSVs (shadervars, relevantLights[i], i-lightOfs, 
	    camTransR, framenr);
	}

	// feed light SVs to shader
	csShaderVarStack stacks;
	FillStacks (stacks, mesh, mw, hdl, shader);

	csRenderMeshModes modes (*mesh);
	size_t ticket = shader->GetTicket (modes, stacks);

	// query max light number the shader groks
	const csShaderMetadata& shaderMeta = shader->GetMetadata (ticket);
	int n = csMin ((int)shaderMeta.numberOfLights, 
	  lightCount);

	// Set actual count
	sv->SetValue (n);

	// light SVs will still be in shadervars
	FillStacks (stacks, mesh, mw, hdl, shader);

	// add mesh
	meshNode->AddMesh (mesh, shader, stacks, prio, 
	  ph.IsPrioSpecial (prio), ticket);

	lightOfs += n;

	// Prevent infinite loops
	if (pass.basepass
	  || (pass.maxLights == 0)) break;
      }
    }
  }
  CleanEmptyMeshNodes (node, meshNodes);
  sector->DecRecLevel();
}

csPtr<iTextureHandle> csFatLoopStep::GetAttenuationTexture (
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
    csRef<iGraphics3D> g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
    attTex = g3d->GetTextureManager()->RegisterTexture (
	img, CS_TEXTURE_3D | CS_TEXTURE_CLAMP | CS_TEXTURE_NOMIPMAPS);
    attTex->SetTextureClass ("lookup");
  }
  return csPtr<iTextureHandle> (attTex);
}

void csFatLoopStep::FillStacks (csShaderVarStack& stacks, 
                                csRenderMesh* rm, iMeshWrapper* mw, 
                                iMaterial* hdl, iShader* shader)
{
  iShaderVariableContext* svc = mw->GetSVContext();
  if (svc->IsEmpty())
    svc = 0;

  stacks.Empty();
  shaderManager->PushVariables (stacks);
  shadervars.PushVariables (stacks);
  if (rm->variablecontext)
    rm->variablecontext->PushVariables (stacks);
  if (svc)
    svc->PushVariables (stacks);
  shader->PushVariables (stacks);
  hdl->PushVariables (stacks);
}

void csFatLoopStep::SetLightSVs (csShaderVariableContext& shadervars, 
				 iLight* light, size_t lightId, 
				 const csReversibleTransform& camTransR,
				 uint framenr)
{
  csRef<csShaderVariable> sv;
  sv = GetFrameUniqueSV (framenr, shadervars, 
    lsvCache.GetLightSVId (lightId, 
      csLightShaderVarCache::lightDiffuse));
  const csColor& color = light->GetColor ();
  sv->SetValue (csVector3 (color.red, color.green, color.blue));

  const csVector3 lightPos = light->GetCenter ();
  sv = GetFrameUniqueSV (framenr, shadervars, 
    lsvCache.GetLightSVId (lightId, 
      csLightShaderVarCache::lightPosition));
  sv->SetValue (lightPos * camTransR);

  sv = GetFrameUniqueSV (framenr, shadervars, 
    lsvCache.GetLightSVId (lightId, 
      csLightShaderVarCache::lightAttenuation));
  csLightAttenuationMode attnMode = light->GetAttenuationMode ();
  if (attnMode == CS_ATTN_LINEAR)
  {
    float r = light->GetAttenuationConstants ().x;
    sv->SetValue (csVector3(r, 1/r, 0));
  }
  else
  {
    sv->SetValue (light->GetAttenuationConstants ());
  }

  sv = GetFrameUniqueSV (framenr, shadervars, 
    lsvCache.GetLightSVId (lightId, 
      csLightShaderVarCache::lightAttenuationMode));
  sv->SetValue ((int)attnMode);

  sv = GetFrameUniqueSV (framenr, shadervars, 
    lsvCache.GetLightSVId (lightId, 
      csLightShaderVarCache::lightType));
  sv->SetValue ((int)light->GetType());

  sv = GetFrameUniqueSV (framenr, shadervars, 
    lsvCache.GetLightSVId (lightId, 
      csLightShaderVarCache::lightAttenuationTex));
  //sv->SetAccessor (GetLightAccessor (light));
  if (!attTex.IsValid())
    attTex = GetAttenuationTexture (attnMode);
  sv->SetValue (attTex);
}

csRef<csShaderVariable> csFatLoopStep::GetFrameUniqueSV (uint framenr,
  csShaderVariableContext& shadervars, csStringID name)
{
  csRef<csShaderVariable> sv = svHolder.GetFrameUniqueSV (framenr, name);
  shadervars.ReplaceVariable (sv);
  return sv;
}

void csFatLoopStep::BuildPortalNodes (RenderNode* node, 
  iMeshWrapper* meshwrapper, iPortalContainer* portals, iRenderView* rview)
{
  int clip_plane, clip_portal, clip_z_plane;

  iCamera* camera = rview->GetCamera ();
  const csReversibleTransform& camtrans = camera->GetTransform ();
  iMovable* movable = meshwrapper->GetMovable();
  const csReversibleTransform movtrans = movable->GetFullTransform ();

  csSphere sphere, world_sphere;
  csBox3 object_bbox;
  meshwrapper->GetWorldBoundingBox (object_bbox);
  float max_object_radius = csQsqrt (csSquaredDist::PointPoint (
  	object_bbox.Max (), object_bbox.Min ())) * 0.5f;
  sphere.SetCenter (object_bbox.GetCenter ());
  sphere.SetRadius (max_object_radius);

  csReversibleTransform tr_o2c = camtrans;
  if (!meshwrapper->GetMovable()->IsFullTransformIdentity())
  {
    tr_o2c /= movtrans;
    world_sphere = movtrans.This2Other (sphere);
  }
  else
  {
    world_sphere = sphere;
  }
  csSphere cam_sphere = tr_o2c.Other2This (sphere);
  csVector3 camera_origin = cam_sphere.GetCenter ();

  if (!rview->ClipBSphere (cam_sphere, world_sphere, clip_portal,
      clip_plane, clip_z_plane)) return;

  bool doClip = clip_plane || clip_portal || clip_z_plane;
  for (int i = 0 ; i < portals->GetPortalCount(); i++)
  {
    iPortal* portal = portals->GetPortal (i);
    csPortalRenderNode* portalNode = 
      portalNodeFact.CreatePortalNode (portal, rview, movtrans, doClip, 
      shadervars);
    if (portalNode == 0) continue;

    RenderNode* newNode = renderNodeAlloc.Alloc();
    newNode->renderNode = portalNode;

    if (portalNode->PreMeshCollect (rview))
    {
      BuildNodeGraph (newNode, rview, portal->GetSector());
      portalNode->PostMeshCollect (rview);
    }

    if (newNode->containedNodes.GetSize() == 0)
      renderNodeAlloc.Free (newNode);
    else
      node->containedNodes.Push (newNode);
  }
}

void csFatLoopStep::ProcessNode (iRenderView* rview, RenderNode* node)
{
  SetupFog (node);
  if ((!node->renderNode) || node->renderNode->Preprocess (rview))
  {
    for (size_t i = 0; i < node->containedNodes.GetSize(); i++)
    {
      ProcessNode (rview, node->containedNodes[i]);
      SetupFog (node);
    }
    if (node->renderNode) node->renderNode->Postprocess (rview);
  }
}

