/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
  
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
#include "csengine/polyset.h"
#include "csengine/polygon.h"
#include "csengine/polytmap.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/engine.h"
#include "csengine/camera.h"
#include "csengine/sector.h"
#include "csengine/curve.h"
#include "csengine/rview.h"
#include "csengine/stats.h"
#include "csengine/dumper.h"
#include "csengine/cbuffer.h"
#include "csengine/quadtr3d.h"
#include "csengine/thing.h"
#include "csengine/covtree.h"
#include "csengine/bspbbox.h"
#include "csengine/cssprite.h"
#include "csengine/bsp.h"
#include "csengine/keyval.h"
#include "csgeom/polypool.h"
#include "igraph3d.h"
#include "itxtmgr.h"

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csPolygonSet,csPObject);

IMPLEMENT_IBASE_EXT (csPolygonSet)
  IMPLEMENTS_EMBEDDED_INTERFACE (iPolygonSet)
  IMPLEMENTS_EMBEDDED_INTERFACE (iPolygonMesh)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE(csPolygonSet::PolySet)
  IMPLEMENTS_INTERFACE(iPolygonSet)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_EMBEDDED_IBASE(csPolygonSet::PolyMesh)
  IMPLEMENTS_INTERFACE(iPolygonMesh)
IMPLEMENT_EMBEDDED_IBASE_END

long csPolygonSet::current_light_frame_number = 0;

csPolygonSet::csPolygonSet (csEngine* engine) : csPObject(),
  polygons (64, 64), curves (16, 16)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiPolygonSet);
  CONSTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  max_vertices = num_vertices = 0;

  curves_center.x = curves_center.y = curves_center.z = 0;
  curves_scale=40;  
  curve_vertices = NULL;
  curve_texels = NULL;
  num_curve_vertices = max_curve_vertices = 0;

  wor_verts = NULL;
  obj_verts = NULL;
  cam_verts = NULL;
  draw_busy = 0;
  fog.enabled = false;
  bbox = NULL;
  obj_bbox_valid = false;

  light_frame_number = -1;

  cam_verts_set.SetTransformationManager (&engine->tr_manager);

  csPolygonSet::engine = engine;
}

csPolygonSet::~csPolygonSet ()
{
  delete [] wor_verts;
  delete [] obj_verts;
  delete [] curve_vertices;
  delete [] curve_texels;
  delete bbox;
}

void csPolygonSet::Prepare (csSector* sector)
{
  int i;
  csPolygon3D* p;
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    p = polygons.Get (i);
    p->Finish (sector);
  }
}

int csPolygonSet::AddCurveVertex (csVector3& v, csVector2& t)
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
    memcpy (new_vertices, curve_vertices, sizeof (csVector3)*num_curve_vertices);
    memcpy (new_texels,   curve_texels,   sizeof (csVector2)*num_curve_vertices);
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

int csPolygonSet::AddVertex (float x, float y, float z)
{
  if (!wor_verts)
  {
    max_vertices = 10;
    wor_verts = new csVector3 [max_vertices];
    obj_verts = new csVector3 [max_vertices];
  }
  while (num_vertices >= max_vertices)
  {
    if (max_vertices < 10000)
      max_vertices *= 2;
    else
      max_vertices += 10000;
    csVector3* new_wor_verts = new csVector3 [max_vertices];
    csVector3* new_obj_verts = new csVector3 [max_vertices];
    memcpy (new_wor_verts, wor_verts, sizeof (csVector3)*num_vertices);
    memcpy (new_obj_verts, obj_verts, sizeof (csVector3)*num_vertices);

    delete [] wor_verts;
    delete [] obj_verts;

    wor_verts = new_wor_verts;
    obj_verts = new_obj_verts;
  }

  // By default all vertices are set with the same object space and world space.
  wor_verts[num_vertices].Set (x, y, z);
  obj_verts[num_vertices].Set (x, y, z);
  num_vertices++;
  return num_vertices-1;
}

int csPolygonSet::AddVertexSmart (float x, float y, float z)
{
  if (!wor_verts) { AddVertex (x, y, z); return 0; }
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

void csPolygonSet::CompressVertices ()
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
  csVector3* new_wor = new csVector3 [count_unique];
  new_obj[0] = obj_verts[vt[0].orig_idx];
  new_wor[0] = wor_verts[vt[0].orig_idx];
  vt[0].new_idx = 0;
  j = 1;
  for (i = 1 ; i < num_vertices ; i++)
  {
    if (vt[i].new_idx == i)
    {
      new_obj[j] = obj_verts[vt[i].orig_idx];
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
  delete [] wor_verts;
  delete [] obj_verts;
  wor_verts = new_wor;
  obj_verts = new_obj;
  num_vertices = max_vertices = count_unique;

  // Now we can remap the vertices in all polygons.
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolygon3D* p = polygons.Get (i);
    csPolyIndexed& pi = p->GetVertices ();
    int* idx = pi.GetVertexIndices ();
    for (j = 0 ; j < pi.GetNumVertices () ; j++)
      idx[j] = vt[idx[j]].new_idx;
  }

  delete [] vt;

  // If there is a bounding box we recreate it.
  if (bbox) CreateBoundingBox ();
}

csPolygonInt* csPolygonSet::GetPolygonInt (int idx)
{
  return (csPolygonInt*)GetPolygon3D (idx);
}

csPolygon3D* csPolygonSet::GetPolygon3D (const char* name)
{
  int idx = polygons.FindKey (name);
  return idx >= 0 ? polygons.Get (idx) : NULL;
}

iPolygon3D *csPolygonSet::PolySet::GetPolygon (int idx)
{
  return QUERY_INTERFACE(scfParent->GetPolygon3D (idx), iPolygon3D);
}

csPolygon3D* csPolygonSet::NewPolygon (csMaterialWrapper* material)
{
  csPolygon3D* p = new csPolygon3D (material);
  AddPolygon (p);
  return p;
}

iPolygon3D *csPolygonSet::PolySet::CreatePolygon (const char *iName)
{
  csPolygon3D *p = new csPolygon3D ((csMaterialWrapper *)NULL);
  p->SetName (iName);
  scfParent->AddPolygon (p);
  iPolygon3D *ip = QUERY_INTERFACE (p, iPolygon3D);
  p->DecRef ();
  return ip;
}

void csPolygonSet::AddPolygon (csPolygonInt* poly)
{
  ((csPolygon3D *)poly)->SetParent (this);
  polygons.Push ((csPolygon3D *)poly);
}

csCurve* csPolygonSet::GetCurve (char* name)
{
  int idx = curves.FindKey (name);
  return idx >= 0 ? curves.Get (idx) : NULL;
}

void csPolygonSet::AddCurve (csCurve* curve)
{
  curve->SetParent (this);
  curves.Push (curve);
}

void csPolygonSet::HardTransform (const csReversibleTransform& t)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
  {
    obj_verts[i] = t.This2Other (obj_verts[i]);
    wor_verts[i] = obj_verts[i];
  }
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolygon3D* p = GetPolygon3D (i);
    p->HardTransform (t);
  }
}

csPolygon3D* csPolygonSet::IntersectSegment (const csVector3& start, 
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

void csPolygonSet::DrawOnePolygon (csPolygon3D* p, csPolygon2D* poly,
	csRenderView* d, bool use_z_buf)
{
  if (d->callback)
  {
    d->callback (d, CALLBACK_POLYGON, (void*)p);
    d->callback (d, CALLBACK_POLYGON2D, (void*)poly);
  }

  if (d->added_fog_info)
  {
    // If fog info was added then we are dealing with vertex fog and
    // the current sector has fog. This means we have to complete the
    // fog_info structure with the plane of the current polygon.
    d->fog_info->outgoing_plane = p->GetPlane ()->GetCameraPlane ();
  }

  Stats::polygons_drawn++;

  csPortal* po = p->GetPortal ();
  if (csSector::do_portals && po)
  {
    bool filtered = false;
    // is_this_fog is true if this sector is fogged.
    bool is_this_fog = d->this_sector->HasFog ();

    // If there is filtering (alpha mapping or something like that) we need
    // to keep the texture plane so that it can be drawn after the sector has
    // been drawn. The texture plane needs to be kept because this polygon
    // may be rendered again (through mirrors) possibly overwriting the plane.
    csPolyPlane* keep_plane = NULL;

    if (d->g3d->GetRenderState (G3DRENDERSTATE_TRANSPARENCYENABLE))
      filtered = p->IsTransparent ();
              
    if (filtered || is_this_fog || (po && po->flags.Check (CS_PORTAL_ZFILL)))
    {
      keep_plane = new csPolyPlane (*(p->GetPlane ()));
    }

    // Draw through the portal. If this fails we draw the original polygon
    // instead. Drawing through a portal can fail because we have reached
    // the maximum number that a sector is drawn (for mirrors).
    if (po->Draw (poly, p, *d))
    {
      if (!d->callback)
      {
	if (filtered) poly->DrawFilled (d, p, keep_plane, use_z_buf);
	if (is_this_fog) poly->AddFogPolygon (d->g3d, p, keep_plane,
		d->IsMirrored (), d->this_sector->GetID (), CS_FOG_BACK);
	// Here we z-fill the portal contents to make sure that sprites
	// that are drawn outside of this portal cannot accidently cross
	// into the others sector space (we cannot trust the Z-buffer here).
	if (po->flags.Check (CS_PORTAL_ZFILL))
	  poly->FillZBuf (d, p, keep_plane);
      }
    }
    else if (!d->callback)
      poly->DrawFilled (d, p, p->GetPlane (), use_z_buf);

    // Cleanup.
    if (keep_plane) keep_plane->DecRef ();
  }
  else if (!d->callback)
    poly->DrawFilled (d, p, p->GetPlane (), use_z_buf);
}

void csPolygonSet::DrawPolygonArray (csPolygonInt** polygon, int num,
	csRenderView* d, bool use_z_buf)
{
  csPolygon3D* p;
  csVector3* verts;
  int num_verts;
  int i;
  csPoly2DPool* render_pool = d->engine->render_pol2d_pool;
  csPolygon2D* clip;
  
  for (i = 0 ; i < num ; i++)
  {
    if (polygon[i]->GetType () != 1) continue;
    p = (csPolygon3D*)polygon[i];
    if (p->flags.Check (CS_POLY_NO_DRAW)) continue;
    p->CamUpdate ();
    if (p->ClipToPlane (d->do_clip_plane ? &d->clip_plane : (csPlane3*)NULL,
	 	d->GetOrigin (), verts, num_verts)) //@@@Use pool for verts?
    {
      if (!d->UseFarPlane() || d->GetFarPlane()->ClipPolygon (verts, num_verts))
      {
        clip = (csPolygon2D*)(render_pool->Alloc ());
	if (p->DoPerspective (*d, verts, num_verts, clip, NULL, d->IsMirrored ()) &&
            clip->ClipAgainst (d->view))
        {
          p->GetPlane ()->WorldToCamera (*d, verts[0]);
          DrawOnePolygon (p, clip, d, use_z_buf);
        }
        render_pool->Free (clip);
      }
    }
  }
}

void csPolygonSet::DrawPolygonArrayDPM (csPolygonInt** /*polygon*/, int /*num*/,
	csRenderView* d, bool use_z_buf)
{
  // @@@ We should include object 2 world transform here too like it
  // happens with sprites.
  int i;
  csReversibleTransform tr_o2c = (*d);
  d->g3d->SetObjectToCamera (&tr_o2c);
  d->g3d->SetClipper (d->view->GetClipPoly (), d->view->GetNumVertices ());
  // @@@ This should only be done when aspect changes...
  d->g3d->SetPerspectiveAspect (d->GetFOV ());
  d->g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE,
    use_z_buf ? CS_ZBUF_USE : CS_ZBUF_FILL);

  G3DPolygonMesh mesh;
  mesh.num_vertices = GetNumVertices ();
  mesh.num_polygons = GetNumPolygons ();
  mesh.master_mat_handle = NULL;
  // @@@ It would be nice if we could avoid this allocate.
  // Even nicer would be if we didn't have to copy the data
  // to this structure every time. Maybe hold this array native
  // in every detail object?
  // IMPORTANT OPT!!! CACHE THIS ARRAY IN EACH ENTITY!
  mesh.polygons = new csPolygonDPM [GetNumPolygons ()];
  mesh.mat_handle = new iMaterialHandle* [GetNumPolygons ()];
  mesh.normal = new csPlane3 [GetNumPolygons ()];
  mesh.plane = new G3DTexturePlane [GetNumPolygons ()];
  mesh.poly_texture = new iPolygonTexture* [GetNumPolygons ()];
  for (i = 0 ; i < GetNumPolygons () ; i++)
  {
    csPolygon3D* p = GetPolygon3D (i);

    // Vertices.
    int num_v = p->GetNumVertices ();
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
  mesh.do_mirror = d->IsMirrored ();
  mesh.vertex_mode = G3DPolygonMesh::VM_WORLDSPACE;
  mesh.vertices = wor_verts;
  mesh.vertex_fog = NULL;

  // @@@ FAR plane not yet supported here!
  // @@@ fog not supported yet.
  // @@@ clipping not supported yet.

  if (!d->callback)
    d->g3d->DrawPolygonMesh (mesh);
  //else
  // @@@ Provide functionality for visible edges here...

cleanup:
  delete [] mesh.polygons;
  delete [] mesh.mat_handle;
  delete [] mesh.plane;
  delete [] mesh.normal;
  delete [] mesh.poly_texture;
}

void* csPolygonSet::TestQueuePolygonArray (csPolygonInt** polygon, int num,
	csRenderView* d, csPolygon2DQueue* poly_queue, bool pvs)
{
  csPolygon3D* p;
  csPortal* po;
  csVector3* verts;
  int num_verts;
  int i, j;
  csCBuffer* c_buffer = d->engine->GetCBuffer ();
  csCoverageMaskTree* covtree = d->engine->GetCovtree ();
  csQuadTree3D* quad3d = d->engine->GetQuad3D ();
  bool visible;
  csPoly2DPool* render_pool = d->engine->render_pol2d_pool;
  csPolygon2D* clip;
  
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
      csObject* obj = bsppol->GetOriginator ();
      bool obj_vis;
      csPolyTreeBBox* tbb;
      if (obj->GetType () >= csSprite::Type)
      {
        csSprite* sp = (csSprite*)obj;
	obj_vis = sp->IsVisible ();
	tbb = (csPolyTreeBBox*)(sp->GetPolyTreeObject ());
      }
      else
      {
        csThing* th = (csThing*)obj;
	obj_vis = th->IsVisible ();
	tbb = (csPolyTreeBBox*)(th->GetPolyTreeObject ());
      }

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
	  tbb->World2Camera (*d);
	}

	// Culling.
	bool mark_vis = false;
	if (quad3d)
	{
	  clip = NULL;
	  csVector3 test_poly[128];	// @@@ Bad hardcoded limit.
	  csPolyTreeBBox* par = bsppol->GetParent ();
	  csVector3Array& verts = par->GetVertices ();
	  csPolyIndexed& pi = bsppol->GetPolygon ();
	  for (j = 0 ; j < pi.GetNumVertices () ; j++)
	    test_poly[j] = verts[pi[j]]-quad3d->GetCenter ();
	  csBox3 bbox;
//@@@ BF CULLING
	  int rc = quad3d->TestPolygon (test_poly, pi.GetNumVertices (), bbox);
	  mark_vis = (rc != CS_QUAD3D_NOCHANGE);
	}
	else
	{
          clip = (csPolygon2D*)(render_pool->Alloc ());
          if ( bsppol->ClipToPlane (d->do_clip_plane ? &d->clip_plane :
		  (csPlane3*)NULL, d->GetOrigin (), verts, num_verts))
	  {
	    if (!d->UseFarPlane	() || d->GetFarPlane ()->ClipPolygon (verts, num_verts))   
	    {
               if (bsppol->DoPerspective (*d, verts, num_verts, clip, d->IsMirrored ()) 
	          && clip->ClipAgainst (d->view) )
               {
	         if (covtree)
	         {
	           if (covtree->TestPolygon (clip->GetVertices (),
	    	      clip->GetNumVertices (), clip->GetBoundingBox ()))
	           mark_vis = true;
	         }
	         else if (c_buffer->TestPolygon (clip->GetVertices (),
	  	   clip->GetNumVertices ()))
	          mark_vis = true;
              }
	    }
	  }    
	}
	if (mark_vis)
	{
          if (obj->GetType () >= csSprite::Type)
          {
            csSprite* sp = (csSprite*)obj;
            sp->MarkVisible ();
	  }
	  else
	  {
            csThing* th = (csThing*)obj;
            th->MarkVisible ();
	  }
	}
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
        if (!p->IsVisible ())
        {
          // Polygon is not visible because of PVS.
          //@@@ CURRENTLY DISABLED continue;
        }
      }

      visible = false;
      clip = NULL;
      // Culling.
      if (p->flags.Check (CS_POLY_NO_DRAW))
      {
        // Don't draw this polygon.
      }
      else if (quad3d && !d->engine->IsPVSOnly ())
      {
	csPlane3* wplane = p->GetPolyPlane ();
	float cl = wplane->Classify (quad3d->GetCenter ());
    	if (cl > 0)
	  visible = false;
	else
	{
	  csVector3 test_poly[128];	// @@@ Bad hardcoded limit.
	  for (j = 0 ; j < p->GetNumVertices () ; j++)
	    test_poly[j] = p->Vwor (j)-quad3d->GetCenter ();
	  csBox3 bbox;
          po = p->GetPortal ();
	  int rc = quad3d->InsertPolygon (test_poly, p->GetNumVertices (), bbox);
	  visible = (rc != CS_QUAD3D_NOCHANGE);
	}
      }
      else
      {
        clip = (csPolygon2D*)(render_pool->Alloc ());
        if (
         p->ClipToPlane (d->do_clip_plane ? &d->clip_plane : (csPlane3*)NULL,
                            d->GetOrigin (), verts, num_verts)
	 && (!d->UseFarPlane () || d->GetFarPlane ()->ClipPolygon (verts, num_verts))		    
         && p->DoPerspective (*d, verts, num_verts, clip, NULL,
                              d->IsMirrored ())
         && clip->ClipAgainst (d->view))
        {
          po = p->GetPortal ();
	  if (d->engine->IsPVSOnly ())
            visible = true;
	  if (covtree)
            visible = covtree->InsertPolygon (clip->GetVertices (),
		    clip->GetNumVertices (), clip->GetBoundingBox ());
	  else
            visible = c_buffer->InsertPolygon (clip->GetVertices (),
		    clip->GetNumVertices ());
        }
      }

      if (visible && !clip)
      {
	// If visible and we don't already have a clip (i.e. we did 3D culling)
	// then we need to make it here. It is still possible that the
	// polygon will be culled at this stage.
        clip = (csPolygon2D*)(render_pool->Alloc ());
        if (!(
           p->ClipToPlane (d->do_clip_plane ? &d->clip_plane : (csPlane3*)NULL,
                              d->GetOrigin (), verts, num_verts)
           && (!d->UseFarPlane () || d->GetFarPlane ()->ClipPolygon (verts, num_verts))		    
           && p->DoPerspective (*d, verts, num_verts, clip, NULL,
                                d->IsMirrored ())
           && clip->ClipAgainst (d->view)))
	{
	  visible = false;
	}
      }

      if (visible)
      {
        poly_queue->Push (p, clip);
	Stats::polygons_accepted++;
	if (quad3d && quad3d->IsFull ()) return (void*)1;	// End BSP processing
	if (c_buffer && c_buffer->IsFull ()) return (void*)1;	// End BSP processing
	if (covtree && covtree->IsFull ()) return (void*)1;	// End BSP processing
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

void csPolygonSet::GetCameraMinMaxZ (float& minz, float& maxz)
{
  int i;
  minz = maxz = Vcam (0).z;
  for (i = 1 ; i < num_vertices ; i++)
  {
    if (minz > Vcam (i).z) minz = Vcam (i).z;
    else if (maxz < Vcam (i).z) maxz = Vcam (i).z;
  }
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
csFrustumList* csPolygonSet::GetShadows (csSector* sector, csVector3& origin)
{
  csFrustumList* list = new csFrustumList ();
  csShadowFrustum* frust;
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

    frust = new csShadowFrustum (origin);
    frust->sector = sector;
    frust->draw_busy = sector->draw_busy;
    list->AddFirst (frust);
    csPlane3 pl = p->GetPlane ()->GetWorldPlane ();
    pl.DD += origin * pl.norm;
    pl.Invert ();
    frust->SetBackPlane (pl);
    frust->polygon = p;
    for (j = 0 ; j < p->GetVertices ().GetNumVertices () ; j++)
      frust->AddVertex (p->Vwor (j)-origin);
  }
  return list;
}

void csPolygonSet::CreateBoundingBox ()
{
  float minx, miny, minz, maxx, maxy, maxz;
  delete bbox; bbox = NULL;
  if (num_vertices <= 0 && num_curve_vertices <= 0) return;
  bbox = new csPolygonSetBBox ();
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

const csVector3& csPolygonSet::GetRadius ()
{
  if (!obj_bbox_valid)
  {
    csBox3 b;
    GetBoundingBox (b);
  }
  return obj_radius;
}

void csPolygonSet::GetBoundingBox (csBox3& box)
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

//---------------------------------------------------------------------------------------------------------
/*
 * Ken Clarkson wrote this.  Copyright (c) 1996 by AT&T..
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

int ccw (csVector2 *P[], int i, int j, int k)
{
  double a = P[i]->x - P[j]->x,
	 b = P[i]->y - P[j]->y,
	 c = P[k]->x - P[j]->x,
	 d = P[k]->y - P[j]->y;
  return a*d - b*c <= 0;	   /* true if points i, j, k counterclockwise */
}

int cmpl (const void *a, const void *b)
{
  float v;
  csVector2 **av,**bv;
  av = (csVector2 **)a;
  bv = (csVector2 **)b;

  v = (*av)->x - (*bv)->x;

  if (v>0) return 1;
  if (v<0) return -1;

  v = (*bv)->y - (*av)->y;

  if (v>0) return 1;
  if (v<0) return -1;

  return 0;
}

int cmph (const void *a, const void *b) { return cmpl (b, a); }

int make_chain (csVector2 *V[], int n, int (*cmp)(const void*, const void*)) 
{
  int i, j, s = 1;
  csVector2 *t;

  qsort (V, n, sizeof (csVector2*), cmp);
  for (i = 2; i < n; i++) 
  {
    for (j=s ; j>=1 && ccw(V, i, j, j-1) ; j--) ;
    s = j+1;
    t = V[s]; V[s] = V[i]; V[i] = t;
  }
  return s;
}

int ch2d (csVector2 *P[], int n)
{
  int u = make_chain (P, n, cmpl);		/* make lower hull */
  if (!n) return 0;
  P[n] = P[0];
  return u+make_chain (P+u, n-u+1, cmph);	/* make upper hull */
}

/*
 * Someone from Microsoft wrote this, but copyrights are absent.
 * So I assume that it's also in public domain ;). However, it's not
 * a very critical function, I just was very lazy and didn't want to
 * write my own Convex-Hull finder ;). -DDK
 */

void Find2DConvexHull (int nverts, csVector2 *pntptr, int *cNumOutIdxs, int **OutHullIdxs)
{
  csVector2 **PntPtrs;
  int i;

  *cNumOutIdxs=0;               //max space needed is n+1 indices
  *OutHullIdxs = (int *)malloc((nverts+1)*(sizeof(int)+sizeof(csVector2 *)));

  PntPtrs = (csVector2**) &(*OutHullIdxs)[nverts+1];

  // alg requires array of ptrs to verts (for qsort) instead of array of verts, so do the conversion
  for (i=0 ; i < nverts ; i++)
    PntPtrs[i] = &pntptr[i];

  *cNumOutIdxs=ch2d (PntPtrs, nverts);

  // convert back to array of idxs
  for (i = 0; i < *cNumOutIdxs; i++)
    (*OutHullIdxs)[i]= (int) (PntPtrs[i]-&pntptr[0]);

  // caller will free returned array
}

//---------------------------------------------------------------------------------------------------------

int find_chull (int nverts, csVector2 *vertices, csVector2 *&chull)
{
  int num;
  int *order;

  Find2DConvexHull (nverts, vertices, &num, &order);

  chull = new csVector2[num];
  for (int i = 0; i < num; i++)
    chull[i]=vertices[order[i]];

  free (order);

  return num;
}

#define EPS	0.00001

csVector2* csPolygonSet::IntersectCameraZPlane (float z,csVector2* /*clipper*/,
  int /*num_clip*/, int &num_vertices)
{
  int i, j, n, num_pts=0;

  struct point_list
  {
    csVector2 data;
    point_list *next;
  } list,*head=&list,*prev;

  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D *p = polygons.Get (i);
    int nv = p->GetVertices ().GetNumVertices ();
    int *v_i = p->GetVertices ().GetVertexIndices ();

    for (j=0,n=1 ; j<nv ; j++,n=(n+1)%nv)
    {
      csVector3 _1=Vcam(v_i[j]);
      csVector3 _2=Vcam(v_i[n]);

      if (_1.z > _2.z) { csVector3 _=_1; _1=_2; _2=_; }
      if (_2.z-_1.z < EPS) continue;

      float _1z=_1.z-z;
      float _2z=_2.z-z;

      if (_1z<=0.0 && _2z>=0.0)
      {
	float t=_1z/(_1z-_2z);
	float x_=_1.x+t*(_2.x-_1.x);
	float y_=_1.y+t*(_2.y-_1.y);

	num_pts++;

	head->data.x=x_;
	head->data.y=y_;

	head->next=new point_list;
	head=head->next;
      }
    }
  }

  csVector2 *final_data,*data;
  data=new csVector2[num_pts];
  for (head = &list, i = 0; i < num_pts; i++)
  {
    data[i]=head->data;

    prev=head;
    head=head->next;
    if (prev!=&list) { delete prev; }
  }

  if (i) { delete[] head; }

  num_vertices = find_chull (num_pts, data, final_data);
  if (num_pts) { delete data; }

  return final_data;
}

//---------------------------------------------------------------------------------------------------------

bool csPolygonSet::PolySet::CreateKey (const char *iName, const char *iValue)
{
  scfParent->ObjAdd (new csKeyValuePair (iName, iValue));
  return true;
}

//---------------------------------------------------------------------------------------------------------

csMeshedPolygon* csPolygonSet::PolyMesh::GetPolygons ()
{
  if (!polygons)
  {
    int i;
    num = 0;
    const csPolygonArray& pol = scfParent->polygons;
    for (i = 0 ; i < scfParent->GetNumPolygons () ; i++)
    {
      csPolygon3D* p = pol.Get (i);
      if (!p->GetUnsplitPolygon () && p->flags.Check (CS_POLY_COLLDET))
        num++;
    }
  
    polygons = new csMeshedPolygon [num];
    num = 0;
    for (i = 0 ; i < scfParent->GetNumPolygons () ; i++)
    {
      csPolygon3D* p = pol.Get (i);
      if (!p->GetUnsplitPolygon () && p->flags.Check (CS_POLY_COLLDET))
      {
        polygons[num].num_vertices = p->GetNumVertices ();
        polygons[num].vertices = p->GetVertexIndices ();
	num++;
      }
    }
  }
  return polygons;
}
