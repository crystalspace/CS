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
#include "csgeom/sphere.h"
#include "csengine/sector.h"
#include "csengine/meshobj.h"
#include "csengine/light.h"
#include "csengine/engine.h"
#include "csengine/thing.h"
#include "ivideo/graph3d.h"
#include "iengine/rview.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/portal.h"

SCF_IMPLEMENT_IBASE_EXT_QUERY (csMeshWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMeshWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iVisibilityObject)
  SCF_IMPLEMENTS_INTERFACE (csMeshWrapper)
SCF_IMPLEMENT_IBASE_EXT_QUERY_END

SCF_IMPLEMENT_IBASE_EXT_INCREF(csMeshWrapper)
SCF_IMPLEMENT_IBASE_EXT_GETREFCOUNT(csMeshWrapper)

// We implement a custom DecRef() in order to work around a shortcoming of the
// NextStep compiler.  The UnlinkMesh(this) invocation which appears here used
// to appear in the destructor of this class.  During the processing of
// UnlinkMesh(), QueryInterface(iMeshWrapper) is invoked on this object.
// Unfortunately, the NextStep compiler modifies the `vptr' of this object to
// point at its superclass' `vtbl' as soon as the destructor is entered, rather
// than modifying it after the destructor has completed, which is how all other
// compilers behave.  This early vptr modification, thus transmogrifies this
// object into its superclass (csObject) too early; before
// QueryInterface(iMeshWrapper) is invoked.  As a result, by the time
// UnlinkMesh(this) was being called, the object already appeared to be a
// csObject and failed to respond positively to QueryInterface(iMeshWrapper).
// To work around this problem, the UnlinkMesh() invocation was moved out of
// the destructor and into DecRef(), thus it is now called prior to the
// undesirable transmogrification.  Note that the csMeshWrapper destructor is
// now private, thus it is ensured that terrain wrappers can only be destroyed
// via DecRef(), which is public.

void csMeshWrapper::DecRef()
{
  CS_ASSERT (scfRefCount > 0);
#ifdef CS_DEBUG
  if (scfRefCount == 1)
  {
    int idx = csEngine::current_engine->GetMeshes ()->Find (&scfiMeshWrapper);
    CS_ASSERT (idx == -1);
  }
#endif
  __scf_superclass::DecRef();
}

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
  if (!Parent) s->AddMesh (this);
}

void csMeshWrapper::RemoveFromSectors ()
{
  if (Parent) return;

  int i;
  const iSectorList *sectors = movable.GetSectors ();
  for (i = 0 ; i < sectors->GetSectorCount () ; i++)
  {
    iSector* ss = sectors->GetSector (i);
    if (ss)
      ss->GetMeshes ()->RemoveMesh (&scfiMeshWrapper);
  }
}

void csMeshWrapper::SetRenderPriority (long rp)
{
  render_priority = rp;

  if (Parent) return;

  int i;
  const iSectorList *sectors = movable.GetSectors ();
  for (i = 0 ; i < sectors->GetSectorCount () ; i++)
  {
    iSector* ss = sectors->GetSector (i);
    if (ss) ss->GetPrivateObject ()->RelinkMesh (this);
  }
}

/// The list of lights that hit the mesh
static CS_DECLARE_GROWING_ARRAY_REF (light_worktable, iLight*);

void csMeshWrapper::UpdateDeferedLighting (const csVector3& pos)
{
  const iSectorList* movable_sectors = movable.GetSectors ();
  if (defered_num_lights && movable_sectors->GetSectorCount () > 0)
  {
    if (defered_num_lights > light_worktable.Limit ())
      light_worktable.SetLimit (defered_num_lights);

    iSector* sect = movable_sectors->GetSector (0);
    int num_lights = csEngine::current_engine->GetNearbyLights (
      sect->GetPrivateObject (),
      pos, defered_lighting_flags,
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
    csOrthoTransform& trans = rview->GetCamera ()->GetTransform ();
    csVector3 old = trans.GetO2TTranslation ();
    trans.SetO2TTranslation (csVector3 (0));
    DrawInt (rview);
    trans.SetO2TTranslation (old);
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
  if (movable_sectors->GetSectorCount () == 0) return;	// Do nothing

  csSphere sphere;
  csVector3 radius;
  mesh->GetRadius (radius, sphere.GetCenter ());
  iSector* sector = movable_sectors->GetSector (0);
  movable.SetSector (sector);	// Make sure all other sectors are removed

  // Transform the sphere from object to world space.
  float max_radius = radius.x;
  if (max_radius < radius.y) max_radius = radius.y;
  if (max_radius < radius.z) max_radius = radius.z;
  sphere.SetRadius (max_radius);
  sphere *= movable.GetFullTransform ();
  max_radius = sphere.GetRadius ();
  float max_sq_radius = max_radius * max_radius;

  // @@@ This function is currently ignoring children but that's
  // not good!
  // @@@ This function only goes one level deep in portals. Should be fixed!
  // @@@ It would be nice if we could find a more optimal portal representation
  // for large sectors.
  int i, j;
  iMeshList* ml = sector->GetMeshes ();
  for (i = 0 ; i < ml->GetMeshCount () ; i++)
  {
    iMeshWrapper* mesh = ml->GetMesh (i);
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
	  iPolygon3D* portal_poly = thing->GetPortalPolygon (j);
	  const csPlane3& pl = portal_poly->GetWorldPlane ();

	  float sqdist = csSquaredDist::PointPlane (sphere.GetCenter (), pl);
	  if (sqdist <= max_sq_radius)
	  {
	    // Plane of portal is close enough.
	    // If N is the normal of the portal plane then center-N
	    // will be the point on the plane that is closest to 'center'.
	    // We check if that point is inside the portal polygon.
	    if (portal_poly->PointOnPolygon (sphere.GetCenter ()
	    	- pl.Normal ()))
	    {
	      iPortal* portal = thing->GetPortal (j);
	      iSector* dest_sector = portal->GetSector ();
	      if (movable_sectors->Find (dest_sector) == -1)
	      {
	        movable_sectors->AddSector (dest_sector);
	      }
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

void csMeshWrapper::ScaleBy (float factor)
{
  csMatrix3 trans = movable.GetTransform ().GetT2O ();
  trans.m11 *= factor;
  trans.m22 *= factor;
  trans.m33 *= factor;
  movable.SetTransform (trans);
  UpdateMove ();
}


void csMeshWrapper::Rotate (float angle)
{
  csZRotMatrix3 rotz(angle);
  movable.Transform (rotz);
  csXRotMatrix3 rotx(angle);
  movable.Transform (rotx);
  UpdateMove ();
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
	const csCamera& camera, csBox2& sbox, csBox3& cbox)
{
  csVector2 oneCorner;
  GetTransformedBoundingBox (camera, cbox);

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
    camera.Perspective (cbox.Max (), oneCorner);
    sbox.StartBoundingBox (oneCorner);
    csVector3 v (cbox.MinX (), cbox.MinY (), cbox.MaxZ ());
    camera.Perspective (v, oneCorner);
    sbox.AddBoundingVertexSmart (oneCorner);
    camera.Perspective (cbox.Min (), oneCorner);
    sbox.AddBoundingVertexSmart (oneCorner);
    v.Set (cbox.MaxX (), cbox.MaxY (), cbox.MinZ ());
    camera.Perspective (v, oneCorner);
    sbox.AddBoundingVertexSmart (oneCorner);
  }

  return cbox.MaxZ ();
}

void csMeshWrapper::AddChild (iMeshWrapper* child)
{
  // First we increase reference on the mesh to make sure it will
  // not get deleted by unlinking it from it's previous parent.
  child->IncRef ();

  // First unlink the mesh from the engine or another parent.
  iMeshWrapper *oldParent = child->GetParentContainer ();
  if (oldParent)
    oldParent->GetChildren ()->RemoveMesh (child);
  else
    csEngine::current_engine->GetMeshes ()->RemoveMesh (child);

  child->SetParentContainer (&scfiMeshWrapper);
  children.Push (child);
  child->GetMovable ()->SetParent (&movable.scfiMovable);
  child->DecRef ();
}

void csMeshWrapper::RemoveChild (iMeshWrapper* child)
{
  // @@@ is this test really required?
  if (child->GetParentContainer () != &scfiMeshWrapper) return;

  csMeshMeshList& ch = children;
  int idx = ch.Find (child);
  CS_ASSERT (idx != -1);
  ch.Delete (idx);		// Remove object
  child->SetParentContainer (NULL);
  child->GetMovable ()->SetParent (NULL);
}

void csMeshWrapper::MeshWrapper::SetFactory (iMeshFactoryWrapper* factory)
{
  scfParent->SetFactory (factory->GetPrivateObject ());
}

float csMeshWrapper::MeshWrapper::GetScreenBoundingBox (iCamera* camera,
	csBox2& sbox, csBox3& cbox)
{
  return scfParent->GetScreenBoundingBox (camera->GetPrivateObject (),
  	sbox, cbox);
}

//--------------------------------------------------------------------------

void csMeshMeshList::AddMesh (iMeshWrapper* child)
{
  CS_ASSERT (mesh != NULL);
  mesh->AddChild (child);
}

void csMeshMeshList::RemoveMesh (iMeshWrapper* child)
{
  CS_ASSERT (mesh != NULL);
  mesh->RemoveChild (child);
}

//--------------------------------------------------------------------------

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
}

csMeshFactoryWrapper::csMeshFactoryWrapper ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
  csMeshFactoryWrapper::meshFact = NULL;
  parent = NULL;
  children.SetMeshFactory (this);
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

csMeshWrapper* csMeshFactoryWrapper::NewMeshObject ()
{
  iMeshObject* mesh = meshFact->NewInstance ();
  csMeshWrapper* meshObj = new csMeshWrapper (NULL, mesh);
  mesh->DecRef ();
  if (GetName ()) meshObj->SetName (GetName ());
  meshObj->SetFactory (this);
  int i;
  for (i = 0 ; i < children.Length () ; i++)
  {
    csMeshFactoryWrapper* childfact = children.Get (i)->GetPrivateObject ();
    csMeshWrapper* relchild = childfact->NewMeshObject ();
    meshObj->scfiMeshWrapper.GetChildren ()->AddMesh (
    	&(relchild->scfiMeshWrapper));
    relchild->GetMovable ().SetTransform (childfact->GetTransform ());
    relchild->GetMovable ().UpdateMove ();
    relchild->DecRef ();
  }
  return meshObj;
}

void csMeshFactoryWrapper::HardTransform (const csReversibleTransform& t)
{
  meshFact->HardTransform (t);
}

void csMeshFactoryWrapper::AddChild (iMeshFactoryWrapper* child)
{
  // First unlink the factory from another possible parent.
  csMeshFactoryWrapper* c = child->GetPrivateObject ();
  // First we increase reference on the mesh to make sure it will
  // not get deleted by unlinking it from it's previous parent.
  c->IncRef ();
  if (c->parent)
  {
    csMeshFactoryFactoryList& ch = c->parent->children;
    int idx = ch.Find (child);
    CS_ASSERT (idx != -1);	// Impossible!
    ch.Delete (idx);		// Remove object
  }

  c->parent = this;
  children.Push (child); // here child will be IncRef'ed again, so we now do a:
  c->DecRef ();
}

void csMeshFactoryWrapper::RemoveChild (iMeshFactoryWrapper* child)
{
  // First unlink the mesh from the parent.
  csMeshFactoryWrapper* c = child->GetPrivateObject ();
  if (c->parent != this) return;	// Wrong parent, nothing to do.
  csMeshFactoryFactoryList& ch = children;
  int idx = ch.Find (child);
  CS_ASSERT (idx != -1);
  ch.Delete (idx);			// Remove object
  c->parent = NULL;
}

iMeshWrapper* csMeshFactoryWrapper::MeshFactoryWrapper::CreateMeshWrapper ()
{
  csMeshWrapper* wrapper = scfParent->NewMeshObject ();
  if (wrapper)
    return &(wrapper->scfiMeshWrapper);
  else
    return NULL;
}

iMeshFactoryWrapper* csMeshFactoryWrapper::MeshFactoryWrapper::
	GetParentContainer () const
{
  csMeshFactoryWrapper* par = scfParent->parent;
  if (!par) return NULL;
  return &(par->scfiMeshFactoryWrapper);
}

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

bool csMeshList::FreeItem (csSome Item)
{
  iMeshWrapper* mesh = (iMeshWrapper*)Item;
  mesh->DecRef ();
  return true;
}

void csMeshList::AddMesh (iMeshWrapper *mesh)
{
  Push (mesh);
}

void csMeshList::RemoveMesh (iMeshWrapper *mesh)
{
  int n = Find (mesh);
  if (n >= 0) Delete (n); 
}

int csMeshList::MeshList::GetMeshCount () const
{ return scfParent->Length (); }
iMeshWrapper *csMeshList::MeshList::GetMesh (int idx) const
{ return scfParent->Get (idx); }
iMeshWrapper *csMeshList::MeshList::FindByName (const char *name) const
{ return scfParent->FindByName (name); }
int csMeshList::MeshList::Find (iMeshWrapper *mesh) const
{ return scfParent->Find (mesh); }

//--------------------------------------------------------------------------

void csMeshFactoryFactoryList::AddMeshFactory (iMeshFactoryWrapper* child)
{
  CS_ASSERT (meshfact != NULL);
  meshfact->AddChild (child);
}

void csMeshFactoryFactoryList::RemoveMeshFactory (iMeshFactoryWrapper* child)
{
  CS_ASSERT (meshfact != NULL);
  meshfact->RemoveChild (child);
}

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

bool csMeshFactoryList::FreeItem (csSome Item)
{
  iMeshFactoryWrapper* mesh = (iMeshFactoryWrapper*)Item;
  mesh->DecRef ();
  Item = NULL;
  return true;
}

void csMeshFactoryList::AddMeshFactory (iMeshFactoryWrapper *mesh)
{
  Push (mesh);
}

void csMeshFactoryList::RemoveMeshFactory (iMeshFactoryWrapper *mesh)
{
  int n = Find (mesh);
  if (n >= 0) Delete (n); 
}

int csMeshFactoryList::MeshFactoryList::GetMeshFactoryCount () const
{ return scfParent->Length (); }
iMeshFactoryWrapper *csMeshFactoryList::MeshFactoryList::GetMeshFactory (
	int idx) const
{ return scfParent->Get (idx); }
iMeshFactoryWrapper *csMeshFactoryList::MeshFactoryList::FindByName (
	const char *name) const
{ return scfParent->FindByName (name); }
int csMeshFactoryList::MeshFactoryList::Find (iMeshFactoryWrapper *mesh) const
{ return scfParent->Find (mesh); }

//--------------------------------------------------------------------------

