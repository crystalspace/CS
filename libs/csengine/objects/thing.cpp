/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include "sysdef.h"
#include "qint.h"
#include "csobject/nameobj.h"
#include "csengine/thing.h"
#include "csengine/thingtpl.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/light.h"
#include "csengine/world.h"
#include "csengine/stats.h"
#include "csengine/sector.h"
#include "csengine/curve.h"
#include "csengine/texture.h"
#include "csengine/sysitf.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "itexture.h"
#include "qint.h"


//---------------------------------------------------------------------------

CSOBJTYPE_IMPL(csThing,csPolygonSet);

csThing::csThing () : csPolygonSet (), obj()
{
  flags = 0;
  center_idx = -1;
  ParentTemplate = NULL;
}

csThing::~csThing ()
{
}

void csThing::SetConvex (bool c)
{
  SetFlags (CS_ENTITY_CONVEX, c ? CS_ENTITY_CONVEX : 0);
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

void csThing::Transform ()
{
  int i;
  if (!bbox) CreateBoundingBox ();
  for (i = 0 ; i < num_vertices ; i++)
    wor_verts[i] = obj.This2Other (obj_verts[i]);
  for (i = 0 ; i < num_polygon ; i++)
  {
    csPolygon3D* p = (csPolygon3D*)polygons[i];
    p->ObjectToWorld (obj);
  }
}

void csThing::SetMove (csSector* home, float x, float y, float z)
{
  obj.SetOrigin (csVector3(x,y,z));
  sector = home;
}

void csThing::SetTransform (const csMatrix3& matrix)
{
  obj.SetT2O (matrix);
}

void csThing::Move (float dx, float dy, float dz)
{
  obj.Translate(csVector3(dx,dy,dz));
}

void csThing::Transform (csMatrix3& matrix)
{
  obj.SetT2O (matrix * obj.GetT2O ());
}

void csThing::CreateLightMaps (IGraphics3D* g3d)
{
  int i;
  for (i = 0 ; i < num_polygon ; i++)
  {
    csPolygon3D* p = (csPolygon3D*)polygons[i];
    p->CreateLightMaps (g3d);
  }
}

csPolygon3D* csThing::IntersectSphere (csVector3& center, float radius, float* pr)
{
  float d, min_d = radius;
  int i;
  csPolygon3D* p, * min_p = NULL;
  csPolyPlane* pl;
  csVector3 hit;
  float A, B, C, D;

  for (i = 0 ; i < num_polygon ; i++)
  {
    p = (csPolygon3D*)polygons[i];
    pl = p->GetPlane ();
    d = pl->Distance (center);
    if (d < min_d && pl->VisibleFromPoint (center))
    {
      pl->GetWorldNormal (&A, &B, &C, &D);
      hit.x = d*(-A-center.x)+center.x;
      hit.y = d*(-B-center.y)+center.y;
      hit.z = d*(-C-center.z)+center.z;
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

void csThing::DrawCurves (csRenderView& rview, bool use_z_buf)
{
  int i;
  int res=1;

  // Calculate tesselation resolution
  csVector3 wv = curves_center;
  csVector3 world_coord = obj.This2Other (wv);
  csVector3 camera_coord = rview.Other2This (world_coord);

  if (camera_coord.z >= SMALL_Z)
  {
    res=(int)(curves_scale/camera_coord.z);
  }
  else
    res=1000; // some big tesselation value...

  // Variables for drawing curves.
  csVector2* texes[3];
  int control_x[3];
  int control_y[3];
  csVector2* persp;
  float* z_array;
  csVector2 triangle[3];
  csVector2 clipped_triangle[10];	//@@@BAD HARDCODED!
  csVector3 coord;
  UByte* mapR=NULL, * mapG=NULL, * mapB=NULL;
  int lm_width=0, lm_height=0;

  // Get this field from the current view for conveniance.
  bool mirror = rview.IsMirrored ();

  // Create the combined transform of object to camera by
  // combining object to world and world to camera.
  csReversibleTransform obj_cam = rview;
  obj_cam /= obj;

  // Loop over all curves
  csCurve* c;
  for (i = 0 ; i < num_curves ; i++)
  {
    int j;
    c = curves[i];
    csCurveTesselated* tess = c->Tesselate (res);

    // First I transform all tesselated vertices from object to
    // world space and then from world to camera space.
    CHK (persp = new csVector2 [tess->GetNumVertices ()]);
    CHK (z_array = new float [tess->GetNumVertices ()]);
    for (j = 0 ;  j < tess->GetNumVertices () ; j++)
    {
      csCurveVertex& cv = tess->GetVertex (j);
      coord = obj_cam.Other2This (cv.object_coord);
      if (coord.z >= SMALL_Z)
      {
    	float iz = rview.aspect/coord.z;
        persp[j].x = coord.x * iz + rview.shift_x;
        persp[j].y = coord.y * iz + rview.shift_y;
      }
      z_array[j] = coord.z;
    }

    // Clipped polygon (assume it cannot have more than 64 vertices)

    // Draw all the triangles within this curve
    G3DPolygonDPFX poly;
    memset (&poly, 0, sizeof(poly));
    poly.txt_handle = c->GetTextureHandle ();
    poly.txt_handle->GetMeanColor (poly.flat_color_r, poly.flat_color_g, poly.flat_color_b);
    poly.inv_aspect = rview.inv_aspect;
    if (poly.txt_handle == NULL)
    {
      CsPrintf (MSG_STDOUT, "Warning! Curve without texture!\n");
      continue;
    }
    bool gouraud = false;
    if (c->lightmap)
    {
      gouraud = true;
      csRGBLightMap& map = c->lightmap->GetStaticMap ();
      mapR = map.mapR;
      mapG = map.mapG;
      mapB = map.mapB;
      lm_width = c->lightmap->GetWidth ()-2;
      lm_height = c->lightmap->GetWidth ()-2;
    }
    rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, use_z_buf);
    rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);
    if (!rview.callback)
      rview.g3d->StartPolygonFX (poly.txt_handle, CS_FX_COPY | (gouraud ? CS_FX_GOURAUD : 0));

    for (j = 0 ; j < tess->GetNumTriangles () ; j++)
    {
      csCurveTriangle& ct = tess->GetTriangle (j);

      bool visible = z_array [ct.i1] >= SMALL_Z &&
      		     z_array [ct.i2] >= SMALL_Z &&
      		     z_array [ct.i3] >= SMALL_Z;

      if (visible)
      {
  	//-----
  	// Do backface culling. Note that this depends on mirroring
	// of the current view.
  	//-----
  	float area = csMath2::Area2 (persp [ct.i1].x, persp [ct.i1].y,
          		 	     persp [ct.i2].x, persp [ct.i2].y,
          		 	     persp [ct.i3].x, persp [ct.i3].y);

	if (mirror)
	{
          if (area <= 0) continue;
	}
	else
          if (area >= 0) continue;

        triangle[mirror ? 2 : 0] = persp[ct.i1];
        triangle[1] = persp[ct.i2];
        triangle[mirror ? 0 : 2] = persp[ct.i3];

  	// Clip triangle
  	int rescount;
  	if (!rview.view->Clip (triangle, clipped_triangle, 3, rescount)) continue;

        texes[0] = &tess->GetVertex (ct.i1).txt_coord;
        texes[1] = &tess->GetVertex (ct.i2).txt_coord;
        texes[2] = &tess->GetVertex (ct.i3).txt_coord;

        if (gouraud)
        {
          control_x[0] = QInt (tess->GetVertex (ct.i1).control.x * lm_width);
          control_y[0] = QInt (tess->GetVertex (ct.i1).control.y * lm_height);
          control_x[1] = QInt (tess->GetVertex (ct.i2).control.x * lm_width);
          control_y[1] = QInt (tess->GetVertex (ct.i2).control.y * lm_height);
          control_x[2] = QInt (tess->GetVertex (ct.i3).control.x * lm_width);
          control_y[2] = QInt (tess->GetVertex (ct.i3).control.y * lm_height);
        }

  	poly.num = rescount;
        int trivert [3] = { ct.i1, ct.i2, ct.i3 };
        int j, idx, dir;
        // If mirroring we store the vertices in the other direction.
        if (mirror) { idx = 2; dir = -1; }
        else { idx = 0; dir = 1; }
  	for (j = 0; j < 3; j++)
  	{
      	  poly.vertices [idx].z = 1 / z_array[trivert [j]];
    	  poly.vertices [idx].u = texes[j]->x;
    	  poly.vertices [idx].v = texes[j]->y;
    	  if (gouraud)
    	  {
      	    int lm_idx = control_y[j]*(lm_width+2) + control_x[j];
      	    poly.vertices[idx].r = ((float)mapR[lm_idx])/256.;
      	    poly.vertices[idx].g = ((float)mapG[lm_idx])/256.;
      	    poly.vertices[idx].b = ((float)mapB[lm_idx])/256.;
    	  }
	  idx += dir;
  	}

        PreparePolygonFX (&poly, clipped_triangle, rescount, (csVector2 *)triangle, gouraud);

  	// Draw resulting polygon
	if (!rview.callback)
	{
	  extern void CalculateFogPolygon (csRenderView* rview, G3DPolygonDPFX& poly);
	  CalculateFogPolygon (&rview, poly);
  	  rview.g3d->DrawPolygonFX (poly);
	}
	else
          rview.callback (&rview, CALLBACK_POLYGONQ, (void*)&poly);
      }
    }
    if (!rview.callback)
      rview.g3d->FinishPolygonFX ();

    CHK (delete [] persp);
    CHK (delete [] z_array);
  }
}

void csThing::Draw (csRenderView& rview, bool use_z_buf)
{
  draw_busy++;
  UpdateTransformation (rview);

  // @@@ Wouldn't it be nice if we checked if all vertices are behind the view plane?
  {
    if (rview.callback) rview.callback (&rview, CALLBACK_THING, (void*)this);
    Stats::polygons_considered += num_polygon;

    DrawCurves (rview, use_z_buf);

    int res=1;

    // Calculate tesselation resolution
    // TODO should depend on a point in space, and a scale param in place of 40
    if (num_vertices>0)
    {
      csVector3 wv = wor_verts[0];
      csVector3 world_coord = obj.This2Other (wv);
      csVector3 camera_coord = rview.Other2This (world_coord);
  
      if (camera_coord.z > 0.0001)
      {
        res=(int)(40.0/camera_coord.z);
        if (res<1) res=1;
        else if (res>9) res=9;
      }
    }

    DrawPolygonArray (polygons, num_polygon, &rview, use_z_buf);
    if (rview.callback) rview.callback (&rview, CALLBACK_THINGEXIT, (void*)this);
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
  csPoly2DPool* render_pool = csWorld::current_world->render_pol2d_pool;
  csPolygon2D* clip;

  // @@@ Wouldn't it be nice if we checked all vertices against the Z plane?
  {
    csVector2 orig_triangle[3];
    if (!d.callback) d.g3d->OpenFogObject (GetID (), &GetFog ());
    Stats::polygons_considered += num_polygon;

    d.SetMirrored (!d.IsMirrored ());
    for (i = 0 ; i < num_polygon ; i++)
    {
      p = (csPolygon3D*)polygons[i];
      if (p->dont_draw) continue;
      clip = (csPolygon2D*)(render_pool->Alloc ());
      bool front = p->GetPlane ()->VisibleFromPoint (d.GetOrigin ());

      if (!front &&
        p->ClipToPlane (d.do_clip_plane ? &d.clip_plane : (csPlane*)NULL, d.GetOrigin (),
              verts, num_verts, false) &&
        p->DoPerspective (d, verts, num_verts, clip, orig_triangle, d.IsMirrored ()) &&
        clip->ClipAgainst (d.view))
      {
        p->GetPlane ()->WorldToCamera (d, verts[0]);
	if (d.callback)
	{
          d.callback (&d, CALLBACK_POLYGON, (void*)p);
          d.callback (&d, CALLBACK_POLYGON2D, (void*)clip);
	}

        Stats::polygons_drawn++;

	if (!d.callback)
          clip->AddFogPolygon (d.g3d, p, p->GetPlane (), d.IsMirrored (), GetID (),
	  	CS_FOG_BACK);
      }
      render_pool->Free (clip);
    }
    d.SetMirrored (!d.IsMirrored ());
    for (i = 0 ; i < num_polygon ; i++)
    {
      p = (csPolygon3D*)polygons[i];
      if (p->dont_draw) continue;
      clip = (csPolygon2D*)(render_pool->Alloc ());
      bool front = p->GetPlane ()->VisibleFromPoint (d.GetOrigin ());

      if (front &&
        p->ClipToPlane (d.do_clip_plane ? &d.clip_plane : (csPlane*)NULL, d.GetOrigin (),
            verts, num_verts, true) &&
        p->DoPerspective (d, verts, num_verts, clip, orig_triangle, d.IsMirrored ()) &&
        clip->ClipAgainst (d.view))
      {
        p->GetPlane ()->WorldToCamera (d, verts[0]);
	if (d.callback)
	{
          d.callback (&d, CALLBACK_POLYGON, (void*)p);
          d.callback (&d, CALLBACK_POLYGON2D, (void*)clip);
	}

        Stats::polygons_drawn++;

        if (!d.callback)
	  clip->AddFogPolygon (d.g3d, p, p->GetPlane (), d.IsMirrored (), GetID (),
	  	CS_FOG_FRONT);
      }
      render_pool->Free (clip);
    }
    if (!d.callback) d.g3d->CloseFogObject (GetID ());
  }

  draw_busy--;
}


void csThing::CalculateLighting (csLightView& lview)
{
  csPolygon3D* p;
  int i;

  draw_busy++;
  UpdateTransformation (lview.light_frustrum->GetOrigin ());

  if (light_frame_number != current_light_frame_number)
  {
    light_frame_number = current_light_frame_number;
    lview.gouraud_color_reset = true;
  }
  else lview.gouraud_color_reset = false;

  for (i = 0 ; i < num_polygon ; i++)
  {
    p = (csPolygon3D*)polygons[i];
    p->CamUpdate ();
    p->CalculateLighting (&lview);
  }

  // Loop over all curves
  csCurve* c;
  for (i = 0 ; i < num_curves ; i++)
  {
    c = curves[i];
    c->CalculateLighting (lview);
  }

  draw_busy--;
}

void csThing::InitLightMaps (bool do_cache)
{
  int i;
  for (i = 0 ; i < num_polygon ; i++)
    ((csPolygon3D*)polygons[i])->InitLightMaps (this, do_cache, i);
  for (i = 0 ; i < num_curves ; i++)
    ((csCurve*)curves[i])->InitLightMaps (this, do_cache, num_polygon+i);
}

void csThing::CacheLightMaps ()
{
  int i;
  for (i = 0 ; i < num_polygon ; i++)
    ((csPolygon3D*)polygons[i])->CacheLightMaps (this, i);
  for (i = 0 ; i < num_curves ; i++)
    ((csCurve*)curves[i])->CacheLightMaps (this, num_polygon+i);
}

void csThing::Merge (csThing* other)
{
  int i, j;
  CHK (int *merge_vertices = new int [other->GetNumVertices ()+1]);
  for (i = 0 ; i < other->GetNumVertices () ; i++)
    merge_vertices[i] = AddVertex (other->Vwor (i));

  for (i = 0 ; i < other->GetNumPolygons () ; i++)
  {
    csPolygon3D* p = (csPolygon3D*)(other->GetPolygon (i));
    int* idx = p->GetVertices ().GetVertexIndices ();
    for (j = 0 ; j < p->GetVertices ().GetNumVertices () ; j++)
      idx[j] = merge_vertices[idx[j]];
    p->SetParent (this);
    AddPolygon (p);
    other->polygons[i] = NULL;
  }

  CHK (delete [] merge_vertices);
}


void csThing::MergeTemplate (csThingTemplate* tpl,
  csTextureHandle* default_texture, float default_texlen,
  CLights* default_lightx,
  csVector3* shift, csMatrix3* transform)
{
  (void)default_texlen;
  int i, j;
  int* merge_vertices;

  //TODO should merge? take averages or something?
  curves_center = tpl->curves_center;
  curves_scale = tpl->curves_scale;
  ParentTemplate = tpl;

  CHK (merge_vertices = new int [tpl->GetNumVertices ()+1]);
  for (i = 0 ; i < tpl->GetNumVertices () ; i++)
  {
    csVector3 v = tpl->Vtex (i);
    if (transform) v = *transform * v;
    if (shift) v += *shift;
    merge_vertices[i] = AddVertex (v);
  }

  for (i = 0 ; i < tpl->GetNumPolygon () ; i++)
  {
    csPolygonTemplate* pt = tpl->GetPolygon (i);
    csPolygon3D* p;
    p = NewPolygon (pt->GetTexture ());
    csNameObject::AddName(*p, pt->GetName());
    if (!pt->GetTexture ()) p->SetTexture (default_texture);
    csLightMapped* pol_lm = p->GetLightMapInfo ();
    if (pol_lm) pol_lm->SetUniformDynLight (default_lightx);
    int* idx = pt->GetVerticesIdx ();
    for (j = 0 ; j < pt->GetNumVertices () ; j++)
      p->AddVertex (merge_vertices[idx[j]]);
    p->SetFlags (CS_POLY_LIGHTING|CS_POLY_MIPMAP|CS_POLY_FLATSHADING,
      (pt->IsLighted () ? CS_POLY_LIGHTING : 0) |
      (pt->IsMipmapped () ? CS_POLY_MIPMAP : 0) |
      (pt->UseFlatColor () ? CS_POLY_FLATSHADING : 0));
    if (pt->GetUVCoords ())
    {
      p->SetTextureType (POLYTXT_GOURAUD);
      csGouraudShaded* gs = p->GetGouraudInfo ();
      gs->Setup (p->GetVertices ().GetNumVertices ());
      for (j = 0 ; j < pt->GetNumVertices () ; j++)
        gs->SetUV (j, pt->GetUVCoords ()[j].x, pt->GetUVCoords ()[j].y);
      if (pt->UseGouraud ())
        gs->EnableGouraud (true);
    }
    p->SetTextureSpace (pt->GetTextureMatrix (), pt->GetTextureVector ());
  }

  for (i=0;i< tpl->GetNumCurveVertices();i++)
  {
    AddCurveVertex (tpl->CurveVertex (i),tpl->CurveTexel (i));
  }

  for (i = 0 ; i < tpl->GetNumCurves () ; i++)
  {
    csCurveTemplate* pt = tpl->GetCurve (i);
    csCurve* p = pt->MakeCurve ();
    csNameObject::AddName(*p, csNameObject::GetName(*pt));
    p->SetParent (this);

    if (!pt->GetTextureHandle ()) p->SetTextureHandle (default_texture);
    for (j = 0 ; j < pt->NumVertices () ; j++)
    {
      p->SetControlPoint (j, pt->GetVertex (j));
    }
    AddCurve (p);
  }

  CHK (delete [] merge_vertices);
}

//---------------------------------------------------------------------------
