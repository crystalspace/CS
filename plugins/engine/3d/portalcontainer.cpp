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
#include "igeom/clip2d.h"
#include "imesh/objmodel.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "iengine/camera.h"
#include "iutil/objreg.h"
#include "plugins/engine/3d/portalcontainer.h"
#include "plugins/engine/3d/meshobj.h"
#include "plugins/engine/3d/engine.h"
#include "cstool/rviewclipper.h"

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{

// ---------------------------------------------------------------------------
// csPortalContainerTriMeshHelper
// ---------------------------------------------------------------------------

void csPortalContainerTriMeshHelper::SetPortalContainer (csPortalContainer* pc)
{
  parent = pc;
  data_nr = pc->GetDataNumber ()-1;
}

void csPortalContainerTriMeshHelper::Setup ()
{
  parent->Prepare ();
  if (data_nr != parent->GetDataNumber () || !vertices)
  {
    data_nr = parent->GetDataNumber ();
    Cleanup ();

    vertices = parent->GetVertices ();
    // Count number of needed triangles.
    num_tri = 0;

    size_t i;
    const csRefArray<csPortal>& portals = parent->GetPortals ();
    for (i = 0 ; i < portals.GetSize () ; i++)
    {
      csPortal *p = portals[i];
      if (p->flags.CheckAll (poly_flag))
        num_tri += p->GetVertexIndicesCount ()-2;
    }

    if (num_tri)
    {
      triangles = new csTriangle[num_tri];
      num_tri = 0;
      for (i = 0 ; i < portals.GetSize () ; i++)
      {
        csPortal *p = portals[i];
        if (p->flags.CheckAll (poly_flag))
        {
          csDirtyAccessArray<int>& vi = p->GetVertexIndices ();
          size_t j;
          for (j = 1 ; j < (size_t)p->GetVertexIndicesCount ()-1 ; j++)
          {
	    triangles[num_tri].a = vi[0];
	    triangles[num_tri].b = vi[j];
	    triangles[num_tri].c = vi[j+1];
            num_tri++;
          }
        }
      }
    }
  }
}

void csPortalContainerTriMeshHelper::Cleanup ()
{
  vertices = 0;
  delete[] triangles;
  triangles = 0;
}

// ---------------------------------------------------------------------------
// csPortalContainer
// ---------------------------------------------------------------------------


csPortalContainer::csPortalContainer (iEngine* engine,
	iObjectRegistry *object_reg) :
	scfImplementationType (this, engine)
{
  prepared = false;
  data_nr = 0;
  movable_nr = -1;
  movable_identity = false;

  meshwrapper = 0;

  csRef<csPortalContainerTriMeshHelper> trimesh;
  trimesh.AttachNew (new csPortalContainerTriMeshHelper (0));
  trimesh->SetPortalContainer (this);
  csEngine* e = static_cast<csEngine*> (engine);
  SetTriangleData (e->base_id, trimesh);
  trimesh.AttachNew (new csPortalContainerTriMeshHelper (CS_PORTAL_COLLDET));
  trimesh->SetPortalContainer (this);
  SetTriangleData (e->colldet_id, trimesh);
  trimesh.AttachNew (new csPortalContainerTriMeshHelper (CS_PORTAL_VISCULL));
  trimesh->SetPortalContainer (this);
  SetTriangleData (e->viscull_id, trimesh);

  shader_man = csQueryRegistry<iShaderManager> (object_reg);
  fog_shader = shader_man->GetShader ("std_lighting_portal");

  fogplane_name = e->svNameStringSet->Request ("fogplane");
  fogdensity_name = e->svNameStringSet->Request ("fog density");
  fogcolor_name = e->svNameStringSet->Request ("fog color");
}

csPortalContainer::~csPortalContainer ()
{
}

csRenderMesh** csPortalContainer::GetRenderMeshes (int& num,
	iRenderView* rview, iMovable* /*movable*/, uint32 /*frustum_mask*/)
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

  csReversibleTransform o2wt = meshwrapper->GetCsMovable().GetFullTransform();

  meshPtr->portal = this;
  meshPtr->material = 0;
  meshPtr->worldspace_origin = o2wt.GetOrigin ();
  if (rmCreated)
  {
    meshPtr->variablecontext.AttachNew (new csShaderVariableContext);
  }
  meshPtr->object2world = o2wt;
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
  for (i = 0 ; i < portals.GetSize () ; i++)
  {
    csPortal* prt = portals[i];
    size_t j;
    csArray<int>& vidx = prt->GetVertexIndices ();
    csPoly3D poly;
    csBox3 box;
    box.StartBoundingBox ();
    for (j = 0 ; j < vidx.GetSize () ; j++)
    {
      if (vt) vidx[j] = (int)vt[vidx[j]].new_idx;
      poly.AddVertex (vertices[vidx[j]]);
      box.AddBoundingVertex (vertices[vidx[j]]);
    }
    float max_sqradius = 0;
    csVector3 center = box.GetCenter ();
    for (j = 0 ; j < vidx.GetSize () ; j++)
    {
      float sqdist = csSquaredDist::PointPoint (center, vertices[vidx[j]]);
      if (sqdist > max_sqradius) max_sqradius = sqdist;
    }
    prt->object_sphere = csSphere (center, csQsqrt (max_sqradius));
    prt->SetObjectPlane (poly.ComputePlane ());
  }
  delete[] vt;
  object_bbox.StartBoundingBox ();
  for (i = 0 ; i < vertices.GetSize () ; i++)
    object_bbox.AddBoundingVertex (vertices[i]);

  object_radius = csQsqrt (csSquaredDist::PointPoint (
  	object_bbox.Max (), object_bbox.Min ())) * 0.5f;

}

//------------------- For iPortalContainer ---------------------------//

iPortal* csPortalContainer::CreatePortal (csVector3* vertices, int num)
{
  prepared = false;
  csPortal* prt = new csPortal (this);
  prt->SetMaterial (static_cast<csEngine*> (Engine)->GetDefaultPortalMaterial ());
  portals.Push (prt);
  movable_nr--;	// Make sure the movable information is updated for new portal!

  int i;
  for (i = 0 ; i < num ; i++)
  {
    int idx = (int)csPortalContainer::vertices.Push (vertices[i]);
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
  Prepare ();
  const csMovable& cmovable = meshwrapper->GetCsMovable ();
  if (movable_nr == cmovable.GetUpdateNumber ()) return;
  const csReversibleTransform movtrans = cmovable.GetFullTransform ();
  ObjectToWorld (cmovable, movtrans);
}

void csPortalContainer::ObjectToWorld (const csMovable& movable,
	const csReversibleTransform& movtrans)
{
  movable_nr = movable.GetUpdateNumber ();
  movable_identity = movable.IsFullTransformIdentity ();
  size_t i;
  world_vertices.SetSize (vertices.GetSize ());
  if (movable_identity)
  {
    world_vertices = vertices;
    for (i = 0 ; i < portals.GetSize () ; i++)
    {
      csPortal* prt = portals[i];
      csPlane3 wp (prt->GetIntObjectPlane ());
      prt->SetWorldPlane (wp);
      prt->world_sphere = prt->object_sphere;
    }
  }
  else
  {
    for (i = 0 ; i < vertices.GetSize () ; i++)
      world_vertices[i] = movtrans.This2Other (vertices[i]);
    for (i = 0 ; i < portals.GetSize () ; i++)
    {
      csPortal* prt = portals[i];
      csPlane3 p;
      csVector3& world_vec = world_vertices[portals[i]->GetVertexIndices ()[0]];
      movtrans.This2Other (prt->GetIntObjectPlane (), world_vec, p);
      p.Normalize ();
      prt->SetWorldPlane (p);
      prt->world_sphere = csSphere (
	  movtrans.This2Other (prt->object_sphere.GetCenter ()),
	  prt->object_sphere.GetRadius ());
    }
  }
}

void csPortalContainer::WorldToCamera (iCamera*,
	const csReversibleTransform& camtrans)
{
  camera_vertices.SetSize (world_vertices.GetSize ());
  size_t i;
  for (i = 0 ; i < world_vertices.GetSize () ; i++)
    camera_vertices[i] = camtrans.Other2This (world_vertices[i]);
  camera_planes.Empty ();
  for (i = 0 ; i < portals.GetSize () ; i++)
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
  num_vertices = (int)vt.GetSize ();
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

#define EXPERIMENTAL_BUG_FIX  1

class PerspectiveOutlet2D
{
protected:
  CS::Math::Matrix4 camProj;
  csPoly2D& dest;
  float iw, ih;
  
  static void Perspective (const csVector3& v, csVector2& p,
    const CS::Math::Matrix4& camProj)
  {
    csVector4 v_proj (camProj * csVector4 (v, 1));
    p.x = v_proj.x / v_proj.w;
    p.y = v_proj.y / v_proj.w;
  }
  static void PerspectiveScreen (const csVector3& v, csVector2& p,
    const CS::Math::Matrix4& camProj, float iw, float ih)
  {
    csVector4 v_proj (camProj * csVector4 (v, 1));
    p.x = (v_proj.x / v_proj.w + 1.0f) / iw;
    p.y = (v_proj.y / v_proj.w + 1.0f) / ih;
  }

public:
  PerspectiveOutlet2D (const CS::Math::Matrix4& camProj,
    csPoly2D& dest, int viewWidth, int viewHeight)
    : camProj (camProj), dest (dest)
  {
    if (viewWidth > 0 && viewHeight > 0)
    {
      iw = 2.0f/viewWidth;
      ih = 2.0f/viewHeight;
    }
    else
    {
      iw = ih = 0.0f;
    }
  }

  void MakeEmpty () { dest.MakeEmpty(); }
  void Add (const csVector3 &v)
  {
    csVector2 p;
    if (iw != 0.0f)
      PerspectiveScreen (v, p, camProj, iw, ih);
    else
      Perspective (v, p, camProj);
    dest.AddVertex (p);
  }
  void AddVertex (float x, float y)
  {
    dest.AddVertex (x, y);
  }
  const csVector2* GetLast() { return dest.GetLast(); }
  void Perspective (const csVector3& v, csVector2& p)
  {
    if (iw != 0.0f)
      PerspectiveScreen (v, p, camProj, iw, ih);
    else
      Perspective (v, p, camProj);
  }
  bool ClipAgainst (iClipper2D* clipper)
  {
    return dest.ClipAgainst (clipper);
  }
};

#define Z_NEAR	    (1.0f/9.0f)

template<typename Outlet>
static bool DoPerspective (Outlet& outlet, csVector3 *source, int num_verts,
  bool mirror, const csPlane3& plane_cam)
{
  csVector3 *ind, *end = source + num_verts;

  if (num_verts == 0) return false;
  outlet.MakeEmpty ();

  // Classify all points as NORMAL (z>=Z_NEAR), NEAR (0<=z<Z_NEAR), or
  // BEHIND (z<0).  Use several processing algorithms: trivially accept if all
  // points are NORMAL, mixed process if some points are NORMAL and some
  // are not, special process if there are no NORMAL points, but some are
  // NEAR.  Assume that the polygon has already been culled if all points
  // are BEHIND.
  // Handle the trivial acceptance case:
  ind = source;
  while (ind < end)
  {
    if (ind->z >= Z_NEAR)
      outlet.Add (*ind);
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
  const csVector2 *evert = 0;

  if (ind == source)
  {
    while (ind < end)
    {
      if (ind->z >= Z_NEAR)
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
    evert = outlet.GetLast ();
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
        if (ind->z >= Z_NEAR)
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
        if (ind->z >= Z_NEAR)
          outlet.Add (*ind);
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

      evert = outlet.GetLast ();
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
    csVector2 rvert;
    outlet.Perspective (*reentern, rvert);

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
    outlet.AddVertex (epointx, epointy);
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
        outlet.AddVertex (epointx, epointy);
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
        outlet.AddVertex (epointx, epointy);
        quad = (quad + 1) & 3;
      }
    }
#endif
    outlet.AddVertex (rpointx, rpointy);

    // Add the rest of the vertices, which are all NORMAL points.
    if (needfinish)
      while (ind < end) outlet.Add(*ind++);
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
      outlet.AddVertex (MAX_VALUE, -MAX_VALUE);
      outlet.AddVertex (MAX_VALUE, MAX_VALUE);
      outlet.AddVertex (-MAX_VALUE, MAX_VALUE);
      outlet.AddVertex (-MAX_VALUE, -MAX_VALUE);
    }
    else
    {
      csVector3 *ind2 = end - 1;
      for (ind = source; ind < end; ind2 = ind, ind++)
        if ((ind->x - ind2->x) *
            (ind2->y) - (ind->y - ind2->y) *
            (ind2->x) < SMALL_EPSILON)
          return false;
      outlet.AddVertex (-MAX_VALUE, -MAX_VALUE);
      outlet.AddVertex (-MAX_VALUE, MAX_VALUE);
      outlet.AddVertex (MAX_VALUE, MAX_VALUE);
      outlet.AddVertex (MAX_VALUE, -MAX_VALUE);
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
  csFlags g3d_flags = 0;
  if (po->flags.Check(CS_PORTAL_FLOAT)) g3d_flags.Set(CS_OPENPORTAL_FLOAT);
  if (po->flags.Check(CS_PORTAL_ZFILL)) g3d_flags.Set(CS_OPENPORTAL_ZFILL);
  if (po->flags.Check(CS_PORTAL_MIRROR)) g3d_flags.Set(CS_OPENPORTAL_MIRROR);
  g3d->OpenPortal (poly.GetVertexCount(), poly.GetVertices(),
      camera_plane, g3d_flags);

  // Draw through the portal. This can fail.
  // Drawing through a portal can fail because we have reached
  // the maximum number that a sector is drawn (for mirrors).
  po->Draw (poly, movtrans, rview, keep_plane);

  if (is_this_fog && fog_shader)
  {
    csSimpleRenderMesh mesh;
    mesh.meshtype = CS_MESHTYPE_TRIANGLEFAN;
    mesh.indexCount = (uint)po->GetVertexIndices ().GetSize ();
    // @@@ Weirdo overloads approaching, captain!
    mesh.indices = (const uint*)(int*)po->GetVertexIndices ().GetArray ();
    mesh.vertexCount = (uint)vertices.GetSize ();
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
    const csRefArray<csShaderVariable>& globVars = shader_man->GetShaderVariables ();
    for (size_t i = 0; i < globVars.GetSize (); i++)
    {
      varContext.AddVariable (globVars[i]);
    }
    const csRefArray<csShaderVariable>& sectorVars =
      rview->GetThisSector()->GetSVContext()->GetShaderVariables();
    for (size_t i = 0; i < sectorVars.GetSize (); i++)
    {
      varContext.AddVariable (sectorVars[i]);
    }
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
    varContext.GetVariableAdd (fogplane_name)->SetValue (fogPlane);

    mesh.dynDomain = &varContext;
    // @@@ Could be used for z-fill and stuff, while we're at it?
    mesh.z_buf_mode = CS_ZBUF_TEST;
    g3d->DrawSimpleMesh (mesh);
  }

  // Make sure to close the portal again.
  g3d->ClosePortal ();
}

//--------------------- For iMeshObject ------------------------------//

bool csPortalContainer::ExtraVisTest (iRenderView* rview,
	csReversibleTransform& tr_o2c, csVector3& camera_origin)
{
  Prepare ();

  csSphere cam_sphere, world_sphere;
  GetBoundingSpheres (rview, &tr_o2c, &camera_origin, world_sphere, 
    cam_sphere);

  return CS::RenderViewClipper::CullBSphere (rview->GetRenderContext (),
      cam_sphere, world_sphere, clip_portal, clip_plane, clip_z_plane);
}
  
void csPortalContainer::GetBoundingSpheres (iRenderView* rview, 
                                            csReversibleTransform* tr_o2c,
					    csVector3* camera_origin, 
					    csSphere& world_sphere, 
					    csSphere& cam_sphere)
{
  iCamera* camera = rview->GetCamera ();
  const csReversibleTransform& camtrans = camera->GetTransform ();
  const csMovable& cmovable = meshwrapper->GetCsMovable ();
  if (movable_nr != cmovable.GetUpdateNumber ())
  {
    const csReversibleTransform movtrans = cmovable.GetFullTransform ();
    ObjectToWorld (cmovable, movtrans);
  }

  csSphere sphere;
  sphere.SetCenter (object_bbox.GetCenter ());
  sphere.SetRadius (object_radius);

  uint8 local_t2c[sizeof(csReversibleTransform)];
  if (tr_o2c == 0) 
  {
#include "csutil/custom_new_disable.h"
    tr_o2c = new (local_t2c) csReversibleTransform;
#include "csutil/custom_new_enable.h"
  }
  *tr_o2c = camtrans;
  if (!movable_identity)
  {
    const csReversibleTransform movtrans = cmovable.GetFullTransform ();
    *tr_o2c /= movtrans;
    world_sphere = movtrans.This2Other (sphere);
  }
  else
  {
    world_sphere = sphere;
  }
  cam_sphere = tr_o2c->Other2This (sphere);
  if (camera_origin) *camera_origin = cam_sphere.GetCenter ();
}

bool csPortalContainer::Draw (iRenderView* rview, iMovable* /*movable*/,
  	csZBufMode /*zbufMode*/)
{
  Prepare ();

  // We assume here that ObjectToWorld has already been called.
  iCamera* camera = rview->GetCamera ();
  const csReversibleTransform& camtrans = camera->GetTransform ();

  WorldToCamera (camera, camtrans);

  // Setup clip and far plane.
  csPlane3 portal_plane, *pportal_plane;
  bool do_portal_plane =
    ((CS::RenderManager::RenderView*)rview)->GetClipPlane (portal_plane);
  if (do_portal_plane)
    pportal_plane = &portal_plane;
  else
    pportal_plane = 0;

  csPlane3 *farplane = camera->GetFarPlane ();
  bool mirrored = camera->IsMirrored ();

  const csReversibleTransform movtrans = 
    meshwrapper->GetCsMovable ().GetFullTransform ();

  csPoly2D poly;
  int viewWidth = rview->GetGraphics3D ()->GetWidth();
  int viewHeight = rview->GetGraphics3D ()->GetHeight();
  PerspectiveOutlet2D outlet (camera->GetProjectionMatrix(), poly, viewWidth, viewHeight);

  size_t i;
  if (clip_plane || clip_portal || clip_z_plane || do_portal_plane || farplane)
  {
    for (i = 0 ; i < portals.GetSize () ; i++)
    {
      csVector3 *verts;
      int num_verts;
      if (ClipToPlane ((int)i, pportal_plane, camtrans.GetOrigin (),
      	verts, num_verts))
      {
        // The far plane is defined negative. So if the portal is entirely
        // in front of the far plane it is not visible. Otherwise we will render
        // it.
        if (!farplane ||
          csPoly3D::Classify (*farplane, verts, num_verts) != CS_POL_FRONT)
        {
	  if (DoPerspective (outlet, verts, num_verts, mirrored,
	  	camera_planes[i]) && outlet.ClipAgainst (rview->GetClipper ()))
	  {
	    DrawOnePortal (portals[i], poly,
	      movtrans, rview, camera_planes[i]);
	  }
        }
      }
    }
  }
  else
  {
    for (i = 0 ; i < portals.GetSize () ; i++)
    {
      csPortal* prt = portals[i];
      csDirtyAccessArray<int>& vt = prt->GetVertexIndices ();
      int num_vertices = (int)vt.GetSize ();
      int j;
      outlet.MakeEmpty ();
      for (j = 0 ; j < num_vertices ; j++)
      {
        outlet.Add (camera_vertices[vt[j]]);
      }
      DrawOnePortal (portals[i], poly, movtrans, rview, camera_planes[i]);
    }
  }

  return false;
}

void csPortalContainer::HardTransform (const csReversibleTransform& t)
{
  size_t i;
  world_vertices.SetSize (vertices.GetSize ());
  for (i = 0 ; i < vertices.GetSize () ; i++)
  {
    vertices[i] = t.This2Other (vertices[i]);
    world_vertices[i] = vertices[i];
  }
  for (i = 0 ; i < portals.GetSize () ; i++)
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
  for (i = 0; i < portals.GetSize (); i++)
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
	int* polygon_idx, iMaterialWrapper** material)
{
  if (material) *material = 0;
  Prepare ();
  size_t i;
  float best_r = 2000000000.;
  int best_p = -1;

  for (i = 0; i < portals.GetSize (); i++)
  {
    csPortal *p = portals[i];
    float r;
    csVector3 cur_isect;
    if (p->IntersectSegment (start, end, cur_isect, &r))
    {
      if (r < best_r)
      {
        best_r = r;
        best_p = (int)i;
        isect = cur_isect;
      }
    }
  }

  if (pr) *pr = best_r;
  if (polygon_idx) *polygon_idx = best_p;
  return best_p != -1;
}

void csPortalContainer::GetRadius (float& radius, csVector3& center)
{
  Prepare ();
  center = object_bbox.GetCenter ();
  radius = object_radius;
}

class ScreenPolyOutputHelper
{
  csVector2* verts2D;
  csVector3* verts3D;
  size_t vertsSize;
  size_t* numVerts;
public:
  ScreenPolyOutputHelper (csVector2* verts2D, csVector3* verts3D,
    size_t vertsSize, size_t* numVerts) : verts2D (verts2D), verts3D (verts3D),
    vertsSize (vertsSize), numVerts (numVerts) {}
  
  void AddPoly (const csPoly2D& poly2D, const csPoly3D& poly3D)
  {
    size_t numVertsToCopy = csMin (vertsSize, poly2D.GetVertexCount());
    memcpy (verts2D, poly2D.GetVertices(), numVertsToCopy * sizeof (csVector2));
    memcpy (verts3D, poly3D.GetVertices(), numVertsToCopy * sizeof (csVector3));
    vertsSize -= numVertsToCopy;
    verts2D += numVertsToCopy;
    verts3D += numVertsToCopy;
    *numVerts++ = numVertsToCopy;
  }
  void AddEmpty ()
  {
    *numVerts++ = 0;
  }
};

class PerspectiveOutlet2D3D : public PerspectiveOutlet2D
{
private:
  csPoly3D& dest3D;
  iCamera* cam;
  int viewWidth, viewHeight;
  
  static csVector3 LerpPC (const csVector3& v1, const csVector3& v2, float t)
  {
    float iz1 = 1.0f / v1.z;
    float iz2 = 1.0f / v2.z;
    csVector3 vLerped = csLerp (v1 * iz1, v2 * iz2, t);
    float z = 1.0f / csLerp (iz1, iz2, t);
    return vLerped * z;
  }
public:
  PerspectiveOutlet2D3D (iCamera* cam, csPoly2D& dest2D, csPoly3D& dest3D,
    int viewWidth, int viewHeight)
    : PerspectiveOutlet2D (cam->GetProjectionMatrix(), dest2D, viewWidth, viewHeight),
    dest3D (dest3D), cam (cam), viewWidth (viewWidth), viewHeight (viewHeight) { }

  void MakeEmpty () 
  { 
    PerspectiveOutlet2D::MakeEmpty ();
    dest3D.MakeEmpty();
  }
  void Add (const csVector3 &v)
  {
    PerspectiveOutlet2D::Add (v);
    dest3D.AddVertex (v);
  }
  void AddVertex (float x, float y)
  {
    PerspectiveOutlet2D::AddVertex (x, y);
    csVector4 p (x, y, 1.0f/Z_NEAR, 1);
    csVector4 p3d (cam->GetInvProjectionMatrix() * p);
    dest3D.AddVertex (csVector3 (p3d[0], p3d[1], p3d[2]));
  }
  bool ClipAgainst (iClipper2D* clipper)
  {
    size_t clipOutputVerts = dest.GetVertexCount() + clipper->GetVertexCount();
    CS_ALLOC_STACK_ARRAY(csVector2, clipOut, clipOutputVerts);
    CS_ALLOC_STACK_ARRAY(csVertexStatus, clipOutStatus, clipOutputVerts);
    size_t outNum;
    csPoly2D destPx;
    destPx.SetVertexCount (dest.GetVertexCount());
    for (size_t p = 0; p < dest.GetVertexCount(); p++)
    {
      destPx[p].Set (dest[p].x, dest[p].y);
    }
    uint8 clipRes = clipper->Clip (
      destPx.GetVertices(), destPx.GetVertexCount(), clipOut, outNum,
      clipOutStatus);
    CS_ASSERT(outNum <= clipOutputVerts);
    if (clipRes == CS_CLIP_OUTSIDE) return false;
    if (clipRes == CS_CLIP_INSIDE) return true;
    
    csPoly2D orgDest2D (destPx);
    csPoly3D orgDest3D (dest3D);
    MakeEmpty ();
      
    for (size_t i = 0 ; i < outNum; i++)
    {
      dest.AddVertex (csVector2 (clipOut[i].x, clipOut[i].y));
      switch (clipOutStatus[i].Type)
      {
	case CS_VERTEX_ORIGINAL:
	  {
	    const size_t vt = clipOutStatus[i].Vertex;
	    dest3D.AddVertex (orgDest3D[vt]);
          }
	  break;
	case CS_VERTEX_ONEDGE:
	  {
	    const size_t vt = clipOutStatus[i].Vertex;
	    const size_t vt2 = (vt+1) % orgDest3D.GetVertexCount();
	    const float t = clipOutStatus[i].Pos;

            dest3D.AddVertex (LerpPC (orgDest3D[vt], orgDest3D[vt2], t));
	  }
          break;
	case CS_VERTEX_INSIDE:
	  {
	    float x = clipOut[i].x;
	    float y = clipOut[i].y;
	    size_t edge[2][2];
	    int edgeToFind = 0;
	    // Determine edges from which to interpolate the vertex data
	    size_t lastVert = orgDest2D.GetVertexCount() - 1;
	    for (size_t v = 0; v < orgDest2D.GetVertexCount(); v++)
	    {
	      if ((fabs(orgDest2D[lastVert].y - orgDest2D[v].y) > EPSILON) 
		&& ((y >= orgDest2D[lastVert].y && y <= orgDest2D[v].y)
		|| (y <= orgDest2D[lastVert].y && y >= orgDest2D[v].y)))
	      {
		edge[edgeToFind][0] = lastVert;
		edge[edgeToFind][1] = v;
		edgeToFind++;
		if (edgeToFind >= 2) break;
	      }
	      lastVert = v;
	    }
	    CS_ASSERT(edgeToFind >= 2);
	    const csVector2& A = orgDest2D[edge[0][0]];
	    const csVector2& B = orgDest2D[edge[0][1]];
	    const csVector2& C = orgDest2D[edge[1][0]];
	    const csVector2& D = orgDest2D[edge[1][1]];
	    // Coefficients
	    const float t1 = (y - A.y) / (B.y - A.y);
	    const float t2 = (y - C.y) / (D.y - C.y);
	    const float x1 = A.x + t1 * (B.x - A.x);
	    const float x2 = C.x + t2 * (D.x - C.x);
	    const float dx = (x2 - x1);
	    const float t = dx ? ((x - x1) / dx) : 0.0f;
	    
	    dest3D.AddVertex (LerpPC (
	      LerpPC (orgDest3D[edge[0][0]], orgDest3D[edge[0][1]], t1),
	      LerpPC (orgDest3D[edge[1][0]], orgDest3D[edge[1][1]], t2),
	      t));
          }
	  break;
	default:
	  CS_ASSERT(false);
      }
    }
      
    return true;
  }
};

void csPortalContainer::ComputeScreenPolygons (iRenderView* rview,
                                               csVector2* verts2D,
                                               csVector3* verts3D,  
                                               size_t vertsSize,
                                               size_t* numVerts, 
                                               int viewWidth, int viewHeight)
{
  Prepare ();
  
  csSphere world_sphere, cam_sphere;
  GetBoundingSpheres (rview, 0, 0, world_sphere, cam_sphere);
  
  if (!CS::RenderViewClipper::CullBSphere (rview->GetRenderContext (),
    cam_sphere, world_sphere, clip_portal, clip_plane, clip_z_plane))
  {
    memset (numVerts, 0, portals.GetSize () * sizeof (size_t));
    return;
  }

  // We assume here that ObjectToWorld has already been called.
  iCamera* camera = rview->GetCamera ();
  const csReversibleTransform& camtrans = camera->GetTransform ();

  WorldToCamera (camera, camtrans);

  // Setup clip and far plane.
  csPlane3 portal_plane, *pportal_plane;
  bool do_portal_plane =
    ((CS::RenderManager::RenderView*)rview)->GetClipPlane (portal_plane);
  if (do_portal_plane)
    pportal_plane = &portal_plane;
  else
    pportal_plane = 0;

  csPlane3 *farplane = camera->GetFarPlane ();
  bool mirrored = camera->IsMirrored ();

  ScreenPolyOutputHelper outHelper (verts2D, verts3D, vertsSize, numVerts);
  size_t i;
  csPoly2D poly2D;
  csPoly3D poly3D;
  PerspectiveOutlet2D3D outlet (camera, poly2D, poly3D, viewWidth, viewHeight);
  if (clip_plane || clip_portal || clip_z_plane || do_portal_plane || farplane)
  {
    for (i = 0 ; i < portals.GetSize () ; i++)
    {
      csVector3 *verts;
      int num_verts;
      if (ClipToPlane ((int)i, pportal_plane, camtrans.GetOrigin (),
	  verts, num_verts)
	// The far plane is defined negative. So if the portal is entirely
	// in front of the far plane it is not visible. Otherwise we will render
	// it.
        && (!farplane ||
	  csPoly3D::Classify (*farplane, verts, num_verts) != CS_POL_FRONT)
        && DoPerspective (outlet, verts, num_verts, mirrored,
	  camera_planes[i]) 
        && outlet.ClipAgainst (rview->GetClipper ()))
      {
        outHelper.AddPoly (poly2D, poly3D);
      }
      else
	outHelper.AddEmpty ();
    }
  }
  else
  {
    for (i = 0 ; i < portals.GetSize () ; i++)
    {
      outlet.MakeEmpty();
      csPortal* prt = portals[i];
      csDirtyAccessArray<int>& vt = prt->GetVertexIndices ();
      int num_vertices = (int)vt.GetSize ();
      int j;
      for (j = 0 ; j < num_vertices ; j++)
	outlet.Add (camera_vertices[vt[j]]);
      outHelper.AddPoly (poly2D, poly3D);
    }
  }
}

size_t csPortalContainer::GetTotalVertexCount () const
{
  size_t n = 0;
  for (size_t i = 0 ; i < portals.GetSize () ; i++)
  {
    n += portals[i]->GetVertexIndicesCount();
  }
  return n;
}

}
CS_PLUGIN_NAMESPACE_END(Engine)
