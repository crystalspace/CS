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
#include "cssys/sysfunc.h"
#include "qint.h"
#include "csutil/scf.h"
#include "ivaria/pmeter.h"
#include "csengine/engine.h"
#include "csengine/halo.h"
#include "csengine/camera.h"
#include "csengine/campos.h"
#include "csengine/light.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/thing.h"
#include "csengine/meshobj.h"
#include "csengine/cscoll.h"
#include "csengine/sector.h"
#include "csengine/cbufcube.h"
#include "csengine/texture.h"
#include "csengine/material.h"
#include "csengine/lghtmap.h"
#include "csengine/stats.h"
#include "csengine/cbuffer.h"
#include "csengine/lppool.h"
#include "csengine/radiosty.h"
#include "csengine/region.h"
#include "csgeom/fastsqrt.h"
#include "csgeom/polypool.h"
#include "csgfx/csimage.h"
#include "csutil/util.h"
#include "csutil/cfgacc.h"
#include "csutil/debug.h"
#include "csutil/vfscache.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "ivideo/halo.h"
#include "ivideo/txtmgr.h"
#include "ivideo/graph3d.h"
#include "iutil/vfs.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/cfgmgr.h"
#include "iutil/databuff.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "imap/reader.h"
#include "imap/ldrctxt.h"
#include "imesh/lighting.h"
#include "imesh/thing/polytmap.h"
#include "imesh/thing/curve.h"
#include "ivaria/reporter.h"
#include "iutil/plugin.h"
#include "iutil/virtclk.h"

//---------------------------------------------------------------------------
void csEngine::Report (const char *description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (!Reporter) Reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  
  if (Reporter)
  {
    Reporter->ReportV (
        CS_REPORTER_SEVERITY_NOTIFY,
        "crystalspace.engine.notify",
        description,
        arg);
  }
  else
  {
    csPrintfV (description, arg);
    csPrintf ("\n");
  }

  va_end (arg);
}

void csEngine::Warn (const char *description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (!Reporter) Reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  if (Reporter)
  {
    Reporter->ReportV (
        CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.engine.warning",
        description,
        arg);
  }
  else
  {
    csPrintfV (description, arg);
    csPrintf ("\n");
  }

  va_end (arg);
}

void csEngine::ReportBug (const char *description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (!Reporter) Reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  if (Reporter)
  {
    Reporter->ReportV (
        CS_REPORTER_SEVERITY_BUG,
        "crystalspace.engine.bug",
        description,
        arg);
  }
  else
  {
    csPrintfV (description, arg);
    csPrintf ("\n");
  }

  va_end (arg);
}

//---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE(csCameraPositionList)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iCameraPositionList)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCameraPositionList::CameraPositionList)
  SCF_IMPLEMENTS_INTERFACE(iCameraPositionList)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csCameraPositionList::csCameraPositionList ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiCameraPositionList);
}

iCameraPosition *csCameraPositionList::NewCameraPosition (const char *name)
{
  csVector3 v (0);
  csCameraPosition *newcp = new csCameraPosition (name, "", v, v, v);
  iCameraPosition *cp = &(newcp->scfiCameraPosition);
  Push (cp);
  cp->DecRef ();
  return cp;
}

iCameraPosition *csCameraPositionList::CameraPositionList::NewCameraPosition (
  const char *name)
{
  return scfParent->NewCameraPosition (name);
}

int csCameraPositionList::CameraPositionList::GetCount () const
{
  return scfParent->Length ();
}

iCameraPosition *csCameraPositionList::CameraPositionList::Get (int n) const
{
  return scfParent->Get (n);
}

int csCameraPositionList::CameraPositionList::Add (iCameraPosition *obj)
{
  return scfParent->Push (obj);
}

bool csCameraPositionList::CameraPositionList::Remove (iCameraPosition *obj)
{
  return scfParent->Delete (obj);
}

bool csCameraPositionList::CameraPositionList::Remove (int n)
{
  return scfParent->Delete (n);
}

void csCameraPositionList::CameraPositionList::RemoveAll ()
{
  scfParent->DeleteAll ();
}

int csCameraPositionList::CameraPositionList::Find (
  iCameraPosition *obj) const
{
  return scfParent->Find (obj);
}

iCameraPosition *csCameraPositionList::CameraPositionList::FindByName (
  const char *Name) const
{
  return scfParent->FindByName (Name);
}

//---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE(csCollectionList)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iCollectionList)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csCollectionList::CollectionList)
  SCF_IMPLEMENTS_INTERFACE(iCollectionList)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csCollectionList::csCollectionList ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiCollectionList);
}

iCollection *csCollectionList::NewCollection (const char *name)
{
  csCollection *c = new csCollection (csEngine::current_engine);
  c->SetName (name);
  Push (&(c->scfiCollection));
  c->DecRef ();
  return &(c->scfiCollection);
}

iCollection *csCollectionList::CollectionList::NewCollection (
  const char *name)
{
  return scfParent->NewCollection (name);
}

int csCollectionList::CollectionList::GetCount () const
{
  return scfParent->Length ();
}

iCollection *csCollectionList::CollectionList::Get (int n) const
{
  return scfParent->Get (n);
}

int csCollectionList::CollectionList::Add (iCollection *obj)
{
  return scfParent->Push (obj);
}

bool csCollectionList::CollectionList::Remove (iCollection *obj)
{
  return scfParent->Delete (obj);
}

bool csCollectionList::CollectionList::Remove (int n)
{
  return scfParent->Delete (n);
}

void csCollectionList::CollectionList::RemoveAll ()
{
  scfParent->DeleteAll ();
}

int csCollectionList::CollectionList::Find (iCollection *obj) const
{
  return scfParent->Find (obj);
}

iCollection *csCollectionList::CollectionList::FindByName (
  const char *Name) const
{
  return scfParent->FindByName (Name);
}

//---------------------------------------------------------------------------
csEngineMeshList::~csEngineMeshList ()
{
  DeleteAll ();
}

bool csEngineMeshList::FreeItem (csSome Item)
{
  iMeshWrapper *mesh = (iMeshWrapper *)Item;
  mesh->GetMovable ()->ClearSectors ();
  mesh->DecRef ();
  return true;
}

//---------------------------------------------------------------------------

/**
 * Iterator to iterate over sectors in the engine which are within
 * a radius from a given point.
 * This iterator assumes there are no fundamental changes
 * in the engine while it is being used.
 * If changes to the engine happen the results are unpredictable.
 */
class csSectorIt :
  public iSectorIterator
{
private:
  // The position and radius.
  csSector *sector;
  csVector3 pos;
  float radius;

  // Polygon index (to loop over all portals).
  // If -1 then we return current sector first.
  int cur_poly;

  // If not null then this is a recursive sector iterator
  // that we are currently using.
  csSectorIt *recursive_it;

  // If true then this iterator has ended.
  bool has_ended;

  // Last position (from Fetch).
  csVector3 last_pos;
public:
  /// Construct an iterator and initialize to start.
  csSectorIt (csSector *sector, const csVector3 &pos, float radius);

  /// Destructor.
  virtual ~csSectorIt ();

  SCF_DECLARE_IBASE;

  /// Restart iterator.
  virtual void Restart ();

  /// Get sector from iterator. Return NULL at end.
  virtual iSector *Fetch ();

  /**
   * Get last position that was used from Fetch. This can be
   * different from 'pos' because of space warping.
   */
  virtual const csVector3 &GetLastPosition () { return last_pos; }
};

/**
 * Iterator to iterate over objects in the engine.
 * This iterator assumes there are no fundamental changes
 * in the engine while it is being used.
 * If changes to the engine happen the results are unpredictable.
 */
class csObjectIt :
  public iObjectIterator
{
  friend class csEngine;
private:
  // The engine for this iterator.
  csEngine *engine;

  // The starting position and radius.
  csSector *start_sector;
  csVector3 start_pos;
  float radius;

  // Current position ('pos' can be warped so that's why it is here).
  csSector *cur_sector;
  csVector3 cur_pos;

  // Current object.
  iObject *cur_object;

  // Current object for iterator.
  iObject *it_cur_obj;

  // Current index.
  int cur_idx;

  // Iterator over the sectors.
  csSectorIt *sectors_it;

  // Current object list to iterate over
  enum
  {
    ITERATE_SECTORS,
    ITERATE_STATLIGHTS,
    ITERATE_DYNLIGHTS,
    ITERATE_MESHES,
    ITERATE_NONE
  } CurrentList;
private:
  // Start looking for stuff.
  void StartStatLights ();
  void StartMeshes ();
  void EndSearch ();

  /// Construct an iterator and initialize to start.
  csObjectIt (
    csEngine *,
    csSector *sector,
    const csVector3 &pos,
    float radius);
  iObject *Fetch ();
public:
  /// Destructor.
  virtual ~csObjectIt ();

  SCF_DECLARE_IBASE;

  virtual void Reset ();
  virtual bool Next ();
  virtual iObject *GetObject () const;
  virtual bool IsFinished () const;
  virtual iObject *GetParentObj () const
  {
    return NULL;
  }

  virtual bool FindName (const char *name)  { (void)name; return false; }
};

//---------------------------------------------------------------------------
csLightIt::csLightIt (csEngine *e, iRegion *r) :
  engine(e),
  region(r)
{
  Restart ();
}

bool csLightIt::NextSector ()
{
  sector_idx++;
  if (region)
    while
    (
      sector_idx < engine->sectors.Length () &&
      !region->IsInRegion (GetLastSector ())
    ) sector_idx++;
  if (sector_idx >= engine->sectors.Length ()) return false;
  return true;
}

void csLightIt::Restart ()
{
  sector_idx = -1;
  light_idx = 0;
}

csLight *csLightIt::Fetch ()
{
  csSector *sector;
  if (sector_idx == -1)
  {
    if (!NextSector ()) return NULL;
    light_idx = -1;
  }

  if (sector_idx >= engine->sectors.Length ()) return NULL;
  sector = engine->sectors[sector_idx]->GetPrivateObject ();

  // Try next light.
  light_idx++;

  if (light_idx >= sector->scfiSector.GetLights ()->GetCount ())
  {
    // Go to next sector.
    light_idx = -1;
    if (!NextSector ()) return NULL;

    // Initialize iterator to start of sector and recurse.
    return Fetch ();
  }

  return sector->scfiSector.GetLights ()->Get (light_idx)->GetPrivateObject ();
}

csSector *csLightIt::GetLastSector ()
{
  return engine->sectors[sector_idx]->GetPrivateObject ();
}

//---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE(csSectorIt)
  SCF_IMPLEMENTS_INTERFACE(iSectorIterator)
SCF_IMPLEMENT_IBASE_END

csSectorIt::csSectorIt (
  csSector *sector,
  const csVector3 &pos,
  float radius)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csSectorIt::sector = sector;
  csSectorIt::pos = pos;
  csSectorIt::radius = radius;
  recursive_it = NULL;

  Restart ();
}

csSectorIt::~csSectorIt ()
{
  delete recursive_it;
}

void csSectorIt::Restart ()
{
  cur_poly = -1;
  delete recursive_it;
  recursive_it = NULL;
  has_ended = false;
}

iSector *csSectorIt::Fetch ()
{
  if (has_ended) return NULL;

  if (recursive_it)
  {
    iSector *rc = recursive_it->Fetch ();
    if (rc)
    {
      last_pos = recursive_it->GetLastPosition ();
      return rc;
    }

    delete recursive_it;
    recursive_it = NULL;
  }

  if (cur_poly == -1)
  {
    cur_poly = 0;
    last_pos = pos;
    return sector ? &(sector->scfiSector) : NULL;
  }

#if 0
  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  // @@@ This function should try to use the octree if available to

  // quickly discard lots of polygons that cannot be close enough.
  while (cur_poly < sector->GetPolygonCount ())
  {
    csPolygon3D *p = sector->GetPolygon3D (cur_poly);
    cur_poly++;

    csPortal *po = p->GetPortal ();
    if (po)
    {
      const csPlane3 &wpl = p->GetPlane ()->GetWorldPlane ();
      float d = wpl.Distance (pos);
      if (d < radius && csMath3::Visible (pos, wpl))
      {
        if (!po->GetSector ()) po->CompleteSector ();

        csVector3 new_pos;
        if (po->flags.Check (CS_PORTAL_WARP))
          new_pos = po->Warp (pos);
        else
          new_pos = pos;
        recursive_it = new csSectorIt (po->GetSector (), new_pos, radius);
        return Fetch ();
      }
    }
  }
#endif

  // End search.
  has_ended = true;
  return NULL;
}

//---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE(csObjectIt)
  SCF_IMPLEMENTS_INTERFACE(iObjectIterator)
SCF_IMPLEMENT_IBASE_END

csObjectIt::csObjectIt (
  csEngine *e,
  csSector *sector,
  const csVector3 &pos,
  float radius)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csObjectIt::engine = e;
  csObjectIt::start_sector = sector;
  csObjectIt::start_pos = pos;
  csObjectIt::radius = radius;
  sectors_it = new csSectorIt (sector, pos, radius);

  Reset ();
}

csObjectIt::~csObjectIt ()
{
  delete sectors_it;
}

void csObjectIt::Reset ()
{
  CurrentList = ITERATE_DYNLIGHTS;
  cur_object = engine->GetFirstDynLight ();
  sectors_it->Restart ();
  it_cur_obj = NULL;
}

void csObjectIt::StartStatLights ()
{
  CurrentList = ITERATE_STATLIGHTS;
  cur_idx = 0;
}

void csObjectIt::StartMeshes ()
{
  CurrentList = ITERATE_MESHES;
  cur_idx = 0;
}

void csObjectIt::EndSearch ()
{
  CurrentList = ITERATE_NONE;
}

bool csObjectIt::Next ()
{
  it_cur_obj = Fetch ();
  return it_cur_obj != NULL;
}

iObject *csObjectIt::GetObject () const
{
  return it_cur_obj;
}

bool csObjectIt::IsFinished () const
{
  return it_cur_obj != NULL;
}

iObject *csObjectIt::Fetch ()
{
  if (CurrentList == ITERATE_NONE) return NULL;

  // Handle csDynLight.
  if (CurrentList == ITERATE_DYNLIGHTS)
  {
    if (cur_object == NULL)
      CurrentList = ITERATE_SECTORS;
    else
    {
      do
      {
        iObject *rc = cur_object;
        iDynLight *dl = SCF_QUERY_INTERFACE_FAST (rc, iDynLight);
        if (!dl)
          cur_object = NULL;
        else
        {
          cur_object = dl->GetNext ()->QueryObject ();

          float r = dl->QueryLight ()->GetRadius () + radius;
          if (
            csSquaredDist::PointPoint (
                dl->QueryLight ()->GetCenter (),
                cur_pos) <= r * r)
          {
            dl->DecRef ();
            return rc;
          }
          else
            dl->DecRef ();
        }
      } while (cur_object);
      if (cur_object == NULL) CurrentList = ITERATE_SECTORS;
    }
  }

  // Handle this sector first.
  if (CurrentList == ITERATE_SECTORS)
  {
    iSector *s = sectors_it->Fetch ();
    if (s)
      cur_sector = s->GetPrivateObject ();
    else
      cur_sector = NULL;
    if (!cur_sector)
    {
      EndSearch ();
      return NULL;
    }

    cur_pos = sectors_it->GetLastPosition ();

    StartStatLights ();
    return cur_sector;
  }

  // Handle csLight.
  if (CurrentList == ITERATE_STATLIGHTS)
  {
    if (cur_idx >= cur_sector->scfiSector.GetLights ()->GetCount ())
      StartMeshes ();
    else
    {
      iLightList *ll = cur_sector->scfiSector.GetLights ();
      do
      {
        iObject *rc = ll->Get (cur_idx)->QueryObject ();
        cur_idx++;

        iStatLight *sl = SCF_QUERY_INTERFACE_FAST (rc, iStatLight);
        if (sl)
        {
          float r = sl->QueryLight ()->GetRadius () + radius;
          if (
            csSquaredDist::PointPoint (
                sl->QueryLight ()->GetCenter (),
                cur_pos) <= r * r)
          {
            sl->DecRef ();
            return rc;
          }
          else
            sl->DecRef ();
        }
      } while (cur_idx < ll->GetCount ());
      if (cur_idx >= ll->GetCount ()) StartMeshes ();
    }
  }

  // Handle csMeshWrapper.
  if (CurrentList == ITERATE_MESHES)
  {
    if (cur_idx >= cur_sector->GetMeshes ()->GetCount ())
      CurrentList = ITERATE_SECTORS;
    else
    {
      iObject *rc = cur_sector->GetMeshes ()->Get (cur_idx)->QueryObject ();
      cur_idx++;
      return rc;
    }
  }
  else
    CurrentList = ITERATE_SECTORS;

  return NULL;
}

//---------------------------------------------------------------------------
int csEngine:: frame_width;
int csEngine:: frame_height;
iObjectRegistry *csEngine:: object_reg = NULL;
csEngine *csEngine:: current_engine = NULL;
iEngine *csEngine:: current_iengine = NULL;
bool csEngine:: use_new_radiosity = false;
int csEngine:: max_process_polygons = 2000000000;
int csEngine:: cur_process_polygons = 0;
int csEngine:: lightcache_mode = CS_ENGINE_CACHE_READ;
int csEngine:: lightmap_quality = 3;
bool csEngine:: do_force_revis = false;
bool csEngine:: do_rad_debug = false;

SCF_IMPLEMENT_IBASE(csEngine)
  SCF_IMPLEMENTS_INTERFACE(iEngine)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iConfig)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iObject)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csEngine::eiComponent)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE
  (csEngine::iObjectInterface) void *itf = csObject::QueryInterface
    (iInterfaceID, iVersion);
  if (itf) return itf;
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csEngine::EventHandler)
  SCF_IMPLEMENTS_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csEngine)
  SCF_EXPORT_CLASS_TABLE(engine)
  SCF_EXPORT_CLASS_DEP
  (
    csEngine, "crystalspace.engine.3d", "Crystal Space 3D Engine",
      "crystalspace.kernel., crystalspace.graphics3d., crystalspace.graphic.image.io."
  )
SCF_EXPORT_CLASS_TABLE_END

csEngine::csEngine (iBase *iParent) :
  sectors(true)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiConfig);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObject);
  DG_TYPE (&scfiObject, "csEngine");
  engine_mode = CS_ENGINE_AUTODETECT;
  first_dyn_lights = NULL;
  object_reg = NULL;
  VFS = NULL;
  G3D = NULL;
  G2D = NULL;
  Reporter = NULL;
  ImageLoader = NULL;
  textures = NULL;
  materials = NULL;
  c_buffer = NULL;
  cbufcube = NULL;
  current_camera = NULL;
  current_engine = this;
  current_iengine = SCF_QUERY_INTERFACE_FAST (this, iEngine);
  current_iengine->DecRef ();
  scfiEventHandler = NULL;
  use_pvs = false;
  use_pvs_only = false;
  freeze_pvs = false;
  clear_zbuf = false;
  engine_states = NULL;
  rad_debug = NULL;
  nextframe_pending = 0;
  virtual_clock = NULL;
  cache_mgr = NULL;

  cbufcube = new csCBufferCube (1024);
  InitCuller ();

  textures = new csTextureList ();
  materials = new csMaterialList ();

  render_pol2d_pool = new csPoly2DPool (csPolygon2DFactory::SharedFactory ());
  lightpatch_pool = new csLightPatchPool ();

  BuildSqrtTable ();
  resize = false;

  thing_type = new csThingObjectType (NULL);
  ClearRenderPriorities ();
}

// @@@ Hack
iCamera *camera_hack = NULL;

csEngine::~csEngine ()
{
  if (scfiEventHandler)
  {
    iEventQueue *q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q != 0)
    {
      q->RemoveListener (scfiEventHandler);
      q->DecRef ();
    }

    scfiEventHandler->DecRef ();
  }

  DeleteAll ();

  int i;
  for (i = 0; i < render_priorities.Length (); i++)
  {
    char *n = (char *)render_priorities[i];
    delete[] n;
  }

  render_priorities.DeleteAll ();

  if (G3D) G3D->DecRef ();
  if (ImageLoader) ImageLoader->DecRef ();
  if (VFS) VFS->DecRef ();
  if (Reporter) Reporter->DecRef ();
  if (virtual_clock) virtual_clock->DecRef ();
  thing_type->DecRef ();
  delete materials;
  delete textures;
  delete render_pol2d_pool;
  delete lightpatch_pool;
  delete cbufcube;
  delete rad_debug;
  delete c_buffer;

  // @@@ temp hack
  if (camera_hack) camera_hack->DecRef ();
  camera_hack = NULL;

  if (cache_mgr) cache_mgr->DecRef ();
}

bool csEngine::Initialize (iObjectRegistry *object_reg)
{
  csEngine::object_reg = object_reg;

  virtual_clock = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  if (!virtual_clock) return false;

  G3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!G3D)
  {
    // If there is no G3D then we still allow initialization of the
    // engine because it might be useful to use the engine stand-alone
    // (i.e. for calculating lighting and so on).
    Warn ("No 3D driver!");
  }

  VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!VFS) return false;

  if (G3D)
    G2D = G3D->GetDriver2D ();
  else
    G2D = NULL;

  // don't check for failure; the engine can work without the image loader
  ImageLoader = CS_QUERY_REGISTRY (object_reg, iImageIO);
  if (!ImageLoader) Warn ("No image loader. Loading images will fail.");

  // Reporter is optional.
  Reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  // Tell event queue that we want to handle broadcast events
  if (!scfiEventHandler) scfiEventHandler = new EventHandler (this);

  iEventQueue *q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
  if (q)
  {
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast);
    q->DecRef ();
  }

  csConfigAccess cfg (object_reg, "/config/engine.cfg");
  ReadConfig (cfg);

  thing_type->Initialize (object_reg);

  return true;
}

// Handle some system-driver broadcasts
bool csEngine::HandleEvent (iEvent &Event)
{
  if (Event.Type == csevBroadcast)
  {
    switch (Event.Command.Code)
    {
      case cscmdSystemOpen:
        {
          if (G3D)
          {
            csGraphics3DCaps *caps = G3D->GetCaps ();
            fogmethod = caps->fog;
            NeedPO2Maps = caps->NeedsPO2Maps;
            MaxAspectRatio = caps->MaxAspectRatio;

            frame_width = G3D->GetWidth ();
            frame_height = G3D->GetHeight ();
          }
          else
          {
            fogmethod = G3DFOGMETHOD_NONE;
            NeedPO2Maps = false;
            MaxAspectRatio = 4096;
            frame_width = 640;
            frame_height = 480;
          }

          if (csCamera::GetDefaultFOV () == 0)
            csCamera::SetDefaultFOV (frame_height, frame_width);

          // @@@ Ugly hack to always have a camera in current_camera.
          // This is needed for the lighting routines.
          if (!current_camera)
          {
            current_camera = &(new csCamera ())->scfiCamera;
            camera_hack = current_camera;
          }

          // Allow context resizing since we handle cscmdContextResize
          if (G2D) G2D->AllowResize (true);

          StartEngine ();

          return true;
        }

      case cscmdSystemClose:
        {
          // We must free all material and texture handles since after
          // G3D->Close() they all become invalid, no matter whenever
          // we did or didn't an IncRef on them.
          DeleteAll ();
          return true;
        }

      case cscmdContextResize:
        {
          if (engine_states)
            engine_states->Resize ((iGraphics2D *)Event.Command.Info);
          else if (((iGraphics2D *)Event.Command.Info) == G2D)
            resize = true;
          return false;
        }

      case cscmdContextClose:
        {
          if (engine_states)
          {
            engine_states->Close ((iGraphics2D *)Event.Command.Info);
            if (!engine_states->Length ())
            {
              delete engine_states;
              engine_states = NULL;
            }
          }

          return false;
        }
    } /* endswitch */
  }

  return false;
}

void csEngine::DeleteAll ()
{
  nextframe_pending = 0;
  if (G3D) G3D->ClearCache ();
  halos.DeleteAll ();
  collections.DeleteAll ();

  int i;

  // @@@ instead of simply calling DeleteAll() we do this loop, because:
  // if a mesh is not held by anything outside the engine, it will get
  // destructed (RefCounter hits 0 and destructor is called). Unfortunatly
  // upon destruction the meshobject checks if it is still in a meshlist.
  // But this is even more unfortunately true, since the entry is removed
  // from the meshlist vector after the DecRef/Destruction happens.
  // With the loop below we simply make sure that the mesh is not destructed
  // while removing from the meshlist. ... norman
  for (i = GetMeshes ()->GetCount () - 1; i >= 0; i--)
  {
    iMeshWrapper *imw = GetMeshes ()->Get (i);
    GetMeshes ()->Remove (imw);                 // this will IncRef before removal and DecRef afterward
  }

  mesh_factories.DeleteAll ();

  // @@@ I suppose the loop below is no longer needed?
  for (i = 0; i < sectors.Length (); i++)
  {
    csSector *sect = sectors[i]->GetPrivateObject ();
    if (sect) sect->CleanupReferences ();
  }

  sectors.DeleteAll ();

  camera_positions.DeleteAll ();

  while (first_dyn_lights)
  {
    csDynLight *dyn = first_dyn_lights->GetNext ();
    delete first_dyn_lights;
    first_dyn_lights = dyn;
  }

  delete materials;
  materials = new csMaterialList ();
  delete textures;
  textures = new csTextureList ();

  iThingEnvironment *te = SCF_QUERY_INTERFACE (thing_type, iThingEnvironment);
  CS_ASSERT (te != NULL);
  te->ClearPolyTxtPlanes ();
  te->ClearCurveTemplates ();
  te->DecRef ();

  // Delete engine states and their references to cullers before cullers are

  // deleted in InitCuller below.
  if (engine_states)
  {
    engine_states->DeleteAll ();
    delete engine_states;
    engine_states = NULL;
  }

  InitCuller ();
  delete render_pol2d_pool;
  render_pol2d_pool = new csPoly2DPool (csPolygon2DFactory::SharedFactory ());
  delete lightpatch_pool;
  lightpatch_pool = new csLightPatchPool ();
  use_pvs = false;

  // Clear all regions.
  region = NULL;
  regions.DeleteAll ();

  // Clear all render priorities.
  ClearRenderPriorities ();

  // remove objects
  QueryObject ()->ObjRemoveAll ();
}

iObject *csEngine::QueryObject ()
{
  return &scfiObject;
}

void csEngine::RegisterRenderPriority (
  const char *name,
  long priority,
  int rendsort)
{
  int i;

  // If our priority goes over the number of defined priorities
  // then we have to initialize.
  if (priority + 1 >= render_priority_sortflags.Limit ())
    render_priority_sortflags.SetLimit (priority + 2);
  for (i = render_priorities.Length (); i <= priority; i++)
  {
    render_priorities[i] = NULL;
    render_priority_sortflags[i] = CS_RENDPRI_NONE;
  }

  char *n = (char *)render_priorities[priority];
  delete[] n;
  n = csStrNew (name);
  render_priorities[priority] = n;
  render_priority_sortflags[priority] = rendsort;
  if (!strcmp (name, "sky"))
    render_priority_sky = priority;
  else if (!strcmp (name, "wall"))
    render_priority_wall = priority;
  else if (!strcmp (name, "object"))
    render_priority_object = priority;
  else if (!strcmp (name, "alpha"))
    render_priority_alpha = priority;
}

long csEngine::GetRenderPriority (const char *name) const
{
  int i;
  for (i = 0; i < render_priorities.Length (); i++)
  {
    char *n = (char *)render_priorities[i];
    if (n && !strcmp (name, n)) return i;
  }

  return 0;
}

int csEngine::GetRenderPrioritySorting (const char *name) const
{
  int i;
  for (i = 0; i < render_priorities.Length (); i++)
  {
    char *n = (char *)render_priorities[i];
    if (n && !strcmp (name, n)) return render_priority_sortflags[i];
  }

  return CS_RENDPRI_NONE;
}

int csEngine::GetRenderPrioritySorting (long priority) const
{
  return render_priority_sortflags[priority];
}

void csEngine::ClearRenderPriorities ()
{
  int i;
  for (i = 0; i < render_priorities.Length (); i++)
  {
    char *n = (char *)render_priorities[i];
    delete[] n;
  }

  render_priorities.DeleteAll ();
  render_priority_sortflags.SetLimit (0);
  RegisterRenderPriority ("sky", 2);
  RegisterRenderPriority ("wall", 4);
  RegisterRenderPriority ("object", 6);
  RegisterRenderPriority ("alpha", 8, CS_RENDPRI_BACK2FRONT);
}

void csEngine::ResolveEngineMode ()
{
  if (engine_mode == CS_ENGINE_AUTODETECT)
  {
    // Here we do some heuristic. We scan all sectors and see if there
    // are some that have big octrees. If so we select CS_ENGINE_FRONT2BACK.
    // If not we select CS_ENGINE_BACK2FRONT.
    // @@@ It might be an interesting option to try to find a good engine
    // mode for every sector and switch dynamically based on the current
    // sector (only switch based on the position of the camera, not switch
    // based on the sector we are currently rendering).
    int switch_f2b = 0;
    int i;
    for (i = 0; i < sectors.Length (); i++)
    {
      csSector *s = sectors[i]->GetPrivateObject ();
      iMeshWrapper *ss = s->GetCullerMesh ();
      if (ss)
      {
#if 0
        // @@@ Disabled because we can't get at the tree now.
        csPolygonTree *ptree = ss->GetPrivateObject ()->GetStaticTree ();
        csOctree *otree = (csOctree *)ptree;
        int num_nodes = otree->GetRoot ()->CountChildren ();
        if (num_nodes > 30)
          switch_f2b += 10;
        else if (num_nodes > 15)
          switch_f2b += 5;
#else
        switch_f2b += 10;
#endif
      }

      if (switch_f2b >= 10) break;
    }

    if (switch_f2b >= 10)
    {
      engine_mode = CS_ENGINE_FRONT2BACK;
      Report ("Engine is using front2back mode.");
    }
    else
    {
      engine_mode = CS_ENGINE_BACK2FRONT;
      Report ("Engine is using back2front mode.");
    }
  }
}

int csEngine::GetLightmapCellSize () const
{
  return csLightMap::lightcell_size;
}

void csEngine::SetLightmapCellSize (int Size)
{
  csLightMap::lightcell_size = Size;
}

void csEngine::InitCuller ()
{
  delete c_buffer;
  c_buffer = NULL;
  c_buffer = new csCBuffer (0, frame_width - 1, frame_height);
}

void csEngine::PrepareTextures ()
{
  int i;

  iTextureManager *txtmgr = G3D->GetTextureManager ();
  txtmgr->ResetPalette ();

  // First register all textures to the texture manager.
  for (i = 0; i < textures->Length (); i++)
  {
    iTextureWrapper *csth = textures->Get (i);
    if (!csth->GetTextureHandle ()) csth->Register (txtmgr);
  }

  // Prepare all the textures.
  txtmgr->PrepareTextures ();

  // Then register all materials to the texture manager.
  for (i = 0; i < materials->Length (); i++)
  {
    iMaterialWrapper *csmh = materials->Get (i);
    if (!csmh->GetMaterialHandle ()) csmh->Register (txtmgr);
  }

  // Prepare all the materials.
  txtmgr->PrepareMaterials ();
}

// If a STAT_BSP level the loader call to UpdateMove is redundant when called
// before csEngine::Prepare().
void csEngine::PrepareMeshes ()
{
  int i;
  for (i = 0; i < meshes.Length (); i++)
  {
    iMeshWrapper *sp = (iMeshWrapper *)meshes[i];
    sp->GetMovable ()->UpdateMove ();
  }
}

bool csEngine::Prepare (iProgressMeter *meter)
{
  PrepareTextures ();
  PrepareMeshes ();

  // The images are no longer needed by the 3D engine.
  iTextureManager *txtmgr = G3D->GetTextureManager ();
  txtmgr->FreeImages ();

  G3D->ClearCache ();

  // Prepare lightmaps if we have any sectors
  if (sectors.Length ()) ShineLights (NULL, meter);

  CheckConsistency ();

  return true;
}

void csEngine::SetCacheManager (iCacheManager* cache_mgr)
{
  if (cache_mgr) cache_mgr->IncRef ();
  if (csEngine::cache_mgr) csEngine::cache_mgr->DecRef ();
  csEngine::cache_mgr = cache_mgr;
}

iCacheManager* csEngine::GetCacheManager ()
{
  if (!cache_mgr)
  {
    char buf[512];
    strcpy (buf, VFS->GetCwd ());
    if (buf[strlen (buf)-1] == '/')
      strcat (buf, "cache");
    else
      strcat (buf, "/cache");
    cache_mgr = new csVfsCacheManager (object_reg, buf);
  }
  return cache_mgr;
}

void csEngine::ShineLights (iRegion *iregion, iProgressMeter *meter)
{
  // If we have to read from the cache then we check if the 'precalc_info'
  // file exists on the VFS. If not then we cannot use the cache.
  // If the file exists but is not valid (some flags are different) then
  // we cannot use the cache either.
  // If we recalculate then we also update this 'precalc_info' file with
  // the new settings.
  struct PrecalcInfo
  {
    int lm_version; // This number identifies a version of the lightmap format.

    // If different then the format is different and we need
    // to recalculate.
    int normal_light_level; // Normal light level (unlighted level).
    int ambient_red;
    int ambient_green;
    int ambient_blue;
    int reflect;
    int radiosity;
    float cosinus_factor;
    int lightmap_size;
  };

  PrecalcInfo current;
  memset (&current, 0, sizeof (current));
  current.lm_version = 2;
  current.normal_light_level = CS_NORMAL_LIGHT_LEVEL;
  current.ambient_red = csLight::ambient_red;
  current.ambient_green = csLight::ambient_green;
  current.ambient_blue = csLight::ambient_blue;
  current.reflect = csSector::cfg_reflections;
  current.radiosity = (int)csSector::do_radiosity;
  current.cosinus_factor = csPolyTexture::cfg_cosinus_factor;
  current.lightmap_size = csLightMap::lightcell_size;

  char *reason = NULL;

  iDataBuffer *data = VFS->ReadFile ("precalc_info");
  if (!data)
    reason = "no 'precalc_info' found";
  else
  {
    char *input = **data;
    while (*input)
    {
      char *keyword = input + strspn (input, " \t");
      char *endkw = strchr (input, '=');
      if (!endkw) break;
      *endkw++ = 0;
      input = strchr (endkw, '\n');
      if (input) *input++ = 0;

      float xf;
      sscanf (endkw, "%f", &xf);

      int xi = QRound (xf);

#define CHECK(keyw, cond, reas) \
	else if (!strcmp (keyword, keyw)) \
	{ \
	  if (cond) \
	  { \
	    reason = reas " changed"; \
	    break; \
	  } \
	}

      if (false)
      {
      }

      CHECK ("LMVERSION", xi != current.lm_version, "lightmap format") CHECK (
        "LIGHTMAP_SIZE",
        xi != current.lightmap_size,
        "lightmap size")
#undef CHECK
    }
  }

  if (data) data->DecRef ();

  if (reason)
  {
    char data[500];
    sprintf (
      data,
      "LMVERSION=%d\nNORMALLIGHTLEVEL=%d\nAMBIENT_RED=%d\nAMBIENT_GREEN=%d\nAMBIENT_BLUE=%d\nREFLECT=%d\nRADIOSITY=%d\nCOSINUS_FACTOR=%g\nLIGHTMAP_SIZE=%d\n",
      current.lm_version,
      current.normal_light_level,
      current.ambient_red,
      current.ambient_green,
      current.ambient_blue,
      current.reflect,
      current.radiosity,
      current.cosinus_factor,
      current.lightmap_size);
    if (lightcache_mode & CS_ENGINE_CACHE_WRITE)
      VFS->WriteFile ("precalc_info", data, strlen (data));
    Report ("Lightmap data is not up to date (reason: %s).", reason);
    lightcache_mode &= ~CS_ENGINE_CACHE_READ;
  }

  if (!(lightcache_mode & CS_ENGINE_CACHE_READ))
  {
    Report ("Recalculation of lightmaps forced.");
    Report (
      "  Pseudo-radiosity system %s.",
      csSector::do_radiosity ? "enabled" : "disabled");
    Report (
      "  Maximum number of visits per sector = %d.",
      csSector::cfg_reflections);
  }
  else
  {
    // If no recalculation is forced we set these variables to default to
    // make sure that we don't do too many unneeded calculations.
    csSector::do_radiosity = false;
    csSector::cfg_reflections = 1;
  }

  csLightIt *lit = NewLightIterator (iregion);

  // Count number of lights to process.
  csLight *l;
  int light_count = 0;
  lit->Restart();
  while (lit->Fetch()) light_count++;

  bool do_relight = false;
  if (!(lightcache_mode & CS_ENGINE_CACHE_READ)) do_relight = true;

  int sn = 0;
  int num_meshes = meshes.Length();

  if (do_relight)
  {
    Report ("Initializing lighting (%d meshes).", num_meshes);
    if (meter)
    {
      meter->SetProgressDescription (
        "crystalspace.engine.lighting.init",
        "Initializing lighting (%d meshes).",
        num_meshes);
      meter->SetTotal (num_meshes);
      meter->Restart ();
    }
  }

  iCacheManager* cm = GetCacheManager ();

  for (sn = 0; sn < num_meshes; sn++)
  {
    iMeshWrapper *s = (iMeshWrapper *)meshes[sn];
    if (!region || region->IsInRegion (s->QueryObject ()))
    {
      iLightingInfo *linfo = SCF_QUERY_INTERFACE_FAST (
        s->GetMeshObject (),
        iLightingInfo);
      if (linfo)
      {
        if (do_relight)
          linfo->InitializeDefault ();
        else
          linfo->ReadFromCache (cm, 0);               // ID?
        linfo->DecRef ();
      }
    }

    if (do_relight && meter) meter->Step ();
  }

  csTicks start, stop;
  start = csGetTicks();
  if (do_relight)
  {
    Report ("Shining lights (%d lights).", light_count);
    if (meter)
    {
      meter->SetProgressDescription (
          "crystalspace.engine.lighting.calc",
        "Shining lights (%d lights)",
        light_count);
      meter->SetTotal (light_count);
      meter->Restart ();
    }

    lit->Restart ();
    while ((l = lit->Fetch ()) != NULL)
    {
      ((csStatLight *)l)->CalculateLighting ();
      if (meter) meter->Step ();
    }

    stop = csGetTicks ();
    Report ("Time taken: %.4f seconds.", (float)(stop - start) / 1000.);
  }

  // Render radiosity
  if (use_new_radiosity && do_relight)
  {
    start = csGetTicks ();

    csRadiosity *rad = new csRadiosity (this, meter);
    if (do_rad_debug)
    {
      rad_debug = rad;
    }
    else
    {
      rad->DoRadiosity ();
      delete rad;
    }

    stop = csGetTicks ();
    if (do_relight)
      Report ("Time taken: %.4f seconds.", (float)(stop - start) / 1000.);
  }

  if (do_relight)
  {
    Report ("Caching lighting (%d meshes).", num_meshes);
    if (meter)
    {
      meter->SetProgressDescription (
          "crystalspace.engine.lighting.cache",
          "Caching lighting (%d meshes)",
          num_meshes);
      meter->SetTotal (num_meshes);
      meter->Restart ();
    }
  }

  for (sn = 0; sn < num_meshes; sn++)
  {
    iMeshWrapper *s = (iMeshWrapper *)meshes[sn];
    if (!region || region->IsInRegion (s->QueryObject ()))
    {
      iLightingInfo *linfo = SCF_QUERY_INTERFACE_FAST (
          s->GetMeshObject (),
          iLightingInfo);
      if (linfo)
      {
        if (do_relight) linfo->WriteToCache (cm, 0);  // @@@ id
        linfo->PrepareLighting ();
        linfo->DecRef ();
      }
    }

    if (do_relight && meter) meter->Step ();
  }

  csThing::current_light_frame_number++;

  if (do_relight) Report ("Updating VFS....");
  if (!VFS->Sync()) Warn ("Error updating lighttable cache!");
  if (do_relight) Report ("DONE!");

  delete lit;
}
void csEngine::InvalidateLightmaps ()
{
#if 0
  //@@@@ TODO!!
  csPolyIt *pit = NewPolyIterator ();
  csPolygon3D *p;
  while ((p = pit->Fetch ()) != NULL)
  {
    csPolyTexLightMap *lmi = p->GetLightMapInfo ();
    if (lmi && lmi->GetLightMap ())
      ((csLightMap *)lmi->GetLightMap ())->MakeDirtyDynamicLights ();
  }

  delete pit;
#endif
}

bool csEngine::CheckConsistency ()
{
  return false;
}

void csEngine::StartEngine ()
{
  DeleteAll ();
}

void csEngine::StartDraw (iCamera *c, iClipper2D *view, csRenderView &rview)
{
  Stats::polygons_considered = 0;
  Stats::polygons_drawn = 0;
  Stats::portals_drawn = 0;
  Stats::polygons_rejected = 0;
  Stats::polygons_accepted = 0;

  // Make sure we select an engine mode.
  ResolveEngineMode ();

  current_camera = c;
  rview.SetEngine (this);
  rview.SetOriginalCamera (c);

  // This flag is set in HandleEvent on a cscmdContextResize event
  if (resize)
  {
    resize = false;
    Resize ();
  }

  top_clipper = view;

  rview.GetClipPlane ().Set (0, 0, 1, -1);      //@@@CHECK!!!

  // Calculate frustum for screen dimensions (at z=1).
  float leftx = -c->GetShiftX () * c->GetInvFOV ();
  float rightx = (frame_width - c->GetShiftX ()) * c->GetInvFOV ();
  float topy = -c->GetShiftY () * c->GetInvFOV ();
  float boty = (frame_height - c->GetShiftY ()) * c->GetInvFOV ();
  rview.SetFrustum (leftx, rightx, topy, boty);

  // Initialize the 2D/3D culler.
  if (engine_mode == CS_ENGINE_FRONT2BACK)
  {
    if (c_buffer)
    {
      c_buffer->Initialize ();
      c_buffer->InsertPolygon (
          view->GetClipPoly (),
          view->GetVertexCount (),
          true);
    }
  }

  cur_process_polygons = 0;
}

void csEngine::Draw (iCamera *c, iClipper2D *view)
{
  ControlMeshes ();

  csRenderView rview (c, view, G3D, G2D);
  StartDraw (c, view, rview);
  rview.SetCallback (NULL);

  // First initialize G3D with the right clipper.
  G3D->SetClipper (view, CS_CLIPPER_TOPLEVEL);  // We are at top-level.
  G3D->ResetNearPlane ();
  G3D->SetPerspectiveAspect (c->GetFOV ());

  iSector *s = c->GetSector ();
  s->Draw (&rview);

  // draw all halos on the screen
  if (halos.Length () > 0)
  {
    csTicks elapsed = virtual_clock->GetElapsedTicks ();
    for (int halo = halos.Length () - 1; halo >= 0; halo--)
      if (!halos.Get (halo)->Process (elapsed, *this)) halos.Delete (halo);
  }

  G3D->SetClipper (NULL, CS_CLIPPER_NONE);
}

void csEngine::DrawFunc (
  iCamera *c,
  iClipper2D *view,
  iDrawFuncCallback *callback)
{
  ControlMeshes ();

  csRenderView rview (c, view, G3D, G2D);
  StartDraw (c, view, rview);
  rview.SetCallback (callback);

  // First initialize G3D with the right clipper.
  G3D->SetClipper (view, CS_CLIPPER_TOPLEVEL);  // We are at top-level.
  G3D->ResetNearPlane ();
  G3D->SetPerspectiveAspect (c->GetFOV ());

  iSector *s = c->GetSector ();
  s->Draw (&rview);

  G3D->SetClipper (NULL, CS_CLIPPER_NONE);
}

void csEngine::AddHalo (csLight *Light)
{
  if (!Light->GetHalo () || Light->flags.Check (CS_LIGHT_ACTIVEHALO))
    return ;

  // Transform light pos into camera space and see if it is directly visible
  csVector3 v = current_camera->GetTransform ().Other2This (
      Light->GetCenter ());

  // Check if light is behind us
  if (v.z <= SMALL_Z) return ;

  // Project X,Y into screen plane
  float iz = current_camera->GetFOV () / v.z;
  v.x = v.x * iz + current_camera->GetShiftX ();
  v.y = frame_height - 1 - (v.y * iz + current_camera->GetShiftY ());

  // If halo is not inside visible region, return
  if (!top_clipper->IsInside (csVector2 (v.x, v.y))) return ;

  // Check if light is not obscured by anything
  float zv = G3D->GetZBuffValue (QRound (v.x), QRound (v.y));
  if (v.z > zv) return ;

  // Halo size is 1/4 of the screen height; also we make sure its odd
  int hs = (frame_height / 4) | 1;

  if (Light->GetHalo ()->Type == cshtFlare)
  {
    // put a new light flare into the queue

    // the cast is safe because of the type check above
    halos.Push (
        new csLightFlareHalo (Light, (csFlareHalo *)Light->GetHalo (), hs));
    return ;
  }

  // Okay, put the light into the queue: first we generate the alphamap
  unsigned char *Alpha = Light->GetHalo ()->Generate (hs);
  iHalo *handle = G3D->CreateHalo (
      Light->GetColor ().red,
      Light->GetColor ().green,
      Light->GetColor ().blue,
      Alpha,
      hs,
      hs);

  // We don't need alpha map anymore
  delete[] Alpha;

  // Does 3D rasterizer support halos?
  if (!handle) return ;

  halos.Push (new csLightHalo (Light, handle));
}

void csEngine::RemoveHalo (csLight *Light)
{
  int idx = halos.FindKey (Light);
  if (idx != -1) halos.Delete (idx);
}

iStatLight *csEngine::FindLight (unsigned long light_id) const
{
  for (int i = 0; i < sectors.Length (); i++)
  {
    iLight *l = sectors[i]->GetLights ()->FindByID (light_id);
    if (l)
    {
      iStatLight *sl = SCF_QUERY_INTERFACE (l, iStatLight);
      if (sl)
      {
        // @@@ this might destroy the object!
        sl->DecRef ();
        return sl;
      }
    }
  }

  return NULL;
}

iStatLight *csEngine::FindLight (const char *name, bool regionOnly) const
{
  // XXX: Need to implement region?
  (void)regionOnly;

  // @@@### regionOnly
  for (int i = 0; i < sectors.Length (); i++)
  {
    iLight *l = sectors[i]->GetLights ()->FindByName (name);
    if (l)
    {
      iStatLight *sl = SCF_QUERY_INTERFACE (l, iStatLight);
      if (sl)
      {
        // @@@ this might destroy the object!
        sl->DecRef ();
        return sl;
      }
    }
  }

  return NULL;
}

void csEngine::AddDynLight (csDynLight *dyn)
{
  dyn->SetNext (first_dyn_lights);
  dyn->SetPrev (NULL);
  if (first_dyn_lights) first_dyn_lights->SetPrev (dyn);
  first_dyn_lights = dyn;
}

void csEngine::RemoveDynLight (csDynLight *dyn)
{
  if (dyn->GetNext ()) dyn->GetNext ()->SetPrev (dyn->GetPrev ());
  if (dyn->GetPrev ())
    dyn->GetPrev ()->SetNext (dyn->GetNext ());
  else if (dyn == first_dyn_lights)
    first_dyn_lights = dyn->GetNext ();
  dyn->SetNext (NULL);
  dyn->SetPrev (NULL);
}

void csEngine::ControlMeshes ()
{
  nextframe_pending = virtual_clock->GetCurrentTicks ();

  // Delete particle systems that self-destructed now.

  // @@@ Need to optimize this?
  int i = meshes.Length () - 1;
  while (i >= 0)
  {
    iMeshWrapper *sp = (iMeshWrapper *)meshes[i];
    if (sp->WantToDie ()) GetMeshes ()->Remove (sp);
    i--;
  }
}

char* csEngine::SplitRegionName (const char* name, iRegion*& region,
	bool& global)
{
  region = NULL;
  global = false;

  char* p = strchr (name, '/');
  if (!p) return (char*)name;
  if (*name == '*' && *(name+1) == '/')
  {
    global = true;
    return p+1;
  }

  *p = 0;
  region = regions.FindByName (name);
  *p = '/';
  if (!region) return NULL;
  return p+1;
}

iMaterialWrapper* csEngine::FindMaterial (const char* name,
	iRegion* reg)
{
  iRegion* region;
  bool global;
  char* n = SplitRegionName (name, region, global);
  if (!n) return NULL;

  iMaterialWrapper* mat;
  if (region)
    mat = region->FindMaterial (n);
  else if (!global && reg)
    mat = reg->FindMaterial (n);
  else
    mat = GetMaterialList ()->FindByName (n);
  return mat;
}

iTextureWrapper* csEngine::FindTexture (const char* name,
	iRegion* reg)
{
  iRegion* region;
  bool global;
  char* n = SplitRegionName (name, region, global);
  if (!n) return NULL;

  iTextureWrapper* txt;
  if (region)
    txt = region->FindTexture (n);
  else if (!global && reg)
    txt = reg->FindTexture (n);
  else
    txt = GetTextureList ()->FindByName (n);
  return txt;
}

iSector* csEngine::FindSector (const char* name,
	iRegion* reg)
{
  iRegion* region;
  bool global;
  char* n = SplitRegionName (name, region, global);
  if (!n) return NULL;

  iSector* sect;
  if (region)
    sect = region->FindSector (n);
  else if (!global && reg)
    sect = reg->FindSector (n);
  else
    sect = GetSectors ()->FindByName (n);
  return sect;
}

iMeshWrapper* csEngine::FindMeshObject (const char* name,
	iRegion* reg)
{
  iRegion* region;
  bool global;
  char* n = SplitRegionName (name, region, global);
  if (!n) return NULL;

  iMeshWrapper* mesh;
  if (region)
    mesh = region->FindMeshObject (n);
  else if (!global && reg)
    mesh = reg->FindMeshObject (n);
  else
    mesh = GetMeshes ()->FindByName (n);
  return mesh;
}

iMeshFactoryWrapper* csEngine::FindMeshFactory (const char* name,
	iRegion* reg)
{
  iRegion* region;
  bool global;
  char* n = SplitRegionName (name, region, global);
  if (!n) return NULL;

  iMeshFactoryWrapper* fact;
  if (region)
    fact = region->FindMeshFactory (n);
  else if (!global && reg)
    fact = reg->FindMeshFactory (n);
  else
    fact = GetMeshFactories ()->FindByName (n);
  return fact;
}

iCameraPosition* csEngine::FindCameraPosition (const char* name,
	iRegion* reg)
{
  iRegion* region;
  bool global;
  char* n = SplitRegionName (name, region, global);
  if (!n) return NULL;

  iCameraPosition* campos;
  if (region)
    campos = region->FindCameraPosition (n);
  else if (!global && reg)
    campos = reg->FindCameraPosition (n);
  else
    campos = GetCameraPositions ()->FindByName (n);
  return campos;
}

iCollection* csEngine::FindCollection (const char* name,
	iRegion* reg)
{
  iRegion* region;
  bool global;
  char* n = SplitRegionName (name, region, global);
  if (!n) return NULL;

  iCollection* col;
  if (region)
    col = region->FindCollection (n);
  else if (!global && reg)
    col = reg->FindCollection (n);
  else
    col = GetCollections ()->FindByName (n);
  return col;
}

void csEngine::ReadConfig (iConfigFile *Config)
{
  csLightMap::SetLightCellSize (
      Config->GetInt ("Engine.Lighting.LightmapSize", 16));
  csEngine::lightmap_quality = Config->GetInt (
      "Engine.Lighting.LightmapQuality",
      3);

  csLight::ambient_red = Config->GetInt (
      "Engine.Lighting.Ambient.Red",
      CS_DEFAULT_LIGHT_LEVEL);
  csLight::ambient_green = Config->GetInt (
      "Engine.Lighting.Ambient.Green",
      CS_DEFAULT_LIGHT_LEVEL);
  csLight::ambient_blue = Config->GetInt (
      "Engine.Lighting.Ambient.Blue",
      CS_DEFAULT_LIGHT_LEVEL);

  int ambient_white = Config->GetInt (
      "Engine.Lighting.Ambient.White",
      CS_DEFAULT_LIGHT_LEVEL);
  csLight::ambient_red += ambient_white;
  csLight::ambient_green += ambient_white;
  csLight::ambient_blue += ambient_white;

  // Do not allow too black environments since software renderer hates it
  if (
    csLight::ambient_red +
      csLight::ambient_green +
      csLight::ambient_blue < 6)
    csLight::ambient_red = csLight::ambient_green = csLight::ambient_blue = 2;

  csSector::cfg_reflections = Config->GetInt (
      "Engine.Lighting.Reflections",
      csSector::cfg_reflections);
  csPolyTexture::cfg_cosinus_factor = Config->GetFloat (
      "Engine.Lighting.CosinusFactor",
      csPolyTexture::cfg_cosinus_factor);

  //@@@ NOT THE RIGHT PLACE! csSprite3D::global_lighting_quality = Config->GetInt ("Engine.Lighting.SpriteQuality", csSprite3D::global_lighting_quality);
  csSector::do_radiosity = Config->GetBool (
      "Engine.Lighting.Radiosity",
      csSector::do_radiosity);

  // radiosity options
  csEngine::use_new_radiosity = Config->GetBool (
      "Engine.Lighting.Radiosity.Enable",
      csEngine::use_new_radiosity);
  csRadiosity::do_static_specular = Config->GetBool (
      "Engine.Lighting.Radiosity.DoStaticSpecular",
      csRadiosity::do_static_specular);
  csRadiosity::static_specular_amount = Config->GetFloat (
      "Engine.Lighting.Radiosity.StaticSpecularAmount",
      csRadiosity::static_specular_amount);
  csRadiosity::static_specular_tightness = Config->GetInt (
      "Engine.Lighting.Radiosity.StaticSpecularTightness",
      csRadiosity::static_specular_tightness);
  csRadiosity::colour_bleed = Config->GetFloat (
      "Engine.Lighting.Radiosity.ColourBleed",
      csRadiosity::colour_bleed);
  csRadiosity::stop_priority = Config->GetFloat (
      "Engine.Lighting.Radiosity.StopPriority",
      csRadiosity::stop_priority);
  csRadiosity::stop_improvement = Config->GetFloat (
      "Engine.Lighting.Radiosity.StopImprovement",
      csRadiosity::stop_improvement);
  csRadiosity::stop_iterations = Config->GetInt (
      "Engine.Lighting.Radiosity.StopIterations",
      csRadiosity::stop_iterations);
  csRadiosity::source_patch_size = Config->GetInt (
      "Engine.Lighting.Radiosity.SourcePatchSize",
      csRadiosity::source_patch_size);
}

struct LightAndDist
{
  iLight *light;
  float sqdist;
};

// csLightArray is a subclass of csCleanable which is registered

// to csEngine.cleanup.
class csLightArray :
  public iBase
{
public:
  SCF_DECLARE_IBASE;

  LightAndDist *array;

  // Size is the physical size of the array. num_lights is the number of lights in it.
  int size, num_lights;

  csLightArray () :
  array(NULL),
  size(0),
  num_lights(0)
  {
    SCF_CONSTRUCT_IBASE (NULL);
  }

  virtual ~csLightArray ()  { delete[] array; }
  void Reset ()             { num_lights = 0; }
  void AddLight (iLight *l, float sqdist)
  {
    if (num_lights >= size)
    {
      LightAndDist *new_array;
      new_array = new LightAndDist[size + 5];
      if (array)
      {
        memcpy (new_array, array, sizeof (LightAndDist) * num_lights);
        delete[] array;
      }

      array = new_array;
      size += 5;
    }

    array[num_lights].light = l;
    array[num_lights++].sqdist = sqdist;
  };
  iLight *GetLight (int i)  { return array[i].light; }
};

SCF_IMPLEMENT_IBASE(csLightArray)
  SCF_IMPLEMENTS_INTERFACE(iBase)
SCF_IMPLEMENT_IBASE_END;

static int compare_light (const void *p1, const void *p2)
{
  LightAndDist *sp1 = (LightAndDist *)p1;
  LightAndDist *sp2 = (LightAndDist *)p2;
  float z1 = sp1->sqdist;
  float z2 = sp2->sqdist;
  if (z1 < z2)
    return -1;
  else if (z1 > z2)
    return 1;
  return 0;
}

int csEngine::GetNearbyLights (
  iSector *sector,
  const csVector3 &pos,
  uint32 flags,
  iLight **lights,
  int max_num_lights)
{
  int i;
  float sqdist;

  // This is a static light array which is adapted to the
  // right size everytime it is used. In the beginning it means
  // that this array will grow a lot but finally it will
  // stabilize to a maximum size (not big). The advantage of
  // this approach is that we don't have a static array which can
  // overflow. And we don't have to do allocation every time we
  // come here. We register this memory to the 'cleanup' array
  // in csEngine so that it will be freed later.
  static csLightArray *light_array = NULL;
  if (!light_array)
  {
    light_array = new csLightArray ();
    csEngine::current_engine->cleanup.Push (light_array);
  }

  light_array->Reset ();

  // Add all dynamic lights to the array (if CS_NLIGHT_DYNAMIC is set).
  if (flags & CS_NLIGHT_DYNAMIC)
  {
    csDynLight *dl = first_dyn_lights;
    while (dl)
    {
      if (dl->GetSector () == sector->GetPrivateObject ())
      {
        sqdist = csSquaredDist::PointPoint (pos, dl->GetCenter ());
        if (sqdist < dl->GetSquaredRadius ())
        {
          iLight *il = SCF_QUERY_INTERFACE_FAST (dl, iLight);
          light_array->AddLight (il, sqdist);
          il->DecRef ();
        }
      }

      dl = dl->GetNext ();
    }
  }

  // Add all static lights to the array (if CS_NLIGHT_STATIC is set).
  if (flags & CS_NLIGHT_STATIC)
  {
    iLightList *ll = sector->GetLights ();
    for (i = 0; i < ll->GetCount (); i++)
    {
      iLight *l = ll->Get (i);
      sqdist = csSquaredDist::PointPoint (pos, l->GetCenter ());
      if (sqdist < l->GetSquaredRadius ())
      {
        light_array->AddLight (l, sqdist);
      }
    }
  }

  if (light_array->num_lights <= max_num_lights)
  {
    // The number of lights that we found is smaller than what fits

    // in the array given us by the user. So we just copy them all

    // and don't need to sort.
    for (i = 0; i < light_array->num_lights; i++)
      lights[i] = light_array->GetLight (i);
    return light_array->num_lights;
  }
  else
  {
    // We found more lights than we can put in the given array

    // so we sort the lights and then return the nearest.
    qsort (
      light_array->array,
      light_array->num_lights,
      sizeof (LightAndDist),
      compare_light);
    for (i = 0; i < max_num_lights; i++)
      lights[i] = light_array->GetLight (i);
    return max_num_lights;
  }
}

iSectorIterator *csEngine::GetNearbySectors (
  iSector *sector,
  const csVector3 &pos,
  float radius)
{
  csSectorIt *it = new csSectorIt (sector->GetPrivateObject (), pos, radius);
  return it;
}

iObjectIterator *csEngine::GetNearbyObjects (
  iSector *sector,
  const csVector3 &pos,
  float radius)
{
  csObjectIt *it = new csObjectIt (
      this,
      sector->GetPrivateObject (),
      pos,
      radius);
  return it;
}

int csEngine::GetTextureFormat () const
{
  iTextureManager *txtmgr = G3D->GetTextureManager ();
  return txtmgr->GetTextureFormat ();
}

void csEngine::SelectRegion (const char *iName)
{
  if (iName == NULL)
  {
    region = NULL;          // Default engine region.
    return ;
  }

  region = regions.FindByName (iName);
  if (!region)
  {
    csRegion *r = new csRegion (this);
    region = &r->scfiRegion;
    r->SetName (iName);
    regions.Push (region);
    r->DecRef ();
  }
}

void csEngine::SelectRegion (iRegion *region)
{
  csEngine::region = region;
}

iRegion *csEngine::GetCurrentRegion () const
{
  return region;
}

void csEngine::AddToCurrentRegion (csObject *obj)
{
  if (region)
  {
    region->QueryObject ()->ObjAdd (obj);
  }
}

iTextureWrapper *csEngine::CreateTexture (
  const char *iName,
  const char *iFileName,
  csColor *iTransp,
  int iFlags)
{
  if (!ImageLoader) return NULL;

  // First of all, load the image file
  iDataBuffer *data = VFS->ReadFile (iFileName);
  if (!data || !data->GetSize ())
  {
    if (data) data->DecRef ();
    Warn ("Cannot read image file \"%s\" from VFS.", iFileName);
    return NULL;
  }

  iImage *ifile = ImageLoader->Load (
      data->GetUint8 (),
      data->GetSize (),
      CS_IMGFMT_TRUECOLOR); // GetTextureFormat ());
  data->DecRef ();

  if (!ifile)
  {
    Warn ("Unknown image file format: \"%s\".", iFileName);
    return NULL;
  }

  iDataBuffer *xname = VFS->ExpandPath (iFileName);
  ifile->SetName (**xname);
  xname->DecRef ();

  // Okay, now create the respective texture handle object
  iTextureWrapper *tex = GetTextures ()->FindByName (iName);
  if (tex)
    tex->SetImageFile (ifile);
  else
    tex = GetTextures ()->NewTexture (ifile);

  tex->SetFlags (iFlags);
  tex->QueryObject ()->SetName (iName);

  // dereference image pointer since tex already incremented it
  ifile->DecRef ();

  if (iTransp)
    tex->SetKeyColor (
        QInt (iTransp->red * 255.2),
        QInt (iTransp->green * 255.2),
        QInt (iTransp->blue * 255.2));

  return tex;
}

iMaterialWrapper *csEngine::CreateMaterial (
  const char *iName,
  iTextureWrapper *texture)
{
  csMaterial *mat = new csMaterial (texture);
  iMaterialWrapper *wrapper = materials->NewMaterial (mat);
  wrapper->QueryObject ()->SetName (iName);
  mat->DecRef ();
  return wrapper;
}

iMeshWrapper *csEngine::CreateSectorWallsMesh (
  csSector *sector,
  const char *iName)
{
  iMeshObjectType *thing_type = GetThingType ();
  iMeshObjectFactory *thing_fact = thing_type->NewFactory ();
  iMeshObject *thing_obj = SCF_QUERY_INTERFACE_FAST (thing_fact, iMeshObject);
  thing_fact->DecRef ();

  csMeshWrapper *thing_wrap = new csMeshWrapper (NULL, thing_obj);

  thing_obj->DecRef ();
  thing_wrap->SetName (iName);
  GetMeshes ()->Add (&(thing_wrap->scfiMeshWrapper));
  thing_wrap->GetMovable ().SetSector (&sector->scfiSector);
  thing_wrap->GetMovable ().UpdateMove ();
  thing_wrap->flags.Set (CS_ENTITY_CONVEX);
  thing_wrap->SetZBufMode (CS_ZBUF_FILL);
  thing_wrap->SetRenderPriority (GetWallRenderPriority ());

  return &(thing_wrap->scfiMeshWrapper);
}

iMeshWrapper *csEngine::CreateSectorWallsMesh (
  iSector *sector,
  const char *iName)
{
  return CreateSectorWallsMesh (sector->GetPrivateObject (), iName);
}

iSector *csEngine::CreateSector (const char *iName)
{
  iSector *sector = &(new csSector (this))->scfiSector;
  sector->QueryObject ()->SetName (iName);
  sectors.Push (sector);
  sector->DecRef ();

  return sector;
}

iMaterial *csEngine::CreateBaseMaterial (iTextureWrapper *txt)
{
  csMaterial *mat = new csMaterial ();
  if (txt) mat->SetTextureWrapper (txt);

  iMaterial *imat = SCF_QUERY_INTERFACE_FAST (mat, iMaterial);
  imat->DecRef ();
  return imat;
}

iMaterial *csEngine::CreateBaseMaterial (
  iTextureWrapper *txt,
  int num_layers,
  iTextureWrapper **wrappers,
  csTextureLayer *layers)
{
  csMaterial *mat = new csMaterial ();
  if (txt) mat->SetTextureWrapper (txt);

  int i;
  for (i = 0; i < num_layers; i++)
  {
    mat->AddTextureLayer (
        wrappers[i],
        layers[i].mode,
        layers[i].uscale,
        layers[i].vscale,
        layers[i].ushift,
        layers[i].vshift);
  }

  iMaterial *imat = SCF_QUERY_INTERFACE_FAST (mat, iMaterial);
  imat->DecRef ();
  return imat;
}

iTextureList *csEngine::GetTextureList () const
{
  return &(GetTextures ()->scfiTextureList);
}

iMaterialList *csEngine::GetMaterialList () const
{
  return &(GetMaterials ()->scfiMaterialList);
}

iRegionList *csEngine::GetRegions ()
{
  return &(regions.scfiRegionList);
}

iCamera *csEngine::CreateCamera ()
{
  return &(new csCamera ())->scfiCamera;
}

iStatLight *csEngine::CreateLight (
  const char *name,
  const csVector3 &pos,
  float radius,
  const csColor &color,
  bool pseudoDyn)
{
  csStatLight *light = new csStatLight (
      pos.x,
      pos.y,
      pos.z,
      radius,
      color.red,
      color.green,
      color.blue,
      pseudoDyn);
  if (name) light->SetName (name);

  iStatLight *il = SCF_QUERY_INTERFACE_FAST (light, iStatLight);
  il->DecRef ();
  return il;
}

iDynLight *csEngine::CreateDynLight (
  const csVector3 &pos,
  float radius,
  const csColor &color)
{
  csDynLight *light = new csDynLight (
      pos.x,
      pos.y,
      pos.z,
      radius,
      color.red,
      color.green,
      color.blue);
  AddDynLight (light);

  iDynLight *il = SCF_QUERY_INTERFACE_FAST (light, iDynLight);
  il->DecRef ();
  return il;
}

void csEngine::RemoveDynLight (iDynLight *light)
{
  RemoveDynLight (light->GetPrivateObject ());
}

iMeshFactoryWrapper *csEngine::CreateMeshFactory (
  const char *classId,
  const char *name)
{
  // WARNING! In the past this routine checked if the factory
  // was already created. This is wrong! This routine should not do
  // this and instead allow duplicate factories (with the same name).
  // That's because that duplicate factory can still be in another
  // region. And even if this is not the case then factories with same
  // name are still allowed.
  iPluginManager *plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  iMeshObjectType *type = CS_QUERY_PLUGIN_CLASS (
      plugin_mgr,
      classId,
      iMeshObjectType);
  if (!type) type = CS_LOAD_PLUGIN (plugin_mgr, classId, iMeshObjectType);
  plugin_mgr->DecRef ();
  if (!type) return NULL;

  iMeshObjectFactory *fact = type->NewFactory ();
  if (!fact) return NULL;

  // don't pass the name to avoid a second search
  iMeshFactoryWrapper *fwrap = CreateMeshFactory (fact, NULL);
  if (fwrap && name) fwrap->QueryObject ()->SetName (name);
  fact->DecRef ();
  type->DecRef ();
  return fwrap;
}

iMeshFactoryWrapper *csEngine::CreateMeshFactory (
  iMeshObjectFactory *fact,
  const char *name)
{
  // WARNING! In the past this routine checked if the factory
  // was already created. This is wrong! This routine should not do
  // this and instead allow duplicate factories (with the same name).
  // That's because that duplicate factory can still be in another
  // region. And even if this is not the case then factories with same
  // name are still allowed.
  csMeshFactoryWrapper *mfactwrap = new csMeshFactoryWrapper (fact);
  if (name) mfactwrap->SetName (name);
  GetMeshFactories ()->Add (&(mfactwrap->scfiMeshFactoryWrapper));
  return &mfactwrap->scfiMeshFactoryWrapper;
}

iMeshFactoryWrapper *csEngine::CreateMeshFactory (const char *name)
{
  // WARNING! In the past this routine checked if the factory
  // was already created. This is wrong! This routine should not do
  // this and instead allow duplicate factories (with the same name).
  // That's because that duplicate factory can still be in another
  // region. And even if this is not the case then factories with same
  // name are still allowed.
  csMeshFactoryWrapper *mfactwrap = new csMeshFactoryWrapper ();
  if (name) mfactwrap->SetName (name);
  GetMeshFactories ()->Add (&(mfactwrap->scfiMeshFactoryWrapper));
  return &mfactwrap->scfiMeshFactoryWrapper;
}

//------------------------------------------------------------------------

/*
 * Loader context class for the engine.
 */
class EngineLoaderContext : public iLoaderContext
{
private:
  iEngine* Engine;
  iRegion* region;

public:
  EngineLoaderContext (iEngine* Engine, iRegion* region);
  virtual ~EngineLoaderContext ();

  SCF_DECLARE_IBASE;

  virtual iSector* FindSector (const char* name);
  virtual iMaterialWrapper* FindMaterial (const char* name);
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name);
  virtual iMeshWrapper* FindMeshObject (const char* name);
};

SCF_IMPLEMENT_IBASE(EngineLoaderContext);
  SCF_IMPLEMENTS_INTERFACE(iLoaderContext);
SCF_IMPLEMENT_IBASE_END;

EngineLoaderContext::EngineLoaderContext (iEngine* Engine,
	iRegion* region)
{
  SCF_CONSTRUCT_IBASE (NULL);
  EngineLoaderContext::Engine = Engine;
  EngineLoaderContext::region = region;
}

EngineLoaderContext::~EngineLoaderContext ()
{
}

iSector* EngineLoaderContext::FindSector (const char* name)
{
  return Engine->FindSector (name, region);
}

iMaterialWrapper* EngineLoaderContext::FindMaterial (const char* name)
{
  return Engine->FindMaterial (name, region);
}

iMeshFactoryWrapper* EngineLoaderContext::FindMeshFactory (const char* name)
{
  return Engine->FindMeshFactory (name, region);
}

iMeshWrapper* EngineLoaderContext::FindMeshObject (const char* name)
{
  return Engine->FindMeshObject (name, region);
}

//------------------------------------------------------------------------

iLoaderContext* csEngine::CreateLoaderContext (iRegion* region)
{
  return new EngineLoaderContext (this, region);
}

//------------------------------------------------------------------------

iMeshFactoryWrapper *csEngine::LoadMeshFactory (
  const char *name,
  const char *loaderClassId,
  iDataBuffer *input)
{
  iPluginManager *plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  iLoaderPlugin *plug = CS_QUERY_PLUGIN_CLASS (
      plugin_mgr,
      loaderClassId,
      iLoaderPlugin);
  if (!plug)
    plug = CS_LOAD_PLUGIN (plugin_mgr, loaderClassId, iLoaderPlugin);
  plugin_mgr->DecRef ();
  if (!plug) return NULL;

  iMeshFactoryWrapper *fact = CreateMeshFactory (name);
  if (!fact) return NULL;

  char *buf = **input;
  iLoaderContext* elctxt = CreateLoaderContext ();
  iBase *mof = plug->Parse (
      buf, elctxt, fact->GetMeshObjectFactory ());
  elctxt->DecRef ();
  plug->DecRef ();
  if (!mof)
  {
    GetMeshFactories ()->Remove (fact);
    return NULL;
  }

  iMeshObjectFactory *mof2 = SCF_QUERY_INTERFACE (mof, iMeshObjectFactory);
  if (!mof2)
  {
    // @@@ ERROR?
    GetMeshFactories ()->Remove (fact);
    return NULL;
  }

  fact->SetMeshObjectFactory (mof2);
  mof2->SetLogicalParent (fact);
  mof2->DecRef ();
  mof->DecRef ();
  return fact;
}

iMeshWrapper *csEngine::LoadMeshWrapper (
  const char *name,
  const char *loaderClassId,
  iDataBuffer *input,
  iSector *sector,
  const csVector3 &pos)
{
  iPluginManager *plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  iLoaderPlugin *plug = CS_QUERY_PLUGIN_CLASS (
      plugin_mgr,
      loaderClassId,
      iLoaderPlugin);
  if (!plug)
    plug = CS_LOAD_PLUGIN (plugin_mgr, loaderClassId, iLoaderPlugin);
  plugin_mgr->DecRef ();
  if (!plug) return NULL;

  csMeshWrapper *meshwrap = new csMeshWrapper (NULL);
  if (name) meshwrap->SetName (name);

  iMeshWrapper *imw = &(meshwrap->scfiMeshWrapper);
  GetMeshes ()->Add (imw);
  if (sector)
  {
    meshwrap->GetMovable ().SetSector (sector);
    meshwrap->GetMovable ().SetPosition (pos);
    meshwrap->GetMovable ().UpdateMove ();
  }

  char *buf = **input;
  iLoaderContext* elctxt = CreateLoaderContext ();
  iBase *mof = plug->Parse (
      buf, elctxt, imw);
  elctxt->DecRef ();
  plug->DecRef ();
  if (!mof)
  {
    GetMeshes ()->Remove (imw);
    meshwrap->DecRef ();
    return NULL;
  }

  meshwrap->SetMeshObject ((iMeshObject *)mof);
  mof->DecRef ();
  return imw;
}

iMeshWrapper *csEngine::CreateMeshWrapper (
  iMeshFactoryWrapper *factory,
  const char *name,
  iSector *sector,
  const csVector3 &pos)
{
  iMeshWrapper *mesh = factory->CreateMeshWrapper ();
  if (name) mesh->QueryObject ()->SetName (name);
  GetMeshes ()->Add (mesh);
  if (sector)
  {
    mesh->GetMovable ()->SetSector (sector);
    mesh->GetMovable ()->SetPosition (pos);
    mesh->GetMovable ()->UpdateMove ();
  }

  return mesh;
}

iMeshWrapper *csEngine::CreateMeshWrapper (
  iMeshObject *mesh,
  const char *name,
  iSector *sector,
  const csVector3 &pos)
{
  csMeshWrapper *meshwrap = new csMeshWrapper (NULL, mesh);
  if (name) meshwrap->SetName (name);
  GetMeshes ()->Add (&(meshwrap->scfiMeshWrapper));
  if (sector)
  {
    meshwrap->GetMovable ().SetSector (sector);
    meshwrap->GetMovable ().SetPosition (pos);
    meshwrap->GetMovable ().UpdateMove ();
  }

  return &meshwrap->scfiMeshWrapper;
}

iMeshWrapper *csEngine::CreateMeshWrapper (const char *name)
{
  csMeshWrapper *meshwrap = new csMeshWrapper (NULL);
  if (name) meshwrap->SetName (name);
  GetMeshes ()->Add (&(meshwrap->scfiMeshWrapper));
  return &meshwrap->scfiMeshWrapper;
}

bool csEngine::RemoveObject (iBase *object)
{
  {
    iSector *sector = SCF_QUERY_INTERFACE_FAST (object, iSector);
    if (sector)
    {
      if (region)
        region->QueryObject ()->ObjRemove (sector->QueryObject ());
      sectors.scfiSectorList.Remove (sector);
      sector->DecRef ();
      return true;
    }
  }
  {
    iCameraPosition *cp = SCF_QUERY_INTERFACE_FAST (object, iCameraPosition);
    if (cp)
    {
      if (region) region->QueryObject ()->ObjRemove (cp->QueryObject ());
      camera_positions.scfiCameraPositionList.Remove (cp);
      cp->DecRef ();
      return true;
    }
  }
  {
    iDynLight *dl = SCF_QUERY_INTERFACE_FAST (object, iDynLight);
    if (dl)
    {
      if (region) region->QueryObject ()->ObjRemove (dl->QueryObject ());
      RemoveDynLight (dl);
      dl->DecRef ();
      return true;
    }
  }
  {
    iCollection *col = SCF_QUERY_INTERFACE_FAST (object, iCollection);
    if (col)
    {
      if (region) region->QueryObject ()->ObjRemove (col->QueryObject ());
      collections.scfiCollectionList.Remove (col);
      col->DecRef ();
      return true;
    }
  }
  {
    iTextureWrapper *txt = SCF_QUERY_INTERFACE_FAST (object, iTextureWrapper);
    if (txt)
    {
      if (region) region->QueryObject ()->ObjRemove (txt->QueryObject ());
      GetTextureList ()->Remove (txt);
      txt->DecRef ();
      return true;
    }
  }
  {
    iMaterialWrapper *mat = SCF_QUERY_INTERFACE_FAST (
        object,
        iMaterialWrapper);
    if (mat)
    {
      if (region) region->QueryObject ()->ObjRemove (mat->QueryObject ());
      GetMaterialList ()->Remove (mat);
      mat->DecRef ();
      return true;
    }
  }
  {
    iMeshFactoryWrapper *factwrap = SCF_QUERY_INTERFACE_FAST (
        object,
        iMeshFactoryWrapper);
    if (factwrap)
    {
      if (region)
        region->QueryObject ()->ObjRemove (factwrap->QueryObject ());
      mesh_factories.scfiMeshFactoryList.Remove (factwrap);
      factwrap->DecRef ();
      return true;
    }
  }
  {
    iMeshWrapper *meshwrap = SCF_QUERY_INTERFACE_FAST (object, iMeshWrapper);
    if (meshwrap)
    {
      if (region)
        region->QueryObject ()->ObjRemove (meshwrap->QueryObject ());
      meshes.scfiMeshList.Remove (meshwrap);
      meshwrap->DecRef ();
      return true;
    }
  }
  {
    iPolyTxtPlane *ptp = SCF_QUERY_INTERFACE_FAST (object, iPolyTxtPlane);
    if (ptp)
    {
      if (region) region->QueryObject ()->ObjRemove (ptp->QueryObject ());

      iThingEnvironment *te = SCF_QUERY_INTERFACE (
          GetThingType (),
          iThingEnvironment);
      te->RemovePolyTxtPlane (ptp);
      te->DecRef ();
      ptp->DecRef ();
      return true;
    }
  }
  {
    iCurveTemplate *ct = SCF_QUERY_INTERFACE_FAST (object, iCurveTemplate);
    if (ct)
    {
      if (region) region->QueryObject ()->ObjRemove (ct->QueryObject ());

      iThingEnvironment *te = SCF_QUERY_INTERFACE (
          GetThingType (),
          iThingEnvironment);
      te->RemoveCurveTemplate (ct);
      te->DecRef ();
      ct->DecRef ();
      return true;
    }
  }

  return false;
}

//----------------Begin-Multi-Context-Support------------------------------
void csEngine::Resize ()
{
  frame_width = G3D->GetWidth ();
  frame_height = G3D->GetHeight ();

  // Reset the culler.
  InitCuller ();
}

csEngine::csEngineState::csEngineState (csEngine *e)
{
  engine = e;
  c_buffer = e->c_buffer;
  cbufcube = e->cbufcube;
  G2D = e->G2D;
  G3D = e->G3D;
  resize = false;
}

csEngine::csEngineState::~csEngineState ()
{
  if (engine->G2D == G2D)
  {
    engine->G3D->DecRef ();
    engine->G3D = NULL;
    engine->G2D = NULL;
    engine->c_buffer = NULL;
    engine->cbufcube = NULL;
  }

  delete c_buffer;
  delete cbufcube;
}

void csEngine::csEngineState::Activate ()
{
  engine->c_buffer = c_buffer;
  engine->cbufcube = cbufcube;
  engine->frame_width = G3D->GetWidth ();
  engine->frame_height = G3D->GetHeight ();

  if (resize)
  {
    engine->Resize ();

    c_buffer = engine->c_buffer;
    cbufcube = engine->cbufcube;
    resize = false;
  }
}

void csEngine::csEngineStateVector::Close (iGraphics2D *g2d)
{
  // Hack-- with the glide back buffer implementations of procedural textures
  // circumstances are that many G3D can be associated with one G2D.
  // It is impossible to tell which is which so destroy them all, and rely on
  // regeneration the next time the context is set to the surviving G3Ds associated
  // with this G2D
  int i;
  for (i = 0; i < Length (); i++)
    if (((csEngineState *)root[i])->G2D == g2d)
    {
      //Delete (i);
      break;
    }
}

void csEngine::csEngineStateVector::Resize (iGraphics2D *g2d)
{
  // With the glide back buffer implementation of procedural textures
  // circumstances are that many G3D can be associated with one G2D, so
  // we test for width and height also.
  int i;
  for (i = 0; i < Length (); i++)
  {
    csEngineState *state = (csEngineState *)root[i];
    if (state->G2D == g2d)
    {
      if (
        (state->G2D->GetWidth () == state->G3D->GetWidth ()) &&
        (state->G2D->GetHeight () == state->G3D->GetHeight ()))
        ((csEngineState *)root[i])->resize = true;
    }
  }
}

void csEngine::SetContext (iGraphics3D *g3d)
{
  G2D = g3d->GetDriver2D ();
  if (g3d != G3D)
  {
    if (G3D) G3D->DecRef ();
    G3D = g3d;
    if (!engine_states)
    {
      engine_states = new csEngineStateVector ();
      Resize ();
      engine_states->Push (new csEngineState (this));
    }
    else
    {
      int idg3d = engine_states->FindKey (g3d);
      if (idg3d < 0)
      {
        // Null out the culler which belongs to another state so its not deleted.
        c_buffer = NULL;
        cbufcube = NULL;
        frame_width = G3D->GetWidth ();
        frame_height = G3D->GetHeight ();
        cbufcube = new csCBufferCube (1024);
        InitCuller ();
        engine_states->Push (new csEngineState (this));
      }
      else
      {
        csEngineState *state = (csEngineState *)engine_states->Get (idg3d);
        state->Activate ();
      }
    }

    G3D->IncRef ();
  }
}

iGraphics3D *csEngine::GetContext () const
{
  return G3D;
}

iClipper2D *csEngine::GetTopLevelClipper () const
{
  return top_clipper;
}

void csEngine::SetAmbientLight (const csColor &c)
{
  csLight::ambient_red = int (c.red * 255);
  csLight::ambient_green = int (c.green * 255);
  csLight::ambient_blue = int (c.blue * 255.0);
}

void csEngine::GetAmbientLight (csColor &c) const
{
  c.red = csLight::ambient_red / 255.0f;
  c.green = csLight::ambient_green / 255.0f;
  c.blue = csLight::ambient_blue / 255.0f;
}

//-------------------End-Multi-Context-Support--------------------------------
