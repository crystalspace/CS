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
#include "csengine/basic/polyset.h"
#include "csengine/polygon/polygon.h"
#include "csengine/polygon/pol2d.h"
#include "csengine/polygon/polytext.h"
#include "csengine/light/dynlight.h"
#include "csengine/world.h"
#include "csengine/camera.h"
#include "csengine/sector.h"
#include "csengine/curve.h"
#include "csengine/rview.h"
#include "csengine/wirefrm.h"
#include "csengine/stats.h"
#include "csengine/dumper.h"
#include "csobject/nameobj.h"
#include "csgeom/bsp.h"
#include "igraph3d.h"
#include "itxtmgr.h"


//---------------------------------------------------------------------------

IMPLEMENT_UNKNOWN_NODELETE( csPolygonSet )

#ifndef BUGGY_EGCS_COMPILER

BEGIN_INTERFACE_TABLE( csPolygonSet )
	IMPLEMENTS_COMPOSITE_INTERFACE( PolygonSet )
END_INTERFACE_TABLE()

#else

const INTERFACE_ENTRY *csPolygonSet::GetInterfaceTable ()
{
  typedef csPolygonSet _InterfaceTableClassName;
  static INTERFACE_ENTRY InterfaceTable[2];
  InterfaceTable[0].pIID = &IID_IPolygonSet;
  InterfaceTable[0].pfnFinder = ENTRY_IS_OFFSET;
  InterfaceTable[0].dwData = COMPOSITE_OFFSET(csPolygonSet, IPolygonSet, IPolygonSet, m_xPolygonSet);
  InterfaceTable[1].pIID = 0;
  InterfaceTable[1].pfnFinder = 0;
  InterfaceTable[1].dwData = 0;
  return InterfaceTable;
}

#endif

long csPolygonSet::current_light_frame_number = 0;

CSOBJTYPE_IMPL(csPolygonSet,csObject);

csPolygonSet::csPolygonSet () : csObject(), csPolygonParentInt ()
{
  max_vertices = num_vertices = 0;
  max_polygon = num_polygon = 0;

  num_curves = max_curves = 0;
  curves = NULL;

  curves_center.x = curves_center.y = curves_center.z = 0;
  curves_scale=40;  
  curve_vertices = NULL;
  curve_texels = NULL;
  num_curve_vertices = max_curve_vertices = 0;

  wor_verts = NULL;
  obj_verts = NULL;
  cam_verts = NULL;
  polygons = NULL;
  bsp = NULL;
  draw_busy = 0;
  fog.enabled = false;

  light_frame_number = -1;
}

csPolygonSet::~csPolygonSet ()
{
  CHK (delete [] wor_verts);
  CHK (delete [] obj_verts);
  CHK (delete [] cam_verts);
  if (polygons)
  {
    for (int i = 0 ; i < num_polygon ; i++)
      CHKB (delete polygons [i]);
    CHK (delete [] polygons);
  }

  if (curves)
  {
    for (int i = 0 ; i < num_curves ; i++)
      CHKB (delete curves [i]);
    CHK (delete [] curves);
  }
  CHK (delete [] curve_vertices);
  CHK (delete [] curve_texels);
  CHK (delete bsp);
}

void csPolygonSet::Prepare ()
{
  int i;
  for (i = 0 ; i < num_polygon ; i++)
  {
    csPolygon3D* p = (csPolygon3D*)polygons[i];
    p->Finish ();
  }
}

void csPolygonSet::AddCurveVertex (csVector3& v, csVector2& t)
{
  if (!curve_vertices)
  {
    max_curve_vertices = 10;
    CHK (curve_vertices = new csVector3 [max_curve_vertices]);
    CHK (curve_texels   = new csVector2 [max_curve_vertices]);
  }
  while (num_curve_vertices >= max_curve_vertices)
  {
    max_curve_vertices += 10;
    CHK (csVector3* new_vertices = new csVector3 [max_curve_vertices]);
    CHK (csVector2* new_texels   = new csVector2 [max_curve_vertices]);
    memcpy (new_vertices, curve_vertices, sizeof (csVector3)*num_curve_vertices);
    memcpy (new_texels,   curve_texels,   sizeof (csVector2)*num_curve_vertices);
    CHK (delete [] curve_vertices);
    CHK (delete [] curve_texels);
    curve_vertices = new_vertices;
    curve_texels   = new_texels;
  }

  curve_vertices[num_curve_vertices] = v;
  curve_texels[num_curve_vertices] = t;
  num_curve_vertices++;
}

int csPolygonSet::AddVertex (float x, float y, float z)
{
  if (!wor_verts)
  {
    max_vertices = 10;
    CHK (wor_verts = new csVector3 [max_vertices]);
    CHK (obj_verts = new csVector3 [max_vertices]);
    CHK (cam_verts = new csVector3 [max_vertices]);
  }
  while (num_vertices >= max_vertices)
  {
    max_vertices += 10;
    CHK (csVector3* new_wor_verts = new csVector3 [max_vertices]);
    CHK (csVector3* new_obj_verts = new csVector3 [max_vertices]);
    CHK (csVector3* new_cam_verts = new csVector3 [max_vertices]);
    memcpy (new_wor_verts, wor_verts, sizeof (csVector3)*num_vertices);
    memcpy (new_obj_verts, obj_verts, sizeof (csVector3)*num_vertices);
    memcpy (new_cam_verts, cam_verts, sizeof (csVector3)*num_vertices);

    CHK (delete [] wor_verts);
    CHK (delete [] obj_verts);
    CHK (delete [] cam_verts);

    wor_verts = new_wor_verts;
    obj_verts = new_obj_verts;
    cam_verts = new_cam_verts;
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

csPolygon3D* csPolygonSet::GetPolygon (char* name)
{
  int i;
  for (i = 0 ; i < num_polygon ; i++)
  {
    csPolygon3D* p = (csPolygon3D*)polygons[i];
    if (!strcmp (csNameObject::GetName(*p), name)) return p;
  }
  return NULL;
}

csPolygon3D* csPolygonSet::NewPolygon (csTextureHandle* texture)
{
  CHK (csPolygon3D* p = new csPolygon3D (texture));
  p->SetSector (sector);
  AddPolygon (p);
  return p;
}

void csPolygonSet::AddPolygon (csPolygonInt* poly)
{
  if (!polygons)
  {
    max_polygon = 6;
    CHK (polygons = new csPolygonInt* [max_polygon]);
  }
  while (num_polygon >= max_polygon)
  {
    max_polygon += 6;
    CHK (csPolygonInt** new_polygons = new csPolygonInt* [max_polygon]);
    memcpy (new_polygons, polygons, sizeof (csPolygonInt*)*num_polygon);
    CHK (delete [] polygons);
    polygons = new_polygons;
  }

  polygons[num_polygon++] = poly;
  poly->SetParent (this);
}


csCurve* csPolygonSet::GetCurve (char* name)
{
  int i;
  for (i = 0 ; i < num_curves ; i++)
  {
    csCurve* p = (csCurve*)polygons[i];
    if (!strcmp (csNameObject::GetName(*p), name)) return p;
  }
  return NULL;
}

/*TODO weg?

csCurve* csPolygonSet::new_bezier (char* name, TextureMM* texture)
{
  CHK (csBezier* p = new csBezier (name, 0x1234));
  p->setTexture(texture);
  //TODO??  p->set_sector (sector);
  AddCurve (p);
  return p;
}
*/

void csPolygonSet::AddCurve (csCurve* curve)
{
  if (!curves)
  {
    max_curves = 6;
    CHK (curves = new csCurve* [max_curves]);
  }
  while (num_curves >= max_curves)
  {
    max_curves += 6;
    CHK (csCurve** new_curves = new csCurve* [max_curves]);
    memcpy (new_curves, curves, sizeof (csCurve*)*num_curves);
    CHK (delete [] curves);
    curves = new_curves;
  }
  curves[num_curves++] = curve;
  curve->SetParent (this);
}


void csPolygonSet::UseBSP ()
{
  CHK (delete bsp);
  CHK (bsp = new csBspTree (this));
}

struct IntersInfo
{
  const csVector3* start;
  const csVector3* end;
  csVector3* isect;
  float* pr;
};

void* test_bsp_intersection (csPolygonParentInt*, csPolygonInt** polygon, int num,
	void* data)
{
  IntersInfo* d = (IntersInfo*)data;
  int i;
  for (i = 0 ; i < num ; i++)
  {
    csPolygon3D* p = (csPolygon3D*)polygon[i];
    if (p->IntersectSegment (*d->start, *d->end, *d->isect, d->pr))
      return (void*)p;
  }
  return NULL;
}

csPolygon3D* csPolygonSet::IntersectSegment (const csVector3& start, 
        const csVector3& end, csVector3& isect, float* pr)
{
  int i;
  if (bsp)
  {
    IntersInfo d;
    d.start = &start;
    d.end = &end;
    d.isect = &isect;
    d.pr = pr;
    return (csPolygon3D*) bsp->Front2Back (start, &test_bsp_intersection, (void*)&d);
  }
  else
    for (i = 0 ; i < num_polygon ; i++)
    {
      csPolygon3D* p = (csPolygon3D*)polygons[i];
      if (p->IntersectSegment (start, end, isect, pr)) return p;
    }
  return NULL;
}

void csPolygonSet::TranslateVector (csVector3& trans)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    cam_verts[i] = wor_verts[i]-trans;
}

bool csPolygonSet::TransformWorld2Cam (csCamera& c)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    cam_verts[i] = c.Other2This (wor_verts[i]);

  return true;
}

void csPolygonSet::NewTransformation (csVector3*& old_tr3)
{
  if (draw_busy == 1)
  {
    // This is the first time we need a transformation this session.
    // Just return NULL and use the default cam_verts array.
    old_tr3 = NULL;
  }
  else
  {
    // Allocate a new cam_verts array.
    old_tr3 = cam_verts;
    CHK (cam_verts = new csVector3 [max_vertices]);
  }
}

void csPolygonSet::RestoreTransformation (csVector3* old_tr3)
{
  if (old_tr3)
  {
    CHKB (delete [] cam_verts);
    cam_verts = old_tr3;
  }
}

void csPolygonSet::DrawPolygonArray (csPolygonInt** polygon, int num, csRenderView* d,
	bool use_z_buf)
{
  csPolygon3D* p;
  csPortal* po;
  csVector3* verts;
  int num_verts;
  int i;
  
  csWireFrame* wf = NULL;
  if (csWorld::current_world->map_mode != MAP_OFF) wf = csWorld::current_world->wf->GetWireframe ();
  
  for (i = 0 ; i < num ; i++)
  {
    csVector2 orig_triangle[3];
    p = (csPolygon3D*)polygon[i];
    if (p->GetUVCoords ())
    {
      //@@@ NOT OPTIMAL AT ALL!
      //@@@@@@@@
      float iz;
      if (p->Vcam (0).z) iz = csCamera::aspect/p->Vcam (0).z;
      else iz = 1000000.;
      orig_triangle[0].x = p->Vcam (0).x * iz + csWorld::shift_x;
      orig_triangle[0].y = p->Vcam (0).y * iz + csWorld::shift_y;
      if (p->Vcam (1).z) iz = csCamera::aspect/p->Vcam (1).z;
      else iz = 1000000.;
      orig_triangle[1].x = p->Vcam (1).x * iz + csWorld::shift_x;
      orig_triangle[1].y = p->Vcam (1).y * iz + csWorld::shift_y;
      if (p->Vcam (2).z) iz = csCamera::aspect/p->Vcam (2).z;
      else iz = 1000000.;
      orig_triangle[2].x = p->Vcam (2).x * iz + csWorld::shift_x;
      orig_triangle[2].y = p->Vcam (2).y * iz + csWorld::shift_y;
    }

    if ( !p->dont_draw &&
         p->ClipToPlane (d->do_clip_plane ? &d->clip_plane : (csPlane*)NULL, d->GetOrigin (),
	 	verts, num_verts) && 
         p->DoPerspective (*d, verts, num_verts, &csPolygon2D::clipped, orig_triangle, d->IsMirrored ())  &&
         csPolygon2D::clipped.ClipAgainst (d->view) )
    {
      if (wf)
      {
        int j;
        csWfPolygon* po = wf->AddPolygon ();
        po->SetVisColor (wf->GetYellow ());
        po->SetNumVertices (p->GetNumVertices ());
        for (j = 0 ; j < p->GetNumVertices () ; j++) po->SetVertex (j, p->Vwor (j));
        po->Prepare ();
      }

      Stats::polygons_drawn++;

      po = p->GetPortal ();
      if (csSector::do_portals && po)
      {
	bool filtered = false;
	// is_this_fog is true if this sector is fogged.
	bool is_this_fog = sector->HasFog ();

        // If there is filtering (alpha mapping or something like that) we need to keep the
        // 2D polygon and texture plane so that it can be drawn after the sector has been drawn.
        // We can't just use 'clipped' because calling Portal->Draw will (possibly)
        // reuse the clipped global variable.
        // The texture plane needs to be kept because this polygon may be rendered again
        // (through mirrors) possibly overwriting the plane.
        csPolygon2D* keep_clipped = NULL;
        csPolyPlane* keep_plane = NULL;
              
        long do_transp;
        d->g3d->GetRenderState (G3DRENDERSTATE_TRANSPARENCYENABLE, do_transp);
	if (do_transp)
          filtered = p->IsTransparent ();
              
        if (filtered || is_this_fog)
        {
          CHK (keep_clipped = new csPolygon2D (csPolygon2D::clipped));
          CHK (keep_plane = new csPolyPlane (*(p->GetPlane ())));
        }
              
        // Draw through the portal. If this fails we draw the original polygon
        // instead. Drawing through a portal can fail because we have reached
        // the maximum number that a sector is drawn (for mirrors).
	// Note that use_z_buf is given to this routine as well but this
	// parameter has nothing to do with the Z buffer but with the fact that
	// we have a portal in the middle of a sector. use_z_buf happens to be
	// true for Things and false for sectors so this works now.
        if (po->Draw (&csPolygon2D::clipped, &p->GetPlane ()->GetCameraPlane (), use_z_buf, *d))
        {
          if (filtered)
	    keep_clipped->DrawFilled (d->g3d, p, keep_plane, d->IsMirrored (),
	    	use_z_buf, orig_triangle);
	  if (is_this_fog) keep_clipped->AddFogPolygon (d->g3d, p, keep_plane, d->IsMirrored (),
	  	sector->GetID (), CS_FOG_BACK);
        }
        else csPolygon2D::clipped.DrawFilled (d->g3d, p, p->GetPlane (), d->IsMirrored (),
		use_z_buf, orig_triangle);
              
        // Cleanup.
        if (keep_clipped)
        {
          CHK (delete keep_clipped);
          CHK (delete keep_plane);
        }
      }
      else
        csPolygon2D::clipped.DrawFilled (d->g3d, p, p->GetPlane (), d->IsMirrored (),
      	  use_z_buf, orig_triangle);
          
      long do_edges;
      d->g3d->GetRenderState (G3DRENDERSTATE_EDGESENABLE, do_edges);
      if (do_edges)
      {
        ITextureManager* txtmgr;
        d->g3d->GetTextureManager (&txtmgr);
        int white;
        txtmgr->FindRGB (255, 255, 255, white);
        csPolygon2D::clipped.Draw (d->g2d, white);
      }
    }
  }
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

#if 0
bool ClipToPlane (csPlane* plane, csVector3*& p_in, int p_in_num,
	csVector3*& p_out, int& p_out_num, bool cw)
{
  int i, i1, cnt_vis;
  float r;
  bool zs, z1s;

  // Assume maximum 100 vertices! (@@@ HARDCODED LIMIT)
  static csVector3 verts[100];
  bool vis[100];

  // Copy the vertices to verts.
  for (i = 0 ; i < p_in_num ; i++) verts[i] = p_in[i];
  p_out = verts;

  // We clip the p_in polygon in 3D against the plane.

  // First count how many vertices are before the plane
  // (so are visible as seen from the plane).
  cnt_vis = 0;
  for (i = 0 ; i < num_vertices ; i++)
  {
    vis[i] = csMath3::Visible (p_in[i], *plane);
    if (vis[i]) cnt_vis++;
  }

  if (cnt_vis == 0) return false; // Polygon is not visible.

  // If all vertices are visible then no clipping needed.
  if (cnt_vis == p_in_num) { p_out_num = p_in_num; return true; }

  // We really need to clip.
  p_out_num = 0;

  float A = plane->A ();
  float B = plane->B ();
  float C = plane->C ();
  float D = plane->D ();

  i1 = p_in_num-1;
  for (i = 0 ; i < p_in_num ; i++)
  {
    zs = !vis[i];
    z1s = !vis[i1];

    if (z1s && !zs)
    {
      csIntersect3::Plane (Vcam (i1), Vcam (i), A, B, C, D, verts[num_verts], r);
      num_verts++;
      verts[num_verts++] = Vcam (i);
    }
    else if (!z1s && zs)
    {
      csIntersect3::Plane (Vcam (i1), Vcam (i), A, B, C, D, verts[num_verts], r);
      num_verts++;
    }
    else if (!z1s && !zs)
    {
      verts[num_verts++] = Vcam (i);
    }
    i1 = i;
  }

  return true;
}
#endif

csVector2 *csPolygonSet::IntersectCameraZPlane (float z, csVector2* clipper,
	int num_clip, int& num_pts)
{
#if 0
  int i, j;
  csVector* new_pol;
  for (i = 0 ; i < num_polygon ; i++)
  {
    // Allocate a new polygon and preserve space for two potential extra vertices.
    CHK (new_pol = new csVector2 [num_clip+2]);
    for (j = 0 ; j < num_clip ; j++)
  }
#endif
  return NULL;
}
