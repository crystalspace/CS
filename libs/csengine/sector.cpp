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
#include "csengine/dumper.h"
#include "csengine/sector.h"
#include "csengine/thing.h"
#include "csengine/cssprite.h"
#include "csengine/polygon.h"
#include "csengine/polytext.h"
#include "csengine/dynlight.h"
#include "csengine/light.h"
#include "csengine/camera.h"
#include "csengine/world.h"
#include "csengine/halo.h"
#include "csengine/stats.h"
#include "csengine/csppulse.h"
#include "csengine/cbuffer.h"
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
    if (!sp->CheckFlags (CS_ENTITY_MOVEABLE) && !sp->GetFog ().enabled) static_thing->Merge (sp);
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

  G3D_FOGMETHOD fogmethod = G3DFOGMETHOD_NONE;

  if (rview.callback)
  {
    rview.callback (&rview, CALLBACK_SECTOR, (void*)this);
  }
  else if (HasFog ())
  {
    rview.g3d->GetFogMode (fogmethod);
    if (fogmethod == G3DFOGMETHOD_VERTEX)
    {
      CHK (csFogInfo* fog_info = new csFogInfo ());
      fog_info->next = rview.fog_info;
      if (rview.portal_polygon)
      {
        fog_info->incoming_plane = rview.portal_polygon->GetPlane ()->GetCameraPlane ();
        fog_info->incoming_plane.Invert ();
	fog_info->has_incoming_plane = true;
      }
      else fog_info->has_incoming_plane = false;
      fog_info->fog = &GetFog ();
      rview.fog_info = fog_info;
      rview.added_fog_info = true;
    }
    else if (fogmethod != G3DFOGMETHOD_NONE)
    {
      rview.g3d->OpenFogObject (GetID (), &GetFog ());
    }
  }

#if 0
  csCBuffer* c_buffer = csWorld::current_world->GetCBuffer ();
  if (c_buffer)
  {
    csVector3* old_tr3;
    if (static_thing && do_things)
    {
      //static_thing->NewTransformation (old_tr3);
      //static_thing->TransformWorld2Cam (rview);
      //static_bsp->Front2Back (rview.GetOrigin (), &TestPolygons, (void*)&rview);
    }
    //TestDrawPolygons ((csPolygonParentInt*)this, polygons, num_polygon, (void*)&rview);
    if (static_thing && do_things)
    {
      //static_bsp->Back2Front (rview.GetOrigin (), &DrawVisiblePolygons, (void*)&rview);
      //static_thing->RestoreTransformation (old_tr3);
    }
  }
#endif

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
    csThing* sort_list[100];    // @@@HARDCODED == BAD == EASY!
    int sort_idx = 0;
    int i;
    csThing* sp = first_thing;
    if (static_thing)
    {
      // Here we have a static bsp. So we draw all csThings using the
      // Z-buffer and put all foggy csThings in the sort_list.
      while (sp)
      {
        if (!sp->IsMerged () && sp != static_thing)
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
        // @@@ Note from Jorrit: temporarily disabled the option of
	// Z sorting convex objects. The reason is that Z sort is not
	// perfect and we really need something better here. So we
	// only Z sort fog objects.
        // @@@ if (sp->CheckFlags (CS_ENTITY_CONVEX) || sp->GetFog ().enabled)
        if (sp->GetFog ().enabled)
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
        else if (!sp->IsMerged () && sp->CheckFlags (CS_ENTITY_CONVEX))
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
        // @@@ Note from Jorrit: temporarily disabled the option of Z sorting
	// convex objects. (see note above).
        // @@@ if (!sp->CheckFlags (CS_ENTITY_CONVEX) && !sp->GetFog ().enabled) sp->Draw (rview);
        if (!sp->GetFog ().enabled) sp->Draw (rview);
        sp = (csThing*)(sp->GetNext ());
      }
    }

    // Draw sprites.
    // To correctly support sprites in multiple sectors we only draw a
    // sprite if the sprite is not in the sector we came from. If the
    // sprite is also present in the previous sector then we will still
    // draw it in any of the following cases:
    //    - the previous sector has fog
    //    - the portal we just came through has alpha transparency
    //    - the portal is a portal on a thing (i.e. a floating portal)
    //    - the portal does space warping
    // In those cases we draw the sprite anyway. @@@ Note that we should
    // draw it clipped (in 3D) to the portal polygon. This is currently not
    // done.
    csSector* previous_sector = rview.portal_polygon ? rview.portal_polygon->GetSector () : (csSector*)NULL;
    for (i = 0 ; i < sprites.Length () ; i++)
    {
      csSprite3D* sp3d = (csSprite3D*)sprites[i];
      if (!previous_sector || sp3d->sectors.Find (previous_sector) == -1)
      {
        // Sprite is not in the previous sector or there is no previous sector.
        sp3d->Draw (rview);
      }
      else
      {
        if (
	  ((csPolygonSet*)rview.portal_polygon->GetParent ())->GetType () == csThing::Type () ||
	  previous_sector->HasFog () ||
	  rview.portal_polygon->IsTransparent () ||
	  rview.portal_polygon->GetPortal ()->IsSpaceWarped ()
	  )
	{
	  // @@@ Here we should draw clipped to the portal.
          sp3d->Draw (rview);
	}
      }
    }
  }

  // queue all halos in this sector to be drawn.
  IHaloRasterizer* piHR = csWorld::current_world->GetHaloRastizer ();
  if (!rview.callback && piHR)
  {
    int numlights = lights.Length();
    
    for (int i = 0; i<numlights; i++)
    {
      csStatLight* light = (csStatLight*)(lights[i]);
      if (!light->CheckFlags (CS_LIGHT_HALO)) continue;
      
      // this light is already in the queue.
      if (light->GetHaloInQueue ())
        continue;
      
      CHK (csHaloInformation* cshaloinfo = new csHaloInformation);
      
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
        CHK (delete cshaloinfo);
      }
    }
  }

  if (fogmethod != G3DFOGMETHOD_NONE)
  {
    G3DPolygonAFP g3dpoly;
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
    else if (fogmethod == G3DFOGMETHOD_VERTEX && rview.added_fog_info)
    {
      csFogInfo* fog_info = rview.fog_info;
      rview.fog_info = rview.fog_info->next;
      CHK (delete fog_info);
    }
  }

  if (rview.callback) rview.callback (&rview, CALLBACK_SECTOREXIT, (void*)this);

  RestoreTransformation (old_tr3);
  draw_busy--;
}

void* csSector::CalculateLightingPolygons (csPolygonParentInt*, csPolygonInt** polygon, int num, void* data)
{
  csPolygon3D* p;
  csLightView* lview = (csLightView*)data;
  int i;
  for (i = 0 ; i < num ; i++)
  {
    p = (csPolygon3D*)polygon[i];
    p->CalculateLighting (lview);
  }
  return NULL;
}

csThing** csSector::GetVisibleThings (csLightView& lview, int& num_things)
{
  csFrustrum* lf = lview.light_frustrum;
  bool infinite = lf->IsInfinite ();
  csVector3& center = lf->GetOrigin ();
  csThing* sp;
  csPolygonSetBBox* bbox;
  bool vis;
  int i, i1;

  /**
   * First count all things to see how big we should allocate
   * our array.
   */
  num_things = 0;
  sp = first_thing;
  while (sp) { num_things++; sp = (csThing*)(sp->GetNext ()); }
  CHK (csThing** visible_things = new csThing* [num_things]);

  num_things = 0;
  sp = first_thing;
  while (sp)
  {
    // If the light frustrum is infinite then every thing
    // in this sector is of course visible.
    if (infinite) vis = true;
    else
    {
      bbox = sp->GetBoundingBox ();
      if (bbox)
      {
        // If we have a bounding box then we can do a quick test to
	// see if the bounding box is visible in the frustrum. This
	// test is not complete in the sense that it will say that
	// some bounding boxes are visible even if they are not. But
	// it is correct in the sense that if it says a bounding box
	// is invisible, then it certainly is invisible.
	//
	// It works by taking all vertices of the bounding box. If
	// ALL of them are on the outside of the same plane from the
	// frustrum then the object is certainly not visible.
	vis = true;
	i1 = lf->GetNumVertices ()-1;
	for (i = 0 ; i < lf->GetNumVertices () ; i1 = i, i++)
	{
	  csVector3& v1 = lf->GetVertex (i);
	  csVector3& v2 = lf->GetVertex (i1);
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i1)-center, v1, v2) < 0) continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i2)-center, v1, v2) < 0) continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i3)-center, v1, v2) < 0) continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i4)-center, v1, v2) < 0) continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i5)-center, v1, v2) < 0) continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i6)-center, v1, v2) < 0) continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i7)-center, v1, v2) < 0) continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i8)-center, v1, v2) < 0) continue;
	  // Here we have a case of all vertices of the bbox being on the
	  // outside of the same plane.
	  vis = false;
	  break;
	}
	if (vis && lf->GetBackPlane ())
	{
	  // If still visible then we can also check the back plane.
	  // @@@ NOTE THIS IS UNTESTED CODE. LIGHT_FRUSTRUMS CURRENTLY DON'T
	  // HAVE A BACK PLANE YET.
	  if (!csMath3::Visible (sp->Vwor (bbox->i1)-center, *lf->GetBackPlane ()) &&
	      !csMath3::Visible (sp->Vwor (bbox->i2)-center, *lf->GetBackPlane ()) &&
	      !csMath3::Visible (sp->Vwor (bbox->i3)-center, *lf->GetBackPlane ()) &&
	      !csMath3::Visible (sp->Vwor (bbox->i4)-center, *lf->GetBackPlane ()) &&
	      !csMath3::Visible (sp->Vwor (bbox->i5)-center, *lf->GetBackPlane ()) &&
	      !csMath3::Visible (sp->Vwor (bbox->i6)-center, *lf->GetBackPlane ()) &&
	      !csMath3::Visible (sp->Vwor (bbox->i7)-center, *lf->GetBackPlane ()) &&
	      !csMath3::Visible (sp->Vwor (bbox->i8)-center, *lf->GetBackPlane ()))
	    vis = false;
	}
      }
      else
      {
        CsPrintf (MSG_WARNING, "Bounding box for thing not found!\n");
        vis = true;
      }
    }

    if (vis) visible_things[num_things++] = sp;
    sp = (csThing*)(sp->GetNext ());
  }
  return visible_things;
}

void csSector::CalculateLighting (csLightView& lview)
{
  if (draw_busy > cfg_reflections) return;
  draw_busy++;

  int i;
  csVector3* old;
  csThing* sp;

  // Translate this sector so that it is oriented around
  // the position of the light (position of the light becomes
  // the new origin).
  csVector3& center = lview.light_frustrum->GetOrigin ();
  NewTransformation (old);
  TranslateVector (center);

  // Check if gouraud shading needs to be updated.
  if (light_frame_number != current_light_frame_number)
  {
    light_frame_number = current_light_frame_number;
    lview.gouraud_color_reset = true;
  }
  else lview.gouraud_color_reset = false;

  // First mark all things which are visible in the current
  // frustrum.
  int num_visible_things;
  csThing** visible_things = GetVisibleThings (lview, num_visible_things);

  // If we are doing shadow casting for things then we also calculate
  // a list of all shadow frustrums which are going to be used
  // in the lighting calculations. This list is appended to the
  // one given in 'lview'. After returning, the list in 'lview'
  // will be restored.
  csShadowFrustrum* previous_last = lview.shadows.GetLast ();
  csFrustrumList* shadows;
  if (lview.l->GetFlags () & CS_LIGHT_THINGSHADOWS)
    for (i = 0 ; i < num_visible_things ; i++)
    {
      sp = visible_things[i];
      shadows = sp->GetShadows (center);
      lview.shadows.AppendList (shadows);
      CHK (delete shadows);
    }

  // @@@ Here we would like to do an optimization pass on all
  // the generated shadow frustrums. In essence this pass would try to
  // find all shadow frustrums which are:
  //    - outside the current light frustrum and thus are not relevant
  //    - completely inside another shadow frustrum and thus do not
  //      contribute to the shadow
  // Note that we already do something similar when finally mapping
  // the light on a polygon but doing it here would eliminate the
  // frustrums earlier in the process.

  // Calculate lighting for all polygons in this sector.
  if (bsp)
    bsp->Back2Front (center, &CalculateLightingPolygons, (void*)&lview);
  else
    CalculateLightingPolygons ((csPolygonParentInt*)this, polygons, num_polygon, (void*)&lview);

  // Calculate lighting for all things in the current sector.
  if (static_thing) static_thing->CalculateLighting (lview);
  for (i = 0 ; i < num_visible_things ; i++)
  {
    sp = visible_things[i];
    if (!sp->IsMerged ()) sp->CalculateLighting (lview);
  }
  CHK (delete [] visible_things);

  // Restore the shadow list in 'lview' and then delete
  // all the shadow frustrums that were added in this recursion
  // level.
  csShadowFrustrum* frustrum;
  if (previous_last) frustrum = previous_last->next;
  else frustrum = lview.shadows.GetFirst ();
  lview.shadows.SetLast (previous_last);
  while (frustrum)
  {
    csShadowFrustrum* sf = frustrum->next;
    CHK (delete frustrum);
    frustrum = sf;
  }

  // Restore the old transformation.
  RestoreTransformation (old);
  draw_busy--;
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

csThing* csSector::GetThing (const char* name)
{
  csThing* s = first_thing;
  while (s)
  {
    if (!strcmp (name, csNameObject::GetName(*s))) return s;
    s = (csThing*)(s->GetNext ());
  }
  return NULL;
}

void csSector::ShineLights ()
{
  csProgressPulse pulse(true);
  for (int i = 0 ; i < lights.Length () ; i++)
  {
    pulse.Step();
    ((csStatLight*)lights[i])->CalculateLighting ();
  }
}

void csSector::ShineLights (csThing* th)
{
  csProgressPulse pulse(true);
  for (int i = 0 ; i < lights.Length () ; i++)
  {
    pulse.Step();
    ((csStatLight*)lights[i])->CalculateLighting (th);
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
