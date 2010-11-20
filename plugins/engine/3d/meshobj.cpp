/*
    Copyright (C) 2000-2007 by Jorrit Tyberghein

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
#include "cstool/rviewclipper.h"
#include "imesh/objmodel.h"
#include "igeom/clip2d.h"
#include "plugins/engine/3d/camera.h"
#include "plugins/engine/3d/sector.h"
#include "plugins/engine/3d/meshobj.h"
#include "plugins/engine/3d/meshfact.h"
#include "plugins/engine/3d/meshlod.h"
#include "plugins/engine/3d/light.h"
#include "plugins/engine/3d/engine.h"
#include "iengine/portal.h"
#include "iengine/material.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"

#include "reflectomotron3000.h"

CS_LEAKGUARD_IMPLEMENT (csMeshWrapper);

using namespace CS_PLUGIN_NAMESPACE_NAME(Engine);

// ---------------------------------------------------------------------------
// csMeshWrapper
// ---------------------------------------------------------------------------
csBox3 csMeshWrapper::infBBox (-CS_BOUNDINGBOX_MAXVALUE, -CS_BOUNDINGBOX_MAXVALUE,
                               -CS_BOUNDINGBOX_MAXVALUE, CS_BOUNDINGBOX_MAXVALUE,
                               CS_BOUNDINGBOX_MAXVALUE,CS_BOUNDINGBOX_MAXVALUE);

csMeshWrapper::csMeshWrapper (csEngine* engine, iMeshObject *meshobj) 
  : scfImplementationType (this), instancing (0),
    engine (engine)
{
//  movable.scfParent = this; //@TODO: CHECK THIS
  wor_bbox_movablenr = -1;
  movable.SetMeshWrapper (this);

  render_priority = engine->GetObjectRenderPriority ();

  last_anim_time = 0;

  //shadow_receiver_valid = false;
  //shadow_caster_valid = false;
  csMeshWrapper::meshobj = meshobj;
  if (meshobj)
  {
    //light_info = scfQueryInterface<iLightingInfo> (meshobj);
    portal_container = scfQueryInterface<iPortalContainer> (meshobj);
    // Only if we have a parent can it possibly be useful to call
    // AddToSectorPortalLists. Because if we don't have a parent yet then
    // we cannot have a sector either. If we have a parent then the parent
    // can have a sector.
    if (movable.GetParent ())
      AddToSectorPortalLists ();
  }
  factory = 0;
  zbufMode = CS_ZBUF_USE;
  using_imposter = false;

  do_minmax_range = false;
  min_render_dist = -1000000000.0f;
  max_render_dist = 1000000000.0f;

  last_camera = 0;
  last_frame_number = 0;

  // Set creation time on the mesh
  csRef<csShaderVariable> sv_creation_time;
  sv_creation_time.AttachNew(new csShaderVariable(engine->id_creation_time));
  sv_creation_time->SetValue((float)engine->virtualClock->GetCurrentTicks() / 1000.0f);
  GetSVContext()->AddVariable(sv_creation_time);

  SetDefaultEnvironmentTexture ();
}

void csMeshWrapper::SetFactory (iMeshFactoryWrapper* factory)
{
  // Check if we're instancing, if so then set the shadervar and instance factory.
  csRef<csShaderVariable> factoryTransformVars = factory->GetInstances();
  if(factoryTransformVars)
  {
    csRef<csShaderVariable>& fadeFactors = GetInstancingData()->fadeFactors;
    csRef<csShaderVariable>& transformVars = GetInstancingData()->transformVars;
  
    transformVars = factoryTransformVars;
    GetSVContext()->AddVariable(transformVars);
    factory = factory->GetInstanceFactory();

    csRef<iShaderVarStringSet> SVstrings = csQueryRegistryTagInterface<iShaderVarStringSet> (
    engine->objectRegistry, "crystalspace.shader.variablenameset");
    CS::ShaderVarStringID varFadeFactor = SVstrings->Request ("alpha factor");

    fadeFactors.AttachNew (new csShaderVariable (varFadeFactor));
    fadeFactors->SetType (csShaderVariable::ARRAY);
    fadeFactors->SetArraySize (0);

    for(size_t i=0; i<transformVars->GetArraySize(); ++i)
    {
      csRef<csShaderVariable> fadeFactor;
      fadeFactor.AttachNew(new csShaderVariable);
      fadeFactor->SetValue(1.0f);
      fadeFactors->AddVariableToArray(fadeFactor);
    }

    GetSVContext()->AddVariable(fadeFactors);
  }

  csMeshWrapper::factory = factory;
  SetParentContext (factory ? factory->GetSVContext() : 0);
}

void csMeshWrapper::SelfDestruct ()
{
  engine->GetMeshes ()->Remove (static_cast<iMeshWrapper*> (this));
}

CS_IMPLEMENT_STATIC_CLASSVAR_REF(csMeshWrapper, instancingAlloc, GetInstancingAlloc,
  csMeshWrapper::InstancingAlloc, ())
  
csMeshWrapper::InstancingData* csMeshWrapper::GetInstancingData()
{
  if (instancing == 0)
    instancing = GetInstancingAlloc().Alloc();
  return instancing;
}

csShaderVariable* csMeshWrapper::AddInstance(csVector3& position, csMatrix3& rotation)
{
  csRef<csShaderVariable>& fadeFactors = GetInstancingData()->fadeFactors;
  csRef<csShaderVariable>& transformVars = GetInstancingData()->transformVars;
  
  if(!transformVars.IsValid())
  {
    csRef<iShaderVarStringSet> SVstrings = csQueryRegistryTagInterface<iShaderVarStringSet>(
      engine->objectRegistry, "crystalspace.shader.variablenameset");
    CS::ShaderVarStringID varTransform = SVstrings->Request("instancing transforms");
    CS::ShaderVarStringID varFadeFactor = SVstrings->Request ("alpha factor");

    transformVars.AttachNew(new csShaderVariable(varTransform));
    transformVars->SetType (csShaderVariable::ARRAY);
    transformVars->SetArraySize (0);
    GetSVContext()->AddVariable(transformVars);

    fadeFactors.AttachNew (new csShaderVariable (varFadeFactor));
    fadeFactors->SetType (csShaderVariable::ARRAY);
    fadeFactors->SetArraySize (0);
    GetSVContext()->AddVariable(fadeFactors);
  }

  csRef<csShaderVariable> fadeFactor;
  fadeFactor.AttachNew(new csShaderVariable);
  fadeFactor->SetValue(1.0f);
  fadeFactors->AddVariableToArray(fadeFactor);

  csRef<csShaderVariable> transformVar;
  transformVar.AttachNew(new csShaderVariable);
  csReversibleTransform tr(rotation.GetInverse(), position);
  transformVar->SetValue (tr);
  transformVars->AddVariableToArray(transformVar);
  
  instancing->instancingTransformsDirty = true;

  return transformVar;
}

void csMeshWrapper::RemoveInstance(csShaderVariable* instance)
{
  CS_ASSERT (instancing != 0);
  csRef<csShaderVariable>& fadeFactors = GetInstancingData()->fadeFactors;
  csRef<csShaderVariable>& transformVars = instancing->transformVars;
  size_t element = transformVars->FindArrayElement(instance);
  transformVars->RemoveFromArray(element);
  fadeFactors->RemoveFromArray(element);
  instancing->instancingTransformsDirty = true;
}

// Stuff to fixup rendermesh bboxes in case of instancing

csMeshWrapper::InstancingData::RenderMeshesSet::RenderMeshesSet () : n (0), meshArray (0),
  meshes (0)
{}

csMeshWrapper::InstancingData::RenderMeshesSet::~RenderMeshesSet ()
{
  cs_free (meshArray);
  cs_free (meshes);
}

void csMeshWrapper::InstancingData::RenderMeshesSet::CopyOriginalMeshes (int n,
  csRenderMesh** origMeshes)
{
  /* This _deliberately_ does not do proper C++ construction/destruction/
     copying.
     csRenderMesh contains a number of csRef<>s and such, but we're really
     only interested in the bbox members, and neither care about nor want to
     pay for the others. Also, the meshes only have to be valid for one
     frame - and if the original meshes are the simple copies are, too. */
  if (n != this->n)
  {
    meshes = (csRenderMesh*)cs_realloc (meshes, n * sizeof (csRenderMesh));
    meshArray = (csRenderMesh**)cs_realloc (meshArray, n * sizeof (csRenderMesh*));
    for (int i = 0; i < n; i++)
      meshArray[i] = meshes+i;
    
    this->n = n;
  }
  for (int i = 0; i < n; i++)
    memcpy (meshes+i, origMeshes[i], sizeof (csRenderMesh));
}
  
csBox3 csMeshWrapper::AdjustBboxForInstances (const csBox3& origBox) const
{
  CS_ASSERT (instancing != 0);
  csRef<csShaderVariable>& transformVars = instancing->transformVars;
  CS_ASSERT (transformVars->GetType() == csShaderVariable::ARRAY);
  size_t numInst = transformVars->GetArraySize ();
  if (numInst == 0) return origBox;
  csBox3 newBox;
  for (size_t i = 0; i < numInst; i++)
  {
    csReversibleTransform tf;
    transformVars->GetArrayElement (i)->GetValue (tf);
    csBox3 box_tf = tf.This2Other (origBox);
    newBox += box_tf;
  }
  return newBox;
}

csRenderMesh** csMeshWrapper::FixupRendermeshesForInstancing (int n,
  csRenderMesh** meshes)
{
  CS_ASSERT (instancing != 0);
  csArray<InstancingData::InstancingBbox>& instancingBoxes = instancing->instancingBoxes;
  
  // Check old bboxes and recompute if necessary
  instancingBoxes.SetSize (n);
  for (int i = 0; i < n; i++)
  {
    const csBox3& origBox = meshes[i]->bbox;
    if (origBox != instancingBoxes[i].oldBox)
    {
      instancingBoxes[i].oldBox = origBox;
      instancingBoxes[i].newBox = AdjustBboxForInstances (origBox);
    }
  }
  
  // Get a new set of rendermeshes valid for this frame...
  csFrameDataHolder<InstancingData::RenderMeshesSet>& instancingRMs =
    instancing->instancingRMs;
  bool created;
  InstancingData::RenderMeshesSet& meshesSet = instancingRMs.GetUnusedData (created,
    engine->csEngine::GetCurrentFrameNumber());
  meshesSet.CopyOriginalMeshes (n, meshes);
  // ... and change the bounding boxes
  for (int i = 0; i < n; i++)
  {
    meshesSet.meshes[i].bbox = instancingBoxes[i].newBox;
  }
  
  return meshesSet.meshArray;
}

void csMeshWrapper::AddToSectorPortalLists ()
{
  if (portal_container)
  {
    int i;
    csMovable* prev = &movable;
    csMovable* m = movable.GetParent ();
    while (m) { prev = m; m = m->GetParent (); }
    const iSectorList *sectors = prev->GetSectors ();
    for (i = 0; i < sectors->GetCount (); i++)
    {
      iSector *ss = sectors->Get (i);
      if (ss) ((csSector*)ss)->RegisterPortalMesh ((iMeshWrapper*)this);
    }
  }
}

void csMeshWrapper::ClearFromSectorPortalLists (iSector* sector)
{
  if (portal_container)
  {
    int i;
    if (sector)
    {
      ((csSector*)sector)->UnregisterPortalMesh ((iMeshWrapper*)this);
    }
    else
    {
      csMovable* prev = &movable;
      csMovable* m = movable.GetParent ();
      while (m) { prev = m; m = m->GetParent (); }

      const iSectorList *sectors = prev->GetSectors ();
      for (i = 0; i < sectors->GetCount (); i++)
      {
        iSector *ss = sectors->Get (i);
        if (ss) ((csSector*)ss)->UnregisterPortalMesh ((iMeshWrapper*)this);
      }
    }
  }
}

void csMeshWrapper::SetMeshObject (iMeshObject *meshobj)
{
  ClearFromSectorPortalLists ();

  csMeshWrapper::meshobj = meshobj;

  if (meshobj)
  {
    //light_info = scfQueryInterface<iLightingInfo> (meshobj);
    portal_container = scfQueryInterface<iPortalContainer> (meshobj);
    AddToSectorPortalLists ();
  }
  else
  {
    //light_info = 0;
    portal_container = 0;
  }
}

csMeshWrapper::~csMeshWrapper ()
{
  if (using_imposter)
  {
    iImposterFactory* factwrap = dynamic_cast<iImposterFactory*> (factory);
    factwrap->RemoveImposter (this);
  }

  // Copy the array because we are going to unlink the children.
  csRefArray<iSceneNode> children = movable.GetChildren ();
  size_t i;
  for (i = 0 ; i < children.GetSize () ; i++)
    children[i]->SetParent (0);
  ClearFromSectorPortalLists ();
  
  if (instancing != 0) GetInstancingAlloc().Free (instancing);
}

void csMeshWrapper::UpdateMove ()
{
}

bool csMeshWrapper::SomeParentHasStaticLOD () const
{
  if (!movable.GetParent ()) return false;
  iSceneNode* parent_node = movable.GetParent ()->GetSceneNode ();
  iMeshWrapper* parent_mesh = parent_node->QueryMesh ();
  while (!parent_mesh)
  {
    parent_node = parent_node->GetParent ();
    if (!parent_node) return false;
    parent_mesh = parent_node->QueryMesh ();
  }

  if (((csMeshWrapper*)parent_mesh)->static_lod) return true;
  return ((csMeshWrapper*)parent_mesh)->SomeParentHasStaticLOD ();
}

void csMeshWrapper::MoveToSector (iSector *s)
{
  // Only move if the meshwrapper is valid.
  iMeshWrapper* mw = (iMeshWrapper*)this;
  if(!mw->GetMeshObject())
    return;

  // Only add this mesh to a sector if the parent is the engine.
  // Otherwise we have a hierarchical object and in that case
  // the parent object controls this.
  if (!movable.GetParent ())
    s->GetMeshes ()->Add (mw);

  // If we are a portal container then we have to register ourselves
  // to the sector.
  if (portal_container)
    ((csSector*)s)->RegisterPortalMesh (mw);

  // Fire the new mesh callbacks in the sector.
  ((csSector*)s)->FireNewMesh (mw);

  const csRefArray<iSceneNode>& children = movable.GetChildren ();
  size_t i;
  for (i = 0; i < children.GetSize (); i++)
  {
    iMeshWrapper* spr = children[i]->QueryMesh ();
    // @@@ What for other types of objects like lights and camera???
    if (spr)
    {
      csMeshWrapper* cspr = (csMeshWrapper*)spr;
      // If we have children then we call MoveToSector() on them so that
      // any potential portal_containers among them will also register
      // themselves to the sector.
      cspr->MoveToSector (s);
    }
  }
}

void csMeshWrapper::RemoveFromSectors (iSector* sector)
{
  // Fire the remove mesh callbacks in the sector.
  if (sector)
    ((csSector*)sector)->FireRemoveMesh ((iMeshWrapper*)this);

  ClearFromSectorPortalLists (sector);
  const csRefArray<iSceneNode>& children = movable.GetChildren ();
  size_t i;
  for (i = 0; i < children.GetSize (); i++)
  {
    iMeshWrapper* spr = children[i]->QueryMesh ();
    // @@@ What to do in case of light!
    if (spr)
    {
      csMeshWrapper* cspr = (csMeshWrapper*)spr;
      // If we have children then we call RemoveFromSectors() on them so that
      // any potential portal_containers among them will also unregister
      // themselves from the sector.
      cspr->RemoveFromSectors (sector);
    }
  }

  if (movable.GetParent ())
    return;

  if (sector)
    sector->GetMeshes ()->Remove ((iMeshWrapper*)this);
  else
  {
    const iSectorList *sectors = movable.GetSectors ();
    if (sectors != 0)
    {
      int i;
      for (i = 0; i < sectors->GetCount (); i++)
      {
	iSector *ss = sectors->Get (i);
	if (ss)
	  ss->GetMeshes ()->Remove ((iMeshWrapper*)this);
      }
    }
  }
}

void csMeshWrapper::SetFlagsRecursive (uint32 mask, uint32 value)
{
  flags.Set (mask, value);
  const csRefArray<iSceneNode>& children = movable.GetChildren ();
  size_t i;
  for (i = 0 ; i < children.GetSize () ; i++)
  {
    iMeshWrapper* mesh = children[i]->QueryMesh ();
    if (mesh)
      mesh->SetFlagsRecursive (mask, value);
  }
}

void csMeshWrapper::SetZBufModeRecursive (csZBufMode mode)
{
  SetZBufMode (mode);
  const csRefArray<iSceneNode>& children = movable.GetChildren ();
  size_t i;
  for (i = 0 ; i < children.GetSize () ; i++)
  {
    iMeshWrapper* mesh = children[i]->QueryMesh ();
    if (mesh)
      mesh->SetZBufModeRecursive (mode);
  }
}

void csMeshWrapper::SetRenderPriorityRecursive (CS::Graphics::RenderPriority rp)
{
  SetRenderPriority (rp);
  const csRefArray<iSceneNode>& children = movable.GetChildren ();
  size_t i;
  for (i = 0 ; i < children.GetSize () ; i++)
  {
    iMeshWrapper* mesh = children[i]->QueryMesh ();
    if (mesh)
      mesh->SetRenderPriorityRecursive (rp);
  }
}

void csMeshWrapper::SetRenderPriority (CS::Graphics::RenderPriority rp)
{
  render_priority = rp;

  if (movable.GetParent ()) return ;

  int i;
  const iSectorList *sectors = movable.GetSectors ();
  for (i = 0; i < sectors->GetCount (); i++)
  {
    iSector *ss = sectors->Get (i);
    if (ss) ((csSector*)ss)->RelinkMesh ((iMeshWrapper*)this);
  }
}

csRenderMesh** csMeshWrapper::GetRenderMeshes (int& n, iRenderView* rview, 
					       uint32 frustum_mask)
{
  if (DoInstancing())
  {
    /* If instancing is enabled, but instance count is 0, simply return 0
       render meshes as well */
    csRef<csShaderVariable>& transformVars = instancing->transformVars;
    CS_ASSERT (transformVars->GetType() == csShaderVariable::ARRAY);
    size_t numInst = transformVars->GetArraySize ();
    if (numInst == 0)
    {
      n = 0;
      return nullptr;
    }
  }

  if (factory && drawing_imposter != rview->GetCamera())
  {
    csRef<iImposterFactory> factwrap = scfQueryInterface<iImposterFactory> (factory);
    if (factwrap)
    {
      if (UseImposter (rview))
      {
        if (!using_imposter)
        {
          factwrap->AddImposter (this, rview);
          using_imposter = true;
        }

        if(factwrap->RenderingImposter (this))
        {
          n = 0;
          return 0;
        }
      }
      else if (using_imposter)
      {
        factwrap->RemoveImposter (this);
        using_imposter = false;
      }
    }
  }

  // Callback are traversed in reverse order so that they can safely
  // delete themselves.
  size_t i = draw_cb_vector.GetSize ();
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
    CS::RenderManager::RenderView* csrview =
      (CS::RenderManager::RenderView*)rview;
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

  csTicks lt = engine->GetLastAnimationTime ();
  meshobj->NextFrame (lt, movable.GetPosition (), 
    rview->GetCurrentFrameNumber ());

  csMeshWrapper *meshwrap = this;
  last_anim_time = lt;
  csMeshWrapper* lastparent = meshwrap;
  csMovable* parent = movable.GetParent ();
  while (parent != 0)
  {
    iMeshWrapper* parent_mesh = parent->GetSceneNode ()->QueryMesh ();
    if (parent_mesh)
    {
      parent_mesh->GetMeshObject()->PositionChild (
      	lastparent->GetMeshObject(), lt);
      lastparent = (csMeshWrapper*)parent_mesh;
    }
    parent = parent->GetParent ();
  }

  CS::Graphics::RenderMesh** rmeshes = meshobj->GetRenderMeshes (n, rview, &movable,
  	old_ctxt != 0 ? 0 : frustum_mask);
  if (DoInstancing())
  {
    rmeshes = FixupRendermeshesForInstancing (n, rmeshes);
  }
  if (old_ctxt)
  {
    CS::RenderManager::RenderView* csrview =
      (CS::RenderManager::RenderView*)rview;
    csrview->SetCsRenderContext (old_ctxt);
  }
  return rmeshes;
}

size_t csMeshWrapper::AddExtraRenderMesh(CS::Graphics::RenderMesh* renderMesh, 
					 csZBufMode zBufMode)
{
  ExtraRenderMeshData data;
  extraRenderMeshes.Push(renderMesh);

  data.zBufMode = zBufMode;
  return extraRenderMeshData.Push(data);
}

CS::Graphics::RenderMesh** csMeshWrapper::GetExtraRenderMeshes (size_t& num, 
                    iRenderView* rview, uint32 frustum_mask)
{
  // Here we check the CS_ENTITY_NOCLIP flag. If that flag is set
  // we will only render the object once in a give frame/camera combination.
  // So if multiple portals arrive in a sector containing this object the
  // object will be rendered at the first portal and not clipped to that
  // portal (as is usually the case).
  csRenderContext* old_ctxt = 0;

  if (flags.Check (CS_ENTITY_NOCLIP))
  {
    CS::RenderManager::RenderView* csrview =
      (CS::RenderManager::RenderView*)rview;
    csRenderContext* ctxt = csrview->GetCsRenderContext ();

    if (last_frame_number == rview->GetCurrentFrameNumber () &&
    	last_camera == ctxt->icamera)
    {
      num = 0;
      return 0;
    }
    last_camera = ctxt->icamera;
    last_frame_number = rview->GetCurrentFrameNumber ();
    old_ctxt = ctxt;
    // Go back to top-level context.
    while (ctxt->previous) ctxt = ctxt->previous;
    csrview->SetCsRenderContext (ctxt);
  }

  int clip_portal, clip_plane, clip_z_plane;
  CS::RenderViewClipper::CalculateClipSettings (rview->GetRenderContext (),
      frustum_mask, clip_portal, clip_plane, clip_z_plane);

  iCamera* pCamera = rview->GetCamera();
  const csReversibleTransform& o2wt = movable.GetFullTransform();
  const csVector3& wo = o2wt.GetOrigin();
  num = extraRenderMeshes.GetSize();
  for (size_t a = 0; a < num; a++)
  {
    extraRenderMeshes[a]->clip_portal = clip_portal;
    extraRenderMeshes[a]->clip_plane = clip_plane;
    extraRenderMeshes[a]->clip_z_plane = clip_z_plane;
    extraRenderMeshes[a]->do_mirror = pCamera->IsMirrored();
    extraRenderMeshes[a]->worldspace_origin = wo;
    extraRenderMeshes[a]->object2world = o2wt;
  }

  return extraRenderMeshes.GetArray();
}

CS::Graphics::RenderMesh* csMeshWrapper::GetExtraRenderMesh (size_t idx) const
{
  return extraRenderMeshes[idx];
}

CS::Graphics::RenderPriority csMeshWrapper::GetExtraRenderMeshPriority(size_t idx) const
{
    return extraRenderMeshes[idx]->renderPrio;
}

csZBufMode csMeshWrapper::GetExtraRenderMeshZBufMode(size_t idx) const
{
    return extraRenderMeshData[idx].zBufMode;
}

void csMeshWrapper::RemoveExtraRenderMesh(csRenderMesh* renderMesh)
{
    size_t len = extraRenderMeshes.GetSize();
    for (size_t a=0; a<len; ++a)
    {
        if (extraRenderMeshes[a] != renderMesh)
            continue;

        extraRenderMeshes.DeleteIndexFast(a);
        extraRenderMeshData.DeleteIndexFast(a);

        return;
    }
}

void csMeshWrapper::RemoveExtraRenderMesh(size_t index)
{
  extraRenderMeshes.DeleteIndexFast(index);
  extraRenderMeshData.DeleteIndexFast(index);
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

void csMeshWrapper::SetParent (iSceneNode* parent)
{
  csMovable* parent_mov = movable.GetParent ();
  if (!parent_mov && !parent) return;
  if (parent_mov && parent_mov->GetSceneNode () == parent) return;

  // Incref to make sure we don't ditch our only reference here!
  this->IncRef ();

  ClearFromSectorPortalLists ();

  // If we are setting a parent then we clear the sectors of this object.
  if (parent)
    movable.ClearSectors ();

#if 0
  if (!movable.GetParent ())
  {
    // Unlink from main engine list.
    csEngine::currentEngine->GetMeshes ()->Remove ((iMeshWrapper*)this);
  }
#endif
  csSceneNode::SetParent ((iSceneNode*)this, parent, &movable);

  /* csSector->PrepareMesh tells the culler about the mesh
     (since removing the mesh above also removes it from the culler...) */
  // First we find the top-level parent.
  iSceneNode* toplevel = (iSceneNode*)this;
  while (toplevel->GetParent ())
    toplevel = toplevel->GetParent ();
  iMovable* mov = toplevel->GetMovable ();
  iSectorList* sl = mov->GetSectors ();
  for (int i = 0 ; i < sl->GetCount() ; i++)
  {
    csSector* sector = (csSector*)(sl->Get (i));
    sector->UnprepareMesh (this);
    sector->PrepareMesh (this);
  }

  if (!movable.GetParent ())
  {
    iSectorList* sl = parent_mov->GetSectors ();
    for (int i = 0 ; i < sl->GetCount() ; i++)
    {     
      csSector* sector = (csSector*)(sl->Get (i));
      sector->UnprepareMesh (this);        
    }
#if 0
    // Link to main engine list.
    csEngine::currentEngine->GetMeshes ()->Add ((iMeshWrapper*)this);
#endif
  }

  AddToSectorPortalLists ();

  this->DecRef ();
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

bool csMeshWrapper::UseImposter (iRenderView *rview)
{
  if (!factory)
    return false;

  csMeshFactoryWrapper* factwrap = static_cast<csMeshFactoryWrapper*> (factory);
  if(!factwrap->GetMinDistance())
    return false;

  float wor_sq_dist = GetSquaredDistance (rview);
  float dist = factwrap->GetMinDistance();
  return (wor_sq_dist > dist*dist);
}

void csMeshWrapper::SetLODFade (float fade)
{
  csRef<csShaderVariable> sv_lod_fade =
    GetSVContext()->GetVariableAdd (engine->id_lod_fade);
  sv_lod_fade->SetValue (fade);
}

void csMeshWrapper::UnsetLODFade ()
{
  GetSVContext()->RemoveVariable (engine->id_lod_fade);
}

void csMeshWrapper::SetDefaultEnvironmentTexture ()
{
  if (!engine->enableEnvTex) return;

  csRef<csShaderVariable> svTexEnvironment;
  svTexEnvironment.AttachNew (new csShaderVariable (engine->svTexEnvironmentName));
  svTexEnvironment->SetType (csShaderVariable::TEXTURE);

  csRef<iShaderVariableAccessor> accessor;
  accessor.AttachNew (new EnvTex::Accessor (this));
  svTexEnvironment->SetAccessor (accessor);

  GetSVContext()->AddVariable(svTexEnvironment);
}

csHitBeamResult csMeshWrapper::HitBeamOutline (
  const csVector3 &start,
  const csVector3 &end)
{
  csHitBeamResult rc;
  rc.hit = meshobj->HitBeamOutline (start, end, rc.isect, &rc.r);
  return rc;
}


csHitBeamResult csMeshWrapper::HitBeamObject (
  const csVector3 &start,
  const csVector3 &end,
  bool do_material)
{
  csHitBeamResult rc;
  rc.material = 0;
  rc.hit = meshobj->HitBeamObject (start, end, rc.isect, &rc.r,
  	&rc.polygon_idx, do_material ? &rc.material : 0);
  return rc;
}


csHitBeamResult csMeshWrapper::HitBeam (
  const csVector3 &start,
  const csVector3 &end,
  bool do_material)
{
  csHitBeamResult rc;

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

  if (HitBeamBBox (startObj, endObj).facehit > -1)
  {
    if (do_material)
    {
      rc.materials.AttachNew (new scfArray<iMaterialArray> ());
      rc.hit = meshobj->HitBeamObject (startObj, endObj, rc.isect, &rc.r, 0, &rc.material, rc.materials);
    }
    else
    {
      rc = HitBeamOutline (startObj, endObj);
    }

    if (rc.hit)
    {
      if (!movable.IsFullTransformIdentity ())
        rc.isect = trans.This2Other (rc.isect);
    }
  }

  return rc;
}

void csMeshWrapper::HardTransform (const csReversibleTransform &t)
{
  meshobj->HardTransform (t);

  const csRefArray<iSceneNode>& children = movable.GetChildren ();
  size_t i;
  for (i = 0 ; i < children.GetSize () ; i++)
  {
    iMeshWrapper* mesh = children[i]->QueryMesh ();
    if (mesh)
      mesh->HardTransform (t);
  }
}

csSphere csMeshWrapper::GetRadius () const
{
  csVector3 cent;
  float rad;

  meshobj->GetObjectModel ()->GetRadius (rad, cent);
  const csRefArray<iSceneNode>& children = movable.GetChildren ();

  csSphere sphere (cent, rad);

  if (children.GetSize () > 0)
  {    
    size_t i;
    for (i = 0; i < children.GetSize (); i++)
    {
      iMeshWrapper *spr = children[i]->QueryMesh ();
      if (spr)
      {
        csSphere childsphere = spr->GetRadius ();

        // @@@ Is this the right transform?
        childsphere *= spr->GetMovable ()->GetTransform ();
        sphere += childsphere;
      }
    }
  }

  return sphere;
}

float csMeshWrapper::GetSquaredDistance (iRenderView *rview)
{
  csVector3 cam_origin = rview->GetCamera ()->GetTransform ().GetOrigin ();
  return GetSquaredDistance (cam_origin);
}

float csMeshWrapper::GetSquaredDistance (const csVector3& pos)
{
  // calculate distance from pos to mesh
  const csBox3& obox = GetObjectModel ()->GetObjectBoundingBox ();
  csVector3 obj_center = (obox.Min () + obox.Max ()) / 2;
  csVector3 wor_center;
  if (movable.IsFullTransformIdentity ())
    wor_center = obj_center;
  else
    wor_center = movable.GetFullTransform ().This2Other (obj_center);
  float wor_sq_dist = csSquaredDist::PointPoint (pos, wor_center);
  return wor_sq_dist;
}

void csMeshWrapper::GetFullBBox (csBox3& box)
{
  box = GetObjectModel ()->GetObjectBoundingBox ();
  csMovable* mov = &movable;
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
    mov = ((csMovable*)mov)->GetParent ();
  }
}


void csMeshWrapper::PlaceMesh ()
{
  iSectorList *movable_sectors = movable.GetSectors ();
  if (movable_sectors->GetCount () == 0) return ; // Do nothing
  csSphere sphere;
  float radius;
  GetObjectModel ()->GetRadius (radius, sphere.GetCenter ());

  iSector *sector = movable_sectors->Get (0);
  movable.SetSector (sector);       // Make sure all other sectors are removed

  // Transform the sphere from object to world space.
  sphere.SetRadius (radius);
  if (!movable.IsFullTransformIdentity ())
    sphere = movable.GetFullTransform ().This2Other (sphere);
  radius = sphere.GetRadius ();
  //float max_sq_radius = radius * radius;

  csRef<iMeshWrapperIterator> it = 
    engine->GetNearbyMeshes (sector, sphere.GetCenter (), radius, true);

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
	const csSphere& portal_sphere = portal->GetWorldSphere ();
	if (portal_sphere.TestIntersect (sphere))
          movable_sectors->Add (dest_sector);
      }
    }
  }
}

csHitBeamResult csMeshWrapper::HitBeamBBox (
  const csVector3 &start,
  const csVector3 &end)
{
  csHitBeamResult rc;
  const csBox3& b = GetObjectModel ()->GetObjectBoundingBox ();

  csSegment3 seg (start, end);
  rc.facehit = csIntersect3::BoxSegment (b, seg, rc.isect, &rc.r);
  return rc;
}


const csBox3& csMeshWrapper::GetWorldBoundingBox ()
{
  if (wor_bbox_movablenr != movable.GetUpdateNumber ())
  {
    wor_bbox_movablenr = movable.GetUpdateNumber ();

    if (movable.IsFullTransformIdentity ())
      wor_bbox = GetObjectModel ()->GetObjectBoundingBox ();
    else
    {
      const csBox3& obj_bbox = GetObjectModel ()->GetObjectBoundingBox ();

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

csBox3 csMeshWrapper::GetTransformedBoundingBox (
  const csReversibleTransform &trans)
{
  csBox3 cbox;
  const csBox3& box = GetObjectModel ()->GetObjectBoundingBox ();
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


csScreenBoxResult csMeshWrapper::GetScreenBoundingBox (iCamera *camera)
{
  csScreenBoxResult rc;

  // Calculate camera space bbox.
  csReversibleTransform tr_o2c = camera->GetTransform ();
  if (!movable.IsFullTransformIdentity ())
    tr_o2c /= movable.GetFullTransform ();
  
  rc.cbox = GetTransformedBoundingBox (tr_o2c);

  // Calculate screen space bbox.
  float minz, maxz;
  const csBox3& wbox = GetWorldBoundingBox();
  if(!wbox.ProjectBox(camera->GetTransform(), camera->GetProjectionMatrix(),
      rc.sbox, minz, maxz, engine->G3D->GetWidth(),
      engine->G3D->GetHeight()))
  {
      rc.distance = -1;
  }
  else
  {
      rc.distance = rc.cbox.MaxZ ();
  }

  return rc;
}

iMeshWrapper* csMeshWrapper::FindChildByName (const char* name)
{
  const csRefArray<iSceneNode>& children = movable.GetChildren ();
  size_t i;

  char const* p = strchr (name, ':');
  if (!p)
  {
    for (i = 0 ; i < children.GetSize () ; i++)
    {
      iMeshWrapper* m = children[i]->QueryMesh ();
      if (m && !strcmp (name, m->QueryObject ()->GetName ()))
        return m;
    }
    return 0;
  }
 
  int firstsize = p-name;
  csString firstName;
  firstName.Append (name, firstsize);

  for (i = 0 ; i < children.GetSize () ; i++)
  {
    iMeshWrapper* m = children[i]->QueryMesh ();
    if (m && !strcmp (firstName, m->QueryObject ()->GetName ()))
    {
      return m->FindChildByName (p+1);
    }
  }
  return 0;
}

//--------------------------------------------------------------------------
// csMeshList
//--------------------------------------------------------------------------

csMeshList::csMeshList (int cap, int thresshold)
  : scfImplementationType (this), list (cap, thresshold)
{
  listener.AttachNew (new NameChangeListener (this));
}

csMeshList::~csMeshList ()
{
  RemoveAll ();
}

void csMeshList::NameChanged (iObject* object, const char* oldname,
  	const char* newname)
{
  csRef<iMeshWrapper> mesh = scfQueryInterface<iMeshWrapper> (object);
  CS_ASSERT (mesh != 0);
  CS::Threading::ScopedWriteLock lock(meshLock);
  if (oldname) meshes_hash.Delete (oldname, mesh);
  if (newname) meshes_hash.Put (newname, mesh);
}

int csMeshList::Add (iMeshWrapper *obj)
{
  PrepareMesh (obj);
  const char* name = obj->QueryObject ()->GetName ();
  CS::Threading::ScopedWriteLock lock(meshLock);
  if (name)
    meshes_hash.Put (name, obj);
  obj->QueryObject ()->AddNameChangeListener (listener);
  return (int)list.Push (obj);
}

void csMeshList::AddBatch (csRef<iMeshLoaderIterator> itr)
{
  CS::Threading::ScopedWriteLock lock(meshLock);
  while(itr->HasNext())
  {
    iMeshWrapper* obj = itr->Next();
    PrepareMesh (obj);
    const char* name = obj->QueryObject ()->GetName ();
    if (name)
      meshes_hash.Put (name, obj);
    obj->QueryObject ()->AddNameChangeListener (listener);
    list.Push (obj);
  }
}

bool csMeshList::Remove (iMeshWrapper *obj)
{
  FreeMesh (obj);
  const char* name = obj->QueryObject ()->GetName ();
  CS::Threading::ScopedWriteLock lock(meshLock);
  if (name)
    meshes_hash.Delete (name, obj);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  list.Delete (obj);
  return true;
}

bool csMeshList::Remove (int n)
{
  CS::Threading::ScopedWriteLock lock(meshLock);
  FreeMesh (list[n]);
  iMeshWrapper* obj = list[n];
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    meshes_hash.Delete (name, obj);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  list.DeleteIndex (n);
  return true;
}

void csMeshList::RemoveAll ()
{
  CS::Threading::ScopedWriteLock lock(meshLock);
  size_t i;
  for (i = 0 ; i < list.GetSize () ; i++)
  {
    list[i]->QueryObject ()->RemoveNameChangeListener (listener);
    FreeMesh (list[i]);
  }
  meshes_hash.DeleteAll ();
  list.DeleteAll ();
}

int csMeshList::GetCount () const
{
  CS::Threading::ScopedReadLock lock(meshLock);
  return (int)list.GetSize ();
}
iMeshWrapper* csMeshList::Get (int n) const
{
  CS::Threading::ScopedReadLock lock(meshLock);
  return list.Get (n);
}

int csMeshList::Find (iMeshWrapper *obj) const
{
  CS::Threading::ScopedReadLock lock(meshLock);
  return (int)list.Find (obj);
}

iMeshWrapper *csMeshList::FindByName (const char *Name) const
{
  CS::Threading::ScopedReadLock lock(meshLock);
  return meshes_hash.Get (Name, 0);
}

#if 0
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
    iSector* isector = (mesh->GetCsMovable ()).csMovable::GetSectors ()
    	->Get (i);
    csSector* sector = (csSector*)isector;
    sector->UnprepareMesh (item);
  }

  item->SetParentContainer (0);
  ((csMovable*)(item->GetMovable ()))->SetParent (0);

  mesh->RemoveMeshFromStaticLOD (item);

  csMeshList::FreeMesh (item);
}
#endif

