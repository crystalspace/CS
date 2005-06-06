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
#include "iengine/movable.h"
#include "iengine/portal.h"
#include "iengine/portalcontainer.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iutil/document.h"
#include "ivideo/material.h"
#include "ivideo/rendermesh.h"

#include "csgeom/math3d.h"
#include "csgeom/poly2d.h"
#include "csgeom/poly3d.h"
#include "csgeom/polyclip.h"
#include "csutil/bitarray.h"
#include "csutil/flags.h"
#include "csutil/sysfunc.h"
#include "cstool/rendermeshlist.h"

#include "fatloop.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csFatLoopType);
SCF_IMPLEMENT_FACTORY(csFatLoopLoader);

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

csFatLoopStep::csFatLoopStep (iObjectRegistry* object_reg) :
  /*buckets(2, 2), */passes(2, 2), meshNodeFact(object_reg), 
  portalNodeFact(object_reg)
{
  SCF_CONSTRUCT_IBASE(0);
  this->object_reg = object_reg;

  shaderManager = CS_QUERY_REGISTRY (object_reg, iShaderManager);
  nullShader = shaderManager->GetShader ("*null");
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
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
  sectorSet.Empty();

  RenderNode* node = renderNodeAlloc.Alloc();
  BuildNodeGraph (node, rview, sector, stacks);
  ProcessNode (rview, node, stacks);
  renderNodeAlloc.Empty();
}

uint32 csFatLoopStep::Classify (csRenderMesh* /*mesh*/)
{
  // @@@ Not very distinguishing atm ... we'll see if it's really needed.
  return (1 << passes.Length())-1;
}

void csFatLoopStep::BuildNodeGraph (RenderNode* node, iRenderView* rview, 
                                    iSector* sector, csShaderVarStack &stacks)
{
  if (!sector) return;
  if (sectorSet.In (sector)) return;
  sectorSet.Add (sector);

  RenderNode* newNode = renderNodeAlloc.Alloc();
  node->containedNodes.Push (newNode);

  csArray<csMeshRenderNode*> meshNodes;
  for (size_t p = 0; p < passes.Length(); p++)
  {
    meshNodes.Push (meshNodeFact.CreateMeshNode (passes[p].shadertype, 
      passes[p].defShader));
    RenderNode* newNode = renderNodeAlloc.Alloc();
    newNode->renderNode = meshNodes[p];
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
  visible_meshes.SetLength (num);
  imeshes_scratch.SetLength (num);
  csRenderMesh** sameShaderMeshes = visible_meshes.GetArray ();
  meshlist->GetSortedMeshes (sameShaderMeshes, imeshes_scratch.GetArray());

  PriorityHelper ph (engine);
  for (size_t n = 0; n < num; n++)
  {
    csRenderMesh* mesh = sameShaderMeshes[n];
    if (mesh->portal) 
    {
      BuildPortalNodes (node, imeshes_scratch[n], mesh->portal, rview, stacks);

      for (size_t p = 0; p < passes.Length(); p++)
      {
        meshNodes[p] = meshNodeFact.CreateMeshNode (passes[p].shadertype, 
          passes[p].defShader);
        RenderNode* newNode = renderNodeAlloc.Alloc();
        newNode->renderNode = meshNodes[p];
        node->containedNodes.Push (newNode);
      }
      continue;
    }

    iMeshWrapper* mw = imeshes_scratch[n];
    long prio = mw->GetRenderPriority();

    uint32 classes = Classify (mesh);
    int c = 0;
    while (classes != 0)
    {
      if (classes & 1)
      {
        meshNodes[c]->AddMesh (mesh, mw, prio, ph.IsPrioSpecial (prio));
      }
      c++;
      classes >>= 1;
    }
  }
}

static void Perspective (const csVector3& v, csVector2& p, int
	aspect, float shift_x, float shift_y)
{
  float iz = aspect / v.z;
  p.x = v.x * iz + shift_x;
  p.y = v.y * iz + shift_y;
}

static void AddPerspective (csPoly2D* dest, const csVector3 &v,
	int aspect, float shift_x, float shift_y)
{
  csVector2 p;
  Perspective (v, p, aspect, shift_x, shift_y);
  dest->AddVertex (p);
}

void csFatLoopStep::BuildPortalNodes (RenderNode* node, 
  iMeshWrapper* meshwrapper, iPortalContainer* portals, iRenderView* rview, 
  csShaderVarStack &stacks)
{
  const csReversibleTransform& movtrans = 
    meshwrapper->GetMovable()->GetFullTransform ();

  for (int i = 0 ; i < portals->GetPortalCount(); i++)
  {
    iPortal* portal = portals->GetPortal (i);
    csPortalRenderNode* portalNode = 
      portalNodeFact.CreatePortalNode (portal, rview, movtrans);
    if (portalNode == 0) continue;

    RenderNode* newNode = renderNodeAlloc.Alloc();
    newNode->renderNode = portalNode;
    node->containedNodes.Push (newNode);

    BuildNodeGraph (node, rview, portal->GetSector(), stacks);
  }
}

void csFatLoopStep::ProcessNode (iRenderView* rview, RenderNode* node,
                                 csShaderVarStack &stacks)
{
  if ((!node->renderNode) || node->renderNode->Preprocess (rview))
  {
    for (size_t i = 0; i < node->containedNodes.Length(); i++)
    {
      ProcessNode (rview, node->containedNodes[i], stacks);
    }
    if (node->renderNode) node->renderNode->Postprocess (rview);
  }
  return;
}

