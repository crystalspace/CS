/*
    Copyright (C) 2000-2004 by Jorrit Tyberghein

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
#include "csqsqrt.h"
#include "csgeom/sphere.h"
#include "igeom/objmodel.h"
#include "igeom/clip2d.h"
#include "plugins/engine/3d/sector.h"
#include "plugins/engine/3d/meshobj.h"
#include "plugins/engine/3d/meshlod.h"
#include "plugins/engine/3d/light.h"
#include "plugins/engine/3d/engine.h"
#include "iengine/portal.h"
#include "csutil/debug.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"


CS_LEAKGUARD_IMPLEMENT (csMeshWrapper);
CS_LEAKGUARD_IMPLEMENT (csMeshFactoryWrapper);

// ---------------------------------------------------------------------------

// Implementations of iShadowCaster and iShadowReceiver that are used
// in case of static lod.

// Static shadow caster will cast shadows from the least detailed object
// that actually has a shadow caster.
class csStaticShadowCaster : public iShadowCaster
{
private:
  // Pointer back to the mesh with static lod.
  csMeshWrapper* static_lod_mesh;

public:
  csStaticShadowCaster (csMeshWrapper* m)
  {
    SCF_CONSTRUCT_IBASE (0);
    static_lod_mesh = m;
  }

  virtual ~csStaticShadowCaster ()
  {
    SCF_DESTRUCT_IBASE ();
  }

  SCF_DECLARE_IBASE;

  virtual void AppendShadows (iMovable* movable, iShadowBlockList* shadows,
  	const csVector3& origin)
  {
    const csMeshMeshList& c = static_lod_mesh->GetCsChildren ();
    int cnt = c.GetCount ();
    int i = cnt-1;
    while (i >= 0)
    {
      iMeshWrapper* child = c.Get (i);
      if (child && child->GetShadowCaster ())
      {
        child->GetShadowCaster ()->AppendShadows (movable, shadows, origin);
	return;
      }
      i--;
    }
  }
};

SCF_IMPLEMENT_IBASE(csStaticShadowCaster)
  SCF_IMPLEMENTS_INTERFACE(iShadowCaster)
SCF_IMPLEMENT_IBASE_END

// Static shadow receiver will send the received shadows to all children
// of the static lod mesh.
class csStaticShadowReceiver : public iShadowReceiver
{
private:
  // Pointer back to the mesh with static lod.
  csMeshWrapper* static_lod_mesh;

public:
  csStaticShadowReceiver (csMeshWrapper* m)
  {
    SCF_CONSTRUCT_IBASE (0);
    static_lod_mesh = m;
  }

  virtual ~csStaticShadowReceiver ()
  {
    SCF_DESTRUCT_IBASE ();
  }

  SCF_DECLARE_IBASE;

  virtual void CastShadows (iMovable* movable, iFrustumView* fview)
  {
    const csMeshMeshList& c = static_lod_mesh->GetCsChildren ();
    int cnt = c.GetCount ();
    int i;
    for (i = 0 ; i < cnt ; i++)
    {
      iMeshWrapper* child = c.Get (i);
      if (child && child->GetShadowReceiver ())
        child->GetShadowReceiver ()->CastShadows (movable, fview);
    }
  }
};

SCF_IMPLEMENT_IBASE(csStaticShadowReceiver)
  SCF_IMPLEMENTS_INTERFACE(iShadowReceiver)
SCF_IMPLEMENT_IBASE_END


// ---------------------------------------------------------------------------
// csMeshWrapper
// ---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT(csMeshWrapper)
  SCF_IMPLEMENTS_INTERFACE(iMeshWrapper)
  SCF_IMPLEMENTS_INTERFACE(iImposter)
  SCF_IMPLEMENTS_INTERFACE(csMeshWrapper)
  SCF_IMPLEMENTS_INTERFACE(iVisibilityObject)
  SCF_IMPLEMENTS_INTERFACE(iShaderVariableContext)
SCF_IMPLEMENT_IBASE_EXT_END

csMeshWrapper::csMeshWrapper (iMeshWrapper *theParent, iMeshObject *meshobj) :
    csObject ()
{
  DG_TYPE (this, "csMeshWrapper");

  movable.scfParent = (iBase*)(csObject*)this;
  wor_bbox_movablenr = -1;
  movable.SetMeshWrapper (this);
  Parent = theParent;
  if (Parent)
  {
    csParent = (csMeshWrapper*)Parent;
    movable.SetParent (Parent->GetMovable ());
  }
  else
  {
    csParent = 0;
  }

  render_priority = csEngine::currentEngine->GetObjectRenderPriority ();

  last_anim_time = 0;

  shadow_receiver_valid = false;
  shadow_caster_valid = false;
  csMeshWrapper::meshobj = meshobj;
  if (meshobj)
  {
    light_info = SCF_QUERY_INTERFACE (meshobj, iLightingInfo);
    portal_container = SCF_QUERY_INTERFACE (meshobj, iPortalContainer);
    // Only if we have a parent can it possibly be useful to call
    // AddToSectorPortalLists. Because if we don't have a parent yet then
    // we cannot have a sector either. If we have a parent then the parent
    // can have a sector.
    if (csParent)
      AddToSectorPortalLists ();
  }
  factory = 0;
  zbufMode = CS_ZBUF_USE;
  children.SetMesh (this);
  imposter_active = false;
  imposter_mesh = 0;
  cast_hardware_shadow = true;
  draw_after_fancy_stuff = false;

  do_minmax_range = false;
  min_render_dist = -1000000000.0f;
  max_render_dist = 1000000000.0f;

  relevant_lights_valid = false;
  relevant_lights_max = 8;
  relevant_lights_flags.SetAll (CS_LIGHTINGUPDATE_SORTRELEVANCE);

  last_camera = 0;
  last_frame_number = 0;
}

void csMeshWrapper::SetParentContainer (iMeshWrapper* newParent)
{
  Parent = newParent;
  if (Parent)
    csParent = (csMeshWrapper*)Parent;
  else
    csParent = 0;
}

iShadowReceiver* csMeshWrapper::GetShadowReceiver ()
{
  if (!shadow_receiver_valid)
  {
    if (static_lod)
    {
      shadow_receiver_valid = true;
      shadow_receiver = csPtr<iShadowReceiver> (
      	new csStaticShadowReceiver (this));
      return shadow_receiver;
    }

    if (!meshobj) return 0;
    shadow_receiver_valid = true;
    shadow_receiver = SCF_QUERY_INTERFACE (meshobj, iShadowReceiver);
  }
  return shadow_receiver;
}

iShadowCaster* csMeshWrapper::GetShadowCaster ()
{
  if (!shadow_caster_valid)
  {
    if (static_lod)
    {
      shadow_caster_valid = true;
      shadow_caster = csPtr<iShadowCaster> (
      	new csStaticShadowCaster (this));
      return shadow_caster;
    }

    if (!meshobj) return 0;
    shadow_caster_valid = true;
    shadow_caster = SCF_QUERY_INTERFACE (meshobj, iShadowCaster);
  }
  return shadow_caster;
}

void csMeshWrapper::AddToSectorPortalLists ()
{
  if (portal_container)
  {
    int i;
    csMeshWrapper* prev = this;
    csMeshWrapper* m = csParent;
    while (m) { prev = m; m = m->GetCsParent (); }
    const iSectorList *sectors = (prev->GetCsMovable ())
    	.csMovable::GetSectors ();
    for (i = 0; i < sectors->GetCount (); i++)
    {
      iSector *ss = sectors->Get (i);
      if (ss) ss->RegisterPortalMesh ((iMeshWrapper*)this);
    }
  }
}

void csMeshWrapper::ClearFromSectorPortalLists (iSector* sector)
{
  if (portal_container)
  {
    int i;
    csMeshWrapper* prev = this;
    csMeshWrapper* m = csParent;
    while (m) { prev = m; m = m->GetCsParent (); }

    if (sector)
    {
      sector->UnregisterPortalMesh ((iMeshWrapper*)this);
    }
    else
    {
      const iSectorList *sectors = (prev->GetCsMovable ())
      	.csMovable::GetSectors ();
      for (i = 0; i < sectors->GetCount (); i++)
      {
        iSector *ss = sectors->Get (i);
        if (ss) ss->UnregisterPortalMesh ((iMeshWrapper*)this);
      }
    }
  }
}

void csMeshWrapper::SetMeshObject (iMeshObject *meshobj)
{
  ClearFromSectorPortalLists ();

  csMeshWrapper::meshobj = meshobj;
  shadow_receiver_valid = false;
  shadow_caster_valid = false;
  shadow_receiver = 0;
  shadow_caster = 0;

  if (meshobj)
  {
    light_info = SCF_QUERY_INTERFACE (meshobj, iLightingInfo);
    portal_container = SCF_QUERY_INTERFACE (meshobj, iPortalContainer);
    AddToSectorPortalLists ();
  }
  else
  {
    light_info = 0;
    portal_container = 0;
  }
}

csMeshWrapper::~csMeshWrapper ()
{
  delete imposter_mesh;
  ClearFromSectorPortalLists ();
}

void csMeshWrapper::UpdateMove ()
{
  relevant_lights_valid = false;
  int i;
  for (i = 0; i < children.GetCount (); i++)
  {
    iMeshWrapper *spr = children.Get (i);
    spr->GetMovable ()->UpdateMove ();
  }
}

bool csMeshWrapper::SomeParentHasStaticLOD () const
{
  if (!csParent) return false;
  if (csParent->static_lod) return true;
  return csParent->SomeParentHasStaticLOD ();
}

void csMeshWrapper::MoveToSector (iSector *s)
{
  // Only add this mesh to a sector if the parent is the engine.
  // Otherwise we have a hierarchical object and in that case
  // the parent object controls this.
  if (!Parent) s->GetMeshes ()->Add ((iMeshWrapper*)this);
  // If we are a portal container then we have to register ourselves
  // to the sector.
  if (portal_container) s->RegisterPortalMesh ((iMeshWrapper*)this);

  // Fire the new mesh callbacks in the sector.
  ((csSector*)s)->FireNewMesh ((iMeshWrapper*)this);

  int i;
  for (i = 0; i < children.GetCount (); i++)
  {
    iMeshWrapper* spr = children.Get (i);
    csMeshWrapper* cspr = (csMeshWrapper*)spr;
    // If we have children then we call MoveToSector() on them so that
    // any potential portal_containers among them will also register
    // themselves to the sector.
    cspr->MoveToSector (s);
  }
}

void csMeshWrapper::RemoveFromSectors (iSector* sector)
{
  // Fire the remove mesh callbacks in the sector.
  if (sector)
    ((csSector*)sector)->FireRemoveMesh ((iMeshWrapper*)this);

  ClearFromSectorPortalLists (sector);
  int i;
  for (i = 0; i < children.GetCount (); i++)
  {
    iMeshWrapper* spr = children.Get (i);
    csMeshWrapper* cspr = (csMeshWrapper*)spr;
    // If we have children then we call RemoveFromSectors() on them so that
    // any potential portal_containers among them will also unregister
    // themselves from the sector.
    cspr->RemoveFromSectors (sector);
  }

  if (Parent) return ;

  if (sector)
  {
    sector->GetMeshes ()->Remove ((iMeshWrapper*)this);
  }
  else
  {
    const iSectorList *sectors = movable.GetSectors ();
    for (i = 0; i < sectors->GetCount (); i++)
    {
      iSector *ss = sectors->Get (i);
      if (ss)
        ss->GetMeshes ()->Remove ((iMeshWrapper*)this);
    }
  }
}

void csMeshWrapper::SetFlagsRecursive (uint32 mask, uint32 value)
{
  flags.Set (mask, value);
  const iMeshList* ml = GetChildren ();
  if (!ml) return;
  int i;
  for (i = 0 ; i < ml->GetCount () ; i++)
    ml->Get (i)->SetFlagsRecursive (mask, value);
}

void csMeshWrapper::SetZBufModeRecursive (csZBufMode mode)
{
  SetZBufMode (mode);
  const iMeshList* ml = GetChildren ();
  if (!ml) return;
  int i;
  for (i = 0 ; i < ml->GetCount () ; i++)
    ml->Get (i)->SetZBufModeRecursive (mode);
}

void csMeshWrapper::SetRenderPriorityRecursive (long rp)
{
  SetRenderPriority (rp);
  const iMeshList* ml = GetChildren ();
  if (!ml) return;
  int i;
  for (i = 0 ; i < ml->GetCount () ; i++)
    ml->Get (i)->SetRenderPriorityRecursive (rp);
}

void csMeshWrapper::SetRenderPriority (long rp)
{
  render_priority = rp;

  if (Parent) return ;

  int i;
  const iSectorList *sectors = movable.GetSectors ();
  for (i = 0; i < sectors->GetCount (); i++)
  {
    iSector *ss = sectors->Get (i);
    if (ss) ((csSector*)ss)->RelinkMesh ((iMeshWrapper*)this);
  }
}

void csMeshWrapper::SetLightingUpdate (int flags, int num_lights)
{
  relevant_lights_flags.SetAll (flags);
  relevant_lights_max = num_lights;
  relevant_lights_valid = false;
}

const csArray<iLight*>& csMeshWrapper::GetRelevantLights (int /*maxLights*/,
	bool /*desireSorting*/)
{
  bool always_update = relevant_lights_flags.Check (
  	CS_LIGHTINGUPDATE_ALWAYSUPDATE);
  if (!always_update)
  {
    // Check if updating is needed.
    if (relevant_lights_valid)
    {
      // Object didn't move. Now check lights (moved or destroyed).
      bool relevant = true;
      size_t i;
      for (i = 0 ; i < relevant_lights.Length () ; i++)
      {
        iLight* l = relevant_lights[i];
	if (!relevant_lights_ref[i].light)
	{
	  relevant = false;	// Light was removed!
	  break;
	}
	if (l->GetLightNumber () != relevant_lights_ref[i].light_nr)
	{
	  relevant = false;	// Light was removed!
	  break;
	}
      }

      if (relevant)
        return relevant_lights;
    }
  }

  relevant_lights.Empty ();
  relevant_lights_ref.Empty ();

  const iSectorList *movable_sectors = movable.GetSectors ();
  if (movable_sectors->GetCount () > 0 && relevant_lights_max > 0)
  {
    csBox3 box;
    GetFullBBox (box);

    if (relevant_lights_max > relevant_lights.Length ())
      relevant_lights.SetLength (relevant_lights_max);

    iSector *sect = movable_sectors->Get (0);
    int num_lights = csEngine::currentEngine->GetNearbyLights (
        sect,
        box,
        relevant_lights.GetArray (),
        (int)relevant_lights_max);
    relevant_lights.SetLength (num_lights);
    relevant_lights_ref.SetLength (num_lights);
    if (!always_update)
    {
      // Update our ref list.
      int i;
      for (i = 0 ; i < num_lights ; i++)
      {
        relevant_lights_ref[i].light = relevant_lights[i];
        relevant_lights_ref[i].light_nr = relevant_lights[i]->GetLightNumber ();
      }
    }
  }
  relevant_lights_valid = true;
  return relevant_lights;
}

csRenderMesh** csMeshWrapper::GetRenderMeshes (int& n, iRenderView* rview, 
					       uint32 frustum_mask)
{
  //if (imposter_active && CheckImposterRelevant (rview))
    //if (DrawImposter (rview))
      //return;

  // Callback are traversed in reverse order so that they can safely
  // delete themselves.
  size_t i = draw_cb_vector.Length ();
  while (i > 0)
  {
    i--;
    iMeshDrawCallback* cb = draw_cb_vector.Get (i);
    if (!cb->BeforeDrawing ((iMeshWrapper*)this, rview))
    {
      n = 0;
      return 0;
    }
  }

  // Here we check the CS_ENTITY_NOCLIP flag. If that flag is set
  // we will only render the object once in a give frame/camera combination.
  // So if multiple portals arrive in a sector containing this object the
  // object will be rendered at the first portal and not clipped to that
  // portal (as is usually the case).
  csRenderContext* old_ctxt = 0;

  if (flags.Check (CS_ENTITY_NOCLIP))
  {
    csRenderView* csrview = (csRenderView*)rview;
    csRenderContext* ctxt = csrview->GetCsRenderContext ();

    if (last_frame_number == rview->GetCurrentFrameNumber () &&
    	last_camera == ctxt->icamera)
    {
      n = 0;
      return 0;
    }
    last_camera = ctxt->icamera;
    last_frame_number = rview->GetCurrentFrameNumber ();
    old_ctxt = ctxt;
    // Go back to top-level context.
    while (ctxt->previous) ctxt = ctxt->previous;
    csrview->SetCsRenderContext (ctxt);
  }

  csTicks lt = csEngine::currentEngine->GetLastAnimationTime ();
  meshobj->NextFrame (lt, movable.GetPosition ());
    
  csMeshWrapper *meshwrap = this;
  last_anim_time = lt;
  csMeshWrapper* lastparent = meshwrap;
  csMeshWrapper* parent = csParent;
  while (parent != 0)
  {
    parent->GetMeshObject()->PositionChild (lastparent->GetMeshObject(), lt);
    lastparent = parent;
    parent = parent->csParent;
  }

  csRenderMesh** rmeshes = meshobj->GetRenderMeshes (n, rview, &movable,
  	old_ctxt != 0 ? 0 : frustum_mask);
  if (old_ctxt)
  {
    csRenderView* csrview = (csRenderView*)rview;
    csrview->SetCsRenderContext (old_ctxt);
  }
  return rmeshes;
}

//----- Min/Max Distance Range ----------------------------------------------

void csMeshWrapper::ClearMinVariable ()
{
  if (var_min_render_dist)
  {
    var_min_render_dist->RemoveListener (var_min_render_dist_listener);
    var_min_render_dist_listener = 0;
    var_min_render_dist = 0;
  }
}

void csMeshWrapper::ClearMaxVariable ()
{
  if (var_max_render_dist)
  {
    var_max_render_dist->RemoveListener (var_max_render_dist_listener);
    var_max_render_dist_listener = 0;
    var_max_render_dist = 0;
  }
}

void csMeshWrapper::ResetMinMaxRenderDistance ()
{
  do_minmax_range = false;
  min_render_dist = -1000000000.0f;
  max_render_dist = -1000000000.0f;
  ClearMinVariable ();
  ClearMaxVariable ();
}

void csMeshWrapper::SetMinimumRenderDistance (float min)
{
  do_minmax_range = true;
  min_render_dist = min;
  ClearMinVariable ();
}

void csMeshWrapper::SetMaximumRenderDistance (float max)
{
  do_minmax_range = true;
  max_render_dist = max;
  ClearMaxVariable ();
}

void csMeshWrapper::SetMinimumRenderDistanceVar (iSharedVariable* min)
{
  do_minmax_range = true;
  ClearMinVariable ();
  var_min_render_dist = min;
  if (var_min_render_dist)
  {
    var_min_render_dist_listener = csPtr<csLODListener> (
    	new csLODListener (&min_render_dist));
    var_min_render_dist->AddListener (var_min_render_dist_listener);
    min_render_dist = var_min_render_dist->Get ();
  }
}

void csMeshWrapper::SetMaximumRenderDistanceVar (iSharedVariable* max)
{
  do_minmax_range = true;
  ClearMaxVariable ();
  var_max_render_dist = max;
  if (var_max_render_dist)
  {
    var_max_render_dist_listener = csPtr<csLODListener> (
    	new csLODListener (&max_render_dist));
    var_max_render_dist->AddListener (var_max_render_dist_listener);
    max_render_dist = var_max_render_dist->Get ();
  }
}

//----- Static LOD ----------------------------------------------------------

iLODControl* csMeshWrapper::CreateStaticLOD ()
{
  shadow_receiver_valid = false;
  shadow_caster_valid = false;
  static_lod = csPtr<csStaticLODMesh> (new csStaticLODMesh ());
  return static_lod;
}

void csMeshWrapper::DestroyStaticLOD ()
{
  shadow_receiver_valid = false;
  shadow_caster_valid = false;
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
  shadow_receiver_valid = false;
  shadow_caster_valid = false;
}

void csMeshWrapper::AddMeshToStaticLOD (int lod, iMeshWrapper* mesh)
{
  if (!static_lod) return;	// No static lod, nothing to do here.
  csArray<iMeshWrapper*>& meshes_for_lod = static_lod->GetMeshesForLOD (lod);
  meshes_for_lod.Push (mesh);
  shadow_receiver_valid = false;
  shadow_caster_valid = false;
}

//---------------------------------------------------------------------------

bool csMeshWrapper::CheckImposterRelevant (iRenderView *rview)
{
  float wor_sq_dist = GetSquaredDistance (rview);
  float dist = min_imposter_distance->Get ();
  return (wor_sq_dist > dist*dist);
}

bool csMeshWrapper::DrawImposter (iRenderView *rview)
{
  // Check for imposter existence.  If not, create it.
  if (!imposter_mesh)
  {
    return false;
  }

  // Check for imposter already ready
  if (!imposter_mesh->GetImposterReady ())
    return false;

  // Check for too much camera movement since last imposter render
  if (!imposter_mesh->CheckIncidenceAngle (rview,
	imposter_rotation_tolerance->Get ()))
    return false;

  // Else draw imposter as-is.
  imposter_mesh->Draw (rview);
  return true;
}

void csMeshWrapper::SetImposterActive (bool flag)
{
  imposter_active = flag;
  if (flag)
  {
    imposter_mesh = new csImposterMesh (this);
    imposter_mesh->SetImposterReady (false);
  }
}

csHitBeamResult csMeshWrapper::HitBeamOutline (
  const csVector3 &start,
  const csVector3 &end)
{
  csHitBeamResult rc;
  rc.hit = meshobj->HitBeamOutline (start, end, rc.isect, &rc.r);
  return rc;
}

bool csMeshWrapper::HitBeamOutline (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr)
{
  return meshobj->HitBeamOutline (start, end, isect, pr);
}

csHitBeamResult csMeshWrapper::HitBeamObject (
  const csVector3 &start,
  const csVector3 &end)
{
  csHitBeamResult rc;
  rc.hit = meshobj->HitBeamObject (start, end, rc.isect, &rc.r,
  	&rc.polygon_idx);
  return rc;
}

bool csMeshWrapper::HitBeamObject (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr, int* polygon_idx)
{
  return meshobj->HitBeamObject (start, end, isect, pr, polygon_idx);
}

csHitBeamResult csMeshWrapper::HitBeam (
  const csVector3 &start,
  const csVector3 &end)
{
  csHitBeamResult rc;
  rc.hit = HitBeam (start, end, rc.isect, &rc.r);
  return rc;
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

csEllipsoid csMeshWrapper::GetRadius () const
{
  csEllipsoid e;
  GetRadius (e.GetRadius (), e.GetCenter ());
  return e;
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

float csMeshWrapper::GetSquaredDistance (iRenderView *rview)
{
  iCamera* camera = rview->GetCamera ();
  // calculate distance from camera to mesh
  csBox3 obox;
  GetObjectModel ()->GetObjectBoundingBox (obox);
  csVector3 obj_center = (obox.Min () + obox.Max ()) / 2;
  csVector3 wor_center;
  if (movable.IsFullTransformIdentity ())
    wor_center = obj_center;
  else
    wor_center = movable.GetFullTransform ().This2Other (obj_center);
  csVector3 cam_origin = camera->GetTransform ().GetOrigin ();
  float wor_sq_dist = csSquaredDist::PointPoint (cam_origin, wor_center);
  return wor_sq_dist;
}

void csMeshWrapper::GetFullBBox (csBox3& box)
{
  GetObjectModel ()->GetObjectBoundingBox (box);
  iMovable* mov = &movable;
  while (mov)
  {
    if (!mov->IsTransformIdentity ())
    {
      const csReversibleTransform& trans = mov->GetTransform ();
      csBox3 b (trans.This2Other (box.GetCorner (0)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (1)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (2)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (3)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (4)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (5)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (6)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (7)));
      box = b;
    }
    mov = mov->GetParent ();
  }
}


void csMeshWrapper::PlaceMesh ()
{
  iSectorList *movable_sectors = movable.GetSectors ();
  if (movable_sectors->GetCount () == 0) return ; // Do nothing
  csSphere sphere;
  csVector3 radius;
  GetObjectModel ()->GetRadius (radius, sphere.GetCenter ());

  iSector *sector = movable_sectors->Get (0);
  movable.SetSector (sector);       // Make sure all other sectors are removed

  // Transform the sphere from object to world space.
  float max_radius = radius.x;
  if (max_radius < radius.y) max_radius = radius.y;
  if (max_radius < radius.z) max_radius = radius.z;
  sphere.SetRadius (max_radius);
  if (!movable.IsFullTransformIdentity ())
    sphere = movable.GetFullTransform ().This2Other (sphere);
  max_radius = sphere.GetRadius ();
  float max_sq_radius = max_radius * max_radius;

  csRef<iMeshWrapperIterator> it = csEngine::currentEngine
  	->GetNearbyMeshes (sector, sphere.GetCenter (), max_radius, true);

  int j;
  while (it->HasNext ())
  {
    iMeshWrapper* mesh = it->Next ();
    iPortalContainer* portals = mesh->GetPortalContainer ();
    if (!portals) continue;	// No portals.
    int pc_count = portals->GetPortalCount ();

    for (j = 0 ; j < pc_count ; j++)
    {
      iPortal *portal = portals->GetPortal (j);
      iSector *dest_sector = portal->GetSector ();
      if (movable_sectors->Find (dest_sector) == -1)
      {
        const csPlane3 &pl = portal->GetWorldPlane ();

        float sqdist = csSquaredDist::PointPlane (sphere.GetCenter (), pl);
        if (sqdist <= max_sq_radius)
        {
          // Plane of portal is close enough.
          // If N is the normal of the portal plane then we
          // can use that to calculate the point on the portal plane.
          csVector3 testpoint = sphere.GetCenter () + pl.Normal () * csQsqrt (
                  sqdist);
          if (portal->PointOnPolygon (testpoint))
            movable_sectors->Add (dest_sector);
        }
      }
    }
  }
}

csHitBeamResult csMeshWrapper::HitBeamBBox (
  const csVector3 &start,
  const csVector3 &end)
{
  csHitBeamResult rc;
  csBox3 b;
  GetObjectModel ()->GetObjectBoundingBox (b);

  csSegment3 seg (start, end);
  rc.facehit = csIntersect3::BoxSegment (b, seg, rc.isect, &rc.r);
  return rc;
}

int csMeshWrapper::HitBeamBBox (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr)
{
  csBox3 b;
  GetObjectModel ()->GetObjectBoundingBox (b);

  csSegment3 seg (start, end);
  return csIntersect3::BoxSegment (b, seg, isect, pr);
}

const csBox3& csMeshWrapper::GetWorldBoundingBox ()
{
  if (wor_bbox_movablenr != movable.GetUpdateNumber ())
  {
    wor_bbox_movablenr = movable.GetUpdateNumber ();

    if (movable.IsFullTransformIdentity ())
      GetObjectModel ()->GetObjectBoundingBox (wor_bbox);
    else
    {
      csBox3 obj_bbox;
      GetObjectModel ()->GetObjectBoundingBox (obj_bbox);

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
  }
  return wor_bbox;
}

void csMeshWrapper::GetWorldBoundingBox (csBox3 &cbox)
{
  cbox = GetWorldBoundingBox ();
}

csBox3 csMeshWrapper::GetTransformedBoundingBox (
  const csReversibleTransform &trans)
{
  csBox3 cbox, box;
  GetObjectModel ()->GetObjectBoundingBox (box);
  cbox.StartBoundingBox (trans * box.GetCorner (0));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (1));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (2));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (3));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (4));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (5));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (6));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (7));
  return cbox;
}

void csMeshWrapper::GetTransformedBoundingBox (
  const csReversibleTransform &trans,
  csBox3 &cbox)
{
  cbox = GetTransformedBoundingBox (trans);
}

csScreenBoxResult csMeshWrapper::GetScreenBoundingBox (iCamera *camera)
{
  csScreenBoxResult rc;
  rc.distance = GetScreenBoundingBox (camera, rc.sbox, rc.cbox);
  return rc;
}

float csMeshWrapper::GetScreenBoundingBox (
  iCamera *camera,
  csBox2 &sbox,
  csBox3 &cbox)
{
  csVector2 oneCorner;
  csReversibleTransform tr_o2c = camera->GetTransform ();
  if (!movable.IsFullTransformIdentity ())
    tr_o2c /= movable.GetFullTransform ();
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

// ---------------------------------------------------------------------------
// csMeshFactoryWrapper
// ---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE_EXT(csMeshFactoryWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iMeshFactoryWrapper)
  SCF_IMPLEMENTS_INTERFACE(iShaderVariableContext)
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

  zbufMode = CS_ZBUF_USE;
  render_priority = csEngine::currentEngine->GetObjectRenderPriority ();
}

csMeshFactoryWrapper::csMeshFactoryWrapper ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
  parent = 0;
  children.SetMeshFactory (this);

  zbufMode = CS_ZBUF_USE;
  render_priority = csEngine::currentEngine->GetObjectRenderPriority ();
}

csMeshFactoryWrapper::~csMeshFactoryWrapper ()
{
  // This line MUST be here to ensure that the children are not
  // removed after the destructor has already finished.
  children.RemoveAll ();
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiMeshFactoryWrapper);
}

void csMeshFactoryWrapper::SetZBufModeRecursive (csZBufMode mode)
{
  SetZBufMode (mode);
  const iMeshFactoryList* ml = &children;
  if (!ml) return;
  int i;
  for (i = 0 ; i < ml->GetCount () ; i++)
    ml->Get (i)->SetZBufModeRecursive (mode);
}

void csMeshFactoryWrapper::SetRenderPriorityRecursive (long rp)
{
  SetRenderPriority (rp);
  const iMeshFactoryList* ml = &children;
  if (!ml) return;
  int i;
  for (i = 0 ; i < ml->GetCount () ; i++)
    ml->Get (i)->SetRenderPriorityRecursive (rp);
}

void csMeshFactoryWrapper::SetRenderPriority (long rp)
{
  render_priority = rp;
}

void csMeshFactoryWrapper::SetMeshObjectFactory (iMeshObjectFactory *meshFact)
{
  csMeshFactoryWrapper::meshFact = meshFact;
}

iMeshWrapper *csMeshFactoryWrapper::NewMeshObject ()
{
  csRef<iMeshObject> basemesh = meshFact->NewInstance ();
  iMeshWrapper *mesh = (iMeshWrapper*)(new csMeshWrapper (0, basemesh));
  basemesh->SetLogicalParent (mesh);

  if (GetName ()) mesh->QueryObject ()->SetName (GetName ());
  mesh->SetFactory (&scfiMeshFactoryWrapper);
  mesh->SetRenderPriority (render_priority);
  mesh->SetZBufMode (zbufMode);

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
        size_t j;
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
  listener.AttachNew (new NameChangeListener (this));
}

csMeshList::~csMeshList ()
{
  RemoveAll ();
  SCF_DESTRUCT_IBASE ();
}

void csMeshList::NameChanged (iObject* object, const char* oldname,
  	const char* newname)
{
  csRef<iMeshWrapper> mesh = SCF_QUERY_INTERFACE (object, iMeshWrapper);
  CS_ASSERT (mesh != 0);
  if (oldname) meshes_hash.Delete (oldname, mesh);
  if (newname) meshes_hash.Put (newname, mesh);
}

iMeshWrapper* csMeshList::FindByNameWithChild (const char *Name) const
{
  char const* p = strchr (Name, ':');
  if (!p) return meshes_hash.Get (Name, 0);

  int firstsize = p-Name;
  csString firstName;
  firstName.Append (Name, firstsize);

  iMeshWrapper* m = meshes_hash.Get (csStrKey (firstName), 0);
  if (!m) return 0;
  return m->GetChildren ()->FindByName (p+1);
}

int csMeshList::Add (iMeshWrapper *obj)
{
  PrepareMesh (obj);
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    meshes_hash.Put (name, obj);
  obj->QueryObject ()->AddNameChangeListener (listener);
  return (int)list.Push (obj);
}

bool csMeshList::Remove (iMeshWrapper *obj)
{
  FreeMesh (obj);
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    meshes_hash.Delete (name, obj);
  list.Delete (obj);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  return true;
}

bool csMeshList::Remove (int n)
{
  FreeMesh (list[n]);
  iMeshWrapper* obj = list[n];
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    meshes_hash.Delete (name, obj);
  list.DeleteIndex (n);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  return true;
}

void csMeshList::RemoveAll ()
{
  size_t i;
  for (i = 0 ; i < list.Length () ; i++)
  {
    list[i]->QueryObject ()->RemoveNameChangeListener (listener);
    FreeMesh (list[i]);
  }
  meshes_hash.DeleteAll ();
  list.DeleteAll ();
}

int csMeshList::Find (iMeshWrapper *obj) const
{
  return (int)list.Find (obj);
}

iMeshWrapper *csMeshList::FindByName (const char *Name) const
{
  if (strchr (Name, ':'))
    return FindByNameWithChild (Name);
  else
    return meshes_hash.Get (Name, 0);
}

//--------------------------------------------------------------------------
// csMeshMeshList
//--------------------------------------------------------------------------
void csMeshMeshList::PrepareMesh (iMeshWrapper* child)
{
  CS_ASSERT (mesh != 0);
  csMeshList::PrepareMesh (child);

  // Unlink the mesh from the engine or another parent.
  csMeshWrapper* cchild = (csMeshWrapper*)child;
  // Make sure that if this child is a portal container that we first
  // uregister it from any sectors it may be in.
  cchild->ClearFromSectorPortalLists ();
  iMeshWrapper *oldParent = child->GetParentContainer ();
  if (oldParent)
    oldParent->GetChildren ()->Remove (child);
  else
    csEngine::currentEngine->GetMeshes ()->Remove (child);

  /* csSector->PrepareMesh tells the culler about the mesh
     (since removing the mesh above also removes it from the culler...) */
  // First we find the top-level parent.
  iMeshWrapper* toplevel = (iMeshWrapper*)mesh;
  while (toplevel->GetParentContainer ())
    toplevel = toplevel->GetParentContainer ();
  iMovable* mov = toplevel->GetMovable ();
  iSectorList* sl = mov->GetSectors ();
  for (int i = 0 ; i < sl->GetCount() ; i++)
  {
    csSector* sector = (csSector*)(sl->Get (i));
    sector->UnprepareMesh (child);
    sector->PrepareMesh (child);
  }

  child->SetParentContainer ((iMeshWrapper*)mesh);
  (cchild->GetCsMovable ()).csMovable::SetParent (&mesh->GetCsMovable ());
  // Readd our child to sectors if it happens to be a portal container.
  cchild->AddToSectorPortalLists ();
}

void csMeshMeshList::FreeMesh (iMeshWrapper* item)
{
  CS_ASSERT (mesh != 0);

  csMeshWrapper* citem = (csMeshWrapper*)item;
  // Make sure that if this child is a portal container that we first
  // uregister it from any sectors it may be in.
  citem->ClearFromSectorPortalLists ();

  for (int i = 0 ; i < mesh->GetCsMovable().GetSectors()->GetCount() ; i++)
  {
    iSector* isector = (mesh->GetCsMovable ()).csMovable::GetSectors ()->Get (i);
    csSector* sector = (csSector*)isector;
    sector->UnprepareMesh (item);
  }

  item->SetParentContainer (0);
  item->GetMovable ()->SetParent (0);

  mesh->RemoveMeshFromStaticLOD (item);

  csMeshList::FreeMesh (item);
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
  listener.AttachNew (new NameChangeListener (this));
}

csMeshFactoryList::~csMeshFactoryList ()
{
  RemoveAll ();
  SCF_DESTRUCT_IBASE ();
}

void csMeshFactoryList::NameChanged (iObject* object, const char* oldname,
  	const char* newname)
{
  csRef<iMeshFactoryWrapper> mesh = SCF_QUERY_INTERFACE (object,
    iMeshFactoryWrapper);
  CS_ASSERT (mesh != 0);
  if (oldname) factories_hash.Delete (oldname, mesh);
  if (newname) factories_hash.Put (newname, mesh);
}

int csMeshFactoryList::Add (iMeshFactoryWrapper *obj)
{
  PrepareFactory (obj);
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    factories_hash.Put (name, obj);
  obj->QueryObject ()->AddNameChangeListener (listener);
  return (int)list.Push (obj);
}

bool csMeshFactoryList::Remove (iMeshFactoryWrapper *obj)
{
  FreeFactory (obj);
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    factories_hash.Delete (name, obj);
  list.Delete (obj);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  return true;
}

bool csMeshFactoryList::Remove (int n)
{
  return Remove (Get (n));
}

void csMeshFactoryList::RemoveAll ()
{
  size_t i;
  for (i = 0 ; i < list.Length () ; i++)
  {
    list[i]->QueryObject ()->RemoveNameChangeListener (listener);
    FreeFactory (list[i]);
  }
  factories_hash.DeleteAll ();
  list.DeleteAll ();
}

int csMeshFactoryList::Find (iMeshFactoryWrapper *obj) const
{
  return (int)list.Find (obj);
}

iMeshFactoryWrapper *csMeshFactoryList::FindByName (
  const char *Name) const
{
  if (!Name) return 0;
  return factories_hash.Get (Name, 0);
}

//--------------------------------------------------------------------------
// csMeshFactoryFactoryList
//--------------------------------------------------------------------------
void csMeshFactoryFactoryList::PrepareFactory (iMeshFactoryWrapper* child)
{
  CS_ASSERT (meshfact != 0);
  csMeshFactoryList::PrepareFactory (child);

  // unlink the factory from another possible parent.
  if (child->GetParentContainer ())
    child->GetParentContainer ()->GetChildren ()->Remove (child);

  child->SetParentContainer (&meshfact->scfiMeshFactoryWrapper);
}

void csMeshFactoryFactoryList::FreeFactory (iMeshFactoryWrapper* item)
{
  CS_ASSERT (meshfact != 0);
  item->SetParentContainer (0);
  meshfact->RemoveFactoryFromStaticLOD (item);
  csMeshFactoryList::FreeFactory (item);
}
