/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein

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
#include "csengine/covcube.h"
#include "csengine/curve.h"
#include "csengine/texture.h"
#include "csengine/material.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "itexture.h"
#include "qint.h"

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csThing,csPolygonSet);

IMPLEMENT_IBASE_EXT (csThing)
  IMPLEMENTS_EMBEDDED_INTERFACE (iThing)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csThing::eiThing)
  IMPLEMENTS_INTERFACE (iThing)
IMPLEMENT_EMBEDDED_IBASE_END

csThing::csThing (csEngine* engine, bool is_sky, bool is_template) :
	csPolygonSet (engine), tree_bbox (NULL), movable ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiThing);
  movable.scfParent = &scfiThing;
  center_idx = -1;
  ParentTemplate = NULL;
  tree_bbox.SetOwner (this);
  csThing::is_sky = is_sky;
  csThing::is_template = is_template;
  movable.SetObject (this);
  engine->AddToCurrentRegion (this);
}

csThing::~csThing ()
{
  if (is_sky)
    engine->UnlinkSky (this);
  else
    engine->UnlinkThing (this);
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
    wor_verts[center_idx].Set ((minx+maxx)/2, (miny+maxy)/2, (minz+maxz)/2);
  }
}

void csThing::UpdateMove ()
{
  int i;
  if (!bbox) CreateBoundingBox ();
  for (i = 0 ; i < num_vertices ; i++)
    wor_verts[i] = movable.GetTransform ().This2Other (obj_verts[i]);
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolygon3D* p = GetPolygon3D (i);
    p->ObjectToWorld (movable.GetTransform ());
  }
  UpdateCurveTransform ();
  UpdateInPolygonTrees ();
}

void csThing::MoveToSector (csSector* s)
{
  if (is_sky)
    s->skies.Push (this);
  else
    s->things.Push (this);
}

void csThing::RemoveFromSectors ()
{
  if (GetPolyTreeObject ())
    GetPolyTreeObject ()->RemoveFromTree ();
  int i;
  csVector& sectors = movable.GetSectors ();
  for (i = 0 ; i < sectors.Length () ; i++)
  {
    csSector* ss = (csSector*)sectors[i];
    if (ss)
    {
      if (is_sky)
      {
        int idx = ss->skies.Find (this);
        if (idx >= 0)
        {
          ss->skies[idx] = NULL;
          ss->skies.Delete (idx);
        }
      }
      else
      {
        int idx = ss->things.Find (this);
        if (idx >= 0)
        {
          ss->things[idx] = NULL;
          ss->things.Delete (idx);
        }
      }
    }
  }
}

void csThing::UpdateCurveTransform()
{
  // since obj has changed (possibly) we need to tell all of our curves
  for (int i = 0 ; i < GetNumCurves () ; i++)
  {
    csCurve* c = curves.Get (i);
 
    csReversibleTransform o2w = movable.GetTransform ().GetInverse();
    c->SetObject2World (&o2w);
  }
}


void csThing::CreateLightMaps (iGraphics3D* g3d)
{
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolygon3D* p = GetPolygon3D (i);
    p->CreateLightMaps (g3d);
  }
}

csPolygon3D* csThing::IntersectSphere (csVector3& center, float radius, float* pr)
{
  float d, min_d = radius;
  int i;
  csPolygon3D* p, * min_p = NULL;
  csVector3 hit;

  for (i = 0 ; i < polygons.Length () ; i++)
  {
    p = GetPolygon3D (i);
    const csPlane3& wpl = p->GetPlane ()->GetWorldPlane ();
    d = wpl.Distance (center);
    if (d < min_d && csMath3::Visible (center, wpl))
    {
      hit = -center;
      hit -= wpl.GetNormal ();
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

void csThing::DrawCurves (csRenderView& rview, bool use_z_buf)
{
  int i;
  int res=1;

  // Calculate tesselation resolution
  csVector3 wv = curves_center;
  csVector3 world_coord = movable.GetTransform ().This2Other (wv);
  csVector3 camera_coord = rview.Other2This (world_coord);

  if (camera_coord.z >= SMALL_Z)
  {
    res=(int)(curves_scale/camera_coord.z);
  }
  else
    res=1000; // some big tesselation value...

  // Create the combined transform of object to camera by
  // combining object to world and world to camera.
  csReversibleTransform obj_cam = rview;
  obj_cam /= movable.GetTransform ();
  rview.GetG3D ()->SetObjectToCamera (&obj_cam);
  rview.GetG3D ()->SetClipper (rview.GetView ()->GetClipPoly (), rview.GetView ()->GetNumVertices ());
  rview.GetG3D ()->SetPerspectiveAspect (rview.GetFOV ());
  rview.GetG3D ()->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE,
      use_z_buf ? CS_ZBUF_USE : CS_ZBUF_FILL);

  // Base of the mesh.
  G3DTriangleMesh mesh;
  mesh.morph_factor = 0;
  mesh.num_vertices_pool = 1;
  mesh.num_materials = 1;
  mesh.do_mirror = rview.IsMirrored ();
  mesh.do_morph_texels = false;
  mesh.do_morph_colors = false;
  mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;

  // Loop over all curves
  csCurve* c;
  for (i = 0 ; i < GetNumCurves () ; i++)
  {
    c = curves.Get (i);

    // Test visibility of entire curve by clipping bounding box against clipper.
    // There are three possibilities:
    //	1. box is not visible -> curve is not visible.
    //	2. box is entirely visible -> curve is visible and need not be clipped.
    //	3. box is partially visible -> curve is visible and needs to be clipped
    //	   if rview has do_clip_plane set to true.
    csBox2 bbox;
    if (c->GetScreenBoundingBox (obj_cam, rview, bbox) < 0) continue;	// Not visible.
    // Test if we need and should clip to the current portal.
    int box_class;
    box_class = rview.GetView ()->ClassifyBox (bbox);
    if (box_class == -1) continue; // Not visible.

    bool do_clip = false;
    if (rview.HasClipPlane () || rview.HasClipFrustum ())
    {
      if (box_class == 0) do_clip = true;
    }

    // If we don't need to clip to the current portal then we
    // test if we need to clip to the top-level portal.
    // Top-level clipping is always required unless we are totally
    // within the top-level frustum.
    // IF it is decided that we need to clip here then we still
    // clip to the inner portal. We have to do clipping anyway so
    // why not do it to the smallest possible clip area.
    if (!do_clip)
    {
      box_class = engine->top_clipper->ClassifyBox (bbox);
      if (box_class == 0) do_clip = true;
    }

    // If we have a dirty lightmap recombine the curves and the shadow maps.
    bool updated_lm = c->RecalculateDynamicLights ();

    // Create a new tesselation reuse an old one.
    csCurveTesselated* tess = c->Tesselate (res);
    // If the lightmap was updated or the new tesselation doesn't yet
    // have a valid colors table we need to update colors here.
    if (updated_lm || !tess->AreColorsValid ())
      tess->UpdateColors (c->lightmap);

    // Setup the structure for DrawTriangleMesh.
    if (tess->GetNumVertices () > fog_verts.Limit ())
    {
      fog_verts.SetLimit (tess->GetNumVertices ());
    }

    c->GetMaterialWrapper ()->Visit ();
    mesh.mat_handle[0] = c->GetMaterialHandle ();
    mesh.num_vertices = tess->GetNumVertices ();
    mesh.vertices[0] = tess->GetVertices ();
    mesh.texels[0][0] = tess->GetTxtCoords ();
    mesh.vertex_colors[0] = tess->GetColors ();
    mesh.num_triangles = tess->GetNumTriangles ();
    mesh.triangles = tess->GetTriangles ();
    mesh.do_clip = do_clip;
    mesh.vertex_fog = fog_verts.GetArray ();
    bool gouraud = !!c->lightmap;
    mesh.fxmode = CS_FX_COPY | (gouraud ? CS_FX_GOURAUD : 0);
    mesh.use_vertex_color = gouraud;
    if (mesh.mat_handle[0] == NULL)
    {
      CsPrintf (MSG_STDOUT, "Warning! Curve without material!\n");
      continue;
    }
    rview.CalculateFogMesh (obj_cam, mesh);

    if (rview.GetCallback ())
      rview.CallCallback (CALLBACK_MESH, (void*)&mesh);
    else
      rview.GetG3D ()->DrawTriangleMesh (mesh);
  }
}

void csThing::Draw (csRenderView& rview, bool use_z_buf)
{
  if (flags.Check (CS_ENTITY_CAMERA))
  {
    csRenderView rview_new = rview;
    rview_new.SetO2TTranslation (csVector3 (0));
    DrawInt (rview_new, use_z_buf);
  }
  else DrawInt (rview, use_z_buf);
}

void csThing::DrawInt (csRenderView& rview, bool use_z_buf)
{
  draw_busy++;
  UpdateTransformation (rview);

  // @@@ Wouldn't it be nice if we checked if all vertices are behind the view plane?
  {
    if (rview.GetCallback ()) rview.CallCallback (CALLBACK_THING, (void*)this);
    Stats::polygons_considered += polygons.Length ();

    DrawCurves (rview, use_z_buf);

    int res=1;

    // Calculate tesselation resolution
    // TODO should depend on a point in space, and a scale param in place of 40
    if (num_vertices>0)
    {
      csVector3 wv = wor_verts[0];
      csVector3 world_coord = movable.GetTransform ().This2Other (wv);
      csVector3 camera_coord = rview.Other2This (world_coord);
  
      if (camera_coord.z > 0.0001)
      {
        res=(int)(40.0/camera_coord.z);
        if (res<1) res=1;
        else if (res>9) res=9;
      }
    }

    if (flags.Check (CS_ENTITY_DETAIL))
      DrawPolygonArrayDPM (polygons.GetArray (), polygons.Length (), &rview, use_z_buf);
    else
      DrawPolygonArray (polygons.GetArray (), polygons.Length (), &rview, use_z_buf);
    if (rview.GetCallback ()) rview.CallCallback (CALLBACK_THINGEXIT, (void*)this);
  }

  draw_busy--;
}

void csThing::DrawFoggy (csRenderView& d)
{
  draw_busy++;
  UpdateTransformation (d);
  csPolygon3D* p;
  csVector3* verts;
  int num_verts;
  int i;
  csPoly2DPool* render_pool = engine->render_pol2d_pool;
  csPolygon2D* clip;

  // @@@ Wouldn't it be nice if we checked all vertices against the Z plane?
  {
    csVector2 orig_triangle[3];
    if (!d.GetCallback ()) d.GetG3D ()->OpenFogObject (GetID (), &GetFog ());
    Stats::polygons_considered += polygons.Length ();

    d.SetMirrored (!d.IsMirrored ());
    for (i = 0 ; i < polygons.Length () ; i++)
    {
      p = GetPolygon3D (i);
      if (p->flags.Check (CS_POLY_NO_DRAW)) continue;
      clip = (csPolygon2D*)(render_pool->Alloc ());
      const csPlane3& wplane = p->GetPlane ()->GetWorldPlane ();
      bool front = csMath3::Visible (d.GetOrigin (), wplane);

      if (!front &&
        p->ClipToPlane (d.HasClipPlane () ? &d.GetClipPlane () : (csPlane3*)NULL, d.GetOrigin (),
              verts, num_verts, false) &&
        p->DoPerspective (d, verts, num_verts, clip, orig_triangle, d.IsMirrored ()) &&
        clip->ClipAgainst (d.GetView ()))
      {
        p->GetPlane ()->WorldToCamera (d, verts[0]);
	if (d.GetCallback ())
	{
          d.CallCallback (CALLBACK_POLYGON, (void*)p);
          d.CallCallback (CALLBACK_POLYGON2D, (void*)clip);
	}

        Stats::polygons_drawn++;

	if (!d.GetCallback ())
          clip->AddFogPolygon (d.GetG3D (), p, p->GetPlane (), d.IsMirrored (),
	  	GetID (), CS_FOG_BACK);
      }
      render_pool->Free (clip);
    }
    d.SetMirrored (!d.IsMirrored ());
    for (i = 0 ; i < polygons.Length () ; i++)
    {
      p = GetPolygon3D (i);
      if (p->flags.Check (CS_POLY_NO_DRAW)) continue;
      clip = (csPolygon2D*)(render_pool->Alloc ());
      const csPlane3& wplane = p->GetPlane ()->GetWorldPlane ();
      bool front = csMath3::Visible (d.GetOrigin (), wplane);

      if (front &&
        p->ClipToPlane (d.HasClipPlane () ? &d.GetClipPlane () : (csPlane3*)NULL,
		d.GetOrigin (), verts, num_verts, true) &&
        p->DoPerspective (d, verts, num_verts, clip, orig_triangle,
		d.IsMirrored ()) &&
        clip->ClipAgainst (d.GetView ()))
      {
        p->GetPlane ()->WorldToCamera (d, verts[0]);
	if (d.GetCallback ())
	{
          d.CallCallback (CALLBACK_POLYGON, (void*)p);
          d.CallCallback (CALLBACK_POLYGON2D, (void*)clip);
	}

        Stats::polygons_drawn++;

        if (!d.GetCallback ())
	  clip->AddFogPolygon (d.GetG3D (), p, p->GetPlane (), d.IsMirrored (),
	  	GetID (), CS_FOG_FRONT);
      }
      render_pool->Free (clip);
    }
    if (!d.GetCallback ()) d.GetG3D ()->CloseFogObject (GetID ());
  }

  draw_busy--;
}


void csThing::CheckFrustum (csFrustumView& lview)
{
  csCBufferCube* cb = engine->GetCBufCube ();
  csCovcube* cc = engine->GetCovcube ();
  if (cb) cb->MakeEmpty ();
  else cc->MakeEmpty ();
  RealCheckFrustum (lview);
}

void csThing::RealCheckFrustum (csFrustumView& lview)
{
  csPolygon3D* p;
  int i;

  draw_busy++;
  UpdateTransformation (lview.light_frustum->GetOrigin ());

  if (light_frame_number != current_light_frame_number)
  {
    light_frame_number = current_light_frame_number;
    lview.gouraud_color_reset = true;
  }
  else lview.gouraud_color_reset = false;

  for (i = 0 ; i < polygons.Length () ; i++)
  {
    p = GetPolygon3D (i);
    p->CamUpdate ();
    lview.poly_func ((csObject*)p, &lview);
  }

  for (i = 0 ; i < GetNumCurves () ; i++)
  {
    csCurve* c = curves.Get (i);
    
    if (!lview.dynamic)
    {
      csReversibleTransform o2w = movable.GetTransform ().GetInverse();
      c->SetObject2World (&o2w);
    }
    
    lview.curve_func ((csObject*)c, &lview);
  }

  draw_busy--;
}

void csThing::InitLightMaps (bool do_cache)
{
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
    polygons.Get (i)->InitLightMaps (this, do_cache, i);
  for (i = 0 ; i < GetNumCurves () ; i++)
    if (movable.InSector ())
      curves.Get (i)->InitLightMaps (this, movable.GetSector (0),
        do_cache, polygons.Length ()+i);
}

void csThing::CacheLightMaps ()
{
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
    polygons.Get (i)->CacheLightMaps (this, i);
  for (i = 0 ; i < GetNumCurves () ; i++)
    curves.Get (i)->CacheLightMaps (this, polygons.Length ()+i);
}

void csThing::Merge (csThing* other)
{
  int i, j;
  int* merge_vertices = new int [other->GetNumVertices ()+1];
  for (i = 0 ; i < other->GetNumVertices () ; i++)
    merge_vertices[i] = AddVertex (other->Vwor (i));

  for (i = 0 ; i < other->polygons.Length () ; i++)
  {
    csPolygon3D* p = other->GetPolygon3D (i);
    int* idx = p->GetVertices ().GetVertexIndices ();
    for (j = 0 ; j < p->GetVertices ().GetNumVertices () ; j++)
      idx[j] = merge_vertices[idx[j]];
    AddPolygon (p);
    other->polygons[i] = NULL;
  }

  for (i = 0 ; i < other->GetNumCurveVertices () ; i++)
    AddCurveVertex (other->CurveVertex (i), other->CurveTexel (i));

  for (i = 0 ; i < other->curves.Length () ; i++)
  {
    csCurve* c = other->GetCurve (i);
    AddCurve (c);
    other->curves[i] = NULL;
  }

  delete [] merge_vertices;
}


void csThing::MergeTemplate (csThing* tpl, csSector* sector,
  csMaterialWrapper* default_material, float default_texlen,
  bool objspace, csVector3* shift, csMatrix3* transform)
{
  (void)default_texlen;
  int i, j;
  int* merge_vertices;

  flags.SetAll (tpl->flags.Get ());

  //TODO should merge? take averages or something?
  curves_center = tpl->curves_center;
  curves_scale = tpl->curves_scale;
  ParentTemplate = tpl;

  merge_vertices = new int [tpl->GetNumVertices ()+1];
  for (i = 0 ; i < tpl->GetNumVertices () ; i++)
  {
    csVector3 v;
    if (objspace) v = tpl->Vobj (i);
    else v = tpl->Vwor (i);
    if (transform) v = *transform * v;
    if (shift) v += *shift;
    merge_vertices[i] = AddVertex (v);
  }

  for (i = 0 ; i < tpl->polygons.Length () ; i++)
  {
    csPolygon3D* pt = tpl->GetPolygon3D (i);
    csPolygon3D* p;
    p = NewPolygon (pt->GetMaterialWrapper ());
    p->SetName (pt->GetName());
    if (!pt->GetMaterialWrapper ()) p->SetMaterial (default_material);
    int* idx = pt->GetVertices ().GetVertexIndices ();
    for (j = 0 ; j < pt->GetVertices ().GetNumVertices () ; j++)
      p->AddVertex (merge_vertices[idx[j]]);

    p->flags.SetAll (pt->flags.Get ());
    p->SetTextureType (pt->GetTextureType ());

    csPolyTexFlat* txtflat_src = pt->GetFlatInfo ();
    if (txtflat_src)
    {
      csPolyTexFlat* txtflat_dst = p->GetFlatInfo ();
      txtflat_dst->Setup (p);
      csVector2 *uv_coords = txtflat_src->GetUVCoords ();
      if (uv_coords)
        for (j = 0; j < pt->GetNumVertices (); j++)
          txtflat_dst->SetUV (j, uv_coords[j].x, uv_coords[j].y);
    }

    csPolyTexGouraud* txtgour_src = pt->GetGouraudInfo ();
    if (txtgour_src)
    {
      csPolyTexGouraud* txtgour_dst = p->GetGouraudInfo ();
      txtgour_dst->Setup (p);
      csColor* col = txtgour_src->GetColors ();
      if (col)
        for (j = 0; j < pt->GetNumVertices (); j++)
          txtgour_dst->SetColor (j, col[j]);
    }

    csMatrix3 m;
    csVector3 v (0);
    csPolyTexLightMap* txtlmi_src = pt->GetLightMapInfo ();
    if (txtlmi_src)
    {
      txtlmi_src->GetTxtPlane ()->GetTextureSpace (m, v);
    }
    p->SetTextureSpace (m, v);
  }

  for (i = 0; i < tpl->GetNumCurveVertices (); i++)
    AddCurveVertex (tpl->CurveVertex (i), tpl->CurveTexel (i));

  for (i = 0; i < tpl->GetNumCurves (); i++)
  {
    csCurveTemplate* pt = tpl->GetCurve (i)->GetParentTemplate ();
    csCurve* p = pt->MakeCurve ();
    p->SetName (pt->GetName ());
    p->SetParent (this);
    p->SetSector (sector);

    if (!pt->GetMaterialWrapper ()) p->SetMaterialWrapper (default_material);
    for (j = 0 ; j < pt->NumVertices () ; j++)
      p->SetControlPoint (j, pt->GetVertex (j));
    AddCurve (p);
  }

  delete [] merge_vertices;
}

void csThing::MergeTemplate (csThing* tpl, csSector* sector, csMaterialList* matList,
  const char* prefix, csMaterialWrapper* default_material, float default_texlen,
  bool objspace, csVector3* shift, csMatrix3* transform)
{
  int i;
  const char *txtname;
  char *newname=NULL;
    
  MergeTemplate (tpl, sector, default_material, default_texlen, objspace,
    shift, transform);
    
  // Now replace the materials.
  for (i = 0; i < GetNumPolygons (); i++)
  {
    csPolygon3D *p = GetPolygon3D (i);
    txtname = p->GetMaterialWrapper ()->GetName ();
    newname = new char [strlen (prefix) + strlen (txtname) + 2];
    sprintf (newname, "%s_%s", prefix, txtname);
    csMaterialWrapper *th = matList->FindByName (newname);
    if (th != NULL)
      p->SetMaterial (th);
    delete [] newname;
  }
}

void csThing::UpdateInPolygonTrees ()
{
  // If thing has not been placed in a sector we do nothing here.
  if (!movable.InSector ()) return;

  // If we are not in a sector which has a polygon tree
  // then we don't really update. We should consider if this is
  // a good idea. Do we only want this object updated when we
  // want to use it in a polygon tree? It is certainly more
  // efficient to do it this way when the thing is currently
  // moving in normal convex sectors.
  csPolygonTree* tree = NULL;
  // @@@ What if we are in more than one sector? To be done later
  // when this becomes possible.
  tree = movable.GetSector (0)->GetStaticTree ();
  if (!tree) return;

  csBox3 b;
  GetBoundingBox (b);

  csReversibleTransform trans = movable.GetTransform ().GetInverse ();
  tree_bbox.Update (b, trans, this);

  // Here we need to insert in trees where this thing lives.
  tree = movable.GetSector (0)->GetStaticTree ();
  if (tree)
  {
    // Temporarily increase reference to prevent free.
    tree_bbox.GetBaseStub ()->IncRef ();
    tree->AddObject (&tree_bbox);
    tree_bbox.GetBaseStub ()->DecRef ();
  }
}
