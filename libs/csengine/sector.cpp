/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "csutil/csppulse.h"
#include "csengine/sector.h"
#include "csengine/thing.h"
#include "csengine/meshobj.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/polytmap.h"
#include "csengine/cscoll.h"
#include "csengine/light.h"
#include "csengine/camera.h"
#include "csengine/engine.h"
#include "csengine/stats.h"
#include "csengine/cbuffer.h"
#include "csengine/bspbbox.h"
#include "csengine/cbufcube.h"
#include "csengine/bsp.h"
#include "csengine/octree.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "isys/vfs.h"
#include "iengine/statlght.h"
#include "iengine/viscull.h"
#include "iengine/rview.h"

// Option variable: render portals?
bool csSector::do_portals = true;
// Configuration variable: number of allowed reflections for static lighting.
int csSector::cfg_reflections = 1;
// Option variable: do pseudo radiosity?
bool csSector::do_radiosity = false;

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT (csSector)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iReferencedObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSector)
  SCF_IMPLEMENTS_INTERFACE (csSector);
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSector::eiSector)
  SCF_IMPLEMENTS_INTERFACE (iSector)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSector::ReferencedObject)
  SCF_IMPLEMENTS_INTERFACE (iReferencedObject)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSector::csSector (csEngine* engine) : csObject ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSector);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiReferencedObject);
  csSector::engine = engine;
  culler_mesh = NULL;
  culler = NULL;
  engine->AddToCurrentRegion (this);
  fog.enabled = false;
  draw_busy = 0;
}

csSector::~csSector ()
{
  // The references to this sector MUST be cleaned up before this
  // sector is destructed.
  CS_ASSERT (references.Length () == 0);

  // Meshes and collections are not deleted by the calls below. They
  // belong to csEngine.
  collections.DeleteAll ();
  meshes.DeleteAll ();
  int i;
  for (i = 0 ; i < mesh_priority_queues.Length () ; i++)
  {
    csVector* m = (csVector*)mesh_priority_queues[i];
    if (m) m->DeleteAll ();
    delete m;
  }
  mesh_priority_queues.DeleteAll ();

  lights.DeleteAll ();
  if (culler) culler->DecRef ();
}

void csSector::CleanupReferences ()
{
  while (references.Length () > 0)
  {
    iReference* ref = (iReference*)references[references.Length ()-1];
#   ifdef CS_DEBUG
    // Sanity check.
    iReferencedObject* refobj = ref->GetReferencedObject ();
    CS_ASSERT (refobj == &scfiReferencedObject);
#   endif
    ref->SetReferencedObject (NULL);
  }
}

//----------------------------------------------------------------------

void csSector::AddMesh (csMeshWrapper* mesh)
{
  meshes.Push ((csSome)mesh);

  //-----
  // Place the mesh in the right priority queue.
  //-----
  long pri = mesh->GetRenderPriority ();
  int i;
  // First initialize all uninitialized queues.
  for (i = mesh_priority_queues.Length () ; i <= pri ; i++)
    mesh_priority_queues[i] = NULL;
  csVector* queue = (csVector*)mesh_priority_queues[pri];
  if (queue == NULL)
  {
    queue = new csVector ();
    mesh_priority_queues[pri] = queue;
  }
  queue->Push ((csSome)mesh);

  if (culler)
  {
    iVisibilityObject* vo = SCF_QUERY_INTERFACE (mesh, iVisibilityObject);
    vo->DecRef ();
    culler->RegisterVisObject (vo);
  }
}

void csSector::UnlinkMesh (csMeshWrapper* mesh)
{
  //-----
  // First remove the mesh from the right priority queue.
  //-----
  long pri = mesh->GetRenderPriority ();
  int i, idx;
  // First initialize all uninitialized queues.
  for (i = mesh_priority_queues.Length () ; i <= pri ; i++)
    mesh_priority_queues[i] = NULL;
  csVector* queue = (csVector*)mesh_priority_queues[pri];
  if (queue != NULL)
  {
    idx = queue->Find ((csSome)mesh);
    queue->Delete (idx);
  }

  idx = meshes.Find ((csSome)mesh);
  if (idx != -1)
  {
    meshes.Delete (idx);
    if (culler)
    {
      iVisibilityObject* vo = SCF_QUERY_INTERFACE (mesh, iVisibilityObject);
      vo->DecRef ();
      culler->UnregisterVisObject (vo);
    }
  }
}

void csSector::RelinkMesh (csMeshWrapper* mesh)
{
  //-----
  // Try to find the mesh in some render priority queue.
  //-----
  int i;
  for (i = 0 ; i < mesh_priority_queues.Length () ; i++)
  {
    csVector* queue = (csVector*)mesh_priority_queues[i];
    if (queue != NULL)
    {
      int idx = queue->Find ((csSome)mesh);
      if (idx != -1)
      {
        queue->Delete (idx);
	break;
      }
    }
  }

  //-----
  // Place the mesh in the right priority queue.
  //-----
  long pri = mesh->GetRenderPriority ();
  // First initialize all uninitialized queues.
  for (i = mesh_priority_queues.Length () ; i <= pri ; i++)
    mesh_priority_queues[i] = NULL;
  csVector* queue = (csVector*)mesh_priority_queues[pri];
  if (queue == NULL)
  {
    queue = new csVector ();
    mesh_priority_queues[pri] = queue;
  }
  queue->Push ((csSome)mesh);
}

csMeshWrapper* csSector::GetMesh (const char* name) const
{
  int i;
  for (i = 0 ; i < meshes.Length () ; i++)
  {
    csMeshWrapper* s = (csMeshWrapper*)meshes[i];
    if (!strcmp (name, s->GetName ()))
      return s;
  }
  return NULL;
}

//----------------------------------------------------------------------

void csSector::AddCollection (csCollection* col)
{
  collections.Push ((csSome)col);
}

void csSector::UnlinkCollection (csCollection* col)
{
  int idx = collections.Find ((csSome)col);
  if (idx != -1) collections.Delete (idx);
}

csCollection* csSector::GetCollection (const char* name) const
{
  int i;
  for (i = 0 ; i < collections.Length () ; i++)
  {
    csCollection* s = (csCollection*)collections[i];
    if (!strcmp (name, s->GetName ()))
      return s;
  }
  return NULL;
}

//----------------------------------------------------------------------

void csSector::AddLight (csStatLight* light)
{
  lights.Push ((csSome)light);
  light->SetSector (this);
}

void csSector::UnlinkLight (csStatLight* light)
{
  int idx = lights.Find ((csSome)light);
  if (idx != -1) { lights[idx] = NULL; lights.Delete (idx); }
}

csStatLight* csSector::FindLight (float x, float y, float z, float dist) const
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

csStatLight* csSector::FindLight (unsigned long id) const
{
  int i;
  for (i = 0 ; i < lights.Length () ; i++)
  {
    csStatLight* l = (csStatLight*)lights[i];
    if (l->GetLightID () == id) return l;
  }
  return NULL;
}

//----------------------------------------------------------------------

void csSector::UseCuller (const char* meshname)
{
  if (culler_mesh) return;
  culler_mesh = GetMesh (meshname);
  if (!culler_mesh) return;
  culler = SCF_QUERY_INTERFACE (culler_mesh->GetMeshObject (), iVisibilityCuller);
  if (!culler) return;
  culler->Setup ();

  // Loop through all meshes and update their bounding box in the
  // polygon trees.
  int i;
  for (i = 0 ; i < meshes.Length () ; i++)
  {
    csMeshWrapper* th = (csMeshWrapper*)meshes[i];
    th->GetMovable ().UpdateMove ();

    iVisibilityObject* vo = SCF_QUERY_INTERFACE (th, iVisibilityObject);
    vo->DecRef ();
    culler->RegisterVisObject (vo);
  }
  
  CsPrintf (CS_MSG_INITIALIZATION, "DONE!\n");
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

csObject* csSector::HitBeam (const csVector3& start, const csVector3& end, csVector3& isect,
	csPolygon3D** polygonPtr)
{
  float r, best_mesh_r = 10000000000.;
  csMeshWrapper* near_mesh = NULL;

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
      csObject* obj = po->HitBeam (new_start, end, isect, polygonPtr);
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

void* IntersectSegmentTestPol (csThing*,
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
  csVector3 obj_start, obj_end, obj_isect;
  csReversibleTransform movtrans;

  int i;
  for (i = 0 ; i < meshes.Length () ; i++)
  {
    csMeshWrapper* mesh = (csMeshWrapper*)meshes[i];
    if (mesh != culler_mesh)
    {
      // @@@ UGLY!!!
      iThingState* ith = SCF_QUERY_INTERFACE (mesh->GetMeshObject (), iThingState);
      if (ith)
      {
        csThing* sp = (csThing*)(ith->GetPrivateObject ());
        r = best_r;
	//@@@ Put this in csMeshWrapper???
        if (sp->GetMovingOption () == CS_THING_MOVE_NEVER)
        {
          obj_start = start;
	  obj_end = end;
        }
        else
        {
          movtrans = mesh->GetMovable ().GetFullTransform ();
          obj_start = movtrans.Other2This (start);
	  obj_end = movtrans.Other2This (end);
        }
        csPolygon3D* p = sp->IntersectSegment (obj_start, obj_end,
		obj_isect, &r);
        if (sp->GetMovingOption () == CS_THING_MOVE_NEVER)
          cur_isect = obj_isect;
        else
          cur_isect = movtrans.This2Other (obj_isect);

        if (p && r < best_r)
        {
          best_r = r;
	  best_p = p;
	  isect = cur_isect;
        }
        ith->DecRef ();
      }
    }
  }

  if (culler_mesh)
  {
    // culler_mesh has option CS_THING_MOVE_NEVER so
    // object space == world space.
    // @@@ UGLY!!! We need another abstraction for this.
    iThingState* ith = SCF_QUERY_INTERFACE (
    	culler_mesh->GetMeshObject (), iThingState);
    csThing* sp = (csThing*)(ith->GetPrivateObject ());
    ith->DecRef ();

    csPolygonTree* static_tree = sp->GetStaticTree ();
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

  if (pr) *pr = best_r;
  return best_p;
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
      po->CompleteSector (NULL);
      if (!po->GetSector ())
      {
        new_position = isect;
	return this;
      }
      if (po->flags.Check (CS_PORTAL_WARP))
      {
        po->WarpSpace (t, mirror);
	new_position = po->Warp (new_position);
      }
      csSector* dest_sect = po->GetSector ()->GetPrivateObject ();
      return dest_sect->FollowSegment (t, new_position, mirror);
    }
    else
      new_position = isect;
  }

  return this;
}

csPolygon3D* csSector::IntersectSphere (csVector3& center, float radius,
  float* pr)
{
  float min_d = radius;
  csPolygon3D* min_p = NULL;
  (void)center;
#if 0
  float d = .0f;
  csPolygon3D* p = NULL, * min_p = NULL;
  csVector3 obj_center;
  csReversibleTransform movtrans;

//@@@ Support for meshes!!!
  int i;
  for (i = 0 ; i < things.Length () ; i++)
  {
    csThing* sp = (csThing*)things[i];
    if (sp->GetMovingOption () == CS_THING_MOVE_NEVER)
      obj_center = center;
    else
    {
      movtrans = sp->GetMovable ().GetFullTransform ();
      obj_center = movtrans.Other2This (center);
    }
    p = sp->IntersectSphere (obj_center, radius, &d);
    if (p && d < min_d)
    {
      min_d = d;
      min_p = p;
    }
  }
#endif

  if (pr) *pr = min_d;
  return min_p;
}

#if 0
int compare_z_thing (const void* p1, const void* p2)
{
  csMeshWrapper* sp1 = *(csMeshWrapper**)p1;
  csMeshWrapper* sp2 = *(csMeshWrapper**)p2;
  float z1 = sp1->Vcam (sp1->GetCenter ()).z;
  float z2 = sp2->Vcam (sp2->GetCenter ()).z;
  if (z1 < z2) return -1;
  else if (z1 > z2) return 1;
  return 0;
}
#endif

// @@@ THE NOTES BELOW ARE MOSTLY OBSOLETE NOW. I DON'T REMOVE THEM
// BECAUSE THERE IS STILL A GRAIN OF USEFUL INFORMATION IN THEM.
//
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

void csSector::Draw (iRenderView* rview)
{
  draw_busy++;
  int i, j;
  iCamera* icam = rview->GetCamera ();
  rview->SetThisSector (&scfiSector);

  G3D_FOGMETHOD fogmethod = G3DFOGMETHOD_NONE;

  if (rview->GetCallback ())
  {
    rview->CallCallback (CALLBACK_SECTOR, (void*)&scfiSector);
  }
  else
  {
    if (HasFog ())
    {
      if ((fogmethod = csEngine::current_engine->fogmethod)
    	  == G3DFOGMETHOD_VERTEX)
      {
        csFogInfo* fog_info = new csFogInfo ();
        fog_info->next = rview->GetFirstFogInfo ();
        iPolygon3D* ipoly3d = rview->GetPortalPolygon ();
        if (ipoly3d)
        {
          fog_info->incoming_plane = ipoly3d->GetCameraPlane ();
          fog_info->incoming_plane.Invert ();
	  fog_info->has_incoming_plane = true;
        }
        else fog_info->has_incoming_plane = false;
        fog_info->fog = &GetFog ();
        fog_info->has_outgoing_plane = true;
        rview->SetFirstFogInfo (fog_info);
      }
      else if (fogmethod != G3DFOGMETHOD_NONE)
      {
        rview->GetGraphics3D ()->OpenFogObject (GetID (), &GetFog ());
      }
    }
  }

  // In some cases this queue will be filled with all visible
  // meshes.
  csMeshWrapper** mesh_queue = NULL;
  int num_mesh_queue = 0;
  // If the following flag is true the queue is actually used.
  bool use_object_queue = false;

  // If we have a visibility culler in this sector we use it here.
  if (culler)
  {
    if (culler->VisTest (rview))
    {
      // The visibility culler worked and marked all registered
      // visible things as visible.

      // Fill the mesh queue for all meshes that were visible.
      use_object_queue = true;
      if (meshes.Length () > 0)
      {
	// Push all visible meshes in a queue.
	// @@@ Avoid memory allocation?
	mesh_queue = new csMeshWrapper* [meshes.Length ()];
	num_mesh_queue = 0;
        for (i = 0 ; i < mesh_priority_queues.Length () ; i++)
        {
	  csVector* v = (csVector*)mesh_priority_queues[i];
	  if (v)
	    for (j = 0 ; j < v->Length () ; j++)
	    {
              csMeshWrapper* sp = (csMeshWrapper*)(*v)[j];
	      if (sp->IsVisible ()) mesh_queue[num_mesh_queue++] = sp;
	    }
	}
      }
    }
    else
    {
      // The visibility culler was either disabled or decided visibility
      // culling was not useful given some circumstances. In this case
      // all objects should be considered visible.
    }
  }

  // If the queues are not used for things we still fill the queue here
  // just to make the code below easier.
  if (!use_object_queue)
  {
    num_mesh_queue = 0;
    if (meshes.Length ())
    {
      mesh_queue = new csMeshWrapper* [meshes.Length ()];
      for (i = 0 ; i < mesh_priority_queues.Length () ; i++)
      {
        csVector* v = (csVector*)mesh_priority_queues[i];
	if (v)
	  for (j = 0 ; j < v->Length () ; j++)
	  {
            csMeshWrapper* sp = (csMeshWrapper*)(*v)[j];
            mesh_queue[num_mesh_queue++] = sp;
	  }
      }
    }
    else
      mesh_queue = NULL;
  }

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
  iSector* previous_sector = rview->GetPreviousSector ();

  int spr_num;
  if (mesh_queue) spr_num = num_mesh_queue;
  else spr_num = meshes.Length ();

  if (rview->AddedFogInfo ())
    rview->GetFirstFogInfo ()->has_outgoing_plane = false;

  for (i = 0 ; i < spr_num ; i++)
  {
    csMeshWrapper* sp;
    if (mesh_queue) sp = mesh_queue[i];
    else sp = (csMeshWrapper*)meshes[i];

    if (!previous_sector || sp->GetMovable ().GetSectors ().
    	Find (previous_sector->GetPrivateObject ()) == -1)
    {
      // Mesh is not in the previous sector or there is no previous sector.
      sp->Draw (rview);
    }
    else
    {
      if (
	  previous_sector->HasFog () ||
	  rview->GetPortalPolygon ()->IsTransparent () ||
	  rview->GetPortalPolygon ()->GetPortal ()->GetFlags ().
	  	Check (CS_PORTAL_WARP))
      {
	// @@@ Here we should draw clipped to the portal.
	sp->Draw (rview);
      }
    }
  }
  delete [] mesh_queue;

  // queue all halos in this sector to be drawn.
  for (i = lights.Length () - 1; i >= 0; i--)
    // Tell the engine to try to add this light into the halo queue
    csEngine::current_engine->AddHalo ((csLight *)lights.Get (i));

  if (rview->GetCallback ())
  {
    rview->CallCallback (CALLBACK_SECTOREXIT, (void*)&scfiSector);
  }
  else
  {
    // Handle the fog, if any
    if (fogmethod != G3DFOGMETHOD_NONE)
    {
      G3DPolygonDFP g3dpoly;
      if (fogmethod == G3DFOGMETHOD_ZBUFFER)
      {
        g3dpoly.num = rview->GetClipper ()->GetVertexCount ();
        csVector2 *clipview = rview->GetClipper ()->GetClipPoly ();
        memcpy (g3dpoly.vertices, clipview, g3dpoly.num * sizeof (csVector2));
        if (icam->GetSector () == &scfiSector && draw_busy == 0)
        {
          // Since there is fog in the current camera sector we simulate
          // this by adding the view plane polygon.
          rview->GetGraphics3D ()->DrawFogPolygon (GetID (),
		g3dpoly, CS_FOG_VIEW);
        }
        else
        {
          // We must add a FRONT fog polygon for the clipper to this sector.
          rview->GetClipPlane (g3dpoly.normal);
	  g3dpoly.normal.Invert ();
          rview->GetGraphics3D ()->DrawFogPolygon (GetID (), g3dpoly,
	  	CS_FOG_FRONT);
        }
      }
      else if (fogmethod == G3DFOGMETHOD_VERTEX && rview->AddedFogInfo ())
      {
        csFogInfo *fog_info = rview->GetFirstFogInfo ();
        rview->SetFirstFogInfo (rview->GetFirstFogInfo ()->next);
        delete fog_info;
      }
    }
  }

  draw_busy--;
}

csObject** csSector::GetVisibleObjects (iFrustumView* lview, int& num_objects)
{
  csFrustum* lf = lview->GetFrustumContext ()->GetLightFrustum ();
  bool infinite = lf->IsInfinite ();
  csVector3& c = lf->GetOrigin ();
  bool vis;
  int i, i1;
  int j;

  num_objects = meshes.Length ();
  if (!num_objects) { return NULL; }
  csObject** visible_objects = new csObject* [num_objects];

  num_objects = 0;
  // @@@ Unify both loops below once csThing becomes a mesh object.
  for (j = 0 ; j < meshes.Length () ; j++)
  {
    csMeshWrapper* sp = (csMeshWrapper*)meshes[j];
    // If the light frustum is infinite then every thing
    // in this sector is of course visible.
    if (infinite) vis = true;
    else
    {
      csBox3 bbox;
      sp->GetWorldBoundingBox (bbox);
      // Here we do a quick test to see if the bounding box is visible in
      // in the frustum. This test is not complete in the sense that it will
      // say that some bounding boxes are visible even if they are not. But
      // it is correct in the sense that if it says a bounding box
      // is invisible, then it certainly is invisible.
      //
      // It works by taking all vertices of the bounding box. If
      // ALL of them are on the outside of the same plane from the
      // frustum then the object is certainly not visible.
      vis = true;
      i1 = lf->GetVertexCount ()-1;
      for (i = 0 ; i < lf->GetVertexCount () ; i1 = i, i++)
      {
        csVector3& v1 = lf->GetVertex (i);
        csVector3& v2 = lf->GetVertex (i1);
        if (csMath3::WhichSide3D (bbox.GetCorner (0)-c, v1, v2) < 0) continue;
        if (csMath3::WhichSide3D (bbox.GetCorner (1)-c, v1, v2) < 0) continue;
        if (csMath3::WhichSide3D (bbox.GetCorner (2)-c, v1, v2) < 0) continue;
        if (csMath3::WhichSide3D (bbox.GetCorner (3)-c, v1, v2) < 0) continue;
        if (csMath3::WhichSide3D (bbox.GetCorner (4)-c, v1, v2) < 0) continue;
        if (csMath3::WhichSide3D (bbox.GetCorner (5)-c, v1, v2) < 0) continue;
        if (csMath3::WhichSide3D (bbox.GetCorner (6)-c, v1, v2) < 0) continue;
        if (csMath3::WhichSide3D (bbox.GetCorner (7)-c, v1, v2) < 0) continue;
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
        if (!csMath3::Visible (bbox.GetCorner (0)-c, *lf->GetBackPlane ()) &&
            !csMath3::Visible (bbox.GetCorner (1)-c, *lf->GetBackPlane ()) &&
            !csMath3::Visible (bbox.GetCorner (2)-c, *lf->GetBackPlane ()) &&
            !csMath3::Visible (bbox.GetCorner (3)-c, *lf->GetBackPlane ()) &&
            !csMath3::Visible (bbox.GetCorner (4)-c, *lf->GetBackPlane ()) &&
            !csMath3::Visible (bbox.GetCorner (5)-c, *lf->GetBackPlane ()) &&
            !csMath3::Visible (bbox.GetCorner (6)-c, *lf->GetBackPlane ()) &&
            !csMath3::Visible (bbox.GetCorner (7)-c, *lf->GetBackPlane ()))
          vis = false;
      }
    }

    if (vis) visible_objects[num_objects++] = sp;
  }
  return visible_objects;
}

void csSector::CheckFrustum (iFrustumView* lview)
{
  csCBufferCube* cb = engine->GetCBufCube ();
  cb->MakeEmpty ();
  RealCheckFrustum (lview);
}

void csSector::RealCheckFrustum (iFrustumView* lview)
{
  if (draw_busy > cfg_reflections) return;
  draw_busy++;

  int i;

  // Translate this sector so that it is oriented around
  // the position of the light (position of the light becomes
  // the new origin).
  csVector3& center = lview->GetFrustumContext ()->GetLightFrustum ()->
  	GetOrigin ();

  iShadowBlockList* shadows = lview->GetFrustumContext ()->GetShadows ();

  // Remember the previous last shadow so that we can remove all
  // shadows that are added in this routine.
  iShadowBlock* previous_last = shadows->GetLastShadowBlock ();

  if (culler && culler->SupportsShadowCasting ())
  {
    culler->CastShadows (lview);
  }
  else
  {
    // Here we have no octree so we know the sector polygons are
    // convex. First find all objects that are visible in the frustum.
    int num_visible_objects;
    csObject** visible_objects = GetVisibleObjects (lview, num_visible_objects);

    // Append the shadows for these objects to the shadow list.
    // This list is appended to the one given in 'lview'. After
    // returning, the list in 'lview' will be restored.
    if (lview->ThingShadowsEnabled ())
    {
      for (i = 0 ; i < num_visible_objects ; i++)
      {
        // @@@ unify with other mesh objects as soon as possible
	// @@@ Also use shadow caster interface to append shadows!
        csObject* o = visible_objects[i];
	csMeshWrapper* mesh = (csMeshWrapper*)o;
	// @@@ should not be known in engine.
	// @@@ UGLY
	iThingState* ithing = SCF_QUERY_INTERFACE (
		mesh->GetMeshObject (), iThingState);
	if (ithing)
	{
	  csThing* sp = (csThing*)(ithing->GetPrivateObject ());
	  // Only if the thing has right flags do we consider it for shadows.
	  if (lview->CheckShadowMask (mesh->flags.Get ()))
	    sp->AppendShadows (&(mesh->GetMovable ().scfiMovable),
	    	shadows, center);
	  ithing->DecRef ();
	}
      }
    }

    // Calculate lighting for all objects in the current sector.
    for (i = 0 ; i < num_visible_objects ; i++)
    {
      // @@@ unify with other mesh objects as soon as possible
      // @@@ Use shadow receiver interface!!!
      csObject* o = visible_objects[i];
      csMeshWrapper* mesh = (csMeshWrapper*)o;
      // @@@ should not be known in engine.
      // @@@ UGLY
      iThingState* ithing = SCF_QUERY_INTERFACE (
      	mesh->GetMeshObject (), iThingState);
      if (ithing)
      {
        csThing* sp = (csThing*)(ithing->GetPrivateObject ());
        // Only if the thing has right flags do we consider it for shadows.
        if (lview->CheckProcessMask (mesh->flags.Get ()))
          sp->RealCheckFrustum (lview, &(mesh->GetMovable ().scfiMovable));
        ithing->DecRef ();
      }
    }
      
    delete [] visible_objects;
  }

  // Restore the shadow list in 'lview' and then delete
  // all the shadow frustums that were added in this recursion
  // level.
  while (shadows->GetLastShadowBlock () != previous_last)
  {
    iShadowBlock* sh = shadows->GetLastShadowBlock ();
    shadows->RemoveLastShadowBlock ();
    sh->DecRef ();
  }

  draw_busy--;
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

void csSector::ShineLights (iMeshWrapper* mesh, csProgressPulse* pulse)
{
  for (int i = 0 ; i < lights.Length () ; i++)
  {
    if (pulse != 0)
      pulse->Step();
    ((csStatLight*)lights[i])->CalculateLighting (mesh);
  }
}

void csSector::CalculateSectorBBox (csBox3& bbox,
	bool do_meshes) const
{
  bbox.StartBoundingBox ();
  csBox3 b;
  int i;
  if (do_meshes)
    for (i = 0 ; i < meshes.Length () ; i++)
    {
      csMeshWrapper* mesh = (csMeshWrapper*)meshes[i];
      mesh->GetTransformedBoundingBox (mesh->GetMovable ().GetTransform (), b);
      bbox += b;
    }
}

//---------------------------------------------------------------------------

iMeshWrapper *csSector::eiSector::GetMesh (int n) const
{
  return &scfParent->GetMesh (n)->scfiMeshWrapper;
}

void csSector::eiSector::AddMesh (iMeshWrapper *pMesh)
{
  scfParent->AddMesh (pMesh->GetPrivateObject ());
}

iMeshWrapper *csSector::eiSector::GetMesh (const char *name) const
{
  csMeshWrapper *mw = scfParent->GetMesh (name);
  return mw ? &mw->scfiMeshWrapper : NULL;
}

void csSector::eiSector::AddLight (iStatLight *light)
{
  scfParent->AddLight (light->GetPrivateObject ());
}

iStatLight *csSector::eiSector::FindLight (float x, float y, float z,
	float dist) const
{
  return &scfParent->FindLight (x, y, z, dist)->scfiStatLight;
}

iCollection* csSector::eiSector::GetCollection (int n) const
{
  return &(scfParent->GetCollection (n)->scfiCollection);
}

void csSector::eiSector::AddCollection (iCollection* col)
{
  scfParent->AddCollection ((csCollection*)(col->GetPrivateObject ()));
}

iCollection* csSector::eiSector::GetCollection (const char *name) const
{
  csCollection* tw = (csCollection*)(scfParent->GetCollection (name));
  return tw ? &tw->scfiCollection : NULL;
}

void csSector::eiSector::UnlinkCollection (iCollection* col)
{
  scfParent->UnlinkCollection ((csCollection*)(col->GetPrivateObject ()));
}

iStatLight* csSector::eiSector::GetLight (int n) const
{
  return &(scfParent->GetLight (n)->scfiStatLight);
}

iStatLight* csSector::eiSector::GetLight (const char *name) const
{
  csStatLight* tw = scfParent->GetLight (name);
  return tw ? &tw->scfiStatLight : NULL;
}

iPolygon3D* csSector::eiSector::HitBeam (const csVector3& start,
	const csVector3& end, csVector3& isect)
{
  csPolygon3D* p = scfParent->HitBeam (start, end, isect);
  if (p) return &(p->scfiPolygon3D);
  else return NULL;
}

iObject* csSector::eiSector::HitBeam (const csVector3& start,
	const csVector3& end, csVector3& isect, iPolygon3D** polygonPtr)
{
  csPolygon3D* p = NULL;
  csObject* obj = scfParent->HitBeam (start, end, isect, polygonPtr ? &p : NULL);
  if (obj)
  {
    if (p)
    {
      *polygonPtr = &(p->scfiPolygon3D);
    }
  }
  return (iObject*)obj;
}

iSector* csSector::eiSector::FollowSegment (csReversibleTransform& t,
  	csVector3& new_position, bool& mirror)
{
  csSector* s = scfParent->FollowSegment (t, new_position, mirror);
  if (s) return &(s->scfiSector);
  else return NULL;
}

void csSector::ReferencedObject::AddReference (iReference* ref)
{
  scfParent->references.Push (ref);
}

void csSector::ReferencedObject::RemoveReference (iReference* ref)
{
  int i;
  // We scan backwards because we know that the code to remove all
  // refs to a sector will also scan backwards. So this is more efficient.
  for (i = scfParent->references.Length ()-1 ; i >= 0 ; i--)
  {
    iReference* r = (iReference*)(scfParent->references[i]);
    if (r == ref)
    {
      scfParent->references.Delete (i);
      return;
    }
  }
  // Hopefully we never come here.
  CS_ASSERT (false);
}

