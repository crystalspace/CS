/*
    Crystal Space 3D Engine
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

#ifndef __IENGINE_ENGINE_H__
#define __IENGINE_ENGINE_H__

#include "csutil/scf.h"
#include "csgeom/vector3.h"

class csEngine;
class csVector3;
class csMatrix3;
class csColor;
struct csTextureLayer;

struct iSector;
struct iSectorIterator;
struct iObjectIterator;
struct iLight;
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
struct iTextureList;
struct iCameraPosition;
struct iCameraPositionList;
struct iRegion;
struct iGraphics3D;
struct iClipper2D;
struct iObject;
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

/**
 * Flag for GetNearbyLights().
 * Detect shadows and don't return lights for which the object
 * is shadowed (not implemented yet).
 */
#define CS_NLIGHT_SHADOWS 1

/**
 * Flag for GetNearbyLights().
 * Return static lights.
 */
#define CS_NLIGHT_STATIC 2

/**
 * Flag for GetNearbyLights().
 * Return dynamic lights.
 */
#define CS_NLIGHT_DYNAMIC 4

/**
 * Flag for GetNearbyLights().
 * Also check lights in nearby sectors (not implemented yet).
 */
#define CS_NLIGHT_NEARBYSECTORS 8


/**
 * Setting for SetEngineMode().
 * Autodetect the best mode according to the level.
 */
#define CS_ENGINE_AUTODETECT 0

/**
 * Setting for SetEngineMode().
 * Use back-to-front rendering (using optional BSP/octree) and Z-fill.
 * Don't init Z-buffer at start of render frame.
 */
#define CS_ENGINE_BACK2FRONT 1

/**
 * Setting for SetEngineMode().
 * Use a 2D/3D culler (c-buffer) and front-to-back sorting.
 */
#define CS_ENGINE_FRONT2BACK 2

/**
 * Setting for SetEngineMode().
 * Use the Z-buffer for culling.
 */
#define CS_ENGINE_ZBUFFER 3

/**
 * Setting for SetLightingCacheMode().
 * Read the cache.
 */
#define CS_ENGINE_CACHE_READ 1

/**
 * Setting for SetLightingCacheMode().
 * Write the cache.
 */
#define CS_ENGINE_CACHE_WRITE 2

/**
 * Flag for RegisterRenderPriority. Do not sort this priority.
 */
#define CS_RENDPRI_NONE 0

/**
 * Flag for RegisterRenderPriority. Sort this priority back to front.
 */
#define CS_RENDPRI_BACK2FRONT 1

/**
 * Flag for RegisterRenderPriority. Sort this priority front to back.
 */
#define CS_RENDPRI_FRONT2BACK 2

/**
 * Flags for the callbacks called via iEngine::DrawFunc().
 * (type iDrawFuncCallback).
 */
#define CS_CALLBACK_SECTOR 1
#define CS_CALLBACK_SECTOREXIT 2
#define CS_CALLBACK_MESH 3
#define CS_CALLBACK_VISMESH 4

SCF_VERSION (iDrawFuncCallback, 0, 0, 1);

/**
 * A callback function for csEngine::DrawFunc().
 */
struct iDrawFuncCallback : public iBase
{
  /**
   * Before drawing.
   * The 'type' will be one of the following:
   * <ul>
   * <li>CS_CALLBACK_SECTOR: a sector is being drawn.
   * <li>CS_CALLBACK_SECTOREXIT: a sector has been drawn completely.
   * <li>CS_CALLBACK_MESH: a mesh will be drawn if visible.
   * <li>CS_CALLBACK_VISMESH: the mesh is visible and will be drawn.
   * </ul>
   */
  virtual void DrawFunc (iRenderView* rview, int type, void* entity) = 0;
};


SCF_VERSION (iEngine, 0, 7, 1);

/**
 * This interface is the main interface to the 3D engine.
 * The engine is responsible for creating new engine-specific objects
 * such as sectors, things, sprites and so on.
 */
struct iEngine : public iBase
{
  /**
   * @@@KLUDGE: This will no longer be needed once the iEngine interface is
   * complete.
   */
  virtual csEngine *GetCsEngine () = 0;

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
   * Return true if engine want to use PVS.
   */
  virtual bool IsPVS () const = 0;

  /**
   * Create or select a new region (name can be NULL for the default main
   * region). All new objects will be marked as belonging to this region.
   */
  virtual void SelectRegion (const char* iName) = 0;
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

  /// Delete everything in the engine.
  virtual void DeleteAll () = 0;

  /**
   * Register a new render priority.
   * The parameter rendsort is one of the CS_RENDPRI_... flags.
   * By default this is CS_RENDPRI_NONE. The following values are possible:
   * <ul>
   * <li>CS_RENDPRI_NONE: objects in this render priority are not sorted.
   * <li>CS_RENDPRI_FRONT2BACK: sort objects front to back (as seen from
   *     camera viewpoint).
   * <li>CS_RENDPRI_BACK2FRONT: sort objects back to front.
   * </ul>
   */
  virtual void RegisterRenderPriority (const char* name, long priority,
  	int rendsort = CS_RENDPRI_NONE) = 0;
  /// Get a render priority by name.
  virtual long GetRenderPriority (const char* name) const = 0;
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
   * manager.
   */
  virtual iMaterial* CreateBaseMaterial (iTextureWrapper* txt) = 0;

  /**
   * Create a base material that can be used to give to the texture
   * manager. This version also supports texture layers.
   */
  virtual iMaterial* CreateBaseMaterial (iTextureWrapper* txt,
  	int num_layers, iTextureWrapper** wrappers, csTextureLayer* layers) = 0;

  /// Register a texture to be loaded during Prepare()
  virtual iTextureWrapper* CreateTexture (const char *iName,
  	const char *iFileName, csColor *iTransp, int iFlags) = 0;
  /// Register a material to be loaded during Prepare()
  virtual iMaterialWrapper* CreateMaterial (const char *iName,
  	iTextureWrapper* texture) = 0;
  /**
   * Create a empty sector with given name.
   * If link == true (default) the sector will be linked to the engine.
   */
  virtual iSector *CreateSector (const char *iName, bool link = true) = 0;

  /**
   * Conveniance function to create the thing containing the
   * convex outline of a sector. The thing will be empty but
   * it will have CS_ZBUF_FILL set (so that the Z-buffer will be filled
   * by the polygons of this object) and have 'wall' as render
   * priority. This version creates a mesh wrapper.
   */
  virtual iMeshWrapper* CreateSectorWallsMesh (iSector* sector,
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
  /// Get the list of all regions
  virtual iRegionList* GetRegions () = 0;

  /**
   * Set the mode for the lighting cache (combination of CS_ENGINE_CACHE_???).
   * Default is CS_ENGINE_CACHE_READ.
   * <ul>
   * <li>CS_ENGINE_CACHE_READ: Read the cache.
   * <li>CS_ENGINE_CACHE_WRITE: Write the cache.
   * </ul>
   */
  virtual void SetLightingCacheMode (int mode) = 0;
  /// Get the mode for the lighting cache.
  virtual int GetLightingCacheMode () = 0;

  /// Return the current lightmap cell size
  virtual int GetLightmapCellSize () const = 0;
  /// Set lightmap cell size
  virtual void SetLightmapCellSize (int Size) = 0;

  /// Create a new camera.
  virtual iCamera* CreateCamera () = 0;
  /// Create a static/pseudo-dynamic light. name can be NULL.
  virtual iStatLight* CreateLight (const char* name, const csVector3& pos,
  	float radius, const csColor& color, bool pseudoDyn) = 0;
  /// Find a static/pseudo-dynamic light by name.
  virtual iStatLight* FindLight (const char *Name, bool RegionOnly = false)
    const = 0;
  /// Create a dynamic light.
  virtual iDynLight* CreateDynLight (const csVector3& pos, float radius,
  	const csColor& color) = 0;
  /// Remove a dynamic light.
  virtual void RemoveDynLight (iDynLight*) = 0;

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

  /**
   * Get the required flags for 3D->BeginDraw() which should be called
   * from the application. These flags must be or-ed with optional other
   * flags that the application might be interested in. Use SetClearZBuf()
   * to let this function return that the Z-buffer must be cleared.
   */
  virtual int GetBeginDrawFlags () const = 0;

  /**
   * Set the desired engine mode.
   * One of the CS_ENGINE_... flags. Default is CS_ENGINE_AUTODETECT.
   * <ul>
   * <li>CS_ENGINE_AUTODETECT: try to auto-detect the best mode to use
   *     for rendering this level (also depends on hardware capabilities).
   *     This is calculated the first time iEngine->Draw() is called.
   * <li>CS_ENGINE_BACK2FRONT: Render polygons back to front (optionally
   *     using octree/bsp for this).
   * <li>CS_ENGINE_FRONT2BACK: Use the c-buffer for culling polygons and
   *     render front to back (only if octree/bsp tree is available).
   * <li>CS_ENGINE_ZBUFFER: Use the Z-buffer for rendering.
   * </ul>
   */
  virtual void SetEngineMode (int mode) = 0;

  /**
   * Get the current engine mode.
   * If called between SetEngineMode() and the first Draw() it is
   * possible that this mode will still be CS_ENGINE_AUTODETECT.
   */
  virtual int GetEngineMode () const = 0;

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
   */
  virtual iMeshFactoryWrapper* CreateMeshFactory (const char* classId,
  	const char* name) = 0;

  /// Create a mesh factory wrapper for an existing mesh factory
  virtual iMeshFactoryWrapper* CreateMeshFactory (iMeshObjectFactory *,
  	const char* name) = 0;

  /// Create an uninitialized mesh factory wrapper
  virtual iMeshFactoryWrapper* CreateMeshFactory (const char* name) = 0;

  /**
   * Conveniance function to load a mesh factory from a given loader plugin.
   */
  virtual iMeshFactoryWrapper* LoadMeshFactory (
  	const char* classId, const char* name,
	const char* loaderClassId,
	iDataBuffer* input) = 0;

  /**
   * Conveniance function to create a mesh object for a given factory.
   * If 'sector' is NULL then the mesh object will not be set to a position.
   * Returns NULL on failure. The object will be given the specified name.
   * 'name' can be NULL if no name is wanted. Different mesh objects can
   * have the same name (in contrast with factory objects).
   */
  virtual iMeshWrapper* CreateMeshWrapper (iMeshFactoryWrapper* factory,
  	const char* name, iSector* sector = NULL,
	const csVector3& pos = csVector3(0, 0, 0)) = 0;
  /// Create a mesh wrapper for an existing mesh object
  virtual iMeshWrapper* CreateMeshWrapper (iMeshObject*,
  	const char* name, iSector* sector = NULL,
	const csVector3& pos = csVector3(0, 0, 0)) = 0;
  /// Create an uninitialized mesh wrapper
  virtual iMeshWrapper* CreateMeshWrapper (const char* name) = 0;
  /**
   * Conveniance function to load a mesh object from a given loader plugin.
   * If sector == NULL the object will not be placed in a sector.
   */
  virtual iMeshWrapper* LoadMeshWrapper (
  	const char* classId, const char* name,
	const char* loaderClassId,
	iDataBuffer* input, iSector* sector, const csVector3& pos) = 0;

  /**
   * @@@ This function is deprecated! Please don't use it!
   * Only the engine and thing mesh objects should use this now.
   */
  virtual iMeshObjectType* GetThingType () const = 0;

  /**
   * Draw the 3D world given a camera and a clipper. Note that
   * in order to be able to draw using the given 3D driver
   * all textures must have been registered to that driver (using
   * Prepare()). Note that you need to call Prepare() again if
   * you switch to another 3D driver.
   */
  virtual void Draw (iCamera* c, iClipper2D* clipper) = 0;

  /**
   * This function is similar to Draw. It will do all the stuff
   * that Draw would do except for one important thing: it will
   * not draw anything. Instead it will call a callback function for
   * every entity that it was planning to draw. This allows you to show
   * or draw debugging information (2D edges for example).
   */
  virtual void DrawFunc (iCamera* c, iClipper2D* clipper,
    iDrawFuncCallback* callback) = 0;

  /// Set the drawing context
  virtual void SetContext (iGraphics3D*) = 0;
  /// Return the current drawing context
  virtual iGraphics3D *GetContext () const = 0;

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
   * <li>CS_NLIGHT_SHADOWS: detect shadows and don't return lights for
   *     which the object is shadowed (not implemented yet).
   * <li>CS_NLIGHT_STATIC: return static lights.
   * <li>CS_NLIGHT_DYNAMIC: return dynamic lights.
   * <li>CS_NLIGHT_NEARBYSECTORS: Also check lights in nearby sectors
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
   * Delete the iterator with 'DecRef()' when ready.
   */
  virtual iSectorIterator* GetNearbySectors (iSector* sector,
  	const csVector3& pos, float radius) = 0;

  /**
   * This routine returns an iterator to iterate over
   * all objects of a given type that are within a radius
   * of a given position. You can use SCF_QUERY_INTERFACE to get
   * any interface from the returned objects.<p>
   * Delete the iterator with 'DecRef()' when ready.
   */
  virtual iObjectIterator* GetNearbyObjects (iSector* sector,
    const csVector3& pos, float radius) = 0;

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
};

#endif // __IENGINE_ENGINE_H__
