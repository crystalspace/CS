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
#include "csengine/sysitf.h"
#include "csengine/sector.h"
#include "csengine/objects/thing.h"
#include "csengine/objects/cssprite.h"
#include "csengine/polygon/polygon.h"
#include "csengine/polygon/polytext.h"
#include "csengine/light/dynlight.h"
#include "csengine/light/light.h"
#include "csengine/camera.h"
#include "csengine/world.h"
#include "csengine/halo.h"
#include "csengine/stats.h"
#include "csengine/wirefrm.h"
#include "csgeom/bsp.h"
#include "csobject/nameobj.h"
#include "ihalo.h"
#include "igraph3d.h"
#include "igraph2d.h"
#include "itxtmgr.h"
#include "itexture.h"

// Option variable: render portals?
bool csSector::do_portals = true;
// Option variable: render things?
bool csSector::do_things = true;
// Configuration variable: number of allowed reflections for static lighting.
int csSector::cfg_reflections = 1;
// Option variable: do pseudo radiosity?
bool csSector::do_radiosity = false;

//---------------------------------------------------------------------------

CSOBJTYPE_IMPL(csSector,csPolygonSet);

csSector::csSector () : csPolygonSet ()
{
  first_thing = NULL;
  sector = this;
  beam_busy = 0;
  level_r = level_g = level_b = 0;
  static_bsp = NULL;
  static_thing = NULL;
}

csSector::~csSector ()
{
  while (first_thing)
  {
    csThing* n = (csThing*)(first_thing->GetNext ());
    CHK (delete first_thing);
    first_thing = n;
  }
  CHK (delete static_bsp);
  CHK (delete static_thing);

  // The sprites are not deleted here because they can occur in more
  // than one sector at the same time. Therefor we first clear the list.
  int i;
  for (i = 0 ; i < sprites.Length (); i++) sprites[i] = NULL;
  sprites.DeleteAll ();

  lights.DeleteAll ();
}

void csSector::Prepare ()
{
  csPolygonSet::Prepare ();
  csThing* th = first_thing;
  while (th)
  {
    th->Prepare ();
    th = (csThing*)(th->GetNext ());
  }
}

void csSector::AddThing (csThing* thing)
{
  thing->SetNext ((csPolygonSet*)first_thing);
  first_thing = thing;
}

void csSector::AddLight (csStatLight* light)
{
  lights.Push (light);
  light->SetSector (this);
}

void csSector::UseStaticBSP ()
{
  CHK (delete bsp); bsp = NULL;
  CHK (delete static_bsp); static_bsp = NULL;

  CHK (delete static_thing);
  CHK (static_thing = new csThing());
  csNameObject::AddName(*static_thing, "__static__");

  static_thing->SetSector (this);
  csThing* sp = first_thing;
  while (sp)
  {
    if (!sp->IsMoveable () && !sp->GetFog ().enabled) static_thing->Merge (sp);
    sp = (csThing*)(sp->GetNext ());
  }

  CHK (static_bsp = new csBspTree (static_thing));
}

csPolygon3D* csSector::HitBeam (csVector3& start, csVector3& end)
{
  csVector3 isect;
  csPolygon3D* p;

  // First check the things of this sector and return the one with
  // the closest distance.
  csThing* sp = first_thing;
  float sq_dist, min_sq_dist = 100000000.;
  csPolygon3D* min_poly = NULL;
  while (sp)
  {
    p = sp->IntersectSegment (start, end, isect);
    if (p)
    {
      sq_dist = (isect.x-start.x)*(isect.x-start.x) +
	(isect.y-start.y)*(isect.y-start.y) +
	(isect.z-start.z)*(isect.z-start.z);
      if (sq_dist < min_sq_dist) { min_sq_dist = sq_dist; min_poly = p; }
    }
    sp = (csThing*)(sp->GetNext ());
  }

  if (min_poly) return min_poly;

  p = IntersectSegment (start, end, isect);
  if (p)
  {
    csPortal* po = p->GetPortal ();
    if (po) return po->HitBeam (start, end);
    else return p;
  }
  else return NULL;
}

void csSector::CreateLightmaps (IGraphics3D* g3d)
{
  int i;
  for (i = 0 ; i < num_polygon ; i++)
  {
    csPolygon3D* p = (csPolygon3D*)polygons[i];
    p->CreateLightmaps (g3d);
  }

  csThing* sp = first_thing;
  while (sp)
  {
    sp->CreateLightmaps (g3d);
    sp = (csThing*)(sp->GetNext ());
  }
}

csPolygon3D* csSector::IntersectSegment (const csVector3& start, const csVector3& end,
				      csVector3& isect, float* pr)
{
  csThing* sp = first_thing;
  while (sp)
  {
    csPolygon3D* p = sp->IntersectSegment (start, end, isect, pr);
    if (p) return p;
    sp = (csThing*)(sp->GetNext ());
  }

  return csPolygonSet::IntersectSegment (start, end, isect, pr);
}

csSector* csSector::FollowSegment (csReversibleTransform& t, csVector3& new_position,
				bool& mirror)
{
  csVector3 isect;
  csPolygon3D* p = sector->IntersectSegment (t.GetOrigin (), new_position, isect);
  csPortal* po;

  if (p)
  {
    po = p->GetPortal ();
    if (po)
      return po->FollowSegment (t, new_position, mirror);
    else
      new_position = isect;
  }

  return this;
}


csPolygon3D* csSector::IntersectSphere (csVector3& center, float radius, float* pr)
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
	csPortal* po = p->GetPortal ();
	if (po)
	{
	  p = po->IntersectSphere (center, min_d, &d);
	  if (p)
	  {
	    min_d = d;
	    min_p = p;
	  }
	}
	else
	{
	  min_d = d;
	  min_p = p;
	}
      }
    }
  }

  csThing* sp = first_thing;
  while (sp)
  {
    p = sp->IntersectSphere (center, radius, &d);
    if (p && d < min_d)
    {
      min_d = d;
      min_p = p;
    }
    sp = (csThing*)(sp->GetNext ());
  }

  if (pr) *pr = min_d;
  return min_p;
}

void* csSector::DrawPolygons (csPolygonParentInt* pi, csPolygonInt** polygon, int num, void* data)
{
  csRenderView* d = (csRenderView*)data;
  csSector* sector = (csSector*)pi;
  sector->DrawPolygonArray (polygon, num, d, false);
  return NULL;
}

int compare_z_thing (const void* p1, const void* p2)
{
  csThing* sp1 = *(csThing**)p1;
  csThing* sp2 = *(csThing**)p2;
  float z1 = sp1->Vcam (sp1->GetCenter ()).z;
  float z2 = sp2->Vcam (sp2->GetCenter ()).z;
  if (z1 < z2) return -1;
  else if (z1 > z2) return 1;
  return 0;
}

void csSector::Draw (csRenderView& rview)
{
  draw_busy++;
  csVector3* old_tr3;
  NewTransformation (old_tr3);
  TransformWorld2Cam (rview);
  Stats::polygons_considered += num_polygon;

  G3D_FOGMETHOD fogmethod;
  if (HasFog ())
  {
    rview.g3d->OpenFogObject (GetID (), &GetFog ());
    rview.g3d->GetFogMode (fogmethod);
  }

  if (bsp)
    bsp->Back2Front (rview.GetOrigin (), &DrawPolygons, (void*)&rview);
  else
  {
    DrawPolygons ((csPolygonParentInt*)this, polygons, num_polygon, (void*)&rview);
    if (static_thing && do_things)
    {
      csVector3* old_tr3;
      static_thing->NewTransformation (old_tr3);
      static_thing->TransformWorld2Cam (rview);
      static_bsp->Back2Front (rview.GetOrigin (), &DrawPolygons, (void*)&rview);
      static_thing->RestoreTransformation (old_tr3);
    }
  }

  if (do_things)
  {
    // All csThings which are not merged with the static bsp still need to
    // be drawn. If there is a static bsp then we cannot do something special
    // for convex things (hulls). In that case we first draw the non-merged
    // things using normal Z-buffer and only Z sort the foggy things and draw
    // them back to front. If there is no static bsp then we can also Z sort
    // the hulls and draw then back to front using Z-fill.
    //
    // A consequence of this algorithm is that if you use a static bsp then
    // you can only use portals in csThings that do not move (i.e. the csThing
    // needs to be merged with the bsp). But the csThing need not be convex in
    // that case. If you don't use a static bsp then you can use portals in
    // moving csThings but the csThings containing portals need to be convex.
    // a static bsp then you can only use portals 
    //
    // We should see if there are better alternatives to Z-sort which are
    // more accurate in more cases (@@@).
    csThing* sort_list[100];	// @@@HARDCODED == BAD == EASY!
    int sort_idx = 0;
    int i;
    csThing* sp = first_thing;
    if (static_thing)
    {
      // Here we have a static bsp. So we draw all csThings using the
      // Z-buffer and put all foggy csThings in the sort_list.
      while (sp)
      {
        if (!sp->IsMerged ())
          if (sp->GetFog ().enabled) sort_list[sort_idx++] = sp;
          else sp->Draw (rview);
        sp = (csThing*)(sp->GetNext ());
      }
    }
    else
    {
      // Here we don't have a static bsp. In this case we put all
      // hulls (convex csThings) and foggy csThings in the sort_list.
      while (sp)
      {
        if (sp->IsConvex () || sp->GetFog ().enabled)
	  sort_list[sort_idx++] = sp;
        sp = (csThing*)(sp->GetNext ());
      }
    }

    if (sort_idx)
    {
      // Now sort the objects in sort_list.
      qsort (sort_list, sort_idx, sizeof (csThing*), compare_z_thing);

      // Draw them back to front.
      for (i = 0 ; i < sort_idx ; i++)
      {
        sp = sort_list[i];
        if (sp->GetFog ().enabled) sp->DrawFoggy (rview);
        else if (!sp->IsMerged () && sp->IsConvex ())
      	  sp->Draw (rview, false);
      }
    }

    // If there is no static bsp then we still need to draw the remaining
    // non-convex csThings.
    if (!static_thing)
    {
      sp = first_thing;
      while (sp)
      {
        if (!sp->IsConvex () && !sp->GetFog ().enabled) sp->Draw (rview);
        sp = (csThing*)(sp->GetNext ());
      }
    }

    // Draw sprites.
    for (i = 0 ; i < sprites.Length (); i++)
    {
      csSprite3D* sp3d = (csSprite3D*)sprites[i];
      sp3d->Draw (rview);
    }
  }

  // queue all halos in this sector to be drawn.
  IHaloRasterizer* piHR = csWorld::current_world->GetHaloRastizer ();
  if (piHR)
  {
    int numlights = lights.Length();
    
    for (int i = 0; i<numlights; i++)
    {
      csStatLight* light = (csStatLight*)(lights[i]);
      if (!light->IsHaloEnabled ()) continue;
      
      // this light is already in the queue.
      if (light->GetHaloInQueue ())
	continue;
      
      csHaloInformation* cshaloinfo = new csHaloInformation; 
      
      cshaloinfo->v = rview.World2Camera (light->GetCenter ());
      
      if (cshaloinfo->v.z > SMALL_Z)
      {
	float iz = rview.aspect/cshaloinfo->v.z;
	cshaloinfo->v.x = cshaloinfo->v.x * iz + csWorld::shift_x;
	cshaloinfo->v.y = csWorld::frame_height - 1 - (cshaloinfo->v.y * iz + csWorld::shift_y);

	cshaloinfo->pLight = light;
	
	cshaloinfo->r = light->GetColor ().red;
	cshaloinfo->g = light->GetColor ().green;
	cshaloinfo->b = light->GetColor ().blue;
	cshaloinfo->intensity = 0.0f;
	
	if (piHR->TestHalo(&cshaloinfo->v) == S_OK)
	{
	  piHR->CreateHalo(cshaloinfo->r, cshaloinfo->g, cshaloinfo->b, &cshaloinfo->haloinfo);
	  csWorld::current_world->AddHalo (cshaloinfo);
	}
      }
      else
      {
	delete cshaloinfo;
      }
    }
  }

  if (HasFog ())
  {
    G3DPolygon g3dpoly;
    int i;
    if (fogmethod == G3DFOGMETHOD_ZBUFFER)
    {
      g3dpoly.num = rview.view->GetNumVertices ();
      if (rview.GetSector () == this)
      {
        // Since there is fog in the current camera sector we simulate
        // this by adding the view plane polygon.
        for (i = 0 ; i < g3dpoly.num ; i++)
        {
          g3dpoly.vertices[g3dpoly.num-i-1].sx = rview.view->GetVertex (i).x;
          g3dpoly.vertices[g3dpoly.num-i-1].sy = rview.view->GetVertex (i).y;
        }
        rview.g3d->AddFogPolygon (GetID (), g3dpoly, CS_FOG_VIEW);
      }
      else
      {
        // We must add a FRONT fog polygon for the clipper to this sector.
        if (rview.IsMirrored ())
          for (i = 0 ; i < g3dpoly.num ; i++)
          {
            g3dpoly.vertices[g3dpoly.num-i-1].sx = rview.view->GetVertex (i).x;
            g3dpoly.vertices[g3dpoly.num-i-1].sy = rview.view->GetVertex (i).y;
          }
        else
          for (i = 0 ; i < g3dpoly.num ; i++)
          {
            g3dpoly.vertices[i].sx = rview.view->GetVertex (i).x;
            g3dpoly.vertices[i].sy = rview.view->GetVertex (i).y;
          }
        g3dpoly.normal.A = -rview.clip_plane.A ();
        g3dpoly.normal.B = -rview.clip_plane.B ();
        g3dpoly.normal.C = -rview.clip_plane.C ();
        g3dpoly.normal.D = -rview.clip_plane.D ();
        g3dpoly.inv_aspect = csCamera::inv_aspect;
        rview.g3d->AddFogPolygon (GetID (), g3dpoly, CS_FOG_FRONT);
      }
    }
    else
    {
#if 0
#     define FOG_PLANE_DIST .3			//@@@SHOULD BE CONFIGURATION VALUE!
      float minz, maxz;
      GetCameraMinMaxZ (minz, maxz);
      int num = QInt ((maxz-minz) / FOG_PLANE_DIST);	// Number of planes
      int p;
      csVector2* pts;
      int num_pts;
      g3dpoly.inv_aspect = csCamera::inv_aspect;

      CHK (csVector2* clipper = new csVector2 [rview.view->GetNumVertices ()]);

      // We divide the object in planes parallel to the view plane.
      for (p = 1 ; p < num ; p++)
      {
        float z = maxz - FOG_PLANE_DIST*(float)p;
	if (z < SMALL_Z) break;
	float iz = csCamera::aspect/z;

	// Take the current clipper and un-perspective project it back
	// to camera space.
	for (i = 0 ; i < rview.view->GetNumVertices () ; i++)
	{
	  clipper[i].x = (rview.view->GetVertex (i).x - csWorld::shift_x) * z;
	  clipper[i].y = (rview.view->GetVertex (i).y - csWorld::shift_y) * z;
	}

	// Clip the clipper to the polygonset.
	pts = IntersectCameraZPlane (z, clipper, rview.view->GetNumVertices (), num_pts);

	// Reverse vertex order.
	//for (i = 0 ; i < num_pts/2 ; i++)
        //{
	  //csVector2 sw = pts[i];
	  //pts[i] = pts[num_pts-i-1];
	  //pts[num_pts-i-1] = sw;
        //}

	if (pts)
	{
	  // Perspective projection and copy to structure for 3D rasterizer.
	  g3dpoly.num = num_pts;
	  for (i = 0 ; i < num_pts ; i++)
	  {
	    g3dpoly.vertices[i].sx = pts[i].x * iz + csWorld::shift_x;
	    g3dpoly.vertices[i].sy = pts[i].y * iz + csWorld::shift_y;
	  }
	  CHK (delete [] pts);
          g3dpoly.fog_plane_z = z;
          rview.g3d->AddFogPolygon (GetID (), g3dpoly, CS_FOG_PLANE);
	}
      }

      CHK (delete [] clipper);
#endif
    }
    rview.g3d->CloseFogObject (GetID ());
  }

  //@@@ Map display is application specific!
  csWireFrame* wf = NULL;
  if (csWorld::current_world->map_mode != MAP_OFF)
  {
    wf = csWorld::current_world->wf->GetWireframe ();
    int i;
    for (i = 0 ; i < lights.Length () ; i++)
    {
      csWfVertex* vt = wf->AddVertex (((csStatLight*)lights[i])->GetCenter ());
      vt->SetColor (wf->GetRed ());
    }
  }

  long do_edges;
  rview.g3d->GetRenderState(G3DRENDERSTATE_EDGESENABLE, do_edges);

  if (do_edges)
  {
    extern bool do_coord_check;
//  extern Vector2 coord_check_vector;
    int i;
    csVector3 v;
    float iz;
    int px, py, r;
    ITextureManager* txtmgr;
    rview.g3d->GetTextureManager (&txtmgr);
    int red, white;
    txtmgr->FindRGB (255, 255, 255, white);
    txtmgr->FindRGB (255, 0, 0, red);
    for (i = 0 ; i < lights.Length () ; i++)
    {
      v = rview.Other2This (((csStatLight*)lights[i])->GetCenter ());
      if (v.z > SMALL_Z)
      {
        iz = rview.aspect/v.z;
        px = QInt (v.x * iz + csWorld::shift_x);
        py = csWorld::frame_height - 1 - QInt (v.y * iz + csWorld::shift_y);
	r = QInt (.3 * iz);
	if (do_coord_check)
	{
     // DAN: Commented out this code for now, until we decide
     // where to put light selection. 10.05.98
/*	  if (ABS (coord_check_vector.x - px) < 5 && ABS (coord_check_vector.y - (csWorld::frame_height-1-py)) < 5)
	  {
	    rview.g3d->selected_light = lights[i];
	    CsPrintf (MSG_CONSOLE, "Selected light %s/(%f,%f,%f).\n", 
                      csNameObject::GetName(*this), lights[i]->GetCenter ().x, 
                      lights[i]->GetCenter ().y, lights[i]->GetCenter ().z);
	    CsPrintf (MSG_DEBUG_0, "Selected light %s/(%f,%f,%f).\n", 
                      csNameObject::GetName(*this), lights[i]->GetCenter ().x,
                      lights[i]->GetCenter ().y, lights[i]->GetCenter ().z);
	  }*/
	}
        rview.g2d->DrawLine (px-r, py-r, px+r, py+r, white);
        rview.g2d->DrawLine (px+r, py-r, px-r, py+r, white);
        rview.g2d->DrawLine (px, py-2, px, py+2, red);
        rview.g2d->DrawLine (px+2, py, px-2, py, red);
      }
    }
  }

  RestoreTransformation (old_tr3);
  draw_busy--;
}


struct SectorShineInfo
{
  csStatLight* light;
  csVector3 center;	// The center of the light (possibly warped)
  bool mirror;		// If everything is mirrored.
  csVector3* frustrum;	// A (possibly warped) view frustrum for the light.
  int num_frustrum;
  // These two are only used for dump_frustrum
  csTransform *trans;
  IGraphics3D* g3d;
};

void* csSector::ShinePolygons (csPolygonParentInt*, csPolygonInt** polygon, int num, void* data)
{
  csPolygon3D* p;
  csPortal* po;
  csLightView* d = (csLightView*)data;
  int i;
  for (i = 0 ; i < num ; i++)
  {
    p = (csPolygon3D*)polygon[i];
    csVector3* new_frustrum = NULL;
    int new_num_frustrum = 0;
    if (p->ClipFrustrum (d->center, d->frustrum, d->num_frustrum, d->mirror, &new_frustrum, &new_num_frustrum))
    {
      // First we check if the light can reach the polygon. If not then there is no
      // need to include it in the view frustrum. This is especially useful for portals
      // since then whole sectors can be culled away efficiently.

      // First compute the distance from the center of the light to the plane of
      // the polygon. If this distance is greater then the radius of the light then
      // we have a trivial case (no hit possible).
      float dist_to_plane = p->GetPolyPlane ()->Distance (d->center);

      bool faraway = true;
      if (dist_to_plane < d->l->GetRadius ())
      {
        // The light is close enough to the plane of the polygon. Now we calculate
	// an accurate minimum squared distance from the light to the polygon. Note
	// that we use the new_frustrum which is relative to the center of the light.
	// So this algorithm works with the light position at (0,0,0) (@@@ we should
	// use this to optimize this algorithm further).
	csVector3 o (0, 0, 0);
	float min_sqdist = csSquaredDist::PointPoly (o, new_frustrum, new_num_frustrum,
		*(p->GetPolyPlane ()), dist_to_plane*dist_to_plane);
	if (min_sqdist < d->l->GetSquaredRadius ()) faraway = false;	// Light reaches polygon.
      }

      if (!faraway)
      {
        csLightView new_lview;
	new_lview.CopyNoFrustrum (*d);
        new_lview.frustrum = new_frustrum;
        new_lview.num_frustrum = new_num_frustrum;
        p->ShineLightmaps (new_lview);
        po = p->GetPortal ();
        if (po) po->ShineLightmaps (new_lview);
        else if (!new_lview.dynamic && do_radiosity)
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
  return NULL;
}

void csSector::ShineLightmaps (csLightView& lview)
{
  if (draw_busy >= cfg_reflections) return;
  int i;

  draw_busy++;
  csVector3* old;
  NewTransformation (old);

  TranslateVector (lview.center);

  csLightView new_lview;
  new_lview.Copy (lview);

  // First we slightly expand the frustrum. This is to make sure that
  // boundary cases are also catched correctly.
  if (new_lview.frustrum)
  {
    // First calculate the center point of the frustrum. We will
    // expand around that point.
    csVector3 minf, maxf, cent;
    minf.x = minf.y = minf.z = 1000000;
    maxf.x = maxf.y = maxf.z = -1000000;
    for (i = 0 ; i < lview.num_frustrum ; i++)
    {
      if (lview.frustrum[i].x < minf.x) minf.x = lview.frustrum[i].x;
      if (lview.frustrum[i].y < minf.y) minf.y = lview.frustrum[i].y;
      if (lview.frustrum[i].z < minf.z) minf.z = lview.frustrum[i].z;
      if (lview.frustrum[i].x > maxf.x) maxf.x = lview.frustrum[i].x;
      if (lview.frustrum[i].y > maxf.y) maxf.y = lview.frustrum[i].y;
      if (lview.frustrum[i].z > maxf.z) maxf.z = lview.frustrum[i].z;
    }
    cent = (minf+maxf) / 2.0;

    // Scale the frustrum.
    for (i = 0 ; i < lview.num_frustrum ; i++)
      new_lview.frustrum[i] = (lview.frustrum[i]-cent) * 1.15 + cent;
  }

  if (light_frame_number != current_light_frame_number)
  {
    light_frame_number = current_light_frame_number;
    new_lview.gouroud_color_reset = true;
  }
  else new_lview.gouroud_color_reset = false;

  if (bsp)
    bsp->Back2Front (new_lview.center, &ShinePolygons, (void*)&new_lview);
  else
    ShinePolygons ((csPolygonParentInt*)this, polygons, num_polygon, (void*)&new_lview);

  if (static_thing) static_thing->ShineLightmaps (lview);
  csThing* sp = first_thing;
  while (sp)
  {
    if (!sp->IsMerged ()) sp->ShineLightmaps (lview);
    sp = (csThing*)(sp->GetNext ());
  }

  RestoreTransformation (old);
  draw_busy--;
}

void* csSector::DumpFrustrumPolygons (csPolygonParentInt*, csPolygonInt** polygon, int num, void* data)
{
#if 0
  csPolygon3D* p;
  csPortal* po;
  SectorShineInfo* d = (SectorShineInfo*)data;
  int i;
  ITextureManager* txtmgr;
  d->g3d->GetTextureManager (&txtmgr);
  int red, white;
  txtmgr->FindRGB (255, 255, 255, white);
  txtmgr->FindRGB (255, 0, 0, red);

  for (i = 0 ; i < num ; i++)
  {
    p = (csPolygon3D*)polygon[i];
    csVector3* new_frustrum = NULL;
    int new_num_frustrum = 0;
    if (p->ClipFrustrum (d->light->GetCenter (), d->frustrum, d->num_frustrum, false/*@@@unsupported*/,
    	&new_frustrum, &new_num_frustrum))
    {
      int j;
      csVector3 light_cam;
      csVector3 v0, v1, v2;
      light_cam = d->trans->Other2This (d->light->GetCenter ());

      for (j = 0 ; j < new_num_frustrum ; j++)
      {
        v0 = new_frustrum[j] + d->light->GetCenter ();
        v1 = d->trans->Other2This (v0);
        v0 = new_frustrum[(j+1)%new_num_frustrum] + d->light->GetCenter ();
        v2 = d->trans->Other2This (v0);
        d->g3d->DrawLine (light_cam, v1, csCamera::aspect, red);
        d->g3d->DrawLine (light_cam, v2, csCamera::aspect, red);
        d->g3d->DrawLine (v1, v2, csCamera::aspect, white);
      }

      po = p->GetPortal ();
      if (po)
      {
        po->dump_frustrum (d->light, new_frustrum, new_num_frustrum, *(d->trans));
      }
      if (new_frustrum) CHKB (delete [] new_frustrum);
    }
  }
#endif
  return NULL;
}

void csSector::DumpFrustrum (csStatLight* l, csVector3* frustrum, int num_frustrum,
	csTransform& t)
{
//@@@ Maybe move to application?
#if 0
  if (draw_busy) return;

  draw_busy++;
  csVector3* old = NewTransformation ();

  TranslateVector (l->get_center ());

  SectorShineInfo sh;
  sh.light = l;
  sh.frustrum = frustrum;
  sh.num_frustrum = num_frustrum;
  sh.trans = &t;
  sh.g3d = @@@;

  if (bsp)
    bsp->Back2Front (l->get_center (), &DumpFrustrumPolygons, (void*)&sh);
  else
    DumpFrustrumPolygons ((csPolygonParentInt*)this, polygons, num_polygon, (void*)&sh);

  csThing* sp = first_thing;
  while (sp)
  {
    sp->dump_frustrum (l, frustrum, num_frustrum, t);
    sp = (csThing*)(sp->GetNext ());
  }

  RestoreTransformation (old);
  draw_busy--;
#endif
}

void csSector::InitLightmaps (bool do_cache)
{
  int i;
  for (i = 0 ; i < num_polygon ; i++)
    ((csPolygon3D*)polygons[i])->InitLightmaps (this, do_cache, i);

  csThing* sp = first_thing;
  while (sp)
  {
    sp->InitLightmaps (do_cache);
    sp = (csThing*)(sp->GetNext ());
  }
}

void csSector::CacheLightmaps ()
{
  int i;
  for (i = 0 ; i < num_polygon ; i++)
    ((csPolygon3D*)polygons[i])->CacheLightmaps (this, i);

  csThing* sp = first_thing;
  while (sp)
  {
    if (strcmp (csNameObject::GetName(*sp), "__static__") != 0) 
      sp->CacheLightmaps ();
    sp = (csThing*)(sp->GetNext ());
  }
}

csThing* csSector::GetThing (char* name)
{
  csThing* s = first_thing;
  while (s)
  {
    if (!strcmp (name, csNameObject::GetName(*s))) return s;
    s = (csThing*)(s->GetNext ());
  }
  return NULL;
}

//---------------------------------------------------------------------------
// Everything needed for the precomputation of the lighting.
//---------------------------------------------------------------------------

struct BeamInfo
{
  csVector3 start;
  csVector3 end;
  csPolygon3D* poly;
  float sqdist;
};

void* csSector::BeamPolygons (csPolygonParentInt*, csPolygonInt** polygon, int num, void* data)
{
  BeamInfo* d = (BeamInfo*)data;
  int i;
  void* rc;
  csPortal* po;
  csPolygon3D* p;

  for (i = 0 ; i < num ; i++)
  {
    p = (csPolygon3D*)polygon[i];
    if (p->IntersectRay (d->start, d->end))
    {
      po = p->GetPortal ();
      rc = NULL;
      if (po && p != d->poly)
        return po->FollowBeam (d->start, d->end, d->poly, &d->sqdist);
      else
      {
        csVector3 isect;
	p->GetPlane ()->IntersectSegment (d->start, d->end, isect, NULL);
        d->sqdist = csSquaredDist::PointPoint (isect, d->start);
        return (void*)p;
      }
    }
  }
  return NULL;
}

void* csSector::CheckForThings (csPolygonParentInt*, csPolygonInt** polygon, int num, void* data)
{
  BeamInfo* d = (BeamInfo*)data;
  int i;
  csPortal* po;
  csPolygon3D* p;
  bool block = false;
  csPolygon3D* block_poly;
  bool noblock = false;

  for (i = 0 ; i < num ; i++)
  {
    p = (csPolygon3D*)polygon[i];
    void* rc = NULL;
    bool isc = p->IntersectRay (d->start, d->end);
    if (isc)
    {
      po = p->GetPortal ();
      if (po)
      {
         rc = (void*)po->BlockingThings (d->start, d->end, &(d->poly));
         if (rc) return rc;
         // Set noblock to true to indicate that there is no block (no
         // matter what 'block' says).
         noblock = true;
      }
      else if (do_radiosity)
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
	rc = (void*)mirror.BlockingThings (d->start, d->end, &(d->poly));
        if (rc) return rc;
        // Set noblock to true to indicate that there is no block (no
        // matter what 'block' says).
        noblock = true;
      }
      else
      {
        // There is an intersection but it is not with a portal.
        // Remember this for later. If we find no other portals to go through
        // we will return true that there is something blocking our path.
        block = true;
	block_poly = p;
      }
    }
  }
#if 0
  if (noblock)
    return NULL;
  else if (block)
  {
    d->poly = block_poly;
    return (void*)true;
  }
#endif
  return NULL;
}


csPolygon3D* csSector::FollowBeam (csVector3& start, csVector3& end, csPolygon3D* poly,
				float* sqdist)
{
  BeamInfo beam;
  beam.start = start;
  beam.end = end;
  beam.poly = poly;

  beam_busy++;
  csPolygon3D* p;
  visited++;
  if (bsp) p = (csPolygon3D*)bsp->Front2Back (start, &BeamPolygons, (void*)&beam);
  else p = (csPolygon3D*)BeamPolygons ((csPolygonParentInt*)this, polygons, num_polygon, (void*)&beam);
  if (!p) visited--;
  beam_busy--;

  // Check all the things.
  csPolygon3D* p2;
  float sqdist2;
  csVector3 isect2;
  csThing* sp = first_thing;
  while (sp)
  {
    p2 = sp->IntersectSegment (start, end, isect2);
    if (p2)
    {
      sqdist2 = csSquaredDist::PointPoint (isect2, start);
      if (!p || sqdist2 < beam.sqdist)
      {
        p = p2;
	beam.sqdist = sqdist2;
      }
    }
    sp = (csThing*)(sp->GetNext ());
  }

  if (sqdist) *sqdist = beam.sqdist;
  return p;
}


// hit_beam is called by csPolyTexture::shine with 'start' equal to
// the original world space start of the light and 'end' the corresponding
// world space coordinate of the end of the light beam. This 'end' point
// does not necessarily correspond with the point on the polygon.

bool csSector::HitBeam (csVector3& start, csVector3& end, csPolygon3D* poly, float* sqdist)
{
  // First we set visited to 0 in the sector of the polygon we are
  // looking for. 'follow_beam' will increase visited by 1 for all sectors
  // that the beam passed to get to the specific polygon.
  poly->GetSector ()->visited = 0;

  csPolygon3D* p = FollowBeam (start, end, poly, sqdist);

  if (!p) return false;

  if (p->SamePlane (poly))
  {
    // If we have the same plane then we have a hit.
    return true;
  }

  // exp_sqdist is what we would expect the distance to our polygon
  // to be if we have a hit.
  float exp_sqdist = csSquaredDist::PointPoint (start, end);
  
  if (!poly->GetSector ()->bsp && poly->GetSector () == p->GetSector () &&
  	(csPolygonSet*)(p->GetParent ()) == (csPolygonSet*)(p->GetSector ()))
  {
    // If the polygon that is hit is a member of the same sector as
    // our sector, and if the sector does NOT use a bsp (it is convex),
    // and if the polygon that is hit does not belong to a csThing, then
    // we also consider it a hit. This takes care of misses where the
    // miss is just on an adjacent polygon.
    *sqdist = exp_sqdist;
    return true;
  }

  if (*sqdist > exp_sqdist && poly->GetSector ()->visited)
  {
    // We have missed this polygon and got a hit on some polygon that is
    // further away than this one. Since the original purpose of
    // this routine is to aim a beam of light at the polygon, we can't
    // really miss. So we assume it is a hit (easy, isn't it :-)
    // To make sure this test really works in all cases we also check
    // that the portal of our polygon is at least visited by the
    // beam.
    *sqdist = exp_sqdist;
    return true;
  }

  // The only case that is not fixed is similar to the last case but
  // for sectors that do use a BSP. Here we can't just use the same test.

  return false;
}

bool csSector::BlockingThings (csVector3& start, csVector3& end, csPolygon3D** poly, bool same_sector)
{
  // Check all the things in the current sector.
  csVector3 isect2;
  csPolygon3D* p;
  csThing* sp = first_thing;
  while (sp)
  {
    p = sp->IntersectSegment (start, end, isect2);
    if (p) { if (poly) *poly = p; return true; }
    sp = (csThing*)(sp->GetNext ());
  }

  // If no things are blocking the path then check in the other sectors.
  BeamInfo beam;
  beam.start = start;
  beam.end = end;
  void* rc = NULL;

  beam_busy++;

  // Verify here if the maximum cfg_reflections have been reached to avoid
  // to loose time to find the portal from which the light is coming
  // for nothing.
  if (beam_busy < cfg_reflections)
  {
    if (bsp) rc = bsp->Front2Back (start, &CheckForThings, (void*)&beam);
    else if (!same_sector)
      rc = CheckForThings ((csPolygonParentInt*)this, polygons, num_polygon, (void*)&beam);
  }

  beam_busy--;

  if (rc) { if (poly) *poly = beam.poly; return true; }

  return false;
}


void csSector::ShineLights ()
{
  int i;
  for (i = 0 ; i < lights.Length () ; i++)
  {
    CsPrintf(MSG_TICKER, "");
    ((csStatLight*)lights[i])->ShineLightmaps ();
  }
}

void csSector::ShineLights (csThing* th)
{
  int i;
  for (i = 0 ; i < lights.Length () ; i++)
  {
    CsPrintf(MSG_TICKER, "");
    ((csStatLight*)lights[i])->ShineLightmaps (th);
  }
}

csStatLight* csSector::FindLight (float x, float y, float z, float dist)
{
  int i;
  for (i = 0 ; i < lights.Length () ; i++)
  {
    csStatLight* l = (csStatLight*)lights[i];
    if (ABS (x-l->GetCenter ().x) < SMALL_EPSILON &&
    	ABS (y-l->GetCenter ().y) < SMALL_EPSILON &&
    	ABS (z-l->GetCenter ().z) < SMALL_EPSILON &&
    	ABS (dist-l->GetRadius ()) < SMALL_EPSILON)
      return l;
  }
  return NULL;
}

//---------------------------------------------------------------------------
