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
#include "csengine/sector.h"
#include "csengine/meshobj.h"
#include "csengine/light.h"
#include "csengine/engine.h"
#include "csengine/thing.h"
#include "ivideo/graph3d.h"
#include "iengine/rview.h"

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
    int idx = csEngine::current_engine->meshes.Find (this);
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

csMeshWrapper::csMeshWrapper (csObject* theParent, iMeshObject* mesh)
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
  parent = theParent;
  movable.SetObject (this);

  iMeshWrapper *sparent = SCF_QUERY_INTERFACE_FAST (parent, iMeshWrapper);
  if (sparent)
  {
    movable.SetParent(
      ((csMovable::eiMovable*)sparent->GetMovable())->scfParent);
    sparent->DecRef ();
  }

  csEngine::current_engine->AddToCurrentRegion (this);
  csMeshWrapper::mesh = mesh;
  mesh->IncRef ();
  draw_cb = NULL;
  factory = NULL;
  zbufMode = CS_ZBUF_USE;
  render_priority = csEngine::current_engine->GetObjectRenderPriority ();
}

csMeshWrapper::csMeshWrapper (csObject* theParent)
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
  parent = theParent;
  movable.SetObject (this);

  iMeshWrapper *sparent = SCF_QUERY_INTERFACE_FAST (parent, iMeshWrapper);
  if (sparent)
  {
    movable.SetParent(
      ((csMovable::eiMovable*)sparent->GetMovable())->scfParent);
    sparent->DecRef ();
  }

  csEngine::current_engine->AddToCurrentRegion (this);
  csMeshWrapper::mesh = NULL;
  draw_cb = NULL;
  factory = NULL;
  zbufMode = CS_ZBUF_USE;
  render_priority = csEngine::current_engine->GetObjectRenderPriority ();
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
    csMeshWrapper* spr = (csMeshWrapper*)children[i];
    spr->GetMovable ().UpdateMove ();
  }
}

void csMeshWrapper::MoveToSector (csSector* s)
{
  // Only add this mesh to a sector if the parent is the engine.
  // Otherwise we have a hierarchical object and in that case
  // the parent object controls this.
  iEngine *e = SCF_QUERY_INTERFACE_FAST (parent, iEngine);
  if (e)
  {
    s->AddMesh (this);
    e->DecRef ();
  }
}

void csMeshWrapper::RemoveFromSectors ()
{
  iEngine *e = SCF_QUERY_INTERFACE_FAST (parent, iEngine);
  if (!e) return;
  int i;
  const csVector& sectors = movable.GetSectors ();
  for (i = 0 ; i < sectors.Length () ; i++)
  {
    csSector* ss = (csSector*)sectors[i];
    if (ss)
      ss->UnlinkMesh (this);
  }
  e->DecRef ();
}

void csMeshWrapper::SetRenderPriority (long rp)
{
  render_priority = rp;
  iEngine *e = SCF_QUERY_INTERFACE_FAST (parent, iEngine);
  if (!e) return;
  int i;
  const csVector& sectors = movable.GetSectors ();
  for (i = 0 ; i < sectors.Length () ; i++)
  {
    csSector* ss = (csSector*)sectors[i];
    if (ss) ss->RelinkMesh (this);
  }
  e->DecRef ();
}

/// The list of lights that hit the mesh
static DECLARE_GROWING_ARRAY_REF (light_worktable, iLight*);

void csMeshWrapper::UpdateDeferedLighting (const csVector3& pos)
{
  if (defered_num_lights)
  {
    if (defered_num_lights > light_worktable.Limit ())
      light_worktable.SetLimit (defered_num_lights);

    csSector* sect = movable.GetSector (0);
    int num_lights = csEngine::current_engine->GetNearbyLights (sect,
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
    rview->CallCallback (CALLBACK_MESH, (void*)&scfiMeshWrapper);
  }
  if (draw_cb) if (!draw_cb->BeforeDrawing (meshwrap, rview)) return;
  if (mesh->DrawTest (rview, &movable.scfiMovable))
  {
    if (rview->GetCallback ())
    {
      rview->CallCallback (CALLBACK_VISMESH, (void*)&scfiMeshWrapper);
    }
    else
    {
      cs_time lt = csEngine::current_engine->GetLastAnimationTime ();
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
    csMeshWrapper* spr = (csMeshWrapper*)children[i];
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
    csMeshWrapper* spr = (csMeshWrapper*)children[i];
    spr->UpdateLighting (lights, num_lights);
  }
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
  bool rc = HitBeamObject (startObj, endObj, isect, pr);
  if (rc)
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
    csMeshWrapper* spr = (csMeshWrapper*)children[i];
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

void csMeshWrapper::MeshWrapper::SetFactory (iMeshFactoryWrapper* factory)
{
  scfParent->SetFactory (factory->GetPrivateObject ());
}

void csMeshWrapper::MeshWrapper::AddChild (iMeshWrapper* child)
{
  // First unlink the mesh from the engine or another parent.
  csMeshWrapper* c = child->GetPrivateObject ();
  csObject* par = c->GetParentContainer ();

  // First we increase reference on the mesh to make sure it will
  // not get deleted by unlinking it from it's previous parent.
  // We will also keep this incremented because the parent mesh
  // now holds an additional reference to this child.
  c->IncRef ();

  if (par)
  {
    iEngine *engine = SCF_QUERY_INTERFACE_FAST (par, iEngine);
    if (engine)
    {
      engine->GetCsEngine ()->RemoveMesh (c);
      engine->DecRef ();
    }
    else
    {
      csMeshWrapper* old_mesh = (csMeshWrapper*)par;
      csNamedObjVector& ch = old_mesh->children;
      int idx = ch.Find (c);
      if (idx != -1)
      {
        ch.Delete (idx, false);		// Unlink object
	c->DecRef ();
      }
    }
  }
  c->SetParentContainer ((csMeshWrapper*)scfParent);
  scfParent->children.Push (c);
  c->GetMovable ().SetParent (&scfParent->movable);
}

void csMeshWrapper::MeshWrapper::RemoveChild (iMeshWrapper* child)
{
  // First unlink the mesh from the engine or another parent.
  csMeshWrapper* c = child->GetPrivateObject ();
  csObject* par = c->GetParentContainer ();

  if (par != scfParent) return;	// Wrong parent, nothing to do.
  csNamedObjVector& ch = scfParent->children;
  int idx = ch.Find (c);
  if (idx != -1)
  {
    ch.Delete (idx, false);		// Unlink object
    c->DecRef ();
  }
  c->SetParentContainer (NULL);
  c->GetMovable ().SetParent (NULL);
}

iBase* csMeshWrapper::MeshWrapper::GetParentContainer ()
{
  csObject* par = scfParent->GetParentContainer ();
  if (!par) return NULL;
  return (iBase*)par;	// par == iObject == iBase
}

iMeshWrapper* csMeshWrapper::MeshWrapper::GetChild (int idx) const
{
  csMeshWrapper* child = (csMeshWrapper*)(scfParent->GetChildren ()[idx]);
  return &(child->scfiMeshWrapper);
}

float csMeshWrapper::MeshWrapper::GetScreenBoundingBox (iCamera* camera,
	csBox2& sbox, csBox3& cbox)
{
  return scfParent->GetScreenBoundingBox (camera->GetPrivateObject (),
  	sbox, cbox);
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
}

csMeshFactoryWrapper::csMeshFactoryWrapper ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
  csMeshFactoryWrapper::meshFact = NULL;
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

csMeshWrapper* csMeshFactoryWrapper::NewMeshObject (csObject* parent)
{
  iMeshObject* mesh = meshFact->NewInstance ();
  csMeshWrapper* meshObj = new csMeshWrapper (parent, mesh);
  mesh->DecRef ();
  meshObj->SetFactory (this);
  return meshObj;
}

void csMeshFactoryWrapper::HardTransform (const csReversibleTransform& t)
{
  meshFact->HardTransform (t);
}

