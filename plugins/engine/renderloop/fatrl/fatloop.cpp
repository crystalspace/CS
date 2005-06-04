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
  /*buckets(2, 2), */passes(2, 2), meshNodeFact(object_reg) 
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
  node->nodeType = RenderNode::Container;
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
  newNode->nodeType = RenderNode::Container;
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
      /*BuildPortalNodes (node, imeshes_scratch[n], mesh->portal, rview, stacks);

      RenderNode* newNode = renderNodeAlloc.Alloc();
      newNode->nodeType = RenderNode::Meshes;
      node->containedNodes.Push (newNode);*/
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
  // We assume here that ObjectToWorld has already been called.
  iCamera* camera = rview->GetCamera ();
  const csReversibleTransform& camtrans = camera->GetTransform ();

  // Setup clip and far plane.
  csRenderContext* rcontext = rview->GetRenderContext();
  bool do_portal_plane = rcontext->do_clip_plane;
  csPlane3 portal_plane, *pportal_plane;
  if (do_portal_plane)
    pportal_plane = &rcontext->clip_plane;
  else
    pportal_plane = 0;

  csPlane3 *farplane = camera->GetFarPlane ();
  bool mirrored = camera->IsMirrored ();
  int fov = camera->GetFOV ();
  float shift_x = camera->GetShiftX ();
  float shift_y = camera->GetShiftY ();

  const csReversibleTransform movtrans = 
    meshwrapper->GetMovable()->GetFullTransform ();
  
  int i;
  if (/*clip_plane || clip_portal || clip_z_plane || */do_portal_plane || farplane)
  {
    for (i = 0 ; i < portals->GetPortalCount(); i++)
    {
      iPortal* portal = portals->GetPortal (i);

      camera_vertices.SetLength (portal->GetVertexIndicesCount());
      const int* vertexIndices = portal->GetVertexIndices();
      const csVector3* world_vertices = portal->GetWorldVertices();
      size_t i;
      for (i = 0 ; i < camera_vertices.Length () ; i++)
        camera_vertices[i] = camtrans.Other2This (world_vertices[vertexIndices[i]]);
      //camera_planes.Empty ();
      /*for (i = 0 ; i < portals.Length () ; i++)
      {
        csPortal* prt = portals[i];
        csPlane3 p;
        csVector3& cam_vec = camera_vertices[portals[i]->GetVertexIndices ()[0]];
        camtrans.Other2This (prt->GetIntWorldPlane (), cam_vec, p);
        p.Normalize ();
        camera_planes.Push (p);
      }*/
      csPlane3 camera_plane;
      csVector3& cam_vec = camera_vertices[0];
      camtrans.Other2This (portal->GetWorldPlane (), cam_vec, camera_plane);
      camera_plane.Normalize ();
      csVector3 *verts;
      int num_verts;
      if (ClipToPlane (portal, pportal_plane, camtrans.GetOrigin (),
      	verts, num_verts))
      {
        // The far plane is defined negative. So if the portal is entirely
        // in front of the far plane it is not visible. Otherwise we will render
        // it.
        if (!farplane ||
          csPoly3D::Classify (*farplane, verts, num_verts) != CS_POL_FRONT)
        {
	  csPoly2D clip;
	  if (DoPerspective (verts, num_verts, &clip, mirrored, fov,
	  	shift_x, shift_y, camera_planes[i]) &&
	      clip.ClipAgainst (rview->GetClipper ()))
	  {
	    /*DrawOnePortal (portals[i], clip,
	      movtrans, rview, camera_planes[i]);*/
            DoPortal (node, portal, clip, movtrans, camera_plane, rview, stacks);
	  }
        }
      }
    }
  }
  else
  {
    for (i = 0 ; i < portals->GetPortalCount(); i++)
    {
      iPortal* portal = portals->GetPortal (i);
      csPoly2D poly;
      const int* vertexIndices = portal->GetVertexIndices();
      int num_vertices = portal->GetVertexIndicesCount();

      camera_vertices.SetLength (portal->GetVertexIndicesCount());
      const csVector3* world_vertices = portal->GetWorldVertices();
      size_t i;
      for (i = 0 ; i < camera_vertices.Length () ; i++)
        camera_vertices[i] = camtrans.Other2This (world_vertices[vertexIndices[i]]);

      csPlane3 camera_plane;
      csVector3& cam_vec = camera_vertices[0];
      camtrans.Other2This (portal->GetWorldPlane (), cam_vec, camera_plane);
      camera_plane.Normalize ();

      int j;
      for (j = 0 ; j < num_vertices ; j++)
        AddPerspective (&poly, camera_vertices[j], fov, shift_x, shift_y);
      //DrawOnePortal (portals[i], poly, movtrans, rview, camera_planes[i]);
      DoPortal (node, portal, poly, movtrans, camera_plane, rview, stacks);
    }
  }
}

void csFatLoopStep::DoPortal (RenderNode* node, iPortal* portal, 
                              const csPoly2D& poly,
                              const csReversibleTransform& movtrans, 
                              const csPlane3& camera_plane, 
                              iRenderView* rview, csShaderVarStack &stacks)
{
  RenderNode* newNode = renderNodeAlloc.Alloc();
  newNode->nodeType = RenderNode::Portal;
  newNode->portal = portal;
  newNode->poly = poly;
  newNode->movtrans = movtrans;
  newNode->camera_plane = camera_plane;
  node->containedNodes.Push (newNode);

  BuildNodeGraph (node, rview, portal->GetSector(), stacks);
}

bool csFatLoopStep::ClipToPlane (
  iPortal* portal,
  csPlane3 *portal_plane,
  const csVector3 &v_w2c,
  csVector3 * &pverts,
  int &num_verts)
{
  int i, i1;
  int cnt_vis, num_vertices;
  float r;
  bool zs, z1s;

  // Assume maximum 100 vertices! (@@@ HARDCODED LIMIT)
  static csVector3 verts[100];
  static bool vis[100];

  // Count the number of visible vertices for this polygon (note
  // that the transformation from world to camera space for all the
  // vertices has been done earlier).
  // If there are no visible vertices this polygon need not be drawn.
  cnt_vis = 0;
  int* vt = portal->GetVertexIndices ();
  num_vertices = portal->GetVertexIndicesCount();
  for (i = 0; i < num_vertices; i++)
    if (camera_vertices[vt[i]].z >= 0)
    {
      cnt_vis++;
      break;
    }

  if (cnt_vis == 0) return false;

  // Perform backface culling.
  // Note! The plane normal needs to be correctly calculated for this
  // to work!
  const csPlane3 &wplane = portal->GetWorldPlane ();
  float cl = wplane.Classify (v_w2c);
  if (cl > 0) return false;

  // If there is no portal polygon then everything is ok.
  if (!portal_plane)
  {
    // Copy the vertices to verts.
    for (i = 0; i < num_vertices; i++)
      verts[i] = camera_vertices[vt[i]];
    pverts = verts;
    num_verts = num_vertices;
    return true;
  }

  // Otherwise we will have to clip this polygon in 3D against the
  // portal polygon. This is to make sure that objects behind the
  // portal polygon are not accidently rendered.
  // First count how many vertices are before the portal polygon
  // (so are visible as seen from the portal).
  cnt_vis = 0;
  for (i = 0; i < num_vertices; i++)
  {
    vis[i] = portal_plane->Classify (camera_vertices[vt[i]]) <= EPSILON;
    if (vis[i]) cnt_vis++;
  }

  if (cnt_vis == 0) return false; // Polygon is not visible.

  // Copy the vertices to verts.
  for (i = 0; i < num_vertices; i++) verts[i] = camera_vertices[vt[i]];
  pverts = verts;

  // If all vertices are visible then everything is ok.
  if (cnt_vis == num_vertices)
  {
    num_verts = num_vertices;
    return true;
  }

  // We really need to clip.
  num_verts = 0;

  i1 = num_vertices - 1;
  z1s = vis[i1];
  for (i = 0; i < num_vertices; i++)
  {
    zs = vis[i];

    if (!z1s && zs)
    {
      csIntersect3::SegmentPlane (
          camera_vertices[vt[i1]],
          camera_vertices[vt[i]],
          *portal_plane,
          verts[num_verts],
          r);
      num_verts++;
      verts[num_verts++] = camera_vertices[vt[i]];
    }
    else if (z1s && !zs)
    {
      csIntersect3::SegmentPlane (
          camera_vertices[vt[i1]],
          camera_vertices[vt[i]],
          *portal_plane,
          verts[num_verts],
          r);
      num_verts++;
    }
    else if (z1s && zs)
    {
      verts[num_verts++] = camera_vertices[vt[i]];
    }

    z1s = zs;
    i1 = i;
  }

  return true;
}

#define EXPERIMENTAL_BUG_FIX  1

bool csFatLoopStep::DoPerspective (csVector3 *source, int num_verts, 
                                   csPoly2D *dest, bool mirror,
                                   int fov, float shift_x, float shift_y,
                                   const csPlane3& plane_cam)
{
  csVector3 *ind, *end = source + num_verts;

  if (num_verts == 0) return false;
  dest->MakeEmpty ();

  // Classify all points as NORMAL (z>=SMALL_Z), NEAR (0<=z<SMALL_Z), or
  // BEHIND (z<0).  Use several processing algorithms: trivially accept if all
  // points are NORMAL, mixed process if some points are NORMAL and some
  // are not, special process if there are no NORMAL points, but some are
  // NEAR.  Assume that the polygon has already been culled if all points
  // are BEHIND.
  // Handle the trivial acceptance case:
  ind = source;
  while (ind < end)
  {
    if (ind->z >= SMALL_Z)
      AddPerspective (dest, *ind, fov, shift_x, shift_y);
    else
      break;
    ind++;
  }

  // Check if special or mixed processing is required
  if (ind == end) return true;

  // If we are processing a triangle (uv_coords != 0) then
  // we stop here because the triangle is only visible if all
  // vertices are visible (this is not exactly true but it is
  // easier this way! @@@ CHANGE IN FUTURE).

  csVector3 *exit = 0, *exitn = 0, *reenter = 0, *reentern = 0;
  csVector2 *evert = 0;

  if (ind == source)
  {
    while (ind < end)
    {
      if (ind->z >= SMALL_Z)
      {
        reentern = ind;
        reenter = ind - 1;
        break;
      }

      ind++;
    }
  }
  else
  {
    exit = ind;
    exitn = ind - 1;
    evert = dest->GetLast ();
  }

  // Check if mixed processing is required
  if (exit || reenter)
  {
    bool needfinish = false;

    if (exit)
    {
      // we know where the polygon is no longer NORMAL, now we need to

      // to find out on which edge it becomes NORMAL again.
      while (ind < end)
      {
        if (ind->z >= SMALL_Z)
        {
          reentern = ind;
          reenter = ind - 1;
          break;
        }

        ind++;
      }

      if (ind == end)
      {
        reentern = source;
        reenter = ind - 1;
      }
      else
        needfinish = true;
    } /* if (exit) */
    else
    {
      // we know where the polygon becomes NORMAL, now we need to
      // to find out on which edge it ceases to be NORMAL.
      while (ind < end)
      {
        if (ind->z >= SMALL_Z)
          AddPerspective (dest, *ind, fov, shift_x, shift_y);
        else
        {
          exit = ind;
          exitn = ind - 1;
          break;
        }

        ind++;
      }

      if (ind == end)
      {
        exit = source;
        exitn = ind - 1;
      }

      evert = dest->GetLast ();
    }

    // Add the NEAR points appropriately.
#define MAX_VALUE 1000000.
    // First, for the exit point.
    float ex, ey, epointx, epointy;
    ex = exitn->z * exit->x - exitn->x * exit->z;
    ey = exitn->z * exit->y - exitn->y * exit->z;
    if (ABS (ex) < SMALL_EPSILON && ABS (ey) < SMALL_EPSILON)
    {
      // Uncommon special case:  polygon passes through origin.
      //plane->WorldToCamera (trans, source[0]);  //@@@ Why is this needed???
      ex = plane_cam.A ();
      ey = plane_cam.B ();
      if (ABS (ex) < SMALL_EPSILON && ABS (ey) < SMALL_EPSILON)
      {
        // Downright rare case:  polygon near parallel with viewscreen.
        ex = exit->x - exitn->x;
        ey = exit->y - exitn->y;
      }
    }

    if (ABS (ex) > ABS (ey))
    {
      if (ex > 0)
        epointx = MAX_VALUE;
      else
        epointx = -MAX_VALUE;
      epointy = (epointx - evert->x) * ey / ex + evert->y;
    }
    else
    {
      if (ey > 0)
        epointy = MAX_VALUE;
      else
        epointy = -MAX_VALUE;
      epointx = (epointy - evert->y) * ex / ey + evert->x;
    }

    // Next, for the reentry point.
    float rx, ry, rpointx, rpointy;

    // Perspective correct the point.
    float iz = fov / reentern->z;
    csVector2 rvert;
    rvert.x = reentern->x * iz + shift_x;
    rvert.y = reentern->y * iz + shift_y;

    if (reenter == exit && reenter->z > -SMALL_EPSILON)
    {
      rx = ex;
      ry = ey;
    }
    else
    {
      rx = reentern->z * reenter->x - reentern->x * reenter->z;
      ry = reentern->z * reenter->y - reentern->y * reenter->z;
    }

    if (ABS (rx) < SMALL_EPSILON && ABS (ry) < SMALL_EPSILON)
    {
      // Uncommon special case:  polygon passes through origin.
      //plane->WorldToCamera (trans, source[0]);  //@@@ Why is this needed?
      rx = plane_cam.A ();
      ry = plane_cam.B ();
      if (ABS (rx) < SMALL_EPSILON && ABS (ry) < SMALL_EPSILON)
      {
        // Downright rare case:  polygon near parallel with viewscreen.
        rx = reenter->x - reentern->x;
        ry = reenter->y - reentern->y;
      }
    }

    if (ABS (rx) > ABS (ry))
    {
      if (rx > 0)
        rpointx = MAX_VALUE;
      else
        rpointx = -MAX_VALUE;
      rpointy = (rpointx - rvert.x) * ry / rx + rvert.y;
    }
    else
    {
      if (ry > 0)
        rpointy = MAX_VALUE;
      else
        rpointy = -MAX_VALUE;
      rpointx = (rpointy - rvert.y) * rx / ry + rvert.x;
    }

#define QUADRANT(x, y)  ((y < x ? 1 : 0) ^ (x < -y ? 3 : 0))
#define MQUADRANT(x, y) ((y < x ? 3 : 0) ^ (x < -y ? 1 : 0))
    dest->AddVertex (epointx, epointy);
#if EXPERIMENTAL_BUG_FIX
    if (mirror)
    {
      int quad = MQUADRANT (epointx, epointy);
      int rquad = MQUADRANT (rpointx, rpointy);
      if ((quad == 0 && -epointx == epointy) ||
          (quad == 1 && epointx == epointy))
        quad++;
      if ((rquad == 0 && -rpointx == rpointy) ||
          (rquad == 1 && rpointx == rpointy))
        rquad++;
      while (quad != rquad)
      {
        epointx = float((quad & 2) ? MAX_VALUE : -MAX_VALUE);
        epointy = float((quad == 0 || quad == 3) ? MAX_VALUE : -MAX_VALUE);
        dest->AddVertex (epointx, epointy);
        quad = (quad + 1) & 3;
      }
    }
    else
    {
      int quad = QUADRANT (epointx, epointy);
      int rquad = QUADRANT (rpointx, rpointy);
      if ((quad == 0 && epointx == epointy) ||
          (quad == 1 && -epointx == epointy))
        quad++;
      if ((rquad == 0 && rpointx == rpointy) ||
          (rquad == 1 && -rpointx == rpointy))
        rquad++;
      while (quad != rquad)
      {
        epointx = float((quad & 2) ? -MAX_VALUE : MAX_VALUE);
        epointy = float((quad == 0 || quad == 3) ? MAX_VALUE : -MAX_VALUE);
        dest->AddVertex (epointx, epointy);
        quad = (quad + 1) & 3;
      }
    }
#endif
    dest->AddVertex (rpointx, rpointy);

    // Add the rest of the vertices, which are all NORMAL points.
    if (needfinish)
      while (ind < end) AddPerspective (dest, *ind++, fov, shift_x, shift_y);
  } /* if (exit || reenter) */

  // Do special processing (all points are NEAR or BEHIND)
  else
  {
    if (mirror)
    {
      csVector3 *ind2 = end - 1;
      for (ind = source; ind < end; ind2 = ind, ind++)
        if ((ind->x - ind2->x) *
            (ind2->y) - (ind->y - ind2->y) *
            (ind2->x) > -SMALL_EPSILON)
          return false;
      dest->AddVertex (MAX_VALUE, -MAX_VALUE);
      dest->AddVertex (MAX_VALUE, MAX_VALUE);
      dest->AddVertex (-MAX_VALUE, MAX_VALUE);
      dest->AddVertex (-MAX_VALUE, -MAX_VALUE);
    }
    else
    {
      csVector3 *ind2 = end - 1;
      for (ind = source; ind < end; ind2 = ind, ind++)
        if ((ind->x - ind2->x) *
            (ind2->y) - (ind->y - ind2->y) *
            (ind2->x) < SMALL_EPSILON)
          return false;
      dest->AddVertex (-MAX_VALUE, -MAX_VALUE);
      dest->AddVertex (-MAX_VALUE, MAX_VALUE);
      dest->AddVertex (MAX_VALUE, MAX_VALUE);
      dest->AddVertex (MAX_VALUE, -MAX_VALUE);
    }
  }

  return true;
}

void csFatLoopStep::ProcessNode (iRenderView* rview, RenderNode* node,
                                 csShaderVarStack &stacks)
{
  if (node->renderNode)
  {
    node->renderNode->Process (rview);
    return;
  }

  switch (node->nodeType)
  {
    case RenderNode::Container:
      {
        for (size_t i = 0; i < node->containedNodes.Length(); i++)
        {
          ProcessNode (rview, node->containedNodes[i], stacks);
        }
      }
      break;
    case RenderNode::Portal:
      RenderPortal (node, rview, stacks);
      break;
    default:
      CS_ASSERT(false);
  }
}

void csFatLoopStep::RenderPortal (RenderNode* node, iRenderView* rview,
                                  csShaderVarStack &stacks)
{
  iGraphics3D* g3d = rview->GetGraphics3D ();
  csPlane3& keep_plane = node->camera_plane;
  float keep_camera_z = 0;	// Also keep z-coordinate of vertex 0.
  /*if (is_this_fog || po->flags.Check (CS_PORTAL_ZFILL))
  {
    keep_camera_z = camera_vertices[po->GetVertexIndices ()[0]].z;
  }*/
  bool use_float_portal = node->portal->GetFlags().Check (CS_PORTAL_FLOAT);
  g3d->OpenPortal (node->poly.GetVertexCount(), node->poly.GetVertices(),
    node->camera_plane, use_float_portal);

  if (node->portal->CompleteSector (rview)) 
  {
    iSector* sector = rview->GetThisSector();
    if (sector->GetRecLevel () >= 5/*max_sector_visit*/)
      return;

    if (!node->poly.GetVertexCount ()) return;

    csRenderContext *old_ctxt = rview->GetRenderContext ();
    iCamera *icam = old_ctxt->icamera;
    csPolygonClipper new_view ((csPoly2D*)&node->poly,
  	  icam->IsMirrored (), true);

    rview->CreateRenderContext ();
    rview->SetRenderRecursionLevel (rview->GetRenderRecursionLevel () + 1);
    rview->SetClipper (&new_view);
    rview->ResetFogInfo ();
    rview->SetLastPortal ((iPortal*)this);
    rview->SetPreviousSector (rview->GetThisSector ());
    rview->SetClipPlane (node->camera_plane);
    rview->GetClipPlane ().Invert ();
    if (node->portal->GetFlags().Check (CS_PORTAL_CLIPDEST))
    {
      rview->UseClipPlane (true);
      rview->UseClipFrustum (true);
    }
    // When going through a portal we first remember the old clipper
    // and clip plane (if any). Then we set a new one. Later we restore.
    iGraphics3D *G3D = rview->GetGraphics3D ();
    csRef<iClipper2D> old_clipper = G3D->GetClipper ();

    int old_cliptype = G3D->GetClipType ();
    G3D->SetClipper (
        rview->GetClipper (),
        rview->IsClipperRequired ()
      	  ? CS_CLIPPER_REQUIRED : CS_CLIPPER_OPTIONAL);

    csPlane3 old_near_plane = G3D->GetNearPlane ();
    bool old_do_near_plane = G3D->HasNearPlane ();
    csPlane3 cp;
    if (rview->GetClipPlane (cp))
      G3D->SetNearPlane (cp);
    else
      G3D->ResetNearPlane ();

    if (node->portal->GetFlags().Check (CS_PORTAL_WARP))
    {
      iCamera *inewcam = rview->CreateNewCamera ();

      bool mirror = inewcam->IsMirrored ();
      csReversibleTransform warp_wor;
      node->portal->ObjectToWorld (node->movtrans, warp_wor);
      node->portal->WarpSpace (warp_wor, inewcam->GetTransform (), mirror);
      inewcam->SetMirrored (mirror);

      //sector->Draw (rview);
    }
    //else
      //sector->Draw (rview);

    for (size_t n = 0; n < node->containedNodes.Length(); n++)
    {
      ProcessNode (rview, node->containedNodes[n], stacks);
    }

    rview->RestoreRenderContext ();

    // Now restore our G3D clipper and plane.
    G3D->SetClipper (old_clipper, old_cliptype);
    if (old_do_near_plane)
      G3D->SetNearPlane (old_near_plane);
    else
      G3D->ResetNearPlane ();
  }

  bool use_zfill_portal = node->portal->GetFlags().Check (CS_PORTAL_ZFILL);
  g3d->ClosePortal (use_zfill_portal);

/*
  iGraphics3D* g3d = rview->GetGraphics3D ();

  // is_this_fog is true if this sector is fogged.
  bool is_this_fog = rview->GetThisSector ()->HasFog ();

  // If we have fog or the portal is z-filled we need to keep the
  // camera plane because recursive rendering may cause it to change.
  csPlane3 keep_plane = camera_plane;;
  float keep_camera_z = 0;	// Also keep z-coordinate of vertex 0.
  if (is_this_fog || po->flags.Check (CS_PORTAL_ZFILL))
  {
    keep_camera_z = camera_vertices[po->GetVertexIndices ()[0]].z;
  }
  // First call OpenPortal().
  bool use_float_portal = po->flags.Check (CS_PORTAL_FLOAT);
  g3d->OpenPortal (poly.GetVertexCount(), poly.GetVertices(),
      camera_plane, use_float_portal);

  // Draw through the portal. This can fail.
  // Drawing through a portal can fail because we have reached
  // the maximum number that a sector is drawn (for mirrors).
  po->Draw (poly, movtrans, rview, keep_plane);*/

  /*if (!CompleteSector (rview)) return false;
  if (sector->GetRecLevel () >= max_sector_visit)
    return false;

  if (!new_clipper.GetVertexCount ()) return false;

  csRenderView* csrview = (csRenderView*)rview;
  csRenderContext *old_ctxt = csrview->GetCsRenderContext ();
  iCamera *icam = old_ctxt->icamera;
  csPolygonClipper new_view ((csPoly2D*)&new_clipper,
  	icam->IsMirrored (), true);

  csrview->CreateRenderContext ();
  csrview->SetRenderRecursionLevel (csrview->GetRenderRecursionLevel () + 1);
  csrview->SetClipper (&new_view);
  csrview->ResetFogInfo ();
  csrview->SetLastPortal ((iPortal*)this);
  csrview->SetPreviousSector (rview->GetThisSector ());
  csrview->SetClipPlane (camera_plane);
  csrview->GetClipPlane ().Invert ();
  if (flags.Check (CS_PORTAL_CLIPDEST))
  {
    csrview->UseClipPlane (true);
    csrview->UseClipFrustum (true);
  }
  // When going through a portal we first remember the old clipper
  // and clip plane (if any). Then we set a new one. Later we restore.
  iGraphics3D *G3D = rview->GetGraphics3D ();
  csRef<iClipper2D> old_clipper = G3D->GetClipper ();

  int old_cliptype = G3D->GetClipType ();
  G3D->SetClipper (
      rview->GetClipper (),
      csrview->IsClipperRequired ()
      	? CS_CLIPPER_REQUIRED : CS_CLIPPER_OPTIONAL);

  csPlane3 old_near_plane = G3D->GetNearPlane ();
  bool old_do_near_plane = G3D->HasNearPlane ();
  csPlane3 cp;
  if (csrview->GetClipPlane (cp))
    G3D->SetNearPlane (cp);
  else
    G3D->ResetNearPlane ();

  if (flags.Check (CS_PORTAL_WARP))
  {
    iCamera *inewcam = csrview->CreateNewCamera ();

    bool mirror = inewcam->IsMirrored ();
    csReversibleTransform warp_wor;
    ObjectToWorld (t, warp_wor);
    WarpSpace (warp_wor, inewcam->GetTransform (), mirror);
    inewcam->SetMirrored (mirror);

    sector->Draw (rview);
  }
  else
    sector->Draw (rview);

  csrview->RestoreRenderContext ();

  // Now restore our G3D clipper and plane.
  G3D->SetClipper (old_clipper, old_cliptype);
  if (old_do_near_plane)
    G3D->SetNearPlane (old_near_plane);
  else
    G3D->ResetNearPlane ();

  return true;*/
/*  if (is_this_fog && fog_shader)
  {
    csSimpleRenderMesh mesh;
    mesh.meshtype = CS_MESHTYPE_TRIANGLEFAN;
    mesh.indexCount = (uint)po->GetVertexIndices ().Length ();
    // @@@ Weirdo overloads approaching, captain!
    mesh.indices = (const uint*)(int*)po->GetVertexIndices ().GetArray ();
    mesh.vertexCount = (uint)vertices.Length ();
    mesh.vertices = vertices.GetArray ();
    mesh.texcoords = 0;
    mesh.texture = 0;
    mesh.colors = 0;
    //mesh.object2camera = rview->GetCamera ()->GetTransform ();
    mesh.alphaType.alphaType = csAlphaMode::alphaSmooth;
    mesh.alphaType.autoAlphaMode = false;
    mesh.shader = fog_shader;
    // @@@ Hackish...
    csShaderVariableContext varContext;
    csShaderVarStack &stacks = shader_man->GetShaderVariableStack ();
    for (size_t i=0; i<stacks.Length (); i++)
    {
      if (stacks[i].Length ()>0)
        varContext.AddVariable (stacks[i].Top ());
    }
    mesh.dynDomain = &varContext;
    // @@@ Could be used for z-fill and stuff, while we're at it?
    mesh.z_buf_mode = CS_ZBUF_TEST;
    g3d->DrawSimpleMesh (mesh);
  }

  // Make sure to close the portal again.
  bool use_zfill_portal = po->flags.Check (CS_PORTAL_ZFILL);
  g3d->ClosePortal (use_zfill_portal);
*/
}

