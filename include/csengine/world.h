/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein
  
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
#include "csobject/csobject.h"
#include "igraph3d.h"
#include "isystem.h"
#include "iworld.h"
#include "iconfig.h"

class csSector;
class csSprite;
class csTextureList;
class csPolygon3D;
class csCamera;
class csThing;
class csThingTemplate;
class csCollection;
class csStatLight;
class csDynLight;
class csSpriteTemplate;
class csClipper;
class csSolidBsp;
class csCovcube;
class csCBufferCube;
class csWorld;
class Dumper;
class csLight;
class csCBuffer;
class csCoverageMaskTree;
class csCovMaskLUT;
class csPoly2DPool;
class csLightPatchPool;
struct iSystem;
struct iVFS;

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
class csWorld : public iWorld, public csObject
{
  friend class Dumper;

  // Private class for keeping an array of halos
  class csHaloVector : public csVector
  {
  public:
    // Constructor
    csHaloVector () : csVector (16, 16) {}
    // Destructor
    virtual ~csHaloVector () { DeleteAll (); }
    // Free an item from array
    virtual bool FreeItem (csSome Item)
    { CHK (delete (csLightHalo *)Item); return true; }
    // Find a halo by referenced light
    virtual int CompareKey (csSome Item, csConstSome Key, int /*Mode*/) const
    { return ((csLightHalo *)Item)->Light == (csLight *)Key ? 0 : -1; }
    // Return an reference to Nth halo info
    inline csLightHalo *Get (int n) const
    { return (csLightHalo *)csVector::Get (n); }
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
   * csPolyTxtPlane*. Note that this vector only contains named
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
   * of type csSprite*. Use UnlinkSprite() and RemoveSprite()
   * to unlink and/or remove sprites from this list. These functions
   * take care of correctly removing the sprites from all sectors
   * as well. Note that after you add a sprite to the list you still
   * need to add it to all sectors that you want it to be visible in.
   */
  csNamedObjVector sprites;

  /**
   * List of all particle systems in the world. This vector contains
   * objects of type csParticleSystem*. Note that the sprites of a
   * particle system are in the sprites list. 
   */
  csNamedObjVector particle_systems;

  /**
   * The list of all camera position objects.
   */
  csNamedObjVector camera_positions;

  /// Remember dimensions of display.
  static int frame_width, frame_height;
  /// Remember iSystem interface.
  static iSystem* System;
  /// Current world.
  static csWorld* current_world;
  /// Need to render using newradiosity?
  static bool use_new_radiosity;
  /// An object pool for 2D polygons used by the rendering process.
  csPoly2DPool* render_pol2d_pool;
  /// An object pool for lightpatches.
  csLightPatchPool* lightpatch_pool;
  /// The transformation manager.
  csTransformationManager tr_manager;
  /// The 3D driver
  iGraphics3D* G3D;
  /// The fog mode this G3D implements
  G3D_FOGMETHOD fogmethod;
  /// Does the 3D driver require power-of-two lightmaps?
  bool NeedPO2Maps;
  /// Maximum texture aspect ratio
  int MaxAspectRatio;
  /// A pointer to current object library
  csObject *Library;
  /// The list of all object libraries currently loaded
  csNamedObjVector Libraries;

  /// Option variable: inhibit lightmap recalculation?
  static bool do_not_force_relight;
  /// Option variable: force lightmap recalculation?
  static bool do_force_relight;
  /// Option variable: inhibit visibility recalculation?
  static bool do_not_force_revis;
  /// Option variable: force visibility recalculation?
  static bool do_force_revis;

private:
  /// Texture and color information object.
  csTextureList* textures;
  /// Linked list of dynamic lights.
  csDynLight* first_dyn_lights;
  /// List of halos (csHaloInformation).
  csHaloVector halos;  
  /// If true then the lighting cache is enabled.
  static bool do_lighting_cache;
  /// Debugging: maximum number of polygons to process in one frame.
  static int max_process_polygons;
  /// Current number of processed polygons.
  static int cur_process_polygons;

  /// Optional c-buffer used for rendering.
  csCBuffer* c_buffer;

  /// Optional solid BSP used for rendering.
  csSolidBsp* solidbsp;

  /// Optional coverage mask tree used for rendering.
  csCoverageMaskTree* covtree;

  /// Lookup table for coverage mask trees.
  csCovMaskLUT* covtree_lut;

  /// Coverage mask cube used for lighting.
  csCovcube* covcube;

  /// C-buffer cube used for lighting.
  csCBufferCube* cbufcube;

  /// Use PVS.
  bool use_pvs;

  /// Flag set when window resized.
  bool resize;

public:
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
   * for all found errors. Returns true if everything is in order.
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
   * Calculate all lighting information. Normally you shouldn't call
   * this function directly, because it will be called by Prepare().
   */
  void ShineLights ();

  /**
   * Prepare the world. This function must be called after
   * you loaded/created the world. It will prepare all lightmaps
   * for use and also free all images that were loaded for
   * the texture manager (the texture manager should have them
   * locally now).
   */
  bool Prepare ();

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
   * Enable/disable c-buffer.
   */
  void EnableCBuffer (bool en);

  /**
   * Return c-buffer (or NULL if not used).
   */
  csCBuffer* GetCBuffer () { return c_buffer; }

  /**
   * Enable/disable coverage mask tree.
   */
  void EnableCovtree (bool en);

  /**
   * Return coverage mask tree (or NULL if not used).
   */
  csCoverageMaskTree* GetCovtree () { return covtree; }

  /**
   * Return the lookup table used for the coverage mask tree.
   */
  csCovMaskLUT* GetCovMaskLUT () { return covtree_lut; }

  /**
   * Enable/disable solidbsp.
   */
  void EnableSolidBsp (bool en);

  /**
   * Return solidbsp (or NULL if not used).
   */
  csSolidBsp* GetSolidBsp () { return solidbsp; }

  /**
   * Return coverage mask cube.
   */
  csCovcube* GetCovcube () { return covcube; }

  /**
   * Return cbuffer cube.
   */
  csCBufferCube* GetCBufCube () { return cbufcube; }

  /**
   * Enable PVS.
   */
  void EnablePVS () { use_pvs = true; }

  /**
   * Disable PVS.
   */
  void DisablePVS () { use_pvs = false; }

  /**
   * Is PVS enabled?
   */
  bool IsPVS () { return use_pvs; }

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
   * Clear everything in the world.
   */
  void Clear ();

  /**
   * Create a new sector and add it to the world.
   * @@obsoleted by CreateSector(): should be removed in the long run
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
   * Add a halo attached to given light to the world.
   */
  void AddHalo (csLight* Light);

  /**
   * Check if a light has a halo attached.
   */
  bool HasHalo (csLight* Light);

  /**
   * Remove halo attached to given light from the world.
   */
  void RemoveHalo (csLight* Light);

  /**
   * Process a existing light halo. The function changes halo brightness
   * in dependence whenever the halo is obscured or not and returns "false"
   * if halo has reached zero intensity and should be removed from halo queue.
   * The function also actually projects, clips and draws the halo.
   */
  bool ProcessHalo (csLightHalo *Halo);

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
   * Advance the frames of all sprites given the current time.
   */
  void AdvanceSpriteFrames (time_t current_time);

  /**
   * Update the particle systemns given an elapsed time.
   */
  void UpdateParticleSystems (time_t elapsed_time);

  /**
   * Unlink a sprite from the world (but do not delete it).
   * It is also removed from all sectors.
   */
  void UnlinkSprite (csSprite* sprite);

  /**
   * Unlink and delete a sprite from the world.
   * It is also removed from all sectors.
   */
  void RemoveSprite (csSprite* sprite);

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

  virtual csWorld *GetCsWorld () { return this; };

  /// Query the format to load textures (usually this depends on texture manager)
  virtual int GetTextureFormat ();

  /**
   * Create or select a new object library (name can be NULL for world).
   * All new objects will be marked as belonging to this library.
   * You can then delete a whole library at once, for example.
   */
  virtual void SelectLibrary (const char *iName);
  /// Delete a whole library (all objects that are part of library)
  virtual bool DeleteLibrary (const char *iName);
  /// Clear the entire world (delete all libraries)
  virtual void DeleteAll ();

  /// Register a texture to be loaded during Prepare()
  virtual bool CreateTexture (const char *iName, const char *iFileName,
    csColor *iTransp, int iFlags);
  /// Create a named camera position object
  virtual bool CreateCamera (const char *iName, const char *iSector,
    const csVector3 &iPos, const csVector3 &iForward, const csVector3 &iUpward);
  /// Create a key/value pair object
  virtual bool CreateKey (const char *iName, const char *iValue);
  /// Create a texture plane
  virtual bool CreatePlane (const char *iName, const csVector3 &iOrigin,
    const csMatrix3 &iMatrix);
  /// Create a empty sector with given name
  iSector *CreateSector (const char *iName);
  /// Create a empty thing with given name
  virtual iThing *CreateThing (const char *iName, iSector *iParent);

  //--------------------- iConfig interface implementation --------------------

  struct csWorldConfig : public iConfig
  {
    DECLARE_EMBEDDED_IBASE (csWorld);
    virtual bool GetOptionDescription (int idx, csOptionDescription *option);
    virtual bool SetOption (int id, csVariant* value);
    virtual bool GetOption (int id, csVariant* value);
  } scfiConfig;
};

// This is a global replacement for printf ()
#define CsPrintf csWorld::System->Printf

#endif // __CS_WORLD_H__
