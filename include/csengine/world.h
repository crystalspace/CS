/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
  
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

#ifndef __CS_WORLD_H__
#define __CS_WORLD_H__

#include "csutil/scf.h"
#include "csgeom/math3d.h"
#include "csobject/nobjvec.h"
#include "csengine/rview.h"
#include "csengine/tranman.h"
#include "csengine/halo.h"
#include "csobject/csobj.h"
#include "iworld.h"
#include "iconfig.h"

class csSector;
class csTextureList;
class csSprite3D;
class csPolygon3D;
class csCamera;
class csThing;
class csThingTemplate;
class csCollection;
class csStatLight;
class csDynLight;
class csSpriteTemplate;
class csClipper;
class csQuadcube;
class csWorld;
class Dumper;
class csLight;
class csCBuffer;
class csQuadtree;
class csPoly2DPool;
class csLightPatchPool;
scfInterface iHaloRasterizer;
scfInterface iGraphics3D;
scfInterface iSystem;
scfInterface iVFS;

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
 * Iterator to iterate over all polygons in the world.
 * This iterator assumes there are no fundamental changes
 * in the world while it is being used.
 * If changes to the world happen the results are unpredictable.
 */
class csPolyIt
{
private:
  // The world for this iterator.
  csWorld* world;
  // Current sector index.
  int sector_idx;
  // Current thing.
  csThing* thing;
  // Current polygon index.
  int polygon_idx;

public:
  /// Construct an iterator and initialize to start.
  csPolyIt (csWorld* w);

  /// Restart iterator.
  void Restart ();

  /// Get polygon from iterator. Return NULL at end.
  csPolygon3D* Fetch ();
};

/**
 * Iterator to iterate over all static lights in the world.
 * This iterator assumes there are no fundamental changes
 * in the world while it is being used.
 * If changes to the world happen the results are unpredictable.
 */
class csLightIt
{
private:
  // The world for this iterator.
  csWorld* world;
  // Current sector index.
  int sector_idx;
  // Current light index.
  int light_idx;

public:
  /// Construct an iterator and initialize to start.
  csLightIt (csWorld* w);

  /// Restart iterator.
  void Restart ();

  /// Get light from iterator. Return NULL at end.
  csLight* Fetch ();
};

/**
 * The world! This class basicly represents the 3D engine.
 * It is the main anchor class for working with Crystal Space.
 */
class csWorld : public iWorld, public iConfig, public csObject
{
  friend class Dumper;

  // Private class for keeping an array of halo infos
  class csHaloVector : public csVector
  {
  public:
    // Constructor
    csHaloVector () : csVector (16, 16) {}
    // Destructor
    virtual ~csHaloVector () { DeleteAll (); }
    // Free an item from array
    virtual bool FreeItem (csSome Item)
    { CHK (delete (csHaloInformation *)Item); return true; }
    // Find a halo by referenced light
    virtual int CompareKey (csSome Item, csConstSome Key, int /*Mode*/) const
    { return ((csHaloInformation *)Item)->pLight == (csLight *)Key ? 0 : -1; }
    // Return an reference to Nth halo info
    inline csHaloInformation *Get (int n) const
    { return (csHaloInformation *)csVector::Get (n); }
  };

public:
  /**
   * This is the Virtual File System object where all the files
   * used in this world live. Textures, models, data, everything -
   * reside on this virtual disk volume. You should avoid using
   * the standard file functions (such as fopen(), fread() and so on)
   * since they are highly system-dependent (for example, DOS uses
   * '\' as path separator, Mac uses ':' and Unix uses '/').
   */
  iVFS *VFS;

  /**
   * This is a vector which holds objects of type 'csCleanable'.
   * They will be destroyed when the world is destroyed. That's
   * the only special thing. This is useful for holding memory
   * which you allocate locally in a function but you want
   * to reuse accross function invocations. There is no general
   * way to make sure that the memory will be freed it only exists
   * as a static pointer in your function code. Adding a class
   * encapsulating that memory to this array will ensure that the
   * memory is removed once the world is destroyed.
   */
  csObjVector cleanup;

  /**
   * List of sectors in the world. This vector contains
   * objects of type csSector*. Use NewSector() to add sectors
   * to the world.
   */
  csNamedObjVector sectors;

  /**
   * List of planes. This vector contains objects of type
   * csPolyPlane*. Note that this vector only contains named
   * planes. Default planes which are created for polygons
   * are not in this list.
   */
  csNamedObjVector planes;

  /**
   * List of collections. This vector contains objects of type
   * csCollection*.
   */
  csNamedObjVector collections;

  /**
   * List of sprite templates. This vector contains objects of
   * type csSpriteTemplate*. You can use GetSpriteTemplate() to locate
   * a template for a sprite. This function can optionally look in
   * all loaded libraries as well.
   */
  csNamedObjVector sprite_templates;

  /**
   * List of thing templates. This vector contains objects of
   * type csThingTemplate*. You can use GetThingTemplate() to locate
   * a template for a thing. This function can optionally look in
   * all loaded libraries as well.
   */
  csNamedObjVector thing_templates;

  /**
   * List of all sprites in the world. This vector contains objects
   * of type csSprite3D*. Use UnlinkSprite() and RemoveSprite()
   * to unlink and/or remove sprites from this list. These functions
   * take care of correctly removing the sprites from all sectors
   * as well. Note that after you add a sprite to the list you still
   * need to add it to all sectors that you want it to be visible in.
   */
  csNamedObjVector sprites;

  // Remember dimensions of display.
  static int frame_width, frame_height;
  // Remember iSystem interface.
  static iSystem* System;
  // Current world.
  static csWorld* current_world;
  // An object pool for 2D polygons used by the rendering process.
  csPoly2DPool* render_pol2d_pool;
  // An object pool for lightpatches.
  csLightPatchPool* lightpatch_pool;
  // The transformation manager.
  csTransformationManager tr_manager;
  // The 3D driver
  iGraphics3D* G3D;

private:
  /// Texture and color information object.
  csTextureList* textures;
  /// Linked list of dynamic lights.
  csDynLight* first_dyn_lights;
  /// List of halos (csHaloInformation).
  csHaloVector halos;  
  /// The Halo rasterizer. If NULL halo's are not supported by the rasterizer.
  iHaloRasterizer* HaloRast;
  /// If true then the lighting cache is enabled.
  bool do_lighting_cache;

  /// Optional c-buffer used for rendering.
  csCBuffer* c_buffer;

  /// Optional quad-tree used for rendering.
  csQuadtree* quadtree;

  /// Quad-cube used for lighting.
  csQuadcube* quadcube;

  ///
  void ShineLights ();

public:
  /**
   * The starting sector for the camera as specified in the world file.
   * This is optional. If the world file does not have a starting sector
   * then this field will be NULL.
   */
  char* start_sector;

  /**
   * The starting vector for the camera as specified in the world file.
   * This is optional. If the world file does not have a starting vector then
   * this field will be equal to the 0-vector (i.e. (0,0,0)).
   */
  csVector3 start_vec;

  /**
   * The current camera for drawing the world.
   */
  csCamera* current_camera;

  /**
   * The top-level clipper we are currently using for drawing.
   */
  csClipper* top_clipper;

public:
  /**
   * Initialize an empty world. The only thing that is valid just
   * after creating the world is the configurator object which you
   * can use to configure the world before continuing (see GetEngineConfig()).
   */
  csWorld (iBase *iParent);

  /**
   * Delete the world and all entities in the world. All objects added to this
   * world by the user (like Things, Sectors, ...) will be deleted as well. If
   * you don't want this then you should unlink them manually before destroying
   * the world.
   */
  virtual ~csWorld ();

  /**
   * Check consistency of the loaded world. Currently this function only
   * checks if polygons have three or more vertices and if the vertices
   * are coplanar (if more than three). This function prints out warnings
   * for all found errors.  * Returns true if everything is in order.
   */
  bool CheckConsistency ();

  /**
   * Prepare the textures. It will initialise all loaded textures
   * for the texture manager. (Normally you shouldn't call this function
   * directly, because it will be called by Prepare() for you.
   */
  void PrepareTextures ();

  /**
   * Prepare all the loaded sectors. (Normally you shouldn't call this 
   * function directly, because it will be called by Prepare() for you.
   */
  void PrepareSectors();

  /**
   * Prepare the world. This function must be called after
   * you loaded/created the world. It will prepare all lightmaps
   * for use and also free all images that were loaded for
   * the texture manager (the texture manager should have them
   * locally now).
   */
  bool Prepare ();

  /**
   * Enable/disable c-buffer.
   */
  void EnableCBuffer (bool en);

  /**
   * Return c-buffer (or NULL if not used).
   */
  csCBuffer* GetCBuffer () { return c_buffer; }

  /**
   * Enable/disable quadtree.
   */
  void EnableQuadtree (bool en);

  /**
   * Return quadtree (or NULL if not used).
   */
  csQuadtree* GetQuadtree () { return quadtree; }

  /**
   * Return quad-cube.
   */
  csQuadcube* GetQuadcube () { return quadcube; }

  /**
   * Cache lighting. If true (default) then lighting will be cached in
   * either the world file or else 'precalc.zip'. If false then this
   * will not happen and lighting will be calculated at startup.
   * If set to 'false' recalculating of lightmaps will be forced.
   * If set to 'true' recalculating of lightmaps will depend on
   * wether or not the lightmap was cached.
   */
  void EnableLightingCache (bool en);

  /// Return true if lighting cache is enabled.
  bool IsLightingCacheEnabled () { return do_lighting_cache; }

  /**
   * Read configuration file (using the system driver) for all engine
   * specific values. This function is called by Initialize() so you
   * normally do not need to call it yourselves.
   */
  void ReadConfig ();

  /**
   * Prepare for creation of a world. This function is called
   * by Initialize() so you normally do not need to call it
   * yourselves.
   */
  void StartWorld ();

  /**
   * Get the Halo Rasterizer SCF interface if supported (NULL if not).
   */
  iHaloRasterizer* GetHaloRastizer () { return HaloRast; }

  /**
   * Clear everything in the world.
   */
  void Clear ();

  /**
   * Create a new sector and add it to the world.
   */
  csSector* NewSector ();

  /**
   * Find a named sprite template in the loaded world and
   * optionally in all loaded libraries. This template can then
   * be used to create sprites.
   */
  csSpriteTemplate* GetSpriteTemplate (const char* name);

  /**
   * Find a named thing template in the loaded world and
   * optionally in all loaded libraries. This template can then
   * be used to create things.
   */
  csThingTemplate* GetThingTemplate (const char* name);

  /**
   * Find a thing with a given name. This function will scan all sectors
   * of the current world and return the first thing that it can find with
   * the given name.
   */
  csThing* GetThing (const char* name);

  /**
   * Return the object managing all loaded textures.
   */
  csTextureList* GetTextures () { return textures; }

  /**
   * Add a dynamic light to the world.
   */
  void AddDynLight (csDynLight* dyn);

  /**
   * Remove a dynamic light from the world.
   */
  void RemoveDynLight (csDynLight* dyn);

  /**
   * Return the first dynamic light in this world.
   */
  csDynLight* GetFirstDynLight () { return first_dyn_lights; }

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
  	csLight** lights, int max_num_lights);

  /**
   * Add a halo to the world.
   */
  void AddHalo (csHaloInformation* iHalo);

  /**
   * Check if a light has a halo attached.
   */
  bool HasHalo (csLight* pLight);

  /**
   * Draw the world given a camera and a clipper. Note that
   * in order to be able to draw using the given 3D driver
   * all textures must have been registered to that driver (using
   * Prepare()). Note that you need to call Prepare() again if
   * you switch to another 3D driver.
   */
  void Draw (csCamera* c, csClipper* clipper);

  /**
   * This function is similar to Draw. It will do all the stuff
   * that Draw would do except for one important thing: it will
   * not draw anything. Instead it will call a callback function for
   * every entity that it was planning to draw. This allows you to show
   * or draw debugging information (2D egdes for example).
   */
  void DrawFunc (csCamera* c, csClipper* clipper,
    csDrawFunc* callback, void* callback_data = NULL);

  /**
   * Locate the first static light which is closer than 'dist' to the
   * given position. This function scans all sectors and locates
   * the first one which statisfies that criterium.
   */
  csStatLight* FindLight (float x, float y, float z, float dist);

  /**
   * Advance the frames of all sprites given an elapsed time.
   */
  void AdvanceSpriteFrames (time_t current_time);

  /**
   * Unlink a sprite from the world (but do not delete it).
   * It is also removed from all sectors.
   */
  void UnlinkSprite (csSprite3D* sprite);

  /**
   * Unlink and delete a sprite from the world.
   * It is also removed from all sectors.
   */
  void RemoveSprite (csSprite3D* sprite);

  /**
   * Create an iterator to iterate over all polygons of the world.
   */
  csPolyIt* NewPolyIterator ()
  {
    csPolyIt* it;
    CHK (it = new csPolyIt (this));
    return it;
  }

  /**
   * Create an iterator to iterate over all static lights of the world.
   */
  csLightIt* NewLightIterator ()
  {
    csLightIt* it;
    CHK (it = new csLightIt (this));
    return it;
  }

  CSOBJTYPE;
  DECLARE_IBASE;

  //--------------------- iPlugIn interface implementation --------------------

  /**
   * Initialize the world. This is automatically called by system driver
   * at startup so that plugin can do basic initialization stuff, register
   * with the system driver and so on.
   */
  virtual bool Initialize (iSystem* sys);

  /// We need to handle some events
  virtual bool HandleEvent (csEvent &Event);

  //--------------------- iWorld interface implementation ---------------------

  csWorld *GetCsWorld () { return this; };

  //--------------------- iConfig interface implementation --------------------

  ///
  virtual int GetOptionCount ();
  ///
  virtual bool GetOptionDescription (int idx, csOptionDescription *option);
  ///
  virtual bool SetOption (int id, csVariant* value);
  ///
  virtual bool GetOption (int id, csVariant* value);
};

#endif // __CS_WORLD_H__
