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
#include "csgeom/math3d.h"
#include "csutil/nobjvec.h"
#include "csengine/arrays.h"
#include "csengine/rview.h"
#include "csengine/thing.h"
#include "csutil/csobject.h"
#include "ivideo/graph3d.h"
#include "isys/system.h"
#include "iengine/engine.h"
#include "iutil/config.h"

class csRegion;
class csRadiosity;
class csSector;
class csMeshWrapper;
class csTerrainWrapper;
class csTextureWrapper;
class csMaterialWrapper;
class csTextureList;
class csMaterialList;
class csPolygon3D;
class csCamera;
class csCollection;
class csStatLight;
class csDynLight;
class csCBufferCube;
class csEngine;
class Dumper;
class csLight;
class csCBuffer;
class csPoly2DPool;
class csLightPatchPool;
class csLightHalo;
class csRenderView;
struct iSystem;
struct iVFS;
struct iMaterialWrapper;
struct iRegion;
struct iLight;
struct iImageIO;
struct iClipper2D;

DECLARE_FAST_INTERFACE (iEngine)
DECLARE_FAST_INTERFACE (iSector)
DECLARE_FAST_INTERFACE (iMeshWrapper)
DECLARE_FAST_INTERFACE (iCollection)
DECLARE_FAST_INTERFACE (iMeshFactoryWrapper)
DECLARE_FAST_INTERFACE (iCurveTemplate)
DECLARE_FAST_INTERFACE (iMaterialWrapper)
DECLARE_FAST_INTERFACE (iTextureWrapper)
DECLARE_FAST_INTERFACE (iCameraPosition)
DECLARE_FAST_INTERFACE (iPolyTxtPlane)
DECLARE_FAST_INTERFACE (iStatLight)
DECLARE_FAST_INTERFACE (iDynLight)
DECLARE_FAST_INTERFACE (iMaterialHandle)
DECLARE_FAST_INTERFACE (iTerrainWrapper)
DECLARE_FAST_INTERFACE (iTerrainFactoryWrapper)
DECLARE_FAST_INTERFACE (iMapNode)

DECLARE_FAST_INTERFACE (csPolyTxtPlane)
DECLARE_FAST_INTERFACE (csCollection)
DECLARE_FAST_INTERFACE (csMeshWrapper)
DECLARE_FAST_INTERFACE (csMeshFactoryWrapper)
DECLARE_FAST_INTERFACE (csCurveTemplate)
DECLARE_FAST_INTERFACE (csSector)
DECLARE_FAST_INTERFACE (csTextureWrapper)
DECLARE_FAST_INTERFACE (csMaterialWrapper)
DECLARE_FAST_INTERFACE (csCollider)
DECLARE_FAST_INTERFACE (csRadPoly)
DECLARE_FAST_INTERFACE (csRadCurve)

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
  csRegion* region;
  // Current sector index.
  int sector_idx;
  // Current light index.
  int light_idx;

  // Go to next sector. Return false if finished.
  bool NextSector ();

public:
  /// Construct an iterator and initialize to start.
  csLightIt (csEngine*, csRegion* region = NULL);

  /// Restart iterator.
  void Restart ();

  /// Get light from iterator. Return NULL at end.
  csLight* Fetch ();

  /// Get the sector for the last fetched light.
  csSector* GetLastSector ();
};

/**
 * Iterator to iterate over sectors in the engine which are within
 * a radius from a given point.
 * This iterator assumes there are no fundamental changes
 * in the engine while it is being used.
 * If changes to the engine happen the results are unpredictable.
 */
class csSectorIt
{
private:
  // The position and radius.
  csSector* sector;
  csVector3 pos;
  float radius;
  // Polygon index (to loop over all portals).
  // If -1 then we return current sector first.
  int cur_poly;
  // If not null then this is a recursive sector iterator
  // that we are currently using.
  csSectorIt* recursive_it;
  // If true then this iterator has ended.
  bool has_ended;
  // Last position (from Fetch).
  csVector3 last_pos;

public:
  /// Construct an iterator and initialize to start.
  csSectorIt (csSector* sector, const csVector3& pos, float radius);

  /// Destructor.
  ~csSectorIt ();

  /// Restart iterator.
  void Restart ();

  /// Get sector from iterator. Return NULL at end.
  csSector* Fetch ();

  /**
   * Get last position that was used from Fetch. This can be
   * different from 'pos' because of space warping.
   */
  const csVector3& GetLastPosition () { return last_pos; }
};

/**
 * Iterator to iterate over objects in the engine.
 * This iterator assumes there are no fundamental changes
 * in the engine while it is being used.
 * If changes to the engine happen the results are unpredictable.
 */
class csObjectIt
{
  friend class csEngine;

private:
  // The engine for this iterator.
  csEngine* engine;
  // The starting position and radius.
  csSector* start_sector;
  csVector3 start_pos;
  float radius;
  // Current position ('pos' can be warped so that's why it is here).
  csSector* cur_sector;
  csVector3 cur_pos;
  // Current object.
  iObject* cur_object;
  // Current index.
  int cur_idx;
  // Iterator over the sectors.
  csSectorIt* sectors_it;
  // Current object list to iterate over
  enum {
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
  csObjectIt (csEngine*, csSector* sector,
    const csVector3& pos, float radius);

public:
  /// Destructor.
  ~csObjectIt ();

  /// Restart iterator.
  void Restart ();

  /// Get object from iterator. Return NULL at end.
  iObject* Fetch ();
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
  DECLARE_EMBEDDED_IBASE (csEngine);
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
  friend class Dumper;

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
   * objects of type csSector*. Use CreateCsSector() or CreateSector()
   * to add sectors to the engine.
   */
  csNamedObjVector sectors;

  /**
   * List of planes. This vector contains objects of type
   * csPolyTxtPlane*. Note that this vector only contains named
   * planes. Default planes which are created for polygons
   * are not in this list.
   */
  csNamedObjVector planes;

  /**
   * List of all collections in the engine. This vector contains objects
   * of type csCollection*. Use RemoveCollection()
   * to remove collections from this list. These functions
   * take care of correctly removing the collections from all sectors
   * as well. Note that after you add a collection to the list you still
   * need to add it to all sectors that you want it to be visible in.
   */
  csNamedObjVector collections;

  /**
   * List of mesh object factories. This vector contains objects of
   * type csMeshFactoryWrapper*.
   */
  csNamedObjVector mesh_factories;

  /**
   * List of terrain object factories. This vector contains objects of
   * type csTerrainFactoryWrapper*.
   */
  csNamedObjVector terrain_factories;

  /**
   * List of curve templates (bezier templates). This vector contains objects of
   * type csCurveTemplate*.
   */
  csNamedObjVector curve_templates;

  /**
   * List of all meshes in the engine. This vector contains objects
   * of type csMeshWrapper*. Use RemoveMesh() to remove meshes from this
   * list. This function will take care of correctly removing the meshes
   * from all sectors as well. Note that after you add a mesh to the list
   * you still need to add it to all sectors that you want it to be visible in.
   */
  csNamedObjVector meshes;

  /**
   * List of all terrain in the engine. This vector contains objects
   * of type csTerrainWrapper*. Use RemoveTerrain()
   * to remove terrains from this list. These functions
   * take care of correctly removing the terrains from all sectors
   * as well. Note that after you add a terrain to the list you still
   * need to add it to all sectors that you want it to be visible in.
   */
  csNamedObjVector terrains;

  /**
   * The list of all camera position objects.
   */
  csNamedObjVector camera_positions;

  /// Remember dimensions of display.
  static int frame_width, frame_height;
  /// Remember iSystem interface.
  static iSystem* System;
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
  /// A pointer to current object library
  csObject* library;
  /**
   * The list of all object libraries currently loaded.
   * @@@ When regions are complete support for libraries will be removed.
   */
  csNamedObjVector libraries;
  /// A pointer to the current region.
  csRegion* region;
  /// The list of all regions currently loaded.
  csNamedObjVector regions;

  /// The list of all named render priorities.
  csVector render_priorities;
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

  /// Option variable: inhibit lightmap recalculation?
  static bool do_not_force_relight;
  /// Option variable: force lightmap recalculation?
  static bool do_force_relight;
  /// Option variable: inhibit visibility recalculation?
  static bool do_not_force_revis;
  /// Option variable: force visibility recalculation?
  static bool do_force_revis;
  /// Option variable: radiosity debugging (step by step)?
  static bool do_rad_debug;

private:
  /// Texture and color information objects.
  csTextureList* textures;
  /// Material objects.
  csMaterialList* materials;
  /// Linked list of dynamic lights.
  csDynLight* first_dyn_lights;
  /// List of halos (csHaloInformation).
  csHaloArray halos;  
  /// If true then the lighting cache is enabled.
  static bool do_lighting_cache;
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

  /**
   * If this nextframe_pending is not 0 then a call of NextFrame
   * has happened. As soon as some object is visible (DrawTest() returns
   * true) we will really call NextFrame() if its locally remembered
   * last-anim value is different from this one. This should improve
   * global speed of the engine as this means that invisible particle
   * systems will now not be updated anymore until they are really visible.
  */
  cs_time nextframe_pending;

private:
  /**
   * Resolve the engine mode if it is CS_ENGINE_AUTODETECT.
   */
  void ResolveEngineMode ();

  /**
   * Setup for starting a Draw or DrawFunc.
   */
  void StartDraw (csCamera* c, iClipper2D* view, csRenderView& rview);

  /**
   * Find some object (given a name) which is both in a region
   * and the given vector.
   */
  csObject* FindObjectInRegion (csRegion* region,
	const csNamedObjVector& vector, const char* name) const;

  /**
   * Controll animation and delete meshes that want to die.
   */
  void ControlMeshes ();

public:
  /**
   * The current camera for drawing the world.
   */
  csCamera* current_camera;

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
  void PrepareTextures ();

  /**
   * Calls UpdateMove for all meshes to initialise bsp bounding boxes.
   * Call this after creating a BSP tree. csEngine::Prepare() will call
   * this function automatically so you normally don't have to call it.
   */
  void PrepareMeshes ();

  /**
   * Calculate all lighting information. Normally you shouldn't call
   * this function directly, because it will be called by Prepare().
   * If the optional 'region' parameter is given then only lights will
   * be recalculated for the given region.
   */
  void ShineLights (csRegion* region = NULL);

  /// Query the iObject for the engine.
  virtual iObject *QueryObject();
  /// Query the csObject for the engine.
  inline csObject *QueryCsObject () { return &scfiObject; }

  /**
   * Prepare the engine. This function must be called after
   * you loaded/created the world. It will prepare all lightmaps
   * for use and also free all images that were loaded for
   * the texture manager (the texture manager should have them
   * locally now).
   */
  virtual bool Prepare ();

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
  void SetEngineMode (int mode)
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
    if (engine_mode == CS_ENGINE_ZBUFFER)
      return CSDRAW_CLEARZBUFFER;
    else
      return 0;
  }

  /**
   * Get the last animation time.
   */
  cs_time GetLastAnimationTime () const { return nextframe_pending; }

  /**
   * Initialize the culler.
   */
  void InitCuller ();

  /**
   * Return c-buffer (or NULL if not used).
   */
  csCBuffer* GetCBuffer () const { return c_buffer; }

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
   * Cache lighting. If true (default) then lighting will be cached in
   * either the map file or else 'precalc.zip'. If false then this
   * will not happen and lighting will be calculated at startup.
   * If set to 'false' recalculating of lightmaps will be forced.
   * If set to 'true' recalculating of lightmaps will depend on
   * wether or not the lightmap was cached.
   */
  void EnableLightingCache (bool en);

  /// Return true if lighting cache is enabled.
  bool IsLightingCacheEnabled () const { return do_lighting_cache; }

  /// Return current lightmap cell size
  virtual int GetLightmapCellSize () const;
  /// Set lightmap cell size
  virtual void SetLightmapCellSize (int Size);

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
   * Clear everything in the engine.
   */
  void Clear ();

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

  /**
   * Create a empty sector with given name.
   */
  csSector* CreateCsSector (const char *iName, bool link = true);

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
  int GetNearbyLights (csSector* sector, const csVector3& pos, ULong flags,
  	iLight** lights, int max_num_lights);

  /**
   * This routine returns an iterator to iterate over
   * all nearby sectors.
   * Delete the iterator with 'delete' when ready.
   */
  csSectorIt* GetNearbySectors (csSector* sector,
  	const csVector3& pos, float radius);

  /**
   * This routine returns an iterator to iterate over
   * all objects of a given type that are within a radius
   * of a given position. You can use QUERY_INTERFACE to get
   * any interface from the returned objects. <p>
   * Delete the iterator with 'delete' when ready.
   */
  csObjectIt* GetNearbyObjects (csSector* sector,
    const csVector3& pos, float radius);

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
   * Locate the first static light which is closer than 'dist' to the
   * given position. This function scans all sectors and locates
   * the first one which statisfies that criterium.
   */
  csStatLight* FindCsLight (float x, float y, float z, float dist) const;

  /**
   * Find the light with the given light id.
   */
  csStatLight* FindCsLight (unsigned long id) const;

  /**
   * Find the light with the given name.
   */
  csStatLight* FindCsLight (const char* name, bool regionOnly = false) const;

  /**
   * Unlink and delete (using DecRef()) a mesh from the engine.
   * It is also removed from all sectors.
   */
  void RemoveMesh (csMeshWrapper* mesh);

  /**
   * Unlink and delete (using DecRef()) a mesh from the engine.
   * It is also removed from all sectors.
   */
  virtual void RemoveMesh (iMeshWrapper* mesh);

  /**
   * Unlink and delete (using DecRef ()) a terrain from the engine.
   * It is also removed from all sectors.
   */
  void RemoveTerrain (csTerrainWrapper *pTerrain);

  /**
   * Unlink and delete (using DecRef ()) a terrain from the engine.
   * It is also removed from all sectors.
   */
  virtual void RemoveTerrain (iTerrainWrapper *pTerrain);

  /**
   * Unlink and delete a collection from the engine.
   * It is also removed from all sectors.
   */
  void RemoveCollection (csCollection* collection);

  /**
   * Create an iterator to iterate over all static lights of the engine.
   */
  csLightIt* NewLightIterator (csRegion* region = NULL)
  {
    csLightIt* it;
    it = new csLightIt (this, region);
    return it;
  }

  /**
   * Get a reference to the current region (or NULL if the default main
   * region is selected).
   */
  csRegion* GetCsCurrentRegion () const
  {
    return region;
  }

  /**
   * Add an object to the current region.
   */
  void AddToCurrentRegion (csObject* obj);

  /// Find a loaded texture by name.
  csTextureWrapper* FindCsTexture (const char* iName, bool regionOnly = false)
    const;
  /// Find a loaded material by name.
  csMaterialWrapper* FindCsMaterial (const char* iName,
  	bool regionOnly = false) const;

  /// Register a new render priority.
  virtual void RegisterRenderPriority (const char* name, long priority);
  /// Get a render priority by name.
  virtual long GetRenderPriority (const char* name) const;
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

  DECLARE_IBASE;

  //--------------------- iPlugIn interface implementation --------------------

  /**
   * Initialize the engine. This is automatically called by system driver
   * at startup so that plugin can do basic initialization stuff, register
   * with the system driver and so on.
   */
  virtual bool Initialize (iSystem* sys);

  /// We need to handle some events
  virtual bool HandleEvent (iEvent &Event);

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

  /// find a region by name
  virtual iRegion* FindRegion (const char *name) const;

  /**
   * Create or select a new object library (name can be NULL for engine).
   * All new objects will be marked as belonging to this library.
   * You can then delete a whole library at once, for example.
   * @@@ Libraries are obsolete!!! When regions are finished use them
   * instead.
   */
  virtual void SelectLibrary (const char *iName);
  /// Delete a whole library (all objects that are part of library)
  virtual bool DeleteLibrary (const char *iName);
  /// Clear the entire engine (delete all libraries)
  virtual void DeleteAll ();

  /// Register a texture to be loaded during Prepare()
  virtual iTextureWrapper* CreateTexture (const char *iName, const char *iFileName,
    csColor *iTransp, int iFlags);
  /// Register a material to be loaded during Prepare()
  virtual iMaterialWrapper* CreateMaterial (const char *iName, iTextureWrapper* texture);
  /// Create a named camera position object
  virtual iCameraPosition* CreateCameraPosition (const char *iName, const char *iSector,
    const csVector3 &iPos, const csVector3 &iForward, const csVector3 &iUpward);
  /// Create a texture plane
  virtual bool CreatePlane (const char *iName, const csVector3 &iOrigin,
    const csMatrix3 &iMatrix);
  /// Create a empty sector with given name.
  virtual iSector *CreateSector (const char *iName, bool link = true);

  /// Query number of sectors in engine
  virtual int GetSectorCount () const
  { return sectors.Length (); }
  /// Get a sector by index
  virtual iSector *GetSector (int iIndex) const;
  /// Find a sector by name
  virtual iSector *FindSector (const char *iName, bool regionOnly = false)
    const;
  /// Delete a sector
  virtual void DeleteSector (iSector *Sector);

  /// Find a mesh by name
  virtual iMeshWrapper *FindMeshObject (const char *iName,
  	bool regionOnly = false) const;
  /// Find a mesh factory by name
  virtual iMeshFactoryWrapper *FindMeshFactory (const char *iName,
  	bool regionOnly = false) const;
  /// Delete a mesh factory by name
  virtual void DeleteMeshFactory (const char* iName, bool regionOnly = false);

  /// Find a terrain by name
  virtual iTerrainWrapper *FindTerrainObject (const char *iName,
  	bool regionOnly = false) const;
  /// Find a terrain factory by name
  virtual iTerrainFactoryWrapper *FindTerrainFactory (const char *iName,
  	bool regionOnly) const;

  /// Find a loaded texture by name.
  virtual iTextureWrapper* FindTexture (const char* iName,
  	bool regionOnly = false) const;
  /// Find a loaded material by name.
  virtual iMaterialWrapper* FindMaterial (const char* iName,
  	bool regionOnly = false) const;
  /// Find a loaded camera position by name.
  virtual iCameraPosition* FindCameraPosition (const char* iName,
  	bool regionOnly = false) const;
  /// Find a collection by name
  virtual iCollection* FindCollection (const char* iName,
  	bool regionOnly = false) const;
  /// Create a new collection.
  virtual iCollection* CreateCollection (const char* iName);

  /// Create a new view on the engine.
  virtual iView* CreateView (iGraphics3D* g3d);

  /// Create a static/pseudo-dynamic light.
  virtual iStatLight* CreateLight (const char* name,
  	const csVector3& pos, float radius,
  	const csColor& color, bool pseudoDyn);
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
  /// Load mesh factory.
  virtual iMeshFactoryWrapper* LoadMeshFactory (
  	const char* classId, const char* name,
	const char* loaderClassId,
	iDataBuffer* input);

  /// Create a mesh wrapper from a mesh factory wrapper
  virtual iMeshWrapper* CreateMeshObject (iMeshFactoryWrapper* factory,
  	const char* name, iSector* sector = NULL,
	const csVector3& pos = csVector3(0, 0, 0));
  /// Create a mesh wrapper for an existing mesh object
  virtual iMeshWrapper* CreateMeshObject (iMeshObject*,
  	const char* name, iSector* sector = NULL,
	const csVector3& pos = csVector3(0, 0, 0));
  /// Create an uninitialized mesh wrapper
  virtual iMeshWrapper* CreateMeshObject (const char* name);
  /// Load mesh object.
  virtual iMeshWrapper* LoadMeshObject (
  	const char* classId, const char* name,
	const char* loaderClassId,
	iDataBuffer* input, iSector* sector, const csVector3& pos);

  /// return the number of mesh objects
  virtual int GetMeshObjectCount () const;
  /// return a mesh object by index
  virtual iMeshWrapper *GetMeshObject (int n) const;

  /// return the number of mesh factories
  virtual int GetMeshFactoryCount () const;
  /// return a mesh factory by index
  virtual iMeshFactoryWrapper *GetMeshFactory (int n) const;

  /// Create a terrain factory wrapper from a terrain plugin
  virtual iTerrainFactoryWrapper* CreateTerrainFactory (const char* pClassId,
	  const char* pName);
  /// Create a terrain factory wrapper for an existing terrain factory
  virtual iTerrainFactoryWrapper* CreateTerrainFactory (iTerrainObjectFactory*,
  	const char* name);
  /// Create an uninitialized terrain factory wrapper
  virtual iTerrainFactoryWrapper* CreateTerrainFactory (const char* name);

  /// Create a terrain wrapper from a terrain factory wrapper
  virtual iTerrainWrapper* CreateTerrainObject (
  	iTerrainFactoryWrapper* pFactWrap,
  	const char* name, iSector* sector);
  /// Create a terrain wrapper for an existing terrain object
  virtual iTerrainWrapper* CreateTerrainObject (iTerrainObject*,
  	const char* name, iSector* sector);
  /// Create an uninitialized terrain wrapper
  virtual iTerrainWrapper* CreateTerrainObject (const char* name);

  virtual iPolyTxtPlane* CreatePolyTxtPlane (const char* name = NULL);
  virtual iPolyTxtPlane* FindPolyTxtPlane (const char* name,
  	bool regionOnly = false) const;
  virtual iCurveTemplate* CreateBezierTemplate (const char* name = NULL);
  virtual iCurveTemplate* FindCurveTemplate (const char *iName,
  	bool regionOnly = false) const;

  virtual iClipper2D* GetTopLevelClipper () const;

  /// Create a map node.
  virtual iMapNode* CreateMapNode (const char* name);
  /// Create a key value pair.

  /// Get the number of collections in the engine
  virtual int GetCollectionCount () const;
  /// Get a collection by its index
  virtual iCollection* GetCollection (int idx) const;

  /// Get the number of camera positions in the engine
  virtual int GetCameraPositionCount () const;
  /// Get a camera position by its index
  virtual iCameraPosition* GetCameraPosition (int idx) const;

  //--------------------- iConfig interface implementation --------------------

  csEngineConfig scfiConfig;

  //----------------Begin-Multi-Context-Support------------------------------

  /// Point engine to rendering context
  virtual void SetContext (iGraphics3D* g3d);

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
    DECLARE_EMBEDDED_IBASE (csEngine);
  } scfiObject;
};

// This is a global replacement for printf ()
#define CsPrintf csEngine::System->Printf

#endif // __CS_ENGINE_H__
