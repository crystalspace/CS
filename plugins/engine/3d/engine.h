/*
    Copyright (C) 1998-2001,2007 by Jorrit Tyberghein
              (C) 2005 by Marten Svanfeldt

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

#ifndef __CS_ENGINE_H__
#define __CS_ENGINE_H__

#include "csgeom/math3d.h"
#include "csplugincommon/rendermanager/renderview.h"
#include "csutil/array.h"
#include "csutil/cfgacc.h"
#include "csutil/csobject.h"
#include "csutil/hash.h"
#include "csutil/set.h"
#include "csutil/nobjvec.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/stringarray.h"
#include "csutil/threading/rwmutex.h"
#include "csutil/weakref.h"
#include "csutil/weakrefarr.h"
#include "csutil/eventnames.h"
#include "csutil/eventhandlers.h"
#include "iengine/campos.h"
#include "iengine/engine.h"
#include "iengine/renderloop.h"
#include "iengine/rendermanager.h"
#include "igraphic/imageio.h"
#include "iutil/cache.h"
#include "iutil/comp.h"
#include "iutil/dbghelp.h"
#include "iutil/eventh.h"
#include "iutil/pluginconfig.h"
#include "iutil/string.h"
#include "iutil/strset.h"
#include "iutil/vfs.h"
#include "iutil/virtclk.h"
#include "ivaria/bugplug.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "plugins/engine/3d/collection.h"
#include "plugins/engine/3d/halo.h"
#include "plugins/engine/3d/meshobj.h"
#include "plugins/engine/3d/meshfact.h"
#include "plugins/engine/3d/renderloop.h"
#include "plugins/engine/3d/sector.h"
#include "plugins/engine/3d/sharevar.h"

#include "reflectomotron3000.h"

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{
  class csLight;
  class csMeshWrapper;
}
CS_PLUGIN_NAMESPACE_END(Engine)

class csEngine;
class csLightPatchPool;
class csMaterialList;
class csPolygon3D;
class csSector;
class csSectorList;
class csTextureList;
struct iClipper2D;
struct iConfigFile;
struct iDocumentSystem;
struct iLight;
struct iMaterialWrapper;
struct iObjectRegistry;
struct iProgressMeter;

/**
 * Iterator to iterate over all static lights in the engine.
 * This iterator assumes there are no fundamental changes
 * in the engine while it is being used.
 * If changes to the engine happen the results are unpredictable.
 */
class csLightIt : public scfImplementation1<csLightIt,
                                            iLightIterator>
{
public:
  /// Construct an iterator and initialize to start.
  csLightIt (csEngine*, iCollection* collection = 0);

  virtual ~csLightIt ();

  /// Restart iterator.
  virtual void Reset ();

  /// Return true if there are more elements.
  virtual bool HasNext ();

  /// Get light from iterator. Return 0 at end.
  virtual iLight* Next ();

  /// Get the sector for the last fetched light.
  virtual iSector* GetLastSector ();
private:
  // The engine for this iterator.
  csEngine* engine;
  // The collection we are iterating in (optional).
  iCollection* collection;
  // Current sector index.
  int sectorIndex;
  // Current light index.
  int lightIndex;
  // Get current light.
  iLight* currentLight;

  // Go to next sector. Return false if finished.
  bool NextSector ();

  /// Get light from iterator. Return 0 at end.
  iLight* FetchNext ();
};

struct csSectorPos
{
  iSector* sector;
  csVector3 pos;
  csSectorPos (iSector* sector, const csVector3& pos) :
  	sector (sector), pos (pos) { }
};

struct csDelayedRemoveObject
{
  csRef<iBase> object;
  csTicks time_to_delete;
};

/**
 * List of camera positions for the engine. This class implements
 * iCameraPositionList.
 */
class csCameraPositionList : public scfImplementation1<csCameraPositionList,
                                                       iCameraPositionList>
{
public:
  /// constructor
  csCameraPositionList ();
  virtual ~csCameraPositionList ();
  //-- iCameraPositionList
  virtual iCameraPosition* NewCameraPosition (const char* name);
  virtual csPtr<iCameraPosition> CreateCameraPosition (const char* name);

  virtual int GetCount () const;
  virtual iCameraPosition *Get (int n) const;
  virtual int Add (iCameraPosition *obj);
  void AddBatch (csRef<iCamposLoaderIterator> itr);
  virtual bool Remove (iCameraPosition *obj);
  virtual bool Remove (int n);
  virtual void RemoveAll ();
  virtual int Find (iCameraPosition *obj) const;
  virtual iCameraPosition *FindByName (const char *Name) const;
private:
  csRefArrayObject<iCameraPosition> positions;
  mutable CS::Threading::ReadWriteMutex camLock;
};

/**
 * A list of meshes for the engine.
 */
class csEngineMeshList : public CS_PLUGIN_NAMESPACE_NAME(Engine)::csMeshList
{
public:
  csEngineMeshList () : csMeshList (256, 256) { }
  virtual ~csEngineMeshList () { RemoveAll (); }
  virtual void FreeMesh (iMeshWrapper*);
};

/**
 * An averaging variable. Stores the last 'bufsize' values in a circular
 * buffer. Computes the average with method Average.
 */
template<typename T, int bufsize>
class AverageVar
{
  T buf[bufsize];
  int start, length;
public:
  AverageVar(): start(0), length(0) {}
  void Add(T val)
  {
    buf[(start+length)%bufsize] = val;
    length++;
    if (length > bufsize)
    {
      length--;
      start = (start+1)%bufsize;
    }
  }
  T Average(int n) const
  {
    CS_ASSERT(n >= 1 && n <= bufsize);
    T accum = 0;
    int m = (n < length) ? n : length;
    for (int i = 1; i <= m; i++)
    {
      accum += buf[(start+length-i)%bufsize];
    }
    return accum / m;
  }
};


#include "csutil/deprecated_warn_off.h"


using namespace CS_PLUGIN_NAMESPACE_NAME(Engine);

/**
 * The 3D engine.
 * This class manages all components which comprise a 3D world including
 * sectors, polygons, curves, mesh objects, etc.
 */
class csEngine : public ThreadedCallable<csEngine>,
  public scfImplementationExt5<csEngine, csObject,
  iEngine, iComponent, iPluginConfig, iDebugHelper, iEventHandler>
{
  // friends
  friend class CS_PLUGIN_NAMESPACE_NAME(Engine)::csLight;
  friend class csLightIt;
  friend class csRenderLoop;
  friend class csSectorList;

public:

  //-- Constructor & destructor

  /**
   * Initialize an empty engine.  The only thing that is valid just after
   * creating the engine is the configurator object which you can use to
   * configure the engine before continuing (see GetEngineConfig()).
   */
  csEngine (iBase *iParent);

  /**
   * Delete the engine and all entities it contains.  All objects added to this
   * engine by the user (like meshes, sectors, ...) will be deleted as well.
   * If you don't want this then you should unlink them manually before
   * destroying the engine.
   */
  virtual ~csEngine ();

  // -- INTERFACE IMPLEMENTATIONS

  // -- iComponent
  virtual bool Initialize (iObjectRegistry* object_reg);

  // -- iEngine
  
  /// Get the iObject for the engine.
  virtual iObject *QueryObject();

  inline iObjectRegistry* GetObjectRegistry() const
  {
    return objectRegistry;
  }

  //-- Preparation and relighting methods
  virtual bool Prepare (iProgressMeter* meter = 0);
  virtual void PrepareTextures ();
  virtual void PrepareMeshes ();

  virtual void SetCacheManager (iCacheManager* cache_mgr);
  virtual void SetVFSCacheManager (const char* vfspath = 0);

  virtual iCacheManager* GetCacheManager ();

  //-- Render priority functions

  virtual void RegisterRenderPriority (const char* name, long priority,
  	csRenderPrioritySorting rendsort = CS_RENDPRI_SORT_NONE);
  virtual void RegisterDefaultRenderPriorities ();
  virtual long GetRenderPriority (const char* name) const;
  virtual csRenderPrioritySorting GetRenderPrioritySorting (
  	const char* name) const;
  virtual csRenderPrioritySorting GetRenderPrioritySorting (
  	long priority) const;

  virtual long GetSkyRenderPriority ()
  {
    UpdateStandardRenderPriorities ();
    return renderPrioritySky;
  }
  
  virtual long GetPortalRenderPriority ()
  {
    UpdateStandardRenderPriorities ();
    return renderPriorityPortal;
  }

  virtual long GetWallRenderPriority ()
  {
    UpdateStandardRenderPriorities ();
    return renderPriorityWall;
  }

  virtual long GetObjectRenderPriority ()
  {
    UpdateStandardRenderPriorities ();
    return renderPriorityObject;
  }

  virtual long GetAlphaRenderPriority ()
  {
    UpdateStandardRenderPriorities ();
    return renderPriorityAlpha;
  }

  virtual void ClearRenderPriorities ();
  virtual int GetRenderPriorityCount () const;
  virtual const char* GetRenderPriorityName (long priority) const;

  //-- Material handling

  virtual csPtr<iMaterial> CreateBaseMaterial (iTextureWrapper* txt);
  virtual iMaterialWrapper* CreateMaterial (const char *name,
  	iTextureWrapper* texture);
  virtual iMaterialList* GetMaterialList () const;
  virtual iMaterialWrapper* FindMaterial (const char* name,
  	iCollection* collection = 0);

  //-- Texture handling

  virtual iTextureWrapper* CreateTexture (const char *name,
  	const char *fileName, csColor *transp, int flags);
  virtual iTextureWrapper* CreateBlackTexture (const char *name,
	int w, int h, csColor *transp, int flags);
  virtual int GetTextureFormat () const;
  virtual iTextureList* GetTextureList () const;
  virtual iTextureWrapper* FindTexture (const char* name,
  	iCollection* collection = 0);
  
  //-- Light handling

  virtual csPtr<iLight> CreateLight (const char* name, const csVector3& pos,
  	float radius, const csColor& color,
	csLightDynamicType dyntype = CS_LIGHT_DYNAMICTYPE_STATIC);
  virtual iLight* FindLight (const char *Name, bool RegionOnly = false)
    const;
  virtual iLight* FindLightID (const char* light_id) const;

  virtual csPtr<iLightIterator> GetLightIterator (iCollection* collection = 0)
  {
    return csPtr<iLightIterator> (new csLightIt (this, collection));
  }

  virtual void RemoveLight (iLight* light);

  virtual void SetAmbientLight (const csColor &c);
  virtual void GetAmbientLight (csColor &c) const;
  virtual void GetDefaultAmbientLight (csColor &c) const;

  virtual int GetNearbyLights (iSector* sector, const csVector3& pos,
  	iLight** lights, int max_num_lights);
  virtual int GetNearbyLights (iSector* sector, const csBox3& box,
  	iLight** lights, int max_num_lights);
  
  //-- Sector handling

  virtual iSector *CreateSector (const char *name, bool addToList);
  virtual iSectorList* GetSectors ()
  { return &sectors; }
  virtual iSector* FindSector (const char* name,
  	iCollection* collection);
  virtual csPtr<iSectorIterator> GetNearbySectors (iSector* sector,
  	const csVector3& pos, float radius);
  virtual csPtr<iSectorIterator> GetNearbySectors (iSector* sector,
  	const csBox3& box);

  virtual void AddEngineSectorCallback (iEngineSectorCallback* cb);
  virtual void RemoveEngineSectorCallback (iEngineSectorCallback* cb);
  virtual void AddEngineFrameCallback (iEngineFrameCallback* cb);
  virtual void RemoveEngineFrameCallback (iEngineFrameCallback* cb);

  //-- Mesh handling

  virtual csPtr<iMeshWrapper> CreateMeshWrapper (iMeshFactoryWrapper* factory,
  	const char* name, iSector* sector = 0, const csVector3& pos = csVector3 (0, 0, 0),
    bool addToList = true);

  virtual csPtr<iMeshWrapper> CreateMeshWrapper (iMeshObject* meshobj,
  	const char* name, iSector* sector = 0, const csVector3& pos = csVector3 (0, 0, 0),
    bool addToList = true);

  virtual csPtr<iMeshWrapper> CreateMeshWrapper (const char* classid,	const char* name,
    iSector* sector = 0, const csVector3& pos = csVector3 (0, 0, 0), bool addToList = true);

  virtual csPtr<iMeshWrapper> CreateMeshWrapper (const char* name, bool addToList = true);

  virtual csPtr<iMeshWrapper> LoadMeshWrapper (
  	const char* name, const char* loaderClassId,
	iDataBuffer* input, iSector* sector, const csVector3& pos);

  THREADED_CALLABLE_DECL1(csEngine, AddMeshAndChildren, csThreadReturn, iMeshWrapper*, mesh,
    MED, false, false);

  virtual csPtr<iMeshWrapperIterator> GetNearbyMeshes (iSector* sector,
    const csVector3& pos, float radius, bool crossPortals = true );
  virtual csPtr<iMeshWrapperIterator> GetNearbyMeshes (iSector* sector,
    const csBox3& box, bool crossPortals = true );
  virtual csPtr<iMeshWrapperIterator> GetNearbyMeshes (iSector* sector,
    const csVector3& start, const csVector3& end, bool crossPortals = true );

  virtual iMeshList* GetMeshes ()
  { return &meshes; }

  virtual iMeshWrapper* FindMeshObject (const char* name,
  	iCollection* collection = 0);

  virtual void WantToDie (iMeshWrapper* mesh);

  //-- Mesh factory handling

  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (const char* classId,
  	const char* name, bool addToList);

  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (iMeshObjectFactory * factory,
    const char* name, bool addToList);

  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (const char* name,
    bool addToList);

  virtual csPtr<iMeshFactoryWrapper> LoadMeshFactory (
  	const char* name, const char* loaderClassId, iDataBuffer* input, bool addToList);

  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name,
  	iCollection* collection = 0);

  virtual iMeshFactoryList* GetMeshFactories ()
  { return &meshFactories; }

  // -- Collection handling

  virtual iCollection* CreateCollection(const char* name);

  virtual iCollection* GetCollection(const char* name) const;

  virtual csPtr<iCollectionArray> GetCollections();

  virtual void RemoveCollection(iCollection* collect);

  virtual void RemoveCollection(const char* name);

  virtual void RemoveAllCollections();

  //-- Camera handling

  virtual csPtr<iCamera> CreateCamera ();
  virtual csPtr<iPerspectiveCamera> CreatePerspectiveCamera ();
  virtual csPtr<iCustomMatrixCamera> CreateCustomMatrixCamera (iCamera* copyFrom = 0);

  virtual iCameraPosition* FindCameraPosition (const char* name,
  	iCollection* collection = 0);

  virtual iCameraPositionList* GetCameraPositions ()
  { return &cameraPositions; }

  //-- Portal handling
  
  virtual csPtr<iMeshWrapper> CreatePortal (
  	const char* name,
  	iMeshWrapper* parentMesh, iSector* destSector,
	csVector3* vertices, int num_vertices,
	iPortal*& portal);

  virtual csPtr<iMeshWrapper> CreatePortal (
  	const char* name,
  	iSector* sourceSector, const csVector3& pos,
	iSector* destSector,
	csVector3* vertices, int num_vertices,
	iPortal*& portal);

  virtual csPtr<iMeshWrapper> CreatePortalContainer (const char* name,
  	iSector* sector = 0, const csVector3& pos = csVector3 (0, 0, 0));


  //-- Drawing related

  virtual void SetClearZBuf (bool yesno)
  {}
  virtual bool GetClearZBuf () const
  { return false; }
  virtual bool GetDefaultClearZBuf () const 
  { return false; }

  virtual void SetClearScreen (bool yesno)
  { }
  virtual bool GetClearScreen () const
  { return true; }
  virtual bool GetDefaultClearScreen () const
  { return true; }

  virtual int GetBeginDrawFlags () const
  {
    return 0;
  }

  virtual iRenderView* GetTopLevelClipper () const
  { return (iRenderView*)topLevelClipper; }

  virtual void PrecacheMesh (iMeshWrapper* s);
  virtual void PrecacheDraw (iCollection* collection = 0);
  virtual void Draw (iCamera* c, iClipper2D* clipper, iMeshWrapper* mesh = 0);

  virtual void SetContext (iTextureHandle* ctxt);
  virtual iTextureHandle *GetContext () const;

  virtual iRenderLoopManager* GetRenderLoopManager ();
  virtual iRenderLoop* GetCurrentDefaultRenderloop ();
  virtual bool SetCurrentDefaultRenderloop (iRenderLoop* loop);

  virtual uint GetCurrentFrameNumber () const
  { return currentFrameNumber; }
  virtual void UpdateNewFrame ()
  { 
    currentFrameNumber++; 
    envTexHolder.NextFrame ();
    ControlMeshes ();
  }

  //-- Adaptive LODs

  virtual void EnableAdaptiveLODs(bool enable, float target_fps)
  { bAdaptiveLODsEnabled = enable; adaptiveLODsTargetFPS = target_fps; }
  virtual void UpdateAdaptiveLODs();
  virtual float GetAdaptiveLODsMultiplier() const
  { return (bAdaptiveLODsEnabled ? adaptiveLODsMultiplier : 1.0f); }

  //-- Saving/loading

  virtual void SetSaveableFlag (bool enable)
  { worldSaveable = enable; }

  virtual bool GetSaveableFlag ()
  { return worldSaveable; }
  
  virtual csPtr<iLoaderContext> CreateLoaderContext (
  	iCollection* collection = 0, bool searchCollectionOnly = true);
  
  virtual void SetDefaultKeepImage (bool enable) 
  { defaultKeepImage = enable; }
  virtual bool GetDefaultKeepImage ()
  { return defaultKeepImage; }
  
  //-- Other
  
  virtual csPtr<iObjectIterator> GetNearbyObjects (iSector* sector,
    const csVector3& pos, float radius, bool crossPortals = true );
  
  virtual csPtr<iObjectIterator> GetVisibleObjects (iSector* sector,
    const csVector3& pos);

  virtual csPtr<iMeshWrapperIterator> GetVisibleMeshes (iSector* sector,
    const csVector3& pos);

  virtual csPtr<iObjectIterator> GetVisibleObjects (iSector* sector,
    const csFrustum& frustum);

  virtual csPtr<iMeshWrapperIterator> GetVisibleMeshes (iSector* sector,
    const csFrustum& frustum);

  virtual csPtr<iObjectWatcher> CreateObjectWatcher ();

  virtual iSharedVariableList* GetVariableList () const;

  virtual bool RemoveObject (iBase* object);
  virtual void DelayedRemoveObject (csTicks delay, iBase *object);
  virtual void RemoveDelayedRemoves (bool remove = false);

  THREADED_CALLABLE_DECL(csEngine, DeleteAll, csThreadReturn, HIGH, true, false);
  void DeleteAllForce ();

  virtual void ResetWorldSpecificSettings(); 

  // -- iPluginConfig
  virtual bool GetOptionDescription (int idx, csOptionDescription *option);

  virtual bool SetOption (int id, csVariant* value);
  virtual bool GetOption (int id, csVariant* value);

  // -- iEventHandler
  bool HandleEvent (iEvent &Event);

  CS_EVENTHANDLER_PHASE_LOGIC("crystalspace.engine.3d")

  // -- iDebugHelper
  
  virtual int GetSupportedTests () const
  { return 0; }

  virtual csPtr<iString> UnitTest ()
  { return 0; }

  virtual csPtr<iString> StateTest ()
  { return 0; }

  virtual csTicks Benchmark (int /*num_iterations*/)
  { return 0; }
 
  virtual csPtr<iString> Dump ()
  { return 0; }

  virtual void Dump (iGraphics3D* /*g3d*/)
  { }

  virtual bool DebugCommand (const char* cmd);


  // -- OTHER PUBLIC METHODS

  /**
   * Report a notification message.
   */
  void Report (const char* description, ...) CS_GNUC_PRINTF (2, 3);

  /**
   * Report a warning.
   */
  void Warn (const char* description, ...) CS_GNUC_PRINTF (2, 3);

  /**
   * Report an error.
   */
  void Error (const char* description, ...) CS_GNUC_PRINTF (2, 3);

  /**
   * Report a bug.
   */
  void ReportBug (const char* description, ...) CS_GNUC_PRINTF (2, 3);

  /**
   * Get the last animation time.
   */
  csTicks GetLastAnimationTime () const 
  { return nextframePending; }

  /**
   * Return the object managing all loaded textures.
   */
  csTextureList* GetTextures () const { return textures; }

  /**
   * Return the object managing all loaded materials.
   */
  csMaterialList* GetMaterials () const { return materials; }

  /**
   * Return the object managing all shared variables.
   */
  csSharedVariableList* GetVariables () const { return sharedVariables; }

  /// Get the renderloop manager.
  csRenderLoopManager* GetRenderLoopManager () const
  {
    return renderLoopManager;
  }

  iMaterialWrapper* GetDefaultPortalMaterial () const
  { return defaultPortalMaterial; }

  /**
   * Sync engine lists with loader lists.
   */
  THREADED_CALLABLE_DECL1(csEngine, SyncEngineLists, csThreadReturn, csRef<iThreadedLoader>,
    loader, LOW, false, true);

  void SyncEngineListsNow(csRef<iThreadedLoader> loader)
  {
    csRef<iThreadReturn> itr;
    itr.AttachNew(new csThreadReturn(tman));
    SyncEngineListsTC(itr, false, loader);
  }

private:
  // -- PRIVATE METHODS

 /**
   * Check consistency of the loaded elements which comprise the world.
   * Currently this function only checks if polygons have three or more
   * vertices and if the vertices are coplanar (if more than three).  This
   * function prints out warnings for all found errors.  Returns true if
   * everything is in order.
   */
  bool CheckConsistency ();

  // Renderloop loading/creation
  csPtr<iRenderLoop> CreateDefaultRenderLoop ();
  void LoadDefaultRenderLoop (const char* fileName);
  csRef<iShader> LoadShader (iDocumentSystem* docsys, const char* filename);

  /**
   * Setup for starting a Draw or DrawFunc.
   */
  void StartDraw (iCamera* c, iClipper2D* view,
    CS::RenderManager::RenderView& rview);

  /**
   * Controll animation and delete meshes that want to die.
   */
  void ControlMeshes ();

  void GetNearbySectorList (iSector* sector,
    const csVector3& pos, float radius, csArray<csSectorPos>& list,
    csSet<csPtrKey<iSector> >& visited_sectors);
  void GetNearbySectorList (iSector* sector,
    const csBox3& box, csArray<csSectorPos>& list,
    csSet<csPtrKey<iSector> >& visited_sectors);
  void GetNearbyObjectList (iSector* sector,
    const csVector3& pos, float radius, csArray<iObject*>& list,
    csSet<csPtrKey<iSector> >& visited_sectors, bool crossPortals = true);
  void GetNearbyMeshList (iSector* sector,
    const csBox3& box, csArray<iMeshWrapper*>& list,
    csSet<csPtrKey<iSector> >& visited_sectors, bool crossPortals = true);
  void GetNearbyMeshList (iSector* sector,
    const csVector3& pos, float radius, csArray<iMeshWrapper*>& list,
    csSet<csPtrKey<iSector> >& visited_sectors, bool crossPortals = true);
  void GetNearbyMeshList (iSector* sector,
    const csVector3& start, const csVector3& end, csArray<iMeshWrapper*>& list,
    csSet<csPtrKey<iSector> >& visited_sectors, bool crossPortals = true);

  /**
   * Read configuration file (using the system driver) for all engine
   * specific values. This function is called by Initialize() so you
   * normally do not need to call it yourselves.
   */
  void ReadConfig (iConfigFile*);

  /**
   * Prepare for creation of a world. This function is called
   * by Initialize() so you normally do not need to call it
   * yourselves.
   */
  void StartEngine ();

  /// Update the standard render priorities if needed.
  void UpdateStandardRenderPriorities ();

  /// Resizes frame width and height dependent data members
  void Resize ();

  /**
   * Add a halo attached to given light to the engine.
   */
  void AddHalo (iCamera* camera,
    CS_PLUGIN_NAMESPACE_NAME(Engine)::csLight* Light);

  /**
   * Remove halo attached to given light from the engine.
   */
  void RemoveHalo (CS_PLUGIN_NAMESPACE_NAME(Engine)::csLight* Light);

  //Sector event helpers
  void FireNewSector (iSector* sector);
  void FireRemoveSector (iSector* sector);
  void FireStartFrame (iRenderView* rview);

  /**
   * Split a name into optional 'collection/name' format.
   * This function returns the pointer to the real name of the object.
   * The 'collection' variable will contain the collection is one is given.
   * If a collection was given but none could be found this function returns
   * 0 (this is an error).<br>
   * If '*' was given as a collection name then all collections are searched EVEN if
   * the the FindXxx() routine is called for a specific collection only. i.e.
   * this forces the search to be global. In this case 'global' will be set
   * to true.
   */
  const char* SplitCollectionName(const char* name, iCollection*& collection, bool& global);

  // Precache a single mesh
  void PrecacheMesh (iMeshWrapper* s, iRenderView* rview);

  iRenderManager* GetRenderManager () { return renderManager; }
  void SetRenderManager (iRenderManager*);
  void ReloadRenderManager (csConfigAccess& cfg);
  void ReloadRenderManager ();
public:
  // -- PUBLIC MEMBERS. THESE ARE FOR CONVENIANCE WITHIN ENGINE PLUGIN

  /// For debugging purposes.
  csRef<iBugPlug> bugplug;

  /// Remember iObjectRegistry.
  iObjectRegistry* objectRegistry;
  /// The global string set
  csRef<iStringSet> globalStringSet;
  /// The shader variable name string set
  csRef<iShaderVarStringSet> svNameStringSet;
  /// The 3D driver
  csRef<iGraphics3D> G3D;
  /// Pointer to the shader manager
  csRef<iShaderManager> shaderManager;
  
  /// Store virtual clock to speed up time queries.
  csRef<iVirtualClock> virtualClock;

  /// Store engine shadervar names
  CS::ShaderVarStringID id_creation_time;
  CS::ShaderVarStringID id_lod_fade;
  CS::ShaderVarStringID svTexEnvironmentName;

  csRef<iRenderManager> renderManager;
  EnvTex::Holder envTexHolder;
  bool enableEnvTex;

  /// For triangle meshes.
  csStringID colldet_id;
  csStringID viscull_id;
  csStringID base_id;

  /**
   * This is the Virtual File System object where all the files
   * used by the engine live. Textures, models, data, everything -
   * reside on this virtual disk volume. You should avoid using
   * the standard file functions (such as fopen(), fread() and so on)
   * since they are highly system-dependent (for example, DOS uses
   * '\' as path separator, Mac uses ':' and Unix uses '/').
   */
  csRef<iVFS> VFS;

  /// Remember dimensions of display.
  int frameWidth, frameHeight;

  /** 
   * Config value: light ambient red value.
   */
  int lightAmbientRed;
  /**
   * Config value: light ambient green value.
   */
  int lightAmbientGreen;
  /** 
   * Config value: light ambient blue value.
   */
  int lightAmbientBlue;

  /// Default shader to attach to all materials
  // \todo move back to private and make accessible
  csRef<iShader> defaultShader;

  /// Shader variable names for light SVs
  csLightShaderVarCache lightSvNames;
  
  /// Get the shader attenuation texture SV
  csShaderVariable* GetLightAttenuationTextureSV();
private:

  // -- PRIVATE MEMBERS

  /// Pool from which to allocate render views.
  CS::RenderManager::RenderView::Pool rviewPool;

  // -- Object lists
  /**
   * List of sectors in the engine. This vector contains
   * objects of type iSector*. Use CreateSector()
   * to add sectors to the engine.
   */
  csSectorList sectors;

  /**
   * List of mesh object factories. This vector contains objects of
   * type csMeshFactoryWrapper*.
   */
  csMeshFactoryList meshFactories;

  /**
   * List of all meshes in the engine. This vector contains objects
   * of type csMeshWrapper*. Use RemoveMesh() to remove meshes from this
   * list. This function will take care of correctly removing the meshes
   * from all sectors as well. Note that after you add a mesh to the list
   * you still need to add it to all sectors that you want it to be visible in.
   */
  csEngineMeshList meshes;

  /**
   * The list of all camera position objects.
   */
  csCameraPositionList cameraPositions;
  /// Texture and color information objects.
  csTextureList* textures;
  /// Material objects.
  csMaterialList* materials;
  /// The list of all shared variables in the engine.
  csSharedVariableList* sharedVariables;
  /// List of halos (csHaloInformation).
  csPDelArray<csLightHalo> halos;
  /// The hash of all collections currently existing.
  csHash<csRef<iCollection>, csString> collections;

  /// Sector callbacks.
  csRefArray<iEngineSectorCallback> sectorCallbacks;
  /// Frame callbacks.
  csRefArray<iEngineFrameCallback> frameCallbacks;

  /**
   * This is a vector which holds objects of type 'csCleanable'.
   * They will be destroyed when the engine is destroyed. That's
   * the only special thing. This is useful for holding memory
   * which you allocate locally in a function but you want
   * to reuse accross function invocations. There is no general
   * way to make sure that the memory will be freed it only exists
   * as a static pointer in your function code. Adding a class
   * encapsulating that memory to this array will ensure that the
   * memory is removed once the engine is destroyed.
   */
  csRefArray<iBase> cleanupList;

  /// Array of objects that want to die next frame (iMeshWrapper*).
  csSet<csPtrKey<iMeshWrapper> > wantToDieSet;

  /// An array of objects to remove at a specific time.
  csArray<csDelayedRemoveObject> delayedRemoves;

  /// The list of all named render priorities.
  csStringArray renderPriorities;
  /// Sorting flags for the render priorities.
  csArray<csRenderPrioritySorting> renderPrioritySortflag;

  // - Pointers to other plugins

  /**
   * Pointer to an optional reporter that will be used for notification
   * and warning messages.
   */
  csRef<iReporter> reporter;

  /// Default render loop
  csRef<iRenderLoop> defaultRenderLoop;
  csString override_renderloop;
  /// Render loop manager
  csRenderLoopManager* renderLoopManager;
  /// The graphics loader
  csRef<iImageIO> imageLoader;
  /// The 2D driver
  csRef<iGraphics2D> G2D;

  /**
   * The following variable is only set if the engine had to create its
   * own cache manager. In that case the engine is also responsible
   * for cleaning this up.
   */
  csRef<iCacheManager> cacheManager;

  // -- Internal settings
  /**
   * The top-level clipper we are currently using for drawing.
   */
  CS::RenderManager::RenderView* topLevelClipper;
    
  /// Flag set when window requires resizing.
  bool resize;

  /// 'Saveable' flag
  bool worldSaveable;
  
  /// Default 'keep image' flag
  bool defaultKeepImage;

  /// Maximum texture aspect ratio
  int maxAspectRatio;

  /**
   * If the following flag is dirty then the render_priority_xxx values are
   * potentially invalid and need to be recalculated.
   */
  bool renderPrioritiesDirty;
  /**
   * The engine knows about the following render priorities and keeps
   * them here:
   * - "sky": usually rendered using ZFILL or ZNONE
   * - "portal": usually for portal containers
   * - "wall": usually rendered using ZFILL
   * - "object": usually rendered using ZUSE
   * - "alpha": usually rendered using ZTEST
   */
  long renderPrioritySky;
  long renderPriorityPortal;
  long renderPriorityWall;
  long renderPriorityObject;
  long renderPriorityAlpha;

  /**
   * If this nextframe_pending is not 0 then a call of NextFrame
   * has happened. As soon as some object is visible (DrawTest() returns
   * true) we will really call NextFrame() if its locally remembered
   * last-anim value is different from this one. This should improve
   * global speed of the engine as this means that invisible particle
   * systems will now not be updated anymore until they are really visible.
   */
  csTicks nextframePending; 

  /// Store the current framenumber. Is incremented every Draw ()
  uint currentFrameNumber;

  /// default ambient color
  int defaultAmbientRed, defaultAmbientGreen, defaultAmbientBlue;
  
  /// Verbose flag.
  static bool doVerbose;

  /// Current render context (proc texture) or 0 if global.
  iTextureHandle* currentRenderContext;
  
  /// Default portal material
  csRef<iMaterialWrapper> defaultPortalMaterial;
  
  csRef<csShaderVariable> lightAttenuationTexture;

  CS_DECLARE_SYSTEM_EVENT_SHORTCUTS;
  csEventID CanvasResize;
  csEventID CanvasClose;
  csRef<iEventHandler> weakEventHandler;

  /// Pointer to the thread manager.
  csWeakRef<iThreadManager> tman;

  /// To precache or not to precache....
  bool precache;

  /// Average the elapsed time of the last 30 frames.
  AverageVar<float, 30> virtualClockBuffer;

  /// Whether to use adaptive LODs
  bool bAdaptiveLODsEnabled;

  /// User-specified target FPS for adaptive LODs
  float adaptiveLODsTargetFPS;

  /// If using adaptive LODs, this stores the computed adaptive LOD multiplier
  float adaptiveLODsMultiplier;

};

#include "csutil/deprecated_warn_on.h"

#endif // __CS_ENGINE_H__
