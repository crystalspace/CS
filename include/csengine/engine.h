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

#ifndef __CS_ENGINE_H__
#define __CS_ENGINE_H__

#include "csutil/scf.h"
#include "csutil/nobjvec.h"
#include "csutil/csobject.h"
#include "csutil/garray.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/config.h"
#include "csgeom/math3d.h"
#include "csengine/arrays.h"
#include "csengine/rview.h"
#include "csengine/thing.h"
#include "csengine/meshobj.h"
#include "csengine/region.h"
#include "iengine/engine.h"
#include "iengine/collectn.h"
#include "iengine/campos.h"
#include "ivideo/graph3d.h"

class csRegion;
class csRadiosity;
class csSector;
class csMeshWrapper;
class csTextureList;
class csMaterialList;
class csPolygon3D;
class csCamera;
class csStatLight;
class csDynLight;
class csCBufferCube;
class csEngine;
class csLight;
class csCBuffer;
class csXORBuffer;
class csPoly2DPool;
class csLightPatchPool;
class csLightHalo;
class csRenderView;
struct iVFS;
struct iMaterialWrapper;
struct iRegion;
struct iLight;
struct iImageIO;
struct iClipper2D;
struct iReporter;
struct iProgressMeter;
struct iObjectRegistry;
struct iVirtualClock;
struct iCacheManager;

SCF_DECLARE_FAST_INTERFACE (iEngine)
SCF_DECLARE_FAST_INTERFACE (iSector)
SCF_DECLARE_FAST_INTERFACE (iMeshWrapper)
SCF_DECLARE_FAST_INTERFACE (iCollection)
SCF_DECLARE_FAST_INTERFACE (iMeshFactoryWrapper)
SCF_DECLARE_FAST_INTERFACE (iCurveTemplate)
SCF_DECLARE_FAST_INTERFACE (iMaterialWrapper)
SCF_DECLARE_FAST_INTERFACE (iTextureWrapper)
SCF_DECLARE_FAST_INTERFACE (iCameraPosition)
SCF_DECLARE_FAST_INTERFACE (iPolyTxtPlane)
SCF_DECLARE_FAST_INTERFACE (iStatLight)
SCF_DECLARE_FAST_INTERFACE (iDynLight)
SCF_DECLARE_FAST_INTERFACE (iLightingInfo)
SCF_DECLARE_FAST_INTERFACE (iLight)
SCF_DECLARE_FAST_INTERFACE (iMeshObject)
SCF_DECLARE_FAST_INTERFACE (iMaterial)
SCF_DECLARE_FAST_INTERFACE (iCrossHalo)
SCF_DECLARE_FAST_INTERFACE (iNovaHalo)
SCF_DECLARE_FAST_INTERFACE (iFlareHalo)
SCF_DECLARE_FAST_INTERFACE (iThingState)
SCF_DECLARE_FAST_INTERFACE (iShadowCaster)
SCF_DECLARE_FAST_INTERFACE (iPolygon3D)
SCF_DECLARE_FAST_INTERFACE (iMeshObjectFactory)
SCF_DECLARE_FAST_INTERFACE (iReferencedObject)
SCF_DECLARE_FAST_INTERFACE (iFile)
SCF_DECLARE_FAST_INTERFACE (iVisibilityObject)
SCF_DECLARE_FAST_INTERFACE (iVisibilityCuller)

SCF_DECLARE_FAST_INTERFACE (csRadPoly)
SCF_DECLARE_FAST_INTERFACE (csRadCurve)

/**
 * Iterator to iterate over all static lights in the engine.
 * This iterator assumes there are no fundamental changes
 * in the engine while it is being used.
 * If changes to the engine happen the results are unpredictable.
 */
class csLightIt
{
private:
  // The engine for this iterator.
  csEngine* engine;
  // The region we are iterating in (optional).
  iRegion* region;
  // Current sector index.
  int sector_idx;
  // Current light index.
  int light_idx;

  // Go to next sector. Return false if finished.
  bool NextSector ();

public:
  /// Construct an iterator and initialize to start.
  csLightIt (csEngine*, iRegion* region = NULL);

  /// Restart iterator.
  void Restart ();

  /// Get light from iterator. Return NULL at end.
  csLight* Fetch ();

  /// Get the sector for the last fetched light.
  csSector* GetLastSector ();
};

CS_DECLARE_OBJECT_VECTOR (csCollectionListHelper, iCollection);

/**
 * List of collections for the engine. This class implements iCollectionList.
 */
class csCollectionList : public csCollectionListHelper
{
public:
  SCF_DECLARE_IBASE;

  /// constructor
  csCollectionList ();
  /// Create a new collection.
  virtual iCollection* NewCollection (const char* name);

  class CollectionList : public iCollectionList
  {
  public:
    SCF_DECLARE_EMBEDDED_IBASE (csCollectionList);
    virtual iCollection* NewCollection (const char* name);
    virtual int GetCount () const;
    virtual iCollection *Get (int n) const;
    virtual int Add (iCollection *obj);
    virtual bool Remove (iCollection *obj);
    virtual bool Remove (int n);
    virtual void RemoveAll ();
    virtual int Find (iCollection *obj) const;
    virtual iCollection *FindByName (const char *Name) const;
  } scfiCollectionList;
};

CS_DECLARE_OBJECT_VECTOR (csCameraPositionListHelper, iCameraPosition);

/**
 * List of camera positions for the engine. This class implements
 * iCameraPositionList.
 */
class csCameraPositionList : public csCameraPositionListHelper
{
public:
  SCF_DECLARE_IBASE;

  /// constructor
  csCameraPositionList ();
  /// New camera position.
  virtual iCameraPosition* NewCameraPosition (const char* name);

  class CameraPositionList : public iCameraPositionList
  {
    SCF_DECLARE_EMBEDDED_IBASE (csCameraPositionList);
    virtual iCameraPosition* NewCameraPosition (const char* name);
    virtual int GetCount () const;
    virtual iCameraPosition *Get (int n) const;
    virtual int Add (iCameraPosition *obj);
    virtual bool Remove (iCameraPosition *obj);
    virtual bool Remove (int n);
    virtual void RemoveAll ();
    virtual int Find (iCameraPosition *obj) const;
    virtual iCameraPosition *FindByName (const char *Name) const;
  } scfiCameraPositionList;
};

/**
 * A list of meshes for the engine.
 */
class csEngineMeshList : public csMeshList
{
public:
  virtual ~csEngineMeshList ();
  virtual bool FreeItem (csSome Item);
};

/**
 * Object which implements iConfig interface on behalf of csEngine.
 * This class is used as an embedded SCF object within csEngine.  Typically,
 * this class would be declared as an inner class of csEngine, but the NextStep
 * compiler was unable to grok that usage after csObject (from which csEngine
 * inherits) was changed so that it inherits from iObject.  Somehow, the
 * compiler was getting confused by the QueryInterface(), IncRef(), and
 * DecRef() methods declared here as well as in iEngine and csObject (both of
 * which csEngine inherits from).  Making csEngineConfig stand-alone works
 * around the problem.
 */
struct csEngineConfig : public iConfig
{
  SCF_DECLARE_EMBEDDED_IBASE (csEngine);
  virtual bool GetOptionDescription (int idx, csOptionDescription *option);
  virtual bool SetOption (int id, csVariant* value);
  virtual bool GetOption (int id, csVariant* value);
};

/**
 * The 3D engine.
 * This class manages all components which comprise a 3D world including
 * sectors, polygons, curves, mesh objects, etc.
 */
class csEngine : public iEngine
{
public:
  /**
   * This is the Virtual File System object where all the files
   * used by the engine live. Textures, models, data, everything -
   * reside on this virtual disk volume. You should avoid using
   * the standard file functions (such as fopen(), fread() and so on)
   * since they are highly system-dependent (for example, DOS uses
   * '\' as path separator, Mac uses ':' and Unix uses '/').
   */
  iVFS *VFS;

  /**
   * Pointer to an optional reporter that will be used for notification
   * and warning messages.
   */
  iReporter* Reporter;

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
  csObjVector cleanup;

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
  csMeshFactoryList mesh_factories;

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
  csCameraPositionList camera_positions;

  /// Remember dimensions of display.
  static int frame_width, frame_height;
  /// Remember iObjectRegistry.
  static iObjectRegistry* object_reg;
  /// The shared engine instance.
  static csEngine* current_engine;
  /// The shared engine instance.
  static iEngine* current_iengine;
  /// Need to render using newradiosity?
  static bool use_new_radiosity;
  /// An object pool for 2D polygons used by the rendering process.
  csPoly2DPool* render_pol2d_pool;
  /// An object pool for lightpatches.
  csLightPatchPool* lightpatch_pool;
  /// The 3D driver
  iGraphics3D* G3D;
  /// The 2D driver
  iGraphics2D* G2D;
  /// The graphics loader
  iImageIO* ImageLoader;
  /// The fog mode this G3D implements
  G3D_FOGMETHOD fogmethod;
  /// Does the 3D driver require power-of-two lightmaps?
  bool NeedPO2Maps;
  /// Maximum texture aspect ratio
  int MaxAspectRatio;
  /// A pointer to the current region.
  iRegion* region;
  /// The list of all regions currently loaded.
  csRegionList regions;

  /// The list of all named render priorities.
  csVector render_priorities;
  /// Sorting flags for the render priorities.
  CS_DECLARE_GROWING_ARRAY (render_priority_sortflags, int);
  /**
   * The engine knows about the following render priorities and keeps
   * them here:
   * <ul>
   * <li>"sky": usually rendered using ZFILL or ZNONE
   * <li>"wall": usually rendered using ZFILL
   * <li>"object": usually rendered using ZUSE
   * <li>"alpha": usually rendered using ZTEST
   * </ul>
   */
  long render_priority_sky;
  long render_priority_wall;
  long render_priority_object;
  long render_priority_alpha;

  /// Option variable: force lightmap recalculation?
  static int lightcache_mode;
  /// Option variable: quality for lightmap calculation.
  static int lightmap_quality;
  /// Option variable: force visibility recalculation?
  static bool do_force_revis;
  /// Option variable: radiosity debugging (step by step)?
  static bool do_rad_debug;
  /// Maximum lightmap dimensions
  static int max_lightmap_w;
  static int max_lightmap_h;

private:
  /// Texture and color information objects.
  csTextureList* textures;
  /// Material objects.
  csMaterialList* materials;
  /// Linked list of dynamic lights.
  csDynLight* first_dyn_lights;
  /// List of halos (csHaloInformation).
  csHaloArray halos;
  /// Debugging: maximum number of polygons to process in one frame.
  static int max_process_polygons;
  /// Current number of processed polygons.
  static int cur_process_polygons;

  /// Current engine mode (one of CS_ENGINE_... flags).
  int engine_mode;

  /// Pointer to radiosity system if we are in step-by-step radiosity mode.
  csRadiosity* rad_debug;

  /// Optional c-buffer used for rendering.
  csCBuffer* c_buffer;
  /// Optional XOR-buffer used for rendering.
  csXORBuffer* xor_buffer;
  bool use_xorbuf;

  /// C-buffer cube used for lighting.
  csCBufferCube* cbufcube;

  /// Use PVS.
  bool use_pvs;

  /**
   * Use PVS only. If this flag is true (and use_pvs == true)
   * then no other culling mechanisms will be used.
   */
  bool use_pvs_only;

  /**
   * Freeze the PVS.
   * If this flag is true then the PVS will be 'frozen'.
   * The freeze_pvs_pos will be used as the fixed position
   * to calculate the PVS from.
   */
  bool freeze_pvs;
  /// Frozen PVS position.
  csVector3 freeze_pvs_pos;

  /// Clear the Z-buffer every frame.
  bool clear_zbuf;

  /// default buffer clear flag
  bool default_clear_zbuf;

  /// default lightmap cell size
  int default_lightmap_cell_size;

  /// default maximum lightmap width/height
  int default_max_lightmap_w, default_max_lightmap_h;

  /**
   * If this nextframe_pending is not 0 then a call of NextFrame
   * has happened. As soon as some object is visible (DrawTest() returns
   * true) we will really call NextFrame() if its locally remembered
   * last-anim value is different from this one. This should improve
   * global speed of the engine as this means that invisible particle
   * systems will now not be updated anymore until they are really visible.
   */
  csTicks nextframe_pending;

  /// Store virtual clock to speed up time queries.
  iVirtualClock* virtual_clock;

private:
  /**
   * Resolve the engine mode if it is CS_ENGINE_AUTODETECT.
   */
  void ResolveEngineMode ();

  /**
   * Setup for starting a Draw or DrawFunc.
   */
  void StartDraw (iCamera* c, iClipper2D* view, csRenderView& rview);

  /**
   * Controll animation and delete meshes that want to die.
   */
  void ControlMeshes ();

  /**
   * Split a name into optional 'region/name' format.
   * This function returns the pointer to the real name of the object.
   * The 'region' variable will contain the region is one is given.
   * If a region was given but none could be found this function returns
   * NULL (this is an error).<br>
   * If '*' was given as a region name then all regions are searched EVEN if
   * the the FindXxx() routine is called for a specific region only. i.e.
   * this forces the search to be global. In this case 'global' will be set
   * to true.
   */
  char* SplitRegionName (const char* name, iRegion*& region, bool& global);

  /**
   * The following variable is only set if the engine had to create its
   * own cache manager. In that case the engine is also responsible
   * for cleaning this up.
   */
  iCacheManager* cache_mgr;

public:
  /**
   * The current camera for drawing the world.
   */
  iCamera* current_camera;

  /**
   * The top-level clipper we are currently using for drawing.
   */
  iClipper2D* top_clipper;

public:
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

  /**
   * Report a notification message.
   */
  void Report (const char* description, ...) CS_GNUC_PRINTF (2, 3);

  /**
   * Report a warning.
   */
  void Warn (const char* description, ...) CS_GNUC_PRINTF (2, 3);

  /**
   * Report a bug.
   */
  void ReportBug (const char* description, ...) CS_GNUC_PRINTF (2, 3);

  /**
   * Check consistency of the loaded elements which comprise the world.
   * Currently this function only checks if polygons have three or more
   * vertices and if the vertices are coplanar (if more than three).  This
   * function prints out warnings for all found errors.  Returns true if
   * everything is in order.
   */
  bool CheckConsistency ();

  /**
   * Prepare the textures. It will initialise all loaded textures
   * for the texture manager. (Normally you shouldn't call this function
   * directly, because it will be called by Prepare() for you.
   * This function will also prepare all loaded materials after preparing
   * the textures.
   */
  virtual void PrepareTextures ();

  /**
   * Calls UpdateMove for all meshes to initialise bsp bounding boxes.
   * Call this after creating a BSP tree. csEngine::Prepare() will call
   * this function automatically so you normally don't have to call it.
   */
  virtual void PrepareMeshes ();

  /**
   * Calculate all lighting information. Normally you shouldn't call
   * this function directly, because it will be called by Prepare().
   * If the optional 'region' parameter is given then only lights will
   * be recalculated for the given region.
   */
  virtual void ShineLights (iRegion* region = NULL,
  	iProgressMeter* meter = NULL);

  /// Query the iObject for the engine.
  virtual iObject *QueryObject();
  /// Query the csObject for the engine.
  inline csObject *QueryCsObject () { return &scfiObject; }

  /**
   * Prepare the engine. This function must be called after
   * you loaded/created the world. It will prepare all lightmaps
   * for use and also free all images that were loaded for
   * the texture manager (the texture manager should have them
   * locally now). The optional progress meter will be used to
   * report progress.
   */
  virtual bool Prepare (iProgressMeter* meter = NULL);

  /**
   * Set the maximum number of polygons to process in
   * one frame. This is mainly useful for debugging.
   */
  static void SetMaxProcessPolygons (int m) { max_process_polygons = m; }

  /**
   * Get the maximum number of polygons to process in one frame.
   */
  static int GetMaxProcessPolygons () { return max_process_polygons; }

  /**
   * Indicate that we will process another polygon. Returns false
   * if we need to stop.
   */
  static bool ProcessPolygon ()
  {
    if (cur_process_polygons > max_process_polygons) return false;
    cur_process_polygons++;
    return true;
  }

  /**
   * Return true if we are processing the last polygon.
   */
  static bool ProcessLastPolygon ()
  {
    return cur_process_polygons >= max_process_polygons;
  }

  /**
   * Get the pointer to the radiosity object (used with step-by-step
   * debugging).
   */
  csRadiosity* GetRadiosity () const { return rad_debug; }

  /**
   * Invalidate all lightmaps. This can be called after doing
   * a significant change on the static lightmaps (i.e. after doing
   * a radiosity debug function).
   */
  void InvalidateLightmaps ();

  /**
   * Set the desired engine mode.
   * One of the CS_ENGINE_... flags. Default is CS_ENGINE_AUTODETECT.
   * If you select CS_ENGINE_AUTODETECT then the mode will be
   * auto-detected (depending on level and/or hardware capabilities)
   * the first time csEngine::Draw() is called.
   */
  virtual void SetEngineMode (int mode)
  {
    engine_mode = mode;
  }

  /**
   * Get the current engine mode.
   * If called between SetEngineMode() and the first Draw() it is
   * possible that this mode will still be CS_ENGINE_AUTODETECT.
   */
  virtual int GetEngineMode () const { return engine_mode; }

  /**
   * Get the required flags for 3D->BeginDraw() which should be called
   * from the application. These flags must be or-ed with optional other
   * flags that the application might be interested in.
   */
  virtual int GetBeginDrawFlags () const
  {
    if (clear_zbuf || engine_mode == CS_ENGINE_ZBUFFER)
      return CSDRAW_CLEARZBUFFER;
    else
      return 0;
  }

  /**
   * Get the last animation time.
   */
  csTicks GetLastAnimationTime () const { return nextframe_pending; }

  /**
   * Initialize the culler.
   */
  void InitCuller ();

  /**
   * Return c-buffer (or NULL if not used).
   */
  csCBuffer* GetCBuffer () const { return c_buffer; }

  /**
   * Return xor-buffer (or NULL if not used).
   */
  csXORBuffer* GetXORBuffer () const { return xor_buffer; }

  /**
   * Return cbuffer cube.
   */
  csCBufferCube* GetCBufCube () const { return cbufcube; }

  /**
   * Enable PVS.
   */
  void EnablePVS () { use_pvs = true; use_pvs_only = false; }

  /**
   * Disable PVS.
   */
  void DisablePVS () { use_pvs = false; }

  /**
   * Is PVS enabled?
   */
  virtual bool IsPVS () const { return use_pvs; }

  /**
   * Use only PVS for culling. This flag only makes sense when
   * PVS is enabled.
   */
  void EnablePVSOnly () { use_pvs_only = true; }

  /**
   * Don't use only PVS for culling.
   */
  void DisablePVSOnly () { use_pvs_only = false; }

  /**
   * Is PVS only enabled?
   */
  bool IsPVSOnly () { return use_pvs_only; }

  /**
   * Freeze the PVS for some position.
   */
  void FreezePVS (const csVector3& pos) { freeze_pvs = true; freeze_pvs_pos = pos; }

  /**
   * Unfreeze the PVS.
   */
  void UnfreezePVS () { freeze_pvs = false; }

  /**
   * Is the PVS frozen?
   */
  bool IsPVSFrozen () { return freeze_pvs; }

  /**
   * Return the frozen position for the PVS.
   */
  const csVector3& GetFrozenPosition () const { return freeze_pvs_pos; }

  /**
   * Set the mode for the lighting cache (combination of CS_ENGINE_CACHE_???).
   * Default is CS_ENGINE_CACHE_READ.
   */
  virtual void SetLightingCacheMode (int mode) { lightcache_mode = mode; }
  /// Get the mode for the lighting cache.
  virtual int GetLightingCacheMode () { return lightcache_mode; }

  /// Return current lightmap cell size
  virtual int GetLightmapCellSize () const;
  /// Set lightmap cell size
  virtual void SetLightmapCellSize (int Size);
  /// Return default lightmap cell size
  virtual int GetDefaultLightmapCellSize () const { return default_lightmap_cell_size; }

  /// Set clear z buffer flag
  virtual void SetClearZBuf (bool yesno)
  {
    clear_zbuf = yesno;
  }

  /// Get clear z buffer flag
  virtual bool GetClearZBuf () const { return clear_zbuf; }

  /// Get default clear z-buffer flag
  virtual bool GetDefaultClearZBuf () const { return default_clear_zbuf; }

  /**
   * Set the maximum lightmap dimensions. Polys with lightmaps larger than
   * this are not lit.
   */
  virtual void SetMaxLightmapSize(int w, int h) 
  { max_lightmap_w = w; max_lightmap_h = h; }
  /// Retrieve maximum lightmap size
  virtual void GetMaxLightmapSize(int& w, int& h)
  { w = max_lightmap_w; h = max_lightmap_h; }
  /// Retrieve default maximum lightmap size
  virtual void GetDefaultMaxLightmapSize(int& w, int& h)
  { w = default_max_lightmap_w; h = default_max_lightmap_h; }
  
  /**
   * Reset a subset of flags/settings (which may differ from one world/map to 
   * another) to its defaults. This currently includes:
   *   - clear z buffer flag
   *   - lightmap cell size
   *   - maximum lighmap size
   */
  virtual void ResetWorldSpecificSettings();  

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

  /**
   * Return the object managing all loaded textures.
   */
  csTextureList* GetTextures () const { return textures; }

  /**
   * Return the object managing all loaded materials.
   */
  csMaterialList* GetMaterials () const { return materials; }

  /**
   * Create a base material.
   */
  virtual iMaterial* CreateBaseMaterial (iTextureWrapper* txt);
  virtual iMaterial* CreateBaseMaterial (iTextureWrapper* txt,
  	int num_layers, iTextureWrapper** wrappers, csTextureLayer* layers);

  virtual iMaterialList* GetMaterialList () const;
  virtual iTextureList* GetTextureList () const;
  virtual iRegionList* GetRegions ();

  /**
   * Conveniance function to create the thing containing the
   * convex outline of a sector. The thing will be empty but
   * it will have CS_ZBUF_FILL set. This version creates a mesh wrapper.
   */
  iMeshWrapper* CreateSectorWallsMesh (csSector* sector, const char* name);

  /**
   * Conveniance function to create the thing containing the
   * convex outline of a sector. The thing will be empty but
   * it will have CS_ZBUF_FILL set. This version creates a mesh wrapper.
   */
  virtual iMeshWrapper* CreateSectorWallsMesh (iSector* sector,
      const char* name);

  /**
   * Add a dynamic light to the engine.
   */
  void AddDynLight (csDynLight* dyn);

  /**
   * Remove a dynamic light from the engine.
   */
  void RemoveDynLight (csDynLight* dyn);

  /**
   * Return the first dynamic light in this engine.
   */
  csDynLight* GetFirstDynLight () const { return first_dyn_lights; }

  /**
   * This routine returns all lights which might affect an object
   * at some position according to the following flags:<br>
   * <ul>
   * <li>CS_NLIGHT_SHADOWS: detect shadows and don't return lights for
   *     which the object is shadowed (not implemented yet).
   * <li>CS_NLIGHT_STATIC: return static lights.
   * <li>CS_NLIGHT_DYNAMIC: return dynamic lights.
   * <li>CS_NLIGHT_NEARBYSECTORS: Also check lights in nearby sectors
   *     (not implemented yet).
   * </ul><br>
   * It will only return as many lights as the size that you specified
   * for the light array. The returned lights are not guaranteed to be sorted
   * but they are guaranteed to be the specified number of lights closest to
   * the given position.<br>
   * This function returns the actual number of lights added to the 'lights'
   * array.
   */
  virtual int GetNearbyLights (iSector* sector, const csVector3& pos,
  	uint32 flags, iLight** lights, int max_num_lights);

  /**
   * This routine returns an iterator to iterate over
   * all nearby sectors.
   * Delete the iterator with 'DecRef()' when ready.
   */
  virtual iSectorIterator* GetNearbySectors (iSector* sector,
  	const csVector3& pos, float radius);

  /**
   * This routine returns an iterator to iterate over
   * all objects of a given type that are within a radius
   * of a given position. You can use SCF_QUERY_INTERFACE to get
   * any interface from the returned objects. <p>
   * Delete the iterator with 'DecRef()' when ready.
   */
  virtual iObjectIterator* GetNearbyObjects (iSector* sector,
    const csVector3& pos, float radius);

  virtual bool RemoveObject (iBase* object);

  /**
   * Add a halo attached to given light to the engine.
   */
  void AddHalo (csLight* Light);

  /**
   * Remove halo attached to given light from the engine.
   */
  void RemoveHalo (csLight* Light);

  /**
   * Draw the 3D world given a camera and a clipper. Note that
   * in order to be able to draw using the given 3D driver
   * all textures must have been registered to that driver (using
   * Prepare()). Note that you need to call Prepare() again if
   * you switch to another 3D driver.
   */
  virtual void Draw (iCamera* c, iClipper2D* clipper);

  /**
   * This function is similar to Draw. It will do all the stuff
   * that Draw would do except for one important thing: it will
   * not draw anything. Instead it will call a callback function for
   * every entity that it was planning to draw. This allows you to show
   * or draw debugging information (2D egdes for example).
   */
  virtual void DrawFunc (iCamera* c, iClipper2D* clipper,
    iDrawFuncCallback* callback);

  /**
   * Create an iterator to iterate over all static lights of the engine.
   */
  csLightIt* NewLightIterator (iRegion* region = NULL)
  {
    csLightIt* it;
    it = new csLightIt (this, region);
    return it;
  }

  /**
   * Add an object to the current region.
   */
  void AddToCurrentRegion (csObject* obj);

  /// Register a new render priority.
  virtual void RegisterRenderPriority (const char* name, long priority,
  	int rendsort = CS_RENDPRI_NONE);
  /// Get a render priority by name.
  virtual long GetRenderPriority (const char* name) const;
  /// Get the render priority sorting flag.
  virtual int GetRenderPrioritySorting (const char* name) const;
  /// Get the render priority sorting flag.
  virtual int GetRenderPrioritySorting (long priority) const;
  /// Get the render priority for sky objects (attached to 'sky' name).
  virtual long GetSkyRenderPriority () const { return render_priority_sky; }
  /// Get the render priority for wall objects (attached to 'wall' name).
  virtual long GetWallRenderPriority () const { return render_priority_wall; }
  /// Get the render priority for general objects (attached to 'object' name).
  virtual long GetObjectRenderPriority () const { return render_priority_object; }
  /// Get the render priority for alpha objects (attached to 'alpha' name).
  virtual long GetAlphaRenderPriority () const { return render_priority_alpha; }
  /// Clear all render priorities.
  virtual void ClearRenderPriorities ();

  /// @@@ Temporary until things move to their own mesh plugin system.
  csThingObjectType* thing_type;
  virtual iMeshObjectType* GetThingType () const
  {
    return (iMeshObjectType*)thing_type;
  }

  SCF_DECLARE_IBASE;

  //------------------ iComponent interface implementation --------------------

  /**
   * Initialize the engine. This is automatically called by system driver
   * at startup so that plugin can do basic initialization stuff, register
   * with the system driver and so on.
   */
  virtual bool Initialize (iObjectRegistry* object_reg);

  /// We need to handle some events
  virtual bool HandleEvent (iEvent &Event);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csEngine);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
  struct EventHandler : public iEventHandler
  {
  private:
    csEngine* parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler (csEngine* parent)
    {
      SCF_CONSTRUCT_IBASE (NULL);
      EventHandler::parent = parent;
    }
    virtual bool HandleEvent (iEvent& e) { return parent->HandleEvent(e); }
  } * scfiEventHandler;

  //--------------------- iEngine interface implementation --------------------

  virtual csEngine *GetCsEngine () { return this; };

  /**
   * Query the format to load textures
   * (usually this depends on texture manager).
   */
  virtual int GetTextureFormat () const;

  /**
   * Create or select a new region (name can be NULL for the default main
   * region). All new objects will be marked as belonging to this region.
   */
  virtual void SelectRegion (const char* iName);

  /**
   * Create or select a new region (region can be NULL for the default main
   * region). All new objects will be marked as belonging to this region.
   */
  virtual void SelectRegion (iRegion* region);

  /**
   * Get a reference to the current region (or NULL if the default main
   * region is selected).
   */
  virtual iRegion* GetCurrentRegion () const;

  /// Clear the entire engine.
  virtual void DeleteAll ();

  /// Register a texture to be loaded during Prepare()
  virtual iTextureWrapper* CreateTexture (const char *iName, const char *iFileName,
    csColor *iTransp, int iFlags);
  /// Register a material to be loaded during Prepare()
  virtual iMaterialWrapper* CreateMaterial (const char *iName, iTextureWrapper* texture);

  /// Create a empty sector with given name.
  virtual iSector *CreateSector (const char *iName);

  /// Return the list of sectors
  virtual iSectorList *GetSectors ()
    { return &sectors.scfiSectorList; }
  /// Return the list of mesh factories
  virtual iMeshFactoryList *GetMeshFactories ()
    { return &mesh_factories.scfiMeshFactoryList; }
  /// Return the list of meshes
  virtual iMeshList *GetMeshes ()
    { return &meshes.scfiMeshList; }
  /// Return the list of collections
  virtual iCollectionList *GetCollections ()
    { return &collections.scfiCollectionList; }
  /// Return the list of camera positions.
  virtual iCameraPositionList *GetCameraPositions ()
    { return &camera_positions.scfiCameraPositionList; }

  /// Create a new camera.
  virtual iCamera* CreateCamera ();

  /// Create a static/pseudo-dynamic light.
  virtual iStatLight* CreateLight (const char* name,
  	const csVector3& pos, float radius,
  	const csColor& color, bool pseudoDyn);
  /// Find a static/pseudo-dynamic light by ID.
  virtual iStatLight* FindLight (unsigned long light_id) const;
  /// Find a static/pseudo-dynamic light by name.
  virtual iStatLight* FindLight (const char *Name, bool RegionOnly = false)
    const;
  /// Create a dynamic light.
  virtual iDynLight* CreateDynLight (const csVector3& pos, float radius,
  	const csColor& color);
  /// Remove a dynamic light.
  virtual void RemoveDynLight (iDynLight*);

  /// Create a mesh factory wrapper from a mesh plugin
  virtual iMeshFactoryWrapper* CreateMeshFactory (const char* classId,
  	const char* name);
  /// Create a mesh factory wrapper for an existing mesh factory
  virtual iMeshFactoryWrapper* CreateMeshFactory (iMeshObjectFactory *,
  	const char* name);
  /// Create an uninitialized mesh factory wrapper
  virtual iMeshFactoryWrapper* CreateMeshFactory (const char* name);
  virtual iLoaderContext* CreateLoaderContext (iRegion* region = NULL);
  /// Load mesh factory.
  virtual iMeshFactoryWrapper* LoadMeshFactory (
  	const char* name, const char* loaderClassId,
	iDataBuffer* input);

  /// Create a mesh wrapper from a mesh factory wrapper
  virtual iMeshWrapper* CreateMeshWrapper (iMeshFactoryWrapper* factory,
  	const char* name, iSector* sector = NULL,
	const csVector3& pos = csVector3(0, 0, 0));
  /// Create a mesh wrapper for an existing mesh object
  virtual iMeshWrapper* CreateMeshWrapper (iMeshObject*,
  	const char* name, iSector* sector = NULL,
	const csVector3& pos = csVector3(0, 0, 0));
  /// Create an uninitialized mesh wrapper
  virtual iMeshWrapper* CreateMeshWrapper (const char* name);
  /// Load mesh object.
  virtual iMeshWrapper* LoadMeshWrapper (
  	const char* name, const char* loaderClassId,
	iDataBuffer* input, iSector* sector, const csVector3& pos);

  virtual iClipper2D* GetTopLevelClipper () const;

  /// Set the amount of ambient light
  virtual void SetAmbientLight (const csColor &c);
  /// Return the amount of ambient light
  virtual void GetAmbientLight (csColor &c) const;

  virtual iMaterialWrapper* FindMaterial (const char* name,
  	iRegion* region = NULL);
  virtual iTextureWrapper* FindTexture (const char* name,
  	iRegion* region = NULL);
  virtual iSector* FindSector (const char* name,
  	iRegion* region = NULL);
  virtual iMeshWrapper* FindMeshObject (const char* name,
  	iRegion* region = NULL);
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name,
  	iRegion* region = NULL);
  virtual iCameraPosition* FindCameraPosition (const char* name,
  	iRegion* region = NULL);
  virtual iCollection* FindCollection (const char* name,
  	iRegion* region = NULL);

  virtual void DebugCommand (const char* cmd);

  //----------------------- iCacheManager implementation ---------------------

  virtual void SetCacheManager (iCacheManager* cache_mgr);
  virtual iCacheManager* GetCacheManager ();

  //--------------------- iConfig interface implementation -------------------

  csEngineConfig scfiConfig;

  //----------------Begin-Multi-Context-Support-------------------------------

  /// Point engine to rendering context
  virtual void SetContext (iGraphics3D* g3d);
  /// Return the current drawing context
  virtual iGraphics3D *GetContext () const;

private:
  /// Resizes frame width and height dependent data members
  void Resize ();
  /// Flag set when window requires resizing.
  bool resize;
  /**
   * Private class which keeps state information about the engine specific to
   * each context.
   */

  class csEngineState
  {
  public:
    csEngine *engine;
    bool resize;
    iGraphics2D *G2D;
    iGraphics3D *G3D;
    csCBuffer* c_buffer;
    csXORBuffer* xor_buffer;
    csCBufferCube* cbufcube;
    /// Creates an engine state by copying the relevant data members
    csEngineState (csEngine *this_engine);

    /// Destroys buffers and trees
    virtual ~csEngineState ();

    /// Swaps state into engine and deals with resizing issues.
    void Activate ();
  };

  friend class csEngineState;

  class csEngineStateVector : public csVector
  {
  public:
     // Constructor
    csEngineStateVector () : csVector (8, 8) {}
    // Destructor
    virtual ~csEngineStateVector () { DeleteAll (); }
    // Free an item from array
    virtual bool FreeItem (csSome Item)
    { delete (csEngineState *)Item; return true; }
    // Find a state by referenced g2d
    virtual int CompareKey (csSome Item, csConstSome Key, int /*Mode*/) const
    { return ((csEngineState *)Item)->G3D == (iGraphics3D *)Key ? 0 : -1; }
    // Get engine state according to index
    inline csEngineState *Get (int n) const
    { return (csEngineState *)csVector::Get (n); }

    // Mark engine state to be resized
    void Resize (iGraphics2D *g2d);

    // Dispose of engine state dependent on g2d
    void Close (iGraphics2D *g2d);
  };

  csEngineStateVector *engine_states;

  //------------End-Multi-Context-Support-------------------------------------

  /**
   * This object is used in the engine as an embedded iObject interface.
   */
  class iObjectInterface : public csObject
  {
    SCF_DECLARE_EMBEDDED_IBASE (csEngine);
  } scfiObject;
};

#endif // __CS_ENGINE_H__
