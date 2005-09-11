/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
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
#include "csutil/array.h"
#include "csutil/csobject.h"
#include "csutil/hash.h"
#include "csutil/nobjvec.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/stringarray.h"
#include "csutil/weakref.h"
#include "iengine/campos.h"
#include "iengine/collectn.h"
#include "iengine/engine.h"
#include "iengine/renderloop.h"
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
#include "ivaria/engseq.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"
#include "plugins/engine/3d/halo.h"
#include "plugins/engine/3d/meshobj.h"
#include "plugins/engine/3d/region.h"
#include "plugins/engine/3d/renderloop.h"
#include "plugins/engine/3d/rview.h"
#include "plugins/engine/3d/sharevar.h"

class csCamera;
class csEngine;
class csLight;
class csLightPatchPool;
class csMaterialList;
class csMeshWrapper;
class csPoly2DPool;
class csPolygon3D;
class csRegion;
class csRenderView;
class csSector;
class csSectorIt;
class csSectorList;
class csTextureList;
struct iClipper2D;
struct iConfigFile;
struct iLight;
struct iMaterialWrapper;
struct iObjectRegistry;
struct iProgressMeter;
struct iRegion;

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
  csLightIt (csEngine*, iRegion* region = 0);

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
  // The region we are iterating in (optional).
  iRegion* region;
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

/**
 * List of collections for the engine. This class implements iCollectionList.
 */
class csCollectionList : public scfImplementation1<csCollectionList,
                                                   iCollectionList>
{
public:
  /// constructor
  csCollectionList ();
  virtual ~csCollectionList ();

  //-- iCollectionList
  virtual iCollection* NewCollection (const char* name);
  virtual int GetCount () const;
  virtual iCollection *Get (int n) const;
  virtual int Add (iCollection *obj);
  virtual bool Remove (iCollection *obj);
  virtual bool Remove (int n);
  virtual void RemoveAll ();
  virtual int Find (iCollection *obj) const;
  virtual iCollection *FindByName (const char *Name) const;

private:
  csRefArrayObject<iCollection> collections;
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
  virtual int GetCount () const;
  virtual iCameraPosition *Get (int n) const;
  virtual int Add (iCameraPosition *obj);
  virtual bool Remove (iCameraPosition *obj);
  virtual bool Remove (int n);
  virtual void RemoveAll ();
  virtual int Find (iCameraPosition *obj) const;
  virtual iCameraPosition *FindByName (const char *Name) const;
private:
  csRefArrayObject<iCameraPosition> positions;
};

/**
 * A list of meshes for the engine.
 */
class csEngineMeshList : public csMeshList
{
public:
  virtual ~csEngineMeshList () { RemoveAll (); }
  virtual void FreeMesh (iMeshWrapper*);
};


/**
 * The 3D engine.
 * This class manages all components which comprise a 3D world including
 * sectors, polygons, curves, mesh objects, etc.
 */
class csEngine : public scfImplementationExt4<csEngine,
                                              csObject,
                                              iEngine,
                                              iComponent,
                                              iPluginConfig,
                                              iDebugHelper>
{
  // friends
  friend class csLight;
  friend class csLightIt;
  friend class csRenderLoop;
  friend class csSectorIt;
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
   * engine by the user (like Things, Sectors, ...) will be deleted as well.
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

  //-- Preparation and relighting methods
  virtual bool Prepare (iProgressMeter* meter = 0);

  virtual void PrepareTextures ();

  virtual void PrepareMeshes ();

  virtual void ForceRelight (iRegion* region = 0,
  	iProgressMeter* meter = 0);

  virtual void ForceRelight (iLight* light, iRegion* region = 0);

  virtual void ShineLights (iRegion* region = 0, 
    iProgressMeter* meter = 0);

  virtual void SetLightingCacheMode (int mode)
  { lightmapCacheMode = mode; }

  virtual int GetLightingCacheMode ()
  { return lightmapCacheMode; }

  virtual void SetCacheManager (iCacheManager* cache_mgr);
  virtual void SetVFSCacheManager (const char* vfspath = 0);

  virtual iCacheManager* GetCacheManager ();

  virtual void SetMaxLightmapSize (int w, int h)
  { maxLightmapWidth = w; maxLightmapHeight = h; }

  virtual void GetMaxLightmapSize (int& w, int& h)
  { w = maxLightmapWidth; h = maxLightmapHeight; }

  virtual void GetDefaultMaxLightmapSize (int& w, int& h)
  { w = defaultMaxLightmapWidth; h = defaultMaxLightmapHeight; }

  virtual int GetMaxLightmapAspectRatio () const
  { return maxAspectRatio; }


  //-- Render priority functions

  virtual void RegisterRenderPriority (const char* name, long priority,
  	csRenderPrioritySorting rendsort = CS_RENDPRI_SORT_NONE);

  virtual long GetRenderPriority (const char* name) const;

  virtual csRenderPrioritySorting GetRenderPrioritySorting (const char* name) const;
  
  virtual csRenderPrioritySorting GetRenderPrioritySorting (long priority) const;

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
  	iRegion* region = 0);

  //-- Texture handling

  virtual iTextureWrapper* CreateTexture (const char *name,
  	const char *fileName, csColor *transp, int flags);

  virtual iTextureWrapper* CreateBlackTexture (const char *name,
	int w, int h, csColor *transp, int flags);

  virtual int GetTextureFormat () const;

  virtual iTextureList* GetTextureList () const;

  virtual iTextureWrapper* FindTexture (const char* name,
  	iRegion* region = 0);
  
  //-- Light handling

  virtual csPtr<iLight> CreateLight (const char* name, const csVector3& pos,
  	float radius, const csColor& color,
	csLightDynamicType dyntype = CS_LIGHT_DYNAMICTYPE_STATIC);

  virtual iLight* FindLight (const char *Name, bool RegionOnly = false)
    const;

  virtual iLight* FindLightID (const char* light_id) const;

  virtual csPtr<iLightIterator> GetLightIterator (iRegion* region = 0)
  {
    return csPtr<iLightIterator> (new csLightIt (this, region));
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

  virtual iSector *CreateSector (const char *name);

  virtual iSectorList* GetSectors ()
  { return &sectors; }

  virtual iSector* FindSector (const char* name,
  	iRegion* region = 0);

  virtual csPtr<iSectorIterator> GetNearbySectors (iSector* sector,
  	const csVector3& pos, float radius);

  virtual void AddEngineSectorCallback (iEngineSectorCallback* cb);

  virtual void RemoveEngineSectorCallback (iEngineSectorCallback* cb);

  //-- Mesh handling

  virtual csPtr<iMeshWrapper> CreateMeshWrapper (iMeshFactoryWrapper* factory,
  	const char* name, iSector* sector = 0,
	const csVector3& pos = csVector3 (0, 0, 0));

  virtual csPtr<iMeshWrapper> CreateMeshWrapper (iMeshObject* meshobj,
  	const char* name, iSector* sector = 0,
	const csVector3& pos = csVector3 (0, 0, 0));

  virtual csPtr<iMeshWrapper> CreateMeshWrapper (const char* classid,
  	const char* name, iSector* sector = 0,
	const csVector3& pos = csVector3 (0, 0, 0));

  virtual csPtr<iMeshWrapper> CreateMeshWrapper (const char* name);

  virtual csPtr<iMeshWrapper> CreateSectorWallsMesh (iSector* sector,
      const char* name);

  virtual csPtr<iMeshWrapper> CreateThingMesh (iSector* sector,
  	const char* name);

  virtual csPtr<iMeshWrapper> LoadMeshWrapper (
  	const char* name, const char* loaderClassId,
	iDataBuffer* input, iSector* sector, const csVector3& pos);

  virtual void AddMeshAndChildren (iMeshWrapper* mesh);

  virtual csPtr<iMeshWrapperIterator> GetNearbyMeshes (iSector* sector,
    const csVector3& pos, float radius, bool crossPortals = true );

  virtual csPtr<iMeshWrapperIterator> GetNearbyMeshes (iSector* sector,
    const csBox3& box, bool crossPortals = true );

  virtual iMeshList* GetMeshes ()
  { return &meshes; }

  virtual iMeshWrapper* FindMeshObject (const char* name,
  	iRegion* region = 0);

  virtual void WantToDie (iMeshWrapper* mesh);

  //-- Mesh factory handling

  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (const char* classId,
  	const char* name);

  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (
  	iMeshObjectFactory * factory, const char* name);

  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (const char* name);

  virtual csPtr<iMeshFactoryWrapper> LoadMeshFactory (
  	const char* name, const char* loaderClassId,
	iDataBuffer* input);

  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name,
  	iRegion* region = 0);

  virtual iMeshFactoryList* GetMeshFactories ()
  { return &meshFactories; }

  //-- Region handling
  
  virtual iRegion* CreateRegion (const char* name);
  
  virtual iRegionList* GetRegions ();

  //-- Camera handling

  virtual csPtr<iCamera> CreateCamera ();

  virtual iCameraPosition* FindCameraPosition (const char* name,
  	iRegion* region = 0);

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
  { clearZBuf = yesno; }

  virtual bool GetClearZBuf () const
  { return clearZBuf;}

  virtual bool GetDefaultClearZBuf () const 
  { return defaultClearZBuf; }

  virtual void SetClearScreen (bool yesno)
  { clearScreen = yesno; }

  virtual bool GetClearScreen () const
  { return clearScreen; }

  virtual bool GetDefaultClearScreen () const
  { return defaultClearScreen; }

  virtual int GetBeginDrawFlags () const
  {
    int flag = 0;
    if (clearScreen) flag |= CSDRAW_CLEARSCREEN;
    if (clearZBuf) flag |= CSDRAW_CLEARZBUFFER;
    return flag;
  }

  virtual iRenderView* GetTopLevelClipper () const
  { return (iRenderView*)topLevelClipper; }

  virtual void PrecacheDraw (iRegion* region = 0);

  virtual void Draw (iCamera* c, iClipper2D* clipper);

  virtual void SetContext (iTextureHandle* ctxt);
  
  virtual iTextureHandle *GetContext () const;

  virtual iRenderLoopManager* GetRenderLoopManager ();
  
  virtual iRenderLoop* GetCurrentDefaultRenderloop ();

  virtual bool SetCurrentDefaultRenderloop (iRenderLoop* loop);

  virtual uint GetCurrentFrameNumber () const
  { return currentFrameNumber; }

  //-- Saving/loading

  virtual void SetSaveableFlag (bool enable)
  { worldSaveable = enable; }

  virtual bool GetSaveableFlag ()
  { return worldSaveable; }
  
  virtual csPtr<iLoaderContext> CreateLoaderContext (
  	iRegion* region = 0, bool curRegOnly = true);
  
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

  virtual csPtr<iFrustumView> CreateFrustumView ();

  virtual csPtr<iObjectWatcher> CreateObjectWatcher ();

  virtual iSharedVariableList* GetVariableList () const;

  virtual iCollectionList* GetCollections ()
  { return &collections; }
  
  virtual iCollection* FindCollection (const char* name,
  	iRegion* region = 0);

  virtual bool RemoveObject (iBase* object);

  virtual void DeleteAll ();

  virtual void ResetWorldSpecificSettings(); 

  // -- iPluginConfig
  virtual bool GetOptionDescription (int idx, csOptionDescription *option);

  virtual bool SetOption (int id, csVariant* value);

  virtual bool GetOption (int id, csVariant* value);

  bool HandleEvent (iEvent &Event);

  // -- iEventHandler
  struct eiEventHandler : public scfImplementation1<
  	eiEventHandler,iEventHandler>
  {
    csWeakRef<csEngine> parent;
    eiEventHandler (csEngine* parent) : scfImplementationType (this)
    {
      eiEventHandler::parent = parent;
    }
    virtual ~eiEventHandler ()
    {
    }
    virtual bool HandleEvent (iEvent& ev)
    {
      if (parent) return parent->HandleEvent (ev);
      else return false;
    }
  } * scfiEventHandler;

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
   * Get/create the engine sequence manager.
   * \param create
   * Should a sequence manager be created if one does not exist.
   */
  iEngineSequenceManager* GetEngineSequenceManager (bool create = true);
 
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

  // Sector iteration functions
  csSectorIt* AllocSectorIterator (iSector *sector, const csVector3 &pos, 
    float radius);
  void RecycleSectorIterator (csSectorIt* iterator);
  void FreeSectorIteratorPool ();

  // Renderloop loading/creation
  csPtr<iRenderLoop> CreateDefaultRenderLoop ();
  void LoadDefaultRenderLoop (const char* fileName);

  /**
   * Setup for starting a Draw or DrawFunc.
   */
  void StartDraw (iCamera* c, iClipper2D* view, csRenderView& rview);

  /**
   * Controll animation and delete meshes that want to die.
   */
  void ControlMeshes ();

  /**
   * Get a list of all objects in the given sphere.
   */
  void GetNearbyObjectList (iSector* sector,
    const csVector3& pos, float radius, csArray<iObject*>& list,
    csArray<iSector*>& visited_sectors, bool crossPortals = true);

  /**
   * Get a list of all meshes in the given box.
   */
  void GetNearbyMeshList (iSector* sector,
    const csBox3& box, csArray<iMeshWrapper*>& list,
    csArray<iSector*>& visited_sectors, bool crossPortals = true);

  /**
   * Get a list of all meshes in the given sphere.
   */
  void GetNearbyMeshList (iSector* sector,
    const csVector3& pos, float radius, csArray<iMeshWrapper*>& list,
    csArray<iSector*>& visited_sectors, bool crossPortals = true);

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
  void AddHalo (iCamera* camera, csLight* Light);

  /**
   * Remove halo attached to given light from the engine.
   */
  void RemoveHalo (csLight* Light);

  //Sector event helpers
  void FireNewSector (iSector* sector);
  void FireRemoveSector (iSector* sector);

  /**
   * Split a name into optional 'region/name' format.
   * This function returns the pointer to the real name of the object.
   * The 'region' variable will contain the region is one is given.
   * If a region was given but none could be found this function returns
   * 0 (this is an error).<br>
   * If '*' was given as a region name then all regions are searched EVEN if
   * the the FindXxx() routine is called for a specific region only. i.e.
   * this forces the search to be global. In this case 'global' will be set
   * to true.
   */
  char* SplitRegionName (const char* name, iRegion*& region, bool& global);

  // Precache a single mesh
  void PrecacheMesh (iMeshWrapper* s, iRenderView* rview);

  iMeshObjectType* GetThingType ();

public:
  // -- PUBLIC MEMBERS. THESE ARE FOR CONVENIANCE WITHIN ENGINE PLUGIN

  /// For debugging purposes.
  csRef<iBugPlug> bugplug;

  /// The shared engine instance.
  static csEngine* currentEngine;
  /// The shared engine instance.
  static iEngine* currentiEngine;
  /// Remember iObjectRegistry.
  static iObjectRegistry* objectRegistry;
  /// The global material/shader string set
  csRef<iStringSet> globalStringSet;
  /// The 3D driver
  csRef<iGraphics3D> G3D;
  /// Pointer to the shader manager
  csRef<iShaderManager> shaderManager;
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
  static int frameWidth, frameHeight;

private:

  // -- PRIVATE MEMBERS

  // -- Object lists
  /**
   * List of sectors in the engine. This vector contains
   * objects of type iSector*. Use CreateSector()
   * to add sectors to the engine.
   */
  csSectorList sectors;

  /**
   * List of all collections in the engine. This vector contains objects
   * of type iCollection*.
   */
  csCollectionList collections;

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
  /// The list of all regions currently loaded.
  csRegionList regions;

  /// Sector callbacks.
  csRefArray<iEngineSectorCallback> sectorCallbacks;

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

  // Sector iterator pool
  csSectorIt* sectorItPool; //TOOD: REWORK THIS! UGLY

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

  /**
   * Pointer to the engine sequence manager.
   */
  csWeakRef<iEngineSequenceManager> sequenceManager;

  /// Store virtual clock to speed up time queries.
  csRef<iVirtualClock> virtualClock;

  /// Default render loop
  csRef<iRenderLoop> defaultRenderLoop;
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
  csRenderView* topLevelClipper;
    
  /// Flag set when window requires resizing.
  bool resize;

  /// Default shader to attach to all materials
  csRef<iShader> defaultShader;

  /// 'Saveable' flag
  bool worldSaveable;

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
   * <ul>
   * <li>"sky": usually rendered using ZFILL or ZNONE
   * <li>"portal": usually for portal containers
   * <li>"wall": usually rendered using ZFILL
   * <li>"object": usually rendered using ZUSE
   * <li>"alpha": usually rendered using ZTEST
   * </ul>
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

    /// Option variable: force lightmap recalculation?
  static int lightmapCacheMode;
  /// Maximum lightmap dimensions
  static int maxLightmapWidth;
  static int maxLightmapHeight;

  /// Clear the Z-buffer every frame.
  bool clearZBuf;

  /// default buffer clear flag.
  bool defaultClearZBuf;

  /// Clear the screen every frame.
  bool clearScreen;

  /// default buffer clear flag.
  bool defaultClearScreen;

  /// default maximum lightmap width/height
  int defaultMaxLightmapWidth, defaultMaxLightmapHeight;

  /// default ambient color
  int defaultAmbientRed, defaultAmbientGreen, defaultAmbientBlue;
  
  /// Verbose flag.
  static bool doVerbose;

  /// Thing mesh object type for convenience.
  csRef<iMeshObjectType> thingMeshType;
  
  /// Current render context (proc texture) or 0 if global.
  iTextureHandle* currentRenderContext; 
};

#endif // __CS_ENGINE_H__
