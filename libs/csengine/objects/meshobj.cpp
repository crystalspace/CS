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
#include "igeom/objmodel.h"
#include "csengine/sector.h"
#include "csengine/meshobj.h"
#include "csengine/meshlod.h"
#include "csengine/light.h"
#include "csengine/engine.h"
#include "iengine/portal.h"
#include "csutil/debug.h"
#include "iengine/rview.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/polygon.h"
#include "ivideo/graph3d.h"


// ---------------------------------------------------------------------------
// csMeshWrapper
// ---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE_EXT(csMeshWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iMeshWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iImposter)
  SCF_IMPLEMENTS_INTERFACE(csMeshWrapper)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMeshWrapper::MeshWrapper)
  SCF_IMPLEMENTS_INTERFACE(iMeshWrapper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMeshWrapper::MeshImposter)
  SCF_IMPLEMENTS_INTERFACE(iImposter)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMeshWrapper::csMeshWrapper (iMeshWrapper *theParent, iMeshObject *meshobj) :
    csSectorObject (theParent)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshWrapper);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiImposter);
  DG_TYPE (this, "csMeshWrapper");

  defered_num_lights = 0;
  defered_lighting_flags = 0;
  last_anim_time = 0;

  csMeshWrapper::meshobj = meshobj;
  if (meshobj)
  {
    light_info = SCF_QUERY_INTERFACE (meshobj, iLightingInfo);
    shadow_receiver = SCF_QUERY_INTERFACE (meshobj, iShadowReceiver);
  }
  factory = 0;
  zbufMode = CS_ZBUF_USE;
  children.SetMesh (this);
  imposter_active = false;
  imposter_mesh = 0;
  cast_hardware_shadow = true;
  draw_after_fancy_stuff = false;
}

csMeshWrapper::csMeshWrapper (iMeshWrapper *theParent) :
  csSectorObject(theParent)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshWrapper);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiImposter);
  DG_TYPE (this, "csMeshWrapper");

  defered_num_lights = 0;
  defered_lighting_flags = 0;
  last_anim_time = 0;
  imposter_active = false;
  imposter_mesh = 0;

  factory = 0;
  zbufMode = CS_ZBUF_USE;
  children.SetMesh (this);
  cast_hardware_shadow = true;
  draw_after_fancy_stuff = false;
}

void csMeshWrapper::ClearFromSectorPortalLists ()
{
  if (meshobj && meshobj->GetPortalCount () > 0)
  {
    int i;
    const iSectorList *sectors = movable.GetSectors ();
    for (i = 0; i < sectors->GetCount (); i++)
    {
      iSector *ss = sectors->Get (i);
      if (ss) ss->UnregisterPortalMesh (&scfiMeshWrapper);
    }
  }
}

void csMeshWrapper::SetMeshObject (iMeshObject *meshobj)
{
  ClearFromSectorPortalLists ();

  csMeshWrapper::meshobj = meshobj;
  if (meshobj)
  {
    light_info = SCF_QUERY_INTERFACE (meshobj, iLightingInfo);
    shadow_receiver = SCF_QUERY_INTERFACE (meshobj, iShadowReceiver);
    if (meshobj->GetPortalCount () > 0)
    {
      int i;
      const iSectorList *sectors = movable.GetSectors ();
      for (i = 0; i < sectors->GetCount (); i++)
      {
        iSector *ss = sectors->Get (i);
        if (ss) ss->UnregisterPortalMesh (&scfiMeshWrapper);
      }
    }
  }
  else
  {
    light_info = 0;
    shadow_receiver = 0;
  }
}

csMeshWrapper::~csMeshWrapper ()
{
  delete imposter_mesh;
  ClearFromSectorPortalLists ();
}

void csMeshWrapper::UpdateMove ()
{
  int i;
  for (i = 0; i < children.GetCount (); i++)
  {
    iMeshWrapper *spr = children.Get (i);
    spr->GetMovable ()->UpdateMove ();
  }
}

void csMeshWrapper::MoveToSector (iSector *s)
{
  // Only add this mesh to a sector if the parent is the engine.
  // Otherwise we have a hierarchical object and in that case
  // the parent object controls this.
  if (!Parent) s->GetMeshes ()->Add (&scfiMeshWrapper);
  if (meshobj && meshobj->GetPortalCount () > 0)
    s->RegisterPortalMesh (&scfiMeshWrapper);
}

void csMeshWrapper::RemoveFromSectors ()
{
  if (Parent) return ;

  int i;
  const iSectorList *sectors = movable.GetSectors ();
  for (i = 0; i < sectors->GetCount (); i++)
  {
    iSector *ss = sectors->Get (i);
    if (ss)
    {
      ss->GetMeshes ()->Remove (&scfiMeshWrapper);
      ss->UnregisterPortalMesh (&scfiMeshWrapper);
    }
  }
}

void csMeshWrapper::SetRenderPriority (long rp)
{
  csSectorObject::SetRenderPriority (rp);

  if (Parent) return ;

  int i;
  const iSectorList *sectors = movable.GetSectors ();
  for (i = 0; i < sectors->GetCount (); i++)
  {
    iSector *ss = sectors->Get (i);
    if (ss) ss->GetPrivateObject ()->RelinkMesh (&scfiMeshWrapper);
  }
}

/// The list of lights that hit the mesh
typedef csDirtyAccessArray<iLight*> engine3d_LightWorkTable;
CS_IMPLEMENT_STATIC_VAR (GetStaticLightWorkTable, engine3d_LightWorkTable,())

void csMeshWrapper::UpdateDeferedLighting (const csBox3 &box)
{
  static engine3d_LightWorkTable &light_worktable = *GetStaticLightWorkTable ();
  const iSectorList *movable_sectors = movable.GetSectors ();
  if (defered_num_lights && movable_sectors->GetCount () > 0)
  {
    if (defered_num_lights > light_worktable.Length ())
      light_worktable.SetLength (defered_num_lights);

    iSector *sect = movable_sectors->Get (0);
    int num_lights = csEngine::current_iengine->GetNearbyLights (
        sect,
        box,
        defered_lighting_flags,
        light_worktable.GetArray (),
        defered_num_lights);
    UpdateLighting (light_worktable.GetArray (), num_lights);
  }
}

void csMeshWrapper::DeferUpdateLighting (int flags, int num_lights)
{
  defered_num_lights = num_lights;
  defered_lighting_flags = flags;
}

void csMeshWrapper::Draw (iRenderView *rview)
{
  if (flags.Check (CS_ENTITY_INVISIBLE)) return ;
  DrawInt (rview);
}

csRenderMesh** csMeshWrapper::GetRenderMeshes (int& n)
{
//  iMeshWrapper *meshwrap = &scfiMeshWrapper;

  //int i;
  // Callback are traversed in reverse order so that they can safely
  // delete themselves.
/*  i = draw_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iMeshDrawCallback* cb = draw_cb_vector.Get (i);
    if (!cb->BeforeDrawing (meshwrap, rview)) return 0;
    i--;
  }*/

  /*draw_test = meshobj->DrawTest (rview, &movable.scfiMovable);
  if (draw_test)
  {
    csTicks lt = csEngine::current_engine->GetLastAnimationTime ();
    if (lt != 0)
    {
      if (lt != last_anim_time)
      {
        meshobj->NextFrame (lt,movable.GetPosition ());
        last_anim_time = lt;
      }
    }*/

    csTicks lt = csEngine::current_engine->GetLastAnimationTime ();
    meshobj->NextFrame (lt,movable.GetPosition ());
    UpdateDeferedLighting (movable.GetFullPosition ());
    return meshobj->GetRenderMeshes (n);
/*  }
  return 0;*/

  /*
  for (i = 0; i < children.GetCount (); i++)
  {
    iMeshWrapper *spr = children.Get (i);
    spr->DrawZ (rview);
  }
  */
}

void csMeshWrapper::DrawShadow (iRenderView* rview, iLight* light)
{
  /*
  if (cast_hardware_shadow)
    meshobj->DrawShadow (rview, &movable.scfiMovable, zbufMode, light);
  */
}

void csMeshWrapper::DrawLight (iRenderView* rview, iLight* light)
{
  /*
  if (draw_test) 
    meshobj->DrawLight (rview, &movable.scfiMovable, zbufMode, light);
  */
}

void csMeshWrapper::CastHardwareShadow (bool castShadow)
{
  cast_hardware_shadow = castShadow;
}

void csMeshWrapper::SetDrawAfterShadow (bool drawAfter)
{
  draw_after_fancy_stuff = drawAfter;
}

bool csMeshWrapper::GetDrawAfterShadow ()
{
  return draw_after_fancy_stuff;
}

//----- Static LOD ----------------------------------------------------------

iLODControl* csMeshWrapper::CreateStaticLOD ()
{
  static_lod = csPtr<csStaticLODMesh> (new csStaticLODMesh ());
  return static_lod;
}

void csMeshWrapper::DestroyStaticLOD ()
{
  static_lod = 0;
}

iLODControl* csMeshWrapper::GetStaticLOD ()
{
  return (iLODControl*)static_lod;
}

void csMeshWrapper::RemoveMeshFromStaticLOD (iMeshWrapper* mesh)
{
  if (!static_lod) return;	// No static lod, nothing to do here.
  int lod;
  for (lod = 0 ; lod < static_lod->GetLODCount () ; lod++)
  {
    csArray<iMeshWrapper*>& meshes_for_lod = static_lod->GetMeshesForLOD (lod);
    meshes_for_lod.Delete (mesh);
  }
}

void csMeshWrapper::AddMeshToStaticLOD (int lod, iMeshWrapper* mesh)
{
  if (!static_lod) return;	// No static lod, nothing to do here.
  csArray<iMeshWrapper*>& meshes_for_lod = static_lod->GetMeshesForLOD (lod);
  meshes_for_lod.Push (mesh);
}

//---------------------------------------------------------------------------

void csMeshWrapper::DrawInt (iRenderView *rview)
{
  if (imposter_active && CheckImposterRelevant(rview) )
    if (DrawImposter (rview))
    return;

  DrawIntFull (rview);
}

bool csMeshWrapper::CheckImposterRelevant (iRenderView *rview)
{
  float wor_sq_dist = GetSquaredDistance (rview);
  float dist = min_imposter_distance->Get ();
  return (wor_sq_dist > dist*dist);
}

void csMeshWrapper::DrawIntFull (iRenderView *rview)
{
  iMeshWrapper *meshwrap = &scfiMeshWrapper;

  int i;
  // Callback are traversed in reverse order so that they can safely
  // delete themselves.
  i = draw_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iMeshDrawCallback* cb = draw_cb_vector.Get (i);
    if (!cb->BeforeDrawing (meshwrap, rview)) return ;
    i--;
  }

  if (meshobj->DrawTest (rview, &movable.scfiMovable))
  {
    csTicks lt = csEngine::current_engine->GetLastAnimationTime ();
    if (lt != 0)
    {
      if (lt != last_anim_time)
      {
        meshobj->NextFrame (lt,movable.GetPosition ());
        last_anim_time = lt;
      }
    }

    csBox3 bbox;
    GetFullBBox (bbox);
    UpdateDeferedLighting (bbox);
    meshobj->Draw (rview, &movable.scfiMovable, zbufMode);
  }

  if (static_lod)
  {
    // If we have static lod we only draw the children for the right LOD level.
    float distance = qsqrt (GetSquaredDistance (rview));
    float lod = static_lod->GetLODValue (distance);
    csArray<iMeshWrapper*>& meshes = static_lod->GetMeshesForLOD (lod);
    for (i = 0 ; i < meshes.Length () ; i++)
    {
      meshes[i]->Draw (rview);
    }
  }
  else
  {
    for (i = 0; i < children.GetCount (); i++)
    {
      iMeshWrapper *spr = children.Get (i);
      spr->Draw (rview);
    }
  }
}

bool csMeshWrapper::DrawImposter (iRenderView *rview)
{
  // Check for imposter existence.  If not, create it.
  if (!imposter_mesh)
  {
      return false;
  }

  // Check for imposter already ready
  if (!imposter_mesh->GetImposterReady())
      return false;

  // Check for too much camera movement since last imposter render
  if (!imposter_mesh->CheckIncidenceAngle (rview,
    imposter_rotation_tolerance->Get () ))
      return false;

  // Else draw imposter as-is.
  imposter_mesh->Draw (rview);
  return true;
}

void csMeshWrapper::SetImposterActive(bool flag,iObjectRegistry *objreg)
{
  imposter_active = flag;
  if (flag)
  {
    imposter_mesh = new csImposterMesh (this, objreg);
    imposter_mesh->SetImposterReady (false);
  }
}

void csMeshWrapper::UpdateLighting (iLight **lights, int num_lights)
{
  defered_num_lights = 0;

  //if (num_lights <= 0) return;
  meshobj->UpdateLighting (lights, num_lights, &movable.scfiMovable);

  int i;
  for (i = 0; i < children.GetCount (); i++)
  {
    iMeshWrapper *spr = children.Get (i);
    spr->UpdateLighting (lights, num_lights);
  }
}

bool csMeshWrapper::HitBeamOutline (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr)
{
  return meshobj->HitBeamOutline (start, end, isect, pr);
}

bool csMeshWrapper::HitBeamObject (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr)
{
  return meshobj->HitBeamObject (start, end, isect, pr);
}

bool csMeshWrapper::HitBeam (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr)
{
  csVector3 startObj;
  csVector3 endObj;
  csReversibleTransform trans;
  if (movable.IsFullTransformIdentity ())
  {
    startObj = start;
    endObj = end;
  }
  else
  {
    trans = movable.GetFullTransform ();
    startObj = trans.Other2This (start);
    endObj = trans.Other2This (end);
  }
  bool rc = false;
  if (HitBeamBBox (startObj, endObj, isect, 0) > -1)
  {
    rc = HitBeamOutline (startObj, endObj, isect, pr);
    if (rc)
    {
      if (!movable.IsFullTransformIdentity ())
        isect = trans.This2Other (isect);
    }
  }

  return rc;
}

void csMeshWrapper::HardTransform (const csReversibleTransform &t)
{
  meshobj->HardTransform (t);

  int i;
  for (i = 0; i < children.GetCount (); i++)
  {
    iMeshWrapper *spr = children.Get (i);
    spr->HardTransform (t);
  }
}

void csMeshWrapper::GetRadius (csVector3 &rad, csVector3 &cent) const
{
  meshobj->GetObjectModel ()->GetRadius (rad, cent);
  if (children.GetCount () > 0)
  {
    float max_radius = rad.x;
    if (max_radius < rad.y) max_radius = rad.y;
    if (max_radius < rad.z) max_radius = rad.z;

    csSphere sphere (cent, max_radius);
    int i;
    for (i = 0; i < children.GetCount (); i++)
    {
      iMeshWrapper *spr = children.Get (i);
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

float csMeshWrapper::MeshWrapper::GetScreenBoundingBox (
  iCamera *camera,
  csBox2 &sbox,
  csBox3 &cbox)
{
  return scfParent->GetScreenBoundingBox (camera, sbox, cbox);
}

// ---------------------------------------------------------------------------
// csMeshFactoryWrapper
// ---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE_EXT(csMeshFactoryWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iMeshFactoryWrapper)
  SCF_IMPLEMENTS_INTERFACE(csMeshFactoryWrapper)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMeshFactoryWrapper::MeshFactoryWrapper)
  SCF_IMPLEMENTS_INTERFACE(iMeshFactoryWrapper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMeshFactoryWrapper::csMeshFactoryWrapper (
  iMeshObjectFactory *meshFact)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
  csMeshFactoryWrapper::meshFact = meshFact;
  parent = 0;
  children.SetMeshFactory (this);
}

csMeshFactoryWrapper::csMeshFactoryWrapper ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
  parent = 0;
  children.SetMeshFactory (this);
}

csMeshFactoryWrapper::~csMeshFactoryWrapper ()
{
  // This line MUST be here to ensure that the children are not
  // removed after the destructor has already finished.
  children.RemoveAll ();
}

void csMeshFactoryWrapper::SetMeshObjectFactory (iMeshObjectFactory *meshFact)
{
  csMeshFactoryWrapper::meshFact = meshFact;
}

iMeshWrapper *csMeshFactoryWrapper::NewMeshObject ()
{
  csRef<iMeshObject> basemesh (meshFact->NewInstance ());
  iMeshWrapper *mesh = &(new csMeshWrapper (0, basemesh))->scfiMeshWrapper;

  if (GetName ()) mesh->QueryObject ()->SetName (GetName ());
  mesh->SetFactory (&scfiMeshFactoryWrapper);

  if (static_lod)
  {
    iLODControl* lod = mesh->CreateStaticLOD ();
    iSharedVariable* varm, * vara;
    static_lod->GetLOD (varm, vara);
    if (varm)
    {
      lod->SetLOD (varm, vara);
    }
    else
    {
      float m, a;
      static_lod->GetLOD (m, a);
      lod->SetLOD (m, a);
    }
  }

  int i;
  for (i = 0; i < children.GetCount (); i++)
  {
    iMeshFactoryWrapper *childfact = children.Get (i);
    iMeshWrapper *child = childfact->CreateMeshWrapper ();
    mesh->GetChildren ()->Add (child);
    child->GetMovable ()->SetTransform (childfact->GetTransform ());
    child->GetMovable ()->UpdateMove ();

    if (static_lod)
    {
      // We have static lod so we need to put the child in the right
      // lod level.
      int l;
      for (l = 0 ; l < static_lod->GetLODCount () ; l++)
      {
        csArray<iMeshFactoryWrapper*>& facts_for_lod =
      	  static_lod->GetMeshesForLOD (l);
        int j;
	for (j = 0 ; j < facts_for_lod.Length () ; j++)
	{
	  if (facts_for_lod[j] == childfact)
	    mesh->AddMeshToStaticLOD (l, child);
	}
      }
    }

    child->DecRef ();
  }

  return mesh;
}

void csMeshFactoryWrapper::HardTransform (const csReversibleTransform &t)
{
  meshFact->HardTransform (t);
}

iLODControl* csMeshFactoryWrapper::CreateStaticLOD ()
{
  static_lod = csPtr<csStaticLODFactoryMesh> (new csStaticLODFactoryMesh ());
  return static_lod;
}

void csMeshFactoryWrapper::DestroyStaticLOD ()
{
  static_lod = 0;
}

iLODControl* csMeshFactoryWrapper::GetStaticLOD ()
{
  return (iLODControl*)static_lod;
}

void csMeshFactoryWrapper::SetStaticLOD (float m, float a)
{
  if (static_lod) static_lod->SetLOD (m, a);
}

void csMeshFactoryWrapper::GetStaticLOD (float& m, float& a) const
{
  if (static_lod)
    static_lod->GetLOD (m, a);
  else
  {
    m = 0;
    a = 0;
  }
}

void csMeshFactoryWrapper::RemoveFactoryFromStaticLOD (
	iMeshFactoryWrapper* fact)
{
  if (!static_lod) return;	// No static lod, nothing to do here.
  int lod;
  for (lod = 0 ; lod < static_lod->GetLODCount () ; lod++)
  {
    csArray<iMeshFactoryWrapper*>& meshes_for_lod =
    	static_lod->GetMeshesForLOD (lod);
    meshes_for_lod.Delete (fact);
  }
}

void csMeshFactoryWrapper::AddFactoryToStaticLOD (int lod,
	iMeshFactoryWrapper* fact)
{
  if (!static_lod) return;	// No static lod, nothing to do here.
  csArray<iMeshFactoryWrapper*>& meshes_for_lod =
  	static_lod->GetMeshesForLOD (lod);
  meshes_for_lod.Push (fact);
}

//--------------------------------------------------------------------------
// csMeshList
//--------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE(csMeshList)
  SCF_IMPLEMENTS_INTERFACE(iMeshList)
SCF_IMPLEMENT_IBASE_END

csMeshList::csMeshList ()
{
  SCF_CONSTRUCT_IBASE (0);
}

csMeshList::~csMeshList ()
{
  RemoveAll ();
}

iMeshWrapper* csMeshList::FindByNameWithChild (const char *Name) const
{
  char* p = strchr (Name, ':');
  if (!p) return list.FindByName (Name);

  int i;
  for (i = 0 ; i < list.Length () ; i++)
  {
    iMeshWrapper* m = list.Get (i);
    if (!strncmp (m->QueryObject ()->GetName (), Name, p-Name))
    {
      return m->GetChildren ()->FindByName (p+1);
    }
  }
  return 0;
}

int csMeshList::Add (iMeshWrapper *obj)
{
  PrepareItem (obj);
  list.Push (obj);
  return true;
}

bool csMeshList::Remove (iMeshWrapper *obj)
{
  FreeItem (obj);
  list.Delete (obj);
  return true;
}

bool csMeshList::Remove (int n)
{
  FreeItem (list[n]);
  list.DeleteIndex (n);
  return true;
}

void csMeshList::RemoveAll ()
{
  int i;
  for (i = 0 ; i < list.Length () ; i++)
  {
    FreeItem (list[i]);
  }
  list.DeleteAll ();
}

int csMeshList::Find (iMeshWrapper *obj) const
{
  return list.Find (obj);
}

iMeshWrapper *csMeshList::FindByName (const char *Name) const
{
  if (strchr (Name, ':'))
    return FindByNameWithChild (Name);
  else
    return list.FindByName (Name);
}

//--------------------------------------------------------------------------
// csMeshMeshList
//--------------------------------------------------------------------------
void csMeshMeshList::PrepareItem (iMeshWrapper* child)
{
  CS_ASSERT (mesh != 0);
  csMeshList::PrepareItem (child);

  // unlink the mesh from the engine or another parent.
  iMeshWrapper *oldParent = child->GetParentContainer ();
  if (oldParent)
    oldParent->GetChildren ()->Remove (child);
  else
    csEngine::current_engine->GetMeshes ()->Remove (child);

  /* csSector->PrepareMesh tells the culler about the mesh
     (since removing the mesh above also removes it from the culler...) */
  for (int i = 0 ; i < mesh->GetCsMovable().GetSectors()->GetCount() ; i++)
  {
    csSector* sector = mesh->GetCsMovable ().GetSectors ()->Get (i)
    	->GetPrivateObject ();
    sector->UnprepareMesh (child);
    sector->PrepareMesh (child);
  }

  child->SetParentContainer (&mesh->scfiMeshWrapper);
  child->GetMovable ()->SetParent (&mesh->GetCsMovable ().scfiMovable);
}

void csMeshMeshList::FreeItem (iMeshWrapper* item)
{
  CS_ASSERT (mesh != 0);

  for (int i = 0 ; i < mesh->GetCsMovable().GetSectors()->GetCount() ; i++)
  {
    csSector* sector = mesh->GetCsMovable ().GetSectors ()->Get (i)
        ->GetPrivateObject ();
    sector->UnprepareMesh (item);
  }

  item->SetParentContainer (0);
  item->GetMovable ()->SetParent (0);

  mesh->RemoveMeshFromStaticLOD (item);

  csMeshList::FreeItem (item);
}

//--------------------------------------------------------------------------
// csMeshFactoryList
//--------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE(csMeshFactoryList)
  SCF_IMPLEMENTS_INTERFACE(iMeshFactoryList)
SCF_IMPLEMENT_IBASE_END

csMeshFactoryList::csMeshFactoryList ()
{
  SCF_CONSTRUCT_IBASE (0);
}

int csMeshFactoryList::Add (iMeshFactoryWrapper *obj)
{
  PrepareItem (obj);
  list.Push (obj);
  return true;
}

bool csMeshFactoryList::Remove (iMeshFactoryWrapper *obj)
{
  FreeItem (obj);
  list.Delete (obj);
  return true;
}

bool csMeshFactoryList::Remove (int n)
{
  FreeItem (list[n]);
  list.Delete (Get (n));
  return true;
}

void csMeshFactoryList::RemoveAll ()
{
  int i;
  for (i = 0 ; i < list.Length () ; i++)
  {
    FreeItem (list[i]);
  }
  list.DeleteAll ();
}

int csMeshFactoryList::Find (iMeshFactoryWrapper *obj) const
{
  return list.Find (obj);
}

iMeshFactoryWrapper *csMeshFactoryList::FindByName (
  const char *Name) const
{
  return list.FindByName (Name);
}

//--------------------------------------------------------------------------
// csMeshFactoryFactoryList
//--------------------------------------------------------------------------
void csMeshFactoryFactoryList::PrepareItem (iMeshFactoryWrapper* child)
{
  CS_ASSERT (meshfact != 0);
  csMeshFactoryList::PrepareItem (child);

  // unlink the factory from another possible parent.
  if (child->GetParentContainer ())
    child->GetParentContainer ()->GetChildren ()->Remove (child);

  child->SetParentContainer (&meshfact->scfiMeshFactoryWrapper);
}

void csMeshFactoryFactoryList::FreeItem (iMeshFactoryWrapper* item)
{
  CS_ASSERT (meshfact != 0);
  item->SetParentContainer (0);
  meshfact->RemoveFactoryFromStaticLOD (item);
  csMeshFactoryList::FreeItem (item);
}
