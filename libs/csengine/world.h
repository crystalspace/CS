/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef WORLD_H
#define WORLD_H

#include "cscom/com.h"
#include "csgeom/math3d.h"
#include "csengine/basic/csobjvec.h"
#include "csobject/csobj.h"

class csSector;
class csTextureList;
class csScript;
class csSprite3D;
class csPolygon3D;
class csCamera;
class csThing;
class csThingTemplate;
class csCollection;
class LanguageLayer;
class csStatLight;
class csDynLight;
class Archive;
class csSpriteTemplate;
class csWireFrameCam;
class csLibrary;
class csClipper;
class Dumper;
class csLight;
class csTextureHandle;
class csHaloInformation;
class csIniFile;
class csEngineConfig;
interface IHaloRasterizer;
interface IGraphics3D;
interface IGraphicsInfo;
interface ISystem;
interface IConfig;

// Several map modes. @@@ This is application specific, should move outside world!
#define MAP_OFF 0
#define MAP_OVERLAY 1
#define MAP_ON 2

/**
 * The world! This class basicly represents the 3D engine.
 * It is the main anchor class for working with Crystal Space.
 */
class csWorld : public csObject
{
  friend class Dumper;

public:
  /**
   * List of sectors in the world. This vector contains
   * objects of type csSector*. Use NewSector() to add sectors
   * to the world.
   */
  csObjVector sectors;

  /**
   * List of libraries. This vector contains objects of type
   * csLibrary*. You can use CSLoader::LoadLibrary() to add a new library
   * or manipulate this vector yourselves.
   */
  csObjVector libraries;

  /**
   * List of planes. This vector contains objects of type
   * csPolyPlane*. Note that this vector only contains named
   * planes. Default planes which are created for polygons
   * are not in this list.
   */
  csObjVector planes;

  /**
   * List of scripts. Obsolete but still functional.
   */
  csObjVector scripts;

  /**
   * List of collections. This vector contains objects of type
   * csCollection*.
   */
  csObjVector collections;

  /**
   * List of sprite templates. This vector contains objects of
   * type csSpriteTemplate*. You can use GetSpriteTemplate() to locate
   * a template for a sprite. This function can optionally look in
   * all loaded libraries as well.
   */
  csObjVector sprite_templates;

  /**
   * List of thing templates. This vector contains objects of
   * type csThingTemplate*. You can use GetThingTemplate() to locate
   * a template for a thing. This function can optionally look in
   * all loaded libraries as well.
   */
  csObjVector thing_templates;

  /**
   * List of all sprites in the world. This vector contains objects
   * of type csSprite3D*. Use UnlinkSprite() and RemoveSprite()
   * to unlink and/or remove sprites from this list. These functions
   * take care of correctly removing the sprites from all sectors
   * as well. Note that after you add a sprite to the list you still
   * need to add it to all sectors that you want it to be visible in.
   */
  csObjVector sprites;

  // Shift for perspective correction (corresponds to half of frame size).
  static float shift_x, shift_y;
  // Remember dimensions of display.
  static int frame_width, frame_height;
  // Remember ISystem interface.
  static ISystem* isys;
  // Current world.
  static csWorld* current_world;

private:
  /// Texture and color information object.
  csTextureList* textures;
  /// Linked list of dynamic lights.
  csDynLight* first_dyn_lights;
  /**
   * If world file was loaded from a ZIP archive.
   * If not this points to the 'precalc.zip' archive.
   */
  Archive* world_file;
  /// List of halos (csHaloInformation).
  csVector halos;  
  /// The Halo rasterizer. If NULL halo's are not supported by the rasterizer.
  IHaloRasterizer* piHR;
  /// The engine configurator object.
  csEngineConfig* cfg_engine;

  ///
  void ShineLights ();
  ///
  void CreateLightmaps (IGraphics3D* g3d);

public:
  /// A WireFrame object for the map. @@@ DOES NOT BELONG IN WORLD
  csWireFrameCam* wf;
  /// Map mode. @@@ DOES NOT BELONG IN WORLD
  int map_mode;

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

public:
  /**
   * Initialize an empty world. The only thing that is valid just
   * after creating the world is the configurator object which you
   * can use to configure the world before continuing (see GetEngineConfig()).
   */
  csWorld ();

  /**
   * Delete the world and all entities in the world. All objects added to this
   * world by the user (like Things, Sectors, ...) will be deleted as well. If
   * you don't want this then you should unlink them manually before destroying
   * the world.
   */
  virtual ~csWorld ();

  /**
   * Initialize the world. This function must be called before
   * you do anything else with this world. It will read the configuration
   * file (ReadConfig()) and start a new empty world (StartWorld()).
   */
  bool Initialize (ISystem* sys, IGraphicsInfo* ginfo, csIniFile* config);

  /**
   * Prepare the world. This function must be called after
   * you loaded/created the world. It will prepare all lightmaps
   * for use and also free all images that were loaded for
   * the texture manager (the texture manager should have them
   * locally now).
   */
  bool Prepare (IGraphics3D* g3d);

  /**
   * Read configuration file for all engine specific values.
   * This function is called by Initialize() so you normally do
   * not need to call it yourselves.
   */
  void ReadConfig (csIniFile* config);

  /**
   * Prepare for creation of a world. This function is called
   * by Initialize() so you normally do not need to call it
   * yourselves.
   */
  void StartWorld ();

  /**
   * Get the Halo Rasterizer COM interface if supported (NULL if not).
   */
  IHaloRasterizer* GetHaloRastizer () { return piHR; }

  /**
   * Get the configurator class for this engine. This class can be
   * used to query/set engine specific settings.
   */
  csEngineConfig* GetEngineConfig () { return cfg_engine; }

  /**
   * Get the configurator interface for this engine. This interface
   * can be used to query/set engine specific settings. This is the COM
   * version.
   */
  IConfig* GetEngineConfigCOM ();

  /**
   * Get the default world archive. If there is none this function
   * will open precalc.zip and use that.
   */
  Archive* GetWorldFile ();

  /**
   * Open a file as the world file.
   */
  Archive* OpenWorldFile (char* filename);

  /**
   * Close the world file.
   */
  void CloseWorldFile ();

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
  csSpriteTemplate* GetSpriteTemplate (char* name, bool use_libs = false);

  /**
   * Find a named thing template in the loaded world and
   * optionally in all loaded libraries. This template can then
   * be used to create things.
   */
  csThingTemplate* GetThingTemplate (char* name, bool use_libs = false);

  /**
   * Find a thing with a given name. This function will scan all sectors
   * of the current world and return the first thing that it can find with
   * the given name.
   */
  csThing* GetThing (char* name);

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
   * Add a halo to the world.
   */
  void AddHalo (csHaloInformation* pinfo);

  /**
   * Check if a light has a halo attached.
   */
  bool HasHalo (csLight* pLight);

  /**
   * Create a new script. 'params' should start with a
   * qualifier indicating the language of the script.
   * The syntax is as follows:<p>
   *     SCRIPT 'name' (prim: dim 1,0,0)<p>
   *	 SCRIPT 'name' (C:script1.c)
   */
  csScript* NewScript (LanguageLayer* layer, char* name, char* params);

  /**
   * Activate all triggers valid at this camera position.
   */
  void TriggerActivate (csCamera& c);

  /**
   * Draw the world given a camera and a clipper. Note that
   * in order to be able to draw using the given 3D driver
   * all textures must have been registered to that driver (using
   * Prepare()). Note that you need to call Prepare() again if
   * you switch to another 3D driver.
   */
  void Draw (IGraphics3D* g3d, csCamera* c, csClipper* clipper);

  /**
   * For debugging purposes: set the polygon that should
   * be hilighted.
   */
  void SetHilight (csPolygon3D* hi);

  /// Get the hilighted polygon.
  csPolygon3D* GetHilight ();

  /**
   * Locate the first static light which is closer than 'dist' to the
   * given position. This function scans all sectors and locates
   * the first one which statisfies that criterium.
   */
  csStatLight* FindLight (float x, float y, float z, float dist);

  /**
   * Advance the frames of all sprites given an elapsed time.
   */
  void AdvanceSpriteFrames (long current_time);

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

  CSOBJTYPE;
};

#endif /*WORLD_H*/
