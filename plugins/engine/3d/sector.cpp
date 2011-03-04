/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
              (C) 2004-2008 by Marten Svanfeldt

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
#include "csqint.h"
#include "csqsqrt.h"
#include "csutil/csppulse.h"
#include "csutil/csstring.h"
#include "cstool/csview.h"
#include "iengine/portal.h"
#include "iengine/rview.h"
#include "igeom/clip2d.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iutil/virtclk.h"
#include "ivaria/bugplug.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"

#include "plugins/engine/3d/camera.h"
#include "plugins/engine/3d/engine.h"
#include "plugins/engine/3d/light.h"
#include "plugins/engine/3d/material.h"
#include "plugins/engine/3d/sector.h"
#include "meshgen/meshgen.h"
#include "plugins/engine/3d/meshobj.h"

//--------------------------------------------------------------------------

csSectorMeshList::csSectorMeshList () : csMeshList (64, 128)
{
  sector = 0;
}

void csSectorMeshList::PrepareMesh (iMeshWrapper* item)
{
  CS_ASSERT (sector != 0);
  csMeshList::PrepareMesh (item);
  sector->PrepareMesh (item);
}

void csSectorMeshList::FreeMesh (iMeshWrapper* item)
{
  CS_ASSERT (sector != 0);
  sector->UnprepareMesh (item);
  csMeshList::FreeMesh (item);
}

//--------------------------------------------------------------------------


csSectorLightList::csSectorLightList (csSector* isect)
  : sector (isect)
{
}

csSectorLightList::~csSectorLightList ()
{
  RemoveAll ();
}

void csSectorLightList::PrepareLight (iLight* item)
{
  csLight* clight = static_cast<csLight*> (item);
  csLightList::PrepareLight (item);

  lightTree.AddObject (clight);

  clight->SetSector (sector);
}

void csSectorLightList::FreeLight (iLight* item)
{
  csLight* clight = static_cast<csLight*> (item);
  lightTree.RemoveObject (clight);
  clight->SetSector (0); 
}

void csSectorLightList::UpdateLightBounds (csLight* light, const csBox3& oldBox)
{
  lightTree.MoveObject (light, oldBox);
}

//---------------------------------------------------------------------------

csSector::csSector (csEngine *engine) :
  scfImplementationType (this), lights (this), engine (engine)
{
  drawBusy = 0;
  dynamicAmbientLightColor.Set (0,0,0);
  dynamicAmbientLightVersion = (uint)~0;
  meshes.SetSector (this);
  //portal_containers.SetSector (this);
  currentVisibilityNumber = 0;
  renderloop = 0;

  single_mesh = 0;

  SetupSVNames();
  svDynamicAmbient.AttachNew (new csShaderVariable (SVNames().dynamicAmbient));
  svDynamicAmbient->SetValue (dynamicAmbientLightColor);
  AddVariable (svDynamicAmbient);
  svLightAmbient.AttachNew (new csShaderVariable (SVNames().lightAmbient));
  svLightAmbient->SetType (csShaderVariable::VECTOR3);
  {
    csRef<iShaderVariableAccessor> sva;
    sva.AttachNew (new LightAmbientAccessor (this));
    svLightAmbient->SetAccessor (sva);
  }
  AddVariable (svLightAmbient);
  svFogColor.AttachNew (new csShaderVariable (SVNames().fogColor));
  AddVariable (svFogColor);
  svFogMode.AttachNew (new csShaderVariable (SVNames().fogMode));
  AddVariable (svFogMode);
  svFogFadeStart.AttachNew (new csShaderVariable (SVNames().fogFadeStart));
  AddVariable (svFogFadeStart);
  svFogFadeEnd.AttachNew (new csShaderVariable (SVNames().fogFadeEnd));
  AddVariable (svFogFadeEnd);
  svFogLimit.AttachNew (new csShaderVariable (SVNames().fogLimit));
  AddVariable (svFogLimit);
  svFogDensity.AttachNew (new csShaderVariable (SVNames().fogDensity));
  AddVariable (svFogDensity);
  UpdateFogSVs();
}

csSector::~csSector ()
{
  lights.RemoveAll ();
}

void csSector::SelfDestruct ()
{
  engine->GetSectors ()->Remove ((iSector*)this);
}

void csSector::UnlinkObjects ()
{
  int i;
  for (i = meshes.GetCount()-1; i >= 0; i--)
  {
    csRef<iMeshWrapper> m = meshes.Get (i);
    iSectorList* sl = m->GetMovable ()->GetSectors ();
    sl->Remove ((iSector*)this);
    m->GetMovable ()->UpdateMove ();
  }
  for (i = lights.GetCount()-1; i >= 0; i--)
  {
    iLight* l = lights.Get (i);
    iSectorList* sl = l->GetMovable ()->GetSectors ();
    sl->Remove ((iSector*)this);
    l->GetMovable ()->UpdateMove ();
  }
}

//----------------------------------------------------------------------

void csSector::RegisterEntireMeshToCuller (iMeshWrapper* mesh)
{
  csMeshWrapper* cmesh = (csMeshWrapper*)mesh;
  if (cmesh->SomeParentHasStaticLOD ()) return;

  csRef<iVisibilityObject> vo = 
        scfQueryInterface<iVisibilityObject> (mesh);
  culler->RegisterVisObject (vo);

  if (cmesh->GetStaticLODMesh ()) return;
  size_t i;
  const csRefArray<iSceneNode>& ml = cmesh->GetChildren ();
  for (i = 0 ; i < ml.GetSize () ; i++)
  {
    iMeshWrapper* child = ml[i]->QueryMesh ();
    if (child)
      RegisterEntireMeshToCuller (child);
  }
}

void csSector::RegisterMeshToCuller (iMeshWrapper* mesh)
{
  csMeshWrapper* cmesh = (csMeshWrapper*)mesh;
  if (cmesh->SomeParentHasStaticLOD ()) return;

  csRef<iVisibilityObject> vo = 
        scfQueryInterface<iVisibilityObject> (mesh);
  culler->RegisterVisObject (vo);
}

void csSector::UnregisterMeshToCuller (iMeshWrapper* mesh)
{
  csRef<iVisibilityObject> vo = 
        scfQueryInterface<iVisibilityObject> (mesh);
  culler->UnregisterVisObject (vo);
}

void csSector::PrepareMesh (iMeshWrapper *mesh)
{
  bool do_camera = mesh->GetFlags ().Check (CS_ENTITY_CAMERA);
  if (do_camera) cameraMeshes.Push (mesh);

  if (culler) RegisterMeshToCuller (mesh);
  size_t i;
  const csRefArray<iSceneNode>& ml = ((csMeshWrapper*)mesh)->GetChildren ();
  for (i = 0 ; i < ml.GetSize () ; i++)
  {
    iMeshWrapper* child = ml[i]->QueryMesh ();
    if (child)
      PrepareMesh (child);
  }
}

void csSector::UnprepareMesh (iMeshWrapper *mesh)
{
  cameraMeshes.Delete (mesh);

  if (culler) UnregisterMeshToCuller (mesh);
  size_t i;
  const csRefArray<iSceneNode>& ml = ((csMeshWrapper*)mesh)->GetChildren ();
  for (i = 0 ; i < ml.GetSize () ; i++)
  {
    iMeshWrapper* child = ml[i]->QueryMesh ();
    if (child)
      UnprepareMesh (child);
  }
}

void csSector::RelinkMesh (iMeshWrapper *mesh)
{
  cameraMeshes.Delete (mesh);
  bool do_camera = mesh->GetFlags ().Check (CS_ENTITY_CAMERA);
  if (do_camera) cameraMeshes.Push (mesh);

  size_t i;
  const csRefArray<iSceneNode>& ml = ((csMeshWrapper*)mesh)->GetChildren ();
  for (i = 0 ; i < ml.GetSize () ; i++)
  {
    iMeshWrapper* child = ml[i]->QueryMesh ();
    if (child)
      RelinkMesh (child);
  }
}

void csSector::PrecacheDraw ()
{
  GetVisibilityCuller ()->PrecacheCulling ();

  // First calculate the box of all objects in the level.
  csBox3 box;
  box.StartBoundingBox ();
  int i;
  for (i = 0; i < meshes.GetCount (); i++)
  {
    iMeshWrapper* m = meshes.Get (i);
    const csBox3& mesh_box = m->GetWorldBoundingBox ();
    box += mesh_box;
  }

  // Try to position our camera somewhere above the bounding
  // box of the sector so we see as much as possible.
  csVector3 pos = box.GetCenter ();
  pos.y = box.MaxY () + (box.MaxY () - box.MinY ());
  csVector3 lookat = pos;
  lookat.y = box.MinY ();

  csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (
      engine->objectRegistry);
  csRef<csView> view;
  view.AttachNew (new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  iCamera* camera = view->GetCamera ();
  camera->SetSector (this);
  camera->GetTransform ().SetOrigin (pos);
  camera->GetTransform ().LookAt (lookat-pos, csVector3 (0, 0, 1));

  // @@@ Ideally we would want to disable visibility culling
  // here so that all objects are visible.
  /*g3d->BeginDraw (CSDRAW_3DGRAPHICS);
  view->Draw ();
  g3d->FinishDraw ();*/
  engine->renderManager->PrecacheView (view);
}

//----------------------------------------------------------------------

bool csSector::SetVisibilityCullerPlugin (const char *plugname,
	iDocumentNode* culler_params)
{
  // Loop through all meshes and unregister them from the old visibility culler.
  if (culler.IsValid())
  {
    for (int i = 0; i < meshes.GetCount (); i++)
    {
      UnregisterMeshToCuller(meshes.Get(i));
    }
  }
  culler = 0;

  // If the plugname is NULL it means we are requesting the default culler
  // (as of writing occluvis). So we are done here.
  if (!plugname)
  {
    return true;
  }

  // Load the culler plugin.
  csRef<iPluginManager> plugmgr = 
  	csQueryRegistry<iPluginManager> (engine->objectRegistry);
  culler = csLoadPlugin<iVisibilityCuller> (plugmgr, plugname);

  // The plugin couldn't be loaded.
  if (!culler.IsValid())
  {
    return false;
  }

  const char* err = culler->ParseCullerParameters (culler_params);
  if (err)
  {
    engine->Error ("Error loading visibility culler: %s!",
    	err);
    return false;
  }

  // load cache data
  csString cachename;
  cachename.Format ("%s_%s", GetName (), plugname);
  culler->Setup (cachename);

  // Loop through all meshes and register them to the visibility culler.
  for (int i = 0; i < meshes.GetCount (); i++)
  {
    iMeshWrapper* m = meshes.Get (i);
    m->GetMovable ()->UpdateMove ();
    RegisterEntireMeshToCuller (m);
  }

  return true;
}

iVisibilityCuller* csSector::GetVisibilityCuller ()
{
  if (!culler)
  {
    // Check for a culler in the RM.
    csRef<iRenderManagerVisCull> rmVC = scfQueryInterfaceSafe<iRenderManagerVisCull> (engine->renderManager);

    if (rmVC)
    {
      culler = rmVC->GetVisCuller ();
    }

    if (culler)
    {
      // Loop through all meshes and register them to the visibility culler.
      for (int i = 0; i < meshes.GetCount (); i++)
      {
        iMeshWrapper* m = meshes.Get (i);
        m->GetMovable ()->UpdateMove ();
        RegisterEntireMeshToCuller (m);
      }
    }
    else // Else try to load frustvis.
    {
      SetVisibilityCullerPlugin ("crystalspace.culling.frustvis");
    }
  }

  CS_ASSERT (culler != 0);
  return culler;
}

class csSectorVisibleMeshCallback : public scfImplementation1<
	csSectorVisibleMeshCallback,
	iVisibilityCullerListener>
{
public:

  csSectorVisibleMeshCallback ()
    : scfImplementationType (this), privMeshlist (0)
  {
  }

  virtual ~csSectorVisibleMeshCallback()
  {
  }

  // NR version
  void Setup (csRenderMeshList *meshlist, iRenderView *rview, iSector* sector)
  {
    privMeshlist = meshlist;
    csSectorVisibleMeshCallback::rview = rview;
    csSectorVisibleMeshCallback::sector = sector;
  }

  void MarkMeshAndChildrenVisible (iMeshWrapper* mesh, uint32 frustum_mask,
                                   bool doFade = false, float fade = 1.0f)
  {
    csMeshWrapper* cmesh = (csMeshWrapper*)mesh;
    ObjectVisible (cmesh, frustum_mask, doFade, fade);
    size_t i;
    const csRefArray<iSceneNode>& children = cmesh->GetChildren ();
    for (i = 0 ; i < children.GetSize () ; i++)
    {
      iMeshWrapper* child = children[i]->QueryMesh ();
      // @@@ Traverse too in case there are lights/cameras?
      if (child)
        MarkMeshAndChildrenVisible (child, frustum_mask, doFade, fade);
    }
  }

  void ObjectVisible (csMeshWrapper* cmesh, uint32 frustum_mask,
                      bool doFade = false, float fade = 1.0f)
  {
    csStaticLODMesh* static_lod = cmesh->GetStaticLODMesh ();
    bool mm = cmesh->DoMinMaxRange ();
    float distance = 0;
    if (static_lod || mm)
      distance = csQsqrt (cmesh->GetSquaredDistance (rview));

    if (mm)
    {
      if (distance < cmesh->csMeshWrapper::GetMinimumRenderDistance ())
        return;
      if (distance > cmesh->csMeshWrapper::GetMaximumRenderDistance ())
        return;
    }

    if (doFade)
      cmesh->SetLODFade (fade);
    else
      cmesh->UnsetLODFade ();

    if (static_lod)
    {
      float lod = static_lod->GetLODValue (distance);
      csArray<iMeshWrapper*>* meshes1;
      csArray<iMeshWrapper*>* meshes2;
      float lodFade;
      bool hasFade = static_lod->GetMeshesForLODFaded (lod,
	meshes1, meshes2, lodFade);
      size_t i;
      if (meshes1 != 0)
      {
	for (i = 0 ; i < meshes1->GetSize () ; i++)
	  MarkMeshAndChildrenVisible ((*meshes1)[i], frustum_mask,
	    hasFade, fade*lodFade);
      }
      if (meshes2 != 0)
      {
	for (i = 0 ; i < meshes2->GetSize () ; i++)
	  MarkMeshAndChildrenVisible ((*meshes2)[i], frustum_mask,
	    hasFade, fade*(1.0f-lodFade));
      }
    }

    int num;
    csRenderMesh** meshes = cmesh->GetRenderMeshes (num, rview, frustum_mask);
    CS_ASSERT(!((num != 0) && (meshes == 0)));
#ifdef CS_DEBUG
    for (int i = 0 ; i < num ; i++)
      meshes[i]->db_mesh_name = cmesh->GetName ();
#endif
    if (num > 0)
    {
      privMeshlist->AddRenderMeshes (meshes, num,
      	cmesh->csMeshWrapper::GetRenderPriority (),
      	cmesh->csMeshWrapper::GetZBufMode (), (iMeshWrapper*)cmesh);

      // get extra render meshes
      size_t numExtra = 0;
      csRenderMesh** extraMeshes = cmesh->GetExtraRenderMeshes (numExtra, rview,
                                            frustum_mask);
      CS_ASSERT(!((numExtra != 0) && (extraMeshes == 0)));
      for (size_t i = 0; i < numExtra; ++i)
      {
          privMeshlist->AddRenderMeshes (&extraMeshes[i], 1,
                  cmesh->csMeshWrapper::GetExtraRenderMesh (i)->renderPrio,
                  cmesh->csMeshWrapper::GetExtraRenderMesh (i)->z_buf_mode,
                  (iMeshWrapper*)cmesh);
      }
    }
  }

  virtual void ObjectVisible (iVisibilityObject* visobj, iMeshWrapper *mesh,
  	uint32 frustum_mask)
  {
    if (mesh)
    {
      csMeshWrapper* cmesh = (csMeshWrapper*)mesh;
      ObjectVisible (cmesh, frustum_mask);
    }
    else
    {
      csRef<iLight> light = scfQueryInterface<iLight> (visobj);
      if (light)
      {
        csSector* csector = (csSector*)sector;
        csector->FireLightVisibleCallbacks (light);
      }
    }
  }

  virtual int GetVisibleMeshes(iMeshWrapper *,uint32,csSectorVisibleRenderMeshes *&)
  {
    return 0;
  }

  virtual void MarkVisible(iMeshWrapper *,int,csSectorVisibleRenderMeshes *&)
  {
  }

private:
  csRenderMeshList *privMeshlist;
  iRenderView *rview;
  iSector* sector;
};

CS_IMPLEMENT_STATIC_VAR (GetVisMeshCb, csSectorVisibleMeshCallback, ())

csRenderMeshList *csSector::GetVisibleMeshes (iRenderView *rview)
{
  if (rview == 0) return 0;

  // This is used for csMeshObject::IsChildVisible to determine
  // when to update its cache. Should be changed to something more
  // sensible. @@@ What about engine frame number?
  currentVisibilityNumber++;

/*  if (engine->GetCurrentFrameNumber () != cachedFrameNumber ||
      rview->GetCamera () != cachedCamera )
  {
    visibleMeshCache->Empty ();
    cb.Setup (visibleMeshCache, rview);
    GetVisibilityCuller()->VisTest (rview, GetVisMeshCb ());

    cachedFrameNumber = engine->GetCurrentFrameNumber ();
    cachedCamera = rview->GetCamera();
  }

  return visibleMeshCache;*/

  size_t i;
  uint32 cur_framenr = engine->GetCurrentFrameNumber ();
  uint32 cur_context_id = rview->GetRenderContext ()->context_id;
  for (i = 0 ; i < visibleMeshCache.GetSize () ; i++)
  {
    visibleMeshCacheHolder& entry = visibleMeshCache[i];
    if (entry.cachedFrameNumber == cur_framenr &&
        entry.cached_context_id == cur_context_id)
    {
      return entry.meshList;
    }
  }

  //try to find a spot to do a new cache
  for (i = 0 ; i < visibleMeshCache.GetSize () ; i++)
  {
    visibleMeshCacheHolder& entry = visibleMeshCache[i];
    if (entry.cachedFrameNumber != cur_framenr)
    {
      //use this slot
      entry.meshList->Empty ();
      GetVisMeshCb ()->Setup (entry.meshList, rview, (iSector*)this);

      if (single_mesh)
	GetVisMeshCb ()->ObjectVisible ((csMeshWrapper*)single_mesh, ~0);
      else
        GetVisibilityCuller()->VisTest (rview, GetVisMeshCb ());

      entry.cachedFrameNumber = cur_framenr;
      entry.cached_context_id = cur_context_id;
      return entry.meshList;
    }
  }

  //create a new cache entry
  visibleMeshCacheHolder holder;
  holder.cachedFrameNumber = cur_framenr;
  holder.cached_context_id = cur_context_id;
  holder.meshList = new csRenderMeshList (engine);
  usedMeshLists.Push (holder.meshList);
  GetVisMeshCb ()->Setup (holder.meshList, rview, (iSector*)this);
  if (single_mesh)
    GetVisMeshCb ()->ObjectVisible ((csMeshWrapper*)single_mesh, ~0);
  else
    GetVisibilityCuller()->VisTest (rview, GetVisMeshCb ());
  visibleMeshCache.Push (holder);
  return holder.meshList;
}

void csSector::MarkMeshAndChildrenVisible (iMeshWrapper* mesh,
					   iRenderView* rview,
					   uint32 frustum_mask,
					   bool doFade, float fade)
{
  csMeshWrapper* cmesh = (csMeshWrapper*)mesh;
  ObjectVisible (cmesh, rview, frustum_mask, doFade, fade);
  size_t i;
  const csRefArray<iSceneNode>& children = cmesh->GetChildren ();
  for (i = 0 ; i < children.GetSize () ; i++)
  {
    iMeshWrapper* child = children[i]->QueryMesh ();
    // @@@ Traverse too in case there are lights/cameras?
    if (child)
      MarkMeshAndChildrenVisible (child, rview, frustum_mask, doFade, fade);
  }
}

void csSector::ObjectVisible (csMeshWrapper* cmesh, iRenderView* rview,
			      uint32 frustum_mask,
			      bool doFade = false, float fade = 1.0f)
{
  csStaticLODMesh* static_lod = cmesh->GetStaticLODMesh ();
  bool mm = cmesh->DoMinMaxRange ();
  float distance = 0;
  if (static_lod || mm)
    distance = csQsqrt (cmesh->GetSquaredDistance (rview));

  if (mm)
  {
    if (distance < cmesh->csMeshWrapper::GetMinimumRenderDistance ())
      return;
    if (distance > cmesh->csMeshWrapper::GetMaximumRenderDistance ())
      return;
  }

  if (doFade)
    cmesh->SetLODFade (fade);
  else
    cmesh->UnsetLODFade ();

  if (static_lod)
  {
    float lod = static_lod->GetLODValue (distance);
    csArray<iMeshWrapper*>* meshes1;
    csArray<iMeshWrapper*>* meshes2;
    float lodFade;
    bool hasFade = static_lod->GetMeshesForLODFaded (lod,
      meshes1, meshes2, lodFade);
    size_t i;
    if (meshes1 != 0)
    {
      for (i = 0 ; i < meshes1->GetSize () ; i++)
	MarkMeshAndChildrenVisible ((*meshes1)[i], rview, frustum_mask,
	  hasFade, fade*lodFade);
    }
    if (meshes2 != 0)
    {
      for (i = 0 ; i < meshes2->GetSize () ; i++)
	MarkMeshAndChildrenVisible ((*meshes2)[i], rview, frustum_mask,
	  hasFade, fade*(1.0f-lodFade));
    }
  }

  csSectorVisibleRenderMeshes visMesh;
  visMesh.imesh = cmesh;
  int num;
  csRenderMesh** meshes = cmesh->GetRenderMeshes (num, rview, frustum_mask);
  CS_ASSERT(!((num != 0) && (meshes == 0)));
#ifdef CS_DEBUG
  for (int i = 0 ; i < num ; i++)
    meshes[i]->db_mesh_name = cmesh->GetName ();
#endif
  visMesh.num = num;
  visMesh.rmeshes = meshes;
  renderMeshesScratch.Push (visMesh);

  if (num > 0)
  {
    // get extra render meshes
    size_t numExtra = 0;
    csRenderMesh** extraMeshes = cmesh->GetExtraRenderMeshes (numExtra, rview,
					  frustum_mask);
    CS_ASSERT(!((numExtra != 0) && (extraMeshes == 0)));
    visMesh.num = (int)numExtra;
    visMesh.rmeshes = extraMeshes;
  }
}

csSectorVisibleRenderMeshes* csSector::GetVisibleRenderMeshes (int& num,
						 iMeshWrapper* mesh,
						 iRenderView *rview,
						 uint32 frustum_mask)
{
  csMeshWrapper* cmesh = (csMeshWrapper*)mesh;
  csStaticLODMesh* static_lod = cmesh->GetStaticLODMesh ();
  bool mm = cmesh->DoMinMaxRange ();
  if (!static_lod && !mm)
  {
    csRenderMesh** meshes = cmesh->GetRenderMeshes (num, rview, frustum_mask);
    CS_ASSERT(!((num != 0) && (meshes == 0)));
  #ifdef CS_DEBUG
    for (int i = 0 ; i < num ; i++)
      meshes[i]->db_mesh_name = cmesh->GetName ();
  #endif

    if (meshes == 0 && num == 0)
      return 0;

    oneVisibleMesh[0].imesh = mesh;
    oneVisibleMesh[0].num = num;
    oneVisibleMesh[0].rmeshes = meshes;

    size_t numExtra = 0;
    csRenderMesh** extraMeshes = 0;
    if (num > 0)
    {
      extraMeshes = cmesh->GetExtraRenderMeshes (numExtra, rview,
				    frustum_mask);
      CS_ASSERT(!((numExtra != 0) && (extraMeshes == 0)));
    }
  
    if (numExtra == 0)
    {
      num = 1;
      return oneVisibleMesh;
    }
    
    oneVisibleMesh[1].imesh = mesh;
    oneVisibleMesh[1].num = (int)numExtra;
    oneVisibleMesh[1].rmeshes = extraMeshes;

    num = 2;
    return oneVisibleMesh;
  }

  renderMeshesScratch.Empty();

  ObjectVisible (cmesh, rview, frustum_mask);

  num = (int)renderMeshesScratch.GetSize();
  return renderMeshesScratch.GetArray();
}

csSectorHitBeamResult csSector::HitBeamPortals (
  const csVector3 &start,
  const csVector3 &end)
{
  csSectorHitBeamResult rc;
  rc.mesh = 0;
  rc.final_sector = static_cast<iSector*> (this);
  int p = IntersectSegment (start, end, rc.isect, 0, false,
		  &rc.mesh);
  if (p != -1)
  {
    iPortalContainer* portals = rc.mesh->GetPortalContainer ();
    if (portals)
    {
      // There are portals.
      iPortal* po = portals->GetPortal (p);
      if (po)
      {
	drawBusy++;
	csVector3 new_start = rc.isect;
	rc.mesh = po->HitBeamPortals (rc.mesh->GetMovable ()
		->GetFullTransform (),
		new_start, end, rc.isect, &p, &rc.final_sector);
	drawBusy--;
      }
    }
  }
  rc.polygon_idx = p;
  return rc;
}

csSectorHitBeamResult csSector::HitBeam (
  const csVector3 &start,
  const csVector3 &end,
  bool accurate)
{
  GetVisibilityCuller ();
  float r;
  csSectorHitBeamResult rc;
  rc.mesh = 0;
  rc.polygon_idx = -1;
  rc.final_sector = 0;
  bool result = culler->IntersectSegment (start, end, rc.isect, &r, &rc.mesh,
  	&rc.polygon_idx, accurate);
  if (!result) rc.mesh = 0;
  return rc;
}

THREADED_CALLABLE_IMPL1(csSector, SetSectorCallback, csRef<iSectorCallback> cb)
{
  sectorCallbackList.Push(cb);
  return true;
}

THREADED_CALLABLE_IMPL1(csSector, RemoveSectorCallback, csRef<iSectorCallback> cb)
{
  sectorCallbackList.Delete(cb);
  return true;
}

int csSector::IntersectSegment (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr,
  bool only_portals,
  iMeshWrapper **p_mesh)
{
  GetVisibilityCuller ();
  float r, best_r = 10000000000.;
  csVector3 cur_isect;
  int best_p = -1;
  csVector3 obj_start, obj_end, obj_isect;

  if (!only_portals)
  {
    iMeshWrapper *mesh;
    int poly;
    bool rc = culler->IntersectSegment (start, end, isect, &r, &mesh, &poly);
    if (rc)
    {
      if (poly != -1) best_p = poly;
      best_r = r;
      if (p_mesh) *p_mesh = mesh;
    }
    if (pr) *pr = best_r;
    return best_p;
  }

  csReversibleTransform movtrans;

  GetVisibilityCuller ();
  csBox3 b (start);
  b.AddBoundingVertexSmart (end);
  csRef<iVisibilityObjectIterator> visit = culler->VisTest (b);

  while (visit->HasNext ())
  {
    iVisibilityObject* vo = visit->Next ();
    iMeshWrapper* mesh = vo->GetMeshWrapper ();
    if (!mesh || mesh->GetFlags ().Check (CS_ENTITY_NOHITBEAM)) continue;

    // Only operate on portal objects.
    if (!mesh->GetPortalContainer ()) continue;

    bool has_not_moved = mesh->GetMovable ()->IsFullTransformIdentity ();
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
    r = best_r;

    // We know it is a portal container. No other object can have a
    // portal.
    int p;
    bool rc = mesh->GetMeshObject ()->HitBeamObject (
      	obj_start, obj_end, obj_isect, &r, &p);
    if (!rc) p = -1;
    if (p != -1 && r < best_r)
    {
      if (has_not_moved)
        cur_isect = obj_isect;
      else
        cur_isect = movtrans.This2Other (obj_isect);
      best_r = r;
      best_p = p;
      isect = cur_isect;
      if (p_mesh) *p_mesh = mesh;
    }
  }

  if (pr) *pr = best_r;
  return best_p;
}

THREADED_CALLABLE_IMPL1(csSector, SetRenderLoop, iRenderLoop* rl)
{
  renderloop = rl;
  return true;
}

iSector *csSector::FollowSegment (
  csReversibleTransform &t,
  csVector3 &new_position,
  bool &mirror,
  bool only_portals,
  iPortal** transversed_portals,
  iMeshWrapper** portal_meshes,
  int firstIndex, int* lastIndex)
{
  csVector3 isect;
  iMeshWrapper* mesh;
  int p = IntersectSegment (
    t.GetOrigin (),
    new_position,
    isect,
    0,
    only_portals,
    &mesh);

  if (p != -1)
  {
    iPortalContainer* portals = mesh->GetPortalContainer ();
    if (portals)
    {
      iPortal *po = portals->GetPortal (p);
      if (po)
      {
        po->CompleteSector (0);
        if (!po->GetSector ())
        {
          new_position = isect;
          return (iSector*)this;
        }

        if (po->GetFlags ().Check (CS_PORTAL_WARP))
        {
	  csReversibleTransform warp_wor;
	  po->ObjectToWorld (mesh->GetMovable ()->GetTransform (), warp_wor);
          po->WarpSpace (warp_wor, t, mirror);
          new_position = po->Warp (warp_wor, new_position);
        }

        if(transversed_portals && portal_meshes && lastIndex && firstIndex <= *lastIndex)
        {
            transversed_portals[firstIndex] = po;
            portal_meshes[firstIndex] = mesh;
            ++firstIndex;
        }

        iSector *dest_sect = po->GetSector ();
        return dest_sect->FollowSegment (t, new_position, mirror, only_portals,
            transversed_portals, portal_meshes, firstIndex, lastIndex);
      }
      else
      {
        new_position = isect;
      }
    }
    else
    {
      new_position = isect;
    }
  }

  if(lastIndex)
    *lastIndex = --firstIndex;

  return (iSector*)this;
}

void csSector::PrepareDraw (iRenderView *rview)
{
  if (engine->bugplug)
    engine->bugplug->AddCounter ("Sector Count", 1);

  // Make sure the visibility culler is loaded.
  GetVisibilityCuller ();
  CS::RenderManager::RenderView* csrview =
    (CS::RenderManager::RenderView*)rview;
  csrview->SetThisSector ((iSector*)this);

  size_t i = sectorCallbackList.GetSize ();
  while (i > 0)
  {
    i--;
    iSectorCallback* cb = sectorCallbackList.Get (i);
    cb->Traverse ((iSector*)this, rview);
  }

  // Mesh generators.
  const csVector3& pos = rview->GetCamera ()->GetTransform ().GetOrigin ();
  for (i = 0 ; i < meshGenerators.GetSize () ; i++)
  {
    meshGenerators[i]->UpdateForPosition (pos);
  }

  // CS_ENTITY_CAMERA meshes have to be moved to right position first.
  const csArray<iMeshWrapper*>& cm = cameraMeshes;
  for (i = 0 ; i < cm.GetSize () ; i++)
  {
    iMeshWrapper* m = cm.Get (i);
    if (m->GetFlags ().Check (CS_ENTITY_CAMERA))
    {
      iMovable* mov = m->GetMovable ();
      // Temporarily move the object to the current camera.
      csReversibleTransform &mov_trans = mov->GetTransform ();
      iCamera *orig_cam = rview->GetOriginalCamera ();
      if (orig_cam)
      {
        // @@@ TEMPORARY: now CS_ENTITY_CAMERA only works at 0,0,0 position.
        csVector3 old_pos = mov_trans.GetOrigin ();
        mov_trans.SetOrigin (csVector3 (0));
        csOrthoTransform &orig_trans = orig_cam->GetTransform ();
        csVector3 v = orig_trans.GetO2TTranslation ();
        mov_trans.SetOrigin (mov_trans.GetOrigin () + v);
        csVector3 diff = old_pos - mov_trans.GetOrigin ();
        if (!(diff < .00001f))
          mov->UpdateMove ();
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
  iRenderLoop* rl = renderloop;
  if (!rl) rl = engine->GetCurrentDefaultRenderloop ();
  rl->Draw (rview, (iSector*)this);
}

void csSector::AddSectorMeshCallback (iSectorMeshCallback* cb)
{
  sectorMeshCallbackList.Push (cb);
}

void csSector::RemoveSectorMeshCallback (iSectorMeshCallback* cb)
{
  sectorMeshCallbackList.Delete (cb);
}

void csSector::FireNewMesh (iMeshWrapper* mesh)
{
  size_t i = sectorMeshCallbackList.GetSize ();
  while (i > 0)
  {
    i--;
    sectorMeshCallbackList[i]->NewMesh ((iSector*)this, mesh);
  }
}

void csSector::FireRemoveMesh (iMeshWrapper* mesh)
{
  size_t i = sectorMeshCallbackList.GetSize ();
  while (i > 0)
  {
    i--;
    sectorMeshCallbackList[i]->RemoveMesh ((iSector*)this, mesh);
  }
}

iObjectRegistry* csSector::GetObjectRegistry() const
{
  return engine->objectRegistry;
}

THREADED_CALLABLE_IMPL1(csSector, AddLight, csRef<iLight> light)
{
  GetLights()->Add(light);
  return true;
}

void csSector::SetDynamicAmbientLight (const csColor& color)
{
  dynamicAmbientLightColor = color;
  dynamicAmbientLightVersion++;
  svDynamicAmbient->SetValue (dynamicAmbientLightColor);
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
      b = mesh->GetTransformedBoundingBox (
          mesh->GetMovable ()->GetTransform ());
      bbox += b;
    }
  }
}


//---------------------------------------------------------------------------

iMeshGenerator* csSector::CreateMeshGenerator (const char* name)
{
  csRef<csMeshGenerator> meshgen;
  meshgen.AttachNew (new csMeshGenerator (engine));
  meshgen->SetSector (this);
  meshgen->QueryObject ()->SetName (name);
  meshGenerators.Push (meshgen);
  return (iMeshGenerator*)meshgen;
}

iMeshGenerator* csSector::GetMeshGenerator (size_t idx)
{
  return meshGenerators[idx];
}

iMeshGenerator* csSector::GetMeshGeneratorByName (const char* name)
{
  return meshGenerators.FindByName (name);
}

void csSector::RemoveMeshGenerator (const char* name)
{
  csMeshGenerator* m = static_cast<csMeshGenerator*>(GetMeshGeneratorByName (name));
  meshGenerators.Delete (m);
}

void csSector::RemoveMeshGenerator (size_t idx)
{
  meshGenerators.DeleteIndex (idx);
}

void csSector::RemoveMeshGenerators ()
{
  meshGenerators.DeleteAll ();
}

//---------------------------------------------------------------------------

void csSector::RegisterPortalMesh (iMeshWrapper* mesh)
{
  portalMeshes.Add (mesh);
}

void csSector::UnregisterPortalMesh (iMeshWrapper* mesh)
{
  portalMeshes.Delete (mesh);
}

CS_IMPLEMENT_STATIC_CLASSVAR_REF(csSector, svNames, SVNames,
                                 csSector::SVNamesHolder, ());

void csSector::UpdateFogSVs ()
{
  svFogColor->SetValue (fog.color);
  svFogMode->SetValue (int (fog.mode));
  svFogFadeStart->SetValue (fog.start);
  svFogFadeEnd->SetValue (fog.end);
  svFogLimit->SetValue (fog.limit);
  svFogDensity->SetValue (fog.density);
}

void csSector::SetupSVNames()
{
  if ((CS::ShaderVarStringID)(SVNames().dynamicAmbient) == CS::InvalidShaderVarStringID)
  {
    SVNames().dynamicAmbient = CS::ShaderVarName (engine->svNameStringSet,
      "dynamic ambient");
    SVNames().lightAmbient = CS::ShaderVarName (engine->svNameStringSet,
      "light ambient");
    SVNames().fogColor = CS::ShaderVarName (engine->svNameStringSet,
      "fog color");
    SVNames().fogMode = CS::ShaderVarName (engine->svNameStringSet,
      "fog mode");
    SVNames().fogFadeStart = CS::ShaderVarName (engine->svNameStringSet,
      "fog fade start");
    SVNames().fogFadeEnd = CS::ShaderVarName (engine->svNameStringSet,
      "fog fade end");
    SVNames().fogLimit = CS::ShaderVarName (engine->svNameStringSet,
      "fog limit");
    SVNames().fogDensity = CS::ShaderVarName (engine->svNameStringSet,
      "fog density");
  }
}

void csSector::UpdateLightBounds (csLight* light, 
                                  const csBox3& oldBox)
{
  lights.UpdateLightBounds (light, oldBox);
}

//---------------------------------------------------------------------------

void csSector::LightAmbientAccessor::PreGetValue (csShaderVariable* sv)
{
  csColor engineAmbient;
  sector->engine->csEngine::GetAmbientLight (engineAmbient);
  sv->SetValue (sector->dynamicAmbientLightColor + engineAmbient);
}
    
//---------------------------------------------------------------------------


csSectorList::csSectorList (csEngine* engine)
  : scfImplementationType (this), engine (engine)
{
  listener.AttachNew (new NameChangeListener (this));
}

csSectorList::~csSectorList ()
{
  RemoveAll ();
}

void csSectorList::NameChanged (iObject* object, const char* oldname,
  	const char* newname)
{
  csRef<iSector> sector = scfQueryInterface<iSector> (object);
  CS_ASSERT (sector != 0);
  CS::Threading::ScopedWriteLock lock(sectorLock);
  if (oldname) sectors_hash.Delete (oldname, sector);
  if (newname) sectors_hash.Put (newname, sector);
}

void csSectorList::FreeSector (iSector* item)
{
  // We scan all objects in this sector and unlink those
  // objects.
  item->UnlinkObjects ();
}

int csSectorList::Add (iSector *obj)
{
  const char* name = obj->QueryObject ()->GetName ();
  CS::Threading::ScopedWriteLock lock(sectorLock);
  if (name)
    sectors_hash.Put (name, obj);
  obj->QueryObject ()->AddNameChangeListener (listener);
  return (int)list.Push (obj);
}

void csSectorList::AddBatch (csRef<iSectorLoaderIterator> itr)
{
  CS::Threading::ScopedWriteLock lock(sectorLock);
  while(itr->HasNext())
  {
    iSector* obj = itr->Next();
    const char* name = obj->QueryObject ()->GetName ();
    if (name)
      sectors_hash.Put (name, obj);
    obj->QueryObject ()->AddNameChangeListener (listener);
    list.Push (obj);
  }
}

bool csSectorList::Remove (iSector *obj)
{
  engine->FireRemoveSector (obj);
  FreeSector (obj);
  const char* name = obj->QueryObject ()->GetName ();
  CS::Threading::ScopedWriteLock lock(sectorLock);
  if (name)
    sectors_hash.Delete (name, obj);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  return list.Delete (obj);
}

bool csSectorList::Remove (int n)
{
  CS::Threading::ScopedWriteLock lock(sectorLock);
  iSector* obj = list[n];
  FreeSector (obj);
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    sectors_hash.Delete (name, obj);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  return list.DeleteIndex (n);
}

void csSectorList::RemoveAll ()
{
  CS::Threading::ScopedWriteLock lock(sectorLock);
  size_t i;
  for (i = 0 ; i < list.GetSize () ; i++)
  {
    list[i]->QueryObject ()->RemoveNameChangeListener (listener);
    FreeSector (list[i]);
  }
  list.DeleteAll ();
  sectors_hash.DeleteAll ();
}

int csSectorList::GetCount () const
{
  CS::Threading::ScopedReadLock lock(sectorLock);
  return (int)list.GetSize ();
}

iSector* csSectorList::Get (int n) const
{
  CS::Threading::ScopedReadLock lock(sectorLock);
  return list.Get (n);
}

int csSectorList::Find (iSector *obj) const
{
  CS::Threading::ScopedReadLock lock(sectorLock);
  return (int)list.Find (obj);
}

iSector *csSectorList::FindByName (const char *Name) const
{
  CS::Threading::ScopedReadLock lock(sectorLock);
  return sectors_hash.Get (Name, 0);
}

