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
#include "csengine/polyset.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/dynlight.h"
#include "csengine/world.h"
#include "csengine/camera.h"
#include "csengine/sector.h"
#include "csengine/curve.h"
#include "csengine/rview.h"
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
  bbox = NULL;

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
      CHKB (delete (csPolygon3D*)polygons [i]);
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
  CHK (delete bbox);
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
    const char* n = csNameObject::GetName (*p);
    if (n && !strcmp (n, name)) return p;
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

#if 1
bool csPolygonSet::TransformWorld2Cam (csCamera& c)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    cam_verts[i] = c.Other2This (wor_verts[i]);

  return true;
}
#else
bool csPolygonSet::TransformWorld2Cam (csCamera& c)
{
  // This loop has been made explicit because this makes better
  // usage of the cache.
  int i;
 
  float dx, dy, dz;
  float cx, cy, cz;
 
  const csMatrix3& m_o2t = c.GetO2T ();
  const csVector3& v_o2t = c.GetO2TTranslation ();

  float m11 = m_o2t.m11;
  float m21 = m_o2t.m21;
  float m31 = m_o2t.m31;
 
  float m12 = m_o2t.m12;
  float m22 = m_o2t.m22;
  float m32 = m_o2t.m32;
  
  float m13 = m_o2t.m13;
  float m23 = m_o2t.m23;
  float m33 = m_o2t.m33;
 
  cx = v_o2t.x;
  cy = v_o2t.y;
  cz = v_o2t.z;
 
  csVector3* world_verts = wor_verts;
  csVector3* camra_verts = cam_verts;
  for (i = 0 ; i < num_vertices ; i++)
  {        
    dx = world_verts->x - cx;
    dy = world_verts->y - cy;
    dz = world_verts->z - cz; 
 
    camra_verts->x  = m11 * dx + m12 * dy + m13 *dz;
    camra_verts->y  = m21 * dx + m22 * dy + m23 *dz;
    camra_verts->z  = m31 * dx + m32 * dy + m33 *dz;

    world_verts++;
    camra_verts++;
  }
 
  return true;
}
#endif

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
  
  for (i = 0 ; i < num ; i++)
  {
    p = (csPolygon3D*)polygon[i];
    if ( !p->dont_draw &&
         p->ClipToPlane (d->do_clip_plane ? &d->clip_plane : (csPlane*)NULL, d->GetOrigin (),
	 	verts, num_verts) && 
         p->DoPerspective (*d, verts, num_verts, &csPolygon2D::clipped,
	 	NULL/*orig_triangle*/, d->IsMirrored ())  &&
         csPolygon2D::clipped.ClipAgainst (d->view) )
    {
      if (d->callback)
      {
        d->callback (d, CALLBACK_POLYGON, (void*)p);
        d->callback (d, CALLBACK_POLYGON2D, (void*)&csPolygon2D::clipped);
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
	  if (!d->callback)
	  {
	    if (filtered)
	      keep_clipped->DrawFilled (d->g3d, p, keep_plane, d->IsMirrored (),
		  use_z_buf);
	    if (is_this_fog) keep_clipped->AddFogPolygon (d->g3d, p, keep_plane, d->IsMirrored (),
		  sector->GetID (), CS_FOG_BACK);
	  }
        }
        else if (!d->callback)
	  csPolygon2D::clipped.DrawFilled (d->g3d, p, p->GetPlane (), d->IsMirrored (),
		use_z_buf);
              
        // Cleanup.
        if (keep_clipped)
        {
          CHK (delete keep_clipped);
          CHK (delete keep_plane);
        }
      }
      else if (!d->callback)
        csPolygon2D::clipped.DrawFilled (d->g3d, p, p->GetPlane (), d->IsMirrored (),
      	  use_z_buf);
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

// @@@ We need a more clever algorithm here. We should try
// to recognize convex sub-parts of a polygonset and return
// convex shadow frustrums for those. This will significantly
// reduce the number of shadow frustrums. There are basicly
// two ways to do this:
//	- Split object into convex sub-parts in 3D.
//	- Split object into convex sub-parts in 2D.
// The first way is probably best because it is more efficient
// at runtime (important if we plan to use dynamic shadows for things)
// and also more correct in that a convex 3D object has no internal
// shadowing while a convex outline may have no correspondance to internal
// shadows.
csFrustrumList* csPolygonSet::GetShadows (csVector3& origin)
{
  CHK (csFrustrumList* list = new csFrustrumList ());
  csShadowFrustrum* frust;
  int i, j;
  csPolygon3D* p;
  bool cw = true; //@@@ Use mirroring parameter here!
  for (i = 0 ; i < num_polygon ; i++)
  {
    p = (csPolygon3D*)polygons[i];
    if (p->GetPlane ()->VisibleFromPoint (origin) != cw) continue;
    CHK (frust = new csShadowFrustrum (origin));
    list->AddFirst (frust);
    csPlane pl = p->GetPlane ()->GetWorldPlane ();
    pl.DD += origin * pl.norm;
    pl.Invert ();
    frust->SetBackPlane (pl);
    frust->polygon = p;
    for (j = 0 ; j < p->GetNumVertices () ; j++)
      frust->AddVertex (p->Vwor (j)-origin);
  }
  return list;
}

void csPolygonSet::CreateBoundingBox ()
{
  float minx, miny, minz, maxx, maxy, maxz;
  CHK (delete bbox); bbox = NULL;
  if (num_vertices <= 0) return;
  CHK (bbox = new csPolygonSetBBox ());
  minx = maxx = obj_verts[0].x;
  miny = maxy = obj_verts[0].y;
  minz = maxz = obj_verts[0].z;
  int i;
  for (i = 1 ; i < num_vertices ; i++)
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

  bbox->i7 = AddVertexSmart (minx, miny, minz);
  bbox->i3 = AddVertexSmart (minx, miny, maxz);
  bbox->i5 = AddVertexSmart (minx, maxy, minz);
  bbox->i1 = AddVertexSmart (minx, maxy, maxz);
  bbox->i8 = AddVertexSmart (maxx, miny, minz);
  bbox->i4 = AddVertexSmart (maxx, miny, maxz);
  bbox->i6 = AddVertexSmart (maxx, maxy, minz);
  bbox->i2 = AddVertexSmart (maxx, maxy, maxz);
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
  for (i=2 ; i<n ; i++) 
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
 * Someone from Microsoft wrote this, but copyrights are absent. So I assume that it's also in public
 * domain ;). However, it's not a very critical function, I just was very lazy and didn't want to write
 * my own Convex-Hull finder ;). -DDK
 */

void Find2DConvexHull (int nverts, csVector2 *pntptr, int *cNumOutIdxs, int **OutHullIdxs)
{
  csVector2 **PntPtrs;
  int i;

  *cNumOutIdxs=0;               //max space needed is n+1 indices
  *OutHullIdxs = (int *)malloc((nverts+1)*(sizeof(int)+sizeof(csVector2 *)));

  PntPtrs = (csVector2**) &(*OutHullIdxs)[nverts+1];

  // alg requires array of ptrs to verts (for qsort) instead of array of verts, so do the conversion
  for (i=0 ; i<nverts ; i++)
    PntPtrs[i] = &pntptr[i];

  *cNumOutIdxs=ch2d (PntPtrs, nverts);

  // convert back to array of idxs
  for (i=0 ; i<*cNumOutIdxs ; i++)
    (*OutHullIdxs)[i]= (int) (PntPtrs[i]-&pntptr[0]);

  // caller will free returned array
}

//---------------------------------------------------------------------------------------------------------

int find_chull (int nverts, csVector2 *vertices, csVector2 *&chull)
{
  int num;
  int *order;

  Find2DConvexHull (nverts, vertices, &num, &order);

  CHK(chull = new csVector2[num]);
  for (int i=0 ; i<num ; i++)
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

  for (i=0 ; i<num_polygon ; i++)
  {
    csPolygon3D *p=(csPolygon3D*)polygons[i];
    int nv=p->GetNumVertices ();
    int *v_i=p->GetVerticesIdx ();

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

	CHK (head->next=new point_list);
	head=head->next;
      }
    }
  }

  csVector2 *final_data,*data;
  CHK (data=new csVector2[num_pts]);
  for (head=&list,i=0 ; i<num_pts ; i++)
  {
    data[i]=head->data;

    prev=head;
    head=head->next;
    if (prev!=&list) { CHK (delete prev); }
  }

  if (i) { CHK (delete[] head); }

  num_vertices = find_chull (num_pts, data, final_data);
  if (num_pts) { CHK (delete data); }

  return final_data;
}

