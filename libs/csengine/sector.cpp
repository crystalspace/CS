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
#include "qint.h"
#include "csutil/csstring.h"
#include "csutil/hashmap.h"
#include "csengine/dumper.h"
#include "csengine/sector.h"
#include "csengine/thing.h"
#include "csengine/meshobj.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/polytmap.h"
#include "csengine/light.h"
#include "csengine/camera.h"
#include "csengine/engine.h"
#include "csengine/stats.h"
#include "csengine/csppulse.h"
#include "csengine/cbuffer.h"
#include "csengine/quadtr3d.h"
#include "csengine/covtree.h"
#include "csengine/bspbbox.h"
#include "csengine/terrain.h"
#include "csengine/covcube.h"
#include "csengine/cbufcube.h"
#include "csengine/bsp.h"
#include "csengine/octree.h"
#include "igraph3d.h"
#include "igraph2d.h"
#include "itxtmgr.h"
#include "itexture.h"
#include "ivfs.h"
#include "istlight.h"

// Option variable: render portals?
bool csSector::do_portals = true;
// Option variable: render things?
bool csSector::do_things = true;
// Configuration variable: number of allowed reflections for static lighting.
int csSector::cfg_reflections = 1;
// Option variable: do pseudo radiosity?
bool csSector::do_radiosity = false;

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csSector,csPolygonSet);

IMPLEMENT_IBASE_EXT (csSector)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSector)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csSector::eiSector)
  IMPLEMENTS_INTERFACE (iSector)
IMPLEMENT_EMBEDDED_IBASE_END

csSector::csSector (csEngine* engine) : csPolygonSet (engine)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiSector);
  beam_busy = 0;
  level_r = level_g = level_b = 0;
  static_tree = NULL;
  static_thing = NULL;
  engine->AddToCurrentRegion (this);
}

csSector::~csSector ()
{
  // Meshes, things, and collections are not deleted by the calls below. They
  // belong to csEngine.
  things.DeleteAll ();
  skies.DeleteAll ();
  meshes.DeleteAll ();
  collections.DeleteAll ();

  delete static_tree;

  lights.DeleteAll ();
  terrains.DeleteAll ();
}

void csSector::Prepare (csSector*)
{
  csPolygonSet::Prepare (this);
  int i;
  for (i = 0 ; i < things.Length () ; i++)
  {
    csThing* th = (csThing*)things[i];
    th->Prepare (this);
  }
  for (i = 0 ; i < skies.Length () ; i++)
  {
    csThing* th = (csThing*)skies[i];
    th->Prepare (this);
  }
}

void csSector::AddLight (csStatLight* light)
{
  lights.Push (light);
  light->SetSector (this);
}

void csSector::UseStaticTree (int mode, bool /*octree*/)
{
  //mode = BSP_BALANCE_AND_SPLITS;

  delete static_tree; static_tree = NULL;

  if (static_thing) return;
  static_thing = new csThing (engine);
  static_thing->SetName ("__static__");

  // First copy the vector of things locally.
  csVector copy_things;
  int i;
  for (i = 0 ; i < things.Length () ; i++)
    copy_things.Push (things[i]);
  
  i = 0;
  while (i < copy_things.Length ())
  {
    csThing* sp = (csThing*)copy_things[i];
    if (!sp->flags.Check (CS_ENTITY_MOVEABLE | CS_ENTITY_DETAIL) && !sp->GetFog ().enabled
    	&& sp->GetNumCurves () == 0)
    {
      static_thing->Merge (sp);
      delete sp;
    }
    i++;
  }
  static_thing->GetMovable ().SetSector (this);
  static_thing->GetMovable ().UpdateMove ();
  static_thing->CreateBoundingBox ();
  engine->things.Push (static_thing);

  csBox3 bbox;
  static_thing->GetBoundingBox (bbox);
  static_tree = new csOctree (this, bbox.Min (), bbox.Max (), 150/*15*/, mode);

  csString str ("vis/octree_");
  str += GetName ();
  csEngine* w = engine;
  bool recalc_octree = true;
  if (!csEngine::do_force_revis && w->VFS->Exists ((const char*)str))
  {
    recalc_octree = false;
    CsPrintf (MSG_INITIALIZATION, "Loading bsp/octree...\n");
    recalc_octree = !((csOctree*)static_tree)->ReadFromCache (
    	w->VFS, (const char*)str, static_thing->GetPolygonArray ().GetArray (),
	static_thing->GetPolygonArray ().Length ());
    if (recalc_octree)
    {
      delete static_tree;
      static_tree = new csOctree (this, bbox.Min (), bbox.Max (), 150/*15*/, mode);
    }
  }
  if (recalc_octree)
  {
    CsPrintf (MSG_INITIALIZATION, "Calculate bsp/octree...\n");
    static_tree->Build (static_thing->GetPolygonArray ());
    CsPrintf (MSG_INITIALIZATION, "Caching bsp/octree...\n");
    ((csOctree*)static_tree)->Cache (w->VFS, (const char*)str);
  }
  CsPrintf (MSG_INITIALIZATION, "Compress vertices...\n");
  static_thing->CompressVertices ();
  CsPrintf (MSG_INITIALIZATION, "Build vertex tables...\n");
  ((csOctree*)static_tree)->BuildVertexTables ();

  // Everything for PVS.
  str = "vis/pvs_";
  str += GetName ();
  bool recalc_pvs = true;
  if ((!csEngine::do_force_revis || csEngine::do_not_force_revis) &&
  	w->VFS->Exists ((const char*)str))
  {
    recalc_pvs = false;
    CsPrintf (MSG_INITIALIZATION, "Loading PVS...\n");
    recalc_pvs = !((csOctree*)static_tree)->ReadFromCachePVS (
    	w->VFS, (const char*)str);;
  }
  if (csEngine::do_not_force_revis) recalc_pvs = false;
  if (recalc_pvs)
  {
#   if 0
    CsPrintf (MSG_INITIALIZATION, "Build PVS...\n");
    ((csOctree*)static_tree)->BuildPVS (static_thing);
#   else
    //CsPrintf (MSG_INITIALIZATION, "Build Dummy PVS...\n");
    //((csOctree*)static_tree)->SetupDummyPVS ();
#   endif
    CsPrintf (MSG_INITIALIZATION, "Caching PVS...\n");
    ((csOctree*)static_tree)->CachePVS (w->VFS, (const char*)str);
  }
  static_tree->Statistics ();

  // Loop through all things and update their bounding box in the
  // polygon trees.
  for (i = 0 ; i < things.Length () ; i++)
  {
    csThing* th = (csThing*)things[i];
    th->GetMovable ().UpdateMove ();
  }
  
  CsPrintf (MSG_INITIALIZATION, "DONE!\n");
}

csPolygon3D* csSector::HitBeam (const csVector3& start, const csVector3& end,
	csVector3& isect)
{
  csPolygon3D* p = IntersectSegment (start, end, isect);
  if (p)
  {
    csPortal* po = p->GetPortal ();
    if (po)
    {
      draw_busy++;
      csVector3 new_start = isect;
      p = po->HitBeam (new_start, end, isect);
      draw_busy--;
      return p;
    }
    else return p;
  }
  else return NULL;
}

csObject* csSector::HitBeam (const csVector3& start, const csVector3& end,
	csPolygon3D** polygonPtr)
{
  float r, best_mesh_r = 10000000000.;
  csMeshWrapper* near_mesh = NULL;
  csVector3 isect;

  // First check all meshes in this sector.
  int i;
  for (i = 0 ; i < meshes.Length () ; i++)
  {
    csMeshWrapper* mesh = (csMeshWrapper*)meshes[i];
    if (mesh->HitBeam (start, end, isect, &r))
    {
      if (r < best_mesh_r)
      {
        best_mesh_r = r;
	near_mesh = mesh;
      }
    }
  }

  float best_poly_r;
  csPolygon3D* p = IntersectSegment (start, end, isect, &best_poly_r);
  // We hit a polygon and the polygon is closer than the mesh.
  if (p && best_poly_r < best_mesh_r)
  {
    csPortal* po = p->GetPortal ();
    if (po)
    {
      draw_busy++;
      csVector3 new_start = isect;
      csObject* obj = po->HitBeam (new_start, end, polygonPtr);
      draw_busy--;
      return obj;
    }
    else
    {
      if (polygonPtr) *polygonPtr = p;
      return (csObject*)(p->GetParent ());
    }
  }
  // The mesh is closer (or there is no mesh).
  if (polygonPtr) *polygonPtr = NULL;
  return (csObject*)near_mesh;
}

void csSector::CreateLightMaps (iGraphics3D* g3d)
{
  int i;
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    csPolygon3D* p = polygons.Get (i);
    p->CreateLightMaps (g3d);
  }

  for (i = 0 ; i < things.Length () ; i++)
  {
    csThing* sp = (csThing*)things[i];
    sp->CreateLightMaps (g3d);
  }
  for (i = 0 ; i < skies.Length () ; i++)
  {
    csThing* sp = (csThing*)skies[i];
    sp->CreateLightMaps (g3d);
  }
}


struct ISectData
{
  csSegment3 seg;
  csVector3 isect;
  float r;
};

/*
 * @@@
 * This function does not yet do anything but it should
 * use the PVS as soon as we have that to make csSector::IntersectSegment
 * even faster.
 */
bool IntersectSegmentCull (csPolygonTree* /*tree*/,
	csPolygonTreeNode* node,
	const csVector3& /*pos*/, void* data)
{
  if (!node) return false;
  if (node->Type () != NODE_OCTREE) return true;

  ISectData* idata = (ISectData*)data;
  csOctreeNode* onode = (csOctreeNode*)node;
  csVector3 isect;
  if (csIntersect3::BoxSegment (onode->GetBox (), idata->seg, isect))
    return true;
  // Segment does not intersect with node so we return false here.
  return false;
}

void* IntersectSegmentTestPol (csSector*,
	csPolygonInt** polygon, int num, bool /*same_plane*/, void* data)
{
  ISectData* idata = (ISectData*)data;
  int i;
  for (i = 0 ; i < num ; i++)
  {
    // @@@ What about other types of polygons?
    if (polygon[i]->GetType () == 1)
    {
      csPolygon3D* p = (csPolygon3D*)polygon[i];
      if (p->IntersectSegment (idata->seg.Start (), idata->seg.End (),
      		idata->isect, &(idata->r)))
        return (void*)p;
    }
  }
  return NULL;
}

csPolygon3D* csSector::IntersectSegment (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
{
  float r, best_r = 10000000000.;
  csVector3 cur_isect;
  csPolygon3D* best_p = NULL;

  int i;
  for (i = 0 ; i < things.Length () ; i++)
  {
    csThing* sp = (csThing*)things[i];
    if (sp != static_thing)
    {
      r = best_r;
      csPolygon3D* p = sp->IntersectSegment (start, end, cur_isect, &r);
      if (p && r < best_r)
      {
        best_r = r;
	best_p = p;
	isect = cur_isect;
      }
    }
  }

  if (static_tree)
  {
    // Handle the octree.
    ISectData idata;
    idata.seg.Set (start, end);
    void* rc = static_tree->Front2Back (start, IntersectSegmentTestPol,
      (void*)&idata, IntersectSegmentCull, (void*)&idata);
    if (rc && idata.r < best_r)
    {
      best_r = idata.r;
      best_p = (csPolygon3D*)rc;
      isect = idata.isect;
    }
  }

  if (best_p)
  {
    if (pr) *pr = best_r;
    return best_p;
  }

  return csPolygonSet::IntersectSegment (start, end, isect, pr);
}

csSector* csSector::FollowSegment (csReversibleTransform& t,
  csVector3& new_position, bool& mirror)
{
  csVector3 isect;
  csPolygon3D* p = IntersectSegment (t.GetOrigin (), new_position, isect);
  csPortal* po;

  if (p)
  {
    po = p->GetPortal ();
    if (po)
    {
      if (!po->GetSector ()) po->CompleteSector ();
      if (po->flags.Check (CS_PORTAL_WARP))
      {
        po->WarpSpace (t, mirror);
	new_position = po->Warp (new_position);
      }
      csSector* dest_sect = po->GetSector ();
      return dest_sect ? dest_sect->FollowSegment (t, new_position, mirror) : (csSector*)NULL;
    }
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
  csVector3 hit;

  for (i = 0 ; i < polygons.Length () ; i++)
  {
    p = polygons.Get (i);
    const csPolyPlane* pl = p->GetPlane ();
    const csPlane3& wpl = pl->GetWorldPlane ();
    d = wpl.Distance (center);
    if (d < min_d && csMath3::Visible (center, wpl))
    {
      hit = -center;
      hit -= wpl.GetNormal ();
      hit *= d;
      hit += center;
      if (p->IntersectRay (center, hit))
      {
        csPortal* po = p->GetPortal ();
        if (po)
        {
	  if (!po->GetSector ()) po->CompleteSector ();
	  csSector* dest_sect = po->GetSector ();
          p = dest_sect->IntersectSphere (center, min_d, &d);
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

  for (i = 0 ; i < things.Length () ; i++)
  {
    csThing* sp = (csThing*)things[i];
    p = sp->IntersectSphere (center, radius, &d);
    if (p && d < min_d)
    {
      min_d = d;
      min_p = p;
    }
  }

  if (pr) *pr = min_d;
  return min_p;
}

void* csSector::DrawPolygons (csSector* sector,
  csPolygonInt** polygon, int num, bool /*same_plane*/, void* data)
{
  csRenderView* d = (csRenderView*)data;
  sector->DrawPolygonArray (polygon, num, d, false);
  return NULL;
}

csPolygon2DQueue* poly_queue;

void* csSector::TestQueuePolygons (csSector* sector,
  csPolygonInt** polygon, int num, bool /*same_plane*/, void* data)
{
  csRenderView* d = (csRenderView*)data;
  return sector->TestQueuePolygonArray (polygon, num, d, poly_queue,
    d->GetEngine ()->IsPVS ());
}

void csSector::DrawPolygonsFromQueue (csPolygon2DQueue* queue,
  csRenderView* rview)
{
  csPolygon3D* poly3d;
  csPolygon2D* poly2d;
  csPoly2DPool* render_pool = rview->GetEngine ()->render_pol2d_pool;
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

  csCBuffer* c_buffer;
  csQuadTree3D* quad3d;
  csCoverageMaskTree* covtree;
  csRenderView* rview = (csRenderView*)data;
  static csPolygon2D persp;
  csVector3 array[7];
  csEngine* w = rview->GetEngine ();

  if (w->IsPVS ())
  {
    // Test for PVS.
    if (!onode->IsVisible ()) return false;
    else if (w->IsPVSOnly ()) goto is_vis;
  }

  c_buffer = w->GetCBuffer ();
  covtree = w->GetCovtree ();
  quad3d = w->GetQuad3D ();
  int num_array;
  otree->GetConvexOutline (onode, pos, array, num_array, rview->UseFarPlane ());

  if (num_array)
  {
    int nVert = MIN (6, num_array); // if we use a farplane we could have up to 7 corners, 
                                    // but the 7th we'll need not here
    
    if (quad3d)
    {
      csVector3 test_poly[6];
      
      for (i = 0 ; i < nVert ; i++)
        test_poly[i] = array[i]-quad3d->GetCenter ();
      csBox3 bbox;
      int rc = quad3d->TestPolygon (test_poly, nVert, bbox);
      bool visible = (rc != CS_QUAD3D_NOCHANGE);
      if (!visible) return false;
      goto is_vis;
    }
  
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
      cam[i] = rview->Other2This (array[i]);
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
    
    if (rview->UseFarPlane ())
    {
      if (num_array == 7) // we havent transformed the 7th yet
       cam[6] = rview->Other2This (array[6]);
      for (i = 0 ; i < num_array ; i++)
        if (rview->GetFarPlane ()->Classify (cam[i]) > SMALL_EPSILON) 
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

    if (!persp.ClipAgainst (rview->GetView ())) return false;

    // c-buffer test.
    bool vis;
    if (covtree)
      vis = covtree->TestPolygon (persp.GetVertices (),
	persp.GetNumVertices (), persp.GetBoundingBox ());
    else
      vis = c_buffer->TestPolygon (persp.GetVertices (),
      	persp.GetNumVertices ());
    if (!vis)
    {
      count_cull_node_notvis_cbuffer++;
      return false;
    }
  }

is_vis:
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
    csPolygonSet* pset = (csPolygonSet*)(otree->GetSector ()->GetStaticThing ());
    cam = pset->GetCameraVertices ();
    indices = onode->GetMiniBspVerts ();
    num_indices = onode->GetMiniBspNumVerts ();
    for (i = 0 ; i < num_indices ; i++)
      cam[indices[i]] = rview->Other2This (pset->Vwor (indices[i]));
  }

  return true;
}

// Some notes about drawing here. These notes are the start for
// a rethinking about how rendering objects in one sector actually
// should happen. Note that the current implementation actually
// implements very little of the things discussed here. Currently
// the entities are just rendered one after the other which can cause
// some problems.
//
// There are a few issues here:
//
// 1. Z-buffering/Z-filling.
// Some objects/entities are more efficiently rendered back
// to front using Z-filling instead of Z-buffering. In some cases
// Z-filling is also required because rendering a sector starts
// with an uninitialized Z-buffer (CS normally doesn't clear the
// Z buffer every frame). In some cases it might be more optimal
// to use Z buffering in any case (to avoid sorting back to front)
// (for hardware 3D) so we would like to have the option to clear
// the Z buffer every frame and use Z-buffering.
//
// 2. Alpha transparency.
// Some entities have alpha transparency. Alpha transparent surfaces
// actually need to be sorted back to front to render correctly.
// Also before rendering an alpha surface all objects behind it should
// already be rendered.
//
// 3. Floating portals.
// Floating portals also take some special consideration. First
// of all the assume a new intialize of the Z buffer for the 2D
// area of the portal in question. This is ok if the first entities
// that are rendered through the portal use Z-fill and cover the
// entire portal (this is the case if you use sector walls for
// example). If Z-fill cannot be used for the portal then an
// extra initial pass would have to clear the Z buffer for the portal
// area in 2D. Also geometry needs to be clipped in 3D if you have
// a floating portal. The reason is that the Z buffer information
// outside of the floating portal may actually contain information
// further than the contents of the portal. This would cause entities
// visible inside the portal to be rendered as if they are in the
// parent sector too.
// After rendering through a floating portal, the floating portal
// itself needs to be covered by the Z-buffer. i.e. we need to make
// sure that the Z-buffer thinks the portal is a regular polygon.
// This is to make sure that meshes or other entities rendered
// afterwards will not get rendered INSIDE the portal contents.
//
// Here is a list of all the entities that we can draw in a sector:
//
// 1. Sector walls.
// Sectors are always convex. So sectors walls are ideal for rendering
// first through Z-fill.
//
// 2. Static things in octree.
// In some cases all static things are collected into one big
// octree with mini-bsp trees. This structure ensures that we can
// actually easily sort polygon back to front or front to back if
// needed. This structure can also easily be rendered using Z-fill.
// The c-buffer/coverage mask tree can also be used to detect
// visibility before rendering. This pushes visible polygons into
// a queue. There is the issue here that it should be possible
// to ignore the mini-bsp trees and only use the octree information.
// This can be done on hardware where Z-buffering is fast. This
// of course implies either the use of a Z-filled sector or else
// a clear of the Z buffer every frame.
// A related issue is when there are portals between the polygons.
// Those portals need to be handled as floating portals (i.e. geometry
// needs to be clipped in 3D) because the Z buffer information
// will not be correct. If rendering the visible octree polygons
// back to front then rendering through the portals presents no
// other difficulties.
//
// 3. Terrain triangles.
// The terrain engine generates a set of triangles. These triangles
// can easily be sorted back to front so they are also suitable for
// Z-fill rendering. However, this conflicts with the use of the
// static octree. You cannot use Z-fill for both because that could
// cause wrong rendering. Using Z-buffer for one of them might be
// expensive but the only solution. Here there is also the issue
// if it isn't possible to combine visibility algorithms for landscape
// and octree stuff. i.e. cull octree nodes if occluded by a part
// of the landscape.
//
// 4. 3D Sprites.
// Sprites are entities that need to be rendered using the Z-buffer
// because the triangles cannot easily be sorted.
//
// 5. Dynamic things.
// Things that are not part of the static octree are handled much
// like normal 3D sprites. The most important exception is when
// such a thing has a floating portal. In this case all the normal
// floating portal issues are valid. However, there are is an important
// issue here: if you are rendering a floating portal that is BEHIND
// an already rendered entity then there is a problem. The contents
// of the portal may actually use Z-fill and thus would overrender
// the entity in front. One obvious solution is to sort ALL entities
// to make sure that everything is rendered back to front. That's of
// course not always efficient and easy to do. Also it is not possible
// in all cases to do it 100% correct (i.e. complex sprites with
// skeletal animation and so on). The ideal solution would be to have
// a way to clear the Z-buffer for an invisible polygon but only
// where the polygon itself is visible according to the old Z-buffer
// values. This is possible with software but I'm currently unsure
// about hardware. With such a routine you could draw the floating
// portal at any time you want. First you clear the Z-buffer for the
// visible area. Then you force Z-buffer use for the contents inside
// (i.e. everything normally rendered using Z-fill will use Z-buffer
// instead), then you render. Finally you update the Z-buffer with
// the Z-value of the polygon to make it 'hard'.
//
// If we can treat floating portals this way then we can in fact
// consider them as normal polygons that behave correctly for the
// Z buffer. Aside from the fact that they clip geometry in 3D
// that passes through the portal. Note that 3D sprites don't
// currently support 3D geometry clipping yet.

void csSector::Draw (csRenderView& rview)
{
  draw_busy++;
  UpdateTransformation (rview);
  Stats::polygons_considered += polygons.Length ();
  int i;
  rview.SetThisSector (this);

  G3D_FOGMETHOD fogmethod = G3DFOGMETHOD_NONE;

  if (rview.GetCallback ())
  {
    rview.CallCallback (CALLBACK_SECTOR, (void*)this);
  }
  else if (HasFog ())
  {
    if ((fogmethod = rview.GetEngine ()->fogmethod) == G3DFOGMETHOD_VERTEX)
    {
      csFogInfo* fog_info = new csFogInfo ();
      fog_info->next = rview.GetFogInfo ();
      if (rview.GetPortalPolygon ())
      {
        fog_info->incoming_plane = rview.GetPortalPolygon ()->GetPlane ()->
		GetCameraPlane ();
        fog_info->incoming_plane.Invert ();
	fog_info->has_incoming_plane = true;
      }
      else fog_info->has_incoming_plane = false;
      fog_info->fog = &GetFog ();
      fog_info->has_outgoing_plane = true;
      rview.SetFogInfo (fog_info);
    }
    else if (fogmethod != G3DFOGMETHOD_NONE)
    {
      rview.GetG3D ()->OpenFogObject (GetID (), &GetFog ());
    }
  }

  // First draw all 'sky' things using Z-fill.
  for (i = 0 ; i < skies.Length () ; i++)
  {
    csThing* th = (csThing*)skies[i];
    th->Draw (rview, false);
  }

  // In some cases this queue will be filled with all visible
  // meshes.
  csMeshWrapper** mesh_queue = NULL;
  int num_mesh_queue = 0;
  // For things we have a similar queue.
  csThing** thing_queue = NULL;
  int num_thing_queue = 0;
  // If the following flag is true the queues are actually used.
  bool use_object_queues = false;

  int engine_mode = rview.GetEngine ()->GetEngineMode ();
  if (engine_mode == CS_ENGINE_FRONT2BACK)
  {
    //-----
    // In this part of the rendering we use the c-buffer or another
    // 2D/3D visibility culler.
    //-----

    // @@@ We should make a pool for queues. The number of queues allocated
    // at the same time is bounded by the recursion through portals. So a
    // pool would be ideal.
    if (static_thing)
    {
      //-----
      // This sector has a static polygon tree (octree).
      //-----
    
      // Mark all meshes as invisible and clear the camera transformation
      // for their bounding boxes.
      if (meshes.Length () > 0)
        for (i = 0 ; i < meshes.Length () ; i++)
        {
          csMeshWrapper* sp = (csMeshWrapper*)meshes[i];
	  csPolyTreeObject* pt = sp->GetPolyTreeObject ();
	  if (pt->GetWorldBoundingBox ().In (rview.GetOrigin ()))
	    sp->MarkVisible ();
	  else
	    sp->MarkInvisible ();
	  sp->VisTestReset ();
        }
      // Similarly mark all things as invisible and clear the camera
      // transformation for their bounding boxes.
      for (i = 0 ; i < things.Length () ; i++)
      {
        csThing* th = (csThing*)things[i];
	csPolyTreeObject* pt = th->GetPolyTreeObject ();
	if (pt->GetWorldBoundingBox ().In (rview.GetOrigin ()))
	  th->MarkVisible ();
	else
	  th->MarkInvisible ();
	th->VisTestReset ();
      }

      // Using the PVS, mark all sectors and polygons that are visible
      // from the current node.
      if (rview.GetEngine ()->IsPVS ())
      {
        csOctree* otree = (csOctree*)static_tree;
	if (rview.GetEngine ()->IsPVSFrozen ())
	  otree->MarkVisibleFromPVS (rview.GetEngine ()->GetFrozenPosition ());
	else
	  otree->MarkVisibleFromPVS (rview.GetOrigin ());
      }

      // Initialize a queue on which all visible polygons will be pushed.
      // The octree is traversed front to back but we want to render
      // back to front. That's one of the reasons for this queue.
      poly_queue = new csPolygon2DQueue (polygons.Length ()+
      	static_thing->GetNumPolygons ());

      // Update the transformation for the static tree. This will
      // not actually transform all vertices from world to camera but
      // it will make sure that when a node (octree node) is visited,
      // the transformation will happen at that time.
      static_thing->UpdateTransformation ();

      // Traverse the tree front to back and push all visible polygons
      // on the queue. This traversal will also mark all visible
      // meshes and things. They will be put on a queue later.
      static_tree->Front2Back (rview.GetOrigin (), &TestQueuePolygons,
      	&rview, CullOctreeNode, &rview);

      // Fill the mesh and thing queues for all meshes and things
      // that were visible.
      use_object_queues = true;
      if (meshes.Length () > 0)
      {
	// Push all visible meshes in a queue.
	// @@@ Avoid memory allocation?
	mesh_queue = new csMeshWrapper* [meshes.Length ()];
	num_mesh_queue = 0;
        for (i = 0 ; i < meshes.Length () ; i++)
        {
          csMeshWrapper* sp = (csMeshWrapper*)meshes[i];
	  if (sp->IsVisible ()) mesh_queue[num_mesh_queue++] = sp;
	}
      }
      if (things.Length () > 0)
      {
        // Push all visible things in a queue.
	// @@@ Avoid memory allocation?
	thing_queue = new csThing* [things.Length ()];
	num_thing_queue = 0;
	for (i = 0 ; i < things.Length () ; i++)
	{
	  csThing* th = (csThing*)things[i];
	  if (th->IsVisible ()) thing_queue[num_thing_queue++] = th;
	}
      }
    }
    else
    {
      // There is no static thing (i.e. octree) in this sector so
      // we just make room for the polygons from the sector.
      poly_queue = new csPolygon2DQueue (polygons.Length ());
    }

    // Also add/queue the polygons of the current sector (which are expected
    // to be behind all other polygons from entities inside the sector).
    csPolygon2DQueue* queue = poly_queue;
    TestQueuePolygonArray (polygons.GetArray (), polygons.Length (), &rview,
    	queue, false);

    // Render all polygons that are visible back to front.
    DrawPolygonsFromQueue (queue, &rview);
    delete queue;
  }
  else if (engine_mode == CS_ENGINE_BACK2FRONT)
  {
    //-----
    // Here we don't use the c-buffer or 2D culler but just render back
    // to front.
    //-----
    DrawPolygons (this, polygons.GetArray (), polygons.Length (), false, &rview);
    if (static_thing)
    {
      static_thing->UpdateTransformation (rview);
      static_tree->Back2Front (rview.GetOrigin (), &DrawPolygons, (void*)&rview);
    }
  }
  else
  {
    //-----
    // Here we render using the Z-buffer.
    //-----
    DrawPolygonArray (polygons.GetArray (), polygons.Length (), &rview, true);
    if (static_thing)
    {
      static_thing->UpdateTransformation (rview);
      csOctree* otree = (csOctree*)static_tree;
      csPolygonIntArray& unsplit = otree->GetRoot ()->GetUnsplitPolygons (); 
      DrawPolygonArray (unsplit.GetPolygons (), unsplit.GetNumPolygons (),
    	  &rview, true);
    }
  }

  if (do_things)
  {
    // If the queues are not used for things we still fill the queue here
    // just to make the code below easier.
    if (!use_object_queues)
    {
      num_thing_queue = 0;
      if (things.Length ())
      {
        thing_queue = new csThing* [things.Length ()];
        for (i = 0 ; i < things.Length () ; i++)
        {
          csThing* th = (csThing*)things[i];
          thing_queue[num_thing_queue++] = th;
        }
      }
      else
        thing_queue = NULL;
    }

    // All csThings which are not merged with the static bsp still need to
    // be drawn. Unless they are fog objects (or transparent, this is a todo!)
    // we just render them using the Z-buffer. Fog or transparent objects
    // are z-sorted and rendered back to front.
    //
    // We should see if there are better alternatives to Z-sort which are
    // more accurate in more cases (@@@).
    csThing* sort_list[256];    // @@@HARDCODED == BAD == EASY!
    int sort_idx = 0;
    int i;

    // First we do z-sorting for fog objects so that they are rendered
    // correctly from back to front. All other objects are drawn using
    // the z-buffer.
    for (i = 0 ; i < num_thing_queue ; i++)
    {
      csThing* th = thing_queue[i];
      if (th != static_thing)
        if (th->GetFog ().enabled) sort_list[sort_idx++] = th;
        else th->Draw (rview);
    }

    if (sort_idx)
    {
      // Now sort the objects in sort_list.
      qsort (sort_list, sort_idx, sizeof (csThing*), compare_z_thing);

      // Draw them back to front.
      for (i = 0 ; i < sort_idx ; i++)
      {
        csThing* th = sort_list[i];
        if (th->GetFog ().enabled) th->DrawFoggy (rview);
        else th->Draw (rview, false);
      }
    }

    delete [] thing_queue;

    // Draw meshes.
    // To correctly support meshes in multiple sectors we only draw a
    // mesh if the mesh is not in the sector we came from. If the
    // mesh is also present in the previous sector then we will still
    // draw it in any of the following cases:
    //    - the previous sector has fog
    //    - the portal we just came through has alpha transparency
    //    - the portal is a portal on a thing (i.e. a floating portal)
    //    - the portal does space warping
    // In those cases we draw the mesh anyway. @@@ Note that we should
    // draw it clipped (in 3D) to the portal polygon. This is currently not
    // done.
    csSector* previous_sector = rview.GetPreviousSector ();

    int spr_num;
    if (mesh_queue) spr_num = num_mesh_queue;
    else spr_num = meshes.Length ();

    if (rview.AddedFogInfo ())
      rview.GetFogInfo ()->has_outgoing_plane = false;

    for (i = 0 ; i < spr_num ; i++)
    {
      csMeshWrapper* sp;
      if (mesh_queue) sp = mesh_queue[i];
      else sp = (csMeshWrapper*)meshes[i];

      if (!previous_sector || sp->GetMovable ().GetSectors ().Find (previous_sector) == -1)
      {
        // Mesh is not in the previous sector or there is no previous sector.
        sp->Draw (rview);
      }
      else
      {
        if (
	  ((csPolygonSet*)rview.GetPortalPolygon ()->GetParent ())->GetType ()
	  	== csThing::Type ||
	  previous_sector->HasFog () ||
	  rview.GetPortalPolygon ()->IsTransparent () ||
	  rview.GetPortalPolygon ()->GetPortal ()->flags.Check (CS_PORTAL_WARP))
	{
	  // @@@ Here we should draw clipped to the portal.
          sp->Draw (rview);
	}
      }
    }
    delete [] mesh_queue;
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
  if (!rview.GetCallback ())
    for (i = lights.Length () - 1; i >= 0; i--)
      // Tell the engine to try to add this light into the halo queue
      rview.GetEngine ()->AddHalo ((csLight *)lights.Get (i));

  // Handle the fog, if any
  if (fogmethod != G3DFOGMETHOD_NONE)
  {
    G3DPolygonDFP g3dpoly;
    if (fogmethod == G3DFOGMETHOD_ZBUFFER)
    {
      g3dpoly.num = rview.GetView ()->GetNumVertices ();
      csVector2 *clipview = rview.GetView ()->GetClipPoly ();
      memcpy (g3dpoly.vertices, clipview, g3dpoly.num * sizeof (csVector2));
      if (rview.GetSector () == this && draw_busy == 0)
      {
        // Since there is fog in the current camera sector we simulate
        // this by adding the view plane polygon.
        rview.GetG3D ()->DrawFogPolygon (GetID (), g3dpoly, CS_FOG_VIEW);
      }
      else
      {
        // We must add a FRONT fog polygon for the clipper to this sector.
        g3dpoly.normal = rview.GetClipPlane ();
	g3dpoly.normal.Invert ();
        g3dpoly.inv_aspect = rview.GetInvFOV ();
        rview.GetG3D ()->DrawFogPolygon (GetID (), g3dpoly, CS_FOG_FRONT);
      }
    }
    else if (fogmethod == G3DFOGMETHOD_VERTEX && rview.AddedFogInfo ())
    {
      csFogInfo *fog_info = rview.GetFogInfo ();
      rview.SetFogInfo (rview.GetFogInfo ()->next);
      delete fog_info;
    }
  }

  if (rview.GetCallback ()) rview.CallCallback (CALLBACK_SECTOREXIT, (void*)this);

  draw_busy--;
}

struct CheckFrustData
{
  csFrustumView* lview;
  csHashSet visible_things;
};

void* csSector::CheckFrustumPolygons (csSector*,
	csPolygonInt** polygon, int num, void* data)
{
  csPolygon3D* p;
  CheckFrustData* fdata = (CheckFrustData*)data;
  csFrustumView* lview = fdata->lview;
  csVector3& center = lview->light_frustum->GetOrigin ();
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
      lview->poly_func ((csObject*)p, lview);
    }
    else
    {
      //@@@ ONLY DO THIS WHEN QUADTREE IS USED!!!
      //csEngine::current_engine->GetQuadcube ()->InsertPolygon (poly, p->GetNumVertices ());
      lview->poly_func ((csObject*)p, lview);
    }
  }
  return NULL;
}

//@@@ Needs to be part of sector?
void CompressShadowFrustums (csFrustumList* list)
{
  csCBufferCube* cb = csEngine::current_engine->GetCBufCube ();
  csCovcube* cc = csEngine::current_engine->GetCovcube ();
  if (cb) cb->MakeEmpty ();
  else cc->MakeEmpty ();

  csShadowFrustum* sf = list->GetLast ();
  csSector* cur_sector = NULL;
  int cur_draw_busy = 0;
  if (sf)
  {
    cur_sector = sf->sector;
    cur_draw_busy = sf->draw_busy;
  }
  while (sf)
  {
    if (sf->sector != cur_sector || sf->draw_busy != cur_draw_busy)
      break;
    bool vis;
    if (cb)
      vis = cb->InsertPolygon (sf->GetVertices (), sf->GetNumVertices ());
    else
      vis = cc->InsertPolygon (sf->GetVertices (), sf->GetNumVertices ());
    if (!vis)
    {
      csShadowFrustum* sfdel = sf;
      sf = sf->prev;
      list->Unlink (sfdel);
      sfdel->DecRef ();
    }
    else
      sf = sf->prev;
  }
}

static int frust_cnt = 50;

//@@@ Needs to be part of sector?
void* CheckFrustumPolygonsFB (csSector* sector,
  csPolygonInt** polygon, int num, bool /*same_plane*/, void* data)
{
  csPolygon3D* p;
  CheckFrustData* fdata = (CheckFrustData*)data;
  csFrustumView* lview = fdata->lview;
  csVector3& center = lview->light_frustum->GetOrigin ();
  csCBufferCube* cb = csEngine::current_engine->GetCBufCube ();
  csCovcube* cc = csEngine::current_engine->GetCovcube ();
  bool cw = true;	// @@@ Mirror flag?
  int i, j;
  for (i = 0 ; i < num ; i++)
  {
    csVector3 poly[128];	// @@@ HARDCODED! BAD!

    if (polygon[i]->GetType () == 3)
    {
      // A BSP polygon. Used for testing visibility of things.
      csBspPolygon* bsppol = (csBspPolygon*)polygon[i];
      csObject* obj = bsppol->GetOriginator ();
      if (obj->GetType () == csThing::Type)
      {
        csThing* th = (csThing*)obj;
	if (!fdata->visible_things.In (th))
	{
	  csPolyIndexed& pi = bsppol->GetPolygon ();
	  csPolyTreeBBox* pi_par = bsppol->GetParent ();
	  csVector3Array& verts = pi_par->GetVertices ();
          for (j = 0 ; j < pi.GetNumVertices () ; j++)
            poly[j] = verts[pi[j]]-center;
          bool vis = false;
          if (cb)
	    vis = cb->TestPolygon (poly, pi.GetNumVertices ());
          else
	    vis = cc->TestPolygon (poly, pi.GetNumVertices ());
	  if (vis)
  	  {
	    csFrustumList* shadows;
	    if (lview->things_shadow)
	      // The thing is visible and we want things to cast
	      // shadows. So we add all shadows generated by this
	      // thing to the shadow list.
	      if (th != sector->GetStaticThing ())
	      {
	        shadows = th->GetShadows (sector, center);
	        lview->shadows.AppendList (shadows);
	        delete shadows;
	      }
	    fdata->visible_things.AddNoTest (th);
	  }
	}
      }
      continue;
    }
    if (polygon[i]->GetType () != 1) continue;
    p = (csPolygon3D*)polygon[i];

    for (j = 0 ; j < p->GetNumVertices () ; j++)
      poly[j] = p->Vwor (j)-center;
    bool vis = false;

    float clas = p->GetPlane ()->GetWorldPlane ().Classify (center);
    if (ABS (clas) < EPSILON) continue;
    if ((clas <= 0) != cw) continue;

    if (p->GetPortal ())
    {
      if (cb) vis = cb->TestPolygon (poly, p->GetNumVertices ());
      else vis = cc->TestPolygon (poly, p->GetNumVertices ());
    }
    else
    {
      if (cb) vis = cb->InsertPolygon (poly, p->GetNumVertices ());
      else vis = cc->InsertPolygon (poly, p->GetNumVertices ());
    }
    if (vis)
    {
      lview->poly_func ((csObject*)p, lview);

      csShadowFrustum* frust;
      frust = new csShadowFrustum (center);
      csPlane3 pl = p->GetPlane ()->GetWorldPlane ();
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
        CompressShadowFrustums (&(lview->shadows));
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
// @@@ This function needs to use the PVS. However, this function itself
// is used for the PVS so we need to take care!
bool CullOctreeNodeLighting (csPolygonTree* tree, csPolygonTreeNode* node,
  const csVector3& /*pos*/, void* data)
{
  if (!node) return false;
  if (node->Type () != NODE_OCTREE) return true;

  csOctree* otree = (csOctree*)tree;
  csOctreeNode* onode = (csOctreeNode*)node;
  csFrustumView* lview = (csFrustumView*)data;

  const csVector3& center = lview->light_frustum->GetOrigin ();
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
  float radius = lview->radius;
  if (radius < dist)
  {
    count_cull_dist++;
    return false;
  }

  if (ABS (dist) < EPSILON)
  {
    // We are in the node.
    if (lview->node_func) lview->node_func (onode, lview);
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
    csCovcube* cc = csEngine::current_engine->GetCovcube ();
    if (cb)
    {
      if (!cb->TestPolygon (outline, num_outline))
      {
        count_cull_quad++;
        return false;
      }
    }
    else if (cc && !cc->TestPolygon (outline, num_outline))
    {
      count_cull_quad++;
      return false;
    }
  }
  count_cull_not++;
  if (lview->node_func) lview->node_func (onode, lview);
  return true;
}

csThing** csSector::GetVisibleThings (csFrustumView& lview, int& num_things)
{
  csFrustum* lf = lview.light_frustum;
  bool infinite = lf->IsInfinite ();
  csVector3& center = lf->GetOrigin ();
  csPolygonSetBBox* bbox;
  bool vis;
  int i, i1;
  int j;

  num_things = things.Length ();
  if (!num_things) { return NULL; }
  csThing** visible_things = new csThing* [num_things];

  num_things = 0;
  for (j = 0 ; j < things.Length () ; j++)
  {
    csThing* sp = (csThing*)things[j];
    // If the light frustum is infinite then every thing
    // in this sector is of course visible.
    if (infinite) vis = true;
    else
    {
      bbox = sp->GetBoundingBox ();
      if (bbox)
      {
        // If we have a bounding box then we can do a quick test to
	// see if the bounding box is visible in the frustum. This
	// test is not complete in the sense that it will say that
	// some bounding boxes are visible even if they are not. But
	// it is correct in the sense that if it says a bounding box
	// is invisible, then it certainly is invisible.
	//
	// It works by taking all vertices of the bounding box. If
	// ALL of them are on the outside of the same plane from the
	// frustum then the object is certainly not visible.
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
	  // @@@ NOTE THIS IS UNTESTED CODE. LIGHT_FRUSTUMS CURRENTLY DON'T
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
  }
  return visible_things;
}

void csSector::CheckFrustum (csFrustumView& lview)
{
  csCBufferCube* cb = engine->GetCBufCube ();
  csCovcube* cc = engine->GetCovcube ();
  if (cb) cb->MakeEmpty ();
  else cc->MakeEmpty ();
  RealCheckFrustum (lview);
}

void csSector::RealCheckFrustum (csFrustumView& lview)
{
  if (draw_busy > cfg_reflections) return;
  draw_busy++;

  int i;
  csThing* sp;

  // Translate this sector so that it is oriented around
  // the position of the light (position of the light becomes
  // the new origin).
  csVector3& center = lview.light_frustum->GetOrigin ();

  // Check if gouraud shading needs to be updated.
  if (light_frame_number != current_light_frame_number)
  {
    light_frame_number = current_light_frame_number;
    lview.gouraud_color_reset = true;
  }
  else lview.gouraud_color_reset = false;

  // Data for the polygon traversal routines that are called below.
  CheckFrustData fdata;
  fdata.lview = &lview;

  // Remember the previous last shadow so that we can remove all
  // shadows that are added in this routine.
  csShadowFrustum* previous_last = lview.shadows.GetLast ();

  // When doing lighting there are two big cases: either we
  // have a static tree (octree) or not.
  if (static_tree)
  {
    // If there is a static tree (BSP and/or octree) then we
    // go front to back and add shadows to the list while we are doing
    // that. In future I would like to add some extra culling stage
    // here using a quad-tree or something similar (for example six
    // quad-trees arranged in a cube around the light).

    // All visible things will cause shadows to be added to 'lview'.
    // Later on we'll restore these shadows.
    count_cull_dist = 0;
    count_cull_quad = 0;
    count_cull_not = 0;
    static_thing->UpdateTransformation (center);
    frust_cnt = 50;
    static_tree->Front2Back (center, CheckFrustumPolygonsFB, (void*)&fdata,
      	CullOctreeNodeLighting, (void*)&lview);
    frust_cnt = 50;
    CheckFrustumPolygonsFB (this, polygons.GetArray (),
      polygons.Length (), false, (void*)&fdata);

    // Calculate lighting for all things in this sector.
    // The 'visible_things' hashset that is in fdata will contain
    // all things that are found visible while traversing the octree.
    // This queue is filled while traversing the octree
    // (CheckFrustumPolygonsFB).
    csHashIterator* it = fdata.visible_things.GetIterator ();
    while (it->HasNext ())
    {
      sp = (csThing*)(it->Next ());
      if (sp != static_thing)
        sp->RealCheckFrustum (lview);
    }
  }
  else
  {
    // Here we have no octree so we know the sector polygons are
    // convex. First find all things that are visible in the frustum.
    int num_visible_things;
    csThing** visible_things = GetVisibleThings (lview, num_visible_things);

    // Append the shadows for these things to the shadow list.
    // This list is appended to the one given in 'lview'. After
    // returning, the list in 'lview' will be restored.
    if (lview.things_shadow)
    {
      csFrustumList* shadows;
      for (i = 0 ; i < num_visible_things ; i++)
      {
        sp = visible_things[i];
        shadows = sp->GetShadows (this, center);
        lview.shadows.AppendList (shadows);
        delete shadows;
      }
    }

    // Calculate lighting for all polygons in this sector.
    CheckFrustumPolygons (this, polygons.GetArray (),
        polygons.Length (), (void*)&fdata);

    // Calculate lighting for all things in the current sector.
    for (i = 0 ; i < num_visible_things ; i++)
      visible_things[i]->RealCheckFrustum (lview);
      
    delete [] visible_things;
  }

  // Restore the shadow list in 'lview' and then delete
  // all the shadow frustums that were added in this recursion
  // level.
  csShadowFrustum* frustum;
  frustum = previous_last ? previous_last->next : lview.shadows.GetFirst ();
  lview.shadows.SetLast (previous_last);
  while (frustum)
  {
    csShadowFrustum* sf = frustum->next;
    frustum->DecRef ();
    frustum = sf;
  }

  draw_busy--;
}

void csSector::InitLightMaps (bool do_cache)
{
  int i;
  for (i = 0; i < polygons.Length (); i++)
    polygons.Get (i)->InitLightMaps (this, do_cache, i);

  for (i = 0 ; i < things.Length () ; i++)
  {
    csThing* sp = (csThing*)things[i];
    sp->InitLightMaps (do_cache);
  }
  for (i = 0 ; i < skies.Length () ; i++)
  {
    csThing* sp = (csThing*)skies[i];
    sp->InitLightMaps (do_cache);
  }
}

void csSector::CacheLightMaps ()
{
  int i;
  for (i = 0 ; i < polygons.Length (); i++)
    polygons.Get (i)->CacheLightMaps (this, i);

  for (i = 0 ; i < things.Length () ; i++)
  {
    csThing* sp = (csThing*)things[i];
    sp->CacheLightMaps ();
  }
  for (i = 0 ; i < skies.Length () ; i++)
  {
    csThing* sp = (csThing*)skies[i];
    sp->CacheLightMaps ();
  }
}

csThing* csSector::GetThing (const char* name)
{
  int i;
  for (i = 0 ; i < things.Length () ; i++)
  {
    csThing* s = (csThing*)things[i];
    if (!strcmp (name, s->GetName ()))
      return s;
  }
  return NULL;
}

csThing* csSector::GetSky (const char* name)
{
  int i;
  for (i = 0 ; i < skies.Length () ; i++)
  {
    csThing* s = (csThing*)skies[i];
    if (!strcmp (name, s->GetName ()))
      return s;
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

csStatLight* csSector::FindLight (CS_ID id)
{
  int i;
  for (i = 0 ; i < lights.Length () ; i++)
  {
    csStatLight* l = (csStatLight*)lights[i];
    if (l->GetID () == id) return l;
  }
  return NULL;
}

//---------------------------------------------------------------------------

iThing *csSector::eiSector::GetSkyThing (const char *name)
{
  return &scfParent->GetSky (name)->scfiThing;
}

iThing *csSector::eiSector::GetSkyThing (int iIndex)
{
  return &((csThing*)(scfParent->skies[iIndex]))->scfiThing;
}

iThing *csSector::eiSector::GetThing (const char *name)
{
  return &scfParent->GetThing (name)->scfiThing;
}

iThing *csSector::eiSector::GetThing (int iIndex)
{
  return &((csThing*)(scfParent->things[iIndex]))->scfiThing;
}

void csSector::eiSector::AddLight (iStatLight *light)
{
  scfParent->AddLight (light->GetPrivateObject ());
}

iStatLight *csSector::eiSector::FindLight (float x, float y, float z, float dist)
{
  return &scfParent->FindLight (x, y, z, dist)->scfiStatLight;
}
