/*
    Crystal Space 3D Engine
    Copyright (C) 1998-2002 by Jorrit Tyberghein

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

#ifndef __CS_IENGINE_ENGINE_H__
#define __CS_IENGINE_ENGINE_H__

/**\file 
 * Crystal Space 3D Engine Interface
 */

/**
 * \addtogroup engine3d
 * @{ */
 
#include "csutil/scf.h"
#include "csgeom/vector3.h"

class csEngine;
class csVector3;
class csFrustum;
class csMatrix3;
class csColor;
struct csTextureLayer;

struct iSector;
struct iFrustumView;
struct iSectorIterator;
struct iObjectIterator;
struct iLight;
struct iLightIterator;
struct iStatLight;
struct iDynLight;
struct iSprite;
struct iMeshObject;
struct iMeshObjectFactory;
struct iMeshWrapper;
struct iMeshFactoryWrapper;
struct iMeshObjectType;
struct iMaterial;
struct iMaterialWrapper;
struct iMaterialList;
struct iTextureWrapper;
struct iTextureHandle;
struct iTextureList;
struct iCameraPosition;
struct iCameraPositionList;
struct iRegion;
struct iGraphics3D;
struct iClipper2D;
struct iObject;
struct iObjectWatcher;
struct iCollection;
struct iCollectionList;
struct iDataBuffer;
struct iCamera;
struct iRenderView;
struct iSectorList;
struct iMeshList;
struct iMeshFactoryList;
struct iProgressMeter;
struct iRegionList;
struct iLoaderContext;
struct iCacheManager;
struct iSharedVariableList;

/** \name GetNearbyLights() flags
 * @{ */
/**
 * Detect shadows and don't return lights for which the object
 * is shadowed (not implemented yet).
 */
#define CS_NLIGHT_SHADOWS 1

/**
 * Return static lights.
 */
#define CS_NLIGHT_STATIC 2

/**
 * Return dynamic lights.
 */
#define CS_NLIGHT_DYNAMIC 4

/**
 * Also check lights in nearby sectors (not implemented yet).
 */
#define CS_NLIGHT_NEARBYSECTORS 8
/** @} */

/** \name SetLightingCacheMode() settings
 * @{ */
/**
 * Read the cache.
 */
#define CS_ENGINE_CACHE_READ 1

/**
 * Write the cache.
 */
#define CS_ENGINE_CACHE_WRITE 2

/**
 * Do not calculate lighting if not up-to-date. On by default.
 */
#define CS_ENGINE_CACHE_NOUPDATE 4
/** @} */

/** \name RegisterRenderPriority() flags
 * @{ */
/**
 * Do not sort this priority.
 */
#define CS_RENDPRI_NONE 0

/**
 * Sort this priority back to front.
 */
#define CS_RENDPRI_BACK2FRONT 1

/**
 * Sort this priority front to back.
 */
#define CS_RENDPRI_FRONT2BACK 2
/** @} */

SCF_VERSION (iEngine, 0, 15, 0);

/**
 * This interface is the main interface to the 3D engine.
 * The engine is responsible for creating new engine-specific objects
 * such as sectors, mesh objects, mesh object factories, lights, and so on.
 */
struct iEngine : public iBase
{
  /// Get the iObject for the engine.
  virtual iObject *QueryObject() = 0;

  /**
   * Prepare the engine. This function must be called after
   * you loaded/created the world. It will prepare all lightmaps
   * for use and also free all images that were loaded for
   * the texture manager (the texture manager should have them
   * locally now). The optional progress meter will be used to
   * report progress.
   */
  virtual bool Prepare (iProgressMeter* meter = NULL) = 0;

  /**
   * Prepare the textures. It will initialise all loaded textures
   * for the texture manager. (Normally you shouldn't call this function
   * directly, because it will be called by Prepare() for you.
   * This function will also prepare all loaded materials after preparing
   * the textures.
   */
  virtual void PrepareTextures () = 0;

  /**
   * Calls UpdateMove for all meshes to initialise bsp bounding boxes.
   * Call this after creating a BSP tree. Prepare() will call
   * this function automatically so you normally don't have to call it.
   */
  virtual void PrepareMeshes () = 0;

  /**
   * Calculate all lighting information. Normally you shouldn't call
   * this function directly, because it will be called by Prepare().
   * If the optional 'region' parameter is given then only lights will
   * be recalculated for the given region.
   */
  virtual void ShineLights (iRegion* region = NULL,
  	iProgressMeter* meter = NULL) = 0;

  /**
   * Query the format to load textures (usually this depends on texture
   * manager)
   */
  virtual int GetTextureFormat () const = 0;

  /**
   * Create or select a new region (name can be NULL for the default main
   * region). All new objects will be marked as belonging to this region.
   */
  virtual void SelectRegion (const char* name) = 0;
  /**
   * Create or select a new region (region can be NULL for the default main
   * region). All new objects will be marked as belonging to this region.
   */
  virtual void SelectRegion (iRegion* region) = 0;
  /**
   * Get a reference to the current region (or NULL if the default main
   * region is selected).
   */
  virtual iRegion* GetCurrentRegion () const = 0;
  /**
   * Add an object to the current region. Normally you don't need to
   * call this function as CS objects already do this automatically.
   */
  virtual void AddToCurrentRegion (iObject* obj) = 0;

  /// Delete everything in the engine.
  virtual void DeleteAll () = 0;

  /**
   * Register a new render priority.
   * The parameter rendsort is one of the CS_RENDPRI_... flags.
   * By default this is #CS_RENDPRI_NONE. The following values are possible:
   * <ul>
   * <li>#CS_RENDPRI_NONE: objects in this render priority are not sorted.
   * <li>#CS_RENDPRI_FRONT2BACK: sort objects front to back (as seen from
   *     camera viewpoint).
   * <li>#CS_RENDPRI_BACK2FRONT: sort objects back to front.
   * </ul>
   * If 'do_camera' is true then this render priority will be scanned
   * for objects that have CS_ENTITY_CAMERA flag set.
   */
  virtual void RegisterRenderPriority (const char* name, long priority,
  	int rendsort = CS_RENDPRI_NONE, bool do_camera = false) = 0;
  /// Get a render priority by name.
  virtual long GetRenderPriority (const char* name) const = 0;
  /// Set the render priority camera flag.
  virtual void SetRenderPriorityCamera (long priority, bool do_camera) = 0;
  /// Get the render priority camera flag.
  virtual bool GetRenderPriorityCamera (const char* name) const = 0;
  /// Get the render priority camera flag.
  virtual bool GetRenderPriorityCamera (long priority) const = 0;
  /// Get the render priority sorting flag.
  virtual int GetRenderPrioritySorting (const char* name) const = 0;
  /// Get the render priority sorting flag.
  virtual int GetRenderPrioritySorting (long priority) const = 0;
  /// Get the render priority for sky objects (attached to 'sky' name).
  virtual long GetSkyRenderPriority () const = 0;
  /// Get the render priority for wall objects (attached to 'wall' name).
  virtual long GetWallRenderPriority () const = 0;
  /// Get the render priority for general objects (attached to 'object' name).
  virtual long GetObjectRenderPriority () const = 0;
  /// Get the render priority for alpha objects (attached to 'alpha' name).
  virtual long GetAlphaRenderPriority () const = 0;
  /// Clear all render priorities.
  virtual void ClearRenderPriorities () = 0;

  /**
   * Create a base material that can be used to give to the texture
   * manager. Assign to a csRef or use DecRef().
   */
  virtual csPtr<iMaterial> CreateBaseMaterial (iTextureWrapper* txt) = 0;

  /**
   * Create a base material that can be used to give to the texture
   * manager. This version also supports texture layers.
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iMaterial> CreateBaseMaterial (iTextureWrapper* txt,
  	int num_layers, iTextureWrapper** wrappers, csTextureLayer* layers) = 0;

  /// Create a texture from a file.
  virtual iTextureWrapper* CreateTexture (const char *name,
  	const char *fileName, csColor *transp, int flags) = 0;
  /// Create a black texture. This is mostly useful for procedural textures.
  virtual iTextureWrapper* CreateBlackTexture (const char *name,
	int w, int h, csColor *iTransp, int iFlags) = 0;

  /// Register a material to be loaded during Prepare()
  virtual iMaterialWrapper* CreateMaterial (const char *name,
  	iTextureWrapper* texture) = 0;
  /**
   * Create a empty sector with given name.
   */
  virtual iSector *CreateSector (const char *name) = 0;

  /**
   * Conveniance function to create the thing containing the
   * convex outline of a sector. The thing will be empty but
   * it will have #CS_ZBUF_FILL set (so that the Z-buffer will be filled
   * by the polygons of this object) and have 'wall' as render
   * priority. This version creates a mesh wrapper.
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iMeshWrapper> CreateSectorWallsMesh (iSector* sector,
      const char* name) = 0;
  /**
   * Conveniance function to create a thing mesh in a sector.
   * This mesh will have #CS_ZBUF_USE set (use Z-buffer fully)
   * and have 'object' as render priority. This means this function
   * is useful for general objects.
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iMeshWrapper> CreateThingMesh (iSector* sector,
  	const char* name) = 0;

  /// Get the list of sectors
  virtual iSectorList* GetSectors () = 0;
  /// Get the list of mesh factories
  virtual iMeshFactoryList* GetMeshFactories () = 0;
  /// Get the list of meshes
  virtual iMeshList* GetMeshes () = 0;
  /// Get the list of collections.
  virtual iCollectionList* GetCollections () = 0;
  /// Get the list of camera positions.
  virtual iCameraPositionList* GetCameraPositions () = 0;
  /// Get the list of all textures.
  virtual iTextureList* GetTextureList () const = 0;
  /// Get the list of all materials.
  virtual iMaterialList* GetMaterialList () const = 0;
  /// Get the list of all shared variables.
  virtual iSharedVariableList* GetVariableList () const = 0;
  /// Get the list of all regions
  virtual iRegionList* GetRegions () = 0;

  /**
   * Find the given material. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not NULL in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * NULL if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   */
  virtual iMaterialWrapper* FindMaterial (const char* name,
  	iRegion* region = NULL) = 0;
  /**
   * Find the given texture. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not NULL in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * NULL if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   */
  virtual iTextureWrapper* FindTexture (const char* name,
  	iRegion* region = NULL) = 0;
  /**
   * Find the given sector. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not NULL in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * NULL if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   */
  virtual iSector* FindSector (const char* name,
  	iRegion* region = NULL) = 0;
  /**
   * Find the given mesh object. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not NULL in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * NULL if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   */
  virtual iMeshWrapper* FindMeshObject (const char* name,
  	iRegion* region = NULL) = 0;
  /**
   * Find the given mesh factory. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not NULL in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * NULL if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   */
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name,
  	iRegion* region = NULL) = 0;
  /**
   * Find the given camera position. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not NULL in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * NULL if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   */
  virtual iCameraPosition* FindCameraPosition (const char* name,
  	iRegion* region = NULL) = 0;
  /**
   * Find the given collection. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not NULL in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * NULL if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   */
  virtual iCollection* FindCollection (const char* name,
  	iRegion* region = NULL) = 0;

  /**
   * Set the mode for the lighting cache (combination of CS_ENGINE_CACHE_???).
   * Default is #CS_ENGINE_CACHE_READ | #CS_ENGINE_CACHE_NOUPDATE.
   * <ul>
   * <li>#CS_ENGINE_CACHE_READ: Read the cache.
   * <li>#CS_ENGINE_CACHE_WRITE: Write the cache.
   * <li>#CS_ENGINE_CACHE_NOUPDATE: Don't update lighting automatically
   *     if it is not up-to-date. This is on by default. If you disable
   *     this then lighting will be calculated even if CS_ENGINE_CACHE_WRITE
   *     is not set which means that the resulting calculation is not
   *     written to the cache.
   * </ul>
   */
  virtual void SetLightingCacheMode (int mode) = 0;
  /// Get the mode for the lighting cache.
  virtual int GetLightingCacheMode () = 0;

  /**
   * Set the thresshold (in number of polygons) after which the thing
   * mesh plugin will automatically switch to FASTMESH mode. If the number
   * of polygons is greater or equal compared to this thresshold then
   * CS_THING_FASTMESH will be made default. 500 is the default.
   */
  virtual void SetFastMeshThresshold (int th) = 0;
  /// Get the fastmesh thresshold.
  virtual int GetFastMeshThresshold () const = 0;

  /**
   * Require that the Z-buffer is cleared every frame. The engine
   * itself will not use this setting but will only return the
   * correct flag in GetBeginDrawFlags() so that the Z-buffer is actually
   * cleared. Note that this requires that the application actually
   * uses GetBeginDrawFlags() in the call to g3d->BeginDraw() (which it should).
   * By default this flag is false. It is useful to set this flag to true
   * if you have a level that doesn't itself have another way to initialize
   * the Z-buffer.
   */
  virtual void SetClearZBuf (bool yesno) = 0;

  /**
   * Get the value of the clear Z-buffer flag set with SetClearZBuf().
   */
  virtual bool GetClearZBuf () const = 0;
  /// Get default clear z-buffer flag.
  virtual bool GetDefaultClearZBuf () const = 0;

  /**
   * Require that the screen is cleared every frame. The engine
   * itself will not use this setting but will only return the
   * correct flag in GetBeginDrawFlags() so that the screen is actually
   * cleared. Note that this requires that the application actually
   * uses GetBeginDrawFlags() in the call to g3d->BeginDraw() (which it should).
   * By default this flag is false. It is useful to set this flag to true
   * if you have a level that doesn't itself have another way to initialize
   * the screen.
   */
  virtual void SetClearScreen (bool yesno) = 0;

  /**
   * Get the value of the clear screen flag set with SetClearScreen().
   */
  virtual bool GetClearScreen () const = 0;
  /// Get default clear screen flag
  virtual bool GetDefaultClearScreen () const = 0;

  /**
   * Set the maximum lightmap dimensions. Polys with lightmaps larger than
   * this are not lit.
   */
  virtual void SetMaxLightmapSize(int w, int h) = 0;
  /// Retrieve maximum lightmap size
  virtual void GetMaxLightmapSize(int& w, int& h) = 0;
  /// Retrieve default maximum lightmap size
  virtual void GetDefaultMaxLightmapSize(int& w, int& h) = 0;
  /// Get a boolean which indicates if power of two lightmaps are required.
  virtual bool GetLightmapsRequirePO2 () const = 0;
  /// Get the maximum aspect ratio for lightmaps.
  virtual int GetMaxLightmapAspectRatio () const = 0;
  
  /**
   * Reset a subset of flags/settings (which may differ from one world/map to 
   * another) to its defaults. This currently includes:
   *   - clear z buffer flag
   *   - lightmap cell size
   *   - maximum lighmap size
   */
  virtual void ResetWorldSpecificSettings() = 0;  

  /**
   * Create a new camera.
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iCamera> CreateCamera () = 0;
  /**
   * Create a static/pseudo-dynamic light. name can be NULL.
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iStatLight> CreateLight (const char* name, const csVector3& pos,
  	float radius, const csColor& color, bool pseudoDyn) = 0;
  /// Find a static/pseudo-dynamic light by name.
  virtual iStatLight* FindLight (const char *Name, bool RegionOnly = false)
    const = 0;
  /// Find a static/pseudo-dynamic light by id.
  virtual iStatLight* FindLight (unsigned long light_id) const = 0;
  /**
   * Create an iterator to iterate over all static lights of the engine.
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iLightIterator> GetLightIterator (iRegion* region = NULL) = 0;

  /**
   * Create a dynamic light. After creating a dynamic light you have to
   * call SetSector() on it. Do NOT add the light to the list of lights
   * in a sector. That list is only for static or pseudo-dynamic lights.
   * You also have to call Setup() on the dynamic light to actually calculate
   * the lighting. This must be redone everytime the radius or the position
   * changes (but not the color).
   */
  virtual csPtr<iDynLight> CreateDynLight (const csVector3& pos, float radius,
  	const csColor& color) = 0;
  /// Remove a dynamic light.
  virtual void RemoveDynLight (iDynLight*) = 0;
  /// Return the first dynamic light in this engine.
  virtual iDynLight* GetFirstDynLight () const = 0;

  /**
   * Get the required flags for 3D->BeginDraw() which should be called
   * from the application. These flags must be or-ed with optional other
   * flags that the application might be interested in. Use SetClearZBuf()
   * to let this function return that the Z-buffer must be cleared.
   */
  virtual int GetBeginDrawFlags () const = 0;

  /**
   * Get the top-level clipper.
   */
  virtual iClipper2D* GetTopLevelClipper () const = 0;

  /**
   * Conveniance function to create a mesh factory from a given type.
   * The type plugin will only be loaded if needed. 'classId' is the
   * SCF name of the plugin (like 'crystalspace.mesh.object.cube').
   * Returns NULL on failure. The factory will be registered with the engine
   * under the given name. If there is already a factory with that name
   * no new factory will be created but the found one is returned instead.
   * If the name is NULL then no name will be set and no check will happen
   * if the factory already exists.
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (const char* classId,
  	const char* name) = 0;

  /**
   * Create a mesh factory wrapper for an existing mesh factory
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (iMeshObjectFactory *,
  	const char* name) = 0;

  /**
   * Create an uninitialized mesh factory wrapper
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (const char* name) = 0;

  /**
   * Create a loader context that you can give to loader plugins.
   * It will basically allow loader plugins to find materials, ...
   * If region != NULL then only that region will
   * be searched. Assign to a csRef or use DecRef().
   */
  virtual csPtr<iLoaderContext> CreateLoaderContext (
  	iRegion* region = NULL) = 0;

  /**
   * Conveniance function to load a mesh factory from a given loader plugin.
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iMeshFactoryWrapper> LoadMeshFactory (
  	const char* name, const char* loaderClassId,
	iDataBuffer* input) = 0;

  /**
   * Conveniance function to create a mesh object for a given factory.
   * If 'sector' is NULL then the mesh object will not be set to a position.
   * Returns NULL on failure. The object will be given the specified name.
   * 'name' can be NULL if no name is wanted. Different mesh objects can
   * have the same name (in contrast with factory objects).
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iMeshWrapper> CreateMeshWrapper (iMeshFactoryWrapper* factory,
  	const char* name, iSector* sector = NULL,
	const csVector3& pos = csVector3(0, 0, 0)) = 0;
  /**
   * Create a mesh wrapper for an existing mesh object.
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iMeshWrapper> CreateMeshWrapper (iMeshObject*,
  	const char* name, iSector* sector = NULL,
	const csVector3& pos = csVector3(0, 0, 0)) = 0;
  /**
   * Create a mesh wrapper from a class id.
   * The type plugin will only be loaded if needed. 'classId' is the
   * SCF name of the plugin (like 'crystalspace.mesh.object.cube').
   * This function will first make a factory from the plugin and then
   * see if that factory itself implements iMeshObject too. This means
   * this function is useful to create thing mesh objects (which are both 
   * factory and object at the same time). If that fails this function
   * will call NewInstance() on the factory and return that object then.
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iMeshWrapper> CreateMeshWrapper (const char* classid,
  	const char* name, iSector* sector = NULL,
	const csVector3& pos = csVector3(0, 0, 0)) = 0;
  /**
   * Create an uninitialized mesh wrapper
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iMeshWrapper> CreateMeshWrapper (const char* name) = 0;

  /**
   * Conveniance function to load a mesh object from a given loader plugin.
   * If sector == NULL the object will not be placed in a sector.
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iMeshWrapper> LoadMeshWrapper (
  	const char* name, const char* loaderClassId,
	iDataBuffer* input, iSector* sector, const csVector3& pos) = 0;

  /**
   * Draw the 3D world given a camera and a clipper. Note that
   * in order to be able to draw using the given 3D driver
   * all textures must have been registered to that driver (using
   * Prepare()). Note that you need to call Prepare() again if
   * you switch to another 3D driver.
   */
  virtual void Draw (iCamera* c, iClipper2D* clipper) = 0;

  /**
   * Set the drawing context. This is a texture handle that is used
   * as the procedural texture to render on. If this is NULL then the
   * screen is assumed.
   */
  virtual void SetContext (iTextureHandle* ctxt) = 0;
  /// Return the current drawing context.
  virtual iTextureHandle *GetContext () const = 0;

  /**
   * Set the amount of ambient light. This has no effect until you
   * recalculate the lightmaps.
   */
  virtual void SetAmbientLight (const csColor &) = 0;
  /// Return the amount of ambient light
  virtual void GetAmbientLight (csColor &) const = 0;

  /**
   * This routine returns all lights which might affect an object
   * at some position according to the following flags:<br>
   * <ul>
   * <li>#CS_NLIGHT_SHADOWS: detect shadows and don't return lights for
   *     which the object is shadowed (not implemented yet).
   * <li>#CS_NLIGHT_STATIC: return static lights.
   * <li>#CS_NLIGHT_DYNAMIC: return dynamic lights.
   * <li>#CS_NLIGHT_NEARBYSECTORS: Also check lights in nearby sectors
   *     (not implemented yet).
   * </ul>
   * <br>
   * It will only return as many lights as the size that you specified
   * for the light array. The returned lights are not guaranteed to be sorted
   * but they are guaranteed to be the specified number of lights closest to
   * the given position.<br>
   * This function returns the actual number of lights added to the 'lights'
   * array.
   */
  virtual int GetNearbyLights (iSector* sector, const csVector3& pos,
  	uint32 flags, iLight** lights, int max_num_lights) = 0;

  /**
   * This routine returns an iterator to iterate over
   * all nearby sectors.
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iSectorIterator> GetNearbySectors (iSector* sector,
  	const csVector3& pos, float radius) = 0;

  /**
   * This routine returns an iterator to iterate over
   * all objects that are within a radius
   * of a given position. You can use #SCF_QUERY_INTERFACE to get
   * any interface from the returned objects.<p>
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iObjectIterator> GetNearbyObjects (iSector* sector,
    const csVector3& pos, float radius) = 0;

  /**
   * This routine returns an iterator to iterate over
   * all objects that are potentially visible as seen from a given position.
   * This routine assumes full 360 degree visibility.
   * You can use #SCF_QUERY_INTERFACE to get any interface from the
   * returned objects.<p>
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iObjectIterator> GetVisibleObjects (iSector* sector,
    const csVector3& pos) = 0;

  /**
   * This routine returns an iterator to iterate over
   * all objects that are potentially visible as seen from a given position.
   * This routine has a frustum restricting the view.
   * You can use #SCF_QUERY_INTERFACE to get any interface from the
   * returned objects.<p>
   * Assign to a csRef or use DecRef().
   */
  virtual csPtr<iObjectIterator> GetVisibleObjects (iSector* sector,
    const csFrustum& frustum) = 0;

  /**
   * Conveniance function to 'remove' a CS object from the engine.
   * This will not clear the object but it will remove all references
   * to that object that the engine itself keeps. This function works
   * for: iSector, iCollection, iMeshWrapper, iMeshFactoryWrapper,
   * iCameraPosition, iDynLight, iMaterialWrapper, and iTextureWrapper.
   * In addition this function also knows about iCurveTemplate and
   * iPolyTxtPlane from the thing environment and will be able to clean
   * those up too. Note that the object is only removed if the resulting
   * ref count will become zero. So basically this function only releases
   * the references that the engine holds.
   * <p>
   * This function returns true if the engine recognized the object as
   * one on which it can operate.
   * <p>
   * This function will also remove the object from the region it may be in.
   */
  virtual bool RemoveObject (iBase* object) = 0;

  /**
   * Set the cache manager that the engine will use. If this is not
   * done then the engine will use its own cache manager based on VFS.
   * This will do an incref on the given cache manager and a decref
   * on the old one. The engine will release the cache manager at
   * destruction time.
   */
  virtual void SetCacheManager (iCacheManager* cache_mgr) = 0;

  /**
   * Get the cache manager that the engine is currently using.
   */
  virtual iCacheManager* GetCacheManager () = 0;

  /// Return the default amount of ambient light
  virtual void GetDefaultAmbientLight (csColor &c) const = 0;

  /**
   * Create a iFrustumView instance that you can give to
   * iVisibilityCuller->CastShadows(). You can initialize that
   * instance so that your own function is called for every object
   * that is being visited.
   */
  virtual csPtr<iFrustumView> CreateFrustumView () = 0;

  /**
   * Create an object watcher instance that you can use to watch
   * other objects. The engine will not keep a reference to this object.
   */
  virtual csPtr<iObjectWatcher> CreateObjectWatcher () = 0;
};

/** @} */

#endif // __CS_IENGINE_ENGINE_H__
