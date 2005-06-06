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
#include "iengine/portal.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "csgeom/poly2d.h"
#include "csgeom/poly3d.h"
#include "csgeom/polyclip.h"
#include "csutil/flags.h"

#include "portalnode.h"

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

csPortalRenderNode::csPortalRenderNode (iPortal* portal, iRenderView* rview,
  const csReversibleTransform& movtrans) : portal(portal), movtrans(movtrans)
{
}

bool csPortalRenderNode::Preprocess (iRenderView* rview)
{
  if (!portal->CompleteSector (rview)) return false;

  iGraphics3D* g3d = rview->GetGraphics3D ();
  bool use_float_portal = portal->GetFlags().Check (CS_PORTAL_FLOAT);
  g3d->OpenPortal (poly.GetVertexCount(), poly.GetVertices(),
    camera_plane, use_float_portal);

  iSector* sector = rview->GetThisSector();
  if (sector->GetRecLevel () >= portal->GetMaximumSectorVisit())
    return false;

  if (!poly.GetVertexCount ()) return false;

  csRenderContext *old_ctxt = rview->GetRenderContext ();
  iCamera *icam = old_ctxt->icamera;
  csRef<csPolygonClipper> new_view;
  new_view.AttachNew (new csPolygonClipper ((csPoly2D*)&poly, 
    icam->IsMirrored (), true));

  rview->CreateRenderContext ();
  rview->SetRenderRecursionLevel (rview->GetRenderRecursionLevel () + 1);
  rview->SetClipper (new_view);
  rview->ResetFogInfo ();
  rview->SetLastPortal ((iPortal*)this);
  rview->SetPreviousSector (rview->GetThisSector ());
  rview->SetClipPlane (camera_plane);
  rview->GetClipPlane ().Invert ();
  if (portal->GetFlags().Check (CS_PORTAL_CLIPDEST))
  {
    rview->UseClipPlane (true);
    rview->UseClipFrustum (true);
  }
  // When going through a portal we first remember the old clipper
  // and clip plane (if any). Then we set a new one. Later we restore.
  old_clipper = g3d->GetClipper ();

  old_cliptype = g3d->GetClipType ();
  g3d->SetClipper (
      rview->GetClipper (),
      rview->IsClipperRequired ()
      	? CS_CLIPPER_REQUIRED : CS_CLIPPER_OPTIONAL);

  old_near_plane = g3d->GetNearPlane ();
  old_do_near_plane = g3d->HasNearPlane ();
  csPlane3 cp;
  if (rview->GetClipPlane (cp))
    g3d->SetNearPlane (cp);
  else
    g3d->ResetNearPlane ();

  if (portal->GetFlags().Check (CS_PORTAL_WARP))
  {
    iCamera *inewcam = rview->CreateNewCamera ();

    bool mirror = inewcam->IsMirrored ();
    csReversibleTransform warp_wor;
    portal->ObjectToWorld (movtrans, warp_wor);
    portal->WarpSpace (warp_wor, inewcam->GetTransform (), mirror);
    inewcam->SetMirrored (mirror);
  }

  return true;
}

void csPortalRenderNode::Postprocess (iRenderView* rview)
{
  iGraphics3D* g3d = rview->GetGraphics3D ();

  rview->RestoreRenderContext ();

  // Now restore our G3D clipper and plane.
  g3d->SetClipper (old_clipper, old_cliptype);
  if (old_do_near_plane)
    g3d->SetNearPlane (old_near_plane);
  else
    g3d->ResetNearPlane ();

  bool use_zfill_portal = portal->GetFlags().Check (CS_PORTAL_ZFILL);
  g3d->ClosePortal (use_zfill_portal);

  old_clipper = 0;
}

//---------------------------------------------------------------------------

csPortalRenderNodeFactory::csPortalRenderNodeFactory (
  iObjectRegistry* object_reg)
{
}

csPortalRenderNode* csPortalRenderNodeFactory::CreatePortalNode (iPortal* portal, 
                                                                 iRenderView* rview,
                                                                 const csReversibleTransform& movtrans)
{
  csDirtyAccessArray<csVector3> camera_vertices;
  csPoly2D poly;
  csPlane3 camera_plane;

  // We assume here that ObjectToWorld has already been called.
  iCamera* camera = rview->GetCamera ();
  const csReversibleTransform& camtrans = camera->GetTransform ();

  // Setup clip and far plane.
  csRenderContext* rcontext = rview->GetRenderContext();
  bool do_portal_plane = rcontext->do_clip_plane;

  csPlane3 *farplane = camera->GetFarPlane ();
  bool mirrored = camera->IsMirrored ();
  int fov = camera->GetFOV ();
  float shift_x = camera->GetShiftX ();
  float shift_y = camera->GetShiftY ();

  //const csReversibleTransform movtrans = 
    //meshwrapper->GetMovable()->GetFullTransform ();

  const int* vertexIndices = portal->GetVertexIndices();
  int num_vertices = portal->GetVertexIndicesCount();

  camera_vertices.SetLength (num_vertices);
  const csVector3* world_vertices = portal->GetWorldVertices();
  int i;
  for (i = 0 ; i < num_vertices; i++)
    camera_vertices[i] = camtrans.Other2This (world_vertices[vertexIndices[i]]);

  csVector3& cam_vec = camera_vertices[0];
  camtrans.Other2This (portal->GetWorldPlane (), cam_vec, camera_plane);
  camera_plane.Normalize ();

  if (/*clip_plane || clip_portal || clip_z_plane || */do_portal_plane || farplane)
  {
    csVector3 *verts;
    int num_verts;
    if (ClipToPlane (portal, do_portal_plane ? &rcontext->clip_plane : 0, 
      camtrans.GetOrigin (), verts, num_verts, camera_vertices.GetArray()))
    {
      // The far plane is defined negative. So if the portal is entirely
      // in front of the far plane it is not visible. Otherwise we will render
      // it.
      if (!farplane ||
        csPoly3D::Classify (*farplane, verts, num_verts) != CS_POL_FRONT)
      {
	csPoly2D clip;
	if (DoPerspective (verts, num_verts, &clip, mirrored, fov,
	      shift_x, shift_y, camera_plane) &&
	    clip.ClipAgainst (rview->GetClipper ()))
          return 0;
      }
    }
  }
  else
  {
    int j;
    for (j = 0 ; j < num_vertices ; j++)
      AddPerspective (&poly, camera_vertices[j], fov, shift_x, shift_y);
  }

  csPortalRenderNode* newNode = new csPortalRenderNode (portal, rview, movtrans);
  newNode->camera_plane = camera_plane;
  newNode->camera_vertices = camera_vertices;
  newNode->poly = poly;
  return newNode;
}

bool csPortalRenderNodeFactory::ClipToPlane (
  iPortal* portal,
  csPlane3 *portal_plane,
  const csVector3 &v_w2c,
  csVector3 * &pverts,
  int &num_verts,
  const csVector3* camera_vertices)
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

bool csPortalRenderNodeFactory::DoPerspective (csVector3 *source, int num_verts, 
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
