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
 
#include "csutil/scf_interface.h"

#include "csgeom/vector3.h"

#include "iengine/light.h"

class csBox3;
class csColor;
class csFrustum;

struct iCacheManager;
struct iCamera;
struct iCameraPosition;
struct iCameraPositionList;
struct iClipper2D;
struct iCollection;
struct iCollectionList;
struct iDataBuffer;
struct iFrustumView;
struct iLight;
struct iLightIterator;
struct iLoaderContext;
struct iMaterial;
struct iMaterialList;
struct iMaterialWrapper;
struct iMeshFactoryList;
struct iMeshFactoryWrapper;
struct iMeshList;
struct iMeshObject;
struct iMeshObjectFactory;
struct iMeshObjectType;
struct iMeshWrapper;
struct iMeshWrapperIterator;
struct iObject;
struct iObjectIterator;
struct iObjectWatcher;
struct iPortal;
struct iProgressMeter;
struct iRegion;
struct iRegionList;
struct iRenderLoop;
struct iRenderLoopManager;
struct iRenderView;
struct iSector;
struct iSectorIterator;
struct iSectorList;
struct iSharedVariableList;
struct iTextureHandle;
struct iTextureList;
struct iTextureWrapper;

struct iEngine;

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
enum csRenderPrioritySorting
{
  /**
  * Do not sort this priority.
  */
  CS_RENDPRI_SORT_NONE = 0,

  /**
  * Sort this priority back to front.
  */
  CS_RENDPRI_SORT_BACK2FRONT = 1,

  /**
  * Sort this priority front to back.
  */
  CS_RENDPRI_SORT_FRONT2BACK = 2
};
/** @} */


/**
 * A callback that will be fired whenever a sector is created or
 * removed from the engine.
 */
struct iEngineSectorCallback : public virtual iBase
{
  SCF_INTERFACE(iEngineSectorCallback,2,0,0);
  /**
   * New sector.
   */
  virtual void NewSector (iEngine* engine, iSector* sector) = 0;

  /**
   * Remove sector.
   */
  virtual void RemoveSector (iEngine* engine, iSector* sector) = 0;
};


/**
 * This interface is the main interface to the 3D engine.
 * The engine is responsible for creating new engine-specific objects
 * such as sectors, mesh objects, mesh object factories, lights, and so on.
 *
 * Main creators of instances implementing this interface:
 * - 3D Engine plugin (crystalspace.engine.3d)
 *
 * Main ways to get pointers to this interface:
 * - CS_QUERY_REGISTRY()
 *
 * Main users of this interface:
 * - Application.
 */
struct iEngine : public virtual iBase
{
  SCF_INTERFACE(iEngine,2,0,0);
  
  /// Get the iObject for the engine.
  virtual iObject *QueryObject() = 0;

  /**\name Preparation and relighting methods
   * @{ */
  
  /**
   * Prepare the engine. This function must be called after
   * you loaded/created the world. It will prepare all lightmaps
   * for use and also free all images that were loaded for
   * the texture manager (the texture manager should have them
   * locally now). The optional progress meter will be used to
   * report progress.
   * <p>
   * The behaviour regarding cached lighting depends on the flag
   * you can set with the SetLightingCacheMode() function. The default
   * behaviour is to read the lightmap cache when present but don't
   * calculate lighting if cache is not present.
   * \param meter If supplied, the meter object will be called back
   * periodically to report the progress of engine preparation.
   */
  virtual bool Prepare (iProgressMeter* meter = 0) = 0;

  /**
   * Prepare the textures. It will initialise all loaded textures
   * for the texture manager. (Normally you shouldn't call this function
   * directly, because it will be called by Prepare() for you.
   * This function will also prepare all loaded materials after preparing
   * the textures.
   */
  virtual void PrepareTextures () = 0;

  /**
   * Calls UpdateMove for all meshes to initialise bounding boxes.
   * Prepare() will call this function automatically so you normally 
   * don't have to call it.
   */
  virtual void PrepareMeshes () = 0;

  /**
   * Force a relight of all lighting. It is better to call this instead
   * of calling engine->Prepare() again as engine->Prepare() will also do
   * other stuff (like registering textures). Warning! This function can
   * be very slow (depending on the number of lights and objects in the
   * world). The optional region can be given to limit calculation to
   * objects in the region.
   * <p>
   * The current flags set with SetLightingCacheMode() controls if the
   * lightmaps will be cached or not.
   * \param region only relight objects in this region 
   * (will relight every object in the engine by default)
   * \param meter If supplied, the meter object will be called back
   * periodically to report the progress of engine lighting calculation.
   */
  virtual void ForceRelight (iRegion* region = 0,
  	iProgressMeter* meter = 0) = 0;

  /**
   * Force a relight for the given light. This is useful to update the
   * lightmaps after a static or pseudo-dynamic light has been added (don't
   * use this for dynamic lights). If there are a lot of objects this function
   * can be slow. The optional region can be given to limit calculation to
   * objects in the region.
   * <p>
   * The current flags set with SetLightingCacheMode() controls if the
   * lightmaps will be cached or not.
   * \param light The newly added light to shine
   * \param region If supplied, only affect objects in this region
   */
  virtual void ForceRelight (iLight* light, iRegion* region = 0) = 0;

  /**
   * Calculate all lighting information. Normally you shouldn't call
   * this function directly, because it will be called by Prepare().
   * If the optional 'region' parameter is given then only lights will
   * be recalculated for the given region.
   * \param region If supplied, only calculate lighting for lights and objects
   * in the given region.
   * \param meter If supplied, the meter object will be called back
   * periodically to report the progress of engine lighting calculation.
   */
  virtual void ShineLights (iRegion* region = 0,
  	iProgressMeter* meter = 0) = 0;

  /**
   * Set the mode for the lighting cache (combination of CS_ENGINE_CACHE_???).
   * Default is #CS_ENGINE_CACHE_READ | #CS_ENGINE_CACHE_NOUPDATE.
   * \param mode 
   *  - #CS_ENGINE_CACHE_READ: Read the cache.
   *  - #CS_ENGINE_CACHE_WRITE: Write the cache.
   *  - #CS_ENGINE_CACHE_NOUPDATE: Don't update lighting automatically
   *     if it is not up-to-date. This is on by default. If you disable
   *     this then lighting will be calculated even if CS_ENGINE_CACHE_WRITE
   *     is not set which means that the resulting calculation is not
   *     written to the cache.
   */
  virtual void SetLightingCacheMode (int mode) = 0;

  /// Get the mode for the lighting cache.
  virtual int GetLightingCacheMode () = 0;

  /**
   * Set the cache manager that the engine will use. If this is not
   * done then the engine will use its own cache manager based on VFS.
   * This will do an incref on the given cache manager and a decref
   * on the old one. The engine will release the cache manager at
   * destruction time. To set the cache manager to the default VFS
   * based cache manager for a given VFS directory you can use the
   * following code:
   * \code
   * engine->SetVFSCacheManager ("/bla/bla");
   * \endcode
   */
  virtual void SetCacheManager (iCacheManager* cache_mgr) = 0;

  /**
   * Set the cache manager to the default VFS based cache manager.
   * Note that this function will not change the VFS current directory.
   * \param vfspath is the path that will be used for the cache manager.
   * If 0 then the current VFS directory will be used instead.
   */
  virtual void SetVFSCacheManager (const char* vfspath = 0) = 0;

  /**
   * Get the cache manager that the engine is currently using.
   */
  virtual iCacheManager* GetCacheManager () = 0;

  /**
   * Set the maximum lightmap dimensions. Polys with lightmaps larger than
   * this are not lit.  
   * \param w lightmap width 
   * \param h lightmap height
   */
  virtual void SetMaxLightmapSize (int w, int h) = 0;

  /** Retrieve maximum lightmap size.
   * \param w lightmap width
   * \param h lightmap height
  */
  virtual void GetMaxLightmapSize (int& w, int& h) = 0;

  /** Retrieve default maximum lightmap size.  
   * \param w lightmap width
   * \param h lightmap height
  */
  virtual void GetDefaultMaxLightmapSize (int& w, int& h) = 0;

  /// Get the maximum aspect ratio for lightmaps.
  virtual int GetMaxLightmapAspectRatio () const = 0;
  
  /** @} */

  /**\name Render priority functions
   * @{ */

  /**
   * Register a new render priority.  Render priorities are assigned to objects
   * and controls the order in which objects are rendered by the engine.
   * \param name a name to refer to this render priority
   * \param priority a numerical priority; this is used to order the
   * render priorities where lower numbers are rendered before higher 
   * numbers.
   * \param rendsort One of the CS_RENDPRI_... flags.
   * By default this is #CS_RENDPRI_SORT_NONE. The following values are
   * possible:
   * - #CS_RENDPRI_SORT_NONE: objects in this render priority are not sorted.
   * - #CS_RENDPRI_SORT_FRONT2BACK: sort objects front to back (as seen from
   *   camera viewpoint).
   * - #CS_RENDPRI_SORT_BACK2FRONT: sort objects back to front.
   *
   * \note The default render priorities are 'sky', 'portal', 'wall', 'object' 
   * and 'alpha' (in that priority order, where sky is rendered first and 
   * alpha is rendered last).  Should you wish to add your own render 
   * priority, you must call ClearRenderPriorities() and re-add the 
   * default render priorities along with your own new priorities.
   */
  virtual void RegisterRenderPriority (const char* name, long priority,
  	csRenderPrioritySorting rendsort = CS_RENDPRI_SORT_NONE) = 0;

  /**
   * Get a render priority by name.
   * \param name is the name you want (usually one of 'sky', 'portal',
   * 'wall', 'object', or 'alpha' but you can define your own render
   * priorities).
   * \return 0 if render priority doesn't exist.
   */
  virtual long GetRenderPriority (const char* name) const = 0;
  /// Get the render priority sorting flag.
  virtual csRenderPrioritySorting GetRenderPrioritySorting (
  	const char* name) const = 0;
  /// Get the render priority sorting flag.
  virtual csRenderPrioritySorting GetRenderPrioritySorting (
  	long priority) const = 0;
  /// Get the render priority for sky objects (attached to 'sky' name).
  virtual long GetSkyRenderPriority () = 0;
  /// Get the render priority for portal objects (attached to 'portal' name).
  virtual long GetPortalRenderPriority () = 0;
  /// Get the render priority for wall objects (attached to 'wall' name).
  virtual long GetWallRenderPriority () = 0;
  /// Get the render priority for general objects (attached to 'object' name).
  virtual long GetObjectRenderPriority () = 0;
  /// Get the render priority for alpha objects (attached to 'alpha' name).
  virtual long GetAlphaRenderPriority () = 0;
  /// Clear all render priorities.
  virtual void ClearRenderPriorities () = 0;
  /// Get the number of render priorities.
  virtual int GetRenderPriorityCount () const = 0;
  /// Get the name of the render priority or 0 if none existant.
  virtual const char* GetRenderPriorityName (long priority) const = 0;

  /** @} */
  
  /**\name Material handling
   * @{ */

  /**
   * Create a base material that can be used to give to the texture
   * manager. Assign to a csRef.
   * \param txt The texture map this material will use. Note that this
   * can be 0 in which case a base material without texture will be created.
   * \note You will need to call iMaterialWrapper::Register() and 
   * iMaterialWrapper::GetMaterialHandler()->Prepare() on you new material
   * if you load the material after iEngine::Prepare() has been called.
   */
  virtual csPtr<iMaterial> CreateBaseMaterial (iTextureWrapper* txt) = 0;

  /**
   * Register a material to be loaded during Prepare()
   * \param name engine name for this material
   * \param texture texture to use for this material
   */
  virtual iMaterialWrapper* CreateMaterial (const char *name,
  	iTextureWrapper* texture) = 0;

  /// Get the list of all materials.
  virtual iMaterialList* GetMaterialList () const = 0;

  /**
   * Find the given material. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not 0 in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * 0 if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   * \param name the engine name of the desired material
   * \param region if specified, search only this region (also see note above)
   */
  virtual iMaterialWrapper* FindMaterial (const char* name,
  	iRegion* region = 0) = 0;

  /** @} */
  
  /**\name Texture handling
   * @{ */

  /**
   * Create a texture from a file.
   * \param name The name to use for this texture in the engine
   * \param fileName the filename (on the VFS!) of the texture to load
   * \param transp pixels in the image with this key color will be considered
   * transparent instead of being drawn
   * \param flags One or more texturing flags OR'd together, flag include
   * - CS_TEXTURE_2D image will be used only for 2D drawing
   * - CS_TEXTURE_3D image will be textured onto 3D polygon
   *   (this is almost always the flag you want)
   * - CS_TEXTURE_DITHER texture will be dithered before use
   * - CS_TEXTURE_NOMIPMAPS texture will not be mipmapped before use
   *
   * \note You will need to call iTextureWrapper::Register()
   * on you new texture if you load the texture after iEngine::Prepare()
   * has been called.
   */
  virtual iTextureWrapper* CreateTexture (const char *name,
  	const char *fileName, csColor *transp, int flags) = 0;

  /**
   * Create a black texture. This is mostly useful for procedural textures.
   * \param name The name to use for this texture in the engine
   * \param w the texture width (must be a power of 2, eg 64, 128, 256, 512...)
   * \param h the texture height (must be a power of 2, eg 64, 128, 256, 512...)
   * \param transp pixels in the image with this key color will be considered
   * transparent instead of being drawn
   * \param flags see CreateTexture()
   * \see CreateTexture() note about registering textures.
   */
  virtual iTextureWrapper* CreateBlackTexture (const char *name,
	int w, int h, csColor *transp, int flags) = 0;

  /**
   * Query the format to load textures (usually this depends on texture
   * manager)
   */
  virtual int GetTextureFormat () const = 0;

  /// Get the list of all textures.
  virtual iTextureList* GetTextureList () const = 0;

  /**
   * Find the given texture. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not 0 in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * 0 if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   * \param name the engine name of the desired texture
   * \param region if specified, search only this region (also see note above)
   */
  virtual iTextureWrapper* FindTexture (const char* name,
  	iRegion* region = 0) = 0;

  /** @} */
  
  /**\name Light handling
   * @{ */

  /**
   * Create a static/pseudo-dynamic light.
   * Assign to a csRef.
   * \param name the engine name for this light (may be 0)
   * \param pos the position of this light in world coordinates
   * \param radius the maximum distance at which this light will affect
   * objects 
   * \param color the color of this light (also affects light intensity)
   * \param dyntype is the type of the light. This can be
   * #CS_LIGHT_DYNAMICTYPE_DYNAMIC, #CS_LIGHT_DYNAMICTYPE_PSEUDO,
   * or #CS_LIGHT_DYNAMICTYPE_STATIC.
   * Note that after creating a light you must add it to a sector
   * by calling sector->GetLights ()->Add (light);
   * If the light is dynamic you also must call Setup() to calculate lighting.
   * Otherwise you must use engine->ForceRelight() if you create a light
   * after calling engine->Prepare(). Otherwise you can let engine->Prepare()
   * do it.
   * <p>
   * Note! If you are using a system with hardware accelerated lighting
   * (i.e. no lightmaps) then the discussion above is not relevant.
   */
  virtual csPtr<iLight> CreateLight (const char* name, const csVector3& pos,
  	float radius, const csColor& color,
	csLightDynamicType dyntype = CS_LIGHT_DYNAMICTYPE_STATIC) = 0;

  /** Find a static/pseudo-dynamic light by name.
   * \param Name the engine name of the desired light
   * \param RegionOnly (parameter presently unused)
   */
  virtual iLight* FindLight (const char *Name, bool RegionOnly = false)
    const = 0;

  /**
   * Find a static/pseudo-dynamic light by id. 
   * \param light_id a 16-byte MD5 checksum for the light.
   */
  virtual iLight* FindLightID (const char* light_id) const = 0;

  /**
   * Create an iterator to iterate over all static lights of the engine.
   * Assign to a csRef.
   * \param region only iterate over the lights in this region
   * (otherwise iterate over all lights)
   */
  virtual csPtr<iLightIterator> GetLightIterator (iRegion* region = 0) = 0;

  /**
   * Remove a light and update all lightmaps. This function only works
   * correctly for dynamic or pseudo-dynamic static lights. If you give a normal
   * static light then the light will be removed but lightmaps will not
   * be affected. You can call ForceRelight() to force relighting then.
   * <p>
   * The current flags set with SetLightingCacheMode() controls if the
   * lightmaps will be cached or not.
   * \param light the light to remove
   */
  virtual void RemoveLight (iLight* light) = 0;

  /**
   * Set the amount of ambient light. This has no effect until you
   * recalculate the lightmaps.
   */
  virtual void SetAmbientLight (const csColor &) = 0;
  /// Return the amount of ambient light
  virtual void GetAmbientLight (csColor &) const = 0;
  /// Return the default amount of ambient light
  virtual void GetDefaultAmbientLight (csColor &c) const = 0;

  /**
   * This routine returns all lights which might affect an object
   * at some position.
   * <br>
   * It will only return as many lights as the size that you specified
   * for the light array. The returned lights are not guaranteed to be sorted
   * but they are guaranteed to be the specified number of lights closest to
   * the given position.<br>
   * This function returns the actual number of lights added to the 'lights'
   * array.
   */
  virtual int GetNearbyLights (iSector* sector, const csVector3& pos,
  	iLight** lights, int max_num_lights) = 0;

  /**
   * This routine returns all lights which might affect an object
   * with some bounding box.
   * <br>
   * It will only return as many lights as the size that you specified
   * for the light array. The returned lights are not guaranteed to be sorted
   * but they are guaranteed to be the specified number of lights closest to
   * the given position.<br>
   * This function returns the actual number of lights added to the 'lights'
   * array.
   */
  virtual int GetNearbyLights (iSector* sector, const csBox3& box,
  	iLight** lights, int max_num_lights) = 0;

  /** @} */
  
  /**\name Sector handling
   * @{ */

  /**
   * Create a empty sector with given name.
   * \param name the sector name
   */
  virtual iSector *CreateSector (const char *name) = 0;

  /// Get the list of sectors
  virtual iSectorList* GetSectors () = 0;

  /**
   * Find the given sector. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not 0 in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * 0 if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   * \param name the engine name of the desired sector
   * \param region if specified, search only this region (also see note above)
   */
  virtual iSector* FindSector (const char* name,
  	iRegion* region = 0) = 0;

  /**
   * This routine returns an iterator to iterate over
   * all nearby sectors.
   * Assign to a csRef.
   */
  virtual csPtr<iSectorIterator> GetNearbySectors (iSector* sector,
  	const csVector3& pos, float radius) = 0;

  /**
   * Add a sector callback. This will call IncRef() on the callback
   * So make sure you call DecRef() to release your own reference.
   */
  virtual void AddEngineSectorCallback (iEngineSectorCallback* cb) = 0;

  /**
   * Remove a sector callback.
   */
  virtual void RemoveEngineSectorCallback (iEngineSectorCallback* cb) = 0;

  /** @} */

  /**\name Mesh handling
   * @{ */

  /**
   * Convenience function to create a mesh object for a given factory.
   * 
   * \param factory the factory that will produce this mesh
   * \param name The engine name for the mesh wrapper; may be null.  
   * Different mesh objects can have the same name (in contrast 
   * with factory objects).
   * \param sector the sector to initially place this mesh in
   * If 'sector' is 0 then the mesh object will not be set to a position.
   * \param pos the position in the sector
   * \return The meshwrapper on success (assign to a csRef),
   * or 0 on failure.
   */
  virtual csPtr<iMeshWrapper> CreateMeshWrapper (iMeshFactoryWrapper* factory,
  	const char* name, iSector* sector = 0,
	const csVector3& pos = csVector3 (0, 0, 0)) = 0;

  /**
   * Create a mesh wrapper for an existing mesh object.
   * \param meshobj the mesh object
   * \param name The engine name for the mesh wrapper; may be null.  
   * Different mesh objects can have the same name (in contrast 
   * with factory objects).
   * \param sector the sector to initially place this mesh in
   * If 'sector' is 0 then the mesh object will not be set to a position.
   * \param pos the position in the sector
   * \return The meshwrapper on success (assign to a csRef),
   * or 0 on failure.
   */
  virtual csPtr<iMeshWrapper> CreateMeshWrapper (iMeshObject* meshobj,
  	const char* name, iSector* sector = 0,
	const csVector3& pos = csVector3 (0, 0, 0)) = 0;

  /**
   * Create a mesh wrapper from a class id.
   * This function will first make a factory from the plugin and then
   * see if that factory itself implements iMeshObject too. This means
   * this function is useful to create thing mesh objects (which are both 
   * factory and object at the same time). If that fails this function
   * will call NewInstance() on the factory and return that object then.
   * \param classid The SCF name of the plugin 
   * (like 'crystalspace.mesh.object.ball').  The type plugin will only 
   * be loaded if needed.
   * \param name The engine name for the mesh wrapper; may be null.  
   * Different mesh objects can have the same name (in contrast 
   * with factory objects).
   * \param sector the sector to initially place this mesh in
   * If 'sector' is 0 then the mesh object will not be set to a position.
   * \param pos the position in the sector
   * \return The meshwrapper on success (assign to a csRef),
   * or 0 on failure.
   */
  virtual csPtr<iMeshWrapper> CreateMeshWrapper (const char* classid,
  	const char* name, iSector* sector = 0,
	const csVector3& pos = csVector3 (0, 0, 0)) = 0;

  /**
   * Create an uninitialized mesh wrapper
   * Assign to a csRef.
   */
  virtual csPtr<iMeshWrapper> CreateMeshWrapper (const char* name) = 0;

  /**
   * Convenience function to create the thing containing the
   * convex outline of a sector. The thing will be empty but
   * it will have #CS_ZBUF_FILL set (so that the Z-buffer will be filled
   * by the polygons of this object) and have 'wall' as render
   * priority. This version creates a mesh wrapper.
   * Assign to a csRef.
   * \param sector the sector to add walls to
   * \param name the engine name of the walls mesh that will be created
   */
  virtual csPtr<iMeshWrapper> CreateSectorWallsMesh (iSector* sector,
      const char* name) = 0;

  /**
   * Convenience function to create a thing mesh in a sector.
   * This mesh will have #CS_ZBUF_USE set (use Z-buffer fully)
   * and have 'object' as render priority. This means this function
   * is useful for general objects.
   * Assign to a csRef.
   * \param sector the sector to add the object to
   * \param name the engine name of the mesh that will be created
   */
  virtual csPtr<iMeshWrapper> CreateThingMesh (iSector* sector,
  	const char* name) = 0;

  /**
   * Convenience function to load a mesh object from a given loader plugin.
   * \param name The engine name for the mesh wrapper; may be null.  
   * Different mesh objects can have the same name (in contrast 
   * with factory objects).
   * \param loaderClassId the SCF class of the loader to use to 
   * create the meshwrapper
   * \param input data passed to the loader to generate the mesh
   * \param sector the sector to initially place this mesh in
   * If 'sector' is 0 then the mesh object will not be set to a position.
   * \param pos the position in the sector
   * \return The meshwrapper on success (assign to a csRef),
   * or 0 on failure.
   */
  virtual csPtr<iMeshWrapper> LoadMeshWrapper (
  	const char* name, const char* loaderClassId,
	iDataBuffer* input, iSector* sector, const csVector3& pos) = 0;

  /**
   * Convenience function to add a mesh and all children of that
   * mesh to the engine.
   */
  virtual void AddMeshAndChildren (iMeshWrapper* mesh) = 0;

  /**
   * This routine returns an iterator to iterate over
   * all meshes that are within a radius of a given position.
   * If crossPortals is true it will search through
   * portals. Otherwise it will limit the search to the sector passed in.
   */
  virtual csPtr<iMeshWrapperIterator> GetNearbyMeshes (iSector* sector,
    const csVector3& pos, float radius, bool crossPortals = true ) = 0;

  /**
   * This routine returns an iterator to iterate over
   * all meshes that are in a box.
   * If crossPortals is true it will search through
   * portals. Otherwise it will limit the search to the sector passed in.
   * Portal visibility is tested with the center of the box.
   */
  virtual csPtr<iMeshWrapperIterator> GetNearbyMeshes (iSector* sector,
    const csBox3& box, bool crossPortals = true ) = 0;

  /// Get the list of meshes
  virtual iMeshList* GetMeshes () = 0;

  /**
   * Find the given mesh object. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not 0 in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * 0 if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   * \param name the engine name of the desired mesh
   * \param region if specified, search only this region (also see note above)
   */
  virtual iMeshWrapper* FindMeshObject (const char* name,
  	iRegion* region = 0) = 0;

  /**
   * Sometimes a mesh wants to destruct itself (for example
   * a particle system that has only limited lifetime). It can do that
   * by calling this function on itself. The engine will then remove
   * the object before the next frame.
   */
  virtual void WantToDie (iMeshWrapper* mesh) = 0;

  /** @} */
  
  /**\name Mesh factory handling
   * @{ */

  /**
   * Convenience function to create a mesh factory from a given type.
   * \param classId the SCF name of the plugin 
   * (like 'crystalspace.mesh.object.ball'). The type plugin will only 
   * be loaded if needed. 
   * \param name The factory will be registered with the engine
   * under the given name. If there is already a factory with that name
   * no new factory will be created but the found one is returned instead.
   * If the name is 0 then no name will be set and no check will happen
   * if the factory already exists.
   * \return 0 on failure; you must assign the result to a csRef or 
   * use DecRef().
   */
  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (const char* classId,
  	const char* name) = 0;

  /**
   * Create a mesh factory wrapper for an existing mesh factory
   * Assign to a csRef.
   * \param factory the mesh factory to be wrapped, the engine doesn't
   * "know" about a mesh factory until associated with a FactoryWrapper
   * \param name the engine name for the factory wrapper
   */
  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (
  	iMeshObjectFactory * factory, const char* name) = 0;

  /**
   * Create an uninitialized mesh factory wrapper
   * Assign to a csRef.
   * \param name the engine name for the factory wrapper
   */
  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (const char* name) = 0;

  /**
   * Convenience function to load a mesh factory from a given loader plugin.
   * \param name engine name for the mesh factory
   * \param loaderClassId the SCF class name of the desired mesh factory plugin
   * \param input data to initialize the mesh factory (plugin-specific)
   * Assign to a csRef.
   */
  virtual csPtr<iMeshFactoryWrapper> LoadMeshFactory (
  	const char* name, const char* loaderClassId,
	iDataBuffer* input) = 0;

  /**
   * Find the given mesh factory. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not 0 in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * 0 if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   * \param name the engine name of the desired mesh factory
   * \param region if specified, search only this region (also see note above)
   */
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name,
  	iRegion* region = 0) = 0;

  /// Get the list of mesh factories
  virtual iMeshFactoryList* GetMeshFactories () = 0;

  /** @} */
  
  /**\name Region handling
   * @{ */
  
  /**
   * Create a new region and add it to the region list.
   * If the region already exists then this function will just
   * return the pointer to that region.
   * \param name the engine name for the region
   */
  virtual iRegion* CreateRegion (const char* name) = 0;
  /// Get the list of all regions
  virtual iRegionList* GetRegions () = 0;

  /** @} */
  
  /**\name Camera handling
   * @{ */

  /**
   * Create a new camera.
   * Assign to a csRef.
   */
  virtual csPtr<iCamera> CreateCamera () = 0;

  /**
   * Find the given camera position. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not 0 in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * 0 if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   * \param name the engine name of the desired camera position
   * \param region if specified, search only this region (also see note above)
   */
  virtual iCameraPosition* FindCameraPosition (const char* name,
  	iRegion* region = 0) = 0;

  /// Get the list of camera positions.
  virtual iCameraPositionList* GetCameraPositions () = 0;

  /** @} */
  
  /**\name Portal handling
   * @{ */
  
  /**
   * Convenience function to create a portal from one sector to another
   * and make this portal a child mesh of another mesh. Use SCF_QUERY_INTERFACE
   * with iPortalContainer on the returned mesh for more control over the
   * portal(s) in the portal object.
   * \param name is the name of the portal container mesh to create the portal
   * in. If the parentMesh already has a mesh with that name then that will
   * be used. If there is already a mesh with that name but it is not a
   * portal container then a new mesh will be created.
   * \param parentMesh is the mesh where the portal container will be placed
   * as a child.
   * \param destSector is the sector where the single portal that is created
   * inside the portal object will point too.
   * \param vertices list of vertices comprising the portal.
   * \param num_vertices number of elements in 'vertices'.
   * \param portal is a return value for the created portal.
   * \return The meshwrapper on success (assign to a csRef),
   * or 0 on failure.
   */
  virtual csPtr<iMeshWrapper> CreatePortal (
  	const char* name,
  	iMeshWrapper* parentMesh, iSector* destSector,
	csVector3* vertices, int num_vertices,
	iPortal*& portal) = 0;

  /**
   * Convenience function to create a portal from one sector to another.
   * Use SCF_QUERY_INTERFACE with iPortalContainer on the returned mesh for
   * more control over the portal(s) in the portal object.
   * \param name is the name of the portal container mesh to create the portal
   * in. If the sourceSector already has a mesh with that name then that will
   * be used. If there is already a mesh with that name but it is not a
   * portal container then a new mesh will be created.
   * \param sourceSector is the sector where the portal container will be
   * placed.
   * \param pos the position inside that sector.
   * \param destSector the sector where the single portal that is created
   * inside the portal object will point too.
   * \param vertices list of vertices comprising the portal.
   * \param num_vertices number of elements in 'vertices'.
   * \param portal return value for the created portal.
   * \return The meshwrapper on success (assign to a csRef),
   * or 0 on failure.
   */
  virtual csPtr<iMeshWrapper> CreatePortal (
  	const char* name,
  	iSector* sourceSector, const csVector3& pos,
	iSector* destSector,
	csVector3* vertices, int num_vertices,
	iPortal*& portal) = 0;

  /**
   * Create an empty portal container in some sector. Use this portal
   * container to create portals to other sectors. Use SCF_QUERY_INTERFACE with
   * iPortalContainer on the mesh object inside the returned mesh to
   * control the portals.
   * \param name of the portal mesh.
   * \param sector is the location of the portal object and not the sector
   * the portals will point too. If not given then the portal container is
   * not put in any mesh.
   * \param pos is an optional position inside the sector (if given).
   * \return The meshwrapper on success (assign to a csRef),
   * or 0 on failure.
   */
  virtual csPtr<iMeshWrapper> CreatePortalContainer (const char* name,
  	iSector* sector = 0, const csVector3& pos = csVector3 (0, 0, 0)) = 0;

  /** @} */
  
  /**\name Drawing related
   * @{ */

  /**
   * Require that the Z-buffer is cleared every frame. The engine
   * itself will not use this setting but will only return the
   * correct flag in GetBeginDrawFlags() so that the Z-buffer is actually
   * cleared. Note that this requires that the application actually
   * uses GetBeginDrawFlags() in the call to g3d->BeginDraw() (which it should).
   * By default this flag is false. It is useful to set this flag to true
   * if you have a level that doesn't itself have another way to initialize
   * the Z-buffer.
   * \param yesno true to clear the Z buffer after each frame, 
   * false to leave the zbuffer as-is
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
   * \param yesno true to clear the screen before each frame, 
   * false to leave the screen as-is (which may leave garbage on the screen)
   */
  virtual void SetClearScreen (bool yesno) = 0;

  /**
   * Get the value of the clear screen flag set with SetClearScreen().
   */
  virtual bool GetClearScreen () const = 0;

  /// Get default clear screen flag
  virtual bool GetDefaultClearScreen () const = 0;

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
  virtual iRenderView* GetTopLevelClipper () const = 0;

  /**
   * This function precaches all meshes by calling GetRenderMeshes()
   * on them. By doing this the level will run smoother if you walk
   * through it because all meshes will have had a chance to update
   * caches and stuff.
   * \param region is an optional region. If given then only objects
   *        in that region will be precached.
   */
  virtual void PrecacheDraw (iRegion* region = 0) = 0;

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
   * as the procedural texture to render on. If this is 0 then the
   * screen is assumed.
   */
  virtual void SetContext (iTextureHandle* ctxt) = 0;
  /// Return the current drawing context.
  virtual iTextureHandle *GetContext () const = 0;

  /**
   * Retrieve the render loop manager.
   */
  virtual iRenderLoopManager* GetRenderLoopManager () = 0;
  
  /**
   * Returns the current render loop.
   * \remark This will the loop that is set to be the current default
   *  with SetCurrentDefaultRenderloop(). This doesn't have to be the engine's
   *  default render loop (note the difference between the "current" and 
   *  "default" render loop - former one is the loop used currently for 
   *  drawing, latter one is a default loop created at engine initialization 
   *  time.) To retrieve the default loop, use 
   * \code
   *  GetRenderLoopManager()->Retrieve (#CS_DEFAULT_RENDERLOOP_NAME);
   * \endcode
   */
  virtual iRenderLoop* GetCurrentDefaultRenderloop () = 0;

  /**
   * Set the current render loop.
   * \param loop The loop to be made the current render loop.
   * \return Whether the change was successful (a value of 0 for \p will 
   *   let the method fail.)
   */
  virtual bool SetCurrentDefaultRenderloop (iRenderLoop* loop) = 0;

  /**
   * Get the current framenumber. This should be incremented once every Draw
   */
  virtual uint GetCurrentFrameNumber () const = 0;

  /** @} */
  
  /**\name Saving/loading
   * @{ */

  /**
   * Set whether saving should be possible (default OFF).
   * To allow saving of a world after it has been loaded, some objects may 
   * need to keep track of extra data that would otherwise not be needed if 
   * the world will never be written out again. The 'saveable' flag informs
   * those objects about whether to keep that information or not. Saving
   * a world with this flag disables is still possible, but the result might
   * incomplete.
   */
  virtual void SetSaveableFlag (bool enable) = 0;

  /**
   * Get whether saving should be possible (default OFF).
   */
  virtual bool GetSaveableFlag () = 0;

  /**
   * Create a loader context that you can give to loader plugins.
   * It will basically allow loader plugins to find materials.
   * \param region optional loader region
   * \param curRegOnly if region is valid and and curRegOnly is true 
   * then only that region will be searched. 
   * Assign to a csRef.
   */
  virtual csPtr<iLoaderContext> CreateLoaderContext (
  	iRegion* region = 0, bool curRegOnly = true) = 0;

  /** @} */
  
  /**\name Other
   * @{ */

  /**
   * This routine returns an iterator to iterate over
   * all objects that are within a radius of a given position.
   * The current implementation only does meshes but in future
   * lights will also be supported.
   * You can use #SCF_QUERY_INTERFACE to get any interface from the
   * returned objects. If crossPortals is true it will search through
   * portals. Otherwise it will limit the search to the sector passed in.
   * If you only want to have meshes then it is more efficient to
   * call GetNearbyMeshes() as you can then avoid the call to
   * #SCF_QUERY_INTERFACE.
   */
  virtual csPtr<iObjectIterator> GetNearbyObjects (iSector* sector,
    const csVector3& pos, float radius, bool crossPortals = true ) = 0;

  

  /**
   * This routine returns an iterator to iterate over
   * all objects that are potentially visible as seen from a given position.
   * This routine assumes full 360 degree visibility.
   * You can use #SCF_QUERY_INTERFACE to get any interface from the
   * returned objects.<p>
   * If you only want meshes then use GetVisibleMeshes().
   * CURRENTLY NOT IMPLEMENTED!
   */
  virtual csPtr<iObjectIterator> GetVisibleObjects (iSector* sector,
    const csVector3& pos) = 0;

  /**
   * This routine returns an iterator to iterate over
   * all meshes that are potentially visible as seen from a given position.
   * This routine assumes full 360 degree visibility.
   * CURRENTLY NOT IMPLEMENTED!
   */
  virtual csPtr<iMeshWrapperIterator> GetVisibleMeshes (iSector* sector,
    const csVector3& pos) = 0;

  /**
   * This routine returns an iterator to iterate over
   * all objects that are potentially visible as seen from a given position.
   * This routine has a frustum restricting the view.
   * You can use #SCF_QUERY_INTERFACE to get any interface from the
   * returned objects.<p>
   * If you only want meshes then use GetVisibleMeshes().
   * CURRENTLY NOT IMPLEMENTED!
   */
  virtual csPtr<iObjectIterator> GetVisibleObjects (iSector* sector,
    const csFrustum& frustum) = 0;

  /**
   * This routine returns an iterator to iterate over
   * all meshes that are potentially visible as seen from a given position.
   * This routine has a frustum restricting the view.
   * CURRENTLY NOT IMPLEMENTED!
   */
  virtual csPtr<iMeshWrapperIterator> GetVisibleMeshes (iSector* sector,
    const csFrustum& frustum) = 0;

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

    /// Get the list of all shared variables.
  virtual iSharedVariableList* GetVariableList () const = 0;

  /// Get the list of collections.
  virtual iCollectionList* GetCollections () = 0;
  
  /**
   * Find the given collection. The name can be a normal
   * name. In that case this function will look in all regions
   * except if region is not 0 in which case it will only
   * look in that region.
   * If the name is specified as 'regionname/objectname' then
   * this function will only look in the specified region and return
   * 0 if that region doesn't contain the object or the region
   * doesn't exist. In this case the region parameter is ignored.
   * \param name the engine name of the desired collection
   * \param region if specified, search only this region (also see note above)
   */
  virtual iCollection* FindCollection (const char* name,
  	iRegion* region = 0) = 0;

  /**
   * Convenience function to 'remove' a CS object from the engine.
   * This will not clear the object but it will remove all references
   * to that object that the engine itself keeps. This function works
   * for: iSector, iCollection, iMeshWrapper, iMeshFactoryWrapper,
   * iCameraPosition, iLight, iMaterialWrapper, and iTextureWrapper.
   * Note that the object is only removed if the resulting ref count will
   * become zero. So basically this function only releases the references
   * that the engine holds.
   * <p>
   * This function returns true if the engine recognized the object as
   * one on which it can operate.
   * <p>
   * This function will also remove the object from the region it may be in.
   */
  virtual bool RemoveObject (iBase* object) = 0;
 
  /// Delete everything in the engine.
  virtual void DeleteAll () = 0;

  /**
   * Reset a subset of flags/settings (which may differ from one world/map to 
   * another) to its defaults. This currently includes:
   *   - clear z buffer flag
   *   - lightmap cell size
   *   - maximum lightmap size
   */
  virtual void ResetWorldSpecificSettings() = 0;  
  
  /** @} */
};

/** @} */

#endif // __CS_IENGINE_ENGINE_H__
