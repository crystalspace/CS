/*
    Copyright (C) 1998-2006 by Jorrit Tyberghein

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
#include "csgeom/kdtree.h"
#include "csgeom/math.h"
#include "csgeom/polyclip.h"
#include "csgeom/sphere.h"
#include "csgfx/imagememory.h"
#include "csqint.h"
#include "csutil/cfgacc.h"
#include "csutil/databuf.h"
#include "csutil/scf.h"
#include "csutil/scfstrset.h"
#include "csutil/scanstr.h"
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
#include "imap/loader.h"
#include "imap/reader.h"
#include "iutil/cfgmgr.h"
#include "iutil/cmdline.h"
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
#include "iutil/selfdestruct.h"
#include "ivaria/engseq.h"
#include "ivaria/pmeter.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/halo.h"
#include "ivideo/txtmgr.h"
#include "plugins/engine/3d/camera.h"
#include "plugins/engine/3d/campos.h"
#include "plugins/engine/3d/collection.h"
#include "plugins/engine/3d/engine.h"
#include "plugins/engine/3d/halo.h"
#include "plugins/engine/3d/light.h"
#include "plugins/engine/3d/lightmgr.h"
#include "plugins/engine/3d/material.h"
#include "plugins/engine/3d/meshobj.h"
#include "plugins/engine/3d/objwatch.h"
#include "plugins/engine/3d/portalcontainer.h"
#include "plugins/engine/3d/sector.h"
#include "plugins/engine/3d/texture.h"
#include "plugins/engine/3d/meshgen.h"

using namespace CS_PLUGIN_NAMESPACE_NAME(Engine);

CS_IMPLEMENT_PLUGIN

bool csEngine::doVerbose = false;

//---------------------------------------------------------------------------
void csEngine::Report (const char *description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (!reporter) reporter = csQueryRegistry<iReporter> (objectRegistry);
  
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

  if (!reporter) reporter = csQueryRegistry<iReporter> (objectRegistry);

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

  if (!reporter) reporter = csQueryRegistry<iReporter> (objectRegistry);

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

  if (!reporter) reporter = csQueryRegistry<iReporter> (objectRegistry);

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
  csRef<csCameraPosition> cp;
  cp.AttachNew (new csCameraPosition (this, name, "", v, v, v));
  positions.Push (cp);
  return cp;
}

csPtr<iCameraPosition> csCameraPositionList::CreateCameraPosition (const char *name)
{
  csVector3 v (0);
  csRef<csCameraPosition> cp;
  cp.AttachNew (new csCameraPosition (this, name, "", v, v, v));
  return csPtr<iCameraPosition>(cp);
}

int csCameraPositionList::GetCount () const
{
  return (int)positions.GetSize ();
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
  CS::Threading::RecursiveMutexScopedLock lock(removeLock);
  return positions.Delete (obj);
}

bool csCameraPositionList::Remove (int n)
{
  CS::Threading::RecursiveMutexScopedLock lock(removeLock);
  return positions.DeleteIndex (n);
}

void csCameraPositionList::RemoveAll ()
{
  CS::Threading::RecursiveMutexScopedLock lock(removeLock);
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
  CS::Threading::RecursiveMutexScopedLock lock(removeLock);
  return positions.FindByName (Name);
}

//---------------------------------------------------------------------------

void csEngineMeshList::FreeMesh (iMeshWrapper* mesh)
{
  mesh->GetMovable ()->ClearSectors ();
  // @@@ Need similar for light/camera!
  mesh->QuerySceneNode ()->SetParent (0);
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
class csSectorIt : public scfImplementation1<csSectorIt, iSectorIterator>
{
public:
  csSectorIt (csArray<csSectorPos>* list)
    : scfImplementationType (this), list (list), 
      num_objects ((int)list->GetSize ()), cur_idx (0) { }
  virtual ~csSectorIt () { delete list; }

  virtual void Reset () { cur_idx = 0; }
  virtual iSector *Next ()
  {
    if (cur_idx >= num_objects) return 0;
    cur_idx++;
    lastPosition = (*list)[cur_idx-1].pos;
    return (*list)[cur_idx-1].sector;
  }
  virtual bool HasNext () const
  {
    return cur_idx < num_objects;
  }
  virtual const csVector3 &GetLastPosition () const
  { return lastPosition; }

private:
  csArray<csSectorPos>* list;
  int num_objects;

  // Current index.
  int cur_idx;
  csVector3 lastPosition;
};

/**
 * Iterator to iterate over meshes in the given list.
 */
class csMeshListIt : public scfImplementation1<csMeshListIt, 
                                               iMeshWrapperIterator>
{
public:
  /// Construct an iterator and initialize to start.
  csMeshListIt (csArray<iMeshWrapper*>* list)
    : scfImplementationType (this), list (list), 
      num_objects ((int)list->GetSize ()), cur_idx (0) { }
  virtual ~csMeshListIt () { delete list; }

  virtual void Reset () { cur_idx = 0; }
  virtual iMeshWrapper* Next ()
  {
    if (cur_idx >= num_objects) return 0;
    cur_idx++;
    return (*list)[cur_idx-1];
  }
  virtual bool HasNext () const
  {
    return cur_idx < num_objects;
  }

private:
  csArray<iMeshWrapper*>* list;
  int num_objects;

  // Current index.
  int cur_idx;
};

/**
 * Iterator to iterate over objects in the given list.
 */
class csObjectListIt : public scfImplementation1<csObjectListIt,
	iObjectIterator>
{
public:
  /// Construct an iterator and initialize to start.
  csObjectListIt (csArray<iObject*>* list)
    : scfImplementationType (this), list (list), 
      num_objects ((int)list->GetSize ()), cur_idx (0) { }
  virtual ~csObjectListIt () { delete list; }

  virtual void Reset () { cur_idx = 0; }
  virtual iObject* Next ()
  {
    if (cur_idx >= num_objects) return 0;
    cur_idx++;
    return (*list)[cur_idx-1];
  }
  virtual bool HasNext () const
  {
    return cur_idx < num_objects;
  }
  virtual iObject *GetParentObj () const
  {
    return 0;
  }

  virtual iObject* FindName (const char *name)  { (void)name; return 0; }

private:
  csArray<iObject*>* list;
  int num_objects;

  // Current index.
  int cur_idx;
};

//---------------------------------------------------------------------------

csLightIt::csLightIt (csEngine *e, iCollection *c) :
  scfImplementationType (this),
  engine(e),
  collection(c)
{
  Reset ();
}

csLightIt::~csLightIt ()
{
}

bool csLightIt::NextSector ()
{
  sectorIndex++;
  if(collection)
  {
    while(sectorIndex < engine->sectors.GetCount() &&
      	  !collection->IsParentOf(GetLastSector()->QueryObject()))
    {
      sectorIndex++;
    }
  }

  if (sectorIndex >= engine->sectors.GetCount())
  {
    return false;
  }
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

// ======================================================================
// Imposter stuff.
// ======================================================================

/**
 * Event handler that takes care of updating the imposters.
 */
class csImposterEventHandler : 
  public scfImplementation1<csImposterEventHandler, iEventHandler>
{
private:
  csWeakRef<csEngine> engine;

public:
  csImposterEventHandler (csEngine* engine)
    : scfImplementationType (this), engine (engine)
  {
  }
  virtual ~csImposterEventHandler ()
  {
  }

  virtual bool HandleEvent (iEvent& event)
  {
    if (engine)
      engine->HandleImposters ();
    return true;
  }

  CS_EVENTHANDLER_NAMES("crystalspace.engine.imposters")
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#include "csutil/custom_new_disable.h"

void csEngine::AddImposterToUpdateQueue (csImposterProcTex* imptex,
      iRenderView* rview)
{
  long camnr = rview->GetCamera ()->GetCameraNumber ();
  if (!imposterUpdateQueue.Contains (camnr))
  {
    // We don't yet have this camera in our queue. Make a clone of
    // the renderview and camera.
    CS::RenderManager::RenderView* copy_rview = 
      new (rviewPool) CS::RenderManager::RenderView (
        *(CS::RenderManager::RenderView*)rview);
    csImposterUpdateQueue qu;
    qu.rview.AttachNew (copy_rview);
    imposterUpdateQueue.Put (camnr, qu);

  }
  csImposterUpdateQueue* q = imposterUpdateQueue.GetElementPointer (camnr);
  q->queue.Push (csWeakRef<csImposterProcTex> (imptex));
}

#include "csutil/custom_new_enable.h"

void csEngine::HandleImposters ()
{
  // Imposter updating where needed.
  csHash<csImposterUpdateQueue,long>::GlobalIterator queue_it =
    imposterUpdateQueue.GetIterator ();
  while (queue_it.HasNext ())
  {
    csImposterUpdateQueue& q = queue_it.Next ();
    iRenderView* rview = q.rview;
    csWeakRefArray<csImposterProcTex>::Iterator it = q.queue.GetIterator ();

    // Update if camera is in a sector.
    iCamera* c = rview->GetCamera ();
    while (it.HasNext ())
    {
      csImposterProcTex* pt = it.Next ();
      pt->RenderToTexture (rview, c->GetSector ());
    }
  }

  // All updates done, empty list for next frame.
  imposterUpdateQueue.Empty ();
}

THREADED_CALLABLE_IMPL2(csEngine, SyncEngineLists, csRef<iThreadedLoader> loader, bool runNow)
{
  {
    csRef<iSectorLoaderIterator> loaderSectors = loader->GetLoaderSectors();
    while(loaderSectors->HasNext())
    {
      sectors.Add(loaderSectors->Next());
    }
  }

  {
    csRef<iMeshFactLoaderIterator> loaderMeshFactories = loader->GetLoaderMeshFactories();
    while(loaderMeshFactories->HasNext())
    {
      meshFactories.Add(loaderMeshFactories->Next());
    }
  }

  {
    csRef<iMeshLoaderIterator> loaderMeshes = loader->GetLoaderMeshes();
    while(loaderMeshes->HasNext())
    {
      meshes.Add(loaderMeshes->Next());
    }
  }

  {
    csRef<iCamposLoaderIterator> loaderCameraPositions = loader->GetLoaderCameraPositions();
    while(loaderCameraPositions->HasNext())
    {
      cameraPositions.Add(loaderCameraPositions->Next());
    }
  }

  {
    csRef<iMaterialLoaderIterator> loaderMaterials = loader->GetLoaderMaterials();
    while(loaderMaterials->HasNext())
    {
      materials->Add(loaderMaterials->Next());
    }
  }

  {
    csRef<iSharedVarLoaderIterator> loaderSharedVariables = loader->GetLoaderSharedVariables();
    while(loaderSharedVariables->HasNext())
    {
      sharedVariables->Add(loaderSharedVariables->Next());
    }
  }

  {
    csRef<iTextureLoaderIterator> loaderTextures = loader->GetLoaderTextures();
    while(loaderTextures->HasNext())
    {
      iTextureWrapper* txt = loaderTextures->Next();
      textures->Add(txt);
      newTextures.Push(txt);
    }
  }

  if(!runNow)
  {
    if(precache && tman->GetThreadCount() > 1)
    {
      // Precache a texture.
      if(!newTextures.IsEmpty())
      {
        csRef<iTextureWrapper> tex = newTextures.Pop();
        if(tex->GetTextureHandle())
        {
          tex->GetTextureHandle()->Precache();
        }
      }
    }
  
    // Schedule another run.
    SyncEngineLists(loader, false);
  }

  return true;
}

//---------------------------------------------------------------------------
SCF_IMPLEMENT_FACTORY (csEngine)

csEngine::csEngine (iBase *iParent) :
  scfImplementationType (this, iParent), objectRegistry (0),
  envTexHolder (this), enableEnvTex (true),
  frameWidth (0), frameHeight (0), 
  lightAmbientRed (CS_DEFAULT_LIGHT_LEVEL),
  lightAmbientGreen (CS_DEFAULT_LIGHT_LEVEL),
  lightAmbientBlue (CS_DEFAULT_LIGHT_LEVEL),
  sectors (this), textures (new csTextureList (this)), 
  materials (new csMaterialList), sharedVariables (new csSharedVariableList),
  renderLoopManager (0), topLevelClipper (0), resize (false),
  worldSaveable (false), maxAspectRatio (0), nextframePending (0),
  currentFrameNumber (0), 
  clearZBuf (false), defaultClearZBuf (false), 
  clearScreen (false),  defaultClearScreen (false), 
  currentRenderContext (0), weakEventHandler(0)
{
  ClearRenderPriorities ();
}

csEngine::~csEngine ()
{
  if (weakEventHandler != 0)
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (objectRegistry));
    if (q != 0)
      CS::RemoveWeakListener (q, weakEventHandler);
  }

  RemoveAllCollections();

  DeleteAllForce();

  renderPriorities.DeleteAll ();

  delete materials;
  delete textures;
  delete sharedVariables;
  delete renderLoopManager;
}

bool csEngine::Initialize (iObjectRegistry *objectRegistry)
{
  csEngine::objectRegistry = objectRegistry;

  tman = csQueryRegistry<iThreadManager>(objectRegistry);

  virtualClock = csQueryRegistry<iVirtualClock> (objectRegistry);
  if (!virtualClock) return false;

  globalStringSet = csQueryRegistryTagInterface<iStringSet> (
      objectRegistry, "crystalspace.shared.stringset");

  svNameStringSet = csQueryRegistryTagInterface<iShaderVarStringSet> (
    objectRegistry, "crystalspace.shader.variablenameset");

  colldet_id = globalStringSet->Request ("colldet");
  viscull_id = globalStringSet->Request ("viscull");
  base_id = globalStringSet->Request ("base");

  G3D = csQueryRegistry<iGraphics3D> (objectRegistry);
  if (!G3D)
  {
    // If there is no G3D then we still allow initialization of the
    // engine because it might be useful to use the engine stand-alone
    // (i.e. for calculating lighting and so on).
    Warn ("No 3D driver!");
  }

  csRef<iVerbosityManager> verbosemgr (
    csQueryRegistry<iVerbosityManager> (objectRegistry));
  if (verbosemgr) 
    doVerbose = verbosemgr->Enabled ("engine");
  if (doVerbose)
  {
    bugplug = csQueryRegistry<iBugPlug> (objectRegistry);
  }
  else
  {
    bugplug = 0;
  }

  VFS = csQueryRegistry<iVFS> (objectRegistry);
  if (!VFS) return false;

  if (G3D)
    G2D = G3D->GetDriver2D ();
  else
    G2D = 0;

  // don't check for failure; the engine can work without the image loader
  imageLoader = csQueryRegistry<iImageIO> (objectRegistry);
  if (!imageLoader) Warn ("No image loader. Loading images will fail.");

  // reporter is optional.
  reporter = csQueryRegistry<iReporter> (objectRegistry);

  // Tell event queue that we want to handle broadcast events
  CS_INITIALIZE_SYSTEM_EVENT_SHORTCUTS(objectRegistry);
  if (G2D)
  {
    CanvasResize = csevCanvasResize (objectRegistry, G2D);
    CanvasClose = csevCanvasClose (objectRegistry, G2D);
  }

  csRef<iEventQueue> q = csQueryRegistry<iEventQueue> (objectRegistry);
  if (q)
  {
    csEventID events[5] = { SystemOpen, SystemClose,
			    CanvasResize, CanvasClose,
			    CS_EVENTLIST_END };

    // discard canvas events if there is no canvas, by truncating the array
    if (!G2D) events[2] = CS_EVENTLIST_END;

    CS::RegisterWeakListener (q, this, events, weakEventHandler);

    csRef<csImposterEventHandler> imphandler;
    imphandler.AttachNew (new csImposterEventHandler (this));
    q->RegisterListener (imphandler, csevFrame (objectRegistry));
  }

  csConfigAccess cfg (objectRegistry, "/config/engine.cfg");
  ReadConfig (cfg);

  // Set up the RL manager.
  renderLoopManager = new csRenderLoopManager (this);

  csLightManager* light_mgr = new csLightManager ();
  objectRegistry->Register (light_mgr, "iLightManager");
  light_mgr->DecRef ();

  csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser> (objectRegistry);
  precache = cmdline->GetBoolOption ("precache", true);

  return true;
}

// Handle some system-driver broadcasts
bool csEngine::HandleEvent (iEvent &Event)
{
  if (Event.Name == SystemOpen)
  {
    if (G3D)
    {
      maxAspectRatio = 4096;
      frameWidth = G3D->GetWidth ();
      frameHeight = G3D->GetHeight ();
    }
    else
    {
      maxAspectRatio = 4096;
      frameWidth = 640;
      frameHeight = 480;
    }

    if (PerspectiveImpl::GetDefaultFOV () == 0)
      PerspectiveImpl::SetDefaultFOV (frameHeight/(float)frameWidth, 1.0f);

    StartEngine ();

    return true;
  }
  else if (Event.Name == SystemClose)
  {
    // We must free all material and texture handles since after
    // G3D->Close() they all become invalid, no matter whenever
    // we did or didn't an IncRef on them.
    DeleteAllForce ();
    return true;
  }
  else if (G2D)
  {
    if (Event.Name == CanvasResize)
    {
      //if (((iGraphics2D *)csCommandEventHelper::GetInfo(&Event)) == G2D)
      resize = true;
      return false;
    }
    else if (Event.Name == CanvasClose)
    {
      return false;
    }
  }

  return false;
}

void csEngine::SetRenderManager (iRenderManager* newRM)
{
  if (newRM == 0) return;

  if (renderManager.IsValid())
    objectRegistry->Unregister (renderManager, "iRenderManager");
  renderManager = newRM;
  
  objectRegistry->Register (renderManager, "iRenderManager");
}

void csEngine::ReloadRenderManager (csConfigAccess& cfg)
{
  const char fallbackRM[] = "crystalspace.rendermanager.rlcompat";
  const char* defaultRM = cfg->GetStr ("Engine.RenderManager.Default", 0);
  if (defaultRM == 0)
  {
    Warn ("No default render manager given, using '%s'", fallbackRM);
    defaultRM = fallbackRM;
  }
  csRef<iRenderManager> newRM = csLoadPlugin<iRenderManager> (objectRegistry,
    defaultRM);
  if (!newRM)
    Error ("No rendermanager set!");
  else
  {
    csEngine::SetRenderManager (newRM);
  }
}

void csEngine::ReloadRenderManager ()
{
  csConfigAccess cfg (objectRegistry, "/config/engine.cfg");
  ReloadRenderManager (cfg);
}

csShaderVariable* csEngine::GetLightAttenuationTextureSV()
{
  if (!lightAttenuationTexture)
  {
    lightAttenuationTexture.AttachNew (new csShaderVariable (
      lightSvNames.GetLightSVId (csLightShaderVarCache::lightAttenuationTex)));
    lightAttenuationTexture->SetType (csShaderVariable::TEXTURE);
    csRef<LightAttenuationTextureAccessor> accessor;
    accessor.AttachNew (new LightAttenuationTextureAccessor (this));
    lightAttenuationTexture->SetAccessor (accessor);
  }
  return lightAttenuationTexture;
}

void csEngine::DeleteAllForce ()
{
  // First notify all sector removal callbacks.
  int i;
  for (i = 0 ; i < sectors.GetCount () ; i++)
    FireRemoveSector (sectors.Get (i));

  nextframePending = 0;
  halos.DeleteAll ();
  wantToDieSet.Empty ();
  RemoveDelayedRemoves (false);

  GetMeshes ()->RemoveAll ();
  meshFactories.RemoveAll ();
  sectors.RemoveAll ();
  cameraPositions.RemoveAll ();

  defaultPortalMaterial.Invalidate ();
  materials->RemoveAll();
  textures->RemoveAll();
  sharedVariables->RemoveAll();

  if (shaderManager)
  {
    shaderManager->UnregisterShaderVariableAcessors ();
    shaderManager->UnregisterShaders ();
  }

  currentRenderContext = 0;

  // Clear all render priorities.
  ClearRenderPriorities ();

  // remove objects
  QueryObject ()->ObjRemoveAll ();

  envTexHolder.Clear ();
  
  lightAttenuationTexture.Invalidate ();
}

void csEngine::DeleteAll ()
{
  DeleteAllForce ();

  // Initialize some of the standard shaders again.
  if (G3D)
  {
    csConfigAccess cfg (objectRegistry, "/config/engine.cfg");
    shaderManager = csQueryRegistryOrLoad<iShaderManager> (objectRegistry,
    	"crystalspace.graphics3d.shadermanager");
    if (!shaderManager)
    {
      Warn ("Shader manager is missing!");
      return;
    }

    // Load default shaders
    csRef<iDocumentSystem> docsys (
      csQueryRegistry<iDocumentSystem> (objectRegistry));
    if (!docsys.IsValid())
      docsys.AttachNew (new csTinyDocumentSystem ());

    const char* shaderPath;
    shaderPath = cfg->GetStr ("Engine.Shader.Default", 
      "/shader/std_lighting.xml");
    defaultShader = LoadShader (docsys, shaderPath);
    if (!defaultShader.IsValid())
      Warn ("Default shader %s not available", shaderPath);

    shaderPath = cfg->GetStr ("Engine.Shader.Portal", 
      "/shader/std_lighting_portal.xml");
    csRef<iShader> portal_shader = LoadShader (docsys, shaderPath);
    if (!portal_shader.IsValid())
      Warn ("Default shader %s not available", shaderPath);
      
    csRef<csMaterial> portalMat;
    portalMat.AttachNew (new csMaterial (this));
    // FIXME: hardcoded shader type; prolly move to render manager
    portalMat->SetShader (globalStringSet->Request ("standard"),
      portal_shader);
    csRef<iMaterialWrapper> portalMaterialWrapper;
    portalMaterialWrapper = materials->NewMaterial (portalMat, 0);
    defaultPortalMaterial = portalMaterialWrapper;

    // Now, try to load the user-specified default render loop.
    const char* configLoop = cfg->GetStr ("Engine.RenderLoop.Default", 0);
    if (!override_renderloop.IsEmpty ())
    {
      defaultRenderLoop = renderLoopManager->Load (override_renderloop);
      if (!defaultRenderLoop)
      {
	Warn ("Default renderloop couldn't be created!");
	return;
      }
    }
    else if (!configLoop)
    {
      defaultRenderLoop = CreateDefaultRenderLoop ();
    }
    else
    {
      defaultRenderLoop = renderLoopManager->Load (configLoop);
      if (!defaultRenderLoop)
      {
	Warn ("Default renderloop couldn't be created!");
	return;
      }
    }

    // Register it.
    renderLoopManager->Register (CS_DEFAULT_RENDERLOOP_NAME, 
      defaultRenderLoop);

    csEngine::ReloadRenderManager();
  }
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
  size_t old_pri_len = renderPriorities.GetSize ();
  if ((size_t)(priority + 1) >= renderPrioritySortflag.GetSize ())
  {
    renderPrioritySortflag.SetSize (priority + 2);
    renderPriorities.SetSize (priority+2);
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

  renderPrioritySky = GetRenderPriority ("sky");
  renderPriorityPortal = GetRenderPriority ("portal");
  renderPriorityWall = GetRenderPriority ("wall");
  renderPriorityObject = GetRenderPriority ("object");
  renderPriorityAlpha = GetRenderPriority ("alpha");

  renderPrioritiesDirty = false;
}

long csEngine::GetRenderPriority (const char *name) const
{
  size_t i;
  for (i = 0; i < renderPriorities.GetSize (); i++)
  {
    const char *n = renderPriorities[i];
    if (n && !strcmp (name, n)) return (long)i;
  }

  return 0;
}

csRenderPrioritySorting csEngine::GetRenderPrioritySorting (const char *name) const
{
  size_t i;
  for (i = 0; i < renderPriorities.GetSize (); i++)
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
  renderPrioritySortflag.SetSize (0);
  RegisterRenderPriority ("init", 1);
  RegisterRenderPriority ("sky", 2);
  RegisterRenderPriority ("sky2", 3);
  RegisterRenderPriority ("portal", 4);
  RegisterRenderPriority ("wall", 5);
  RegisterRenderPriority ("wall2", 6);
  RegisterRenderPriority ("object", 7);
  RegisterRenderPriority ("object2", 8);
  RegisterRenderPriority ("transp", 9);
  RegisterRenderPriority ("alpha", 10, CS_RENDPRI_SORT_BACK2FRONT);
  RegisterRenderPriority ("final", 11);
}

int csEngine::GetRenderPriorityCount () const
{
  return (int)renderPriorities.GetSize ();
}

const char* csEngine::GetRenderPriorityName (long priority) const
{
  if (priority < 0 && (size_t)priority >= renderPriorities.GetSize ()) 
    return 0;
  return renderPriorities[priority];
}

void csEngine::ResetWorldSpecificSettings()
{
  SetClearZBuf (defaultClearZBuf);
  SetClearScreen (defaultClearScreen);
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
  for (i = 0; i < textures->GetSize (); i++)
  {
    iTextureWrapper *csth = textures->Get ((int)i);
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
  CheckConsistency ();
  return true;
}

void csEngine::RemoveLight (iLight* light)
{
  if (light->GetSector ())
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

THREADED_CALLABLE_IMPL1(csEngine, AddMeshAndChildren, iMeshWrapper* mesh)
{
  meshes.Add (mesh);
  // @@@ Consider no longer putting child meshes on main engine list???
  const csRef<iSceneNodeArray> children = mesh->QuerySceneNode ()
    ->GetChildrenArray ();
  size_t i;
  for (i = 0 ; i < children->GetSize() ; i++)
  {
    iMeshWrapper* mesh = children->Get (i)->QueryMesh ();
    if (mesh)
      AddMeshAndChildren (mesh);
  }

  return true;
}

bool csEngine::CheckConsistency ()
{
  return false;
}

void csEngine::StartEngine ()
{
  DeleteAll ();
  id_creation_time = svNameStringSet->Request("mesh creation time");
  svTexEnvironmentName = svNameStringSet->Request("tex environment");
  id_lod_fade = svNameStringSet->Request("lod fade");

  lightSvNames.SetStrings (svNameStringSet);
  /* Generate light SV IDs - that way, space for them will be reserved
   * when shader stacks are set up */
  for (int p = 0; p < csLightShaderVarCache::_lightCount; p++)
  {
    lightSvNames.GetLightSVId (csLightShaderVarCache::LightProperty (p));
  }
  for (int p = 0; p < csLightShaderVarCache::_varCount; p++)
  {
    lightSvNames.GetDefaultSVId (csLightShaderVarCache::DefaultSV (p));
  }
}

void csEngine::PrecacheMesh (iMeshWrapper* s)
{
  csRef<iCamera> c = CreateCamera ();
  csRef<iClipper2D> view;
  view.AttachNew (new csBoxClipper (0.0, 0.0, float (G3D->GetWidth ()),
    float (G3D->GetHeight ())));

  CS::RenderManager::RenderView rview (c, view, G3D, G2D);
  StartDraw (c, view, rview);
  PrecacheMesh (s, &rview);
}

void csEngine::PrecacheMesh (iMeshWrapper* s, iRenderView* rview)
{
  int num;
  if (s->GetMeshObject ())
    s->GetMeshObject ()->GetRenderMeshes (num, rview,
      s->GetMovable (), 0xf);
  const csRef<iSceneNodeArray> children = s->QuerySceneNode ()
    ->GetChildrenArray ();
  size_t i;
  for (i = 0 ; i < children->GetSize () ; i++)
  {
    iMeshWrapper* mesh = children->Get (i)->QueryMesh ();
    if (mesh)
      PrecacheMesh (mesh, rview);
  }
}

void csEngine::PrecacheDraw (iCollection* collection)
{
  currentFrameNumber++;

  csRef<iCamera> c = CreateCamera ();
  csRef<iClipper2D> view;
  view.AttachNew (new csBoxClipper (0.0, 0.0, float (G3D->GetWidth ()),
    float (G3D->GetHeight ())));

  CS::RenderManager::RenderView rview (c, view, G3D, G2D);
  StartDraw (c, view, rview);

  int sn;
  for (sn = 0; sn < meshes.GetCount (); sn++)
  {
    iMeshWrapper *s = meshes.Get (sn);
    if (!collection || collection->IsParentOf(s->QueryObject ()))
      PrecacheMesh (s, &rview);
  }

  for (sn = 0 ; sn < sectors.GetCount () ; sn++)
  {
    iSector* s = sectors.Get (sn);
    if (!collection || collection->IsParentOf(s->QueryObject ()))
      s->PrecacheDraw ();
  }

  while(!newTextures.IsEmpty())
  {
    csRef<iTextureWrapper> tex = newTextures.Pop();
    if(tex->GetTextureHandle() && (!collection || collection->IsParentOf(tex->QueryObject())))
    {
      tex->GetTextureHandle()->Precache();
    }
  }
}

void csEngine::StartDraw (iCamera *c, iClipper2D* /*view*/,
                          CS::RenderManager::RenderView &rview)
{
  rview.SetEngine (this);
  rview.SetOriginalCamera (c);

  // This flag is set in HandleEvent on a CanvasResize event
  if (resize)
  {
    resize = false;
    Resize ();
  }

  topLevelClipper = &rview;

  rview.GetClipPlane ().Set (0, 0, -1, 0);

  // Calculate frustum for screen dimensions (at z=1).
  c->SetViewportSize (rview.GetGraphics3D()->GetWidth(),
    rview.GetGraphics3D()->GetHeight());
  float leftx = -c->GetShiftX () * c->GetInvFOV ();
  float rightx = (frameWidth - c->GetShiftX ()) * c->GetInvFOV ();
  float topy = -c->GetShiftY () * c->GetInvFOV ();
  float boty = (frameHeight - c->GetShiftY ()) * c->GetInvFOV ();
  rview.SetFrustum (leftx, rightx, topy, boty);
}

#include "csutil/custom_new_disable.h"

void csEngine::Draw (iCamera *c, iClipper2D *view, iMeshWrapper* mesh)
{
  if (bugplug)
    bugplug->ResetCounter ("Sector Count");

  currentFrameNumber++;
  c->SetViewportSize (frameWidth, frameHeight);
  ControlMeshes ();
  csRef<CS::RenderManager::RenderView> rview;
  rview.AttachNew (new (rviewPool) CS::RenderManager::RenderView (c, view,
    G3D, G2D));
  StartDraw (c, view, *rview);

  // First initialize G3D with the right clipper.
  G3D->SetClipper (view, CS_CLIPPER_TOPLEVEL);  // We are at top-level.
  G3D->ResetNearPlane ();
  G3D->SetProjectionMatrix (c->GetProjectionMatrix ());

  FireStartFrame (rview);

  iSector *s = c->GetSector ();
  if (s) 
  {
    csShaderVariableStack& varStack = shaderManager->GetShaderVariableStack ();
    varStack.Setup (shaderManager->GetSVNameStringset ()->GetSize ());

    iRenderLoop* rl = s->GetRenderLoop ();
    if (!rl) rl = defaultRenderLoop;
    rl->Draw (rview, s, mesh);

    varStack.Setup (0);
  }

  // draw all halos on the screen
  if (halos.GetSize () > 0)
  {
    csTicks elapsed = virtualClock->GetElapsedTicks ();
    size_t halo = halos.GetSize ();
    while (halo-- > 0)
      if (!halos[halo]->Process (elapsed, c, this))
	halos.DeleteIndex (halo);
  }
  G3D->SetClipper (0, CS_CLIPPER_NONE);
}

#include "csutil/custom_new_enable.h"

csPtr<iRenderLoop> csEngine::CreateDefaultRenderLoop ()
{
  csRef<iRenderLoop> loop = renderLoopManager->Create ();

  csRef<iPluginManager> plugin_mgr (
  	csQueryRegistry<iPluginManager> (objectRegistry));

  char const* const stdstep = "crystalspace.renderloop.step.generic.type";
  csRef<iRenderStepType> genType =
    csLoadPlugin<iRenderStepType> (plugin_mgr, stdstep);

  if (genType.IsValid())
  {
    csRef<iRenderStepFactory> genFact = genType->NewFactory ();

    csRef<iRenderStep> step;
    csRef<iGenericRenderStep> genStep;

    step = genFact->Create ();
    loop->AddStep (step);
    genStep = scfQueryInterface<iGenericRenderStep> (step);
  
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

csRef<iShader> csEngine::LoadShader (iDocumentSystem* docsys,
                                     const char* filename)
{
  csRef<iDocument> shaderDoc = docsys->CreateDocument ();
  csRef<iShader> shader;
  csString shaderFn (filename);
  csString shaderDir;

  size_t slash = shaderFn.FindLast ('/');
  if (slash == (size_t)-1)
    shaderDir = "/shader/";
  else
  {
    shaderDir = shaderFn.Slice (0, slash + 1);
    shaderFn.DeleteAt (0, slash + 1);
  }

  VFS->PushDir();
  VFS->ChDir (shaderDir);
  csRef<iFile> shaderFile = VFS->Open (shaderFn, VFS_FILE_READ);
  if (shaderFile.IsValid())
  {
    const char* err = shaderDoc->Parse (shaderFile, false);
    if (err != 0)
    {
      Warn ("Error parsing %s: %s", filename, err);
      return 0;
    }
    
    csRef<iDocumentNode> shaderNode = shaderDoc->GetRoot ()->
      GetNode ("shader");
    if (!shaderNode)
    {
      Warn ("%s has no 'shader' node", filename);
      return 0;
    }
    
    const char* compilerAttr = shaderNode->GetAttributeValue ("compiler");
    if (!compilerAttr)
    {
      Warn ("%s: 'shader' node has no 'compiler' attribute", filename);
      return 0;
    }
    
    csRef<iShaderCompiler> shcom (shaderManager->GetCompiler (compilerAttr));
    if (!shcom.IsValid())
    {
      Warn ("%s: '%s' shader compiler not available", filename, compilerAttr);
    }
    
    csRef<iLoaderContext> elctxt (CreateLoaderContext (0, true));
    shader = shcom->CompileShader (elctxt, shaderNode);
    if (shader.IsValid()) shaderManager->RegisterShader (shader);
  }
  VFS->PopDir();
  return shader;
}

void csEngine::AddHalo (iCamera* camera, csLight *Light)
{
  if (!Light->GetHalo () || Light->flags.Check (CS_LIGHT_ACTIVEHALO))
    return ;

  csVector3 light_pos = Light->GetMovable ()->GetFullPosition ();

  // Transform light pos into camera space and see if it is directly visible
  csVector3 v = camera->GetTransform ().Other2This (light_pos);

  // Check if light is behind us
  if (v.z <= SMALL_Z) return ;

  // Project X,Y into screen plane
  float iz = camera->GetFOV () / v.z;
  v.x = v.x * iz + camera->GetShiftX ();
  v.y = frameHeight - 1 - (v.y * iz + camera->GetShiftY ());

  // If halo is not inside visible region, return
  if (!topLevelClipper->GetClipper ()->IsInside (csVector2 (v.x, v.y))) return ;

  // Check if light is not obscured by anything
  csSectorHitBeamResult hbresult = camera->GetSector ()->HitBeamPortals (camera->GetTransform().GetOrigin(), light_pos );
  if(hbresult.mesh)
    return; // hit a mesh
  if(hbresult.polygon_idx != -1) // double check on the above.
    return; // hit a polygon

  // Halo size is 1/4 of the screen height; also we make sure its odd
  int hs = (frameHeight / 4) | 1;

  if (((csHalo*)Light->GetHalo ())->Type == cshtFlare)
  {
    // put a new light flare into the queue
    // the cast is safe because of the type check above
    halos.Push (
        new csLightFlareHalo (Light, (csFlareHalo *)Light->GetHalo (), hs));
    return ;
  }

  // Okay, put the light into the queue: first we generate the alphamap
  unsigned char *Alpha = ((csHalo*)Light->GetHalo ())->Generate (hs);
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
  for (i = 0 ; i < halos.GetSize () ; i++)
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
  wantToDieSet.Empty ();

  // Delete all objects that should be removed given the current
  // time.
  csTicks current = virtualClock->GetCurrentTicks ();
  while (delayedRemoves.GetSize () > 0
      && delayedRemoves.Top ().time_to_delete <= current)
  {
    csDelayedRemoveObject ro = delayedRemoves.Pop ();
    RemoveObject (ro.object);
  }
}

const char* csEngine::SplitCollectionName(const char* name, iCollection*& collection,
	bool& global)
{
  collection = 0;
  global = false;

  const char* p = strchr (name, '/');
  if (!p) return name;
  if (*name == '*' && *(name+1) == '/')
  {
    global = true;
    return p+1;
  }

  csString collectionPart (name, p - name);
  collection = GetCollection (collectionPart);
  return p;
}

iMaterialWrapper* csEngine::FindMaterial(const char* name,
	iCollection* col)
{
  iCollection* collection;
  bool global;
  const char* n = SplitCollectionName (name, collection, global);
  if (!n) return 0;

  iMaterialWrapper* mat;
  if (collection)
    mat = collection->FindMaterial (n);
  else if (!global && col)
    mat = col->FindMaterial (n);
  else
    mat = GetMaterialList ()->FindByName (n);
  return mat;
}

iTextureWrapper* csEngine::FindTexture (const char* name,
	iCollection* col)
{
  iCollection* collection;
  bool global;
  const char* n = SplitCollectionName (name, collection, global);
  if (!n) return 0;

  iTextureWrapper* txt;
  if (collection)
    txt = collection->FindTexture (n);
  else if (!global && col)
    txt = col->FindTexture (n);
  else
    txt = GetTextureList ()->FindByName (n);
  return txt;
}

iSector* csEngine::FindSector (const char* name,
                               iCollection* col = 0)
{
  iCollection* collection;
  bool global;
  const char* n = SplitCollectionName (name, collection, global);
  if (!n) return 0;

  csRef<iSector> sect;
  if (collection)
    sect = collection->FindSector(n);
  else if (!global && col)
    sect = col->FindSector(n);
  else
    sect = GetSectors()->FindByName (n);
  return sect;
}

iMeshWrapper* csEngine::FindMeshObject (const char* name,
	iCollection* col)
{
  iCollection* collection;
  bool global;
  const char* n = SplitCollectionName(name, collection, global);
  if (!n) return 0;

  iMeshWrapper* mesh;
  if (collection)
    mesh = collection->FindMeshObject(n);
  else if (!global && col)
    mesh = col->FindMeshObject(n);
  else
    mesh = GetMeshes()->FindByName(n);
  return mesh;
}

iMeshFactoryWrapper* csEngine::FindMeshFactory (const char* name,
	iCollection* col)
{
  iCollection* collection;
  bool global;
  const char* n = SplitCollectionName (name, collection, global);
  if (!n) return 0;

  iMeshFactoryWrapper* fact;
  if (collection)
    fact = collection->FindMeshFactory (n);
  else if (!global && col)
    fact = col->FindMeshFactory (n);
  else
    fact = GetMeshFactories ()->FindByName (n);
  return fact;
}

iCameraPosition* csEngine::FindCameraPosition (const char* name,
	iCollection* col)
{
  iCollection* collection;
  bool global;
  const char* n = SplitCollectionName (name, collection, global);
  if (!n) return 0;

  iCameraPosition* campos;
  if (collection)
    campos = collection->FindCameraPosition (n);
  else if (!global && col)
    campos = col->FindCameraPosition (n);
  else
    campos = GetCameraPositions ()->FindByName (n);
  return campos;
}

void csEngine::ReadConfig (iConfigFile *Config)
{
  defaultAmbientRed = Config->GetInt (
      "Engine.Lighting.Ambient.Red",
      CS_DEFAULT_LIGHT_LEVEL);
  defaultAmbientGreen = Config->GetInt (
      "Engine.Lighting.Ambient.Green",
      CS_DEFAULT_LIGHT_LEVEL);
  defaultAmbientBlue = Config->GetInt (
      "Engine.Lighting.Ambient.Blue",
      CS_DEFAULT_LIGHT_LEVEL);

  lightAmbientRed = defaultAmbientRed;
  lightAmbientGreen = defaultAmbientGreen;
  lightAmbientBlue = defaultAmbientBlue;

  defaultClearZBuf = 
    Config->GetBool ("Engine.ClearZBuffer", defaultClearZBuf);
  clearZBuf = defaultClearZBuf;
  defaultClearScreen = 
    Config->GetBool ("Engine.ClearScreen", defaultClearScreen);
  clearScreen = defaultClearScreen;
  
  enableEnvTex = 
    Config->GetBool ("Engine.AutomaticEnvironmentCube", true);
}

struct LightAndDist
{
  iLight *light;
  float sqdist;
};

// csLightArray is a subclass of csCleanable which is registered
// to csEngine.cleanup.
class csLightArray : public scfImplementation0<csLightArray>
{
public:
  LightAndDist *array;

  // Size is the physical size of the array. num_lights is the number of
  // lights in it.
  int size, num_lights;

  csLightArray () 
    : scfImplementationType (this), array(0), size(0), num_lights(0)
  {
  }

  virtual ~csLightArray ()
  { 
    delete[] array;
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

struct LightCollectPoint
{
  LightCollectPoint (csLightArray* lightArray, const csVector3& pos)
    : lightArray (lightArray), pos (pos)
  {}

  bool operator() (const csSectorLightList::LightAABBTree::Node* node)
  {
    if (!node->GetBBox ().In (pos))
      return true;

    for (size_t i = 0; i < node->GetObjectCount (); ++i)
    {
      csLight* light = node->GetLeafData (i);
      
      float sqdist = csSquaredDist::PointPoint (pos,
        light->GetMovable ()->GetFullPosition ());
      if (sqdist < csSquare(light->GetCutoffDistance ()))
      {
        lightArray->AddLight (light, sqdist);
      }
    }
    return false;
  }

  csLightArray* lightArray;
  const csVector3& pos;
};

struct LightCollectInnerPoint
{
  LightCollectInnerPoint (const csVector3& pos)
    : pos (pos)
  {}

  bool operator() (const csSectorLightList::LightAABBTree::Node* node)
  {
    return node->GetBBox ().In (pos);
  }

  const csVector3& pos;
};

struct LightCollectBox
{
  LightCollectBox (csLightArray* lightArray, const csBox3& box)
    : lightArray (lightArray), box (box)
  {}

  bool operator() (const csSectorLightList::LightAABBTree::Node* node)
  {
    if (!node->GetBBox ().TestIntersect (box))
      return true;

    for (size_t i = 0; i < node->GetObjectCount (); ++i)
    {
      csLight* light = node->GetLeafData (i);
      csVector3 light_pos = light->GetMovable ()->GetFullPosition ();
      csBox3 b (box.Min () - light_pos, box.Max () - light_pos);
      float sqdist = b.SquaredOriginDist ();
      if (sqdist < csSquare (light->GetCutoffDistance ()))
      {
        lightArray->AddLight (light, sqdist);
      }
    }
    return true;
  }

  csLightArray* lightArray;
  const csBox3& box;
};

struct LightCollectInnerBox
{
  LightCollectInnerBox (const csBox3& box)
    : box (box)
  {}

  bool operator() (const csSectorLightList::LightAABBTree::Node* node)
  {
    return node->GetBBox ().TestIntersect (box);
  }

  const csBox3& box;
};


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
    cleanupList.Push (light_array);
    light_array->DecRef ();
  }

  light_array->Reset ();
  
  const csSectorLightList::LightAABBTree& tree = (static_cast<csSector*> (sector))->GetLightAABBTree ();
  LightCollectPoint collector (light_array, pos);
  LightCollectInnerPoint inner (pos);
  tree.TraverseOut (inner, collector, pos);

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
    cleanupList.Push (light_array);
    light_array->DecRef ();
  }

  light_array->Reset ();

  const csSectorLightList::LightAABBTree& tree = (static_cast<csSector*> (sector))->GetLightAABBTree ();
  LightCollectBox collector (light_array, box);
  LightCollectInnerBox inner (box);
  tree.Traverse (inner, collector);

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

static bool TestPortalSphere (iPortal* portal, float radius,
	const csVector3& pos, iSector* sector,
	csSet<csPtrKey<iSector> >& visited_sectors)
{
  const csPlane3& wor_plane = portal->GetWorldPlane ();
  // Can we see the portal?
  if (wor_plane.Classify (pos) < -0.001)
  {
    const csVector3* world_vertices = portal->GetWorldVertices ();
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
      portal->CompleteSector (0);
      if (sector != portal->GetSector () && portal->GetSector ())
      {
        if (!visited_sectors.In (portal->GetSector ()))
        {
          visited_sectors.Add (portal->GetSector ());
	  return true;
	}
      }
    }
  }
  return false;
}

static bool TestPortalBox (iPortal* portal, const csBox3& box,
	const csVector3& pos, iSector* sector,
	csSet<csPtrKey<iSector> >& visited_sectors)
{
  const csPlane3& wor_plane = portal->GetWorldPlane ();
  // Can we see the portal?
  if (wor_plane.Classify (pos) < -0.001)
  {
    const csVector3* world_vertices = portal->GetWorldVertices ();
    int k;
    int* idx = portal->GetVertexIndices ();
    const csVector3& w0 = world_vertices[idx[0]];
    for (k = 0 ; k < portal->GetVertexIndicesCount ()-2 ; k++)
    {
      if (csIntersect3::BoxTriangle (box, w0, world_vertices[idx[k+1]],
        world_vertices[idx[k+2]]))
      {
	portal->CompleteSector (0);
	if (sector != portal->GetSector () && portal->GetSector ())
	{
	  if (!visited_sectors.In (portal->GetSector ()))
	  {
	    visited_sectors.Add (portal->GetSector ());
	    return true;
	  }
        }
      }
    }
  }
  return false;
}

static csVector3 WarpVector (iPortal* portal, const csVector3& pos,
	iMeshWrapper* imw)
{
  csReversibleTransform warp_wor;
  portal->ObjectToWorld (imw->GetMovable ()->GetFullTransform (), warp_wor);
  return warp_wor.Other2This (pos);
}

static csVector3 WarpVectorCond (iPortal* portal, const csVector3& pos,
	iMeshWrapper* imw)
{
  if (!portal->GetFlags ().Check (CS_PORTAL_WARP))
    return pos;
  csReversibleTransform warp_wor;
  portal->ObjectToWorld (imw->GetMovable ()->GetFullTransform (), warp_wor);
  return warp_wor.Other2This (pos);
}

void csEngine::GetNearbySectorList (iSector* sector,
    const csBox3& box, csArray<csSectorPos>& list,
    csSet<csPtrKey<iSector> >& visited_sectors)
{
  iVisibilityCuller* culler = sector->GetVisibilityCuller ();
  csRef<iVisibilityObjectIterator> visit = culler->VisTest (box);
  csVector3 pos = box.GetCenter ();
  list.Push (csSectorPos (sector, pos));

  while (visit->HasNext ())
  {
    iVisibilityObject* vo = visit->Next ();
    iMeshWrapper* imw = vo->GetMeshWrapper ();
    if (imw)
    {
      if (imw->GetPortalContainer ())
      {
	iPortalContainer* portals = imw->GetPortalContainer ();
        int pc = portals->GetPortalCount ();
        int j;
        for (j = 0 ; j < pc ; j++)
        {
          iPortal* portal = portals->GetPortal (j);
          if (TestPortalBox (portal, box, pos, sector, visited_sectors))
          {
	    if (portal->GetFlags ().Check (CS_PORTAL_WARP))
	    {
	      csBox3 tbox = box;
	      tbox.SetCenter (WarpVector (portal, pos, imw));
	      GetNearbySectorList (portal->GetSector (), tbox,
		    	list, visited_sectors);
	    }
	    else
	    {
	      GetNearbySectorList (portal->GetSector (), box,
		    	list, visited_sectors);
	    }
          }
	}
      }
    }
  }
}

void csEngine::GetNearbySectorList (iSector* sector,
    const csVector3& pos, float radius, csArray<csSectorPos>& list,
    csSet<csPtrKey<iSector> >& visited_sectors)
{
  iVisibilityCuller* culler = sector->GetVisibilityCuller ();
  csRef<iVisibilityObjectIterator> visit = culler->VisTest (
  	csSphere (pos, radius));
  list.Push (csSectorPos (sector, pos));

  while (visit->HasNext ())
  {
    iVisibilityObject* vo = visit->Next ();
    iMeshWrapper* imw = vo->GetMeshWrapper ();
    if (imw)
    {
      if (imw->GetPortalContainer ())
      {
	iPortalContainer* portals = imw->GetPortalContainer ();
        int pc = portals->GetPortalCount ();
        int j;
        for (j = 0 ; j < pc ; j++)
        {
          iPortal* portal = portals->GetPortal (j);
	  if (TestPortalSphere (portal, radius, pos, sector, visited_sectors))
          {
	    GetNearbySectorList (portal->GetSector (), WarpVectorCond (portal,
	    	pos, imw), radius, list, visited_sectors);
          }
	}
      }
    }
  }
}

csPtr<iSectorIterator> csEngine::GetNearbySectors (
  iSector *sector,
  const csVector3 &pos,
  float radius)
{
  csArray<csSectorPos>* list = new csArray<csSectorPos>;
  csSet<csPtrKey<iSector> > visited_sectors;
  visited_sectors.Add (sector);
  GetNearbySectorList (sector, pos, radius, *list, visited_sectors);
  csSectorIt *it = new csSectorIt (list);
  return csPtr<iSectorIterator> (it);
}

csPtr<iSectorIterator> csEngine::GetNearbySectors (
  iSector *sector,
  const csBox3& box)
{
  csArray<csSectorPos>* list = new csArray<csSectorPos>;
  csSet<csPtrKey<iSector> > visited_sectors;
  visited_sectors.Add (sector);
  GetNearbySectorList (sector, box, *list, visited_sectors);
  csSectorIt *it = new csSectorIt (list);
  return csPtr<iSectorIterator> (it);
}

void csEngine::GetNearbyObjectList (iSector* sector,
    const csVector3& pos, float radius, csArray<iObject*>& list,
    csSet<csPtrKey<iSector> >& visited_sectors, bool crossPortals)
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
	  if (TestPortalSphere (portal, radius, pos, sector, visited_sectors))
          {
	    GetNearbyObjectList (portal->GetSector (), WarpVectorCond (portal,
	      	pos, imw), radius, list, visited_sectors, crossPortals);
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
  csSet<csPtrKey<iSector> > visited_sectors;
  visited_sectors.Add (sector);
  GetNearbyObjectList (sector, pos, radius, *list, visited_sectors,
  	crossPortals);
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

static void HandleStaticLOD (csMeshWrapper* cmesh, const csVector3& pos,
	csArray<iMeshWrapper*>& list)
{
  csStaticLODMesh* static_lod = cmesh->GetStaticLODMesh ();
  if (!static_lod) return;
  // We also need to add the child here that is at the right LOD
  // distance from the start of the segment.
  float distance = csQsqrt (cmesh->GetSquaredDistance (pos));
  float lod = static_lod->GetLODValue (distance);
  csArray<iMeshWrapper*>& meshes = static_lod->GetMeshesForLOD (lod);
  size_t i;
  // @@@ We assume here that there will be no portals as children.
  // This is perhaps a bad assumption.
  for (i = 0 ; i < meshes.GetSize () ; i++)
    list.Push (meshes[i]);
}

void csEngine::GetNearbyMeshList (iSector* sector,
    const csVector3& start, const csVector3& end,
    csArray<iMeshWrapper*>& list,
    csSet<csPtrKey<iSector> >& visited_sectors, bool crossPortals)
{
  iVisibilityCuller* culler = sector->GetVisibilityCuller ();
  csRef<iVisibilityObjectIterator> visit = culler->IntersectSegment (
  	start, end, true);

  //@@@@@@@@ TODO ALSO SUPPORT LIGHTS!
  while (visit->HasNext ())
  {
    iVisibilityObject* vo = visit->Next ();
    iMeshWrapper* imw = vo->GetMeshWrapper ();
    if (imw)
    {
      list.Push (imw); 
      csMeshWrapper* cmesh = (csMeshWrapper*)imw;
      HandleStaticLOD (cmesh, start, list);

      if (crossPortals && imw->GetPortalContainer ())
      {
        iPortalContainer* portals = imw->GetPortalContainer ();
        int pc = portals->GetPortalCount ();
        int j;
        for (j = 0 ; j < pc ; j++)
        {
          iPortal* portal = portals->GetPortal (j);
          const csPlane3& wor_plane = portal->GetWorldPlane ();
          // Can we see the portal?
          if (wor_plane.Classify (start) < -0.001)
          {
              // Also handle objects in the destination sector unless
              // it is a warping sector.
              portal->CompleteSector (0);
              if (sector != portal->GetSector () && portal->GetSector ())
              {
                if (!visited_sectors.In (portal->GetSector ()))
                {
                  visited_sectors.Add (portal->GetSector ());
		  if (portal->GetFlags ().Check (CS_PORTAL_WARP))
		  {
		    csReversibleTransform warp_wor;
		    portal->ObjectToWorld (
		    	imw->GetMovable ()->GetFullTransform (), warp_wor);
		    csVector3 tstart = warp_wor.Other2This (start);
		    csVector3 tend = warp_wor.Other2This (end);
                    GetNearbyMeshList (portal->GetSector (), tstart, tend,
		    	list, visited_sectors);
		  }
		  else
		  {
                    GetNearbyMeshList (portal->GetSector (), start, end,
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

void csEngine::GetNearbyMeshList (iSector* sector,
    const csVector3& pos, float radius, csArray<iMeshWrapper*>& list,
    csSet<csPtrKey<iSector> >& visited_sectors, bool crossPortals)
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
      csMeshWrapper* cmesh = (csMeshWrapper*)imw;
      HandleStaticLOD (cmesh, pos, list);

      if (crossPortals && imw->GetPortalContainer ())
      {
        iPortalContainer* portals = imw->GetPortalContainer ();
        int pc = portals->GetPortalCount ();
        for (int j = 0 ; j < pc ; j++)
        {
          iPortal* portal = portals->GetPortal (j);
	  if (TestPortalSphere (portal, radius, pos, sector, visited_sectors))
	  {
	    GetNearbyMeshList (portal->GetSector (), WarpVectorCond (portal,
	    	pos, imw), radius, list, visited_sectors);
	  }
	}
      }
    }
  }
}

void csEngine::GetNearbyMeshList (iSector* sector,
    const csBox3& box, csArray<iMeshWrapper*>& list,
    csSet<csPtrKey<iSector> >& visited_sectors, bool crossPortals)
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
      csMeshWrapper* cmesh = (csMeshWrapper*)imw;
      HandleStaticLOD (cmesh, pos, list);
      if (crossPortals && imw->GetPortalContainer ())
      {
        iPortalContainer* portals = imw->GetPortalContainer ();
        int pc = portals->GetPortalCount ();
        int j;
        for (j = 0 ; j < pc ; j++)
        {
          iPortal* portal = portals->GetPortal (j);
          if (TestPortalBox (portal, box, pos, sector, visited_sectors))
          {
	    if (portal->GetFlags ().Check (CS_PORTAL_WARP))
	    {
	      csBox3 tbox = box;
	      tbox.SetCenter (WarpVector (portal, pos, imw));
	      GetNearbyMeshList (portal->GetSector (), tbox,
		    	list, visited_sectors);
	    }
	    else
	    {
	      GetNearbyMeshList (portal->GetSector (), box,
		    	list, visited_sectors);
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
  csSet<csPtrKey<iSector> > visited_sectors;
  visited_sectors.Add (sector);
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
  csSet<csPtrKey<iSector> > visited_sectors;
  visited_sectors.Add (sector);
  GetNearbyMeshList (sector, box, *list, visited_sectors, crossPortals);
  csMeshListIt *it = new csMeshListIt (list);
  return csPtr<iMeshWrapperIterator> (it);
}

csPtr<iMeshWrapperIterator> csEngine::GetNearbyMeshes (
  iSector *sector,
  const csVector3& start,
  const csVector3& end,
  bool crossPortals)
{
  csArray<iMeshWrapper*>* list = new csArray<iMeshWrapper*>;
  csSet<csPtrKey<iSector> > visited_sectors;
  visited_sectors.Add (sector);
  GetNearbyMeshList (sector, start, end, *list, visited_sectors, crossPortals);
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
  if (!name)
    return 0;

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

iSector *csEngine::CreateSector (const char *name, bool addToList)
{
  csRef<iSector> sector;
  sector.AttachNew (new csSector (this));
  sector->QueryObject ()->SetName (name);

  if(addToList)
  {
    sectors.Add (sector);
  }
  else
  {
    sector->IncRef();
  }

  FireNewSector (sector);

  return sector;
}

iCollection* csEngine::CreateCollection(const char *name)
{
  csRef<iCollection> collection = collections.Get(name, NULL);
  if(!collection)
  {
    csRef<csCollection> collect;
    collect.AttachNew(new csCollection());
    collect->SetName(name);
    collection = collect;
    collections.Put(name, collection);
  }
  return collection;
}

void csEngine::RemoveCollection(iCollection* collect)
{
  collections.Delete(collect->QueryObject()->GetName(), collect);
}

void csEngine::RemoveCollection(const char *name)
{
  csRef<iCollection> collect = collections.Get(name, NULL);
  if(collect)
  {
    collections.Delete(name, collect);
  }
}

void csEngine::RemoveAllCollections()
{
  csArray<csRef<iCollection> > cols = collections.GetAll();
  for(size_t i=0; i<cols.GetSize(); i++)
  {
    RemoveCollection(cols[i]->QueryObject()->GetName());
  }
}

void csEngine::AddEngineFrameCallback (iEngineFrameCallback* cb)
{
  frameCallbacks.Push (cb);
}

void csEngine::RemoveEngineFrameCallback (iEngineFrameCallback* cb)
{
  frameCallbacks.Delete (cb);
}

void csEngine::FireStartFrame (iRenderView* rview)
{
  size_t i = frameCallbacks.GetSize ();
  while (i > 0)
  {
    i--;
    frameCallbacks[i]->StartFrame (this, rview);
  }
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
  size_t i = sectorCallbacks.GetSize ();
  while (i > 0)
  {
    i--;
    sectorCallbacks[i]->NewSector (this, sector);
  }
}

void csEngine::FireRemoveSector (iSector* sector)
{
  size_t i = sectorCallbacks.GetSize ();
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

  csRef<iMaterial> imat (scfQueryInterface<iMaterial> (mat));
  return csPtr<iMaterial> (imat);
}

iTextureList *csEngine::GetTextureList () const
{
  return GetTextures ();
}

iMaterialList *csEngine::GetMaterialList () const
{
  return GetMaterials ();
}

iSharedVariableList *csEngine::GetVariableList () const
{
  return GetVariables();
}

iCollection* csEngine::GetCollection(const char *name) const
{
  return collections.Get(name, NULL);
}

csPtr<iCollectionArray> csEngine::GetCollections()
{
  csRef<iCollectionArray> colScfArr;
  colScfArr.AttachNew(
    new scfArray<iCollectionArray, csArray<csRef<iCollection> > >(collections.GetAll()));
  return csPtr<iCollectionArray>(colScfArr);
}

csPtr<iCamera> csEngine::CreateCamera ()
{
  csCameraPerspective* cam = new csCameraPerspective ();
  return csPtr<iCamera> ((iCamera*)cam);
}

csPtr<iPerspectiveCamera> csEngine::CreatePerspectiveCamera ()
{
  csCameraPerspective* cam = new csCameraPerspective ();
  return csPtr<iPerspectiveCamera> (cam);
}

csPtr<iCustomMatrixCamera> csEngine::CreateCustomMatrixCamera (
  iCamera* copyFrom)
{
  csCameraCustomMatrix* cam;
  if (copyFrom)
    cam = new csCameraCustomMatrix (static_cast<csCameraBase*> (copyFrom));
  else
    cam = new csCameraCustomMatrix ();
  return csPtr<iCustomMatrixCamera> (cam);
}

csPtr<iLight> csEngine::CreateLight (
  const char *name,
  const csVector3 &pos,
  float radius,
  const csColor &color,
  csLightDynamicType dyntype)
{
  csLight *light = new csLight (this,
      pos.x, pos.y, pos.z,
      radius,
      color.red, color.green, color.blue,
      dyntype);
  if (name) light->SetName (name);

  return csPtr<iLight> (light);
}

csPtr<iMeshFactoryWrapper> csEngine::CreateMeshFactory (
  const char *classId, const char *name, bool addToList)
{
  // WARNING! In the past this routine checked if the factory
  // was already created. This is wrong! This routine should not do
  // this and instead allow duplicate factories (with the same name).
  // That's because that duplicate factory can still be in another
  // region. And even if this is not the case then factories with same
  // name are still allowed.
  csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
      objectRegistry, classId);
  if (!type) return 0;

  csRef<iMeshObjectFactory> fact = type->NewFactory ();
  if (!fact) return 0;

  // don't pass the name to avoid a second search
  csRef<iMeshFactoryWrapper> fwrap (CreateMeshFactory (fact, name, addToList));
  return csPtr<iMeshFactoryWrapper> (fwrap);
}

csPtr<iMeshFactoryWrapper> csEngine::CreateMeshFactory (
  iMeshObjectFactory *fact, const char *name, bool addToList)
{
  // WARNING! In the past this routine checked if the factory
  // was already created. This is wrong! This routine should not do
  // this and instead allow duplicate factories (with the same name).
  // That's because that duplicate factory can still be in another
  // region. And even if this is not the case then factories with same
  // name are still allowed.
  csMeshFactoryWrapper *mfactwrap = new csMeshFactoryWrapper (this, fact);
  if (name) mfactwrap->SetName (name);
  if(addToList)
  {
    GetMeshFactories ()->Add (mfactwrap);
  }
  fact->SetMeshFactoryWrapper ((iMeshFactoryWrapper*)mfactwrap);
  return csPtr<iMeshFactoryWrapper> (mfactwrap);
}

csPtr<iMeshFactoryWrapper> csEngine::CreateMeshFactory (const char *name, 
                                                        bool addToList)
{
  // WARNING! In the past this routine checked if the factory
  // was already created. This is wrong! This routine should not do
  // this and instead allow duplicate factories (with the same name).
  // That's because that duplicate factory can still be in another
  // region. And even if this is not the case then factories with same
  // name are still allowed.
  csMeshFactoryWrapper *mfactwrap = new csMeshFactoryWrapper (this);
  if (name) mfactwrap->SetName (name);
  if(addToList)
  {
    GetMeshFactories ()->Add (mfactwrap);
  }
  return csPtr<iMeshFactoryWrapper> (mfactwrap);
}

//------------------------------------------------------------------------

/*
 * Loader context class for the engine.
 */
class EngineLoaderContext : public scfImplementation1<EngineLoaderContext,
                                                     iLoaderContext>
{
private:
  csEngine* Engine;

  iCollection* collection;
  bool searchCollectionOnly;
  uint keepFlags;

public:
  EngineLoaderContext (csEngine* Engine, iCollection* collection, bool searchCollectionOnly);

  virtual ~EngineLoaderContext ();

  virtual iSector* FindSector (const char* name);
  virtual iMaterialWrapper* FindMaterial (const char* name, bool dontWaitForLoad = false);
  virtual iMaterialWrapper* FindNamedMaterial (const char* name,
  	const char* filename, bool dontWaitForLoad = false);
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name, bool dontWaitForLoad = false);
  virtual iMeshWrapper* FindMeshObject (const char* name);
  virtual iTextureWrapper* FindTexture (const char* name, bool dontWaitForLoad = false);
  virtual iTextureWrapper* FindNamedTexture (const char* name,
  	const char* filename, bool dontWaitForLoad = false);
  virtual iLight* FindLight (const char *name);
  virtual iShader* FindShader (const char* name);
  virtual iGeneralMeshSubMesh* FindSubmesh(iGeneralMeshState* state, const char* name)
  { return 0; }
  virtual bool CheckDupes () const { return false; }
  virtual iCollection* GetCollection () const { return collection; }
  virtual uint GetKeepFlags() const { return keepFlags; }
  virtual bool CurrentCollectionOnly() const { return searchCollectionOnly; }
  virtual void AddToCollection(iObject* obj);
  bool GetVerbose() { return false; }
};


EngineLoaderContext::EngineLoaderContext (csEngine* Engine,
	iCollection *collection, bool searchCollectionOnly)
  : scfImplementationType (this), Engine (Engine), collection(collection),
    searchCollectionOnly(searchCollectionOnly), keepFlags (0)
{
}

EngineLoaderContext::~EngineLoaderContext ()
{
}

iSector* EngineLoaderContext::FindSector (const char* name)
{
   return Engine->FindSector (name, searchCollectionOnly ? collection : 0);
}

iMaterialWrapper* EngineLoaderContext::FindMaterial (const char* name, bool dontWaitForLoad)
{
  return Engine->FindMaterial (name, searchCollectionOnly ? collection : 0);
}

iMaterialWrapper* EngineLoaderContext::FindNamedMaterial (const char* name,
                                                          const char* /*filename*/,
                                                          bool dontWaitForLoad)
{
  return Engine->FindMaterial (name, searchCollectionOnly ? collection : 0);
}

iMeshFactoryWrapper* EngineLoaderContext::FindMeshFactory (const char* name, bool dontWaitForLoad)
{
  return Engine->FindMeshFactory (name, searchCollectionOnly ? collection : 0);
}

iMeshWrapper* EngineLoaderContext::FindMeshObject (const char* name)
{
  return Engine->FindMeshObject (name, searchCollectionOnly ? collection : 0);
}

iTextureWrapper* EngineLoaderContext::FindTexture (const char* name, bool dontWaitForLoad)
{
  return Engine->FindTexture (name, searchCollectionOnly ? collection : 0);
}

iTextureWrapper* EngineLoaderContext::FindNamedTexture (const char* name,
                                                        const char* /*filename*/,
                                                        bool dontWaitForLoad)
{
  return Engine->FindTexture (name, searchCollectionOnly ? collection : 0);
}

iShader* EngineLoaderContext::FindShader (const char* name)
{
  if((!searchCollectionOnly || !collection)
    || (name && *name == '*')) // Always look up builtin shaders globally
    return Engine->shaderManager->GetShader (name);

  const csRefArray<iShader>& shaders = 
    Engine->shaderManager->GetShaders ();
  size_t i;
  for (i = 0 ; i < shaders.GetSize () ; i++)
  {
    iShader* s = shaders[i];
    if(collection)
    {
      if((collection->IsParentOf(s->QueryObject()) ||
        collection->FindShader(s->QueryObject()->GetName())) &&
        !strcmp (name, s->QueryObject ()->GetName ()))
      {
        return s;
      }
    }
  }
  return 0;
}

iLight* EngineLoaderContext::FindLight(const char *name)
{
  csRef<iLightIterator> li = Engine->GetLightIterator(searchCollectionOnly ? collection : 0);

  iLight *light;

  while (li->HasNext ())
  {
    light = li->Next ();
    if (!strcmp (light->QueryObject ()->GetName (),name))
      return light;
  }
  return 0;
}

void EngineLoaderContext::AddToCollection(iObject* obj)
{
  if(collection)
  {
    collection->Add(obj);
  }
}

//------------------------------------------------------------------------

csPtr<iLoaderContext> csEngine::CreateLoaderContext (iCollection* collection,
	bool searchCollectionOnly)
{
  return csPtr<iLoaderContext> (new EngineLoaderContext (this, collection,
  	searchCollectionOnly));
}

//------------------------------------------------------------------------

csPtr<iMeshFactoryWrapper> csEngine::LoadMeshFactory (const char *name,
  const char *loaderClassId, iDataBuffer *input, bool addToList)
{
  csRef<iDocumentSystem> xml (
    	csQueryRegistry<iDocumentSystem> (objectRegistry));
  if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> doc = xml->CreateDocument ();
  const char* error = doc->Parse (input, true);
  if (error != 0)
  {
    // @@@ Report error?
    return 0;
  }

  csRef<iLoaderPlugin> plug = csLoadPluginCheck<iLoaderPlugin> (
      objectRegistry, loaderClassId);
  if (!plug) return 0;

  csRef<iMeshFactoryWrapper> fact (CreateMeshFactory (name, addToList));
  if (!fact) return 0;

  csRef<iLoaderContext> elctxt (CreateLoaderContext (0, true));
  csRef<iBase> mof = plug->Parse (doc->GetRoot (),
  	0/*ssource*/, elctxt, fact->GetMeshObjectFactory ());
  if (!mof)
  {
    GetMeshFactories ()->Remove (fact);
    return 0;
  }

  csRef<iMeshObjectFactory> mof2 (
  	scfQueryInterface<iMeshObjectFactory> (mof));
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
    	csQueryRegistry<iDocumentSystem> (objectRegistry));
  if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> doc = xml->CreateDocument ();
  const char* error = doc->Parse (input, true);
  if (error != 0)
  {
    // @@@ Report error?
    return 0;
  }

  csRef<iLoaderPlugin> plug = csLoadPluginCheck<iLoaderPlugin> (
      objectRegistry, loaderClassId);
  if (!plug) return 0;

  csMeshWrapper *meshwrap = new csMeshWrapper (this);
  if (name) meshwrap->SetName (name);

  iMeshWrapper *imw = static_cast<iMeshWrapper*> (meshwrap);
  GetMeshes ()->Add (imw);
  imw->DecRef (); // the ref is now stored in the MeshList
  if (sector)
  {
    (meshwrap->GetCsMovable ()).csMovable::SetSector (sector);
    (meshwrap->GetCsMovable ()).csMovable::SetPosition (pos);
    (meshwrap->GetCsMovable ()).csMovable::UpdateMove ();
  }

  csRef<iLoaderContext> elctxt (CreateLoaderContext (0, true));
  csRef<iBase> mof = plug->Parse (doc->GetRoot (), 0/*ssource*/, elctxt, imw);
  if (!mof)
  {
    GetMeshes ()->Remove (imw);
    return 0;
  }

  csRef<iMeshObject> mof2 (scfQueryInterface<iMeshObject> (mof));
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
  csRef<iMeshWrapper> portal_mesh;
  csRef<iPortalContainer> pc;
  if (name)
  {
    const csRef<iSceneNodeArray> children = parentMesh->QuerySceneNode ()
      ->GetChildrenArray ();
    size_t i;
    for (i = 0 ; i < children->GetSize(); i++)
    {
      // @@@ Not efficient.
      iMeshWrapper* mesh = children->Get (i)->QueryMesh ();
      if (mesh)
      {
        if (!strcmp (name, mesh->QueryObject ()->GetName ()))
	{
	  pc = 
  	    scfQueryInterface<iPortalContainer> (mesh->GetMeshObject ());
          if (pc) portal_mesh = mesh;
	  break;
	}
      }
    }
  }
  if (!portal_mesh)
  {
    portal_mesh = CreatePortalContainer (name);
    portal_mesh->QuerySceneNode ()->SetParent (parentMesh->QuerySceneNode ());
    pc = 
  	scfQueryInterface<iPortalContainer> (portal_mesh->GetMeshObject ());
  }
  portal = pc->CreatePortal (vertices, num_vertices);
  portal->SetSector (destSector);
  return csPtr<iMeshWrapper> (portal_mesh);
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
      pc = 
  	scfQueryInterface<iPortalContainer> (mesh->GetMeshObject ());
      if (!pc) mesh = 0;
    }
  }
  if (!mesh)
  {
    mesh = CreatePortalContainer (name, sourceSector, pos);
    pc = 
  	scfQueryInterface<iPortalContainer> (mesh->GetMeshObject ());
  }
  portal = pc->CreatePortal (vertices, num_vertices);
  portal->SetSector (destSector);
  return csPtr<iMeshWrapper> (mesh);
}

csPtr<iMeshWrapper> csEngine::CreateMeshWrapper (
  iMeshFactoryWrapper *factory, const char *name,
  iSector *sector, const csVector3 &pos, bool addToList)
{
  csRef<iMeshWrapper> mesh = factory->CreateMeshWrapper ();
  if (name) mesh->QueryObject ()->SetName (name);
  if(addToList)
  {
    GetMeshes ()->Add (mesh);
  }
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
  iMeshObject *mesh, const char *name, iSector *sector,
  const csVector3 &pos, bool addToList)
{
  csMeshWrapper *meshwrap = new csMeshWrapper (this, mesh);
  if (name) meshwrap->SetName (name);
  if(addToList)
  {
    GetMeshes ()->Add ((iMeshWrapper*)meshwrap);
  }
  if (sector)
  {
    (meshwrap->GetCsMovable ()).csMovable::SetSector (sector);
    (meshwrap->GetCsMovable ()).csMovable::SetPosition (pos);
    (meshwrap->GetCsMovable ()).csMovable::UpdateMove ();
  }

  mesh->SetMeshWrapper ((iMeshWrapper*)meshwrap);
  return csPtr<iMeshWrapper> ((iMeshWrapper*)meshwrap);
}

csPtr<iMeshWrapper> csEngine::CreateMeshWrapper (const char *name,
                                                 bool addToList)
{
  csMeshWrapper *meshwrap = new csMeshWrapper (this);
  if (name) meshwrap->SetName (name);
  if(addToList)
  {
    GetMeshes ()->Add ((iMeshWrapper*)meshwrap);
  }
  return csPtr<iMeshWrapper> ((iMeshWrapper*)meshwrap);
}

csPtr<iMeshWrapper> csEngine::CreateMeshWrapper (
  const char *classId, const char *name, iSector *sector,
  const csVector3 &pos, bool addToList)
{
  csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
      objectRegistry, classId);
  if (!type) return 0;

  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  if (!fact) return 0;

  csRef<iMeshObject> mo (scfQueryInterface<iMeshObject> (fact));
  if (!mo)
  {
    // The factory is not itself a mesh object. Let's see if the
    // factory can return a working mesh object.
    mo = fact->NewInstance ();
    if (mo)
      return CreateMeshWrapper (mo, name, sector, pos, addToList);
    return 0;
  }

  return CreateMeshWrapper (mo, name, sector, pos, addToList);
}

static int CompareDelayedRemoveObject (csDelayedRemoveObject const& r1,
	csDelayedRemoveObject const& r2)
{
  // Reverse sort!
  if (r1.time_to_delete < r2.time_to_delete) return 1;
  else if (r2.time_to_delete < r1.time_to_delete) return -1;
  else return 0;
}

void csEngine::DelayedRemoveObject (csTicks delay, iBase *object)
{
  csDelayedRemoveObject ro;
  ro.object = object;
  ro.time_to_delete = virtualClock->GetCurrentTicks () + delay;
  delayedRemoves.InsertSorted (ro, CompareDelayedRemoveObject);
}

void csEngine::RemoveDelayedRemoves (bool remove)
{
  if (remove)
  {
    while (delayedRemoves.GetSize () > 0)
    {
      csDelayedRemoveObject ro = delayedRemoves.Pop ();
      RemoveObject (ro.object);
    }
  }
  else
  {
    delayedRemoves.DeleteAll ();
  }
}

bool csEngine::RemoveObject (iBase *object)
{
  csRef<iObject> obj = scfQueryInterface<iObject> (object);
  if (obj && obj->GetObjectParent ())
    obj->GetObjectParent ()->ObjRemove (obj);

  csRef<iSelfDestruct> sd = scfQueryInterface<iSelfDestruct> (object);
  if (sd)
  {
    sd->SelfDestruct ();
    return true;
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
  lightAmbientRed = int (c.red * 255.0f);
  lightAmbientGreen = int (c.green * 255.0f);
  lightAmbientBlue = int (c.blue * 255.0f);
}

void csEngine::GetAmbientLight (csColor &c) const
{
  c.red = lightAmbientRed / 255.0f;
  c.green = lightAmbientGreen / 255.0f;
  c.blue = lightAmbientBlue / 255.0f;
}

void csEngine::GetDefaultAmbientLight (csColor &c) const
{   
  c.red = lightAmbientRed / 255.0f;
  c.green = lightAmbientGreen / 255.0f;
  c.blue = lightAmbientBlue / 255.0f;
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

// ======================================================================
// Render loop stuff.
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
  { 1, "renderloop", "Override the default render loop", CSVAR_STRING },
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
      PerspectiveImpl::SetDefaultFOV ((float)value->GetLong (), G3D->GetWidth ());
      break;
    case 1:
      override_renderloop = value->GetString ();
      LoadDefaultRenderLoop (value->GetString ());
      break;
    default:
      return false;
  }

  return true;
}

bool csEngine::GetOption (int id, csVariant *value)
{
  switch (id)
  {
    case 0:
      value->SetLong ((long)PerspectiveImpl::GetDefaultFOV ());
      break;
    case 1:
      value->SetString ("");
      break; // @@@
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

