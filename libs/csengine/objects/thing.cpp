/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "qint.h"
#include "csengine/thing.h"
#include "csengine/polygon.h"
#include "csengine/polytmap.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/light.h"
#include "csengine/engine.h"
#include "csengine/stats.h"
#include "csengine/sector.h"
#include "csengine/cbufcube.h"
#include "csengine/bspbbox.h"
#include "csengine/curve.h"
#include "csengine/texture.h"
#include "csengine/material.h"
#include "csengine/meshobj.h"
#include "csutil/csstring.h"
#include "csutil/hashmap.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "iengine/shadcast.h"
#include "isys/vfs.h"
#include "iengine/rview.h"
#include "iengine/fview.h"
#include "qint.h"

long csThing::current_light_frame_number = 0;

//---------------------------------------------------------------------------

IMPLEMENT_IBASE_EXT (csThing)
  IMPLEMENTS_EMBEDDED_INTERFACE (iThingState)
  IMPLEMENTS_EMBEDDED_INTERFACE (iLightingInfo)
  IMPLEMENTS_EMBEDDED_INTERFACE (iPolygonMesh)
  IMPLEMENTS_EMBEDDED_INTERFACE (iVisibilityCuller)
  IMPLEMENTS_EMBEDDED_INTERFACE (iMeshObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iMeshObjectFactory)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csThing::ThingState)
  IMPLEMENTS_INTERFACE (iThingState)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csThing::LightingInfo)
  IMPLEMENTS_INTERFACE (iLightingInfo)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_EMBEDDED_IBASE(csThing::PolyMesh)
  IMPLEMENTS_INTERFACE(iPolygonMesh)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_EMBEDDED_IBASE(csThing::VisCull)
  IMPLEMENTS_INTERFACE(iVisibilityCuller)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_EMBEDDED_IBASE(csThing::MeshObject)
  IMPLEMENTS_INTERFACE(iMeshObject)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_EMBEDDED_IBASE(csThing::MeshObjectFactory)
  IMPLEMENTS_INTERFACE(iMeshObjectFactory)
IMPLEMENT_EMBEDDED_IBASE_END

int csThing::last_thing_id = 0;

csThing::csThing (iBase* parent) : csObject (parent),
	polygons (64, 64), curves (16, 16)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiThingState);
  CONSTRUCT_EMBEDDED_IBASE (scfiLightingInfo);
  CONSTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  CONSTRUCT_EMBEDDED_IBASE (scfiVisibilityCuller);
  CONSTRUCT_EMBEDDED_IBASE (scfiMeshObject);
  CONSTRUCT_EMBEDDED_IBASE (scfiMeshObjectFactory);

  last_thing_id++;
  thing_id = last_thing_id;
  last_polygon_id = 0;

  csEngine::current_engine->AddToCurrentRegion (this);

  curves_center.x = curves_center.y = curves_center.z = 0;
  curves_scale = 40;  
  curve_vertices = NULL;
  curve_texels = NULL;
  num_curve_vertices = max_curve_vertices = 0;

  max_vertices = num_vertices = 0;
  wor_verts = NULL;
  obj_verts = NULL;
  cam_verts = NULL;
  num_cam_verts = 0;

  draw_busy = 0;
  fog.enabled = false;
  bbox = NULL;
  obj_bbox_valid = false;
  light_frame_number = -1;

  center_idx = -1;
  ParentTemplate = NULL;

  cameranr = -1;
  movablenr = -1;
  wor_bbox_movablenr = -1;
  cached_movable = NULL;

  cfg_moving = CS_THING_MOVE_NEVER;

  static_tree = NULL;
  prepared = false;

  current_lod = 1;
  current_features = ALL_FEATURES;
}

csThing::~csThing ()
{
  if (wor_verts == obj_verts) delete [] obj_verts;
  else { delete [] wor_verts; delete [] obj_verts; }
  delete [] cam_verts;
  delete [] curve_vertices;
  delete [] curve_texels;
  delete bbox;
  delete static_tree;
  int i;
  for (i = 0 ; i < visobjects.Length () ; i++)
  {
    csVisObjInfo* vinf = (csVisObjInfo*)visobjects[i];
    delete vinf->bbox;
    vinf->visobj->DecRef ();
    delete vinf;
  }
}

void csThing::SetMovingOption (int opt)
{
  cfg_moving = opt;
  switch (cfg_moving)
  {
    case CS_THING_MOVE_NEVER:
      if (wor_verts != obj_verts)
        delete[] wor_verts;
      wor_verts = obj_verts;
      break;

    case CS_THING_MOVE_OCCASIONAL:
      if ((wor_verts == NULL || wor_verts == obj_verts) && max_vertices)
      {
        wor_verts = new csVector3[max_vertices];
        memcpy (wor_verts, obj_verts, max_vertices*sizeof (csVector3));
      }
      cached_movable = NULL;
      break;

    case CS_THING_MOVE_OFTEN:
      if (wor_verts != obj_verts)
        delete[] wor_verts;
      wor_verts = obj_verts;
      break;
  }

  movablenr = -1; // @@@ Is this good?
}

void csThing::WorUpdate ()
{
  int i;
  switch (cfg_moving)
  {
    case CS_THING_MOVE_NEVER:
      return;

    case CS_THING_MOVE_OCCASIONAL:
      if (cached_movable && cached_movable->GetUpdateNumber () != movablenr)
      {
        movablenr = cached_movable->GetUpdateNumber ();
	csReversibleTransform movtrans = cached_movable->GetFullTransform ();
        for (i = 0 ; i < num_vertices ; i++)
          wor_verts[i] = movtrans.This2Other (obj_verts[i]);
        for (i = 0 ; i < polygons.Length () ; i++)
        {
          csPolygon3D* p = GetPolygon3D (i);
          p->ObjectToWorld (movtrans, p->Vwor (0));
        }
        UpdateCurveTransform (movtrans);
	// If the movable changed we invalidate the camera number as well
	// to make sure the camera vertices are recalculated as well.
	cameranr--;
      }
      break;

    case CS_THING_MOVE_OFTEN:
      //@@@ Not implemented yet!
      return;
  }
}

void csThing::UpdateTransformation (const csTransform& c, long cam_cameranr)
{
  if (!cam_verts || num_vertices != num_cam_verts)
  {
    delete[] cam_verts;
    cam_verts = new csVector3[num_vertices];
    num_cam_verts = num_vertices;
    cameranr = cam_cameranr-1; // To make sure we will transform.
  }
  if (cameranr != cam_cameranr)
  {
    cameranr = cam_cameranr;
    int i;
    for (i = 0 ; i < num_vertices ; i++)
      cam_verts[i] = c.Other2This (wor_verts[i]);
  }
}

void csThing::Prepare ()
{
  if (prepared) return;
  prepared = true;
  int i;
  csPolygon3D* p;
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    p = polygons.Get (i);
    p->Finish ();
  }
}

int csThing::AddCurveVertex (const csVector3& v, const csVector2& t)
{
  if (!curve_vertices)
  {
    max_curve_vertices = 10;
    curve_vertices = new csVector3 [max_curve_vertices];
    curve_texels   = new csVector2 [max_curve_vertices];
  }
  while (num_curve_vertices >= max_curve_vertices)
  {
    max_curve_vertices += 10;
    csVector3* new_vertices = new csVector3 [max_curve_vertices];
    csVector2* new_texels   = new csVector2 [max_curve_vertices];
    memcpy (new_vertices, curve_vertices,
    	sizeof (csVector3)*num_curve_vertices);
    memcpy (new_texels, curve_texels,
    	sizeof (csVector2)*num_curve_vertices);
    delete [] curve_vertices;
    delete [] curve_texels;
    curve_vertices = new_vertices;
    curve_texels   = new_texels;
  }

  curve_vertices[num_curve_vertices] = v;
  curve_texels[num_curve_vertices] = t;
  num_curve_vertices++;
  return num_curve_vertices-1;
}

int csThing::AddVertex (float x, float y, float z)
{
  if (!obj_verts)
  {
    max_vertices = 10;
    obj_verts = new csVector3 [max_vertices];
    // Only if we occasionally move do we use the world vertex cache.
    if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
      wor_verts = new csVector3 [max_vertices];
    else
      wor_verts = obj_verts;
  }
  while (num_vertices >= max_vertices)
  {
    if (max_vertices < 10000)
      max_vertices *= 2;
    else
      max_vertices += 10000;
    csVector3* new_obj_verts = new csVector3 [max_vertices];
    memcpy (new_obj_verts, obj_verts, sizeof (csVector3)*num_vertices);
    delete [] obj_verts;
    obj_verts = new_obj_verts;

    if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
    {
      csVector3* new_wor_verts = new csVector3 [max_vertices];
      memcpy (new_wor_verts, wor_verts, sizeof (csVector3)*num_vertices);
      delete [] wor_verts;
      wor_verts = new_wor_verts;
    }
    else
      wor_verts = obj_verts;
  }

  // By default all vertices are set with the same object space and world space.
  obj_verts[num_vertices].Set (x, y, z);
  if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
    wor_verts[num_vertices].Set (x, y, z);
  num_vertices++;
  return num_vertices-1;
}

int csThing::AddVertexSmart (float x, float y, float z)
{
  if (!obj_verts) { AddVertex (x, y, z); return 0; }
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    if (ABS (x-obj_verts[i].x) < SMALL_EPSILON &&
	ABS (y-obj_verts[i].y) < SMALL_EPSILON &&
	ABS (z-obj_verts[i].z) < SMALL_EPSILON)
    {
      return i;
    }
  AddVertex (x, y, z);
  return num_vertices-1;
}

struct CompressVertex
{
  int orig_idx;
  float x, y, z;
  int new_idx;
};

int compare_vt (const void* p1, const void* p2)
{
  CompressVertex* sp1 = (CompressVertex*)p1;
  CompressVertex* sp2 = (CompressVertex*)p2;
  if (sp1->x < sp2->x) return -1;
  else if (sp1->x > sp2->x) return 1;
  if (sp1->y < sp2->y) return -1;
  else if (sp1->y > sp2->y) return 1;
  if (sp1->z < sp2->z) return -1;
  else if (sp1->z > sp2->z) return 1;
  return 0;
}

int compare_vt_orig (const void* p1, const void* p2)
{
  CompressVertex* sp1 = (CompressVertex*)p1;
  CompressVertex* sp2 = (CompressVertex*)p2;
  if (sp1->orig_idx < sp2->orig_idx) return -1;
  else if (sp1->orig_idx > sp2->orig_idx) return 1;
  return 0;
}

void csThing::CompressVertices ()
{
  if (num_vertices <= 0)
    return;

  CompressVertex* vt = new CompressVertex [num_vertices];
  int i, j;
  for (i = 0 ; i < num_vertices ; i++)
  {
    vt[i].orig_idx = i;
    vt[i].x = ceil (obj_verts[i].x*1000000);
    vt[i].y = ceil (obj_verts[i].y*1000000);
    vt[i].z = ceil (obj_verts[i].z*1000000);
  }

  // First sort so that all (nearly) equal vertices are together.
  qsort (vt, num_vertices, sizeof (CompressVertex), compare_vt);

  // Count unique values and tag all doubles with the index of the unique one.
  // new_idx in the vt table will be the index inside vt to the unique vector.
  int count_unique = 1;
  int last_unique = 0;
  vt[0].new_idx = last_unique;
  for (i = 1 ; i < num_vertices ; i++)
  {
    if (vt[i].x != vt[last_unique].x || vt[i].y != vt[last_unique].y ||
    	vt[i].z != vt[last_unique].z)
    {
      last_unique = i;
      count_unique++;
    }
    vt[i].new_idx = last_unique;
  }

  // Now allocate and fill new vertex tables.
  // After this new_idx in the vt table will be the new index
  // of the vector.
  csVector3* new_obj = new csVector3 [count_unique];
  new_obj[0] = obj_verts[vt[0].orig_idx];
  csVector3* new_wor = 0;
  if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
  {
    new_wor = new csVector3 [count_unique];
    new_wor[0] = wor_verts[vt[0].orig_idx];
  }
  vt[0].new_idx = 0;
  j = 1;
  for (i = 1 ; i < num_vertices ; i++)
  {
    if (vt[i].new_idx == i)
    {
      new_obj[j] = obj_verts[vt[i].orig_idx];
      if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
        new_wor[j] = wor_verts[vt[i].orig_idx];
      vt[i].new_idx = j;
      j++;
    }
    else
      vt[i].new_idx = j-1;
  }

  // Now we sort the table back on orig_idx so that we have
  // a mapping from the original indices to the new one (new_idx).
  qsort (vt, num_vertices, sizeof (CompressVertex), compare_vt_orig);

  // Replace the old vertex tables.
  delete [] obj_verts;
  obj_verts = new_obj;
  if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
  {
    delete [] wor_verts;
    wor_verts = new_wor;
  }
  else
    wor_verts = obj_verts;
  num_vertices = max_vertices = count_unique;

  // Now we can remap the vertices in all polygons.
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolygon3D* p = polygons.Get (i);
    csPolyIndexed& pi = p->GetVertices ();
    int* idx = pi.GetVertexIndices ();
    for (j = 0 ; j < pi.GetVertexCount () ; j++)
      idx[j] = vt[idx[j]].new_idx;
  }

  delete [] vt;

  // If there is a bounding box we recreate it.
  if (bbox) CreateBoundingBox ();
}

void csThing::BuildStaticTree (int mode)
{
  //mode = BSP_BALANCE_AND_SPLITS;
  delete static_tree; static_tree = NULL;

  CreateBoundingBox ();
  csBox3 bbox;
  GetBoundingBox (bbox);
  static_tree = new csOctree (this, bbox.Min (), bbox.Max (), 150/*15*/, mode);

  csString str ("vis/octree_");
  str += GetName ();
  csEngine* w = csEngine::current_engine;
  bool recalc_octree = true;
  if (!csEngine::do_force_revis && w->VFS->Exists ((const char*)str))
  {
    recalc_octree = false;
    CsPrintf (MSG_INITIALIZATION, "Loading bsp/octree...\n");
    recalc_octree = !((csOctree*)static_tree)->ReadFromCache (
    	w->VFS, (const char*)str, GetPolygonArray ().GetArray (),
	GetPolygonArray ().Length ());
    if (recalc_octree)
    {
      delete static_tree;
      static_tree = new csOctree (this, bbox.Min (), bbox.Max (), 150/*15*/, mode);
    }
  }
  if (recalc_octree)
  {
    CsPrintf (MSG_INITIALIZATION, "Calculate bsp/octree...\n");
    static_tree->Build (GetPolygonArray ());
    CsPrintf (MSG_INITIALIZATION, "Caching bsp/octree...\n");
    ((csOctree*)static_tree)->Cache (w->VFS, (const char*)str);
  }
  CsPrintf (MSG_INITIALIZATION, "Compress vertices...\n");
  CompressVertices ();
  CsPrintf (MSG_INITIALIZATION, "Build vertex tables...\n");
  ((csOctree*)static_tree)->BuildVertexTables ();

  static_tree->Statistics ();

  CsPrintf (MSG_INITIALIZATION, "DONE!\n");
}

csPolygonInt* csThing::GetPolygonInt (int idx)
{
  return (csPolygonInt*)GetPolygon3D (idx);
}

csPolygon3D* csThing::GetPolygon3D (const char* name)
{
  int idx = polygons.FindKey (name);
  return idx >= 0 ? polygons.Get (idx) : NULL;
}

csPolygon3D* csThing::NewPolygon (csMaterialWrapper* material)
{
  csPolygon3D* p = new csPolygon3D (material);
  AddPolygon (p);
  return p;
}

void csThing::AddPolygon (csPolygonInt* poly)
{
  ((csPolygon3D *)poly)->SetParent (this);
  polygons.Push ((csPolygon3D *)poly);
}

csCurve* csThing::GetCurve (char* name)
{
  int idx = curves.FindKey (name);
  return idx >= 0 ? curves.Get (idx) : NULL;
}

void csThing::AddCurve (csCurve* curve)
{
  curve->SetParent (this);
  curves.Push (curve);
}

iCurve* csThing::CreateCurve (iCurveTemplate* tmpl)
{
  iCurve* curve = tmpl->MakeCurve ();
  csCurve* c = curve->GetOriginalObject ();
  c->SetParent (this);
  int i;
  for (i = 0 ; i < tmpl->GetVertexCount () ; i++)
    curve->SetControlPoint (i, tmpl->GetVertex (i));
  AddCurve (c);
  return curve;
}

void csThing::HardTransform (const csReversibleTransform& t)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
  {
    obj_verts[i] = t.This2Other (obj_verts[i]);
    if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
      wor_verts[i] = obj_verts[i];
  }

  curves_center = t.This2Other (curves_center);
  if (curve_vertices)
    for (i = 0 ; i < num_curve_vertices ; i++)
      curve_vertices[i] = t.This2Other (curve_vertices[i]);

  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolygon3D* p = GetPolygon3D (i);
    p->HardTransform (t);
  }
  for (i = 0 ; i < curves.Length () ; i++)
  {
    csCurve* c = GetCurve (i);
    c->HardTransform (t);
  }
}

csPolygon3D* csThing::IntersectSegment (const csVector3& start, 
  const csVector3& end, csVector3& isect, float* pr)
{
  int i;
  float r, best_r = 2000000000.;
  csVector3 cur_isect;
  csPolygon3D* best_p = NULL;
  // @@@ This routine is not very optimal. Especially for things
  // with large number of polygons.
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolygon3D* p = polygons.Get (i);
    if (p->IntersectSegment (start, end, cur_isect, &r))
    {
      if (r < best_r)
      {
	best_r = r;
	best_p = p;
	isect = cur_isect;
      }
    }
  }
  if (pr) *pr = best_r;
  return best_p;
}

void csThing::DrawOnePolygon (csPolygon3D* p, csPolygon2D* poly,
	iRenderView* d, csZBufMode zMode)
{
  iCamera* icam = d->GetCamera ();

  if (d->AddedFogInfo ())
  {
    // If fog info was added then we are dealing with vertex fog and
    // the current sector has fog. This means we have to complete the
    // fog_info structure with the plane of the current polygon.
    d->GetFirstFogInfo ()->outgoing_plane = p->GetPlane ()->GetCameraPlane ();
  }

  Stats::polygons_drawn++;

  csPortal* po = p->GetPortal ();
  if (csSector::do_portals && po)
  {
    bool filtered = false;
    // is_this_fog is true if this sector is fogged.
    bool is_this_fog = d->GetThisSector ()->HasFog ();

    // If there is filtering (alpha mapping or something like that) we need
    // to keep the texture plane so that it can be drawn after the sector has
    // been drawn. The texture plane needs to be kept because this polygon
    // may be rendered again (through mirrors) possibly overwriting the plane.
    csPolyPlane* keep_plane = NULL;

    if (d->GetGraphics3D ()->GetRenderState (G3DRENDERSTATE_TRANSPARENCYENABLE))
      filtered = p->IsTransparent ();
    if (filtered || is_this_fog || (po && po->flags.Check (CS_PORTAL_ZFILL)))
    {
      keep_plane = new csPolyPlane (*(p->GetPlane ()));
    }

    // Draw through the portal. If this fails we draw the original polygon
    // instead. Drawing through a portal can fail because we have reached
    // the maximum number that a sector is drawn (for mirrors).
    if (po->Draw (poly, p, d))
    {
      if (filtered) poly->DrawFilled (d, p, keep_plane, zMode);
      if (is_this_fog)
	poly->AddFogPolygon (d->GetGraphics3D (), p, keep_plane,
	  	icam->IsMirrored (), d->GetThisSector ()->QueryObject ()
			->GetID (),
		CS_FOG_BACK);
      // Here we z-fill the portal contents to make sure that sprites
      // that are drawn outside of this portal cannot accidently cross
      // into the others sector space (we cannot trust the Z-buffer here).
      if (po->flags.Check (CS_PORTAL_ZFILL))
	poly->FillZBuf (d, p, keep_plane);
    }
    else
      poly->DrawFilled (d, p, p->GetPlane (), zMode);

    // Cleanup.
    if (keep_plane) keep_plane->DecRef ();
  }
  else
    poly->DrawFilled (d, p, p->GetPlane (), zMode);
}

void csThing::DrawPolygonArray (csPolygonInt** polygon, int num,
	iRenderView* d, csZBufMode zMode)
{
  csPolygon3D* p;
  csVector3* verts;
  int num_verts;
  int i;
  csPoly2DPool* render_pool = csEngine::current_engine->render_pol2d_pool;
  csPolygon2D* clip;
  iCamera* icam = d->GetCamera ();
  const csReversibleTransform& camtrans = icam->GetTransform ();
  
  // Setup clip and far plane.
  csPlane3 clip_plane, *pclip_plane;
  bool do_clip_plane = d->GetClipPlane (clip_plane);
  if (do_clip_plane) pclip_plane = &clip_plane;
  else pclip_plane = NULL;
  csPlaneClip plclip;
  bool do_plclip = icam->GetFarPlane (plclip);

  for (i = 0 ; i < num ; i++)
  {
    if (polygon[i]->GetType () != 1) continue;
    p = (csPolygon3D*)polygon[i];
    if (p->flags.Check (CS_POLY_NO_DRAW)) continue;
    p->UpdateTransformation (camtrans, icam->GetCameraNumber ());
    if (p->ClipToPlane (pclip_plane,
	 	camtrans.GetOrigin (), verts, num_verts)) //@@@Pool for verts?
    {
      if (!do_plclip || plclip.ClipPolygon (verts, num_verts))
      {
        clip = (csPolygon2D*)(render_pool->Alloc ());
	if (p->DoPerspective (camtrans, verts, num_verts, clip, NULL,
		icam->IsMirrored ()) && clip->ClipAgainst (d->GetClipper ()))
        {
          p->GetPlane ()->WorldToCamera (camtrans, verts[0]);
          DrawOnePolygon (p, clip, d, zMode);
        }
        render_pool->Free (clip);
      }
    }
  }
}

void csThing::DrawPolygonArrayDPM (csPolygonInt** /*polygon*/, int /*num*/,
	iRenderView* d, csZBufMode zMode)
{
  // @@@ We should include object 2 world transform here too like it
  // happens with sprites.
  int i;
  iCamera* icam = d->GetCamera ();

  csReversibleTransform tr_o2c = icam->GetTransform ();
  d->GetGraphics3D ()->SetObjectToCamera (&tr_o2c);
  d->GetGraphics3D ()->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zMode);

  G3DPolygonMesh mesh;
  mesh.num_vertices = GetVertexCount ();
  mesh.num_polygons = GetPolygonCount ();
  mesh.master_mat_handle = NULL;
  // @@@ It would be nice if we could avoid this allocate.
  // Even nicer would be if we didn't have to copy the data
  // to this structure every time. Maybe hold this array native
  // in every detail object?
  // IMPORTANT OPT!!! CACHE THIS ARRAY IN EACH ENTITY!
  mesh.polygons = new csPolygonDPM [GetPolygonCount ()];
  mesh.mat_handle = new iMaterialHandle* [GetPolygonCount ()];
  mesh.normal = new csPlane3 [GetPolygonCount ()];
  mesh.plane = new G3DTexturePlane [GetPolygonCount ()];
  mesh.poly_texture = new iPolygonTexture* [GetPolygonCount ()];
  for (i = 0 ; i < GetPolygonCount () ; i++)
  {
    csPolygon3D* p = GetPolygon3D (i);

    // Vertices.
    int num_v = p->GetVertexCount ();
    mesh.polygons[i].vertices = num_v;
    mesh.polygons[i].vertex = p->GetVertexIndices ();

    // Other info.
    // @@@ ONLY lightmapped polygons right now.
    // This function needs support for DrawTriangleMesh if gouraud
    // shaded polygons are used.
    csPolyTexLightMap *lmi = p->GetLightMapInfo ();
    if (!lmi)
    {
      printf ("INTERNAL ERROR! Don't use gouraud shaded polygons on DETAIL objects right now!\n");
      goto cleanup;
    }
    p->GetMaterialWrapper ()->Visit ();
    mesh.mat_handle[i] = p->GetMaterialHandle ();

    csPolyTxtPlane* txt_plane = lmi->GetTxtPlane ();
    csMatrix3* m_wor2tex;
    csVector3* v_wor2tex;
    txt_plane->GetWorldToTexture (m_wor2tex, v_wor2tex);
    mesh.plane[i].m_cam2tex = m_wor2tex;	// @@@ WRONG NAME
    mesh.plane[i].v_cam2tex = v_wor2tex;
    mesh.normal[i] = p->GetPlane ()->GetWorldPlane ();
    mesh.poly_texture[i] = lmi->GetPolyTex ();
  }

  mesh.do_fog = false;
  mesh.do_clip = false;
  mesh.do_mirror = icam->IsMirrored ();
  mesh.vertex_mode = G3DPolygonMesh::VM_WORLDSPACE;
  mesh.vertices = wor_verts;
  mesh.vertex_fog = NULL;

  // @@@ FAR plane not yet supported here!
  // @@@ fog not supported yet.
  // @@@ clipping not supported yet.

  d->GetGraphics3D ()->DrawPolygonMesh (mesh);

cleanup:
  delete [] mesh.polygons;
  delete [] mesh.mat_handle;
  delete [] mesh.plane;
  delete [] mesh.normal;
  delete [] mesh.poly_texture;
}

void* csThing::TestQueuePolygonArray (csPolygonInt** polygon, int num,
	iRenderView* d, csPolygon2DQueue* poly_queue, bool pvs)
{
  csPolygon3D* p;
  csPortal* po;
  csVector3* verts;
  int num_verts;
  int i;
  csCBuffer* c_buffer = csEngine::current_engine->GetCBuffer ();
  bool visible;
  csPoly2DPool* render_pool = csEngine::current_engine->render_pol2d_pool;
  csPolygon2D* clip;
  iCamera* icam = d->GetCamera ();
  const csReversibleTransform& camtrans = icam->GetTransform ();

  // Normally visibility of objects in the bsp tree is handled by inserting
  // the bounding box polygons of the object in the tree. However, this
  // insertion will not modify the tree meaning that if we reach a bsp leaf
  // we will just insert all remaining bounding box polygons in that leaf
  // without further subdividing the leaf as would be needed for real
  // bsp building.
  //
  // When doing visibility testing we traverse the tree from front to back
  // and insert all polygons in the c-buffer. Bounding box polygons
  // are not inserted but only tested to see if the object is visible or not.
  //
  // So this means that the bsp code should always traverse the bounding
  // box polygons first before giving the real polygons.

  for (i = 0 ; i < num ; i++)
  {
    if (!csEngine::ProcessPolygon ()) return (void*)1;
    // For every polygon we perform some type of culling (2D or 3D culling).
    // If 2D culling (c-buffer) then transform it to screen space
    // and perform clipping to Z plane. Then test against the c-buffer
    // to see if it is visible.
    // If 3D culling (quadtree3D) then we just test it.
    if (polygon[i]->GetType () == 3)
    {
      // We're dealing with a csBspPolygon.
      csBspPolygon* bsppol = (csBspPolygon*)polygon[i];
      csVisObjInfo* obj = bsppol->GetOriginator ();
      bool obj_vis = obj->visobj->IsVisible ();
      csPolyTreeBBox* tbb = obj->bbox;

      // If the object is already marked visible then we don't have
      // to do any of the other processing for this polygon.
      if (!obj_vis)
      {
	// Since we have a csBspPolygon we know that the poly tree object
	// is a csPolyTreeBBox instance.
        if (!tbb->IsTransformed ())
	{
	  // The bbox of this object has not yet been transformed
	  // to camera space.
	  tbb->World2Camera (camtrans);
	}

	// Culling.
	bool mark_vis = false;
    	csPlane3 clip_plane, *pclip_plane;
    	bool do_clip_plane = d->GetClipPlane (clip_plane);
    	if (do_clip_plane) pclip_plane = &clip_plane;
    	else pclip_plane = NULL;
        clip = (csPolygon2D*)(render_pool->Alloc ());
        if ( bsppol->ClipToPlane (pclip_plane, camtrans.GetOrigin (),
	  	verts, num_verts))
	{
      	  csPlaneClip plclip;
      	  bool do_plclip = icam->GetFarPlane (plclip);
	  if (!do_plclip || plclip.ClipPolygon (verts, num_verts))   
	  {
            if (bsppol->DoPerspective (camtrans, verts,
	       		num_verts, clip, icam->IsMirrored ()) 
	          && clip->ClipAgainst (d->GetClipper ()) )
            {
	      if (c_buffer->TestPolygon (clip->GetVertices (),
	  	   clip->GetVertexCount ()))
	        mark_vis = true;
            }
	  }
	}    
	if (mark_vis)
          obj->visobj->MarkVisible ();
        if (clip) render_pool->Free (clip);
      }
    }
    else
    {
      // We're dealing with a csPolygon3D.

      // @@@ We should only alloc the 2D polygon when we need to do it.
      // So this means further down the 'if'.
      p = (csPolygon3D*)polygon[i];
      if (pvs)
      {
        //if (!p->IsVisible ())
        //{
          // Polygon is not visible because of PVS.
          //@@@ CURRENTLY DISABLED continue;
        //}
      }

      visible = false;
      clip = NULL;
      // Culling.
      if (p->flags.Check (CS_POLY_NO_DRAW))
      {
        // Don't draw this polygon.
      }
      else
      {
    	csPlane3 clip_plane, *pclip_plane;
    	bool do_clip_plane = d->GetClipPlane (clip_plane);
    	if (do_clip_plane) pclip_plane = &clip_plane;
    	else pclip_plane = NULL;
      	csPlaneClip plclip;
      	bool do_plclip = icam->GetFarPlane (plclip);

        clip = (csPolygon2D*)(render_pool->Alloc ());
        if (
         p->ClipToPlane (pclip_plane, camtrans.GetOrigin (), verts, num_verts)
	 && (!do_plclip || plclip.ClipPolygon (verts, num_verts))		    
         && p->DoPerspective (camtrans, verts, num_verts, clip, NULL,
                              icam->IsMirrored ())
         && clip->ClipAgainst (d->GetClipper ()))
        {
          po = p->GetPortal ();
	  if (csEngine::current_engine->IsPVSOnly ())
            visible = true;
	  else
            visible = c_buffer->InsertPolygon (clip->GetVertices (),
		    clip->GetVertexCount ());
        }
      }

      if (visible && !clip)
      {
	// If visible and we don't already have a clip (i.e. we did 3D culling)
	// then we need to make it here. It is still possible that the
	// polygon will be culled at this stage.
    	csPlane3 clip_plane, *pclip_plane;
    	bool do_clip_plane = d->GetClipPlane (clip_plane);
    	if (do_clip_plane) pclip_plane = &clip_plane;
    	else pclip_plane = NULL;
      	csPlaneClip plclip;
      	bool do_plclip = icam->GetFarPlane (plclip);
        clip = (csPolygon2D*)(render_pool->Alloc ());
        if (!(
           p->ClipToPlane (pclip_plane, camtrans.GetOrigin (), verts, num_verts)
           && (!do_plclip || plclip.ClipPolygon (verts, num_verts))		    
           && p->DoPerspective (camtrans, verts, num_verts, clip, NULL,
                                icam->IsMirrored ())
           && clip->ClipAgainst (d->GetClipper ())))
	{
	  visible = false;
	}
      }

      if (visible)
      {
        poly_queue->Push (p, clip);
	Stats::polygons_accepted++;
	if (c_buffer && c_buffer->IsFull ()) return (void*)1;	// Stop
      }
      else
      {
        if (clip) render_pool->Free (clip);
        Stats::polygons_rejected++;
      }
    }
  }
  return NULL;
}

// @@@ We need a more clever algorithm here. We should try
// to recognize convex sub-parts of a polygonset and return
// convex shadow frustums for those. This will significantly
// reduce the number of shadow frustums. There are basicly
// two ways to do this:
//	- Split object into convex sub-parts in 3D.
//	- Split object into convex sub-parts in 2D.
// The first way is probably best because it is more efficient
// at runtime (important if we plan to use dynamic shadows for things)
// and also more correct in that a convex 3D object has no internal
// shadowing while a convex outline may have no correspondance to internal
// shadows.
void csThing::AppendShadows (iMovable* movable, iShadowBlockList* shadows,
	csVector3& origin)
{
  iSector* isect = movable->GetSector (0);
  iShadowBlock* list = shadows->NewShadowBlock (
  	isect, isect->GetRecLevel (),
  	polygons.Length ());
  csFrustum* frust;
  int i, j;
  csPolygon3D* p;
  bool cw = true; //@@@ Use mirroring parameter here!
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    p = polygons.Get (i);
    //if (p->GetPlane ()->VisibleFromPoint (origin) != cw) continue;
    float clas = p->GetPlane ()->GetWorldPlane ().Classify (origin);
    if (ABS (clas) < EPSILON) continue;
    if ((clas <= 0) != cw) continue;

    csPlane3 pl = p->GetPlane ()->GetWorldPlane ();
    pl.DD += origin * pl.norm;
    pl.Invert ();
    frust = list->AddShadow (origin, (void*)p, p->GetVertices ().GetVertexCount (),
	pl);
    for (j = 0 ; j < p->GetVertices ().GetVertexCount () ; j++)
      frust->GetVertex (j).Set (p->Vwor (j)-origin);
  }
}

void csThing::CreateBoundingBox ()
{
  float minx, miny, minz, maxx, maxy, maxz;
  delete bbox; bbox = NULL;
  if (num_vertices <= 0 && num_curve_vertices <= 0) return;
  bbox = new csThingBBox ();
  minx = 100000000.;
  miny = 100000000.;
  minz = 100000000.;
  maxx = -100000000.;
  maxy = -100000000.;
  maxz = -100000000.;
  int i;
  for (i = 0 ; i < num_vertices ; i++)
  {
    if (obj_verts[i].x < minx) minx = obj_verts[i].x;
    else if (obj_verts[i].x > maxx) maxx = obj_verts[i].x;
    if (obj_verts[i].y < miny) miny = obj_verts[i].y;
    else if (obj_verts[i].y > maxy) maxy = obj_verts[i].y;
    if (obj_verts[i].z < minz) minz = obj_verts[i].z;
    else if (obj_verts[i].z > maxz) maxz = obj_verts[i].z;
  }
  for (i = 0 ; i < num_curve_vertices ; i++)
  {
    csVector3& cv = curve_vertices[i];
    if (cv.x < minx) minx = cv.x;
    else if (cv.x > maxx) maxx = cv.x;
    if (cv.y < miny) miny = cv.y;
    else if (cv.y > maxy) maxy = cv.y;
    if (cv.z < minz) minz = cv.z;
    else if (cv.z > maxz) maxz = cv.z;
  }

  bbox->i7 = AddVertex (minx, miny, minz);
  bbox->i3 = AddVertex (minx, miny, maxz);
  bbox->i5 = AddVertex (minx, maxy, minz);
  bbox->i1 = AddVertex (minx, maxy, maxz);
  bbox->i8 = AddVertex (maxx, miny, minz);
  bbox->i4 = AddVertex (maxx, miny, maxz);
  bbox->i6 = AddVertex (maxx, maxy, minz);
  bbox->i2 = AddVertex (maxx, maxy, maxz);
}

const csVector3& csThing::GetRadius ()
{
  if (!obj_bbox_valid)
  {
    csBox3 b;
    GetBoundingBox (b);
  }
  return obj_radius;
}

void csThing::GetBoundingBox (csBox3& box)
{
  if (obj_bbox_valid) { box = obj_bbox; return; }
  obj_bbox_valid = true;

  if (!bbox) CreateBoundingBox ();

  if (!obj_verts)
  {
    obj_bbox.Set (0, 0, 0, 0, 0, 0);
    box = obj_bbox;
    return;
  }
  csVector3 min_bbox, max_bbox;
  min_bbox = max_bbox = Vobj (bbox->i1);
  if (Vobj (bbox->i2).x < min_bbox.x) min_bbox.x = Vobj (bbox->i2).x;
  else if (Vobj (bbox->i2).x > max_bbox.x) max_bbox.x = Vobj (bbox->i2).x;
  if (Vobj (bbox->i2).y < min_bbox.y) min_bbox.y = Vobj (bbox->i2).y;
  else if (Vobj (bbox->i2).y > max_bbox.y) max_bbox.y = Vobj (bbox->i2).y;
  if (Vobj (bbox->i2).z < min_bbox.z) min_bbox.z = Vobj (bbox->i2).z;
  else if (Vobj (bbox->i2).z > max_bbox.z) max_bbox.z = Vobj (bbox->i2).z;
  if (Vobj (bbox->i3).x < min_bbox.x) min_bbox.x = Vobj (bbox->i3).x;
  else if (Vobj (bbox->i3).x > max_bbox.x) max_bbox.x = Vobj (bbox->i3).x;
  if (Vobj (bbox->i3).y < min_bbox.y) min_bbox.y = Vobj (bbox->i3).y;
  else if (Vobj (bbox->i3).y > max_bbox.y) max_bbox.y = Vobj (bbox->i3).y;
  if (Vobj (bbox->i3).z < min_bbox.z) min_bbox.z = Vobj (bbox->i3).z;
  else if (Vobj (bbox->i3).z > max_bbox.z) max_bbox.z = Vobj (bbox->i3).z;
  if (Vobj (bbox->i4).x < min_bbox.x) min_bbox.x = Vobj (bbox->i4).x;
  else if (Vobj (bbox->i4).x > max_bbox.x) max_bbox.x = Vobj (bbox->i4).x;
  if (Vobj (bbox->i4).y < min_bbox.y) min_bbox.y = Vobj (bbox->i4).y;
  else if (Vobj (bbox->i4).y > max_bbox.y) max_bbox.y = Vobj (bbox->i4).y;
  if (Vobj (bbox->i4).z < min_bbox.z) min_bbox.z = Vobj (bbox->i4).z;
  else if (Vobj (bbox->i4).z > max_bbox.z) max_bbox.z = Vobj (bbox->i4).z;
  if (Vobj (bbox->i5).x < min_bbox.x) min_bbox.x = Vobj (bbox->i5).x;
  else if (Vobj (bbox->i5).x > max_bbox.x) max_bbox.x = Vobj (bbox->i5).x;
  if (Vobj (bbox->i5).y < min_bbox.y) min_bbox.y = Vobj (bbox->i5).y;
  else if (Vobj (bbox->i5).y > max_bbox.y) max_bbox.y = Vobj (bbox->i5).y;
  if (Vobj (bbox->i5).z < min_bbox.z) min_bbox.z = Vobj (bbox->i5).z;
  else if (Vobj (bbox->i5).z > max_bbox.z) max_bbox.z = Vobj (bbox->i5).z;
  if (Vobj (bbox->i6).x < min_bbox.x) min_bbox.x = Vobj (bbox->i6).x;
  else if (Vobj (bbox->i6).x > max_bbox.x) max_bbox.x = Vobj (bbox->i6).x;
  if (Vobj (bbox->i6).y < min_bbox.y) min_bbox.y = Vobj (bbox->i6).y;
  else if (Vobj (bbox->i6).y > max_bbox.y) max_bbox.y = Vobj (bbox->i6).y;
  if (Vobj (bbox->i6).z < min_bbox.z) min_bbox.z = Vobj (bbox->i6).z;
  else if (Vobj (bbox->i6).z > max_bbox.z) max_bbox.z = Vobj (bbox->i6).z;
  if (Vobj (bbox->i7).x < min_bbox.x) min_bbox.x = Vobj (bbox->i7).x;
  else if (Vobj (bbox->i7).x > max_bbox.x) max_bbox.x = Vobj (bbox->i7).x;
  if (Vobj (bbox->i7).y < min_bbox.y) min_bbox.y = Vobj (bbox->i7).y;
  else if (Vobj (bbox->i7).y > max_bbox.y) max_bbox.y = Vobj (bbox->i7).y;
  if (Vobj (bbox->i7).z < min_bbox.z) min_bbox.z = Vobj (bbox->i7).z;
  else if (Vobj (bbox->i7).z > max_bbox.z) max_bbox.z = Vobj (bbox->i7).z;
  if (Vobj (bbox->i8).x < min_bbox.x) min_bbox.x = Vobj (bbox->i8).x;
  else if (Vobj (bbox->i8).x > max_bbox.x) max_bbox.x = Vobj (bbox->i8).x;
  if (Vobj (bbox->i8).y < min_bbox.y) min_bbox.y = Vobj (bbox->i8).y;
  else if (Vobj (bbox->i8).y > max_bbox.y) max_bbox.y = Vobj (bbox->i8).y;
  if (Vobj (bbox->i8).z < min_bbox.z) min_bbox.z = Vobj (bbox->i8).z;
  else if (Vobj (bbox->i8).z > max_bbox.z) max_bbox.z = Vobj (bbox->i8).z;
  obj_bbox.Set (min_bbox, max_bbox);
  box = obj_bbox;
  obj_radius = (max_bbox - min_bbox) * 0.5f;
}

void csThing::GetBoundingBox (iMovable* movable, csBox3& box)
{
  if (wor_bbox_movablenr != movable->GetUpdateNumber ())
  {
    // First make sure obj_bbox is valid.
    GetBoundingBox (box);
    wor_bbox_movablenr = movable->GetUpdateNumber ();
    // @@@ Maybe it would be better to really calculate the bounding box
    // here instead of just transforming the object space bounding box?
    csReversibleTransform mt = movable->GetFullTransform ();
    wor_bbox.StartBoundingBox (mt.This2Other (obj_bbox.GetCorner (0)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (1)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (2)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (3)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (4)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (5)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (6)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (7)));
  }
  box = wor_bbox;
}

//-------------------------------------------------------------------------

csMeshedPolygon* csThing::PolyMesh::GetPolygons ()
{
  if (!polygons)
  {
    int i;
    num = 0;
    const csPolygonArray& pol = scfParent->polygons;
    for (i = 0 ; i < scfParent->GetPolygonCount () ; i++)
    {
      csPolygon3D* p = pol.Get (i);
      if (!p->GetUnsplitPolygon () && p->flags.Check (CS_POLY_COLLDET))
        num++;
    }
  
    // Always allocate at least one polygon.
    polygons = new csMeshedPolygon [num ? num : 1];
    num = 0;
    for (i = 0 ; i < scfParent->GetPolygonCount () ; i++)
    {
      csPolygon3D* p = pol.Get (i);
      if (!p->GetUnsplitPolygon () && p->flags.Check (CS_POLY_COLLDET))
      {
        polygons[num].num_vertices = p->GetVertexCount ();
        polygons[num].vertices = p->GetVertexIndices ();
	num++;
      }
    }
  }
  return polygons;
}
void csThing::SetConvex (bool c)
{
  flags.Set (CS_ENTITY_CONVEX, c ? CS_ENTITY_CONVEX : 0);
  if (c)
  {
    if (center_idx == -1) center_idx = AddVertex (0, 0, 0);
    int i;
    float minx = 1000000000., miny = 1000000000., minz = 1000000000.;
    float maxx = -1000000000., maxy = -1000000000., maxz = -1000000000.;
    for (i = 0 ; i < num_vertices ; i++)
      if (i != center_idx)
      {
        if (obj_verts[i].x < minx) minx = obj_verts[i].x;
  	if (obj_verts[i].x > maxx) maxx = obj_verts[i].x;
        if (obj_verts[i].y < miny) miny = obj_verts[i].y;
  	if (obj_verts[i].y > maxy) maxy = obj_verts[i].y;
        if (obj_verts[i].z < minz) minz = obj_verts[i].z;
  	if (obj_verts[i].z > maxz) maxz = obj_verts[i].z;
      }
    obj_verts[center_idx].Set ((minx+maxx)/2, (miny+maxy)/2, (minz+maxz)/2);
    if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
      wor_verts[center_idx].Set ((minx+maxx)/2, (miny+maxy)/2, (minz+maxz)/2);
  }
}

void csThing::UpdateCurveTransform (const csReversibleTransform& movtrans)
{
  if (GetCurveCount () == 0) return;
  // since obj has changed (possibly) we need to tell all of our curves
  csReversibleTransform o2w = movtrans.GetInverse();
  for (int i = 0 ; i < GetCurveCount () ; i++)
  {
    csCurve* c = curves.Get (i);
    c->SetObject2World (&o2w);
  }
}


csPolygon3D* csThing::IntersectSphere (csVector3& center, float radius,
	float* pr)
{
  float d, min_d = radius;
  int i;
  csPolygon3D* p, * min_p = NULL;
  csVector3 hit;

  for (i = 0 ; i < polygons.Length () ; i++)
  {
    p = GetPolygon3D (i);
    const csPlane3& wpl = p->GetPlane ()->GetObjectPlane ();
    d = wpl.Distance (center);
    if (d < min_d && csMath3::Visible (center, wpl))
    {
      hit = -center;
      hit -= wpl.Normal ();
      hit *= d;
      hit += center;
      if (p->IntersectRay (center, hit))
      {
  	min_d = d;
  	min_p = p;
      }
    }
  }
  if (pr) *pr = min_d;
  return min_p;
}

/// The list of fog vertices
static DECLARE_GROWING_ARRAY (fog_verts, G3DFogInfo);

bool csThing::DrawCurves (iRenderView* rview, iMovable* movable,
	csZBufMode zMode)
{
  if (GetCurveCount () <= 0) return false;
  iCamera* icam = rview->GetCamera ();
  const csReversibleTransform& camtrans = icam->GetTransform ();

  csReversibleTransform movtrans;
  // Only get the transformation if this thing can move.
  bool can_move = false;
  if (movable && cfg_moving != CS_THING_MOVE_NEVER)
  {
    movtrans = movable->GetFullTransform ();
    can_move = true;
  }

  int i;
  int res=1;

  // Calculate tesselation resolution
  csVector3 wv = curves_center;
  csVector3 world_coord;
  if (can_move) world_coord = movtrans.This2Other (wv);
  else world_coord = wv;
  csVector3 camera_coord = camtrans.Other2This (world_coord);

  if (camera_coord.z >= SMALL_Z)
  {
    res=(int)(curves_scale/camera_coord.z);
  }
  else
    res=1000; // some big tesselation value...

  // Create the combined transform of object to camera by
  // combining object to world and world to camera.
  csReversibleTransform obj_cam = camtrans;
  if (can_move) obj_cam /= movtrans;
  rview->GetGraphics3D ()->SetObjectToCamera (&obj_cam);
  rview->GetGraphics3D ()->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zMode);

  // Base of the mesh.
  G3DTriangleMesh mesh;
  mesh.morph_factor = 0;
  mesh.num_vertices_pool = 1;
  mesh.do_mirror = icam->IsMirrored ();
  mesh.do_morph_texels = false;
  mesh.do_morph_colors = false;
  mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;

  // Loop over all curves
  csCurve* c;
  for (i = 0 ; i < GetCurveCount () ; i++)
  {
    c = curves.Get (i);

    // First get a bounding box in camera space.
    csBox3 cbox;
    csBox2 sbox;
    if (c->GetScreenBoundingBox (obj_cam, icam, cbox, sbox) < 0)
      continue;	// Not visible.
    int clip_portal, clip_plane, clip_z_plane;
    if (!rview->ClipBBox (sbox, cbox, clip_portal, clip_plane, clip_z_plane))
      continue;	// Not visible.

    // If we have a dirty lightmap recombine the curves and the shadow maps.
    bool updated_lm = c->RecalculateDynamicLights ();

    // Create a new tesselation reuse an old one.
    csCurveTesselated* tess = c->Tesselate (res);
    // If the lightmap was updated or the new tesselation doesn't yet
    // have a valid colors table we need to update colors here.
    if (updated_lm || !tess->AreColorsValid ())
      tess->UpdateColors (c->lightmap);

    // Setup the structure for DrawTriangleMesh.
    if (tess->GetVertexCount () > fog_verts.Limit ())
    {
      fog_verts.SetLimit (tess->GetVertexCount ());
    }

    c->GetMaterialWrapper ()->Visit ();
    mesh.mat_handle = c->GetMaterialHandle ();
    mesh.num_vertices = tess->GetVertexCount ();
    mesh.vertices[0] = tess->GetVertices ();
    mesh.texels[0] = tess->GetTxtCoords ();
    mesh.vertex_colors[0] = tess->GetColors ();
    mesh.num_triangles = tess->GetTriangleCount ();
    mesh.triangles = tess->GetTriangles ();
    mesh.clip_portal = clip_portal;
    mesh.clip_plane = clip_plane;
    mesh.clip_z_plane = clip_z_plane;
    mesh.vertex_fog = fog_verts.GetArray ();
    bool gouraud = !!c->lightmap;
    mesh.fxmode = CS_FX_COPY | (gouraud ? CS_FX_GOURAUD : 0);
    mesh.use_vertex_color = gouraud;
    if (mesh.mat_handle == NULL)
    {
      CsPrintf (MSG_STDOUT, "Warning! Curve without material!\n");
      continue;
    }
    rview->CalculateFogMesh (obj_cam, mesh);

    rview->GetGraphics3D ()->DrawTriangleMesh (mesh);
  }

  return true;//@@@ RETURN correct vis info
}

bool csThing::Draw (iRenderView* rview, iMovable* movable, csZBufMode zMode)
{
  if (flags.Check (CS_ENTITY_INVISIBLE)) return false;
  if (flags.Check (CS_ENTITY_CAMERA))
  {
    csOrthoTransform& trans = rview->GetCamera ()->GetTransform ();
    csVector3 old = trans.GetO2TTranslation ();
    trans.SetO2TTranslation (csVector3 (0));
    bool rc = DrawInt (rview, movable, zMode);
    trans.SetO2TTranslation (old);
    return rc;
  }
  else return DrawInt (rview, movable, zMode);
}

static int count_cull_node_notvis_behind;
// static int count_cull_node_vis_cutzplane;
static int count_cull_node_notvis_cbuffer;
static int count_cull_node_vis;

// @@@ This routine need to be cleaned up!!! It probably needs to
// be part of the class.

bool CullOctreeNode (csPolygonTree* tree, csPolygonTreeNode* node,
	const csVector3& pos, void* data)
{
  if (!node) return false;
  if (node->Type () != NODE_OCTREE) return true;

  csPlane3 far_plane;
  bool use_far_plane;

  int i;
  csOctree* otree = (csOctree*)tree;
  csOctreeNode* onode = (csOctreeNode*)node;

  csCBuffer* c_buffer;
  iRenderView* rview = (iRenderView*)data;
  static csPolygon2D persp;
  csVector3 array[7];
  csEngine* w = csEngine::current_engine;
  iCamera* icam = rview->GetCamera ();
  const csReversibleTransform& camtrans = icam->GetTransform ();

  //if (w->IsPVS ())
  //{
    // Test for PVS.
    //if (!onode->IsVisible ()) return false;
    //else if (w->IsPVSOnly ()) goto is_vis;
  //}

  c_buffer = w->GetCBuffer ();
  int num_array;
  use_far_plane = icam->GetFarPlane (far_plane);
  otree->GetConvexOutline (onode, pos, array, num_array, use_far_plane);

  if (num_array)
  {
    int nVert = MIN (6, num_array); // if we use a farplane we could have up to 7 corners, 
                                    // but the 7th we'll need not here

    csVector3 cam[7];
    // If all vertices are behind z plane then the node is
    // not visible. If some vertices are behind z plane then we
    // need to clip the polygon.
    // We also test the node against the view frustum here.
    int num_z_0 = 0;
    bool left = true, right = true, top = true, bot = true;
    float lx, rx, ty, by;
    rview->GetFrustum (lx, rx, ty, by);
    for (i = 0 ; i < nVert; i++)
    {
      cam[i] = camtrans.Other2This (array[i]);
      if (cam[i].z < SMALL_EPSILON) num_z_0++;
      if (left && cam[i].x >= cam[i].z * lx) left = false;
      if (right && cam[i].x <= cam[i].z * rx) right = false;
      if (top && cam[i].y >= cam[i].z * ty) top = false;
      if (bot && cam[i].y <= cam[i].z * by) bot = false;
    }
    if (left || right || top || bot) return false;

    if (num_z_0 == nVert)
    {
      // Node behind camera.
      count_cull_node_notvis_behind++;
      return false;
    }
    
    if (use_far_plane)
    {
      if (num_array == 7) // we havent transformed the 7th yet
       cam[6] = camtrans.Other2This (array[6]);
      for (i = 0 ; i < num_array ; i++)
        if (far_plane.Classify (cam[i]) > SMALL_EPSILON) 
	  break;
      if (i == num_array) return false;
    }
    
    persp.MakeEmpty ();
    if (num_z_0 == 0)
    {
      // No vertices are behind. Just perspective correct.
      for (i = 0 ; i < nVert ; i++)
        persp.AddPerspective (cam[i]);
    }
    else
    {
      // Some vertices are behind. We need to clip.
      csVector3 isect;
      int i1 = nVert-1;
      for (i = 0 ; i < nVert ; i++)
      {
        if (cam[i].z < SMALL_EPSILON)
	{
	  if (cam[i1].z < SMALL_EPSILON)
	  {
	    // Just skip vertex.
	  }
	  else
	  {
	    // We need to intersect and add intersection point.
	    csIntersect3::ZPlane (SMALL_EPSILON, cam[i], cam[i1], isect);
	    persp.AddPerspective (isect);
	  }
	}
	else
	{
	  if (cam[i1].z < SMALL_EPSILON)
	  {
	    // We need to intersect and add both intersection point and this point.
	    csIntersect3::ZPlane (SMALL_EPSILON, cam[i], cam[i1], isect);
	    persp.AddPerspective (isect);
	  }
	  // Just perspective correct and add to the 2D polygon.
	  persp.AddPerspective (cam[i]);
	}
        i1 = i;
      }
    }

    if (!persp.ClipAgainst (rview->GetClipper ())) return false;

    // c-buffer test.
    bool vis = c_buffer->TestPolygon (persp.GetVertices (),
      	persp.GetVertexCount ());
    if (!vis)
    {
      count_cull_node_notvis_cbuffer++;
      return false;
    }
  }

  count_cull_node_vis++;
  // If a node is visible we check wether or not it has a minibsp.
  // If it has a minibsp then we need to transform all vertices used
  // by that minibsp to camera space.
  csVector3* cam;
  int* indices;
  int num_indices;
  if (onode->GetMiniBspVerts ())
  {
    // Here we get the polygon set as the static thing from the sector itself.
    csThing* pset = otree->GetThing ();
    cam = pset->GetCameraVertices (camtrans, icam->GetCameraNumber ());
    indices = onode->GetMiniBspVerts ();
    num_indices = onode->GetMiniBspVertexCount ();
    for (i = 0 ; i < num_indices ; i++)
      cam[indices[i]] = camtrans.Other2This (pset->Vwor (indices[i]));
  }

  return true;
}

/**
 * This is data that is created by the visibility culler
 * and registered to the current recursion level (as render
 * context data). The data contains the queue of all visible
 * polygons. Since visibility testing and drawing is now seperate
 * we need to remember everything in a queue. Since it is also
 * possible that between visibility testing and drawing other
 * objects (or even this thing again!) can be called we have to
 * attach this queue to the current recursion level.
 */
struct csPolygonVisInfo : public iBase
{
  csPolygon2DQueue* poly_queue;
  csPolygonVisInfo (int num);
  virtual ~csPolygonVisInfo () { delete poly_queue; }
  DECLARE_IBASE;
};

IMPLEMENT_IBASE (csPolygonVisInfo)
  IMPLEMENTS_INTERFACE (iBase)
IMPLEMENT_IBASE_END

csPolygonVisInfo::csPolygonVisInfo (int num)
{
  CONSTRUCT_IBASE (NULL);
  poly_queue = new csPolygon2DQueue (num);
}

void* csThing::TestQueuePolygons (csThing* thing,
  csPolygonInt** polygon, int num, bool /*same_plane*/, void* data)
{
  iRenderView* d = (iRenderView*)data;
  csPolygonVisInfo* pvi = (csPolygonVisInfo*)(d->FindRenderContextData (
  	(void*)thing));
  if (!pvi)
  {
    printf ("INTERNAL ERROR! polygon queue is missing!!!\n");
    fflush (stdout);
    DEBUG_BREAK;
  }
  return csThing::TestQueuePolygonArray (polygon, num, d, pvi->poly_queue,
    d->GetEngine ()->IsPVS ());
}

void csThing::DrawPolygonsFromQueue (csPolygon2DQueue* queue,
  iRenderView* rview)
{
  csPolygon3D* poly3d;
  csPolygon2D* poly2d;
  csPoly2DPool* render_pool = csEngine::current_engine->render_pol2d_pool;
  const csReversibleTransform& camtrans = rview->GetCamera ()->GetTransform ();
  while (queue->Pop (&poly3d, &poly2d))
  {
    poly3d->UpdateTransformation (camtrans,
    	rview->GetCamera ()->GetCameraNumber ());
    poly3d->GetPlane ()->WorldToCamera (camtrans, poly3d->Vcam (0));
    DrawOnePolygon (poly3d, poly2d, rview, CS_ZBUF_FILL);
    render_pool->Free (poly2d);
  }
}

void* csThing::DrawPolygons (csThing* /*thing*/,
  csPolygonInt** polygon, int num, bool /*same_plane*/, void* data)
{
  iRenderView* d = (iRenderView*)data;
  csThing::DrawPolygonArray (polygon, num, d, CS_ZBUF_FILL);
  return NULL;
}

bool csThing::DrawInt (iRenderView* rview, iMovable* movable, csZBufMode zMode)
{
  Prepare ();

  iCamera* icam = rview->GetCamera ();
  const csReversibleTransform& camtrans = icam->GetTransform ();
  csReversibleTransform movtrans;
  // Only get the transformation if this thing can move.
  bool can_move = false;
  if (movable && cfg_moving != CS_THING_MOVE_NEVER)
  {
    movtrans = movable->GetFullTransform ();
    can_move = true;
  }

  //@@@ Ok?
  cached_movable = movable;
  WorUpdate ();

  draw_busy++;

  // If this thing has a static tree we follow a totally different procedure
  // to draw it.
  if (static_tree)
  {
    int engine_mode = csEngine::current_engine->GetEngineMode ();
    // If this thing has a static polygon tree (octree) there
    // are three possibilities.
    if (engine_mode == CS_ENGINE_FRONT2BACK)
    {
      csPolygonVisInfo* pvi = (csPolygonVisInfo*)(
      	rview->FindRenderContextData ((void*)this));
      if (!pvi)
      {
        printf ("INTERNAL ERROR! polygon queue is missing!!!\n");
        fflush (stdout);
        DEBUG_BREAK;
      }
      // Render all polygons that are visible back to front.
      DrawPolygonsFromQueue (pvi->poly_queue, rview);
      rview->DeleteRenderContextData ((void*)this);
    }
    else if (engine_mode == CS_ENGINE_BACK2FRONT)
    {
      //-----
      // Here we don't use the c-buffer or 2D culler but just render back
      // to front.
      //-----
      static_tree->Back2Front (camtrans.GetOrigin (), &DrawPolygons,
      	(void*)rview);
    }
    else
    {
      //-----
      // Here we render using the Z-buffer.
      //-----
      csOctree* otree = (csOctree*)static_tree;
      csPolygonIntArray& unsplit = otree->GetRoot ()->GetUnsplitPolygons (); 
      DrawPolygonArray (unsplit.GetPolygons (), unsplit.GetPolygonCount (),
    	  rview, CS_ZBUF_USE);
    }
  }
  else
  {
    // This thing has no static tree
    UpdateTransformation (camtrans, icam->GetCameraNumber ());

    Stats::polygons_considered += polygons.Length ();

    DrawCurves (rview, movable, zMode);

    int res=1;

    // Calculate tesselation resolution
    // TODO should depend on a point in space, and a scale param in place of 40
    if (num_vertices>0)
    {
      csVector3 wv = wor_verts[0];
      csVector3 world_coord;
      if (can_move) world_coord = movtrans.This2Other (wv);
      else world_coord = wv;
      csVector3 camera_coord = camtrans.Other2This (world_coord);
  
      if (camera_coord.z > 0.0001)
      {
        res=(int)(40.0/camera_coord.z);
        if (res<1) res=1;
        else if (res>9) res=9;
      }
    }

    if (flags.Check (CS_ENTITY_DETAIL))
      DrawPolygonArrayDPM (polygons.GetArray (), polygons.Length (), rview,
      	zMode);
    else
      DrawPolygonArray (polygons.GetArray (), polygons.Length (), rview,
      	zMode);
  }

  draw_busy--;
  return true;	// @@@@ RETURN correct vis info
}

bool csThing::DrawFoggy (iRenderView* d, iMovable* movable)
{
  draw_busy++;
  iCamera* icam = d->GetCamera ();
  const csReversibleTransform& camtrans = icam->GetTransform ();
  UpdateTransformation (camtrans, icam->GetCameraNumber ());
  csPolygon3D* p;
  csVector3* verts;
  int num_verts;
  int i;
  csPoly2DPool* render_pool = csEngine::current_engine->render_pol2d_pool;
  csPolygon2D* clip;

  // @@@ Wouldn't it be nice if we checked all vertices against the Z plane?
  {
    csVector2 orig_triangle[3];
    d->GetGraphics3D ()->OpenFogObject (GetID (), &GetFog ());
    Stats::polygons_considered += polygons.Length ();

    icam->SetMirrored (!icam->IsMirrored ());
    for (i = 0 ; i < polygons.Length () ; i++)
    {
      p = GetPolygon3D (i);
      if (p->flags.Check (CS_POLY_NO_DRAW)) continue;
      clip = (csPolygon2D*)(render_pool->Alloc ());
      const csPlane3& wplane = p->GetPlane ()->GetWorldPlane ();
      bool front = csMath3::Visible (camtrans.GetOrigin (), wplane);

      csPlane3 clip_plane, *pclip_plane;
      bool do_clip_plane = d->GetClipPlane (clip_plane);
      if (do_clip_plane) pclip_plane = &clip_plane;
      else pclip_plane = NULL;
      if (!front &&
        p->ClipToPlane (pclip_plane, camtrans.GetOrigin (), verts, num_verts, false) &&
        p->DoPerspective (camtrans, verts, num_verts, clip, orig_triangle, icam->IsMirrored ()) &&
        clip->ClipAgainst (d->GetClipper ()))
      {
        p->GetPlane ()->WorldToCamera (camtrans, verts[0]);

        Stats::polygons_drawn++;

        clip->AddFogPolygon (d->GetGraphics3D (), p, p->GetPlane (),
		icam->IsMirrored (),
	  	GetID (), CS_FOG_BACK);
      }
      render_pool->Free (clip);
    }
    icam->SetMirrored (!icam->IsMirrored ());
    for (i = 0 ; i < polygons.Length () ; i++)
    {
      p = GetPolygon3D (i);
      if (p->flags.Check (CS_POLY_NO_DRAW)) continue;
      clip = (csPolygon2D*)(render_pool->Alloc ());
      const csPlane3& wplane = p->GetPlane ()->GetWorldPlane ();
      bool front = csMath3::Visible (camtrans.GetOrigin (), wplane);

      csPlane3 clip_plane, *pclip_plane;
      bool do_clip_plane = d->GetClipPlane (clip_plane);
      if (do_clip_plane) pclip_plane = &clip_plane;
      else pclip_plane = NULL;
      if (front &&
        p->ClipToPlane (pclip_plane, camtrans.GetOrigin (), verts, num_verts, true) &&
        p->DoPerspective (camtrans, verts, num_verts, clip, orig_triangle,
		icam->IsMirrored ()) &&
        clip->ClipAgainst (d->GetClipper ()))
      {
        p->GetPlane ()->WorldToCamera (camtrans, verts[0]);

        Stats::polygons_drawn++;

	clip->AddFogPolygon (d->GetGraphics3D (), p, p->GetPlane (),
		icam->IsMirrored (),
	  	GetID (), CS_FOG_FRONT);
      }
      render_pool->Free (clip);
    }
    d->GetGraphics3D ()->CloseFogObject (GetID ());
  }

  draw_busy--;
  return true;	// @@@@ RETURN correct vis info
}

void csThing::RegisterVisObject (iVisibilityObject* visobj)
{
  csVisObjInfo* vinf = new csVisObjInfo ();
  vinf->visobj = visobj;
  iShadowCaster* shadcast = QUERY_INTERFACE (visobj, iShadowCaster);
  if (shadcast) shadcast->DecRef ();
  vinf->shadcast = shadcast;
  vinf->bbox = new csPolyTreeBBox ();
  // Initialize the movable and shape numbers with a number different
  // from the one that the objects currently have so that we know
  // we will add the object to the tree when we first need it.
  vinf->last_movablenr = visobj->GetMovable ()->GetUpdateNumber ()-1;
  vinf->last_shapenr = visobj->GetShapeNumber ()-1;
  visobjects.Push (vinf);
  visobj->IncRef ();
}

void csThing::UnregisterVisObject (iVisibilityObject* visobj)
{
  int idx;
  csVisObjInfo* vinf = NULL;
  for (idx = 0 ; idx < visobjects.Length () ; idx++)
  {
    vinf = (csVisObjInfo*)visobjects[idx];
    if (vinf->visobj == visobj) break;
    vinf = NULL;
  }
  if (vinf == NULL) return;
  visobjects.Delete (idx);
  delete vinf->bbox;
  delete vinf;
  visobj->DecRef ();
}

void csThing::CheckVisUpdate (csVisObjInfo* vinf)
{
  iVisibilityObject* visobj = vinf->visobj;
  iMovable* movable = visobj->GetMovable ();
  long movablenr = movable->GetUpdateNumber ();
  long shapenr = visobj->GetShapeNumber ();
  if (movablenr != vinf->last_movablenr || shapenr != vinf->last_shapenr)
  {
    vinf->last_movablenr = movablenr;
    vinf->last_shapenr = shapenr;
    csPolyTreeBBox* bbox = vinf->bbox;

    // If object has not been placed in a sector we do nothing here.
    if (!movable->InSector ()) return;

    csBox3 b;
    visobj->GetBoundingBox (b);

    csReversibleTransform trans = movable->GetFullTransform ().GetInverse ();
    bbox->Update (b, trans, vinf);

    // Temporarily increase reference to prevent free.
    bbox->GetBaseStub ()->IncRef ();
    static_tree->AddObject (bbox);
    bbox->GetBaseStub ()->DecRef ();
  }
}

bool csThing::VisTest (iRenderView* irview)
{
  if (!static_tree) return false;
  iEngine* iengine = irview->GetEngine ();
  iCamera* icam = irview->GetCamera ();
  int engine_mode = iengine->GetEngineMode ();

  // If this thing has a static polygon tree (octree) there
  // are three possibilities.
  if (engine_mode == CS_ENGINE_FRONT2BACK)
  {
    //-----
    // In this part of the rendering we use the c-buffer or another
    // 2D/3D visibility culler.
    //-----

    csOrthoTransform& camtrans = icam->GetTransform ();
    const csVector3& origin = camtrans.GetOrigin ();

    int i;
    for (i = 0 ; i < visobjects.Length () ; i++)
    {
      csVisObjInfo* vinf = (csVisObjInfo*)visobjects[i];
      // First update visibility information if object has moved or
      // changed shape fundamentally.
      CheckVisUpdate (vinf);
      iVisibilityObject* vo = vinf->visobj;

      iMeshWrapper* mw = QUERY_INTERFACE_FAST (vo, iMeshWrapper);
      mw->DecRef ();
      if (mw->GetMeshObject () == &scfiMeshObject)
      {
        // If the object represents the object of the culler then
        // the object is automatically visible.
	vo->MarkVisible ();
      }
      else
      {
        // If the current viewpoint is inside the bounding box of the
        // object then we consider the object visible.
        csBox3 bbox;
        vo->GetBoundingBox (bbox);
        if (bbox.In (origin))
          vo->MarkVisible ();
        else
          vo->MarkInvisible ();
      }
      vinf->bbox->ClearTransform ();
    }

    // Using the PVS, mark all sectors and polygons that are visible
    // from the current node.
    //if (engine->IsPVS ())
    //{
      //csOctree* otree = (csOctree*)static_tree;
      //@@@if (engine->IsPVSFrozen ())
	//@@@otree->MarkVisibleFromPVS (engine->GetFrozenPosition ());
      //@@@else
      //otree->MarkVisibleFromPVS (origin);
    //}

    // Initialize a queue on which all visible polygons will be pushed.
    // The octree is traversed front to back but we want to render
    // back to front. That's one of the reasons for this queue.
    csPolygonVisInfo* pvi = new csPolygonVisInfo (GetPolygonCount ());
    irview->AttachRenderContextData ((void*)this, (iBase*)pvi);

    // Update the transformation for the static tree. This will
    // not actually transform all vertices from world to camera but
    // it will make sure that when a node (octree node) is visited,
    // the transformation will happen at that time.
    UpdateTransformation (camtrans, icam->GetCameraNumber ());

    // Traverse the tree front to back and push all visible polygons
    // on the queue. This traversal will also mark all visible
    // meshes and things. They will be put on a queue later.
    static_tree->Front2Back (origin, &TestQueuePolygons,
      	irview, CullOctreeNode, irview);
    return true;
  }
  else if (engine_mode == CS_ENGINE_BACK2FRONT)
  {
    //-----
    // Here we don't use the c-buffer or 2D culler but just render back
    // to front.
    //-----
    UpdateTransformation (icam->GetTransform (), icam->GetCameraNumber ());
    return false;
  }
  else
  {
    //-----
    // Here we render using the Z-buffer.
    //-----
    UpdateTransformation (icam->GetTransform (), icam->GetCameraNumber ());
    return false;
  }
}

void csThing::RegisterShadowReceiver (iShadowReceiver* receiver)
{
}

void csThing::UnregisterShadowReceiver (iShadowReceiver* receiver)
{
}

struct CheckFrustData
{
  iFrustumView* fview;
  csHashSet visible_things;
};

static int frust_cnt = 50;

//@@@ Needs to be part of sector?
static void CompressShadowFrustums (iShadowBlockList* list)
{
  iShadowIterator* shadow_it = list->GetShadowIterator (true);
  csFrustum* sf;
  if (!shadow_it->HasNext ()) { shadow_it->DecRef (); return; }

  csCBufferCube* cb = csEngine::current_engine->GetCBufCube ();
  cb->MakeEmpty ();

  iSector* cur_sector = shadow_it->GetNextShadowBlock ()->GetSector ();
  int cur_draw_busy = shadow_it->GetNextShadowBlock ()->GetRecLevel ();
  while (shadow_it->HasNext ())
  {
    iShadowBlock* shadlist = shadow_it->GetNextShadowBlock ();
    sf = shadow_it->Next ();
    if (shadlist->GetSector () != cur_sector || shadlist->GetRecLevel () != cur_draw_busy)
      break;
    bool vis = cb->InsertPolygon (sf->GetVertices (), sf->GetVertexCount ());
    if (!vis)
      shadow_it->DeleteCurrent ();
  }

  shadow_it->DecRef ();
}

//@@@ Needs to be part of sector?
static void* CheckFrustumPolygonsFB (csThing* thing,
  csPolygonInt** polygon, int num, bool /*same_plane*/, void* data)
{
  iThingState* ithing = &(thing->scfiThingState);

  csPolygon3D* p;
  CheckFrustData* fdata = (CheckFrustData*)data;
  iFrustumView* fview = fdata->fview;
  csFrustumContext* ctxt = fview->GetFrustumContext ();
  iShadowBlockList* shadows = ctxt->GetShadows ();
  csVector3& center = ctxt->GetLightFrustum ()->GetOrigin ();
  csCBufferCube* cb = csEngine::current_engine->GetCBufCube ();
  bool cw = true;	// @@@ Mirror flag?
  int i, j;
  for (i = 0 ; i < num ; i++)
  {
    csVector3 poly[128];	// @@@ HARDCODED! BAD!

    if (polygon[i]->GetType () == 3)
    {
      // A BSP polygon. Used for testing visibility of things.
      csBspPolygon* bsppol = (csBspPolygon*)polygon[i];
      csVisObjInfo* obj = bsppol->GetOriginator ();
      iThingState* ith = QUERY_INTERFACE (obj->visobj, iThingState);
      if (ith)
      {
        ith->DecRef ();
	if (!fdata->visible_things.In (ith))
	{
	  csPolyIndexed& pi = bsppol->GetPolygon ();
	  csPolyTreeBBox* pi_par = bsppol->GetParent ();
	  csVector3Array& verts = pi_par->GetVertices ();
          for (j = 0 ; j < pi.GetVertexCount () ; j++)
            poly[j] = verts[pi[j]]-center;
	  bool vis = cb->TestPolygon (poly, pi.GetVertexCount ());
	  if (vis)
  	  {
	    if (fview->ThingShadowsEnabled ())
	    {
	      // The thing is visible and we want things to cast
	      // shadows. So we add all shadows generated by this
	      // thing to the shadow list.
	      if (ith != ithing)
	      {
	        csThing* th = (csThing*)(ith->GetPrivateObject ());
		// @@@ UGLY!!!: flags are in mesh wrapper!!?
		if (fview->CheckShadowMask (th->flags.Get ()))
	          th->AppendShadows (obj->visobj->GetMovable (),
		  	shadows, center);
	      }
	    }
	    fdata->visible_things.AddNoTest (ith);
	  }
	}
      }
      continue;
    }
    if (polygon[i]->GetType () != 1) continue;
    p = (csPolygon3D*)polygon[i];

    // Polygons that are merged with the octree have world==obj space.
    for (j = 0 ; j < p->GetVertexCount () ; j++)
      poly[j] = p->Vobj (j)-center;
    bool vis = false;

    float clas = p->GetPlane ()->GetWorldPlane ().Classify (center);
    if (ABS (clas) < EPSILON) continue;
    if ((clas <= 0) != cw) continue;

    if (p->GetPortal ())
    {
      vis = cb->TestPolygon (poly, p->GetVertexCount ());
    }
    else
    {
      vis = cb->InsertPolygon (poly, p->GetVertexCount ());
    }
    if (vis)
    {
      fview->CallPolygonFunction ((csObject*)p);

      if (!shadows->GetLastShadowBlock ())
      {
        shadows->NewShadowBlock ();
      }
      csPlane3 pl = p->GetPlane ()->GetWorldPlane ();
      pl.DD += center * pl.norm;
      pl.Invert ();
      csFrustum* frust = shadows->GetLastShadowBlock ()->
      		AddShadow (center, (void*)p,
		p->GetVertices ().GetVertexCount (), pl);
      // Polygons that are merged with the octree have world==obj space.
      for (j = 0 ; j < p->GetVertices ().GetVertexCount () ; j++)
        frust->GetVertex (j).Set (p->Vobj (j)-center);
      frust_cnt--;
      if (frust_cnt < 0)
      {
        frust_cnt = 200;
        CompressShadowFrustums (shadows);
      }
    }
  }
  return NULL;
}

// @@@ This routine need to be cleaned up!!! It needs to
// be part of the class.
// @@@ This function needs to use the PVS. However, this function itself
// is used for the PVS so we need to take care!
static bool CullOctreeNodeLighting (csPolygonTree* tree, csPolygonTreeNode* node,
  const csVector3& /*pos*/, void* data)
{
  if (!node) return false;
  if (node->Type () != NODE_OCTREE) return true;

  csOctree* otree = (csOctree*)tree;
  csOctreeNode* onode = (csOctreeNode*)node;
  iFrustumView* fview = (iFrustumView*)data;

  csFrustumContext* ctxt = fview->GetFrustumContext ();
  const csVector3& center = ctxt->GetLightFrustum ()->GetOrigin ();
  csVector3 bmin = onode->GetMinCorner ()-center;
  csVector3 bmax = onode->GetMaxCorner ()-center;

  // Calculate the distance between (0,0,0) and the box.
  csVector3 result (0,0,0);
  if (bmin.x > 0) result.x = bmin.x;
  else if (bmax.x < 0) result.x = -bmax.x;
  if (bmin.y > 0) result.y = bmin.y;
  else if (bmax.y < 0) result.y = -bmax.y;
  if (bmin.z > 0) result.z = bmin.z;
  else if (bmax.z < 0) result.z = -bmax.z;
  float dist = result.Norm ();
  float radius = fview->GetRadius ();
  if (radius < dist)
  {
    return false;
  }

  if (ABS (dist) < EPSILON)
  {
    // We are in the node.
    fview->CallNodeFunction (onode);
    return true;
  }

  // Test node against quad-tree.
  csVector3 outline[6];
  int num_outline;
  otree->GetConvexOutline (onode, center, outline, num_outline);
  if (num_outline > 0)
  {
    int i;
    for (i = 0 ; i < num_outline ; i++)
      outline[i] -= center;
    csCBufferCube* cb = csEngine::current_engine->GetCBufCube ();
    if (cb && !cb->TestPolygon (outline, num_outline))
      return false;
  }
  fview->CallNodeFunction (onode);
  return true;
}

void csThing::CastShadows (iFrustumView* fview)
{
  if (!static_tree) return;
  // If there is a static tree (BSP and/or octree) then we
  // go front to back and add shadows to the list while we are doing
  // that. In future I would like to add some extra culling stage
  // here using a quad-tree or something similar (for example six
  // quad-trees arranged in a cube around the light).

  CheckFrustData fdata;
  fdata.fview = fview;

  // Translate this sector so that it is oriented around
  // the position of the light (position of the light becomes
  // the new origin).
  csVector3& center = fview->GetFrustumContext ()->GetLightFrustum ()->
  	GetOrigin ();

  // All visible things will cause shadows to be added to 'lview'.
  // Later on we'll restore these shadows.
  frust_cnt = 50;
  static_tree->Front2Back (center, CheckFrustumPolygonsFB, (void*)&fdata,
      	CullOctreeNodeLighting, (void*)fview);
  frust_cnt = 50;

  // Calculate lighting for all things in this sector.
  // The 'visible_things' hashset that is in fdata will contain
  // all things that are found visible while traversing the octree.
  // This queue is filled while traversing the octree
  // (CheckFrustumPolygonsFB).
  csHashIterator it (fdata.visible_things.GetHashMap ());
  csObject* o;
  // @@@ THIS SHOULD USE THE SHADOW RECEIVERS!!!
  while (it.HasNext ())
  {
    o = (csObject*)(it.Next ());
    csMeshWrapper* mesh = (csMeshWrapper*)o;
    // @@@ should not be known in engine.
    // @@@ UGLY
    iThingState* ithing = QUERY_INTERFACE (mesh->GetMeshObject (), iThingState);
    if (ithing)
    {
      csThing* sp = (csThing*)(ithing->GetPrivateObject ());
      // Only if the thing has right flags do we consider it for shadows.
      if (fview->CheckProcessMask (mesh->flags.Get ()))
        sp->RealCheckFrustum (fview, &(mesh->GetMovable ().scfiMovable));
      ithing->DecRef ();
    }
  }
}

void csThing::CheckFrustum (iFrustumView* lview, iMovable* movable)
{
  csCBufferCube* cb = csEngine::current_engine->GetCBufCube ();
  cb->MakeEmpty ();
  RealCheckFrustum (lview, movable);
}

void csThing::RealCheckFrustum (iFrustumView* lview, iMovable* movable)
{
  //@@@ Ok?
  cached_movable = movable;
  WorUpdate ();

  csPolygon3D* p;
  int i;

  draw_busy++;

  if (light_frame_number != current_light_frame_number)
  {
    light_frame_number = current_light_frame_number;
    lview->GetFrustumContext ()->SetFirstTime (true);
  }
  else lview->GetFrustumContext ()->SetFirstTime (false);

  for (i = 0 ; i < polygons.Length () ; i++)
  {
    p = GetPolygon3D (i);
    lview->CallPolygonFunction ((csObject*)p);
  }

  for (i = 0 ; i < GetCurveCount () ; i++)
  {
    csCurve* c = curves.Get (i);
    
    if (!lview->IsDynamic ())
    {
      csReversibleTransform o2w = movable->GetFullTransform ().GetInverse();
      c->SetObject2World (&o2w);
    }
    
    lview->CallCurveFunction ((csObject*)c);
  }

  draw_busy--;
}

void csThing::InitializeDefault ()
{
  Prepare ();
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
    polygons.Get (i)->InitializeDefault ();
  for (i = 0 ; i < GetCurveCount () ; i++)
    curves.Get (i)->InitializeDefault ();
}

bool csThing::ReadFromCache (int id)
{
  Prepare ();
  if (id == 0) id = thing_id;
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
    if (!polygons.Get (i)->ReadFromCache (id))
      return false;
  for (i = 0 ; i < GetCurveCount () ; i++)
    if (!curves.Get (i)->ReadFromCache (id))
      return false;
  return true;
}

bool csThing::WriteToCache (int id)
{
  if (id == 0) id = thing_id;
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
    if (!polygons.Get (i)->WriteToCache (id))
      return false;
  for (i = 0 ; i < GetCurveCount () ; i++)
    if (!curves.Get (i)->WriteToCache (id))
      return false;
  return true;
}

void csThing::PrepareLighting ()
{
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
    polygons.Get (i)->PrepareLighting ();
  for (i = 0 ; i < GetCurveCount () ; i++)
    curves.Get (i)->PrepareLighting ();
}

void csThing::Merge (csThing* other)
{
  int i, j;
  int* merge_vertices = new int [other->GetVertexCount ()+1];
  for (i = 0 ; i < other->GetVertexCount () ; i++)
    merge_vertices[i] = AddVertex (other->Vwor (i));

  for (i = 0 ; i < other->polygons.Length () ; i++)
  {
    csPolygon3D* p = other->GetPolygon3D (i);
    int* idx = p->GetVertices ().GetVertexIndices ();
    for (j = 0 ; j < p->GetVertices ().GetVertexCount () ; j++)
      idx[j] = merge_vertices[idx[j]];
    AddPolygon (p);
    other->polygons[i] = NULL;
  }

  for (i = 0 ; i < other->GetCurveVertexCount () ; i++)
    AddCurveVertex (other->CurveVertex (i), other->CurveTexel (i));

  for (i = 0 ; i < other->curves.Length () ; i++)
  {
    csCurve* c = other->GetCurve (i);
    AddCurve (c);
    other->curves[i] = NULL;
  }

  delete [] merge_vertices;
}

void csThing::MergeTemplate (iThingState* tpl,
	iMaterialWrapper* default_material,
	csVector3* shift, csMatrix3* transform)
{
  int i, j;
  int* merge_vertices;

  flags.SetAll (tpl->GetFlags ().Get ());
  //@@@ OBSOLETE: SetZBufMode (tpl->GetZBufMode ());
  SetMovingOption (tpl->GetMovingOption ());

  //TODO should merge? take averages or something?
  curves_center = tpl->GetCurvesCenter ();
  curves_scale = tpl->GetCurvesScale ();
  //@@@ TEMPORARY
  iThingState* ith = QUERY_INTERFACE (tpl, iThingState);
  ParentTemplate = (csThing*)(ith->GetPrivateObject ());
  ith->DecRef ();

  merge_vertices = new int [tpl->GetVertexCount ()+1];
  for (i = 0 ; i < tpl->GetVertexCount () ; i++)
  {
    csVector3 v;
    v = tpl->GetVertex (i);
    if (transform) v = *transform * v;
    if (shift) v += *shift;
    merge_vertices[i] = AddVertex (v);
  }

  for (i = 0 ; i < tpl->GetPolygonCount () ; i++)
  {
    iPolygon3D* pt = tpl->GetPolygon (i);
    csPolygon3D* p;
    csMaterialWrapper* mat = NULL;
    if (pt->GetMaterial ())
      mat = pt->GetMaterial ()->GetPrivateObject (); //@@@
    p = NewPolygon (mat); //@@@
    p->SetName (pt->QueryObject()->GetName ());
    iMaterialWrapper* wrap = pt->GetMaterial ();
    if (!wrap && default_material)
      p->SetMaterial (default_material->GetPrivateObject ());
    int* idx = pt->GetVertexIndices ();
    for (j = 0 ; j < pt->GetVertexCount () ; j++)
      p->AddVertex (merge_vertices[idx[j]]);

    p->flags.SetAll (pt->GetFlags ().Get ());
    p->CopyTextureType (pt);
  }

  for (i = 0; i < tpl->GetCurveVertexCount (); i++)
  {
    csVector3 v = tpl->CurveVertex (i);
    if (transform) v = *transform * v;
    if (shift) v += *shift;
    AddCurveVertex (v, tpl->CurveTexel (i));
  }

  for (i = 0; i < tpl->GetCurveCount (); i++)
  {
    iCurve* orig_curve = tpl->GetCurve (i);
    iCurve* p = CreateCurve (orig_curve->GetParentTemplate ());
    p->QueryObject()->SetName (orig_curve->QueryObject()->GetName ());
    if (orig_curve->GetMaterial ())
      p->SetMaterial (orig_curve->GetMaterial ());
    else
      p->SetMaterial (default_material);
  }

  delete [] merge_vertices;
}

void csThing::ReplaceMaterials (iMaterialList* matList, const char* prefix)
{
  int i;
  for (i = 0; i < GetPolygonCount (); i++)
  {
    csPolygon3D *p = GetPolygon3D (i);
    const char* txtname = p->GetMaterialWrapper ()->GetName ();
    char* newname = new char [strlen (prefix) + strlen (txtname) + 2];
    sprintf (newname, "%s_%s", prefix, txtname);
    iMaterialWrapper *th = matList->FindByName (newname);
    if (th != NULL)
      p->SetMaterial (th->GetPrivateObject ());
    delete [] newname;
  }
}

//---------------------------------------------------------------------------

iCurve *csThing::ThingState::GetCurve (int idx)
{
  csCurve* c = scfParent->GetCurve (idx);
  return &(c->scfiCurve);
}

iPolygon3D *csThing::ThingState::GetPolygon (int idx)
{
  csPolygon3D* p = scfParent->GetPolygon3D (idx);
  return &(p->scfiPolygon3D);
}

iPolygon3D *csThing::ThingState::GetPolygon (const char* name)
{
  csPolygon3D* p = scfParent->GetPolygon3D (name);
  return &(p->scfiPolygon3D);
}

iPolygon3D *csThing::ThingState::CreatePolygon (const char *iName)
{
  csPolygon3D *p = new csPolygon3D ((csMaterialWrapper *)NULL);
  if (iName) p->SetName (iName);
  scfParent->AddPolygon (p);
  iPolygon3D *ip = QUERY_INTERFACE (p, iPolygon3D);
  p->DecRef ();
  return ip;
}

//---------------------------------------------------------------------------

iMeshObjectFactory* csThing::MeshObject::GetFactory () const
{
  if (!scfParent->ParentTemplate) return NULL;
  return &scfParent->ParentTemplate->scfiMeshObjectFactory;
}

//---------------------------------------------------------------------------

iMeshObject* csThing::MeshObjectFactory::NewInstance ()
{
  csThing* thing = new csThing (scfParent);
  thing->MergeTemplate (&(scfParent->scfiThingState), NULL);
  return &thing->scfiMeshObject;
}

//---------------------------------------------------------------------------

IMPLEMENT_IBASE (csThingObjectType)
  IMPLEMENTS_INTERFACE (iMeshObjectType)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csThingObjectType)

EXPORT_CLASS_TABLE (thing)
  EXPORT_CLASS (csThingObjectType, "crystalspace.mesh.object.thing",
    "Crystal Space Thing Mesh Type")
EXPORT_CLASS_TABLE_END

csThingObjectType::csThingObjectType (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csThingObjectType::~csThingObjectType ()
{
}

bool csThingObjectType::Initialize (iSystem* pSystem)
{
  iSCF::SCF->RegisterStaticClass (thing_scfInitialize (iSCF::SCF));
  System = pSystem;
  return true;
}

iMeshObjectFactory* csThingObjectType::NewFactory ()
{
  csThing* cm = new csThing (this);
  iMeshObjectFactory* ifact = QUERY_INTERFACE (cm, iMeshObjectFactory);
  ifact->DecRef ();
  return ifact;
}

//---------------------------------------------------------------------------

