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
#include "csobject/nameobj.h"
#include "csengine/objects/thing.h"
#include "csengine/objects/thingtpl.h"
#include "csengine/polygon/polygon.h"
#include "csengine/polygon/pol2d.h"
#include "csengine/polygon/polytext.h"
#include "csengine/light/light.h"
#include "csengine/world.h"
#include "csengine/stats.h"
#include "csengine/sector.h"
#include "csengine/wirefrm.h"
#include "csengine/curve.h"
#include "csengine/texture.h"
#include "csengine/sysitf.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "itexture.h"


//---------------------------------------------------------------------------

CSOBJTYPE_IMPL(csThing,csPolygonSet);

csThing::csThing () : csPolygonSet (), obj() 
{
  moveable = false;
  merged = NULL;
  convex = false;
  center_idx = -1;
}

csThing::~csThing ()
{
  if (merged)
  {
    int i;
    for (i = 0 ; i < num_polygon ; i++)
      polygons[i] = NULL;
  }
}

void csThing::SetConvex (bool c)
{
  convex = c;
  if (convex)
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

void csThing::SetTransform (csMatrix3& matrix)
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

void csThing::CreateLightmaps (IGraphics3D* g3d)
{
  int i;
  for (i = 0 ; i < num_polygon ; i++)
  {
    csPolygon3D* p = (csPolygon3D*)polygons[i];
    p->CreateLightmaps (g3d);
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


#	define INTERPOLATE(val,tl,bl,tr,br)	\
        {					\
          float vl,vr;				\
          if (tl != bl)				\
            vl = tl + (bl - tl) * tL;		\
          else					\
            vl = tl;				\
          if (tr != br)				\
            vr = tr + (br - tr) * tR;		\
          else					\
            vr = tr;				\
          val = vl + (vr - vl) * tX;		\
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
//Jorrit: moved the code that was here to the relevant bezier, as clamping the
// allowed tesselation resolutions is part of the curve itself I think...      
    }
    else
      res=1000; // some big tesselation value...

  //CsPrintf(MSG_WARNING,"res=%d\n",res);
  
  // Loop over all curves
  csCurve* c;
  for (i = 0 ; i < num_curves ; i++)
    {
      int j;
      c = curves[i];
      csCurveTesselated* tess = c->Tesselate (res);
      
      // First I transform all tesselated vertices from object to
      // world space and then from world to camera space. @@@ NOTE!
      // These transformations should be combined for better efficiency!
      for (j = 0 ;  j < tess->GetNumVertices () ; j++)
	{
	  csCurveVertex& cv = tess->GetVertex (j);
	  cv.world_coord = obj.This2Other (cv.object_coord);
	  cv.camera_coord = rview.Other2This (cv.world_coord);
	}
      
      // Clipped polygon (assume it cannot have more than 64 vertices)
      
      // Draw all the triangles within this curve
      G3DPolygon poly;
      memset (&poly, 0, sizeof(G3DPolygon));
      poly.txt_handle = c->GetTextureHandle ();
      if (poly.txt_handle == NULL)
      {
        CsPrintf (MSG_STDOUT, "Warning! Curve without texture!\n");
        continue;
      }
      csVector2 persp[3];
      CHK (poly.pi_texcoords = new G3DPolygon::poly_texture_def [64]);
      rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, use_z_buf);
      rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);
      for (j = 0 ; j < tess->GetNumTriangles () ; j++)
	{
	  csCurveTriangle& ct = tess->GetTriangle (j);
	  
	  // Transform all vertices from camera space to perspective 
	  // correct coords
	  
	  csVector3* varr[3];
	  varr[0] = &tess->GetVertex (ct.i1).camera_coord;
	  varr[1] = &tess->GetVertex (ct.i2).camera_coord;
	  varr[2] = &tess->GetVertex (ct.i3).camera_coord;
	  
	  csVector2* texes[3];
	  texes[0] = &tess->GetVertex (ct.i1).txt_coord;
	  texes[1] = &tess->GetVertex (ct.i2).txt_coord;
	  texes[2] = &tess->GetVertex (ct.i3).txt_coord;
	  
	  int visible = true;
	  int k;
	  for (k = 0 ; k < 3 ; k++)
	    {
	      if (varr[k]->z >= SMALL_Z)
		{
		  float iz = csCamera::aspect/varr[k]->z;
		  persp[k].x = varr[k]->x * iz + csWorld::shift_x;
		  persp[k].y = varr[k]->y * iz + csWorld::shift_y;
		}
	      else
		visible = false;
	    }
	  
	  // Draw all triangles.
	  if (visible)
	    {
	      //-----
	      // Do backface culling.
	      //-----
	      if (csMath2::Area2 (persp [0].x, persp [0].y,
				  persp [1].x, persp [1].y,
				  persp [2].x, persp [2].y) >= 0)
		continue;

	      
	      // Clip triangle
	      int rescount;
	      csVector2 *cpoly = rview.view->Clip (persp, 3, rescount);
	      if (!cpoly)
		{
		  continue;
		}
	      
	      // Compute U & V in vertices of the polygon
	      // Find first the topmost triangle vertex
	      int top;
	      if (persp [0].y < persp [1].y)
		if (persp [0].y < persp [2].y)
		  top = 0;
		else
		  top = 2;
	      else
		if (persp [1].y < persp [2].y)
		  top = 1;
		else
		  top = 2;
	      
	      poly.num = rescount;
	      
	      poly.pi_triangle = (csVector2 *)persp;
	      int j;
	      for (j = 0; j < 3; j++)
		{
		  if (varr[j]->z > 0.0001)
		    poly.pi_tritexcoords [j].z = 1 / varr[j]->z;
		  poly.pi_tritexcoords [j].u = texes[j]->x;
		  poly.pi_tritexcoords [j].v = texes[j]->y;
		} // endfor 
	      
	      for (j = 0; j < rescount; j++)
		{
		  float x = poly.vertices [j].sx = cpoly [j].x;
		  float y = poly.vertices [j].sy = cpoly [j].y;
		  
		  // Find the triangle left/right edges between which
		  // is located given x,y point
		  int vtl, vtr, vbl, vbr;
		  vtl = vtr = top;
		  vbl = (vtl + 1) % 3;
		  vbr = (vtl + 3 - 1) % 3;
		  if (y > persp [vbl].y)
		    {
		      vtl = vbl;
		      vbl = (vbl + 1) % 3;
		    } 
		  else if (y > persp [vbr].y)
		    {
		      vtr = vbr;
		      vbr = (vbr + 3 - 1) % 3;
		    } // endif 
		  
		  // Now interpolate Z,U,V by Y
		  float tL = persp [vbl].y - persp [vtl].y;
		  if (tL< -0.001 || tL>0.001)
		    tL = (y - persp [vtl].y) / tL;
		  float tR = persp [vbr].y - persp [vtr].y;
		  if (tR< -0.001 || tR>0.001)
		    tR = (y - persp [vtr].y) / tR;
		  float xL = persp [vtl].x + tL * (persp [vbl].x - persp [vtl].x);
		  float xR = persp [vtr].x + tR * (persp [vbr].x - persp [vtr].x);
		  float tX = xR - xL;
		  if (tX< -0.001 || tX>0.001)
		    tX = (x - xL) / tX;
		  
		  // Calculate Z
		  INTERPOLATE (poly.pi_texcoords [j].z,
			       poly.pi_tritexcoords [vtl].z, poly.pi_tritexcoords [vbl].z,
			       poly.pi_tritexcoords [vtr].z, poly.pi_tritexcoords [vbr].z);
		  // Calculate U
		  INTERPOLATE (poly.pi_texcoords [j].u,
			       poly.pi_tritexcoords [vtl].u, poly.pi_tritexcoords [vbl].u,
			       poly.pi_tritexcoords [vtr].u, poly.pi_tritexcoords [vbr].u);
		  // Calculate V
		  INTERPOLATE (poly.pi_texcoords [j].v,
			       poly.pi_tritexcoords [vtl].v, poly.pi_tritexcoords [vbl].v,
			       poly.pi_tritexcoords [vtr].v, poly.pi_tritexcoords [vbr].v);
		} // endfor 
	      CHK (delete[] cpoly);
	      // Draw resulting polygon

	      rview.g3d->DrawPolygonQuick (poly, false);
	    }
	}
      CHK (delete [] poly.pi_texcoords);
    }
}

void csThing::Draw (csRenderView& rview, bool use_z_buf)
{
  draw_busy++;
  csVector3* old;
  NewTransformation (old);

  csWireFrame* wf = NULL;
  if (csWorld::current_world->map_mode != MAP_OFF) wf = csWorld::current_world->wf->GetWireframe ();

  if (TransformWorld2Cam (rview))
  {
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
  }

  RestoreTransformation (old);
  draw_busy--;
}

void csThing::DrawFoggy (csRenderView& d)
{
  draw_busy++;
  csVector3* old;
  NewTransformation (old);
  csPolygon3D* p;
  csVector3* verts;
  int num_verts;
  int i;

  csWireFrame* wf = NULL;
  if (csWorld::current_world->map_mode != MAP_OFF) wf = csWorld::current_world->wf->GetWireframe ();

  if (TransformWorld2Cam (d))
  {
    csVector2 orig_triangle[3];
    d.g3d->OpenFogObject (GetID (), &GetFog ());
    Stats::polygons_considered += num_polygon;

    d.SetMirrored (!d.IsMirrored ());
    for (i = 0 ; i < num_polygon ; i++)
    {
      p = (csPolygon3D*)polygons[i];
      if (p->dont_draw) continue;
      bool front = p->GetPlane ()->VisibleFromPoint (d.GetOrigin ());

      if ( !front &&
           p->ClipToPlane (d.do_clip_plane ? &d.clip_plane : (csPlane*)NULL, d.GetOrigin (),
	   	verts, num_verts, false) &&
           p->DoPerspective (d, verts, num_verts, &csPolygon2D::clipped, orig_triangle, d.IsMirrored ()) &&
	   csPolygon2D::clipped.ClipAgainst (d.view) )
      {
        if (wf)
        {
          int j;
          csWfPolygon* po = wf->AddPolygon ();
          po->SetVisColor (wf->GetYellow ());
          po->SetNumVertices (p->GetNumVertices ());
          for (j = 0 ; j < p->GetNumVertices () ; j++)
            po->SetVertex (j, p->Vwor (j));
          po->Prepare ();
        }
        Stats::polygons_drawn++;

	csPolygon2D::clipped.AddFogPolygon (d.g3d, p, p->GetPlane (), d.IsMirrored (),
		GetID (), CS_FOG_BACK);

        long do_edges;
        d.g3d->GetRenderState (G3DRENDERSTATE_EDGESENABLE, do_edges);
	if (do_edges)
	{
  	  ITextureManager* txtmgr;
  	  d.g3d->GetTextureManager (&txtmgr);
  	  int white;
  	  txtmgr->FindRGB (255, 255, 255, white);
          csPolygon2D::clipped.Draw (d.g2d, white);
	}
      }
    }
    d.SetMirrored (!d.IsMirrored ());
    for (i = 0 ; i < num_polygon ; i++)
    {
      p = (csPolygon3D*)polygons[i];
      if (p->dont_draw) continue;
      bool front = p->GetPlane ()->VisibleFromPoint (d.GetOrigin ());

      if ( front &&
           p->ClipToPlane (d.do_clip_plane ? &d.clip_plane : (csPlane*)NULL, d.GetOrigin (),
	   	verts, num_verts, true) &&
           p->DoPerspective (d, verts, num_verts, &csPolygon2D::clipped, orig_triangle, d.IsMirrored ()) &&
	   csPolygon2D::clipped.ClipAgainst (d.view) )
      {
        if (wf)
        {
          int j;
          csWfPolygon* po = wf->AddPolygon ();
          po->SetVisColor (wf->GetYellow ());
          po->SetNumVertices (p->GetNumVertices ());
          for (j = 0 ; j < p->GetNumVertices () ; j++)
            po->SetVertex (j, p->Vwor (j));
          po->Prepare ();
        }
        Stats::polygons_drawn++;

	csPolygon2D::clipped.AddFogPolygon (d.g3d, p, p->GetPlane (), d.IsMirrored (),
		GetID (), CS_FOG_FRONT);

        long do_edges;
        d.g3d->GetRenderState (G3DRENDERSTATE_EDGESENABLE, do_edges);
	if (do_edges)
	{
  	  ITextureManager* txtmgr;
  	  d.g3d->GetTextureManager (&txtmgr);
  	  int white;
  	  txtmgr->FindRGB (255, 255, 255, white);
          csPolygon2D::clipped.Draw (d.g2d, white);
	}
      }
    }
    d.g3d->CloseFogObject (GetID ());
  }

  RestoreTransformation (old);
  draw_busy--;
}


void csThing::ShineLightmaps (csLightView& lview)
{
  csPolygon3D* p;
  csPortal* po;
  int i;

  draw_busy++;
  csVector3* old;
  NewTransformation (old);

  TranslateVector (lview.center);

  if (light_frame_number != current_light_frame_number)
  {
    light_frame_number = current_light_frame_number;
    lview.gouroud_color_reset = true;
  }
  else lview.gouroud_color_reset = false;

  for (i = 0 ; i < num_polygon ; i++)
  {
    p = (csPolygon3D*)polygons[i];
    csVector3* new_frustrum = NULL;
    int new_num_frustrum = 0;
    if (p->ClipFrustrum (lview.center, lview.frustrum, lview.num_frustrum, lview.mirror,
    	&new_frustrum, &new_num_frustrum))
    {
      // First we check if the light can reach the polygon. If not then there is no
      // need to include it in the view frustrum. This is especially useful for portals
      // since then whole sectors can be culled away efficiently.
      float dist_to_plane = p->GetPolyPlane ()->Distance (lview.center);

      bool faraway = true;
      if (dist_to_plane < lview.l->GetRadius ())
      {
        // The light is close enough to the plane of the polygon. Now we calculate
	// an accurate minimum squared distance from the light to the polygon. Note
	// that we use the new_frustrum which is relative to the center of the light.
	// So this algorithm works with the light position at (0,0,0) (@@@ we should
	// use this to optimize this algorithm further).
	csVector3 o (0, 0, 0);
	float min_sqdist = csSquaredDist::PointPoly (o, new_frustrum, new_num_frustrum,
		*(p->GetPolyPlane ()), dist_to_plane*dist_to_plane);
	if (min_sqdist < lview.l->GetSquaredRadius ()) faraway = false;	// Light reaches polygon.
      }

      if (!faraway)
      {
        csLightView new_lview = lview;
        new_lview.frustrum = new_frustrum;
        new_lview.num_frustrum = new_num_frustrum;
        p->ShineLightmaps (new_lview);
        po = p->GetPortal ();
        if (po) po->ShineLightmaps (new_lview);
        else if (!new_lview.dynamic && csSector::do_radiosity)
        {
          // If there is no portal we simulate radiosity by creating
	  // a dummy portal for this polygon which reflects light.
	  csPortalCS mirror;
	  mirror.SetSector (p->GetSector ());
	  mirror.SetAlpha (10);
	  float r, g, b;
	  p->GetTextureHandle ()->GetMeanColor (r, g, b);
	  mirror.SetFilter (r/4, g/4, b/4);
	  mirror.SetWarp (csTransform::GetReflect ( *(p->GetPolyPlane ()) ));
	  mirror.ShineLightmaps (new_lview);
        }
      }
      else
      {
        CHK (delete [] new_frustrum);
      }
    }
  }

  RestoreTransformation (old);
  draw_busy--;
}

void csThing::DumpFrustrum (csStatLight* l, csVector3* frustrum, int num_frustrum,
	csTransform& t)
{
//@@@
#if 0
  draw_busy++;
  csVector3* old = NewTransformation ();

  TranslateVector (l->get_center ());

  int i;
  csPolygon3D* p;
  for (i = 0 ; i < num_polygon ; i++)
  {
    p = (csPolygon3D*)polygons[i];
    csVector3* new_frustrum = NULL;
    int new_num_frustrum = 0;
    if (p->ClipFrustrum (l->get_center (), frustrum, num_frustrum, false/*@@@unsupported*/,
    	&new_frustrum, &new_num_frustrum))
    {
      ITextureManager* txtmgr;
      g3d->GetTextureManager (&txtmgr);
      int red, white;
      txtmgr->FindRGB (255, 255, 255, white);
      txtmgr->FindRGB (255, 0, 0, red);
      int j;
      csVector3 light_cam;
      csVector3 v0, v1, v2;
      light_cam = t.Other2This (l->get_center ());

      for (j = 0 ; j < new_num_frustrum ; j++)
      {
        v0 = new_frustrum[j] + l->get_center ();
        v1 = t.Other2This (v0);
        v0 = new_frustrum[(j+1)%new_num_frustrum] + l->get_center ();
        v2 = t.Other2This (v0);
        g3d->DrawLine (light_cam, v1, csCamera::aspect, red);
        g3d->DrawLine (light_cam, v2, csCamera::aspect, red);
        g3d->DrawLine (v1, v2, csCamera::aspect, white);
      }
    }
    if (new_frustrum) CHKB (delete [] new_frustrum);
  }

  RestoreTransformation (old);
  draw_busy--;
#endif
}

void csThing::InitLightmaps (bool do_cache)
{
  int i;
  for (i = 0 ; i < num_polygon ; i++)
    ((csPolygon3D*)polygons[i])->InitLightmaps (this, do_cache, i);
}

void csThing::CacheLightmaps ()
{
  int i;
  for (i = 0 ; i < num_polygon ; i++)
    ((csPolygon3D*)polygons[i])->CacheLightmaps (this, i);
}

void csThing::Merge (csThing* other)
{
  int i, j;
  other->merged = this;
  CHK (int *merge_vertices = new int [other->GetNumVertices ()+1]);
  for (i = 0 ; i < other->GetNumVertices () ; i++)
    merge_vertices[i] = AddVertexSmart (other->Vwor (i));

  for (i = 0 ; i < other->GetNumPolygons () ; i++)
  {
    csPolygon3D* p = (csPolygon3D*)(other->GetPolygon (i));
    int* idx = p->GetVerticesIdx ();
    for (j = 0 ; j < p->GetNumVertices () ; j++)
      idx[j] = merge_vertices[idx[j]];
    p->SetParent (this);
    AddPolygon (p);
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

  CHK (merge_vertices = new int [tpl->GetNumVertices ()+1]);
  for (i = 0 ; i < tpl->GetNumVertices () ; i++)
  {
    csVector3 v = tpl->Vtex (i);
    if (transform) v = *transform * v;
    if (shift) v += *shift;
    merge_vertices[i] = AddVertexSmart (v);
  }
  
  for (i = 0 ; i < tpl->GetNumPolygon () ; i++)
  {
    csPolygonTemplate* pt = tpl->GetPolygon (i);
    csPolygon3D* p;
    p = NewPolygon (pt->GetTexture ());
    csNameObject::AddName(*p, pt->GetName());
    if (!pt->GetTexture ()) p->SetTexture (default_texture);
    p->theDynLight = default_lightx;
    int* idx = pt->GetVerticesIdx ();
    for (j = 0 ; j < pt->GetNumVertices () ; j++)
      p->AddVertex (merge_vertices[idx[j]]);
    p->SetLighting (pt->IsLighted ());
    p->SetMipmapping (pt->IsMipmapped ());
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
