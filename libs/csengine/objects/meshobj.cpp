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
#include "igraph3d.h"
#include "irview.h"

IMPLEMENT_CSOBJTYPE (csMeshWrapper, csPObject)

IMPLEMENT_IBASE_EXT (csMeshWrapper)
  IMPLEMENTS_EMBEDDED_INTERFACE (iMeshWrapper)
  IMPLEMENTS_EMBEDDED_INTERFACE (iVisibilityObject)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csMeshWrapper::MeshWrapper)
  IMPLEMENTS_INTERFACE (iMeshWrapper)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csMeshWrapper::VisObject)
  IMPLEMENTS_INTERFACE (iVisibilityObject)
IMPLEMENT_EMBEDDED_IBASE_END

csMeshWrapper::csMeshWrapper (csObject* theParent, iMeshObject* mesh)
	: csPObject ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiMeshWrapper);
  CONSTRUCT_EMBEDDED_IBASE (scfiVisibilityObject);

  movable.scfParent = this;
  defered_num_lights = 0;
  defered_lighting_flags = 0;
  is_visible = false;
  camera_cookie = 0;
  myOwner = NULL;
  parent = theParent;
  movable.SetObject (this);
  if (parent->GetType () >= csMeshWrapper::Type)
  {
    csMeshWrapper* sparent = (csMeshWrapper*)parent;
    movable.SetParent (&sparent->GetMovable ());
  }

  csEngine::current_engine->AddToCurrentRegion (this);
  csMeshWrapper::mesh = mesh;
  mesh->IncRef ();
  draw_cb = NULL;
  factory = NULL;
}

csMeshWrapper::csMeshWrapper (csObject* theParent)
	: csPObject ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiMeshWrapper);
  CONSTRUCT_EMBEDDED_IBASE (scfiVisibilityObject);

  movable.scfParent = this;
  defered_num_lights = 0;
  defered_lighting_flags = 0;
  is_visible = false;
  camera_cookie = 0;
  myOwner = NULL;
  parent = theParent;
  movable.SetObject (this);
  if (parent->GetType () >= csMeshWrapper::Type)
  {
    csMeshWrapper* sparent = (csMeshWrapper*)parent;
    movable.SetParent (&sparent->GetMovable ());
  }

  csEngine::current_engine->AddToCurrentRegion (this);
  csMeshWrapper::mesh = NULL;
  draw_cb = NULL;
  factory = NULL;
}

void csMeshWrapper::SetMeshObject (iMeshObject* mesh)
{
  if (mesh) mesh->IncRef ();
  if (csMeshWrapper::mesh) csMeshWrapper::mesh->DecRef ();
  csMeshWrapper::mesh = mesh;
}

csMeshWrapper::~csMeshWrapper ()
{
  if (mesh) mesh->DecRef ();
  if (parent->GetType () == csEngine::Type)
  {
    csEngine* engine = (csEngine*)parent;
    engine->UnlinkMesh (this);
  }
}

void csMeshWrapper::UpdateInPolygonTrees ()
{
return; //@@@@@@@@@@@@@@@@@@@@@@
#if 0
  bbox.RemoveFromTree ();

  // If we are not in a sector which has a polygon tree
  // then we don't really update. We should consider if this is
  // a good idea. Do we only want this object updated when we
  // want to use it in a polygon tree? It is certainly more
  // efficient to do it this way when the object is currently
  // moving in normal convex sectors.
  int i;
  csPolygonTree* tree = NULL;
  csVector& sects = movable.GetSectors ();
  for (i = 0 ; i < sects.Length () ; i++)
  {
    csThing* stat = ((csSector*)sects[i])->GetStaticThing ();
    if (stat) { tree = stat->GetStaticTree (); break; }
  }
  if (!tree) return;

  csBox3 b;
  mesh->GetObjectBoundingBox (b);

  // This transform should be part of the object class and not just calculated
  // every time we need it. @@@!!!
  csTransform trans = movable.GetFullTransform ().GetInverse ();

  bbox.Update (b, trans, this);

  // Here we need to insert in trees where this mesh lives.
  for (i = 0 ; i < sects.Length () ; i++)
  {
    csThing* stat = ((csSector*)sects[i])->GetStaticThing ();
    if (stat)
    {
      tree = stat->GetStaticTree ();
      // Temporarily increase reference to prevent free.
      bbox.GetBaseStub ()->IncRef ();
      tree->AddObject (&bbox);
      bbox.GetBaseStub ()->DecRef ();
    }
  }
#endif
}

void csMeshWrapper::UpdateMove ()
{
  UpdateInPolygonTrees ();
  int i;
  for (i = 0 ; i < children.Length () ; i++)
  {
    csMeshWrapper* spr = (csMeshWrapper*)children[i];
    spr->GetMovable ().UpdateMove ();
  }
}

void csMeshWrapper::MoveToSector (csSector* s)
{
  s->AddMesh (this);
}

void csMeshWrapper::RemoveFromSectors ()
{
  if (parent->GetType () != csEngine::Type) return;
  int i;
  csVector& sectors = movable.GetSectors ();
  for (i = 0 ; i < sectors.Length () ; i++)
  {
    csSector* ss = (csSector*)sectors[i];
    if (ss)
      ss->UnlinkMesh (this);
  }
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
  iMeshWrapper* meshwrap = &scfiMeshWrapper;
  if (draw_cb) draw_cb (meshwrap, rview, draw_cbData);
  if (mesh->DrawTest (rview, &movable.scfiMovable))
  {
    UpdateDeferedLighting (movable.GetFullPosition ());
    mesh->Draw (rview, &movable.scfiMovable);
  }
  int i;
  for (i = 0 ; i < children.Length () ; i++)
  {
    csMeshWrapper* spr = (csMeshWrapper*)children[i];
    spr->Draw (rview);
  }
}

void csMeshWrapper::NextFrame (cs_time current_time)
{
  mesh->NextFrame (current_time);
  int i;
  for (i = 0 ; i < children.Length () ; i++)
  {
    csMeshWrapper* spr = (csMeshWrapper*)children[i];
    spr->NextFrame (current_time);
  }
}

void csMeshWrapper::UpdateLighting (iLight** lights, int num_lights)
{
  defered_num_lights = 0;
  if (num_lights <= 0) return;
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
  const csReversibleTransform& trans = movable.GetTransform ();
  csVector3 startObj = trans * start;
  csVector3 endObj = trans * end;
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

//--------------------------------------------------------------------------

IMPLEMENT_IBASE (csMeshFactoryWrapper)
  IMPLEMENTS_EMBEDDED_INTERFACE (iMeshFactoryWrapper)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csMeshFactoryWrapper::MeshFactoryWrapper)
  IMPLEMENTS_INTERFACE (iMeshFactoryWrapper)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_CSOBJTYPE (csMeshFactoryWrapper, csObject)

csMeshFactoryWrapper::csMeshFactoryWrapper (iMeshObjectFactory* meshFact)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
  csMeshFactoryWrapper::meshFact = meshFact;
  meshFact->IncRef ();
}

csMeshFactoryWrapper::csMeshFactoryWrapper ()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
  csMeshFactoryWrapper::meshFact = NULL;
}

csMeshFactoryWrapper::~csMeshFactoryWrapper ()
{
  if (meshFact) meshFact->DecRef ();
}

void csMeshFactoryWrapper::SetMeshObjectFactory (iMeshObjectFactory* meshFact)
{
  if (meshFact) meshFact->IncRef ();
  if (csMeshFactoryWrapper::meshFact) csMeshFactoryWrapper::meshFact->DecRef ();
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

