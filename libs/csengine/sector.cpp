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
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/dynlight.h"
#include "csengine/light.h"
#include "csengine/camera.h"
#include "csengine/world.h"
#include "csengine/halo.h"
#include "csengine/stats.h"
#include "csengine/csppulse.h"
#include "csengine/cbuffer.h"
#include "csengine/bspbbox.h"
#include "csengine/terrain.h"
#include "csengine/quadcube.h"
#include "csgeom/bsp.h"
#include "csgeom/octree.h"
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

CSOBJTYPE_IMPL (csSector,csPolygonSet);

csSector::csSector () : csPolygonSet ()
{
  first_thing = NULL;
  sector = this;
  beam_busy = 0;
  level_r = level_g = level_b = 0;
  static_tree = NULL;
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
  CHK (delete static_tree);

  // The sprites are not deleted here because they can occur in more
  // than one sector at the same time. Therefor we first clear the list.
  int i;
  for (i = 0 ; i < sprites.Length (); i++) sprites[i] = NULL;
  sprites.DeleteAll ();

  lights.DeleteAll ();

  terrains.DeleteAll ();
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
  thing->SetNext (first_thing);
  first_thing = thing;
}

bool csSector::RemoveThing (csThing* thing)
{
  if (first_thing == thing)
  {
    first_thing = (csThing*)thing->GetNext();
    return true;
  }
  else
  {
    csThing* th = first_thing;
    while (th)
    {
      csThing* next = (csThing*)th->GetNext();
      if (next==thing)
      {
        th->SetNext(next->GetNext());
        return true;
      }
      th = next;
    }
  }
  return false; //Thing was not found
}

void csSector::AddLight (csStatLight* light)
{
  lights.Push (light);
  light->SetSector (this);
}

void csSector::UseStaticTree (int mode, bool octree)
{
  // @@@ Always use octrees now.
  octree = true;

  CHK (delete bsp); bsp = NULL;
  CHK (delete static_tree); static_tree = NULL;

  if (static_thing) return;
  CHK (static_thing = new csThing ());
  static_thing->SetName ("__static__");

  static_thing->SetSector (this);
  csThing* sp = first_thing;
  csThing* sp_prev = NULL;
  while (sp)
  {
    csThing* n = (csThing*)(sp->GetNext ());
    if (!sp->CheckFlags (CS_ENTITY_MOVEABLE) && !sp->GetFog ().enabled)
    {
      static_thing->Merge (sp);
      CHK (delete sp);
      if (sp_prev) sp_prev->SetNext (n);
      else first_thing = n;
    }
    else sp_prev = sp;
    sp = n;
  }
  static_thing->SetNext (first_thing);
  first_thing = static_thing;
  static_thing->CreateBoundingBox ();

  if (octree)
  {
    csVector3 min_bbox, max_bbox;
    static_thing->GetBoundingBox (min_bbox, max_bbox);
    CHK (static_tree = new csOctree (static_thing, min_bbox, max_bbox, 100, mode));
  }
  else
  {
    CHK (static_tree = new csBspTree (static_thing, mode));
  }
  CsPrintf (MSG_INITIALIZATION, "Calculate bsp/octree...\n");
  static_tree->Build ();
  CsPrintf (MSG_INITIALIZATION, "Compress vertices...\n");
  static_thing->CompressVertices ();
  CsPrintf (MSG_INITIALIZATION, "Build vertex tables...\n");
  if (octree) { ((csOctree*)static_tree)->BuildVertexTables (); }
  CsPrintf (MSG_INITIALIZATION, "DONE!\n");
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

void csSector::CreateLightMaps (iGraphics3D* g3d)
{
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolygon3D* p = polygons.Get (i);
    p->CreateLightMaps (g3d);
  }

  csThing* sp = first_thing;
  while (sp)
  {
    sp->CreateLightMaps (g3d);
    sp = (csThing*)(sp->GetNext ());
  }
}

csPolygon3D* csSector::IntersectSegment (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
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

csSector* csSector::FollowSegment (csReversibleTransform& t,
  csVector3& new_position, bool& mirror)
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


csPolygon3D* csSector::IntersectSphere (csVector3& center, float radius,
  float* pr)
{
  float d, min_d = radius;
  int i;
  csPolygon3D* p, * min_p = NULL;
  csPolyPlane* pl;
  csVector3 hit;
  float A, B, C, D;

  for (i = 0 ; i < polygons.Length () ; i++)
  {
    p = polygons.Get (i);
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

void* csSector::DrawPolygons (csPolygonParentInt* pi,
  csPolygonInt** polygon, int num, void* data)
{
  csRenderView* d = (csRenderView*)data;
  csSector* sector = (csSector*)pi;
  sector->DrawPolygonArray (polygon, num, d, false);
  return NULL;
}

csPolygon2DQueue* poly_queue;

void* csSector::TestQueuePolygons (csPolygonParentInt* pi,
  csPolygonInt** polygon, int num, void* data)
{
  csRenderView* d = (csRenderView*)data;
  csSector* sector = (csSector*)pi;
  return sector->TestQueuePolygonArray (polygon, num, d, poly_queue);
}

//@@@ DEBUG
//@@@int max_num_polygons = 1;
//@@@int polygons_still_left = 1;

#if 0
void* csSector::TestQueuePolygonsQuad (csPolygonParentInt* pi,
  csPolygonInt** polygon, int num, void* data)
{
  csRenderView* d = (csRenderView*)data;
  csSector* sector = (csSector*)pi;
//@@@
//@@@if (polygons_still_left <= 0) return NULL;
  return sector->TestQueuePolygonArrayQuad (polygon, num, d, poly_queue);
}
#endif

void csSector::DrawPolygonsFromQueue (csPolygon2DQueue* queue,
  csRenderView* rview)
{
  csPolygon3D* poly3d;
  csPolygon2D* poly2d;
  csPoly2DPool* render_pool = csWorld::current_world->render_pol2d_pool;
  while (queue->Pop (&poly3d, &poly2d))
  {
    poly3d->CamUpdate ();
    poly3d->GetPlane ()->WorldToCamera (*rview, poly3d->Vcam (0));
    DrawOnePolygon (poly3d, poly2d, rview, false);
    render_pool->Free (poly2d);
  }
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
  int i;
  csOctree* otree = (csOctree*)tree;
  csOctreeNode* onode = (csOctreeNode*)node;
  csCBuffer* c_buffer = csWorld::current_world->GetCBuffer ();
  csQuadtree* quadtree = csWorld::current_world->GetQuadtree ();
  csRenderView* rview = (csRenderView*)data;
  csVector3 array[6];
  static csPolygon2D persp;
  int num_array;
  otree->GetConvexOutline (onode, pos, array, num_array);
  if (num_array)
  {
    csVector3 cam[6];
    // If all vertices are behind z plane then the node is
    // not visible. If some vertices are behind z plane then we
    // need to clip the polygon.
    int num_z_0 = 0;
    for (i = 0 ; i < num_array ; i++)
    {
      cam[i] = rview->Other2This (array[i]);
      if (cam[i].z < SMALL_EPSILON) num_z_0++;
    }
    if (num_z_0 == num_array)
    {
      // Node behind camera.
      count_cull_node_notvis_behind++;
      return false;
    }
    persp.MakeEmpty ();
    if (num_z_0 == 0)
    {
      // No vertices are behind. Just perspective correct.
      for (i = 0 ; i < num_array ; i++)
        persp.AddPerspective (cam[i]);
    }
    else
    {
      // Some vertices are behind. We need to clip.
      csVector3 isect;
      int i1 = num_array-1;
      for (i = 0 ; i < num_array ; i++)
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
    if (!persp.ClipAgainst (rview->view)) return false;

    // c-buffer test.
    bool vis;
    if (quadtree) vis = quadtree->TestPolygon (persp.GetVertices (), persp.GetNumVertices (),
    	persp.GetBoundingBox ());
    else vis = c_buffer->TestPolygon (persp.GetVertices (), persp.GetNumVertices ());
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
    csPolygonSet* pset = (csPolygonSet*)(otree->GetParent ());
    cam = pset->GetCameraVertices ();
    indices = onode->GetMiniBspVerts ();
    num_indices = onode->GetMiniBspNumVerts ();
    for (i = 0 ; i < num_indices ; i++)
      cam[indices[i]] = rview->Other2This (pset->Vwor (indices[i]));
  }
  return true;
}

void csSector::Draw (csRenderView& rview)
{
  draw_busy++;
  UpdateTransformation (rview);
  Stats::polygons_considered += polygons.Length ();
  int i;

  G3D_FOGMETHOD fogmethod = G3DFOGMETHOD_NONE;

  if (rview.callback)
  {
    rview.callback (&rview, CALLBACK_SECTOR, (void*)this);
  }
  else if (HasFog ())
  {
    if ((fogmethod = rview.g3d->GetFogMode ()) == G3DFOGMETHOD_VERTEX)
    {
      CHK (csFogInfo* fog_info = new csFogInfo ());
      fog_info->next = rview.fog_info;
      if (rview.portal_polygon)
      {
        fog_info->incoming_plane = rview.portal_polygon->GetPlane ()->
		GetCameraPlane ();
        fog_info->incoming_plane.Invert ();
	fog_info->has_incoming_plane = true;
      }
      else fog_info->has_incoming_plane = false;
      fog_info->fog = &GetFog ();
      fog_info->has_outgoing_plane = true;
      rview.fog_info = fog_info;
      rview.added_fog_info = true;
    }
    else if (fogmethod != G3DFOGMETHOD_NONE)
    {
      rview.g3d->OpenFogObject (GetID (), &GetFog ());
    }
  }

  // In some cases this queue will be filled with all visible
  // sprites.
  csSprite3D** sprite_queue = NULL;
  int num_sprite_queue = 0;

  csCBuffer* c_buffer = csWorld::current_world->GetCBuffer ();
  csQuadtree* quadtree = csWorld::current_world->GetQuadtree ();
  if (c_buffer || quadtree)
  {
    // @@@ We should make a pool for queues. The number of queues allocated
    // at the same time is bounded by the recursion through portals. So a
    // pool would be ideal.
    if (static_thing && do_things)
    {
      // Add all bounding boxes for all sprites to the BSP tree dynamically.
      // @@@ Avoid memory allocation?
      csBspContainer* spr_container = NULL;
      if (sprites.Length () > 0)
      {
        CHK (spr_container = new csBspContainer ());
        for (i = 0 ; i < sprites.Length () ; i++)
        {
          csSprite3D* sp3d = (csSprite3D*)sprites[i];
	  sp3d->MarkInvisible ();
	  sp3d->AddBoundingBox (spr_container);
        }
	static_tree->AddDynamicPolygons (spr_container->GetPolygons (),
          spr_container->GetNumPolygons ());
	spr_container->World2Camera (rview);
      }

      CHK (poly_queue = new csPolygon2DQueue (polygons.Length ()+
      	static_thing->GetNumPolygons ()));
      static_thing->UpdateTransformation ();
      static_tree->Front2Back (rview.GetOrigin (), &TestQueuePolygons,
      	&rview, CullOctreeNode, &rview);

      if (sprites.Length () > 0)
      {
        // Clean up the dynamically added BSP polygons.
	static_tree->RemoveDynamicPolygons ();
        CHK (delete spr_container);

	// Push all visible sprites in a queue.
	// @@@ Avoid memory allocation?
	CHK (sprite_queue = new csSprite3D* [sprites.Length ()]);
	num_sprite_queue = 0;
        for (i = 0 ; i < sprites.Length () ; i++)
        {
          csSprite3D* sp3d = (csSprite3D*)sprites[i];
	  if (sp3d->IsVisible ()) sprite_queue[num_sprite_queue++] = sp3d;
	}
      }
    }
    else
    {
      CHK (poly_queue = new csPolygon2DQueue (polygons.Length ()));
    }
    csPolygon2DQueue* queue = poly_queue;
    TestQueuePolygons (this, polygons.GetArray (), polygons.Length (), &rview);
    DrawPolygonsFromQueue (queue, &rview);
    CHK (delete queue);
  }
  else if (bsp)
    bsp->Back2Front (rview.GetOrigin (), &DrawPolygons, &rview);
  else
  {
    DrawPolygons (this, polygons.GetArray (), polygons.Length (), &rview);
    if (static_thing && do_things)
    {
      static_thing->UpdateTransformation (rview);
      static_tree->Back2Front (rview.GetOrigin (), &DrawPolygons, (void*)&rview);
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
        if (sp != static_thing)
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
        else if (sp->CheckFlags (CS_ENTITY_CONVEX))
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
    csSector* previous_sector = rview.portal_polygon ?
    	rview.portal_polygon->GetSector () : (csSector*)NULL;

    int spr_num;
    if (sprite_queue) spr_num = num_sprite_queue;
    else spr_num = sprites.Length ();

    if (rview.added_fog_info)
      rview.fog_info->has_outgoing_plane = false;

    for (i = 0 ; i < spr_num ; i++)
    {
      csSprite3D* sp3d;
      if (sprite_queue) sp3d = sprite_queue[i];
      else sp3d = (csSprite3D*)sprites[i];

      if (!previous_sector || sp3d->sectors.Find (previous_sector) == -1)
      {
        // Sprite is not in the previous sector or there is no previous sector.
        sp3d->Draw (rview);
      }
      else
      {
        if (
	  ((csPolygonSet*)rview.portal_polygon->GetParent ())->GetType ()
	  	== csThing::Type () ||
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
    CHK (delete [] sprite_queue);
  }

  // Draw all terrain surfaces.
  if (terrains.Length () > 0)
  {
    for (i = 0 ; i < terrains.Length () ; i++)
    {
      csTerrain* terrain = (csTerrain*)terrains[i];
      terrain->Draw (rview, true);
    }
  }

  // queue all halos in this sector to be drawn.
  iHaloRasterizer* HaloRast = csWorld::current_world->GetHaloRastizer ();
  if (!rview.callback && HaloRast)
  {
    int numlights = lights.Length();
    
    for (i = 0; i<numlights; i++)
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
        cshaloinfo->v.x = cshaloinfo->v.x * iz + rview.shift_x;
        cshaloinfo->v.y = csWorld::frame_height - 1 -
		(cshaloinfo->v.y * iz + rview.shift_y);

        cshaloinfo->pLight = light;

        cshaloinfo->r = light->GetColor ().red;
        cshaloinfo->g = light->GetColor ().green;
        cshaloinfo->b = light->GetColor ().blue;
        cshaloinfo->intensity = 0.0f;

        if (HaloRast->TestHalo(&cshaloinfo->v))
        {
          float hi, hc;
          light->GetHaloType (hi, hc);
          cshaloinfo->haloinfo = HaloRast->CreateHalo (cshaloinfo->r,
            cshaloinfo->g, cshaloinfo->b, hi, hc);
          csWorld::current_world->AddHalo (cshaloinfo);
        }
      }
      else
        CHKB (delete cshaloinfo);
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
        g3dpoly.inv_aspect = rview.inv_aspect;
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

  draw_busy--;
}

void* csSector::CalculateLightingPolygons (csPolygonParentInt*,
	csPolygonInt** polygon, int num, void* data)
{
  csPolygon3D* p;
  csLightView* lview = (csLightView*)data;
  csVector3& center = lview->light_frustrum->GetOrigin ();
  int i, j;
  for (i = 0 ; i < num ; i++)
  {
    p = (csPolygon3D*)polygon[i];
    if (p->GetUnsplitPolygon ()) p = (csPolygon3D*)(p->GetUnsplitPolygon ());

    csVector3 poly[50];	// @@@ HARDCODED! BAD!
    for (j = 0 ; j < p->GetNumVertices () ; j++)
      poly[j] = p->Vwor (j)-center;
    if (p->GetPortal ())
    {
      p->CalculateLighting (lview);
    }
    else
    {
      //@@@ ONLY DO THIS WHEN QUADTREE IS USED!!!
      //csWorld::current_world->GetQuadcube ()->InsertPolygon (poly, p->GetNumVertices ());
      p->CalculateLighting (lview);
    }
  }
  return NULL;
}

//@@@ Needs to be part of sector?
//@@@ ONLY DELETE FRUSTRUMS ADDED THIS SECTOR???
void CompressShadowFrustrums (csFrustrumList* list)
{
  csQuadcube* qc = csWorld::current_world->GetQuadcube ();
  qc->MakeEmpty ();
  csShadowFrustrum* sf = list->GetLast ();
int cnt=0,del=0;
  while (sf)
  {
    bool vis = qc->InsertPolygon (sf->GetVertices (), sf->GetNumVertices ());
    if (!vis)
    {
      csShadowFrustrum* sfdel = sf;
      sf = sf->prev;
      list->Unlink (sfdel);
      CHK (delete sfdel);
del++;
    }
    else
      sf = sf->prev;
cnt++;
  }
//printf ("Tested %d frustrums, deleted %d.\n", cnt, del);
}

//@@@ Needs to be part of sector?
void* CalculateLightingPolygonsFB (csPolygonParentInt*,
	csPolygonInt** polygon, int num, void* data)
{
  csPolygon3D* p;
  csLightView* lview = (csLightView*)data;
  csVector3& center = lview->light_frustrum->GetOrigin ();
  csQuadcube* qc = csWorld::current_world->GetQuadcube ();
  bool cw = true;	// @@@ Mirror flag?
  int i, j;
  static int frust_cnt = 50;
  for (i = 0 ; i < num ; i++)
  {
    p = (csPolygon3D*)polygon[i];

    csVector3 poly[50];	// @@@ HARDCODED! BAD!
    for (j = 0 ; j < p->GetNumVertices () ; j++)
      poly[j] = p->Vwor (j)-center;
    bool vis = false;

#define QUADTREE_SHADOW 0
#if QUADTREE_SHADOW
    vis = qc->TestPolygon (poly, p->GetNumVertices ());
#else
    if (p->GetPortal ())
      vis = qc->TestPolygon (poly, p->GetNumVertices ());
    else
      vis = qc->InsertPolygon (poly, p->GetNumVertices ());
#endif
    if (vis)
    {
      p->CalculateLighting (lview);

#if QUADTREE_SHADOW
      if (!p->GetPortal ())
        qc->InsertPolygon (poly, p->GetNumVertices ());
#endif

      //if (p->GetPlane ()->VisibleFromPoint (center) != cw) continue;
      float clas = p->GetPlane ()->GetWorldPlane ().Classify (center);
      if (ABS (clas) < EPSILON) continue;
      if ((clas <= 0) != cw) continue;

      csShadowFrustrum* frust;
      CHK (frust = new csShadowFrustrum (center));
      csPlane pl = p->GetPlane ()->GetWorldPlane ();
      pl.DD += center * pl.norm;
      pl.Invert ();
      frust->SetBackPlane (pl);
      frust->polygon = p;
      for (j = 0 ; j < p->GetVertices ().GetNumVertices () ; j++)
        frust->AddVertex (p->Vwor (j)-center);
      lview->shadows.AddLast (frust);
      frust_cnt--;
      if (frust_cnt < 0) 
      {
        frust_cnt = 200;
	CompressShadowFrustrums (&(lview->shadows));
      }
    }
  }
  return NULL;
}

static int count_cull_dist;
static int count_cull_quad;
static int count_cull_not;

// @@@ This routine need to be cleaned up!!! It needs to
// be part of the class.
bool CullOctreeNodeLighting (csPolygonTree* tree, csPolygonTreeNode* node,
	const csVector3& /*pos*/, void* data)
{
  if (!node) return false;
  if (node->Type () != NODE_OCTREE) return true;

  csOctree* otree = (csOctree*)tree;
  csOctreeNode* onode = (csOctreeNode*)node;
  csLightView* lview = (csLightView*)data;

  const csVector3& center = lview->light_frustrum->GetOrigin ();
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
  float radius = lview->l->GetRadius ();
  if (radius < dist)
  {
    count_cull_dist++;
    return false;
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
    if (!csWorld::current_world->GetQuadcube ()->TestPolygon (outline, num_outline))
    {
      count_cull_quad++;
      return false;
    }
  }
  count_cull_not++;
  return true;
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
  if (!num_things) { return NULL; }
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
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i1)-center, v1, v2) < 0)
	  	continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i2)-center, v1, v2) < 0)
	  	continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i3)-center, v1, v2) < 0)
	  	continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i4)-center, v1, v2) < 0)
	  	continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i5)-center, v1, v2) < 0)
	  	continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i6)-center, v1, v2) < 0)
	  	continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i7)-center, v1, v2) < 0)
	  	continue;
	  if (csMath3::WhichSide3D (sp->Vwor (bbox->i8)-center, v1, v2) < 0)
	  	continue;
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
  csThing* sp;

  // Translate this sector so that it is oriented around
  // the position of the light (position of the light becomes
  // the new origin).
  csVector3& center = lview.light_frustrum->GetOrigin ();

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
      if (sp != static_thing)
      {
        shadows = sp->GetShadows (center);
        lview.shadows.AppendList (shadows);
        CHK (delete shadows);
      }
    }

  // If there is a static tree (BSP and/or octree) then we
  // can use another way to do the lighting. In that case we
  // go front to back and add shadows to the list while we are doing
  // that. In future I would like to add some extra culling stage
  // here using a quad-tree or something similar (for example six
  // quad-trees arranged in a cube around the light).
  if (static_tree)
  {
    count_cull_dist = 0;
    count_cull_quad = 0;
    count_cull_not = 0;
    static_thing->UpdateTransformation (center);
    static_tree->Front2Back (center, &CalculateLightingPolygonsFB, (void*)&lview,
      	CullOctreeNodeLighting, (void*)&lview);
    CalculateLightingPolygonsFB ((csPolygonParentInt*)this, polygons.GetArray (),
      polygons.Length (), (void*)&lview);
    //printf ("Cull: dist=%d quad=%d not=%d\n",
    	//count_cull_dist, count_cull_quad, count_cull_not);
  }
  else
  {
    // Calculate lighting for all polygons in this sector.
    if (bsp)
      bsp->Back2Front (center, &CalculateLightingPolygons, (void*)&lview);
    else
      CalculateLightingPolygons ((csPolygonParentInt*)this, polygons.GetArray (),
        polygons.Length (), (void*)&lview);
  }

  // Calculate lighting for all things in the current sector.
  // @@@ If there is an octree/bsp tree then this should be done
  // differently by adding those things dynamically to the BSP tree.
  for (i = 0 ; i < num_visible_things ; i++)
  {
    sp = visible_things[i];
    if (sp != static_thing)
      sp->CalculateLighting (lview);
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

  draw_busy--;
}

void csSector::InitLightMaps (bool do_cache)
{
  int i;
  for (i = 0; i < polygons.Length (); i++)
    polygons.Get (i)->InitLightMaps (this, do_cache, i);

  csThing* sp = first_thing;
  while (sp)
  {
    sp->InitLightMaps (do_cache);
    sp = (csThing*)(sp->GetNext ());
  }
}

void csSector::CacheLightMaps ()
{
  int i;
  for (i = 0 ; i < polygons.Length (); i++)
    polygons.Get (i)->CacheLightMaps (this, i);

  csThing* sp = first_thing;
  while (sp)
  {
    sp->CacheLightMaps ();
    sp = (csThing*)(sp->GetNext ());
  }
}

csThing* csSector::GetThing (const char* name)
{
  csThing* s = first_thing;
  while (s)
  {
    if (!strcmp (name, s->GetName ()))
      return s;
    s = (csThing*)(s->GetNext ());
  }
  return NULL;
}

void csSector::ShineLights (csProgressPulse* pulse)
{
  for (int i = 0 ; i < lights.Length () ; i++)
  {
    if (pulse != 0)
      pulse->Step();
    ((csStatLight*)lights[i])->CalculateLighting ();
  }
}

void csSector::ShineLights (csThing* th, csProgressPulse* pulse)
{
  for (int i = 0 ; i < lights.Length () ; i++)
  {
    if (pulse != 0)
      pulse->Step();
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
