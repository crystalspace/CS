/*
    Crystal Space 3D Engine
    Copyright (C) 1998-2000 by Jorrit Tyberghein
  
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

#ifndef __IENGINE_H__
#define __IENGINE_H__

#include "csutil/scf.h"
#include "iplugin.h"

class csEngine;
class csVector3;
class csMatrix3;
class csColor;

struct iSector;
struct iStatLight;
struct iDynLight;
struct iThing;
struct iSprite;
struct iMeshWrapper;
struct iMeshFactoryWrapper;
struct iMaterialWrapper;
struct iTextureWrapper;
struct iCameraPosition;
struct iRegion;
struct iView;
struct iGraphics3D;
struct iTransformationManager;

SCF_VERSION (iEngine, 0, 1, 12);

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
   * Create or select a new region (name can be NULL for the default main
   * region). All new objects will be marked as belonging to this region.
   */
  virtual void SelectRegion (const char* iName) = 0;
  /**
   * Get a reference to the current region (or NULL if the default main
   * region is selected).
   */
  virtual iRegion* GetCurrentRegion () = 0;

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

  /// Register a texture to be loaded during Prepare()
  virtual iTextureWrapper* CreateTexture (const char *iName, const char *iFileName,
    csColor *iTransp, int iFlags) = 0;
  /// Register a material to be loaded during Prepare()
  virtual iMaterialWrapper* CreateMaterial (const char *iName, iTextureWrapper* texture) = 0;
  /// Create a named camera position object
  virtual bool CreateCamera (const char *iName, const char *iSector,
    const csVector3 &iPos, const csVector3 &iForward,
    const csVector3 &iUpward) = 0;
  /// Create a key/value pair object
  virtual bool CreateKey (const char *iName, const char *iValue) = 0;
  /// Create a texture plane
  virtual bool CreatePlane (const char *iName, const csVector3 &iOrigin,
    const csMatrix3 &iMatrix) = 0;
  /// Create a empty sector with given name
  virtual iSector *CreateSector (const char *iName) = 0;
  /// Create a empty thing with given name
  virtual iThing *CreateThing (const char *iName, iSector *iParent) = 0;

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

  /**
   * Find a thing by name. If regionOnly is true then the returned
   * thing will belong to the current region. Note that this is different
   * from calling iRegion::FindThing() because the latter will also
   * return things that belong in a region but are not connected to the
   * engine.
   */
  virtual iThing *FindThing (const char *iName, bool regionOnly = false) = 0;
  /**
   * Find a sky thing by name. If regionOnly is true then the returned
   * thing will belong to the current region. Note that this is different
   * from calling iRegion::FindSky() because the latter will also
   * return things that belong in a region but are not connected to the
   * engine.
   */
  virtual iThing *FindSky (const char *iName, bool regionOnly = false) = 0;
  /**
   * Find a thing template by name. If regionOnly is true then the returned
   * thing will belong to the current region. Note that this is different
   * from calling iRegion::FindThingTemplate() because the latter will also
   * return things that belong in a region but are not connected to the
   * engine.
   */
  virtual iThing *FindThingTemplate (const char *iName, bool regionOnly = false) = 0;
  /**
   * Find a mesh object by name. If regionOnly is true then the returned
   * mesh object will belong to the current region. Note that this is different
   * from calling iRegion::FindMeshObject() because the latter will also
   * return mesh objects that belong in a region but are not connected to the
   * engine.
   */
  virtual iMeshWrapper *FindMeshObject (const char *iName, bool regionOnly = false) = 0;
  /**
   * Find a mesh factory by name. If regionOnly is true then the returned
   * factory will belong to the current region. Note that this is different
   * from calling iRegion::FindMeshFactory() because the latter will also
   * return factories that belong in a region but are not connected to the
   * engine.
   */
  virtual iMeshFactoryWrapper *FindMeshFactory (const char *iName, bool regionOnly = false) = 0;
  /**
   * Find a texture by name. If regionOnly is true then the returned
   * texture will belong to the current region. Note that this is different
   * from calling iRegion::FindTexture() because the latter will also
   * return textures that belong in a region but are not connected to the
   * engine.
   */
  virtual iTextureWrapper* FindTexture (const char* iName, bool regionOnly = false) = 0;
  /**
   * Find a material by name. If regionOnly is true then the returned
   * material will belong to the current region. Note that this is different
   * from calling iRegion::FindMaterial() because the latter will also
   * return materials that belong in a region but are not connected to the
   * engine.
   */
  virtual iMaterialWrapper* FindMaterial (const char* iName, bool regionOnly = false) = 0;
  /**
   * Find a camera position by name. If regionOnly is true then the returned
   * campos will belong to the current region. Note that this is different
   * from calling iRegion::FindCameraPosition() because the latter will also
   * return camera positions that belong in a region but are not connected
   * to the engine.
   */
  virtual iCameraPosition* FindCameraPosition (const char* iName, bool regionOnly = false) = 0;

  /// Enable/disable the lighting cache.
  virtual void EnableLightingCache (bool do_cache) = 0;
  /// Return true if lighting cache is enabled.
  virtual bool IsLightingCacheEnabled () = 0;

  /// Create a new view on the engine.
  virtual iView* CreateView (iGraphics3D* g3d) = 0;
  /// Create a static/pseudo-dynamic light.
  virtual iStatLight* CreateLight (const csVector3& pos, float radius,
  	const csColor& color, bool pseudoDyn) = 0;
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

  /// Get the transformation manager.
  virtual iTransformationManager* GetTransformationManager () = 0;

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

  /**
   * Conveniance function to create a mesh object for a given factory.
   * If 'sector' is NULL then the mesh object will not be set to a position.
   * Returns NULL on failure. The object will be given the specified name.
   * 'name' can be NULL if no name is wanted. Different mesh objects can
   * have the same name (in contrast with factory objects).
   */
  virtual iMeshWrapper* CreateMeshObject (iMeshFactoryWrapper* factory,
  	const char* name, iSector* sector, const csVector3& pos) = 0;
};

#endif // __IENGINE_H__
