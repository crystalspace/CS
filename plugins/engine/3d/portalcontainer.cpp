/*
    Copyright (C) 2003 by Jorrit Tyberghein
              (C) 2004 by Marten Svanfeldt

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
#include "csgeom/sphere.h"
#include "csgeom/poly3d.h"
#include "csgeom/poly2d.h"
#include "csgeom/transfrm.h"
#include "igeom/objmodel.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iutil/objreg.h"
#include "plugins/engine/3d/portalcontainer.h"
#include "plugins/engine/3d/rview.h"
#include "plugins/engine/3d/meshobj.h"

// ---------------------------------------------------------------------------
// csPortalContainerPolyMeshHelper
// ---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csPortalContainerPolyMeshHelper)
  SCF_IMPLEMENTS_INTERFACE(iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

void csPortalContainerPolyMeshHelper::SetPortalContainer (csPortalContainer* pc)
{
  parent = pc;
  data_nr = pc->GetDataNumber ()-1;
}

void csPortalContainerPolyMeshHelper::Setup ()
{
  parent->Prepare ();
  if (data_nr != parent->GetDataNumber () || !vertices)
  {
    data_nr = parent->GetDataNumber ();
    Cleanup ();

    vertices = parent->GetVertices ();
    // Count number of needed polygons.
    num_poly = 0;

    size_t i;
    const csRefArray<csPortal>& portals = parent->GetPortals ();
    for (i = 0 ; i < portals.Length () ; i++)
    {
      csPortal *p = portals[i];
      if (p->flags.CheckAll (poly_flag)) num_poly++;
    }

    if (num_poly)
    {
      polygons = new csMeshedPolygon[num_poly];
      num_poly = 0;
      for (i = 0 ; i < portals.Length () ; i++)
      {
        csPortal *p = portals[i];
        if (p->flags.CheckAll (poly_flag))
        {
	  csDirtyAccessArray<int>& vidx = p->GetVertexIndices ();
          polygons[num_poly].num_vertices = vidx.Length ();
          polygons[num_poly].vertices = vidx.GetArray ();
          num_poly++;
        }
      }
    }
  }
}

void csPortalContainerPolyMeshHelper::Cleanup ()
{
  delete[] polygons;
  polygons = 0;
  vertices = 0;
  delete[] triangles;
  triangles = 0;
}

// ---------------------------------------------------------------------------
// csPortalContainer
// ---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT(csPortalContainer)
  SCF_IMPLEMENTS_INTERFACE (iPortalContainer)
  SCF_IMPLEMENTS_INTERFACE (iShadowReceiver)
SCF_IMPLEMENT_IBASE_EXT_END

csPortalContainer::csPortalContainer (iEngine* engine, iObjectRegistry *object_reg) :
	csMeshObject (engine),
	scfiPolygonMesh (0),
	scfiPolygonMeshCD (CS_PORTAL_COLLDET),
	scfiPolygonMeshLOD (CS_PORTAL_VISCULL)
{
  scfiObjectModel.SetPolygonMeshBase (&scfiPolygonMesh);
  scfiObjectModel.SetPolygonMeshColldet (&scfiPolygonMeshCD);
  scfiObjectModel.SetPolygonMeshViscull (&scfiPolygonMeshLOD);
  scfiObjectModel.SetPolygonMeshShadows (&scfiPolygonMeshLOD);

  prepared = false;
  data_nr = 0;
  movable_nr = -1;
  movable_identity = false;

  meshwrapper = 0;

  scfiPolygonMesh.SetPortalContainer (this);
  scfiPolygonMeshCD.SetPortalContainer (this);
  scfiPolygonMeshLOD.SetPortalContainer (this);

  shader_man = CS_QUERY_REGISTRY (object_reg, iShaderManager);
  fog_shader = shader_man->GetShader ("std_lighting_portal");
}

csPortalContainer::~csPortalContainer ()
{
}

csRenderMesh** csPortalContainer::GetRenderMeshes (int& num,
	iRenderView* rview, iMovable* movable, uint32 frustum_mask)
{
  csReversibleTransform tr_o2c;
  csVector3 camera_origin;
  if (!ExtraVisTest (rview, tr_o2c, camera_origin))
  {
    num = 0;
    return 0;
  }

  bool rmCreated;
  csRenderMesh*& meshPtr = rmHolder.GetUnusedMesh (rmCreated,
    rview->GetCurrentFrameNumber ());

  meshPtr->portal = this;
  meshPtr->material = 0;
  meshPtr->object2camera = tr_o2c;
  meshPtr->camera_origin = camera_origin;
  num = 1;
  return &meshPtr;
}

void csPortalContainer::Prepare ()
{
  if (prepared) return;
  prepared = true;
  movable_nr = -1; // Make sure move stuff gets updated.
  movable_identity = false;
  data_nr++;
  csCompressVertex* vt = csVector3Array::CompressVertices (vertices);
  size_t i;
  for (i = 0 ; i < portals.Length () ; i++)
  {
    csPortal* prt = portals[i];
    size_t j;
    csArray<int>& vidx = prt->GetVertexIndices ();
    csPoly3D poly;
    for (j = 0 ; j < vidx.Length () ; j++)
    {
      if (vt) vidx[j] = vt[vidx[j]].new_idx;
      poly.AddVertex (vertices[vidx[j]]);
    }
    prt->SetObjectPlane (poly.ComputePlane ());
  }
  delete[] vt;
  object_bbox.StartBoundingBox ();
  for (i = 0 ; i < vertices.Length () ; i++)
    object_bbox.AddBoundingVertex (vertices[i]);

  object_radius = object_bbox.Max () - object_bbox.GetCenter ();
  max_object_radius = csQsqrt (csSquaredDist::PointPoint (
  	object_bbox.Max (), object_bbox.Min ())) * 0.5f;

}

//------------------- For iPortalContainer ---------------------------//

iPortal* csPortalContainer::CreatePortal (csVector3* vertices, int num)
{
  prepared = false;
  csPortal* prt = new csPortal (this);
  portals.Push (prt);

  int i;
  for (i = 0 ; i < num ; i++)
  {
    int idx = csPortalContainer::vertices.Push (vertices[i]);
    prt->AddVertexIndex (idx);
  }

  prt->DecRef ();
  return prt;
}

void csPortalContainer::RemovePortal (iPortal* portal)
{
  prepared = false;
  portals.Delete ((csPortal*)portal);
}

//------------------------- General ----------------------------------//

void csPortalContainer::CheckMovable ()
{
  const csMovable& cmovable = meshwrapper->GetCsMovable ();
  if (movable_nr == cmovable.GetUpdateNumber ()) return;
  const csReversibleTransform& movtrans = cmovable.GetFullTransform ();
  ObjectToWorld (cmovable, movtrans);
}

void csPortalContainer::ObjectToWorld (const csMovable& movable,
	const csReversibleTransform& movtrans)
{
  movable_nr = movable.GetUpdateNumber ();
  movable_identity = movable.IsFullTransformIdentity ();
  size_t i;
  world_vertices.SetLength (vertices.Length ());
  if (movable_identity)
  {
    world_vertices = vertices;
    for (i = 0 ; i < portals.Length () ; i++)
    {
      csPortal* prt = portals[i];
      prt->SetWorldPlane (prt->GetIntObjectPlane ());
    }
  }
  else
  {
    for (i = 0 ; i < vertices.Length () ; i++)
      world_vertices[i] = movtrans.This2Other (vertices[i]);
    for (i = 0 ; i < portals.Length () ; i++)
    {
      csPortal* prt = portals[i];
      csPlane3 p;
      csVector3& world_vec = world_vertices[portals[i]->GetVertexIndices ()[0]];
      movtrans.This2Other (prt->GetIntObjectPlane (), world_vec, p);
      p.Normalize ();
      prt->SetWorldPlane (p);
    }
  }
}

void csPortalContainer::WorldToCamera (iCamera*,
	const csReversibleTransform& camtrans)
{
  camera_vertices.SetLength (world_vertices.Length ());
  size_t i;
  for (i = 0 ; i < world_vertices.Length () ; i++)
    camera_vertices[i] = camtrans.Other2This (world_vertices[i]);
  camera_planes.Empty ();
  for (i = 0 ; i < portals.Length () ; i++)
  {
    csPortal* prt = portals[i];
    csPlane3 p;
    csVector3& cam_vec = camera_vertices[portals[i]->GetVertexIndices ()[0]];
    camtrans.Other2This (prt->GetIntWorldPlane (), cam_vec, p);
    p.Normalize ();
    camera_planes.Push (p);
  }
}

bool csPortalContainer::ClipToPlane (
  int portal_idx,
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
  csPortal* portal = portals[portal_idx];
  csDirtyAccessArray<int>& vt = portal->GetVertexIndices ();
  num_vertices = vt.Length ();
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
  const csPlane3 &wplane = portal->GetIntWorldPlane ();
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

#define EXPERIMENTAL_BUG_FIX  1

bool csPortalContainer::DoPerspective (
  csVector3 *source,
  int num_verts,
  csPoly2D *dest,
  bool mirror,
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

void csPortalContainer::DrawOnePortal (
  csPortal* po,
  const csPoly2D& poly,
  const csReversibleTransform& movtrans,
  iRenderView *rview,
  const csPlane3& camera_plane)
{
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

  // Draw through the portal. If this fails we draw the original polygon
  // instead. Drawing through a portal can fail because we have reached
  // the maximum number that a sector is drawn (for mirrors).
  if (po->Draw (poly, movtrans, rview, keep_plane))
  {
  }

  if (is_this_fog && fog_shader)
  {
    csSimpleRenderMesh mesh;
    mesh.meshtype = CS_MESHTYPE_TRIANGLEFAN;
    mesh.indexCount = po->GetVertexIndices ().Length ();
    // @@@ Weirdo overloads approaching, captain!
    mesh.indices = (const uint*)(int*)po->GetVertexIndices ().GetArray ();
    mesh.vertexCount = vertices.Length ();
    mesh.vertices = vertices.GetArray ();
    mesh.texcoords = 0;
    mesh.texture = 0;
    mesh.colors = 0;
    mesh.object2camera = rview->GetCamera ()->GetTransform ();
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
}

//------------------- For iShadowReceiver ----------------------------//

void csPortalContainer::CastShadows (iMovable* movable, iFrustumView* fview)
{
  Prepare ();
  CheckMovable ();

  size_t i;
  for (i = 0 ; i < portals.Length () ; i++)
  {
    csPortal *p = portals[i];
    p->CastShadows (movable, fview);
  }
}

//--------------------- For iMeshObject ------------------------------//

bool csPortalContainer::ExtraVisTest (iRenderView* rview,
	csReversibleTransform& tr_o2c, csVector3& camera_origin)
{
  Prepare ();

  csRenderView* csrview = (csRenderView*)rview;
  iCamera* camera = rview->GetCamera ();
  const csReversibleTransform& camtrans = camera->GetTransform ();
  const csMovable& cmovable = meshwrapper->GetCsMovable ();
  if (movable_nr != cmovable.GetUpdateNumber ())
  {
    const csReversibleTransform& movtrans = cmovable.GetFullTransform ();
    ObjectToWorld (cmovable, movtrans);
  }

  csSphere sphere, world_sphere;
  sphere.SetCenter (object_bbox.GetCenter ());
  sphere.SetRadius (max_object_radius);

  tr_o2c = camtrans;
  if (!movable_identity)
  {
    const csReversibleTransform& movtrans = cmovable.GetFullTransform ();
    tr_o2c /= movtrans;
    world_sphere = movtrans.This2Other (sphere);
  }
  else
  {
    world_sphere = sphere;
  }
  csSphere cam_sphere = tr_o2c.Other2This (sphere);
  camera_origin = cam_sphere.GetCenter ();

  return csrview->ClipBSphere (cam_sphere, world_sphere, clip_portal,
      clip_plane, clip_z_plane);
}

bool csPortalContainer::DrawTest (iRenderView* rview, iMovable*,
	uint32)
{
  CS_ASSERT_MSG("Cannot remove this", 0);
  csReversibleTransform tr_o2c;
  csVector3 camera_origin;
  return ExtraVisTest (rview, tr_o2c, camera_origin);
}

bool csPortalContainer::Draw (iRenderView* rview, iMovable* movable,
  	csZBufMode /*zbufMode*/)
{
  Prepare ();

  // We assume here that ObjectToWorld has already been called.
  iCamera* camera = rview->GetCamera ();
  const csReversibleTransform& camtrans = camera->GetTransform ();

  WorldToCamera (camera, camtrans);

  // Setup clip and far plane.
  csPlane3 portal_plane, *pportal_plane;
  bool do_portal_plane = ((csRenderView*)rview)->GetClipPlane (portal_plane);
  if (do_portal_plane)
    pportal_plane = &portal_plane;
  else
    pportal_plane = 0;

  csPlane3 *farplane = camera->GetFarPlane ();
  bool mirrored = camera->IsMirrored ();
  int fov = camera->GetFOV ();
  float shift_x = camera->GetShiftX ();
  float shift_y = camera->GetShiftY ();

  const csReversibleTransform movtrans = 
    meshwrapper->GetCsMovable ().GetFullTransform ();

  size_t i;
  if (clip_plane || clip_portal || clip_z_plane || do_portal_plane || farplane)
  {
    for (i = 0 ; i < portals.Length () ; i++)
    {
      csVector3 *verts;
      int num_verts;
      if (ClipToPlane (i, pportal_plane, camtrans.GetOrigin (),
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
	    DrawOnePortal (portals[i], clip,
	      movtrans, rview, camera_planes[i]);
	  }
        }
      }
    }
  }
  else
  {
    for (i = 0 ; i < portals.Length () ; i++)
    {
      csPoly2D poly;
      csPortal* prt = portals[i];
      csDirtyAccessArray<int>& vt = prt->GetVertexIndices ();
      int num_vertices = vt.Length ();
      int j;
      for (j = 0 ; j < num_vertices ; j++)
        AddPerspective (&poly, camera_vertices[vt[j]], fov, shift_x, shift_y);
      DrawOnePortal (portals[i], poly, movtrans, rview, camera_planes[i]);
    }
  }

  return false;
}

void csPortalContainer::HardTransform (const csReversibleTransform& t)
{
  size_t i;
  world_vertices.SetLength (vertices.Length ());
  for (i = 0 ; i < vertices.Length () ; i++)
  {
    vertices[i] = t.This2Other (vertices[i]);
    world_vertices[i] = vertices[i];
  }
  for (i = 0 ; i < portals.Length () ; i++)
  {
    csPortal* prt = portals[i];
    csPlane3 new_plane;
    csDirtyAccessArray<int>& vidx = prt->GetVertexIndices ();
    t.This2Other (prt->GetIntObjectPlane (), vertices[vidx[0]], new_plane);
    prt->SetObjectPlane (new_plane);
    prt->SetWorldPlane (new_plane);
    if (prt->flags.Check (CS_PORTAL_WARP))
      prt->ObjectToWorld (t, prt->warp_obj);
  }
  movable_nr--;	// Make sure object to world will be recalculated.
  prepared = false;
}

bool csPortalContainer::HitBeamOutline (const csVector3& start,
  	const csVector3& end, csVector3& isect, float* pr)
{
  Prepare ();
  size_t i;
  for (i = 0; i < portals.Length (); i++)
  {
    csPortal *p = portals[i];
    if (p->IntersectSegment (start, end, isect, pr))
    {
      return true;
    }
  }

  return false;
}

bool csPortalContainer::HitBeamObject (const csVector3& start,
	const csVector3& end, csVector3& isect, float* pr,
	int* polygon_idx)
{
  Prepare ();
  size_t i;
  float best_r = 2000000000.;
  int best_p = -1;

  for (i = 0; i < portals.Length (); i++)
  {
    csPortal *p = portals[i];
    float r;
    csVector3 cur_isect;
    if (p->IntersectSegment (start, end, cur_isect, &r))
    {
      if (r < best_r)
      {
        best_r = r;
        best_p = i;
        isect = cur_isect;
      }
    }
  }

  if (pr) *pr = best_r;
  if (polygon_idx) *polygon_idx = best_p;
  return best_p != -1;
}

void csPortalContainer::GetRadius (csVector3& radius, csVector3& center)
{
  Prepare ();
  center = object_bbox.GetCenter ();
  radius = object_radius;
}

