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
#include "csutil/debug.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "iutil/objreg.h"
#include "igeom/clip2d.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "csengine/engine.h"
#include "csengine/sector.h"
#include "csengine/light.h"
#include "iengine/rview.h"
#include "imesh/lighting.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/portal.h"

// Option variable: render portals?
bool csSector:: do_portals = true;

// Configuration variable: number of allowed reflections for static lighting.
int csSector:: cfg_reflections = 1;

// Option variable: do pseudo radiosity?
bool csSector:: do_radiosity = false;

//---------------------------------------------------------------------------
csSectorLightList::csSectorLightList ()
{
  sector = NULL;
}

void csSectorLightList::PrepareItem (iLight* item)
{
  csLightList::PrepareItem (item);
  item->SetSector (&(sector->scfiSector));
}

void csSectorLightList::FreeItem (iLight* item)
{
  item->SetSector (NULL);
  csLightList::FreeItem (item);
}

//---------------------------------------------------------------------------
csSectorMeshList::csSectorMeshList ()
{
  sector = NULL;
}

void csSectorMeshList::PrepareItem (iMeshWrapper* item)
{
  CS_ASSERT (sector != NULL);
  csMeshList::PrepareItem (item);
  sector->PrepareMesh (item);
}

void csSectorMeshList::FreeItem (iMeshWrapper* item)
{
  CS_ASSERT (sector != NULL);
  sector->UnprepareMesh (item);
  csMeshList::FreeItem (item);
}

//---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE_EXT(csSector)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iReferencedObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iSector)
  SCF_IMPLEMENTS_INTERFACE (csSector);
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSector::eiSector)
  SCF_IMPLEMENTS_INTERFACE(iSector)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSector::ReferencedObject)
  SCF_IMPLEMENTS_INTERFACE(iReferencedObject)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSector::csSector (csEngine *engine) :
  csObject()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSector);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiReferencedObject);
  DG_TYPE (this, "csSector");
  csSector::engine = engine;
  engine->AddToCurrentRegion (this);
#ifndef CS_USE_NEW_RENDERER
  fog.enabled = false;
#endif // CS_USE_NEW_RENDERER
  draw_busy = 0;
  dynamic_ambient_color.Set(0,0,0);
  meshes.SetSector (this);
  lights.SetSector (this);

#ifdef CS_USE_NEW_RENDERER
  r3d = CS_QUERY_REGISTRY (csEngine::object_reg, iRender3D);
#endif // CS_USE_NEW_RENDERER
}

csSector::~csSector ()
{
  // The references to this sector MUST be cleaned up before this
  // sector is destructed.
  CS_ASSERT (references.Length () == 0);

  lights.RemoveAll ();
}

void csSector::CleanupReferences ()
{
  while (references.Length () > 0)
  {
    iReference *ref = (iReference *)references[references.Length () - 1];
#ifdef CS_DEBUG
    // Sanity check.
    iReferencedObject *refobj = ref->GetReferencedObject ();
    CS_ASSERT (refobj == &scfiReferencedObject);
#endif
    ref->SetReferencedObject (NULL);
  }
}

//----------------------------------------------------------------------

void csSector::RegisterMeshToCuller (iMeshWrapper* mesh)
{
  csRef<iVisibilityObject> vo (SCF_QUERY_INTERFACE (mesh,
        iVisibilityObject));
  culler->RegisterVisObject (vo);
  int i;
  iMeshList* ml = mesh->GetChildren ();
  for (i = 0 ; i < ml->GetCount () ; i++)
  {
    iMeshWrapper* child = ml->Get (i);
    RegisterMeshToCuller (child);
  }
}

void csSector::UnregisterMeshToCuller (iMeshWrapper* mesh)
{
  csRef<iVisibilityObject> vo (SCF_QUERY_INTERFACE (mesh,
        iVisibilityObject));
  culler->UnregisterVisObject (vo);
  int i;
  iMeshList* ml = mesh->GetChildren ();
  for (i = 0 ; i < ml->GetCount () ; i++)
  {
    iMeshWrapper* child = ml->Get (i);
    UnregisterMeshToCuller (child);
  }
}

void csSector::PrepareMesh (iMeshWrapper *mesh)
{
  RenderQueues.Add (mesh);
  if (culler) RegisterMeshToCuller (mesh);
}

void csSector::UnprepareMesh (iMeshWrapper *mesh)
{
  RenderQueues.Remove (mesh);
  if (culler) UnregisterMeshToCuller (mesh);
}

void csSector::RelinkMesh (iMeshWrapper *mesh)
{
  // @@@ this function would be a lot faster if the previous
  // priority was known!
  RenderQueues.RemoveUnknownPriority (mesh);
  RenderQueues.Add (mesh);
}

//----------------------------------------------------------------------

bool csSector::UseCullerPlugin (const char *plugname)
{
  culler = NULL;

  // Load the culler plugin.
  csRef<iPluginManager> plugmgr (CS_QUERY_REGISTRY (csEngine::object_reg,
  	iPluginManager));
  culler = CS_LOAD_PLUGIN (plugmgr, plugname, iVisibilityCuller);

  if (!culler)
  {
    return false;
  }

  // load cache data
  char cachename[256];
  sprintf (cachename, "%s_%s", GetName (), plugname);
  culler->Setup (cachename);

  // Loop through all meshes and register them to the visibility culler.
  int i;
  for (i = 0; i < meshes.GetCount (); i++)
  {
    iMeshWrapper* m = meshes.Get (i);
    m->GetMovable ()->UpdateMove ();
    RegisterMeshToCuller (m);
  }
  return true;
}

iVisibilityCuller* csSector::GetVisibilityCuller ()
{
  if (!culler) UseCullerPlugin ("crystalspace.culling.frustvis");
  CS_ASSERT (culler != NULL);
  return culler;
}

iPolygon3D *csSector::HitBeam (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect)
{
  iPolygon3D *p = IntersectSegment (start, end, isect);
  if (p)
  {
    iPortal *po = p->GetPortal ();
    if (po)
    {
      draw_busy++;

      csVector3 new_start = isect;
      p = po->HitBeam (new_start, end, isect);
      draw_busy--;
      return p;
    }
    else
      return p;
  }
  else
    return NULL;
}

csMeshWrapper *csSector::HitBeam (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  iPolygon3D **polygonPtr)
{
  GetVisibilityCuller ();
  float r;
  iMeshWrapper* mesh;
  iPolygon3D* poly;
  bool rc = culler->IntersectSegment (start, end, isect, &r, &mesh, &poly);
  if (polygonPtr) *polygonPtr = poly;
  if (rc && mesh)
    return mesh->GetPrivateObject ();
  else
    return NULL;
}

iPolygon3D *csSector::IntersectSegment (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr,
  bool only_portals,
  csMeshWrapper **p_mesh)
{
  float r, best_r = 10000000000.;
  csVector3 cur_isect;
  iPolygon3D *best_p = NULL;
  csVector3 obj_start, obj_end, obj_isect;

  if (!only_portals)
  {
    iMeshWrapper *mesh;
    iPolygon3D* poly;
    bool rc = culler->IntersectSegment (start, end, isect, &r, &mesh, &poly);
    if (rc && poly)
    {
      best_p = poly;
      best_r = r;
      if (p_mesh)
        *p_mesh = mesh ? mesh->GetPrivateObject () : NULL;
    }
    if (pr) *pr = best_r;
    return best_p;
  }

  csReversibleTransform movtrans;

  GetVisibilityCuller ();
  csBox3 b (start);
  b.AddBoundingVertexSmart (end);
  csRef<iVisibilityObjectIterator> visit = culler->VisTest (b);

  while (!visit->IsFinished ())
  {
    iVisibilityObject* vo = visit->GetObject ();
    visit->Next ();
    iMeshWrapper* mesh = vo->GetMeshWrapper ();
    if (!mesh || mesh->GetFlags ().Check (CS_ENTITY_INVISIBLE)) continue;

    bool has_not_moved = mesh->GetMovable ()->IsFullTransformIdentity ();

    csRef<iThingState> ith (SCF_QUERY_INTERFACE (
        mesh->GetMeshObject (),
        iThingState));
    if (ith)
    {
      r = best_r;

      if (has_not_moved)
      {
        obj_start = start;
        obj_end = end;
      }
      else
      {
        movtrans = mesh->GetMovable ()->GetFullTransform ();
        obj_start = movtrans.Other2This (start);
        obj_end = movtrans.Other2This (end);
      }

      iPolygon3D *p = ith->IntersectSegment (
          obj_start,
          obj_end,
          obj_isect,
          &r,
          only_portals);

      if (p && r < best_r)
      {
        if (has_not_moved)
          cur_isect = obj_isect;
        else
          cur_isect = movtrans.This2Other (obj_isect);
        best_r = r;
        best_p = p;
        isect = cur_isect;
        if (p_mesh) *p_mesh = mesh->GetPrivateObject ();
      }
    }
  }

  if (pr) *pr = best_r;
  return best_p;
}

csSector *csSector::FollowSegment (
  csReversibleTransform &t,
  csVector3 &new_position,
  bool &mirror,
  bool only_portals)
{
  csVector3 isect;
  iPolygon3D *p = IntersectSegment (
      t.GetOrigin (),
      new_position,
      isect,
      NULL,
      only_portals);
  iPortal *po;

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

      if (po->GetFlags ().Check (CS_PORTAL_WARP))
      {
        po->WarpSpace (t, mirror);
        new_position = po->Warp (new_position);
      }

      csSector *dest_sect = po->GetSector ()->GetPrivateObject ();
      return dest_sect->FollowSegment (t, new_position, mirror);
    }
    else
      new_position = isect;
  }

  return this;
}

iPolygon3D *csSector::IntersectSphere (
  csVector3 &center,
  float radius,
  float *pr)
{
  float min_d = radius;
  iPolygon3D *min_p = NULL;
  (void)center;
#if 0
  float d = .0f;
  iPolygon3D *p = NULL, *min_p = NULL;
  csVector3 obj_center;
  csReversibleTransform movtrans;

  //@@@ Support for meshes!!!
  int i;
  for (i = 0; i < things.Length (); i++)
  {
    //@@@ NOT VALID!!!
    iThingState *sp = (iThingState *)things[i];
    bool has_not_moved = sp->GetMovable ()->IsFullTransformIdentity ();
    if (has_not_moved)
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

void csSector::PrepareDraw (iRenderView *rview)
{
  draw_busy++;

  // Make sure the visibility culler is loaded.
  GetVisibilityCuller ();

  int i;
  //  iCamera *icam = rview->GetCamera ();
  rview->SetThisSector (&scfiSector);

  i = sector_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iSectorCallback* cb = sector_cb_vector.Get (i);
    cb->Traverse (&scfiSector, rview);
    i--;
  }

  // Here we check all render queues to see if there is a render queue
  // that has the 'do_camera' flag set. If so then we check all meshes
  // in that render queue to see if there is one that has CS_ENTITY_CAMERA
  // set. If so we move that mesh to the right position.
  for (i = 0 ; i < RenderQueues.GetQueueCount () ; i++)
  {
    if (csEngine::current_engine->GetRenderPriorityCamera (i))
    {
      csMeshVectorNodelete* mv = RenderQueues.GetQueue (i);
      if (mv)
      {
        int j;
	for (j = 0 ; j < mv->Length () ; j++)
	{
	  iMeshWrapper* m = mv->Get (j);
	  if (m->GetFlags ().Check (CS_ENTITY_CAMERA))
	  {
	    iMovable* mov = m->GetMovable ();
	    // Temporarily move the object to the current camera.
	    csReversibleTransform &mov_trans = mov->GetTransform ();
// @@@ TEMPORARY: now CS_ENTITY_CAMERA only works at 0,0,0 position.
mov_trans.SetOrigin (csVector3 (0));
//	    csVector3 old_movable_pos = mov_trans.GetOrigin ();
	    iCamera *orig_cam = rview->GetOriginalCamera ();
	    csOrthoTransform &orig_trans = orig_cam->GetTransform ();
	    csVector3 v = orig_trans.GetO2TTranslation ();
	    mov_trans.SetOrigin (mov_trans.GetOrigin () + v);
	    mov->UpdateMove ();
    //mov_trans.SetOrigin (old_movable_pos);
    //movable.UpdateMove ();
	  }
	}
      }
    }
  }
}

/*
 * @@@ THE NOTES BELOW ARE MOSTLY OBSOLETE NOW. I DON'T REMOVE THEM
 * BECAUSE THERE IS STILL A GRAIN OF USEFUL INFORMATION IN THEM.
 *
 * Some notes about drawing here. These notes are the start for
 * a rethinking about how rendering objects in one sector actually
 * should happen. Note that the current implementation actually
 * implements very little of the things discussed here. Currently
 * the entities are just rendered one after the other which can cause
 * some problems.
 *
 * There are a few issues here:
 *
 * 1. Z-buffering/Z-filling.
 * Some objects/entities are more efficiently rendered back
 * to front using Z-filling instead of Z-buffering. In some cases
 * Z-filling is also required because rendering a sector starts
 * with an uninitialized Z-buffer (CS normally doesn't clear the
 * Z buffer every frame). In some cases it might be more optimal
 * to use Z buffering in any case (to avoid sorting back to front)
 * (for hardware 3D) so we would like to have the option to clear
 * the Z buffer every frame and use Z-buffering.
 *
 * 2. Alpha transparency.
 * Some entities have alpha transparency. Alpha transparent surfaces
 * actually need to be sorted back to front to render correctly.
 * Also before rendering an alpha surface all objects behind it should
 * already be rendered.
 *
 * 3. Floating portals.
 * Floating portals also take some special consideration. First
 * of all the assume a new intialize of the Z buffer for the 2D
 * area of the portal in question. This is ok if the first entities
 * that are rendered through the portal use Z-fill and cover the
 * entire portal (this is the case if you use sector walls for
 * example). If Z-fill cannot be used for the portal then an
 * extra initial pass would have to clear the Z buffer for the portal
 * area in 2D. Also geometry needs to be clipped in 3D if you have
 * a floating portal. The reason is that the Z buffer information
 * outside of the floating portal may actually contain information
 * further than the contents of the portal. This would cause entities
 * visible inside the portal to be rendered as if they are in the
 * parent sector too.
 * After rendering through a floating portal, the floating portal
 * itself needs to be covered by the Z-buffer. i.e. we need to make
 * sure that the Z-buffer thinks the portal is a regular polygon.
 * This is to make sure that meshes or other entities rendered
 * afterwards will not get rendered INSIDE the portal contents.
 *
 * Here is a list of all the entities that we can draw in a sector:
 *
 * 1. Sector walls.
 * Sectors are always convex. So sectors walls are ideal for rendering
 * first through Z-fill.
 *
 * 2. Static things in octree.
 * In some cases all static things are collected into one big
 * octree with mini-bsp trees. This structure ensures that we can
 * actually easily sort polygon back to front or front to back if
 * needed. This structure can also easily be rendered using Z-fill.
 * The c-buffer/coverage mask tree can also be used to detect
 * visibility before rendering. This pushes visible polygons into
 * a queue. There is the issue here that it should be possible
 * to ignore the mini-bsp trees and only use the octree information.
 * This can be done on hardware where Z-buffering is fast. This
 * of course implies either the use of a Z-filled sector or else
 * a clear of the Z buffer every frame.
 * A related issue is when there are portals between the polygons.
 * Those portals need to be handled as floating portals (i.e. geometry
 * needs to be clipped in 3D) because the Z buffer information
 * will not be correct. If rendering the visible octree polygons
 * back to front then rendering through the portals presents no
 * other difficulties.
 *
 * 3. Terrain triangles.
 * The terrain engine generates a set of triangles. These triangles
 * can easily be sorted back to front so they are also suitable for
 * Z-fill rendering. However, this conflicts with the use of the
 * static octree. You cannot use Z-fill for both because that could
 * cause wrong rendering. Using Z-buffer for one of them might be
 * expensive but the only solution. Here there is also the issue
 * if it isn't possible to combine visibility algorithms for landscape
 * and octree stuff. i.e. cull octree nodes if occluded by a part
 * of the landscape.
 *
 * 4. 3D Sprites.
 * Sprites are entities that need to be rendered using the Z-buffer
 * because the triangles cannot easily be sorted.
 *
 * 5. Dynamic things.
 * Things that are not part of the static octree are handled much
 * like normal 3D sprites. The most important exception is when
 * such a thing has a floating portal. In this case all the normal
 * floating portal issues are valid. However, there are is an important
 * issue here: if you are rendering a floating portal that is BEHIND
 * an already rendered entity then there is a problem. The contents
 * of the portal may actually use Z-fill and thus would overrender
 * the entity in front. One obvious solution is to sort ALL entities
 * to make sure that everything is rendered back to front. That's of
 * course not always efficient and easy to do. Also it is not possible
 * in all cases to do it 100% correct (i.e. complex sprites with
 * skeletal animation and so on). The ideal solution would be to have
 * a way to clear the Z-buffer for an invisible polygon but only
 * where the polygon itself is visible according to the old Z-buffer
 * values. This is possible with software but I'm currently unsure
 * about hardware. With such a routine you could draw the floating
 * portal at any time you want. First you clear the Z-buffer for the
 * visible area. Then you force Z-buffer use for the contents inside
 * (i.e. everything normally rendered using Z-fill will use Z-buffer
 * instead), then you render. Finally you update the Z-buffer with
 * the Z-value of the polygon to make it 'hard'.
 *
 * If we can treat floating portals this way then we can in fact
 * consider them as normal polygons that behave correctly for the
 * Z buffer. Aside from the fact that they clip geometry in 3D
 * that passes through the portal. Note that 3D sprites don't
 * currently support 3D geometry clipping yet.
 */
void csSector::Draw (iRenderView *rview)
{
#ifndef CS_USE_NEW_RENDERER
  PrepareDraw (rview);
  iCamera *icam = rview->GetCamera ();
  int i;

  G3D_FOGMETHOD fogmethod = G3DFOGMETHOD_NONE;

  if (HasFog ())
  {
    if (
      (fogmethod = csEngine::current_engine->fogmethod)
		== G3DFOGMETHOD_VERTEX)
    {
      csFogInfo *fog_info = new csFogInfo ();
      fog_info->next = rview->GetFirstFogInfo ();

      iPolygon3D *ipoly3d = rview->GetPortalPolygon ();
      if (ipoly3d)
      {
        fog_info->incoming_plane = ipoly3d->GetCameraPlane ();
        fog_info->incoming_plane.Invert ();
        fog_info->has_incoming_plane = true;
      }
      else
        fog_info->has_incoming_plane = false;
      fog_info->fog = &GetFog ();
      fog_info->has_outgoing_plane = true;
      rview->SetFirstFogInfo (fog_info);
    }
    else if (fogmethod != G3DFOGMETHOD_NONE)
    {
      rview->GetGraphics3D ()->OpenFogObject (GetID (), &GetFog ());
    }
  }

  if (rview->AddedFogInfo ())
    rview->GetFirstFogInfo ()->has_outgoing_plane = false;
#endif // CS_USE_NEW_RENDERER

  /*
   * Draw meshes.
   * To correctly support meshes in multiple sectors we only draw a
   * mesh if the mesh is not in the sector we came from. If the
   * mesh is also present in the previous sector then we will still
   * draw it in any of the following cases:
   *    - the previous sector has fog
   *    - the portal we just came through has alpha transparency
   *    - the portal is a portal on a thing (i.e. a floating portal)
   *    - the portal does space warping
   * In those cases we draw the mesh anyway. @@@ Note that we should
   * draw it clipped (in 3D) to the portal polygon. This is currently not
   * done.
   */
  if (meshes.GetCount () > 0)
  {
#ifndef CS_USE_NEW_RENDERER
    // Mark visible objects.
    culler->VisTest (rview);
    uint32 current_visnr = culler->GetCurrentVisibilityNumber ();

    // get a pointer to the previous sector
    iSector *prev_sector = rview->GetPreviousSector ();

    // look if meshes from the previous sector should be drawn
    bool draw_prev_sector = false;

    if (prev_sector)
    {
      draw_prev_sector = prev_sector->HasFog () ||
        rview->GetPortalPolygon ()->IsTransparent () ||
        rview->GetPortalPolygon ()->GetPortal ()->GetFlags ().Check (
            CS_PORTAL_WARP);
    }

    // First sort everything based on render priority and return
    // a big list of visible objects. This will use the visibility
    // information calculated by VisTest() above.
    int num_objects;
    iMeshWrapper** objects = RenderQueues.SortAll (rview, num_objects,
		    current_visnr);

    // Draw the meshes.
    for (i = 0 ; i < num_objects ; i++)
    {
      iMeshWrapper* sp = objects[i];
      if (
            !prev_sector ||
            sp->GetMovable ()->GetSectors ()->Find (prev_sector) == -1)
      {
        // Mesh is not in the previous sector or there is no previous
        // sector.
        sp->Draw (rview);
      }
      else if (draw_prev_sector)
      {
        // @@@ Here we should draw clipped to the portal.
        sp->Draw (rview);
      }
    }

    delete[] objects;
#else

    csLightList alllights;
    csRef<iSectorList> secs = csEngine::current_engine->GetSectors ();
	int i;
    for (i = secs->GetCount () - 1; i >= 0; i --)
    {
      csRef<iLightList> seclights = secs->Get(i)->GetLights ();
      for (int j = seclights->GetCount() - 1; j >= 0; j --)
      {
	// Sphere check against rview before adding it.
	csSphere s = csSphere (seclights->Get(j)->GetCenter (), 
		  seclights->Get(j)->GetRadius());
        if (rview->TestBSphere (rview->GetCamera()->GetTransform(), s))
	      alllights.Add (seclights->Get(j));
        // else { light isn't visible anyway }
      }
    }

    r3d->EnableZOffset ();
    DrawZ (rview);
    r3d->DisableZOffset ();
    for (i = alllights.GetCount () - 1; i >= 0; i--) 
    {
      csColor color = alllights.Get (i)->GetColor ();
      float radius = alllights.Get (i)->GetRadius ();
      radius *= radius;
      r3d->SetObjectToCamera (&rview->GetCamera ()->GetTransform ());
      r3d->SetLightParameter (0, CS_LIGHTPARAM_POSITION,
      	alllights.Get (i)->GetCenter ());
      r3d->SetLightParameter (0, CS_LIGHTPARAM_DIFFUSE,
      	csVector3 (color.red, color.green, color.blue));
      r3d->SetLightParameter (0, CS_LIGHTPARAM_ATTENUATION,
      	csVector3 (0, 0, radius));
      r3d->DisableColorWrite ();
      r3d->SetShadowState (CS_SHADOW_VOLUME_BEGIN);
      DrawShadow (rview, alllights.Get (i));
      r3d->EnableColorWrite ();
      r3d->SetShadowState (CS_SHADOW_VOLUME_USE);
      DrawLight (rview, alllights.Get(i));
      r3d->SetShadowState (CS_SHADOW_VOLUME_FINISH);
    }

#endif
  }

#ifndef CS_USE_NEW_RENDERER
  // queue all halos in this sector to be drawn.
  for (i = lights.GetCount () - 1; i >= 0; i--)
    // Tell the engine to try to add this light into the halo queue
    csEngine::current_engine->AddHalo (lights.Get (i)->GetPrivateObject ());

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
        rview->GetGraphics3D ()->DrawFogPolygon (
              GetID (), g3dpoly, CS_FOG_VIEW);
      }
      else
      {
        // We must add a FRONT fog polygon for the clipper to this sector.
        rview->GetClipPlane (g3dpoly.normal);
        g3dpoly.normal.Invert ();
        rview->GetGraphics3D ()->DrawFogPolygon (
            GetID (), g3dpoly, CS_FOG_FRONT);
      }
    }
    else if (fogmethod == G3DFOGMETHOD_VERTEX && rview->AddedFogInfo ())
    {
      csFogInfo *fog_info = rview->GetFirstFogInfo ();
      rview->SetFirstFogInfo (rview->GetFirstFogInfo ()->next);
      delete fog_info;
    }
  }

  draw_busy--;
#endif // CS_USE_NEW_RENDERER
}

#ifdef CS_USE_NEW_RENDERER
void csSector::DrawZ (iRenderView* rview)
{
  PrepareDraw (rview);
  iCamera *icam = rview->GetCamera ();
  int i;

  // Mark visible objects.
  culler->VisTest (rview);
  uint32 current_visnr = culler->GetCurrentVisibilityNumber ();

  // get a pointer to the previous sector
  iSector *prev_sector = rview->GetPreviousSector ();

  // look if meshes from the previous sector should be drawn
  bool draw_prev_sector = false;

  if (prev_sector)
  {
    draw_prev_sector = 
      rview->GetPortalPolygon ()->IsTransparent () ||
      rview->GetPortalPolygon ()->GetPortal ()->GetFlags ().Check (
          CS_PORTAL_WARP);
  }

  objects = RenderQueues.SortAll (rview, num_objects, current_visnr);
  for (i = 0; i < num_objects; i ++)
  {
    iMeshWrapper *sp = objects[i];
    if (
          !prev_sector ||
          sp->GetMovable ()->GetSectors ()->Find (prev_sector) == -1 ||
          draw_prev_sector)
    {
      // Mesh is not in the previous sector or there is no previous
      // sector.
      sp->DrawZ (rview);
    }
    else
    {
      objects[i] = NULL;
    }
  }
}

void csSector::DrawShadow (iRenderView* rview, iLight* light)
{
  for (int i = 0; i < num_objects; i ++) {
    iMeshWrapper *sp = objects[i];
    if (sp) 
    {
      sp->DrawShadow (rview, light);
    }
  }
}

void csSector::DrawLight (iRenderView* rview, iLight* light)
{
  int i;
  for (i = 0; i < num_objects; i ++) {
    iMeshWrapper *sp = objects[i];
    if (sp)
    {
      sp->DrawLight (rview, light);
    }
  }
  //delete [] objects;
   
  // queue all halos in this sector to be drawn.
  for (i = lights.GetCount () - 1; i >= 0; i--)
    // Tell the engine to try to add this light into the halo queue
    csEngine::current_engine->AddHalo (lights.Get (i)->GetPrivateObject ());

  draw_busy --;
}
#endif // CS_USE_NEW_RENDERER

void csSector::CheckFrustum (iFrustumView *lview)
{
  int i = sector_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iSectorCallback* cb = sector_cb_vector.Get (i);
    cb->Traverse (&scfiSector, lview);
    i--;
  }

  RealCheckFrustum (lview);
}

void csSector::RealCheckFrustum (iFrustumView *lview)
{
  if (draw_busy > cfg_reflections) return ;
  draw_busy++;

  // Make sure we have a culler.
  GetVisibilityCuller ();
  culler->CastShadows (lview);

  draw_busy--;
}

void csSector::ShineLights (csProgressPulse *pulse)
{
  int i;
  for (i = 0; i < lights.GetCount (); i++)
  {
    if (pulse != 0) pulse->Step ();

    csLight *cl = lights.Get (i)->GetPrivateObject ();
    ((csStatLight *)cl)->CalculateLighting ();
  }
}

void csSector::ShineLights (iMeshWrapper *mesh, csProgressPulse *pulse)
{
  int i;
  for (i = 0; i < lights.GetCount (); i++)
  {
    if (pulse != 0) pulse->Step ();

    csLight *cl = lights.Get (i)->GetPrivateObject ();
    ((csStatLight *)cl)->CalculateLighting (mesh);
  }
}

void csSector::SetDynamicAmbientLight (const csColor& color)
{
  iMeshList* ml = GetMeshes ();
  dynamic_ambient_color = color;
  for (int i = 0 ; i < ml->GetCount () ; i++)
  {
    iMeshWrapper* mesh = ml->Get (i);
    iLightingInfo* li = mesh->GetLightingInfo ();
    if (li)
      li->SetDynamicAmbientLight (color);
  }
}

void csSector::CalculateSectorBBox (csBox3 &bbox, bool do_meshes) const
{
  bbox.StartBoundingBox ();

  csBox3 b;
  int i;
  if (do_meshes)
  {
    for (i = 0; i < meshes.GetCount (); i++)
    {
      iMeshWrapper *mesh = meshes.Get (i);
      mesh->GetTransformedBoundingBox (
          mesh->GetMovable ()->GetTransform (),
          b);
      bbox += b;
    }
  }
}

//---------------------------------------------------------------------------
iPolygon3D *csSector::eiSector::HitBeam (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect)
{
  return scfParent->HitBeam (start, end, isect);
}

iMeshWrapper *csSector::eiSector::HitBeam (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  iPolygon3D **polygonPtr)
{
  iPolygon3D *p = NULL;
  csMeshWrapper *obj = scfParent->HitBeam (
      start,
      end,
      isect,
      polygonPtr ? &p : NULL);
  if (obj)
  {
    if (p)
    {
      *polygonPtr = p;
    }
  }

  return obj ? &(obj->scfiMeshWrapper) : NULL;
}

iSector *csSector::eiSector::FollowSegment (
  csReversibleTransform &t,
  csVector3 &new_position,
  bool &mirror,
  bool only_portals)
{
  csSector *s = scfParent->FollowSegment (
      t,
      new_position,
      mirror,
      only_portals);
  if (s)
    return &(s->scfiSector);
  else
    return NULL;
}

void csSector::ReferencedObject::AddReference (iReference *ref)
{
  scfParent->references.Push (ref);
}

void csSector::ReferencedObject::RemoveReference (iReference *ref)
{
  int i;

  // We scan backwards because we know that the code to remove all
  // refs to a sector will also scan backwards. So this is more efficient.
  for (i = scfParent->references.Length () - 1; i >= 0; i--)
  {
    iReference *r = (iReference *) (scfParent->references[i]);
    if (r == ref)
    {
      scfParent->references.Delete (i);
      return ;
    }
  }

  // Hopefully we never come here.
  CS_ASSERT (false);
}

SCF_IMPLEMENT_IBASE(csSectorList)
  SCF_IMPLEMENTS_INTERFACE(iSectorList)
SCF_IMPLEMENT_IBASE_END

csSectorList::csSectorList (bool cr)
{
  SCF_CONSTRUCT_IBASE (NULL);

  CleanupReferences = cr;
}

void csSectorList::FreeItem (iSector* item)
{
  if (CleanupReferences)
    item->GetPrivateObject ()->CleanupReferences ();
}

int csSectorList::Add (iSector *obj)
{
  return list.Push (obj);
}

bool csSectorList::Remove (iSector *obj)
{
  FreeItem (obj);
  return list.Delete (obj);
}

bool csSectorList::Remove (int n)
{
  FreeItem (list[n]);
  return list.Delete (n);
}

void csSectorList::RemoveAll ()
{
  int i;
  for (i = 0 ; i < list.Length () ; i++)
  {
    FreeItem (list[i]);
  }
  list.DeleteAll ();
}

int csSectorList::Find (iSector *obj) const
{
  return list.Find (obj);
}

iSector *csSectorList::FindByName (const char *Name) const
{
  return list.FindByName (Name);
}

