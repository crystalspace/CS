/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "qsqrt.h"
#include "csgeom/sphere.h"
#include "csengine/sector.h"
#include "csengine/meshobj.h"
#include "csengine/light.h"
#include "csengine/engine.h"
#include "csengine/thing.h"
#include "csutil/debug.h"
#include "ivideo/graph3d.h"
#include "iengine/rview.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/portal.h"

// ---------------------------------------------------------------------------
// csMeshWrapper
// ---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT (csMeshWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMeshWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iVisibilityObject)
  SCF_IMPLEMENTS_INTERFACE (csMeshWrapper)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMeshWrapper::MeshWrapper)
  SCF_IMPLEMENTS_INTERFACE (iMeshWrapper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMeshWrapper::VisObject)
  SCF_IMPLEMENTS_INTERFACE (iVisibilityObject)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMeshWrapper::csMeshWrapper (iMeshWrapper *theParent, iMeshObject* mesh)
	: csObject ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshWrapper);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiVisibilityObject);
  DG_TYPE (this, "csMeshWrapper");

  movable.scfParent = this;
  defered_num_lights = 0;
  defered_lighting_flags = 0;
  last_anim_time = 0;
  is_visible = false;
  wor_bbox_movablenr = -1;
  Parent = theParent;
  movable.SetMeshWrapper (this);
  if (Parent) movable.SetParent (Parent->GetMovable ());

  csEngine::current_engine->AddToCurrentRegion (this);
  csMeshWrapper::mesh = mesh;
  mesh->IncRef ();
  draw_cb = NULL;
  factory = NULL;
  zbufMode = CS_ZBUF_USE;
  render_priority = csEngine::current_engine->GetObjectRenderPriority ();
  children.SetMesh (this);
}

csMeshWrapper::csMeshWrapper (iMeshWrapper *theParent)
	: csObject ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshWrapper);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiVisibilityObject);
  DG_TYPE (this, "csMeshWrapper");

  movable.scfParent = this;
  defered_num_lights = 0;
  defered_lighting_flags = 0;
  last_anim_time = 0;
  is_visible = false;
  wor_bbox_movablenr = -1;
  Parent = theParent;
  movable.SetMeshWrapper (this);
  if (Parent) movable.SetParent (Parent->GetMovable ());

  csEngine::current_engine->AddToCurrentRegion (this);
  csMeshWrapper::mesh = NULL;
  draw_cb = NULL;
  factory = NULL;
  zbufMode = CS_ZBUF_USE;
  render_priority = csEngine::current_engine->GetObjectRenderPriority ();
  children.SetMesh (this);
}

void csMeshWrapper::SetMeshObject (iMeshObject* mesh)
{
  if (mesh) mesh->IncRef ();
  if (csMeshWrapper::mesh) csMeshWrapper::mesh->DecRef ();
  csMeshWrapper::mesh = mesh;
}

csMeshWrapper::~csMeshWrapper ()
{
  if (draw_cb) draw_cb->DecRef ();
  if (mesh) mesh->DecRef ();
}

void csMeshWrapper::UpdateMove ()
{
  int i;
  for (i = 0 ; i < children.Length () ; i++)
  {
    iMeshWrapper* spr = children.Get (i);
    spr->GetMovable ()->UpdateMove ();
  }
}

void csMeshWrapper::MoveToSector (csSector* s)
{
  // Only add this mesh to a sector if the parent is the engine.
  // Otherwise we have a hierarchical object and in that case
  // the parent object controls this.
  if (!Parent) s->GetMeshes ()->Add (&scfiMeshWrapper);
}

void csMeshWrapper::RemoveFromSectors ()
{
  if (Parent) return;

  int i;
  const iSectorList *sectors = movable.GetSectors ();
  for (i = 0 ; i < sectors->GetCount () ; i++)
  {
    iSector* ss = sectors->Get (i);
    if (ss)
      ss->GetMeshes ()->Remove (&scfiMeshWrapper);
  }
}

void csMeshWrapper::SetRenderPriority (long rp)
{
  render_priority = rp;

  if (Parent) return;

  int i;
  const iSectorList *sectors = movable.GetSectors ();
  for (i = 0 ; i < sectors->GetCount () ; i++)
  {
    iSector* ss = sectors->Get (i);
    if (ss) ss->GetPrivateObject ()->RelinkMesh (&scfiMeshWrapper);
  }
}

/// The list of lights that hit the mesh
static CS_DECLARE_GROWING_ARRAY_REF (light_worktable, iLight*);

void csMeshWrapper::UpdateDeferedLighting (const csVector3& pos)
{
  const iSectorList* movable_sectors = movable.GetSectors ();
  if (defered_num_lights && movable_sectors->GetCount () > 0)
  {
    if (defered_num_lights > light_worktable.Limit ())
      light_worktable.SetLimit (defered_num_lights);

    iSector* sect = movable_sectors->Get (0);
    int num_lights = csEngine::current_iengine->GetNearbyLights (
      sect, pos, defered_lighting_flags,
      light_worktable.GetArray (), defered_num_lights);
    UpdateLighting (light_worktable.GetArray (), num_lights);
  }
}

void csMeshWrapper::DeferUpdateLighting (int flags, int num_lights)
{
  defered_num_lights = num_lights;
  defered_lighting_flags = flags;
}

void csMeshWrapper::Draw (iRenderView* rview)
{
  if (flags.Check (CS_ENTITY_INVISIBLE)) return;
  if (flags.Check (CS_ENTITY_CAMERA))
  {
    // Temporarily move the object to the camera.
    csReversibleTransform& mov_trans = movable.GetTransform ();
    csVector3 old_movable_pos = mov_trans.GetOrigin ();
    iCamera* orig_cam = rview->GetOriginalCamera ();
    csOrthoTransform& orig_trans = orig_cam->GetTransform ();
    csVector3 v = orig_trans.GetO2TTranslation ();
    mov_trans.SetOrigin (mov_trans.GetOrigin ()+v);
    movable.UpdateMove ();
    DrawInt (rview);
    mov_trans.SetOrigin (old_movable_pos);
    movable.UpdateMove ();
  }
  else DrawInt (rview);
}

void csMeshWrapper::DrawInt (iRenderView* rview)
{
  iMeshWrapper* meshwrap = &scfiMeshWrapper;
  if (rview->GetCallback ())
  {
    rview->CallCallback (CS_CALLBACK_MESH, (void*)&scfiMeshWrapper);
  }
  if (draw_cb) if (!draw_cb->BeforeDrawing (meshwrap, rview)) return;

  if (mesh->DrawTest (rview, &movable.scfiMovable))
  {
    if (rview->GetCallback ())
    {
      rview->CallCallback (CS_CALLBACK_VISMESH, (void*)&scfiMeshWrapper);
    }
    else
    {
      csTicks lt = csEngine::current_engine->GetLastAnimationTime ();
      if (lt != 0)
      {
        if (lt != last_anim_time)
        {
          mesh->NextFrame (lt);
          last_anim_time = lt;
        }
      }
      UpdateDeferedLighting (movable.GetFullPosition ());
      mesh->Draw (rview, &movable.scfiMovable, zbufMode);
    }
  }
  int i;
  for (i = 0 ; i < children.Length () ; i++)
  {
    iMeshWrapper* spr = children.Get (i);
    spr->Draw (rview);
  }
}

void csMeshWrapper::UpdateLighting (iLight** lights, int num_lights)
{
  defered_num_lights = 0;
  //if (num_lights <= 0) return;
  mesh->UpdateLighting (lights, num_lights, &movable.scfiMovable);

  int i;
  for (i = 0 ; i < children.Length () ; i++)
  {
    iMeshWrapper* spr = children.Get (i);
    spr->UpdateLighting (lights, num_lights);
  }
}

void csMeshWrapper::PlaceMesh ()
{
  iSectorList* movable_sectors = movable.GetSectors ();
  if (movable_sectors->GetCount () == 0) return;	// Do nothing

  csSphere sphere;
  csVector3 radius;
  mesh->GetRadius (radius, sphere.GetCenter ());
  iSector* sector = movable_sectors->Get (0);
  movable.SetSector (sector);	// Make sure all other sectors are removed

  // Transform the sphere from object to world space.
  float max_radius = radius.x;
  if (max_radius < radius.y) max_radius = radius.y;
  if (max_radius < radius.z) max_radius = radius.z;
  sphere.SetRadius (max_radius);
  sphere = movable.GetFullTransform ().This2Other (sphere);
  max_radius = sphere.GetRadius ();
  float max_sq_radius = max_radius * max_radius;

  // @@@ This function only goes one level deep in portals. Should be fixed!
  // @@@ It would be nice if we could find a more optimal portal representation
  // for large sectors.
  int i, j;
  iMeshList* ml = sector->GetMeshes ();
  for (i = 0 ; i < ml->GetCount () ; i++)
  {
    iMeshWrapper* mesh = ml->Get (i);
    iThingState* thing = SCF_QUERY_INTERFACE_FAST (mesh->GetMeshObject (),
	iThingState);
    if (thing)
    {
      // @@@ This function will currently only consider portals on things
      // that cannot move.
      if (thing->GetMovingOption () == CS_THING_MOVE_NEVER)
      {
        for (j = 0 ; j < thing->GetPortalCount () ; j++)
        {
	  iPortal* portal = thing->GetPortal (j);
	  iSector* dest_sector = portal->GetSector ();
	  if (movable_sectors->Find (dest_sector) == -1)
	  {
	    iPolygon3D* portal_poly = thing->GetPortalPolygon (j);
	    const csPlane3& pl = portal_poly->GetWorldPlane ();

	    float sqdist = csSquaredDist::PointPlane (sphere.GetCenter (), pl);
	    if (sqdist <= max_sq_radius)
	    {
	      // Plane of portal is close enough.
	      // If N is the normal of the portal plane then we
	      // can use that to calculate the point on the portal plane.
	      csVector3 testpoint = sphere.GetCenter ()
	    	  + pl.Normal ()*qsqrt (sqdist);
	      if (portal_poly->PointOnPolygon (testpoint))
	        movable_sectors->Add (dest_sector);
	    }
	  }
        }
      }
      thing->DecRef ();
    }
  }
}

int csMeshWrapper::HitBeamBBox (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
{
  csBox3 b;
  mesh->GetObjectBoundingBox (b, CS_BBOX_MAX);
  csSegment3 seg (start, end);
  return csIntersect3::BoxSegment (b, seg, isect, pr);
}

bool csMeshWrapper::HitBeamOutline (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
{
  return mesh->HitBeamOutline (start, end, isect, pr);
}

bool csMeshWrapper::HitBeamObject (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
{
  return mesh->HitBeamObject (start, end, isect, pr);
}

bool csMeshWrapper::HitBeam (const csVector3& start, const csVector3& end,
	csVector3& isect, float* pr)
{
  csReversibleTransform trans = movable.GetFullTransform ();
  csVector3 startObj = trans.Other2This (start);
  csVector3 endObj = trans.Other2This (end);
  bool rc = false;
  if ((HitBeamBBox(startObj, endObj, isect, NULL) > -1)
       && (rc = HitBeamOutline (startObj, endObj, isect, pr)))
          isect = trans.This2Other (isect);
  return rc;
}

void csMeshWrapper::HardTransform (const csReversibleTransform& t)
{
  mesh->HardTransform (t);
  int i;
  for (i = 0 ; i < children.Length () ; i++)
  {
    iMeshWrapper* spr = children.Get (i);
    spr->HardTransform (t);
  }
}

void csMeshWrapper::GetWorldBoundingBox (csBox3& cbox)
{
  if (wor_bbox_movablenr != movable.GetUpdateNumber ())
  {
    wor_bbox_movablenr = movable.GetUpdateNumber ();
    csBox3 obj_bbox;
    mesh->GetObjectBoundingBox (obj_bbox, CS_BBOX_MAX);
    // @@@ Maybe it would be better to really calculate the bounding box
    // here instead of just transforming the object space bounding box?
    csReversibleTransform mt = movable.GetFullTransform ();
    wor_bbox.StartBoundingBox (mt.This2Other (obj_bbox.GetCorner (0)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (1)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (2)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (3)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (4)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (5)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (6)));
    wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (7)));
  }
  cbox = wor_bbox;
}

void csMeshWrapper::GetRadius (csVector3& rad, csVector3& cent) const
{
  mesh->GetRadius (rad, cent);
  if (children.Length () > 0)
  {
    float max_radius = rad.x;
    if (max_radius < rad.y) max_radius = rad.y;
    if (max_radius < rad.z) max_radius = rad.z;
    csSphere sphere (cent, max_radius);
    int i;
    for (i = 0 ; i < children.Length () ; i++)
    {
      iMeshWrapper* spr = children.Get (i);
      csVector3 childrad, childcent;
      spr->GetRadius (childrad, childcent);
      float child_max_radius = childrad.x;
      if (child_max_radius < childrad.y) child_max_radius = childrad.y;
      if (child_max_radius < childrad.z) child_max_radius = childrad.z;
      csSphere childsphere (childcent, child_max_radius);
      // @@@ Is this the right transform?
      childsphere *= spr->GetMovable ()->GetTransform ();
      sphere += childsphere;
    }
    rad.Set (sphere.GetRadius (), sphere.GetRadius (), sphere.GetRadius ());
    cent.Set (sphere.GetCenter ());
  }
}

void csMeshWrapper::GetTransformedBoundingBox (
	const csReversibleTransform& trans, csBox3& cbox)
{
  csBox3 box;
  mesh->GetObjectBoundingBox (box);
  cbox.StartBoundingBox (trans * box.GetCorner (0));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (1));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (2));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (3));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (4));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (5));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (6));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (7));
}

float csMeshWrapper::GetScreenBoundingBox (
	const iCamera *camera, csBox2& sbox, csBox3& cbox)
{
  csVector2 oneCorner;
  csReversibleTransform tr_o2c = camera->GetTransform ()
    	* movable.GetFullTransform ().GetInverse ();
  GetTransformedBoundingBox (tr_o2c, cbox);

  // if the entire bounding box is behind the camera, we're done
  if ((cbox.MinZ () < 0) && (cbox.MaxZ () < 0))
  {
    return -1;
  }

  // Transform from camera to screen space.
  if (cbox.MinZ () <= 0)
  {
    // Mesh is very close to camera.
    // Just return a maximum bounding box.
    sbox.Set (-10000, -10000, 10000, 10000);
  }
  else
  {
    camera->Perspective (cbox.Max (), oneCorner);
    sbox.StartBoundingBox (oneCorner);
    csVector3 v (cbox.MinX (), cbox.MinY (), cbox.MaxZ ());
    camera->Perspective (v, oneCorner);
    sbox.AddBoundingVertexSmart (oneCorner);
    camera->Perspective (cbox.Min (), oneCorner);
    sbox.AddBoundingVertexSmart (oneCorner);
    v.Set (cbox.MaxX (), cbox.MaxY (), cbox.MinZ ());
    camera->Perspective (v, oneCorner);
    sbox.AddBoundingVertexSmart (oneCorner);
  }

  return cbox.MaxZ ();
}

float csMeshWrapper::MeshWrapper::GetScreenBoundingBox (iCamera* camera,
	csBox2& sbox, csBox3& cbox)
{
  return scfParent->GetScreenBoundingBox (camera, sbox, cbox);
}

// ---------------------------------------------------------------------------
// csMeshFactoryWrapper
// ---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT (csMeshFactoryWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMeshFactoryWrapper)
  SCF_IMPLEMENTS_INTERFACE (csMeshFactoryWrapper)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMeshFactoryWrapper::MeshFactoryWrapper)
  SCF_IMPLEMENTS_INTERFACE (iMeshFactoryWrapper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMeshFactoryWrapper::csMeshFactoryWrapper (iMeshObjectFactory* meshFact)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
  csMeshFactoryWrapper::meshFact = meshFact;
  meshFact->IncRef ();
  parent = NULL;
  children.SetMeshFactory (this);
  csEngine::current_engine->AddToCurrentRegion (this);
}

csMeshFactoryWrapper::csMeshFactoryWrapper ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
  csMeshFactoryWrapper::meshFact = NULL;
  parent = NULL;
  children.SetMeshFactory (this);
  csEngine::current_engine->AddToCurrentRegion (this);
}

csMeshFactoryWrapper::~csMeshFactoryWrapper ()
{
  if (meshFact) meshFact->DecRef ();
}

void csMeshFactoryWrapper::SetMeshObjectFactory (iMeshObjectFactory* meshFact)
{
  if (meshFact) meshFact->IncRef ();
  if (csMeshFactoryWrapper::meshFact)
    csMeshFactoryWrapper::meshFact->DecRef ();
  csMeshFactoryWrapper::meshFact = meshFact;
}

iMeshWrapper* csMeshFactoryWrapper::NewMeshObject ()
{
  iMeshObject* basemesh = meshFact->NewInstance ();
  iMeshWrapper* mesh = &(new csMeshWrapper (NULL, basemesh))->scfiMeshWrapper;
  basemesh->DecRef ();

  if (GetName ()) mesh->QueryObject ()->SetName (GetName ());
  mesh->SetFactory (&scfiMeshFactoryWrapper);

  int i;
  for (i = 0 ; i < children.Length () ; i++)
  {
    iMeshFactoryWrapper *childfact = children.Get (i);
    iMeshWrapper *child = childfact->CreateMeshWrapper ();
    mesh->GetChildren ()->Add (child);
    child->GetMovable ()->SetTransform (childfact->GetTransform ());
    child->GetMovable ()->UpdateMove ();
    child->DecRef ();
  }
  return mesh;
}

void csMeshFactoryWrapper::HardTransform (const csReversibleTransform& t)
{
  meshFact->HardTransform (t);
}

//--------------------------------------------------------------------------
// csMeshList
//--------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csMeshList)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMeshList)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMeshList::MeshList)
  SCF_IMPLEMENTS_INTERFACE (iMeshList)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMeshList::csMeshList ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshList);
}

int csMeshList::MeshList::GetCount () const
  { return scfParent->Length (); }
iMeshWrapper *csMeshList::MeshList::Get (int n) const
  { return scfParent->Get (n); }
int csMeshList::MeshList::Add (iMeshWrapper *obj)
  { scfParent->Push (obj); return true; }
bool csMeshList::MeshList::Remove (iMeshWrapper *obj)
  { scfParent->Delete (obj); return true; }
bool csMeshList::MeshList::Remove (int n)
  { scfParent->Delete (n); return true; }
void csMeshList::MeshList::RemoveAll ()
  { scfParent->DeleteAll (); }
int csMeshList::MeshList::Find (iMeshWrapper *obj) const
  { return scfParent->Find (obj); }
iMeshWrapper *csMeshList::MeshList::FindByName (const char *Name) const
  { return scfParent->FindByName (Name); }

//--------------------------------------------------------------------------
// csMeshMeshList
//--------------------------------------------------------------------------

bool csMeshMeshList::PrepareItem (csSome item)
{
  CS_ASSERT (mesh != NULL);
  csMeshList::PrepareItem (item);
  iMeshWrapper *child = (iMeshWrapper*)item;

  // unlink the mesh from the engine or another parent.
  iMeshWrapper *oldParent = child->GetParentContainer ();
  if (oldParent)
    oldParent->GetChildren ()->Remove (child);
  else
    csEngine::current_engine->GetMeshes ()->Remove (child);

  child->SetParentContainer (&mesh->scfiMeshWrapper);
  child->GetMovable ()->SetParent (&mesh->GetMovable ().scfiMovable);

  return true;
}

bool csMeshMeshList::FreeItem (csSome item)
{
  CS_ASSERT (mesh != NULL);
  ((iMeshWrapper*)item)->SetParentContainer (NULL);
  ((iMeshWrapper*)item)->GetMovable ()->SetParent (NULL);
  csMeshList::FreeItem (item);
  return true;
}

//--------------------------------------------------------------------------
// csMeshFactoryList
//--------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csMeshFactoryList)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMeshFactoryList)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMeshFactoryList::MeshFactoryList)
  SCF_IMPLEMENTS_INTERFACE (iMeshFactoryList)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMeshFactoryList::csMeshFactoryList ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryList);
}

int csMeshFactoryList::MeshFactoryList::GetCount () const
  { return scfParent->Length (); }
iMeshFactoryWrapper *csMeshFactoryList::MeshFactoryList::Get (int n) const
  { return scfParent->Get (n); }
int csMeshFactoryList::MeshFactoryList::Add (iMeshFactoryWrapper *obj)
  { scfParent->Push (obj); return true; }
bool csMeshFactoryList::MeshFactoryList::Remove (iMeshFactoryWrapper *obj)
  { scfParent->Delete (obj); return true; }
bool csMeshFactoryList::MeshFactoryList::Remove (int n)
  { scfParent->Delete (scfParent->Get (n)); return true; }
void csMeshFactoryList::MeshFactoryList::RemoveAll ()
  { scfParent->DeleteAll (); }
int csMeshFactoryList::MeshFactoryList::Find (iMeshFactoryWrapper *obj) const
  { return scfParent->Find (obj); }
iMeshFactoryWrapper *csMeshFactoryList::MeshFactoryList::FindByName (const char *Name) const
  { return scfParent->FindByName (Name); }

//--------------------------------------------------------------------------
// csMeshFactoryFactoryList
//--------------------------------------------------------------------------

bool csMeshFactoryFactoryList::PrepareItem (csSome item)
{
  CS_ASSERT (meshfact != NULL);
  csMeshFactoryList::PrepareItem (item);
  iMeshFactoryWrapper *child = (iMeshFactoryWrapper*)item;

  // unlink the factory from another possible parent.
  if (child->GetParentContainer ())
    child->GetParentContainer ()->GetChildren ()->Remove (child);

  child->SetParentContainer (&meshfact->scfiMeshFactoryWrapper);
  return true;
}

bool csMeshFactoryFactoryList::FreeItem (csSome item)
{
  CS_ASSERT (meshfact != NULL);
  ((iMeshFactoryWrapper*)item)->SetParentContainer (NULL);
  csMeshFactoryList::FreeItem (item);
  return true;
}
