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

#include "csgeom/kdtree.h"
#include "csgeom/math.h"
#include "csgeom/polyclip.h"
#include "csgeom/sphere.h"
#include "csgfx/memimage.h"
#include "csqint.h"
#include "csutil/cfgacc.h"
#include "csutil/databuf.h"
#include "csutil/debug.h"
#include "csutil/scf.h"
#include "csutil/scfstrset.h"
#include "csutil/sysfunc.h"
#include "csutil/util.h"
#include "csutil/vfscache.h"
#include "csutil/xmltiny.h"
#include "csutil/event.h"
#include "iengine/portal.h"
#include "iengine/rendersteps/igeneric.h"
#include "iengine/rendersteps/irenderstep.h"
#include "iengine/rendersteps/irsfact.h"
#include "igeom/clip2d.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "imap/ldrctxt.h"
#include "imap/reader.h"
#include "imesh/lighting.h"
#include "imesh/thing.h"
#include "iutil/cfgmgr.h"
#include "iutil/comp.h"
#include "iutil/databuff.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/verbositymanager.h"
#include "iutil/vfs.h"
#include "iutil/virtclk.h"
#include "ivaria/engseq.h"
#include "ivaria/pmeter.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/halo.h"
#include "ivideo/txtmgr.h"
#include "plugins/engine/3d/camera.h"
#include "plugins/engine/3d/campos.h"
#include "plugins/engine/3d/cscoll.h"
#include "plugins/engine/3d/engine.h"
#include "plugins/engine/3d/halo.h"
#include "plugins/engine/3d/light.h"
#include "plugins/engine/3d/lightmgr.h"
#include "plugins/engine/3d/lview.h"
#include "plugins/engine/3d/material.h"
#include "plugins/engine/3d/meshobj.h"
#include "plugins/engine/3d/objwatch.h"
#include "plugins/engine/3d/portalcontainer.h"
#include "plugins/engine/3d/region.h"
#include "plugins/engine/3d/sector.h"
#include "plugins/engine/3d/texture.h"

CS_IMPLEMENT_PLUGIN

bool csEngine::doVerbose = false;

//---------------------------------------------------------------------------
void csEngine::Report (const char *description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (!reporter) reporter = CS_QUERY_REGISTRY (objectRegistry, iReporter);
  
  if (reporter)
  {
    reporter->ReportV (
        CS_REPORTER_SEVERITY_NOTIFY,
        "crystalspace.engine.notify",
        description,
        arg);
  }
  else
  {
    csPrintfV (description, arg);
    csPrintf ("\n");
    fflush (stdout);
  }

  va_end (arg);
}

void csEngine::Error (const char *description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (!reporter) reporter = CS_QUERY_REGISTRY (objectRegistry, iReporter);

  if (reporter)
  {
    reporter->ReportV (
        CS_REPORTER_SEVERITY_ERROR,
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

void csEngine::Warn (const char *description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (!reporter) reporter = CS_QUERY_REGISTRY (objectRegistry, iReporter);

  if (reporter)
  {
    reporter->ReportV (
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

  if (!reporter) reporter = CS_QUERY_REGISTRY (objectRegistry, iReporter);

  if (reporter)
  {
    reporter->ReportV (
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
csCameraPositionList::csCameraPositionList ()
  : scfImplementationType (this)
{
}

csCameraPositionList::~csCameraPositionList ()
{
}

iCameraPosition *csCameraPositionList::NewCameraPosition (const char *name)
{
  csVector3 v (0);
  csCameraPosition *newcp = new csCameraPosition (name, "", v, v, v);
  iCameraPosition *cp = &(newcp->scfiCameraPosition);
  positions.Push (cp);
  cp->DecRef ();
  return cp;
}

int csCameraPositionList::GetCount () const
{
  return (int)positions.Length ();
}

iCameraPosition *csCameraPositionList::Get (int n) const
{
  return positions.Get (n);
}

int csCameraPositionList::Add (iCameraPosition *obj)
{
  return (int)positions.Push (obj);
}

bool csCameraPositionList::Remove (iCameraPosition *obj)
{
  return positions.Delete (obj);
}

bool csCameraPositionList::Remove (int n)
{
  return positions.DeleteIndex (n);
}

void csCameraPositionList::RemoveAll ()
{
  positions.DeleteAll ();
}

int csCameraPositionList::Find (
  iCameraPosition *obj) const
{
  return (int)positions.Find (obj);
}

iCameraPosition *csCameraPositionList::FindByName (
  const char *Name) const
{
  return positions.FindByName (Name);
}

//---------------------------------------------------------------------------
csCollectionList::csCollectionList ()
  : scfImplementationType (this)
{
}

csCollectionList::~csCollectionList ()
{
}

iCollection *csCollectionList::NewCollection (const char *name)
{
  csCollection *c = new csCollection (csEngine::currentEngine);
  c->SetName (name);
  collections.Push (&(c->scfiCollection));
  c->DecRef ();
  return &(c->scfiCollection);
}

int csCollectionList::GetCount () const
{
  return (int)collections.Length ();
}

iCollection *csCollectionList::Get (int n) const
{
  return collections.Get (n);
}

int csCollectionList::Add (iCollection *obj)
{
  return (int)collections.Push (obj);
}

bool csCollectionList::Remove (iCollection *obj)
{
  return collections.Delete (obj);
}

bool csCollectionList::Remove (int n)
{
  return collections.DeleteIndex (n);
}

void csCollectionList::RemoveAll ()
{
  collections.DeleteAll ();
}

int csCollectionList::Find (iCollection *obj) const
{
  return (int)collections.Find (obj);
}

iCollection *csCollectionList::FindByName (
  const char *Name) const
{
  return collections.FindByName (Name);
}

//---------------------------------------------------------------------------

void csEngineMeshList::FreeMesh (iMeshWrapper* mesh)
{
  mesh->GetMovable ()->ClearSectors ();
  iMeshWrapper* p = mesh->GetParentContainer ();
  if (p) p->GetChildren ()->Remove (mesh);
  csMeshList::FreeMesh (mesh);
}

//---------------------------------------------------------------------------

/**
 * Iterator to iterate over sectors in the engine which are within
 * a radius from a given point.
 * This iterator assumes there are no fundamental changes
 * in the engine while it is being used.
 * If changes to the engine happen the results are unpredictable.
 */
class csSectorIt : public iSectorIterator
{
private:
  csEngine* engine;
  // The position and radius.
  iSector *sector;
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

  // Current sector.
  iSector* current_sector;

  /// Get sector from iterator. Return 0 at end.
  iSector *FetchNext ();

public:
  csSectorIt* nextPooledIt;

  /// Construct an iterator and initialize to start.
  csSectorIt (csEngine* engine);
  void Init (iSector *sector, const csVector3 &pos, float radius);

  /// Destructor.
  virtual ~csSectorIt ();

  SCF_DECLARE_IBASE;

  /// Restart iterator.
  virtual void Reset ();

  /// Return true if there are more elements.
  virtual bool HasNext ();

  /// Get sector from iterator. Return 0 at end.
  virtual iSector *Next ();

  /**
   * Get last position that was used from Fetch. This can be
   * different from 'pos' because of space warping.
   */
  virtual const csVector3 &GetLastPosition () { return last_pos; }
};

/**
 * Iterator to iterate over meshes in the given list.
 */
class csMeshListIt : public iMeshWrapperIterator
{
  friend class csEngine;
private:
  csArray<iMeshWrapper*>* list;
  int num_objects;

  // Current index.
  int cur_idx;

private:
  /// Construct an iterator and initialize to start.
  csMeshListIt (csArray<iMeshWrapper*>* list);

public:
  /// Destructor.
  virtual ~csMeshListIt ();

  SCF_DECLARE_IBASE;

  virtual void Reset ();
  virtual iMeshWrapper* Next ();
  virtual bool HasNext () const;
};

/**
 * Iterator to iterate over objects in the given list.
 */
class csObjectListIt : public iObjectIterator
{
  friend class csEngine;
private:
  csArray<iObject*>* list;
  int num_objects;

  // Current index.
  int cur_idx;

private:
  /// Construct an iterator and initialize to start.
  csObjectListIt (csArray<iObject*>* list);

public:
  /// Destructor.
  virtual ~csObjectListIt ();

  SCF_DECLARE_IBASE;

  virtual void Reset ();
  virtual iObject* Next ();
  virtual bool HasNext () const;
  virtual iObject *GetParentObj () const
  {
    return 0;
  }

  virtual iObject* FindName (const char *name)  { (void)name; return 0; }
};

//---------------------------------------------------------------------------

csLightIt::csLightIt (csEngine *e, iRegion *r) :
  scfImplementationType (this),
  engine(e),
  region(r)
{
  Reset ();
}

csLightIt::~csLightIt ()
{
}

bool csLightIt::NextSector ()
{
  sectorIndex++;
  if (region)
    while ( sectorIndex < engine->sectors.GetCount () &&
      	!region->IsInRegion (GetLastSector ()->QueryObject ()))
      sectorIndex++;
  if (sectorIndex >= engine->sectors.GetCount ()) return false;
  return true;
}

void csLightIt::Reset ()
{
  sectorIndex = -1;
  lightIndex = 0;
  currentLight = 0;
}

iLight *csLightIt::FetchNext ()
{
  iSector *sector;
  if (sectorIndex == -1)
  {
    if (!NextSector ()) return 0;
    lightIndex = -1;
  }

  if (sectorIndex >= engine->sectors.GetCount ()) return 0;
  sector = engine->sectors.Get (sectorIndex);

  // Try next light.
  lightIndex++;

  if (lightIndex >= sector->GetLights ()->GetCount ())
  {
    // Go to next sector.
    lightIndex = -1;
    if (!NextSector ()) return 0;

    // Initialize iterator to start of sector and recurse.
    return FetchNext ();
  }

  return sector->GetLights ()->Get (lightIndex);
}

bool csLightIt::HasNext ()
{
  if (currentLight != 0) return true;
  currentLight = FetchNext ();
  return currentLight != 0;
}

iLight *csLightIt::Next ()
{
  if (currentLight == 0)
  {
    currentLight = FetchNext ();
  }

  iLight* l = currentLight;
  currentLight = 0;
  return l;
}

iSector *csLightIt::GetLastSector ()
{
  return engine->sectors.Get (sectorIndex);
}

//---------------------------------------------------------------------------
csSectorIt* csEngine::AllocSectorIterator (iSector *sector, 
					   const csVector3 &pos, 
					   float radius)
{
  csSectorIt* newIt;
  if (sectorItPool)
  {
    newIt = sectorItPool;
    sectorItPool = sectorItPool->nextPooledIt;
  }
  else
    newIt = new csSectorIt (this);
  newIt->Init (sector, pos, radius);
  return newIt;
}
 
void csEngine::RecycleSectorIterator (csSectorIt* iterator)
{
  if (iterator == 0) return;
  iterator->nextPooledIt = sectorItPool;
  sectorItPool = iterator;
}

void csEngine::FreeSectorIteratorPool ()
{
  while (sectorItPool)
  {
    csSectorIt* toDelete = sectorItPool;
    sectorItPool = sectorItPool->nextPooledIt;
    delete toDelete;
  }
}

void csSectorIt::IncRef ()
{
  scfRefCount++;
}

void csSectorIt::DecRef ()
{
  if (scfRefCount == 1)
  {
    engine->RecycleSectorIterator (this);
    return;
  }
  scfRefCount--;
}

SCF_IMPLEMENT_IBASE_GETREFCOUNT(csSectorIt)
SCF_IMPLEMENT_IBASE_REFOWNER(csSectorIt)
SCF_IMPLEMENT_IBASE_REMOVE_REF_OWNERS(csSectorIt)
SCF_IMPLEMENT_IBASE_QUERY(csSectorIt)
  SCF_IMPLEMENTS_INTERFACE(iSectorIterator)
SCF_IMPLEMENT_IBASE_END

csSectorIt::csSectorIt (csEngine* engine)
{
  SCF_CONSTRUCT_IBASE (0);
  csSectorIt::engine = engine;
  nextPooledIt = 0;
}

void csSectorIt::Init (iSector *sector, const csVector3 &pos,
		       float radius)
{
  scfRefCount = 1;
  csSectorIt::sector = sector;
  csSectorIt::pos = pos;
  csSectorIt::radius = radius;
  recursive_it = 0;

  Reset ();
}

csSectorIt::~csSectorIt ()
{
  engine->RecycleSectorIterator (recursive_it);
  SCF_DESTRUCT_IBASE ();
}

void csSectorIt::Reset ()
{
  cur_poly = -1;
  engine->RecycleSectorIterator (recursive_it);
  recursive_it = 0;
  has_ended = false;
  current_sector = 0;
}

iSector *csSectorIt::FetchNext ()
{
  if (has_ended) return 0;

  if (recursive_it)
  {
    iSector *rc = recursive_it->FetchNext ();
    if (rc)
    {
      last_pos = recursive_it->GetLastPosition ();
      return rc;
    }

    engine->RecycleSectorIterator (recursive_it);
    recursive_it = 0;
  }

  if (cur_poly == -1)
  {
    cur_poly = 0;
    last_pos = pos;
    return sector;
  }

  // End search.
  has_ended = true;
  return 0;
}

bool csSectorIt::HasNext ()
{
  if (current_sector != 0) return true;
  current_sector = FetchNext ();
  return current_sector != 0;
}

iSector *csSectorIt::Next ()
{
  if (current_sector == 0)
  {
    current_sector = FetchNext ();
  }

  iSector* s = current_sector;
  current_sector = 0;
  return s;
}


//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csObjectListIt)
  SCF_IMPLEMENTS_INTERFACE(iObjectIterator)
SCF_IMPLEMENT_IBASE_END

csObjectListIt::csObjectListIt (csArray<iObject*>* list)
{
  SCF_CONSTRUCT_IBASE (0);
  csObjectListIt::list = list;
  num_objects = (int)list->Length ();
  Reset ();
}

csObjectListIt::~csObjectListIt ()
{
  delete list;
  SCF_DESTRUCT_IBASE ();
}

void csObjectListIt::Reset ()
{
  cur_idx = 0;
}

iObject* csObjectListIt::Next ()
{
  if (cur_idx >= num_objects) return 0;
  cur_idx++;
  return (*list)[cur_idx-1];
}

bool csObjectListIt::HasNext () const
{
  return cur_idx < num_objects;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csMeshListIt)
  SCF_IMPLEMENTS_INTERFACE(iMeshWrapperIterator)
SCF_IMPLEMENT_IBASE_END

csMeshListIt::csMeshListIt (csArray<iMeshWrapper*>* list)
{
  SCF_CONSTRUCT_IBASE (0);
  csMeshListIt::list = list;
  num_objects = (int)list->Length ();
  Reset ();
}

csMeshListIt::~csMeshListIt ()
{
  delete list;
  SCF_DESTRUCT_IBASE ();
}

void csMeshListIt::Reset ()
{
  cur_idx = 0;
}

iMeshWrapper* csMeshListIt::Next ()
{
  if (cur_idx >= num_objects) return 0;
  cur_idx++;
  return (*list)[cur_idx-1];
}

bool csMeshListIt::HasNext () const
{
  return cur_idx < num_objects;
}

//---------------------------------------------------------------------------
int csEngine::frameWidth;
int csEngine::frameHeight;
iObjectRegistry *csEngine::objectRegistry = 0;
csEngine *csEngine::currentEngine = 0;
iEngine *csEngine::currentiEngine = 0;
int csEngine::lightmapCacheMode = CS_ENGINE_CACHE_READ | CS_ENGINE_CACHE_NOUPDATE;
int csEngine::maxLightmapWidth = 0;
int csEngine::maxLightmapHeight = 0;

SCF_IMPLEMENT_FACTORY (csEngine)

csEngine::csEngine (iBase *iParent) :
  scfImplementationType (this, iParent),
  textures (new csTextureList), materials (new csMaterialList),
  sharedVariables (new csSharedVariableList),
  sectorItPool (0), renderLoopManager (0), topLevelClipper (0), resize (false),
  worldSaveable (false), maxAspectRatio (0), nextframePending (0),
  currentFrameNumber (0), clearZBuf (false), defaultClearZBuf (false),
  clearScreen (false), defaultClearScreen (false), 
  defaultMaxLightmapWidth (256), defaultMaxLightmapHeight (256),
  currentRenderContext (0)
{
  DG_TYPE (this, "csEngine");

  scfiEventHandler = new eiEventHandler (this);

  currentEngine = this;
  currentiEngine = (iEngine*)this;
  objectRegistry = 0;

  ClearRenderPriorities ();
}

csEngine::~csEngine ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (objectRegistry, iEventQueue));
    if (q != 0)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }

  DeleteAll ();

  renderPriorities.DeleteAll ();

  delete materials;
  delete textures;
  delete sharedVariables;
  delete renderLoopManager;
  FreeSectorIteratorPool ();

}

bool csEngine::Initialize (iObjectRegistry *objectRegistry)
{
  csEngine::objectRegistry = objectRegistry;

  virtualClock = CS_QUERY_REGISTRY (objectRegistry, iVirtualClock);
  if (!virtualClock) return false;

  G3D = CS_QUERY_REGISTRY (objectRegistry, iGraphics3D);
  if (!G3D)
  {
    // If there is no G3D then we still allow initialization of the
    // engine because it might be useful to use the engine stand-alone
    // (i.e. for calculating lighting and so on).
    Warn ("No 3D driver!");
  }

  csRef<iVerbosityManager> verbosemgr (
    CS_QUERY_REGISTRY (objectRegistry, iVerbosityManager));
  if (verbosemgr) 
    doVerbose = verbosemgr->Enabled ("engine");
  if (doVerbose)
  {
    bugplug = CS_QUERY_REGISTRY (objectRegistry, iBugPlug);
  }
  else
  {
    bugplug = 0;
  }

  VFS = CS_QUERY_REGISTRY (objectRegistry, iVFS);
  if (!VFS) return false;

  if (G3D)
    G2D = G3D->GetDriver2D ();
  else
    G2D = 0;

  // don't check for failure; the engine can work without the image loader
  imageLoader = CS_QUERY_REGISTRY (objectRegistry, iImageIO);
  if (!imageLoader) Warn ("No image loader. Loading images will fail.");

  // reporter is optional.
  reporter = CS_QUERY_REGISTRY (objectRegistry, iReporter);

  // Tell event queue that we want to handle broadcast events
  csRef<iEventQueue> q = CS_QUERY_REGISTRY (objectRegistry, iEventQueue);
  if (q)
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast);

  csConfigAccess cfg (objectRegistry, "/config/engine.cfg");
  ReadConfig (cfg);

  // Set up the RL manager.
  renderLoopManager = new csRenderLoopManager (this);

  csLightManager* light_mgr = new csLightManager ();
  objectRegistry->Register (light_mgr, "iLightManager");
  light_mgr->DecRef ();

  return true;
}

// Handle some system-driver broadcasts
bool csEngine::HandleEvent (iEvent &Event)
{
  if (Event.Type == csevBroadcast)
  {
    switch (csCommandEventHelper::GetCode(&Event))
    {
      case cscmdSystemOpen:
        {
          if (G3D)
          {
	    globalStringSet = CS_QUERY_REGISTRY_TAG_INTERFACE (
	      objectRegistry, "crystalspace.shared.stringset", iStringSet);

            maxAspectRatio = 4096;
	    shaderManager = CS_QUERY_REGISTRY(objectRegistry, iShaderManager);
	    if (!shaderManager)
	    {
	      csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (
	      	objectRegistry, iPluginManager);
	      shaderManager = csPtr<iShaderManager> (
	      	CS_LOAD_PLUGIN(plugin_mgr,
			"crystalspace.graphics3d.shadermanager",
			iShaderManager));
	      objectRegistry->Register (shaderManager, "iShaderManager");
	    }

            csRef<iShaderCompiler> shcom (shaderManager->
              GetCompiler ("XMLShader"));

	    if (!shcom.IsValid())
	    {
	      Warn ("'XMLShader' shader compiler not available - "
		"default shaders are unavailable.");
	    }
	    else
	    {
	      // Load default shaders
	      csRef<iDocumentSystem> docsys (
		CS_QUERY_REGISTRY(objectRegistry, iDocumentSystem));
	      if (!docsys.IsValid())
		docsys.AttachNew (new csTinyDocumentSystem ());
	      csRef<iDocument> shaderDoc = docsys->CreateDocument ();

	      VFS->PushDir();
	      VFS->ChDir ("/shader/");
	      char const* shaderPath = "std_lighting.xml";
	      csRef<iFile> shaderFile = VFS->Open (shaderPath, VFS_FILE_READ);
	      if (shaderFile.IsValid())
	      {
		shaderDoc->Parse (shaderFile, true);
		defaultShader = shcom->CompileShader (shaderDoc->GetRoot ()->
		  GetNode ("shader"));
		shaderManager->RegisterShader (defaultShader);
	      }
	      else
		Warn ("Shader %s not available", shaderPath);

	      shaderDoc = docsys->CreateDocument ();
	      shaderPath = "std_lighting_portal.xml";
	      shaderFile = VFS->Open (shaderPath, VFS_FILE_READ);
	      if (shaderFile.IsValid())
	      {
		shaderDoc->Parse (shaderFile, true);
		csRef<iShader> portal_shader = shcom->CompileShader (
		  shaderDoc->GetRoot ()->GetNode ("shader"));
		shaderManager->RegisterShader (portal_shader);
	      }
	      else
		Warn ("Shader %s not available", shaderPath);
	      VFS->PopDir();
	    }

            csConfigAccess cfg (objectRegistry, "/config/engine.cfg");
	    // Now, try to load the user-specified default render loop.
	    const char* configLoop = cfg->GetStr ("Engine.RenderLoop.Default", 
	      0);
	    if (!configLoop)
	    {
	      defaultRenderLoop = CreateDefaultRenderLoop ();
	    }
	    else
	    {
	      defaultRenderLoop = renderLoopManager->Load (configLoop);
	      if (!defaultRenderLoop)
		return false;
	    }

	    // Register it.
	    renderLoopManager->Register (CS_DEFAULT_RENDERLOOP_NAME, 
	      defaultRenderLoop);

            frameWidth = G3D->GetWidth ();
            frameHeight = G3D->GetHeight ();
          }
          else
          {
            maxAspectRatio = 4096;
            frameWidth = 640;
            frameHeight = 480;
          }

          if (csCamera::GetDefaultFOV () == 0)
            csCamera::SetDefaultFOV (frameHeight, frameWidth);

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
          resize = true;
          return false;
        }

      case cscmdContextClose:
        {
          return false;
        }
    } /* endswitch */
  }

  return false;
}

iMeshObjectType* csEngine::GetThingType ()
{
  if (!thingMeshType)
  {
    csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (objectRegistry,
  	  iPluginManager);
    thingMeshType = CS_QUERY_PLUGIN_CLASS (
        plugin_mgr, "crystalspace.mesh.object.thing",
        iMeshObjectType);
    if (!thingMeshType)
    {
      thingMeshType = CS_LOAD_PLUGIN (plugin_mgr,
    	  "crystalspace.mesh.object.thing", iMeshObjectType);
    }
  }

  return (iMeshObjectType*)thingMeshType;
}

void csEngine::DeleteAll ()
{
  // First notify all sector removal callbacks.
  int i;
  for (i = 0 ; i < sectors.GetCount () ; i++)
    FireRemoveSector (sectors.Get (i));

  nextframePending = 0;
  halos.DeleteAll ();
  collections.RemoveAll ();

  GetMeshes ()->RemoveAll ();
  meshFactories.RemoveAll ();
  sectors.RemoveAll ();
  cameraPositions.RemoveAll ();

  delete materials;
  materials = new csMaterialList ();
  delete textures;
  textures = new csTextureList ();
  delete sharedVariables;
  sharedVariables = new csSharedVariableList();

  if (thingMeshType != 0)
  {
    csRef<iThingEnvironment> te (
  	SCF_QUERY_INTERFACE (thingMeshType, iThingEnvironment));
    CS_ASSERT (((iThingEnvironment*)te) != 0);
    te->Clear ();
  }

  currentRenderContext = 0;

  // Clear all regions.
  regions.DeleteAll ();

  // Clear all render priorities.
  ClearRenderPriorities ();

  // remove objects
  QueryObject ()->ObjRemoveAll ();
}

iObject *csEngine::QueryObject ()
{
  return (iObject*)this;
}

void csEngine::RegisterRenderPriority (
  const char *name,
  long priority,
  csRenderPrioritySorting rendsort)
{
  int i;

  // If our priority goes over the number of defined priorities
  // then we have to initialize.
  size_t old_pri_len = renderPriorities.Length ();
  if ((size_t)(priority + 1) >= renderPrioritySortflag.Length ())
  {
    renderPrioritySortflag.SetLength (priority + 2);
    renderPriorities.SetLength (priority+2);
  }
  for (i = (int)old_pri_len; i <= priority; i++)
  {
    renderPrioritySortflag[i] = CS_RENDPRI_SORT_NONE;
  }

  renderPriorities.Put (priority, (char*)name);
  renderPrioritySortflag[priority] = rendsort;
  renderPrioritiesDirty = true;
}

void csEngine::UpdateStandardRenderPriorities ()
{
  if (!renderPrioritiesDirty)
    return;
  renderPrioritiesDirty = false;

  renderPrioritySky = GetRenderPriority ("sky");
  renderPriorityPortal = GetRenderPriority ("portal");
  renderPriorityWall = GetRenderPriority ("wall");
  renderPriorityObject = GetRenderPriority ("object");
  renderPriorityAlpha = GetRenderPriority ("alpha");
}

long csEngine::GetRenderPriority (const char *name) const
{
  size_t i;
  for (i = 0; i < renderPriorities.Length (); i++)
  {
    const char *n = renderPriorities[i];
    if (n && !strcmp (name, n)) return (long)i;
  }

  return 0;
}

csRenderPrioritySorting csEngine::GetRenderPrioritySorting (const char *name) const
{
  size_t i;
  for (i = 0; i < renderPriorities.Length (); i++)
  {
    const char *n = renderPriorities[i];
    if (n && !strcmp (name, n)) return renderPrioritySortflag[i];
  }

  return CS_RENDPRI_SORT_NONE;
}

csRenderPrioritySorting csEngine::GetRenderPrioritySorting (long priority) const
{
  return renderPrioritySortflag[priority];
}

void csEngine::ClearRenderPriorities ()
{
  renderPrioritiesDirty = true;
  renderPriorities.DeleteAll ();
  renderPrioritySortflag.SetLength (0);
  RegisterRenderPriority ("sky", 2);
  RegisterRenderPriority ("portal", 3);
  RegisterRenderPriority ("wall", 4);
  RegisterRenderPriority ("object", 6);
  RegisterRenderPriority ("alpha", 8, CS_RENDPRI_SORT_BACK2FRONT);
}

int csEngine::GetRenderPriorityCount () const
{
  return (int)renderPriorities.Length ();
}

const char* csEngine::GetRenderPriorityName (long priority) const
{
  if (priority < 0 && (size_t)priority >= renderPriorities.Length ()) 
    return 0;
  return renderPriorities[priority];
}

void csEngine::ResetWorldSpecificSettings()
{
  SetClearZBuf (defaultClearZBuf);
  SetClearScreen (defaultClearScreen);
  csRef<iThingEnvironment> te (SCF_QUERY_INTERFACE (
          GetThingType (),
          iThingEnvironment));
  te->SetLightmapCellSize (16);
  SetMaxLightmapSize (defaultMaxLightmapWidth, defaultMaxLightmapHeight);
  SetAmbientLight (csColor (
  	defaultAmbientRed / 255.0f,
	defaultAmbientGreen / 255.0f, 
        defaultAmbientBlue / 255.0f));
  iRenderLoop* defaultRL = renderLoopManager->Retrieve 
    (CS_DEFAULT_RENDERLOOP_NAME);
  SetCurrentDefaultRenderloop (defaultRL);
}

void csEngine::PrepareTextures ()
{
  size_t i;

  iTextureManager *txtmgr = G3D->GetTextureManager ();

  // First register all textures to the texture manager.
  for (i = 0; i < textures->Length (); i++)
  {
    iTextureWrapper *csth = textures->Get (i);
    if (!csth->GetTextureHandle ()) csth->Register (txtmgr);
    if (!csth->KeepImage ()) csth->SetImageFile (0);
  }
}

void csEngine::PrepareMeshes ()
{
  int i;
  for (i = 0; i < meshes.GetCount (); i++)
  {
    iMeshWrapper *sp = meshes.Get (i);
    sp->GetMovable ()->UpdateMove ();
  }
}

bool csEngine::Prepare (iProgressMeter *meter)
{
  PrepareTextures ();
  PrepareMeshes ();


  // Prepare lightmaps if we have any sectors
  if (sectors.GetCount ()) ShineLights (0, meter);

  CheckConsistency ();

  return true;
}

void csEngine::ForceRelight (iRegion* region, iProgressMeter *meter)
{
  int old_lightmapCacheMode = lightmapCacheMode;
  lightmapCacheMode &= ~CS_ENGINE_CACHE_READ;
  lightmapCacheMode &= ~CS_ENGINE_CACHE_NOUPDATE;
  ShineLights (region, meter);
  lightmapCacheMode = old_lightmapCacheMode;
}

void csEngine::ForceRelight (iLight* light, iRegion* region)
{
  int sn;
  int num_meshes = meshes.GetCount ();

  for (sn = 0; sn < num_meshes; sn++)
  {
    iMeshWrapper *s = meshes.Get (sn);
    if (s->GetMovable ()->GetSectors ()->GetCount () <= 0)
      continue;	// No sectors, don't process lighting.
    if (!region || region->IsInRegion (s->QueryObject ()))
    {
      iLightingInfo* linfo = s->GetLightingInfo ();
      if (linfo)
      {
	// Do not clear!
        linfo->InitializeDefault (false);
      }
    }
  }

  ((csLight::Light*)light)->GetPrivateObject ()->CalculateLighting ();

  iCacheManager* cm = GetCacheManager ();
  for (sn = 0 ; sn < num_meshes ; sn++)
  {
    iMeshWrapper *s = meshes.Get (sn);
    if (s->GetMovable ()->GetSectors ()->GetCount () <= 0)
      continue;	// No sectors, don't process lighting.
    if (!region || region->IsInRegion (s->QueryObject ()))
    {
      iLightingInfo* linfo = s->GetLightingInfo ();
      if (linfo)
      {
	if (lightmapCacheMode & CS_ENGINE_CACHE_WRITE)
          linfo->WriteToCache (cm);
        linfo->PrepareLighting ();
      }
    }
  }

  cm->Flush ();
}

void csEngine::RemoveLight (iLight* light)
{

  int sn;
  int num_meshes = meshes.GetCount ();

  for (sn = 0; sn < num_meshes; sn++)
  {
    iMeshWrapper *s = meshes.Get (sn);
    iLightingInfo* linfo = s->GetLightingInfo ();
    if (linfo)
      linfo->LightDisconnect (light);
  }
  light->GetSector ()->GetLights ()->Remove (light);
}

void csEngine::SetVFSCacheManager (const char* vfspath)
{
  if (vfspath)
  {
    VFS->PushDir();
    VFS->ChDir (vfspath);
  }

  SetCacheManager (0);
  GetCacheManager ();

  if (vfspath)
  {
    VFS->PopDir();
  }
}

void csEngine::SetCacheManager (iCacheManager* cacheManager)
{
  csEngine::cacheManager = cacheManager;
}

iCacheManager* csEngine::GetCacheManager ()
{
  if (!cacheManager)
  {
    char buf[512];
    strcpy (buf, VFS->GetCwd ());
    if (buf[strlen (buf)-1] == '/')
      strcat (buf, "cache");
    else
      strcat (buf, "/cache");
    cacheManager = csPtr<iCacheManager> (
    	new csVfsCacheManager (objectRegistry, buf));
  }
  return cacheManager;
}

void csEngine::AddMeshAndChildren (iMeshWrapper* mesh)
{
  meshes.Add (mesh);
  iMeshList* ml = mesh->GetChildren ();
  int i;
  for (i = 0 ; i < ml->GetCount () ; i++)
    AddMeshAndChildren (ml->Get (i));
}

void csEngine::ShineLights (iRegion *region, iProgressMeter *meter)
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
    float cosinus_factor;
    int lightmap_size;
  };

  PrecalcInfo current;
  memset (&current, 0, sizeof (current));
  current.lm_version = 3;
  current.normal_light_level = CS_NORMAL_LIGHT_LEVEL;
  current.ambient_red = csLight::ambient_red;
  current.ambient_green = csLight::ambient_green;
  current.ambient_blue = csLight::ambient_blue;
  current.cosinus_factor = 0;	//@@@
  current.lightmap_size = 0;	//@@@

  char *reason = 0;

  bool do_relight = false;
  if (!(lightmapCacheMode & CS_ENGINE_CACHE_READ))
  {
    if (!(lightmapCacheMode & CS_ENGINE_CACHE_NOUPDATE))
      do_relight = true;
    else if (lightmapCacheMode & CS_ENGINE_CACHE_WRITE)
      do_relight = true;
  }

  iCacheManager* cm = GetCacheManager ();
  csRef<iDataBuffer> data = 0;
  if (lightmapCacheMode & CS_ENGINE_CACHE_READ)
    data = cm->ReadCache ("lm_precalc_info", 0, (uint32)~0);

  if (!data)
  {
    reason = "no 'lm_precalc_info' found in cache";
  }
  else
  {
    // data, 0-terminated
    csDataBuffer* ntData = new csDataBuffer (data);
    data = 0;
    char *input = **ntData;
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

      int xi = int (xf + ((xf < 0) ? -0.5 : + 0.5));

      if (!strcmp (keyword, "LMVERSION"))
      {
	if (xi != current.lm_version)
	{
	  reason = "lightmap format changed";
	  break;
	}
      }
    }
    delete ntData;
  }

  if (reason)
  {
    csString data;
    data.Format (
      "LMVERSION=%d\nNORMALLIGHTLEVEL=%d\nAMBIENT_RED=%d\nAMBIENT_GREEN=%d\nAMBIENT_BLUE=%d\nCOSINUS_FACTOR=%g\nLIGHTMAP_SIZE=%d\n",
      current.lm_version,
      current.normal_light_level,
      current.ambient_red,
      current.ambient_green,
      current.ambient_blue,
      current.cosinus_factor,
      current.lightmap_size);
    if (lightmapCacheMode & CS_ENGINE_CACHE_WRITE)
    {
      cm->CacheData (data.GetData(), data.Length(), "lm_precalc_info", 0, (uint32)~0);
    }
    if (do_relight)
    {
      Report ("Lightmaps are not up to date (%s).", reason);
    }
    else
    {
      Warn ("Lightmaps are not up to date (%s).", reason);
      Warn ("Use -relight cmd option to calc lighting.");
    }
    lightmapCacheMode &= ~CS_ENGINE_CACHE_READ;
  }

  // Recalculate do_relight since the cache mode might have changed.
  do_relight = false;
  if (!(lightmapCacheMode & CS_ENGINE_CACHE_READ))
  {
    if (!(lightmapCacheMode & CS_ENGINE_CACHE_NOUPDATE))
      do_relight = true;
    else if (lightmapCacheMode & CS_ENGINE_CACHE_WRITE)
      do_relight = true;
  }

  if (do_relight)
  {
    Report ("Recalculation of lightmaps forced.");
  }

  csRef<iLightIterator> lit = GetLightIterator (region);

  // Count number of lights to process.
  iLight *l;
  int light_count = 0;
  lit->Reset ();
  while (lit->HasNext ()) { lit->Next (); light_count++; }

  int sn = 0;
  int num_meshes = meshes.GetCount ();

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

  size_t failed = 0;
  csArray<iMeshWrapper*> failed_meshes;
  size_t max_failed_meshes = 4;
  if (doVerbose) max_failed_meshes = 100;
  for (sn = 0; sn < num_meshes; sn++)
  {
    iMeshWrapper *s = meshes.Get (sn);
    if (s->GetMovable ()->GetSectors ()->GetCount () <= 0 &&
	s->GetParentContainer () == 0)
      continue;	// No sectors and no parent mesh, don't process lighting.
    if (!region || region->IsInRegion (s->QueryObject ()))
    {
      iLightingInfo* linfo = s->GetLightingInfo ();
      if (linfo)
      {
        if (do_relight)
          linfo->InitializeDefault (true);
        else
          if (!linfo->ReadFromCache (cm))
	  {
	    if (failed_meshes.Length () < max_failed_meshes)
	      failed_meshes.Push (s);
	    failed++;
          }
      }
    }

    if (do_relight && meter) meter->Step ();
  }
  if (failed > 0)
  {
    Warn ("Couldn't load cached lighting for %zu object(s):",
	  failed);
    size_t i;
    for (i = 0 ; i < failed_meshes.Length () ; i++)
    {
      Warn ("    %s", failed_meshes[i]->QueryObject ()->GetName ());
    }
    if (failed_meshes.Length () < failed)
      Warn ("    ...");
    Warn ("Use -relight cmd option to refresh lighting.");
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

    lit->Reset ();
    int lit_cnt = 0;
    while (lit->HasNext ())
    {
      if (doVerbose)
      {
	csPrintf ("Doing light %d\n", lit_cnt);
	fflush (stdout);
      }
      lit_cnt++;
      l = lit->Next ();
      ((csLight::Light*)l)->GetPrivateObject ()->CalculateLighting ();
      if (meter) meter->Step ();
    }

    stop = csGetTicks ();
    Report ("Time taken: %.4f seconds.", (float)(stop - start) / 1000.);
  }

  if (do_relight && (lightmapCacheMode & CS_ENGINE_CACHE_WRITE))
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
    iMeshWrapper *s = meshes.Get (sn);
    if (s->GetMovable ()->GetSectors ()->GetCount () <= 0 &&
	s->GetParentContainer () == 0)
      continue;	// No sectors or parent mesh, don't process lighting.
    if (!region || region->IsInRegion (s->QueryObject ()))
    {
      iLightingInfo* linfo = s->GetLightingInfo ();
      if (linfo)
      {
	if (do_relight && (lightmapCacheMode & CS_ENGINE_CACHE_WRITE))
          linfo->WriteToCache (cm);
        linfo->PrepareLighting ();
      }
    }

    if (do_relight && meter) meter->Step ();
  }

  if (do_relight && (lightmapCacheMode & CS_ENGINE_CACHE_WRITE))
  {
    cm->Flush ();
  }
}

bool csEngine::CheckConsistency ()
{
  return false;
}

void csEngine::StartEngine ()
{
  DeleteAll ();
}

iEngineSequenceManager* csEngine::GetEngineSequenceManager (bool create)
{
  if (!sequenceManager && create)
  {
    csRef<iEngineSequenceManager> es = CS_QUERY_REGISTRY (
    	objectRegistry, iEngineSequenceManager);
    if (!es)
    {
      csRef<iPluginManager> plugin_mgr =
  	  CS_QUERY_REGISTRY (objectRegistry, iPluginManager);
      es = CS_LOAD_PLUGIN (plugin_mgr,
    	  "crystalspace.utilities.sequence.engine", iEngineSequenceManager);
      if (!es)
      {
	static bool engSeqLoadWarn = false;
	if (!engSeqLoadWarn)
	{
	  Warn ("Could not load the engine sequence manager!");
	  engSeqLoadWarn = true;
	}
        return 0;
      }
      if (!objectRegistry->Register (es, "iEngineSequenceManager"))
      {
	static bool engSeqRegWarn = false;
	if (!engSeqRegWarn)
	{
	  Warn ("Could not register the engine sequence manager!");
	  engSeqRegWarn = true;
	}
        return 0;
      }
    }
    sequenceManager = (iEngineSequenceManager*)es;
  }
  return sequenceManager;
}

void csEngine::PrecacheMesh (iMeshWrapper* s, iRenderView* rview)
{
  int num;
  if (s->GetMeshObject ())
    s->GetMeshObject ()->GetRenderMeshes (num, rview,
      s->GetMovable (), 0xf);
  iMeshList* children = s->GetChildren ();
  int i;
  for (i = 0 ; i < children->GetCount () ; i++)
    PrecacheMesh (children->Get (i), rview);
}

void csEngine::PrecacheDraw (iRegion* region)
{
  currentFrameNumber++;

  csRef<iCamera> c = CreateCamera ();
  csRef<iClipper2D> view;
  view.AttachNew (new csBoxClipper (0.0, 0.0, float (G3D->GetWidth ()),
  	float (G3D->GetHeight ())));

  csRenderView rview (c, view, G3D, G2D);
  StartDraw (c, view, rview);

  int sn;
  for (sn = 0; sn < meshes.GetCount (); sn++)
  {
    iMeshWrapper *s = meshes.Get (sn);
    if (!region || region->IsInRegion (s->QueryObject ()))
      PrecacheMesh (s, &rview);
  }

  for (sn = 0 ; sn < sectors.GetCount () ; sn++)
  {
    iSector* s = sectors.Get (sn);
    if (!region || region->IsInRegion (s->QueryObject ()))
      s->GetVisibilityCuller ()->PrecacheCulling ();
  }

  size_t i;
  for (i = 0 ; i < textures->Length () ; i++)
  {
    iTextureWrapper* txt = textures->Get (i);
    if (txt->GetTextureHandle ())
      if (!region || region->IsInRegion (txt->QueryObject ()))
      {
	txt->GetTextureHandle ()->Precache ();
      }
  }
}

void csEngine::StartDraw (iCamera *c, iClipper2D *view, csRenderView &rview)
{
  rview.SetEngine (this);
  rview.SetOriginalCamera (c);

  iEngineSequenceManager* eseqmgr = GetEngineSequenceManager ();
  if (eseqmgr)
  {
    eseqmgr->SetCamera (c);
  }

  // This flag is set in HandleEvent on a cscmdContextResize event
  if (resize)
  {
    resize = false;
    Resize ();
  }

  topLevelClipper = &rview;

  rview.GetClipPlane ().Set (0, 0, -1, 0);

  // Calculate frustum for screen dimensions (at z=1).
  float leftx = -c->GetShiftX () * c->GetInvFOV ();
  float rightx = (frameWidth - c->GetShiftX ()) * c->GetInvFOV ();
  float topy = -c->GetShiftY () * c->GetInvFOV ();
  float boty = (frameHeight - c->GetShiftY ()) * c->GetInvFOV ();
  rview.SetFrustum (leftx, rightx, topy, boty);
}

void csEngine::Draw (iCamera *c, iClipper2D *view)
{
  if (bugplug)
    bugplug->ResetCounter ("Sector Count");

  currentFrameNumber++;
  ControlMeshes ();
  csRenderView rview (c, view, G3D, G2D);
  StartDraw (c, view, rview);

  // First initialize G3D with the right clipper.
  G3D->SetClipper (view, CS_CLIPPER_TOPLEVEL);  // We are at top-level.
  G3D->ResetNearPlane ();
  G3D->SetPerspectiveAspect (c->GetFOV ());

  iSector *s = c->GetSector ();
  if (s) 
  {
    iRenderLoop* rl = s->GetRenderLoop ();
    if (!rl) rl = defaultRenderLoop;
    rl->Draw (&rview, s);
  }

  // draw all halos on the screen
  if (halos.Length () > 0)
  {
    csTicks elapsed = virtualClock->GetElapsedTicks ();
    size_t halo = halos.Length ();
    while (halo-- > 0)
      if (!halos[halo]->Process (elapsed, c, this))
	halos.DeleteIndex (halo);
  }
  G3D->SetClipper (0, CS_CLIPPER_NONE);
}

csPtr<iRenderLoop> csEngine::CreateDefaultRenderLoop ()
{
  csRef<iRenderLoop> loop = renderLoopManager->Create ();

  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (objectRegistry, iPluginManager));

  char const* const stdstep = "crystalspace.renderloop.step.generic.type";
  csRef<iRenderStepType> genType =
    CS_LOAD_PLUGIN (plugin_mgr, stdstep, iRenderStepType);

  if (genType.IsValid())
  {
    csRef<iRenderStepFactory> genFact = genType->NewFactory ();

    csRef<iRenderStep> step;
    csRef<iGenericRenderStep> genStep;

    step = genFact->Create ();
    loop->AddStep (step);
    genStep = SCF_QUERY_INTERFACE (step, iGenericRenderStep);
  
    genStep->SetShaderType ("standard");
    genStep->SetDefaultShader (defaultShader);
    genStep->SetZBufMode (CS_ZBUF_MESH);
    genStep->SetZOffset (false);
    genStep->SetPortalTraversal (true);
  }
  else
    Error("Failed to load plugin %s; pandemonium will ensue.", stdstep);

  return csPtr<iRenderLoop> (loop);
}


void csEngine::LoadDefaultRenderLoop (const char* fileName)
{
  csRef<iRenderLoop> newDefault = renderLoopManager->Load (fileName);
  if (newDefault != 0)
    defaultRenderLoop = newDefault;
}

void csEngine::AddHalo (iCamera* camera, csLight *Light)
{
  if (!Light->GetHalo () || Light->flags.Check (CS_LIGHT_ACTIVEHALO))
    return ;

  // Transform light pos into camera space and see if it is directly visible
  csVector3 v = camera->GetTransform ().Other2This (Light->GetCenter ());

  // Check if light is behind us
  if (v.z <= SMALL_Z) return ;

  // Project X,Y into screen plane
  float iz = camera->GetFOV () / v.z;
  v.x = v.x * iz + camera->GetShiftX ();
  v.y = frameHeight - 1 - (v.y * iz + camera->GetShiftY ());

  // If halo is not inside visible region, return
  if (!topLevelClipper->GetClipper ()->IsInside (csVector2 (v.x, v.y))) return ;

  // Check if light is not obscured by anything
  csVector3 isect;
  int polyidx = 0;
  if(camera->GetSector ()->HitBeamPortals (camera->GetTransform().GetOrigin(), 
      Light->GetCenter(), isect, &polyidx))
    return; // hit a mesh
  if(polyidx != -1) // double check on the above.
    return; // hit a polygon

  // Halo size is 1/4 of the screen height; also we make sure its odd
  int hs = (frameHeight / 4) | 1;

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
  size_t i;
  for (i = 0 ; i < halos.Length () ; i++)
  {
    csLightHalo* lh = halos[i];
    if (lh->Light == Light)
    {
      halos.DeleteIndex (i);
      return;
    }
  }
}

iLight *csEngine::FindLightID (const char* light_id) const
{
  for (int i = 0; i < sectors.GetCount (); i++)
  {
    iLight *l = sectors.Get (i)->GetLights ()->FindByID (light_id);
    if (l) return l;
  }

  return 0;
}

iLight *csEngine::FindLight (const char *name, bool regionOnly) const
{
  // XXX: Need to implement region?
  (void)regionOnly;

  // @@@### regionOnly
  int i;
  for (i = 0; i < sectors.GetCount (); i++)
  {
    iLight *l = sectors.Get (i)->GetLights ()->FindByName (name);
    if (l) return l;
  }

  return 0;
}

void csEngine::ControlMeshes ()
{
  nextframePending = virtualClock->GetCurrentTicks ();

  // Delete particle systems that self-destructed now.
  csSet<csPtrKey<iMeshWrapper> >::GlobalIterator it = 
    wantToDieSet.GetIterator();
  while (it.HasNext ())
  {
    iMeshWrapper* mesh = it.Next ();
    GetMeshes ()->Remove (mesh);
  }
  wantToDieSet.DeleteAll ();
}

char* csEngine::SplitRegionName (const char* name, iRegion*& region,
	bool& global)
{
  region = 0;
  global = false;

  char* p = (char*)strchr (name, '/');
  if (!p) return (char*)name;
  if (*name == '*' && *(name+1) == '/')
  {
    global = true;
    return p+1;
  }

  *p = 0;
  region = regions.FindByName (name);
  *p = '/';
  if (!region) return 0;
  return p+1;
}

iMaterialWrapper* csEngine::FindMaterial (const char* name,
	iRegion* reg)
{
  iRegion* region;
  bool global;
  char* n = SplitRegionName (name, region, global);
  if (!n) return 0;

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
  if (!n) return 0;

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
  if (!n) return 0;

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
  if (!n) return 0;

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
  if (!n) return 0;

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
  if (!n) return 0;

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
  if (!n) return 0;

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
  defaultMaxLightmapWidth = 
    Config->GetInt ("Engine.Lighting.MaxLightmapWidth", defaultMaxLightmapWidth);
  maxLightmapWidth = defaultMaxLightmapWidth;
  defaultMaxLightmapHeight = 
    Config->GetInt ("Engine.Lighting.MaxLightmapHeight", defaultMaxLightmapHeight);
  maxLightmapHeight = defaultMaxLightmapHeight;

  defaultAmbientRed = Config->GetInt (
      "Engine.Lighting.Ambient.Red",
      CS_DEFAULT_LIGHT_LEVEL);
  defaultAmbientGreen = Config->GetInt (
      "Engine.Lighting.Ambient.Green",
      CS_DEFAULT_LIGHT_LEVEL);
  defaultAmbientBlue = Config->GetInt (
      "Engine.Lighting.Ambient.Blue",
      CS_DEFAULT_LIGHT_LEVEL);

  csLight::ambient_red = defaultAmbientRed;
  csLight::ambient_green = defaultAmbientGreen;
  csLight::ambient_blue = defaultAmbientBlue;

  defaultClearZBuf = 
    Config->GetBool ("Engine.ClearZBuffer", defaultClearZBuf);
  clearZBuf = defaultClearZBuf;
  defaultClearScreen = 
    Config->GetBool ("Engine.ClearScreen", defaultClearScreen);
  clearScreen = defaultClearScreen;
}

struct LightAndDist
{
  iLight *light;
  float sqdist;
};

// csLightArray is a subclass of csCleanable which is registered
// to csEngine.cleanup.
class csLightArray : public iBase
{
public:
  SCF_DECLARE_IBASE;

  LightAndDist *array;

  // Size is the physical size of the array. num_lights is the number of
  // lights in it.
  int size, num_lights;

  csLightArray () : array(0), size(0), num_lights(0)
  {
    SCF_CONSTRUCT_IBASE (0);
  }

  virtual ~csLightArray ()
  { 
    delete[] array;
    SCF_DESTRUCT_IBASE ();
  }

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
SCF_IMPLEMENT_IBASE_END

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

// This is a static light array which is adapted to the
// right size everytime it is used. In the beginning it means
// that this array will grow a lot but finally it will
// stabilize to a maximum size (not big). The advantage of
// this approach is that we don't have a static array which can
// overflow. And we don't have to do allocation every time we
// come here. We register this memory to the 'cleanup' array
// in csEngine so that it will be freed later.
static csLightArray *light_array = 0;

static bool FindLightPos_Front2Back (csKDTree* treenode,
	void* userdata, uint32 cur_timestamp, uint32&)
{
  csVector3 pos = *(csVector3*)userdata;

  const csBox3& node_bbox = treenode->GetNodeBBox ();

  // In the first part of this test we are going to test if the
  // position is inside the node. If not then we don't need to continue.
  if (!node_bbox.In (pos))
  {
    return false;
  }

  treenode->Distribute ();

  int num_objects;
  csKDTreeChild** objects;
  num_objects = treenode->GetObjectCount ();
  objects = treenode->GetObjects ();

  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      // First test the bounding box of the object.
      const csBox3& obj_bbox = objects[i]->GetBBox ();

      if (obj_bbox.In (pos))
      {
        iLight* light = (iLight*)objects[i]->GetObject ();
	float sqdist = csSquaredDist::PointPoint (pos, light->GetCenter ());
	if (sqdist < csSquare(light->GetCutoffDistance ()))
	{
	  light_array->AddLight (light, sqdist);
	}
      }
    }
  }
  return true;
}

static bool FindLightBox_Front2Back (csKDTree* treenode,
	void* userdata, uint32 cur_timestamp, uint32&)
{
  csBox3* box = (csBox3*)userdata;

  const csBox3& node_bbox = treenode->GetNodeBBox ();

  // In the first part of this test we are going to test if the
  // box intersects with the node. If not then we don't need to continue.
  if (!node_bbox.TestIntersect (*box))
  {
    return false;
  }

  treenode->Distribute ();

  int num_objects;
  csKDTreeChild** objects;
  num_objects = treenode->GetObjectCount ();
  objects = treenode->GetObjects ();

  int i;
  for (i = 0 ; i < num_objects ; i++)
  {
    if (objects[i]->timestamp != cur_timestamp)
    {
      objects[i]->timestamp = cur_timestamp;
      // First test the bounding box of the object.
      const csBox3& obj_bbox = objects[i]->GetBBox ();

      if (obj_bbox.TestIntersect (*box))
      {
        iLight* light = (iLight*)objects[i]->GetObject ();
        csBox3 b (box->Min () - light->GetCenter (),
		  box->Max () - light->GetCenter ());
        float sqdist = b.SquaredOriginDist ();
        if (sqdist < csSquare (light->GetCutoffDistance ()))
	{
	  light_array->AddLight (light, sqdist);
	}
      }
    }
  }
  return true;
}


int csEngine::GetNearbyLights (
  iSector *sector,
  const csVector3 &pos,
  iLight **lights,
  int max_num_lights)
{
  int i;

  if (!light_array)
  {
    light_array = new csLightArray ();
    csEngine::currentEngine->cleanupList.Push (light_array);
    light_array->DecRef ();
  }

  light_array->Reset ();

  csKDTree* kdtree = ((csSector*)sector)->GetLightKDTree ();
  csVector3 position = pos;
  kdtree->Front2Back (pos, FindLightPos_Front2Back, &position, 0);

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

int csEngine::GetNearbyLights (
  iSector *sector,
  const csBox3 &box,
  iLight **lights,
  int max_num_lights)
{
  int i;

  if (!light_array)
  {
    light_array = new csLightArray ();
    csEngine::currentEngine->cleanupList.Push (light_array);
    light_array->DecRef ();
  }

  light_array->Reset ();

  csKDTree* kdtree = ((csSector*)sector)->GetLightKDTree ();
  csBox3 bbox = box;
  kdtree->Front2Back (box.Min (), FindLightBox_Front2Back, &bbox, 0);

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

csPtr<iSectorIterator> csEngine::GetNearbySectors (
  iSector *sector,
  const csVector3 &pos,
  float radius)
{
  csSectorIt *it = AllocSectorIterator (sector, pos, radius);
  return csPtr<iSectorIterator> (it);
}

void csEngine::GetNearbyObjectList (iSector* sector,
    const csVector3& pos, float radius, csArray<iObject*>& list,
    csArray<iSector*>& visited_sectors, bool crossPortals)
{
  iVisibilityCuller* culler = sector->GetVisibilityCuller ();
  csRef<iVisibilityObjectIterator> visit = culler->VisTest (
  	csSphere (pos, radius));

  //@@@@@@@@ TODO ALSO SUPPORT LIGHTS!
  while (visit->HasNext ())
  {
    iVisibilityObject* vo = visit->Next ();
    iMeshWrapper* imw = vo->GetMeshWrapper ();
    if (imw)
    {
      list.Push (imw->QueryObject ()); 
      if (crossPortals && imw->GetPortalContainer ())
      {
	iPortalContainer* portals = imw->GetPortalContainer ();
        int pc = portals->GetPortalCount ();
        int j;
        for (j = 0 ; j < pc ; j++)
        {
          iPortal* portal = portals->GetPortal (j);
	  const csVector3* world_vertices = portal->GetWorldVertices ();
          const csPlane3& wor_plane = portal->GetWorldPlane ();
          // Can we see the portal?
          if (wor_plane.Classify (pos) < -0.001)
          {
            csVector3 poly[100];	//@@@ HARDCODE
            int k;
	    int* idx = portal->GetVertexIndices ();
            for (k = 0 ; k < portal->GetVertexIndicesCount () ; k++)
            {
              poly[k] = world_vertices[idx[k]];
            }
            float sqdist_portal = csSquaredDist::PointPoly (
                  pos, poly, portal->GetVertexIndicesCount (),
                  wor_plane);
            if (sqdist_portal <= radius * radius)
            {
              // Also handle objects in the destination sector unless
              // it is a warping sector.
              portal->CompleteSector (0);
              CS_ASSERT (portal != 0);
              if (sector != portal->GetSector () && portal->GetSector ())
              {
                size_t l;
                bool already_visited = false;
                for (l = 0 ; l < visited_sectors.Length () ; l++)
                {
                  if (visited_sectors[l] == portal->GetSector ())
                  {
                    already_visited = true;
                    break;
                  }
                }
                if (!already_visited)
                {
                  visited_sectors.Push (portal->GetSector ());
		  if (portal->GetFlags ().Check (CS_PORTAL_WARP))
		  {
		    csReversibleTransform warp_wor;
		    portal->ObjectToWorld (
		    	imw->GetMovable ()->GetFullTransform (), warp_wor);
		    csVector3 tpos = warp_wor.Other2This (pos);
                    GetNearbyObjectList (portal->GetSector (), tpos, radius,
		    	list, visited_sectors);
		  }
		  else
		  {
                    GetNearbyObjectList (portal->GetSector (), pos, radius,
		    	list, visited_sectors);
		  }
		  // Uncommenting the below causes too many objects to be
		  // returned in some cases.
		  //visited_sectors.Pop ();
                }
              }
            }
          }
	}
      }
    }
  }
}

csPtr<iObjectIterator> csEngine::GetNearbyObjects (
  iSector *sector,
  const csVector3 &pos,
  float radius,
  bool crossPortals)
{
  csArray<iObject*>* list = new csArray<iObject*>;
  csArray<iSector*> visited_sectors;
  visited_sectors.Push (sector);
  GetNearbyObjectList (sector, pos, radius, *list, visited_sectors, crossPortals);
  csObjectListIt *it = new csObjectListIt (list);
  return csPtr<iObjectIterator> (it);
}

csPtr<iObjectIterator> csEngine::GetVisibleObjects (
  iSector* /*sector*/,
  const csVector3& /*pos*/)
{
  // @@@ Not implemented yet.
  return 0;
}

csPtr<iObjectIterator> csEngine::GetVisibleObjects (
  iSector* /*sector*/,
  const csFrustum& /*frustum*/)
{
  // @@@ Not implemented yet.
  return 0;
}

void csEngine::GetNearbyMeshList (iSector* sector,
    const csVector3& pos, float radius, csArray<iMeshWrapper*>& list,
    csArray<iSector*>& visited_sectors, bool crossPortals)
{
  iVisibilityCuller* culler = sector->GetVisibilityCuller ();
  csRef<iVisibilityObjectIterator> visit = culler->VisTest (
  	csSphere (pos, radius));

  //@@@@@@@@ TODO ALSO SUPPORT LIGHTS!
  while (visit->HasNext ())
  {
    iVisibilityObject* vo = visit->Next ();
    iMeshWrapper* imw = vo->GetMeshWrapper ();
    if (imw)
    {
      list.Push (imw); 
      if (crossPortals && imw->GetPortalContainer ())
      {
        iPortalContainer* portals = imw->GetPortalContainer ();
        int pc = portals->GetPortalCount ();
        int j;
        for (j = 0 ; j < pc ; j++)
        {
          iPortal* portal = portals->GetPortal (j);
	  const csVector3* world_vertices = portal->GetWorldVertices ();
          const csPlane3& wor_plane = portal->GetWorldPlane ();
          // Can we see the portal?
          if (wor_plane.Classify (pos) < -0.001)
          {
	    // @@@ Consider having a simpler version that looks
	    // at center of portal instead of trying to calculate distance
	    // to portal polygon?
            csVector3 poly[100];	//@@@ HARDCODE
            int k;
	    int* idx = portal->GetVertexIndices ();
            for (k = 0 ; k < portal->GetVertexIndicesCount () ; k++)
            {
              poly[k] = world_vertices[idx[k]];
            }
            float sqdist_portal = csSquaredDist::PointPoly (
                  pos, poly, portal->GetVertexIndicesCount (),
                  wor_plane);
            if (sqdist_portal <= radius * radius)
            {
              // Also handle objects in the destination sector unless
              // it is a warping sector.
              portal->CompleteSector (0);
              if (sector != portal->GetSector () && portal->GetSector ())
              {
                size_t l;
                bool already_visited = false;
                for (l = 0 ; l < visited_sectors.Length () ; l++)
                {
                  if (visited_sectors[l] == portal->GetSector ())
                  {
                    already_visited = true;
                    break;
                  }
                }
                if (!already_visited)
                {
                  visited_sectors.Push (portal->GetSector ());
		  if (portal->GetFlags ().Check (CS_PORTAL_WARP))
		  {
		    csReversibleTransform warp_wor;
		    portal->ObjectToWorld (
		    	imw->GetMovable ()->GetFullTransform (), warp_wor);
		    csVector3 tpos = warp_wor.Other2This (pos);
                    GetNearbyMeshList (portal->GetSector (), tpos, radius,
		    	list, visited_sectors);
		  }
		  else
		  {
                    GetNearbyMeshList (portal->GetSector (), pos, radius,
		    	list, visited_sectors);
		  }
		  // Uncommenting the below causes too many objects to be
		  // returned in some cases.
		  //visited_sectors.Pop ();
                }
              }
            }
          }
	}
      }
    }
  }
}

void csEngine::GetNearbyMeshList (iSector* sector,
    const csBox3& box, csArray<iMeshWrapper*>& list,
    csArray<iSector*>& visited_sectors, bool crossPortals)
{
  iVisibilityCuller* culler = sector->GetVisibilityCuller ();
  csRef<iVisibilityObjectIterator> visit = culler->VisTest (box);
  csVector3 pos = box.GetCenter ();

  //@@@@@@@@ TODO ALSO SUPPORT LIGHTS!
  while (visit->HasNext ())
  {
    iVisibilityObject* vo = visit->Next ();
    iMeshWrapper* imw = vo->GetMeshWrapper ();
    if (imw)
    {
      list.Push (imw); 
      if (crossPortals && imw->GetPortalContainer ())
      {
        iPortalContainer* portals = imw->GetPortalContainer ();
        int pc = portals->GetPortalCount ();
        int j;
        for (j = 0 ; j < pc ; j++)
        {
          iPortal* portal = portals->GetPortal (j);
	  const csVector3* world_vertices = portal->GetWorldVertices ();
          const csPlane3& wor_plane = portal->GetWorldPlane ();
          // Can we see the portal?
          if (wor_plane.Classify (pos) < -0.001)
          {
	    // @@@ Consider having a simpler version that looks
	    // at center of portal instead of trying to calculate distance
	    // to portal polygon?
            csVector3 poly[100];	//@@@ HARDCODE
            int k;
	    int* idx = portal->GetVertexIndices ();
            for (k = 0 ; k < portal->GetVertexIndicesCount () ; k++)
            {
              poly[k] = world_vertices[idx[k]];
            }
            //@@@float sqdist_portal = csSquaredDist::PointPoly (
                  //@@@pos, poly, portal->GetVertexIndicesCount (),
                  //@@@wor_plane);
            //@@@if (sqdist_portal <= radius * radius)
            {
              // Also handle objects in the destination sector unless
              // it is a warping sector.
              portal->CompleteSector (0);
              if (sector != portal->GetSector () && portal->GetSector ())
              {
                size_t l;
                bool already_visited = false;
                for (l = 0 ; l < visited_sectors.Length () ; l++)
                {
                  if (visited_sectors[l] == portal->GetSector ())
                  {
                    already_visited = true;
                    break;
                  }
                }
                if (!already_visited)
                {
                  visited_sectors.Push (portal->GetSector ());
		  if (portal->GetFlags ().Check (CS_PORTAL_WARP))
		  {
		    csReversibleTransform warp_wor;
		    portal->ObjectToWorld (
		    	imw->GetMovable ()->GetFullTransform (), warp_wor);
		    csVector3 tpos = warp_wor.Other2This (pos);
		    csBox3 tbox = box;
		    tbox.SetCenter (tpos);
                    GetNearbyMeshList (portal->GetSector (), tbox,
		    	list, visited_sectors);
		  }
		  else
		  {
                    GetNearbyMeshList (portal->GetSector (), box,
		    	list, visited_sectors);
		  }
		  // Uncommenting the below causes too many objects to be
		  // returned in some cases.
		  //visited_sectors.Pop ();
                }
              }
            }
          }
	}
      }
    }
  }
}

csPtr<iMeshWrapperIterator> csEngine::GetNearbyMeshes (
  iSector *sector,
  const csVector3 &pos,
  float radius,
  bool crossPortals)
{
  csArray<iMeshWrapper*>* list = new csArray<iMeshWrapper*>;
  csArray<iSector*> visited_sectors;
  visited_sectors.Push (sector);
  GetNearbyMeshList (sector, pos, radius, *list, visited_sectors, crossPortals);
  csMeshListIt *it = new csMeshListIt (list);
  return csPtr<iMeshWrapperIterator> (it);
}

csPtr<iMeshWrapperIterator> csEngine::GetNearbyMeshes (
  iSector *sector,
  const csBox3& box,
  bool crossPortals)
{
  csArray<iMeshWrapper*>* list = new csArray<iMeshWrapper*>;
  csArray<iSector*> visited_sectors;
  visited_sectors.Push (sector);
  GetNearbyMeshList (sector, box, *list, visited_sectors, crossPortals);
  csMeshListIt *it = new csMeshListIt (list);
  return csPtr<iMeshWrapperIterator> (it);
}

csPtr<iMeshWrapperIterator> csEngine::GetVisibleMeshes (
  iSector* /*sector*/,
  const csVector3& /*pos*/)
{
  // @@@ Not implemented yet.
  return 0;
}

csPtr<iMeshWrapperIterator> csEngine::GetVisibleMeshes (
  iSector* /*sector*/,
  const csFrustum& /*frustum*/)
{
  // @@@ Not implemented yet.
  return 0;
}

int csEngine::GetTextureFormat () const
{
  iTextureManager *txtmgr = G3D->GetTextureManager ();
  return txtmgr->GetTextureFormat ();
}

iRegion* csEngine::CreateRegion (const char *name)
{
  iRegion* region = regions.FindByName (name);
  if (!region)
  {
    csRegion *r = new csRegion (this);
    region = &r->scfiRegion;
    r->SetName (name);
    regions.Push (region);
    r->DecRef ();
  }
  return region;
}

iTextureWrapper *csEngine::CreateTexture (
  const char *name,
  const char *iFileName,
  csColor *iTransp,
  int iFlags)
{
  if (!imageLoader) return 0;

  // First of all, load the image file
  csRef<iDataBuffer> data = VFS->ReadFile (iFileName, false);
  if (!data || !data->GetSize ())
  {
    Warn ("Cannot read image file \"%s\" from VFS.", iFileName);
    return 0;
  }

  csRef<iImage> ifile (imageLoader->Load (data,
      G3D->GetTextureManager ()->GetTextureFormat ()));
      //CS_IMGFMT_TRUECOLOR));

  if (!ifile)
  {
    Warn ("Unknown image file format: \"%s\".", iFileName);
    return 0;
  }

  csRef<iDataBuffer> xname = VFS->ExpandPath (iFileName);
  ifile->SetName (**xname);

  // Okay, now create the respective texture handle object
  iTextureWrapper *tex = GetTextures ()->FindByName (name);
  if (tex)
    tex->SetImageFile (ifile);
  else
    tex = GetTextures ()->NewTexture (ifile);

  tex->SetFlags (iFlags);
  tex->QueryObject ()->SetName (name);

  if (iTransp)
    tex->SetKeyColor (
        csQint (iTransp->red * 255.2),
        csQint (iTransp->green * 255.2),
        csQint (iTransp->blue * 255.2));

  return tex;
}

iTextureWrapper *csEngine::CreateBlackTexture (
  const char *name,
  int w, int h,
  csColor *iTransp,
  int iFlags)
{
  csRef<iImage> ifile = csPtr<iImage>(new csImageMemory (w, h));
  ifile->SetName (name);

  // Okay, now create the respective texture handle object
  iTextureWrapper *tex = GetTextures ()->FindByName (name);
  if (tex)
    tex->SetImageFile (ifile);
  else
    tex = GetTextures ()->NewTexture (ifile);

  tex->SetFlags (iFlags);
  tex->QueryObject ()->SetName (name);

  if (iTransp)
    tex->SetKeyColor (
        csQint (iTransp->red * 255.2),
        csQint (iTransp->green * 255.2),
        csQint (iTransp->blue * 255.2));

  return tex;
}

iMaterialWrapper *csEngine::CreateMaterial (
  const char *name,
  iTextureWrapper *texture)
{
  csRef<csMaterial> mat;
  mat.AttachNew (new csMaterial (this, texture));
  iMaterialWrapper *wrapper = materials->NewMaterial (mat, name);

  return wrapper;
}

csPtr<iMeshWrapper> csEngine::CreateThingMesh (
  iSector *sector,
  const char *name)
{
  csRef<iMeshWrapper> thing_wrap (CreateMeshWrapper (
  	"crystalspace.mesh.object.thing", name, sector));
  thing_wrap->SetZBufMode (CS_ZBUF_USE);
  thing_wrap->SetRenderPriority (GetObjectRenderPriority ());
  return csPtr<iMeshWrapper> (thing_wrap);
}

csPtr<iMeshWrapper> csEngine::CreateSectorWallsMesh (
  iSector *sector,
  const char *name)
{
  csRef<iMeshWrapper> thing_wrap = CreateMeshWrapper (
  	"crystalspace.mesh.object.thing", name, sector);
  thing_wrap->SetZBufMode (CS_ZBUF_FILL);
  thing_wrap->SetRenderPriority (GetWallRenderPriority ());
  return csPtr<iMeshWrapper> (thing_wrap);
}

iSector *csEngine::CreateSector (const char *name)
{
  iSector *sector = (iSector*)(new csSector (this));
  sector->QueryObject ()->SetName (name);
  sectors.Add (sector);
  sector->DecRef ();

  FireNewSector (sector);

  return sector;
}

void csEngine::AddEngineSectorCallback (iEngineSectorCallback* cb)
{
  sectorCallbacks.Push (cb);
}

void csEngine::RemoveEngineSectorCallback (iEngineSectorCallback* cb)
{
  sectorCallbacks.Delete (cb);
}

void csEngine::FireNewSector (iSector* sector)
{
  size_t i = sectorCallbacks.Length ();
  while (i > 0)
  {
    i--;
    sectorCallbacks[i]->NewSector (this, sector);
  }
}

void csEngine::FireRemoveSector (iSector* sector)
{
  size_t i = sectorCallbacks.Length ();
  while (i > 0)
  {
    i--;
    sectorCallbacks[i]->RemoveSector (this, sector);
  }
}

csPtr<iMaterial> csEngine::CreateBaseMaterial (iTextureWrapper *txt)
{
  csRef<csMaterial> mat;
  mat.AttachNew (new csMaterial (this));
  if (txt) mat->SetTextureWrapper (txt);

  csRef<iMaterial> imat (SCF_QUERY_INTERFACE (mat, iMaterial));
  return csPtr<iMaterial> (imat);
}

iTextureList *csEngine::GetTextureList () const
{
  return &(GetTextures ()->scfiTextureList);
}

iMaterialList *csEngine::GetMaterialList () const
{
  return GetMaterials ();
}

iSharedVariableList *csEngine::GetVariableList () const
{
  return &(GetVariables()->scfiSharedVariableList);
}


iRegionList *csEngine::GetRegions ()
{
  return &(regions.scfiRegionList);
}

csPtr<iCamera> csEngine::CreateCamera ()
{
  return csPtr<iCamera> ((iCamera*)(new csCamera ()));
}

csPtr<iLight> csEngine::CreateLight (
  const char *name,
  const csVector3 &pos,
  float radius,
  const csColor &color,
  csLightDynamicType dyntype)
{
  csLight *light = new csLight (
      pos.x, pos.y, pos.z,
      radius,
      color.red, color.green, color.blue,
      dyntype);
  if (name) light->SetName (name);

  return csPtr<iLight> (&(light->scfiLight));
}

csPtr<iMeshFactoryWrapper> csEngine::CreateMeshFactory (
  const char *classId,
  const char *name)
{
  // WARNING! In the past this routine checked if the factory
  // was already created. This is wrong! This routine should not do
  // this and instead allow duplicate factories (with the same name).
  // That's because that duplicate factory can still be in another
  // region. And even if this is not the case then factories with same
  // name are still allowed.
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (objectRegistry, iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (
      plugin_mgr,
      classId,
      iMeshObjectType));
  if (!type) type = CS_LOAD_PLUGIN (plugin_mgr, classId, iMeshObjectType);
  if (!type) return 0;

  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  if (!fact) return 0;

  // don't pass the name to avoid a second search
  csRef<iMeshFactoryWrapper> fwrap (CreateMeshFactory (fact, name));
  return csPtr<iMeshFactoryWrapper> (fwrap);
}

csPtr<iMeshFactoryWrapper> csEngine::CreateMeshFactory (
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
  fact->SetMeshFactoryWrapper ((iMeshFactoryWrapper*)mfactwrap);
  return csPtr<iMeshFactoryWrapper> (&mfactwrap->scfiMeshFactoryWrapper);
}

csPtr<iMeshFactoryWrapper> csEngine::CreateMeshFactory (const char *name)
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
  return csPtr<iMeshFactoryWrapper> (&mfactwrap->scfiMeshFactoryWrapper);
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
  bool curRegOnly;

public:
  EngineLoaderContext (iEngine* Engine, iRegion* region, bool curRegOnly);
  virtual ~EngineLoaderContext ();

  SCF_DECLARE_IBASE;

  virtual iSector* FindSector (const char* name);
  virtual iMaterialWrapper* FindMaterial (const char* name);
  virtual iMaterialWrapper* FindNamedMaterial (const char* name,
  	const char* filename);
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name);
  virtual iMeshWrapper* FindMeshObject (const char* name);
  virtual iTextureWrapper* FindTexture (const char* name);
  virtual iTextureWrapper* FindNamedTexture (const char* name,
  	const char* filename);
  virtual iLight* FindLight (const char *name);
  virtual iShader* FindShader (const char* name);
  virtual bool CheckDupes () const { return false; }
  virtual iRegion* GetRegion () const { return region; }
  virtual bool CurrentRegionOnly () const { return curRegOnly; }
};

SCF_IMPLEMENT_IBASE(EngineLoaderContext);
  SCF_IMPLEMENTS_INTERFACE(iLoaderContext);
SCF_IMPLEMENT_IBASE_END

EngineLoaderContext::EngineLoaderContext (iEngine* Engine,
	iRegion* region, bool curRegOnly)
{
  SCF_CONSTRUCT_IBASE (0);
  EngineLoaderContext::Engine = Engine;
  EngineLoaderContext::region = region;
  EngineLoaderContext::curRegOnly = curRegOnly;
}

EngineLoaderContext::~EngineLoaderContext ()
{
  SCF_DESTRUCT_IBASE ();
}

iSector* EngineLoaderContext::FindSector (const char* name)
{
  return Engine->FindSector (name, curRegOnly ? region : 0);
}

iMaterialWrapper* EngineLoaderContext::FindMaterial (const char* name)
{
  return Engine->FindMaterial (name, curRegOnly ? region : 0);
}

iMaterialWrapper* EngineLoaderContext::FindNamedMaterial (const char* name,
                                                          const char* filename)
{
  return Engine->FindMaterial (name, curRegOnly ? region : 0);
}

iMeshFactoryWrapper* EngineLoaderContext::FindMeshFactory (const char* name)
{
  return Engine->FindMeshFactory (name, curRegOnly ? region : 0);
}

iMeshWrapper* EngineLoaderContext::FindMeshObject (const char* name)
{
  return Engine->FindMeshObject (name, curRegOnly ? region : 0);
}

iTextureWrapper* EngineLoaderContext::FindTexture (const char* name)
{
  return Engine->FindTexture (name, curRegOnly ? region : 0);
}

iTextureWrapper* EngineLoaderContext::FindNamedTexture (const char* name,
                                                        const char* filename)
{
  return Engine->FindTexture (name, curRegOnly ? region : 0);
}

iShader* EngineLoaderContext::FindShader (const char* name)
{
  if (!curRegOnly || !region)
    return csEngine::currentEngine->shaderManager->GetShader (name);

  csRefArray<iShader> shaders = csEngine::currentEngine->shaderManager
  	->GetShaders ();
  size_t i;
  for (i = 0 ; i < shaders.Length () ; i++)
  {
    iShader* s = shaders[i];
    if (region->IsInRegion (s->QueryObject ())
    	&& !strcmp (name, s->QueryObject ()->GetName ()))
      return s;
  }

  return 0;
}

iLight* EngineLoaderContext::FindLight(const char *name)
{
  csRef<iLightIterator> li = Engine->GetLightIterator (
  	curRegOnly ? region : 0);
  iLight *light;

  while (li->HasNext ())
  {
    light = li->Next ();
    if (!strcmp (light->QueryObject ()->GetName (),name))
      return light;
  }
  return 0;
}

//------------------------------------------------------------------------

csPtr<iLoaderContext> csEngine::CreateLoaderContext (iRegion* region,
	bool curRegOnly)
{
  return csPtr<iLoaderContext> (new EngineLoaderContext (this, region,
  	curRegOnly));
}

//------------------------------------------------------------------------

csPtr<iMeshFactoryWrapper> csEngine::LoadMeshFactory (
  const char *name,
  const char *loaderClassId,
  iDataBuffer *input)
{
  csRef<iDocumentSystem> xml (
    	CS_QUERY_REGISTRY (objectRegistry, iDocumentSystem));
  if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> doc = xml->CreateDocument ();
  const char* error = doc->Parse (input, true);
  if (error != 0)
  {
    // @@@ Report error?
    return 0;
  }

  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (objectRegistry, iPluginManager));
  csRef<iLoaderPlugin> plug (CS_QUERY_PLUGIN_CLASS (
      plugin_mgr,
      loaderClassId,
      iLoaderPlugin));
  if (!plug)
    plug = CS_LOAD_PLUGIN (plugin_mgr, loaderClassId, iLoaderPlugin);
  if (!plug) return 0;

  csRef<iMeshFactoryWrapper> fact (CreateMeshFactory (name));
  if (!fact) return 0;

  csRef<iLoaderContext> elctxt (CreateLoaderContext (0, true));
  csRef<iBase> mof (plug->Parse (
      doc->GetRoot (), elctxt, fact->GetMeshObjectFactory ()));
  if (!mof)
  {
    GetMeshFactories ()->Remove (fact);
    return 0;
  }

  csRef<iMeshObjectFactory> mof2 (
  	SCF_QUERY_INTERFACE (mof, iMeshObjectFactory));
  if (!mof2)
  {
    // @@@ ERROR?
    GetMeshFactories ()->Remove (fact);
    return 0;
  }

  fact->SetMeshObjectFactory (mof2);
  mof2->SetMeshFactoryWrapper (fact);

  return csPtr<iMeshFactoryWrapper> (fact);
}

csPtr<iMeshWrapper> csEngine::LoadMeshWrapper (
  const char *name,
  const char *loaderClassId,
  iDataBuffer *input,
  iSector *sector,
  const csVector3 &pos)
{
  csRef<iDocumentSystem> xml (
    	CS_QUERY_REGISTRY (objectRegistry, iDocumentSystem));
  if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> doc = xml->CreateDocument ();
  const char* error = doc->Parse (input, true);
  if (error != 0)
  {
    // @@@ Report error?
    return 0;
  }

  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (objectRegistry, iPluginManager));
  csRef<iLoaderPlugin> plug (CS_QUERY_PLUGIN_CLASS (
      plugin_mgr,
      loaderClassId,
      iLoaderPlugin));
  if (!plug)
    plug = CS_LOAD_PLUGIN (plugin_mgr, loaderClassId, iLoaderPlugin);
  if (!plug) return 0;

  csMeshWrapper *meshwrap = new csMeshWrapper (0);
  if (name) meshwrap->SetName (name);

  iMeshWrapper *imw = (iMeshWrapper*)meshwrap;
  GetMeshes ()->Add (imw);
  imw->DecRef (); // the ref is now stored in the MeshList
  if (sector)
  {
    (meshwrap->GetCsMovable ()).csMovable::SetSector (sector);
    (meshwrap->GetCsMovable ()).csMovable::SetPosition (pos);
    (meshwrap->GetCsMovable ()).csMovable::UpdateMove ();
  }

  csRef<iLoaderContext> elctxt (CreateLoaderContext (0, true));
  csRef<iBase> mof (plug->Parse (doc->GetRoot (), elctxt, imw));
  if (!mof)
  {
    GetMeshes ()->Remove (imw);
    return 0;
  }

  csRef<iMeshObject> mof2 (SCF_QUERY_INTERFACE (mof, iMeshObject));
  meshwrap->SetMeshObject (mof2);
  return csPtr<iMeshWrapper> (imw);
}


csPtr<iMeshWrapper> csEngine::CreatePortalContainer (const char* name,
  	iSector* sector, const csVector3& pos)
{
  csPortalContainer* pc = new csPortalContainer (this, objectRegistry);
  csRef<iMeshWrapper> mesh = CreateMeshWrapper ((iMeshObject*)pc,
  	name, sector, pos);
  csMeshWrapper* cmesh = (csMeshWrapper*)(iMeshWrapper*)mesh;
  if (GetPortalRenderPriority () != 0)
    cmesh->SetRenderPriority (GetPortalRenderPriority ());
  pc->SetMeshWrapper (cmesh);
  pc->DecRef ();
  return csPtr<iMeshWrapper> (mesh);
}

csPtr<iMeshWrapper> csEngine::CreatePortal (
	const char* name,
  	iMeshWrapper* parentMesh, iSector* destSector,
	csVector3* vertices, int num_vertices,
	iPortal*& portal)
{
  csRef<iMeshWrapper> mesh;
  csRef<iPortalContainer> pc;
  if (name)
  {
    mesh = parentMesh->GetChildren ()->FindByName (name);
    if (mesh)
    {
      pc = SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
  	iPortalContainer);
      if (!pc) mesh = 0;
    }
  }
  if (!mesh)
  {
    mesh = CreatePortalContainer (name);
    parentMesh->GetChildren ()->Add (mesh);
    pc = SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
  	iPortalContainer);
  }
  portal = pc->CreatePortal (vertices, num_vertices);
  portal->SetSector (destSector);
  return csPtr<iMeshWrapper> (mesh);
}

csPtr<iMeshWrapper> csEngine::CreatePortal (
	const char* name,
  	iSector* sourceSector, const csVector3& pos,
	iSector* destSector,
	csVector3* vertices, int num_vertices,
	iPortal*& portal)
{
  csRef<iMeshWrapper> mesh;
  csRef<iPortalContainer> pc;
  if (name && sourceSector)
  {
    mesh = sourceSector->GetMeshes ()->FindByName (name);
    if (mesh)
    {
      pc = SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
  	iPortalContainer);
      if (!pc) mesh = 0;
    }
  }
  if (!mesh)
  {
    mesh = CreatePortalContainer (name, sourceSector, pos);
    pc = SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
  	iPortalContainer);
  }
  portal = pc->CreatePortal (vertices, num_vertices);
  portal->SetSector (destSector);
  return csPtr<iMeshWrapper> (mesh);
}

csPtr<iMeshWrapper> csEngine::CreateMeshWrapper (
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

  mesh->GetMeshObject ()->SetMeshWrapper (mesh);
  return csPtr<iMeshWrapper> (mesh);
}

csPtr<iMeshWrapper> csEngine::CreateMeshWrapper (
  iMeshObject *mesh,
  const char *name,
  iSector *sector,
  const csVector3 &pos)
{
  csMeshWrapper *meshwrap = new csMeshWrapper (0, mesh);
  if (name) meshwrap->SetName (name);
  GetMeshes ()->Add ((iMeshWrapper*)meshwrap);
  if (sector)
  {
    (meshwrap->GetCsMovable ()).csMovable::SetSector (sector);
    (meshwrap->GetCsMovable ()).csMovable::SetPosition (pos);
    (meshwrap->GetCsMovable ()).csMovable::UpdateMove ();
  }

  mesh->SetMeshWrapper ((iMeshWrapper*)meshwrap);
  return csPtr<iMeshWrapper> ((iMeshWrapper*)meshwrap);
}

csPtr<iMeshWrapper> csEngine::CreateMeshWrapper (const char *name)
{
  csMeshWrapper *meshwrap = new csMeshWrapper (0);
  if (name) meshwrap->SetName (name);
  GetMeshes ()->Add ((iMeshWrapper*)meshwrap);
  return csPtr<iMeshWrapper> ((iMeshWrapper*)meshwrap);
}

csPtr<iMeshWrapper> csEngine::CreateMeshWrapper (
  const char *classId,
  const char *name,
  iSector *sector,
  const csVector3 &pos)
{
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (objectRegistry, iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (
      plugin_mgr, classId, iMeshObjectType));
  if (!type) type = CS_LOAD_PLUGIN (plugin_mgr, classId, iMeshObjectType);
  if (!type) return 0;

  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  if (!fact) return 0;

  csRef<iMeshObject> mo (SCF_QUERY_INTERFACE (fact, iMeshObject));
  if (!mo)
  {
    // The factory is not itself a mesh object. Let's see if the
    // factory can return a working mesh object.
    mo = fact->NewInstance ();
    if (mo)
      return CreateMeshWrapper (mo, name, sector, pos);
    return 0;
  }

  return CreateMeshWrapper (mo, name, sector, pos);
}

bool csEngine::RemoveObject (iBase *object)
{
  {
    csRef<iSector> sector (SCF_QUERY_INTERFACE (object, iSector));
    if (sector)
    {
      // Remove from region it might be in.
      if (sector->QueryObject ()->GetObjectParent ())
      {
	sector->QueryObject ()->GetObjectParent ()->ObjRemove (
		sector->QueryObject ());
      }
      sectors.Remove (sector);
      return true;
    }
  }
  {
    csRef<iCameraPosition> cp (SCF_QUERY_INTERFACE (object, iCameraPosition));
    if (cp)
    {
      // Remove from region it might be in.
      if (cp->QueryObject ()->GetObjectParent ())
      {
	cp->QueryObject ()->GetObjectParent ()->ObjRemove (
		cp->QueryObject ());
      }
      cameraPositions.Remove (cp);
      return true;
    }
  }
  {
    csRef<iLight> dl (SCF_QUERY_INTERFACE (object, iLight));
    if (dl && dl->GetDynamicType () == CS_LIGHT_DYNAMICTYPE_DYNAMIC)
    {
      // Remove from region it might be in.
      if (dl->QueryObject ()->GetObjectParent ())
      {
	dl->QueryObject ()->GetObjectParent ()->ObjRemove (
		dl->QueryObject ());
      }
      if (dl->GetSector ())
        dl->GetSector ()->GetLights ()->Remove (dl);
      return true;
    }
  }
  {
    csRef<iCollection> col (SCF_QUERY_INTERFACE (object, iCollection));
    if (col)
    {
      // Remove from region it might be in.
      if (col->QueryObject ()->GetObjectParent ())
      {
	col->QueryObject ()->GetObjectParent ()->ObjRemove (
		col->QueryObject ());
      }
      collections.Remove (col);
      return true;
    }
  }
  {
    csRef<iTextureWrapper> txt (SCF_QUERY_INTERFACE (object, iTextureWrapper));
    if (txt)
    {
      // Remove from region it might be in.
      if (txt->QueryObject ()->GetObjectParent ())
      {
	txt->QueryObject ()->GetObjectParent ()->ObjRemove (
		txt->QueryObject ());
      }
      GetTextureList ()->Remove (txt);
      return true;
    }
  }
  {
    csRef<iMaterialWrapper> mat (SCF_QUERY_INTERFACE (
        object,
        iMaterialWrapper));
    if (mat)
    {
      // Remove from region it might be in.
      if (mat->QueryObject ()->GetObjectParent ())
      {
	mat->QueryObject ()->GetObjectParent ()->ObjRemove (
		mat->QueryObject ());
      }
      GetMaterialList ()->Remove (mat);
      return true;
    }
  }
  {
    csRef<iMeshFactoryWrapper> factwrap (SCF_QUERY_INTERFACE (
        object,
        iMeshFactoryWrapper));
    if (factwrap)
    {
      // Remove from region it might be in.
      if (factwrap->QueryObject ()->GetObjectParent ())
      {
	factwrap->QueryObject ()->GetObjectParent ()->ObjRemove (
		factwrap->QueryObject ());
      }
      meshFactories.Remove (factwrap);
      return true;
    }
  }
  {
    csRef<iMeshWrapper> meshwrap (SCF_QUERY_INTERFACE (object, iMeshWrapper));
    if (meshwrap)
    {
      // Remove from region it might be in.
      if (meshwrap->QueryObject ()->GetObjectParent ())
      {
	meshwrap->QueryObject ()->GetObjectParent ()->ObjRemove (
		meshwrap->QueryObject ());
      }
      meshes.Remove (meshwrap);
      return true;
    }
  }

  return false;
}

void csEngine::Resize ()
{
  frameWidth = G3D->GetWidth ();
  frameHeight = G3D->GetHeight ();
}

void csEngine::SetContext (iTextureHandle *txthandle)
{
  if (currentRenderContext != txthandle)
  {
    currentRenderContext = txthandle;
    if (currentRenderContext)
    {
      currentRenderContext->GetRendererDimensions (frameWidth, frameHeight);
    }
    else
    {
      frameWidth = G3D->GetWidth ();
      frameHeight = G3D->GetHeight ();
    }
  }
}

iTextureHandle *csEngine::GetContext () const
{
  return currentRenderContext;
}

void csEngine::SetAmbientLight (const csColor &c)
{
  csLight::ambient_red = int (c.red * 255.0f);
  csLight::ambient_green = int (c.green * 255.0f);
  csLight::ambient_blue = int (c.blue * 255.0f);
}

void csEngine::GetAmbientLight (csColor &c) const
{
  c.red = csLight::ambient_red / 255.0f;
  c.green = csLight::ambient_green / 255.0f;
  c.blue = csLight::ambient_blue / 255.0f;
}

void csEngine::GetDefaultAmbientLight (csColor &c) const
{   
  c.red = defaultAmbientRed / 255.0f;
  c.green = defaultAmbientGreen / 255.0f;
  c.blue = defaultAmbientBlue / 255.0f;
}

csPtr<iFrustumView> csEngine::CreateFrustumView ()
{
  csFrustumView* lview = new csFrustumView ();
  lview->EnableThingShadows (CS_LIGHT_THINGSHADOWS);
  lview->SetShadowMask (CS_ENTITY_NOSHADOWS, 0);
  lview->SetProcessMask (CS_ENTITY_NOLIGHTING, 0);
  return csPtr<iFrustumView> (lview);
}

csPtr<iObjectWatcher> csEngine::CreateObjectWatcher ()
{
  csObjectWatcher* watch = new csObjectWatcher ();
  return csPtr<iObjectWatcher> (watch);
}

void csEngine::WantToDie (iMeshWrapper* mesh)
{
  wantToDieSet.Add (mesh);
}

bool csEngine::DebugCommand (const char* cmd)
{
  if (!strcasecmp (cmd, "toggle_cullstat"))
  {
    return true;
  }
  return false;
}

//-------------------End-Multi-Context-Support--------------------------------

// ======================================================================
// Render loop stuff
// ======================================================================
  
iRenderLoopManager* csEngine::GetRenderLoopManager ()
{
  return renderLoopManager;
}

iRenderLoop* csEngine::GetCurrentDefaultRenderloop ()
{

  return defaultRenderLoop;
}

bool csEngine::SetCurrentDefaultRenderloop (iRenderLoop* loop)
{
  if (loop == 0) return false;
  defaultRenderLoop = loop;
  return true;
}


static const csOptionDescription
  config_options[] =
{
  { 0, "fov", "Field of Vision", CSVAR_LONG },
  { 1, "relight", "Force/inhibit recalculation of lightmaps", CSVAR_BOOL },
  { 2, "renderloop", "Override the default render loop", CSVAR_STRING },
};
const int NUM_OPTIONS =
(
  sizeof (config_options) /
  sizeof (config_options[0])
);


bool csEngine::SetOption (int id, csVariant *value)
{
  switch (id)
  {
    case 0:
      csCamera::SetDefaultFOV (value->GetLong (), G3D->GetWidth ());
      break;
    case 1:
      if (value->GetBool ()) csEngine::lightmapCacheMode = CS_ENGINE_CACHE_WRITE;
      else                   csEngine::lightmapCacheMode = CS_ENGINE_CACHE_READ;
      break;
    case 2:
      LoadDefaultRenderLoop (value->GetString ());
    default:
      return false;
  }

  return true;
}

bool csEngine::GetOption (int id, csVariant *value)
{
  switch (id)
  {
    case 0:   value->SetLong (csCamera::GetDefaultFOV ()); break;
    case 1:   value->SetBool (csEngine::lightmapCacheMode == CS_ENGINE_CACHE_WRITE); break;
    case 2:   value->SetString (""); break; // @@@
    default:  return false;
  }

  return true;
}

bool csEngine::GetOptionDescription (
  int idx,
  csOptionDescription *option)
{
  if (idx < 0 || idx >= NUM_OPTIONS) return false;
  *option = config_options[idx];
  return true;
}
