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
#include "isys/plugin.h"
#include "csgeom/vector3.h"

class csEngine;
class csVector3;
class csMatrix3;
class csColor;
struct csTextureLayer;

struct iSector;
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
struct iTerrainObject;
struct iTerrainObjectFactory;
struct iTerrainWrapper;
struct iTerrainFactoryWrapper;
struct iCameraPosition;
struct iRegion;
struct iView;
struct iGraphics3D;
struct iClipper2D;
struct iPolyTxtPlane;
struct iCurveTemplate;
struct iObject;
struct iCollection;
struct iDataBuffer;
struct iCamera;
struct iRenderView;
struct iMapNode;
struct iKeyValuePair;

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
 * Setting for SetCuller().
 * Use the c-buffer (default).
 */
#define CS_CULLER_CBUFFER 0

/**
 * Setting for SetCuller().
 * Use the 3D quadtree.
 */
#define CS_CULLER_QUAD3D 1

/**
 * Setting for SetCuller().
 * Use the coverage mask tree.
 */
#define CS_CULLER_COVTREE 2

/// A callback function for csEngine::DrawFunc().
typedef void (csDrawFunc) (iRenderView* rview, int type, void* entity);


SCF_VERSION (iEngine, 0, 1, 26);

/**
 * This interface is the main interface to the 3D engine.
 * The engine is responsible for creating new engine-specific objects
 * such as sectors, things, sprites and so on.
 */
struct iEngine : public iPlugIn
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
   * locally now).
   */
  virtual bool Prepare () = 0;

  /**
   * Query the format to load textures (usually this depends on texture
   * manager)
   */
  virtual int GetTextureFormat () = 0;

  /**
   * Return true if engine want to use PVS.
   */
  virtual bool IsPVS () = 0;

  /**
   * Create or select a new region (name can be NULL for the default main
   * region). All new objects will be marked as belonging to this region.
   */
  virtual void SelectRegion (const char* iName) = 0;
  /**
   * Get a reference to the current region (or NULL if the default main
   * region is selected).
   */
  virtual iRegion* GetCurrentRegion () = 0;
  /// Find a region by name
  virtual iRegion* FindRegion (const char *name) = 0;

  /**
   * Create or select a new object library (name can be NULL for engine).
   * All new objects will be marked as belonging to this library.
   * You can then delete a whole library at once, for example.
   * @@@ NOTE THE LIBRARY SYSTEM IS OBSOLETE. Use csRegion/iRegion instead.
   */
  virtual void SelectLibrary (const char *iName) = 0;
  /// Delete a whole library (all objects that are part of library)
  virtual bool DeleteLibrary (const char *iName) = 0;
  /// Delete all libraries (clear the engine)
  virtual void DeleteAll () = 0;

  /// Register a new render priority.
  virtual void RegisterRenderPriority (const char* name, long priority) = 0;
  /// Get a render priority by name.
  virtual long GetRenderPriority (const char* name) const = 0;
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
  /// Create a named camera position object
  virtual iCameraPosition* CreateCameraPosition (const char *iName,
	const char *iSector, const csVector3 &iPos,
	const csVector3 &iForward, const csVector3 &iUpward) = 0;
  /// Create a key/value pair object
  virtual bool CreateKey (const char *iName, const char *iValue) = 0;
  /// Create a texture plane
  virtual bool CreatePlane (const char *iName, const csVector3 &iOrigin,
	const csMatrix3 &iMatrix) = 0;
  /**
   * Create a empty sector with given name.
   * If link == true (default) the sector will be linked to the engine.
   */
  virtual iSector *CreateSector (const char *iName, bool link = true) = 0;

  /**
   * Conveniance function to create the thing containing the
   * convex outline of a sector. The thing will be empty but
   * it will have CS_ZBUF_FILL set and have 'wall' as render
   * priority. This version creates a mesh wrapper.
   */
  virtual iMeshWrapper* CreateSectorWallsMesh (iSector* sector,
      const char* name) = 0;

  /// Query number of sectors in engine
  virtual int GetSectorCount () = 0;
  /// Get a sector by index
  virtual iSector *GetSector (int iIndex) = 0;
  /**
   * Find a sector by name. If regionOnly is true then the returned
   * sector will belong to the current region. Note that this is different
   * from calling iRegion::FindSector() because the latter will also
   * return sectors that belong in a region but are not connected to the
   * engine.
   */
  virtual iSector *FindSector (const char *iName, bool regionOnly = false) = 0;
  /// Delete a sector
  virtual void DeleteSector (iSector *) = 0;

  /**
   * Find a mesh object by name. If regionOnly is true then the returned
   * mesh object will belong to the current region. Note that this is different
   * from calling iRegion::FindMeshObject() because the latter will also
   * return mesh objects that belong in a region but are not connected to the
   * engine.
   */
  virtual iMeshWrapper *FindMeshObject (const char *iName,
  	bool regionOnly = false) = 0;

  /**
   * Delete a mesh factory by name. ONLY call this when you're sure
   * no objects are actually using this factory!
   */
  virtual void DeleteMeshFactory (const char* iName,
  	bool regionOnly = false) = 0;

  /**
   * Find a mesh factory by name. If regionOnly is true then the returned
   * factory will belong to the current region. Note that this is different
   * from calling iRegion::FindMeshFactory() because the latter will also
   * return factories that belong in a region but are not connected to the
   * engine.
   */
  virtual iMeshFactoryWrapper *FindMeshFactory (const char *iName,
  	bool regionOnly = false) = 0;
  /**
   * Find a terrain object by name. If regionOnly is true then the returned
   * mesh object will belong to the current region. Note that this is different
   * from calling iRegion::FindTerrainObject() because the latter will also
   * return terrain objects that belong in a region but are not connected to the
   * engine.
   */
  virtual iTerrainWrapper *FindTerrainObject (const char *iName,
  	bool regionOnly = false) = 0;
  /**
   * Find a terrain factory by name. If regionOnly is true then the returned
   * factory will belong to the current region. Note that this is different
   * from calling iRegion::FindTerrainFactory() because the latter will also
   * return factories that belong in a region but are not connected to the
   * engine.
   */
  virtual iTerrainFactoryWrapper *FindTerrainFactory (const char *iName,
  	bool regionOnly = false) = 0;
  /**
   * Find a texture by name. If regionOnly is true then the returned
   * texture will belong to the current region. Note that this is different
   * from calling iRegion::FindTexture() because the latter will also
   * return textures that belong in a region but are not connected to the
   * engine.
   */
  virtual iTextureWrapper* FindTexture (const char* iName,
  	bool regionOnly = false) = 0;
  /**
   * Find a material by name. If regionOnly is true then the returned
   * material will belong to the current region. Note that this is different
   * from calling iRegion::FindMaterial() because the latter will also
   * return materials that belong in a region but are not connected to the
   * engine.
   */
  virtual iMaterialWrapper* FindMaterial (const char* iName,
  	bool regionOnly = false) = 0;
  /**
   * Find a camera position by name. If regionOnly is true then the returned
   * campos will belong to the current region. Note that this is different
   * from calling iRegion::FindCameraPosition() because the latter will also
   * return camera positions that belong in a region but are not connected
   * to the engine.
   */
  virtual iCameraPosition* FindCameraPosition (const char* iName,
  	bool regionOnly = false) = 0;
  /**
   * Find a collection by name. If regionOnly is true then the returned
   * collection will belong to the current region. Note that this is different
   * from calling iRegion::FindCollection() because the latter will also
   * return collections that belong in a region but are not connected
   * to the engine.
   */
  virtual iCollection* FindCollection (const char* iName,
  	bool regionOnly = false) = 0;

  /// Create a new collection.
  virtual iCollection* CreateCollection (const char* iName) = 0;

  /// Enable/disable the lighting cache.
  virtual void EnableLightingCache (bool do_cache) = 0;
  /// Return true if lighting cache is enabled.
  virtual bool IsLightingCacheEnabled () = 0;

  /// Return the current lightmap cell size
  virtual int GetLightmapCellSize () = 0;
  /// Set lightmap cell size
  virtual void SetLightmapCellSize (int Size) = 0;

  /// Create a new view on the engine.
  virtual iView* CreateView (iGraphics3D* g3d) = 0;
  /// Create a static/pseudo-dynamic light. name can be NULL.
  virtual iStatLight* CreateLight (const char* name, const csVector3& pos,
  	float radius, const csColor& color, bool pseudoDyn) = 0;
  /// Find a static/pseudo-dynamic light by name.
  virtual iStatLight* FindLight (const char *Name, bool RegionOnly = false) = 0;
  /// Create a dynamic light.
  virtual iDynLight* CreateDynLight (const csVector3& pos, float radius,
  	const csColor& color) = 0;
  /// Remove a dynamic light.
  virtual void RemoveDynLight (iDynLight*) = 0;

  /**
   * Get the required flags for 3D->BeginDraw() which should be called
   * from the application. These flags must be or-ed with optional other
   * flags that the application might be interested in.
   */
  virtual int GetBeginDrawFlags () = 0;

  /**
   * Get the current engine mode.
   * If called between SetEngineMode() and the first Draw() it is
   * possible that this mode will still be CS_ENGINE_AUTODETECT.
   */
  virtual int GetEngineMode () = 0;

  /**
   * Get the top-level clipper.
   */
  virtual iClipper2D* GetTopLevelClipper () = 0;

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
  virtual iMeshWrapper* CreateMeshObject (iMeshFactoryWrapper* factory,
  	const char* name, iSector* sector = NULL,
	const csVector3& pos = csVector3(0, 0, 0)) = 0;
  /// Create a mesh wrapper for an existing mesh object
  virtual iMeshWrapper* CreateMeshObject (iMeshObject*,
  	const char* name, iSector* sector = NULL,
	const csVector3& pos = csVector3(0, 0, 0)) = 0;
  /// Create an uninitialized mesh wrapper
  virtual iMeshWrapper* CreateMeshObject (const char* name) = 0;
  /**
   * Conveniance function to load a mesh object from a given loader plugin.
   * If sector == NULL the object will not be placed in a sector.
   */
  virtual iMeshWrapper* LoadMeshObject (
  	const char* classId, const char* name,
	const char* loaderClassId,
	iDataBuffer* input, iSector* sector, const csVector3& pos) = 0;

  /// return the number of mesh objects
  virtual int GetNumMeshObjects () = 0;
  /// return a mesh object by index
  virtual iMeshWrapper *GetMeshObject (int n) = 0;

  /**
   * Conveniance function to create a terrain factory from a given type.
   * The type plugin will only be loaded if needed. 'classId' is the
   * SCF name of the plugin (like 'crystalspace.terrain.object.ddg').
   * Returns NULL on failure. The factory will be registered with the engine
   * under the given name. If there is already a factory with that name
   * no new factory will be created but the found one is returned instead.
   * If the name is NULL then no name will be set and no check will happen
   * if the factory already exists.
   */
  virtual iTerrainFactoryWrapper* CreateTerrainFactory (const char* classId,
  	const char* name) = 0;
  /// Create a terrain factory wrapper for an existing terrain factory
  virtual iTerrainFactoryWrapper* CreateTerrainFactory (iTerrainObjectFactory*,
  	const char* name) = 0;
  /// Create an uninitialized terrain factory wrapper
  virtual iTerrainFactoryWrapper* CreateTerrainFactory (const char* name) = 0;

  /**
   * Conveniance function to create a terrain object for a given factory.
   * If 'sector' is NULL then the terrain object will not be set to a position.
   * Returns NULL on failure. The object will be given the specified name.
   * 'name' can be NULL if no name is wanted. Different mesh objects can
   * have the same name (in contrast with factory objects).
   */
  virtual iTerrainWrapper* CreateTerrainObject
	(iTerrainFactoryWrapper* factory, const char* name,
	 iSector* sector = NULL) = 0;
  /// Create a terrain wrapper for an existing terrain object
  virtual iTerrainWrapper* CreateTerrainObject (iTerrainObject*,
  	const char* name, iSector* sector = NULL) = 0;
  /// Create an uninitialized terrain wrapper
  virtual iTerrainWrapper* CreateTerrainObject (const char* name) = 0;

  /**
   * @@@ Temporary function to create a polygon plane. This is temporary
   * until planes are managed by the thing plugin.
   */
  virtual iPolyTxtPlane* CreatePolyTxtPlane (const char* name = NULL) = 0;
  /**
   * @@@ Temporary function to find a polygon plane. This is temporary until
   * planes are managed by the thing plugin.
   */
  virtual iPolyTxtPlane* FindPolyTxtPlane (const char *iName,
  	bool regionOnly = false) = 0;

  /**
   * @@@ Temporary function to create a bezier template. This is temporary
   * until planes are managed by the thing plugin.
   */
  virtual iCurveTemplate* CreateBezierTemplate (const char* name = NULL) = 0;
  /**
   * @@@ Temporary function to find a curve template. This is temporary until
   * planes are managed by the thing plugin.
   */
  virtual iCurveTemplate* FindCurveTemplate (const char *iName,
  	bool regionOnly = false) = 0;

  /// @@@ Temporary function until things are moved to a plugin.
  virtual iMeshObjectType* GetThingType () = 0;

  /**
   * Get the list of all textures.
   */
  virtual iTextureList* GetTextureList () = 0;
  /**
   * Get the list of all materials.
   */
  virtual iMaterialList* GetMaterialList () = 0;

  /**
   * Advance the frames of all objects given the current time.
   */
  virtual void NextFrame (cs_time current_time) = 0;

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
   * or draw debugging information (2D egdes for example).
   */
  virtual void DrawFunc (iCamera* c, iClipper2D* clipper,
    csDrawFunc* callback, void* callback_data = NULL) = 0;

  /// Set the drawing context
  virtual void SetContext (iGraphics3D*) = 0;

  /// Create a map node.
  virtual iMapNode* CreateMapNode (const char* name) = 0;
  /// Create a key value pair.
  virtual iKeyValuePair* CreateKeyValuePair (const char* key,
  	const char* value) = 0;
};

#endif // __IENGINE_ENGINE_H__
