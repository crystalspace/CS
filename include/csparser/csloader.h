/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Ivan Avramovic <ivan@avramovic.com>
  
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

#ifndef __CS_CSLOADER_H__
#define __CS_CSLOADER_H__

#include "csparser/loadinfo.h"
#include "csutil/csvector.h"
#include "csgeom/quaterni.h"

struct iImage;
struct csRGBcolor;
struct iMotion;
struct iMotionAnim;
struct iSoundData;
struct iSkeletonLimb;
struct iPlugIn;
class csTextureWrapper;
class csMaterialWrapper;
class csPolygonTemplate;
class csPolyPlane;
class csPolyTxtPlane;
class csCollection;
class csStatLight;
class csThing;
class csEngine;
class csCurveTemplate;
class csSoundDataObject;
class csMeshFactoryWrapper;
class csMeshWrapper;
class csParticleSystem;
class csKeyValuePair;
class csMapNode;
class csSector;
class csFrame;
class csObject;
class csPolygon3D;

/**
 * Bit flags for the loader (used in csLoader::SetMode).
 * Some actions may be unwanted during loading, thus these flags.
 */
/// Do not compress vertices
#define CS_LOADER_NOCOMPRESS	0x00000001
/// Do not create BSP/octrees
#define CS_LOADER_NOBSP		0x00000002
/// Do not apply transformations to things (and do not create bounding box)
#define CS_LOADER_NOTRANSFORM	0x00000004

/**
 * The loader for Crystal Space maps.
 */
class csLoader
{
  struct LoadedPlugin
  {
    char* name;
    iPlugIn* plugin;
  };
  static csVector loaded_plugins;
  static iPlugIn* FindPlugIn (const char* name);
  static void NewPlugIn (char* name, iPlugIn* plugin);
  
  /// Parse a matrix definition
  static bool load_matrix (char* buf, csMatrix3 &m);
  /// Parse a vector definition
  static bool load_vector (char* buf, csVector3 &v);
  /// Parse a quaternion definition
  static bool load_quaternion (char* buf, csQuaternion &q);
  /// Parse a color definition
  static bool load_color (char *buf, csRGBcolor &c);
  /// Parse a polygon plane definition and return a new object
  static csPolyTxtPlane* load_polyplane (char* buf, char* name = NULL);
  /// Parse a collection definition and return a new object
  static csCollection* load_collection (char* name, char* buf);
  /// Parse a static light definition and return a new object
  static csStatLight* load_statlight (char* name, char* buf);
  /// Parse a key definition and return a new object
  static csKeyValuePair* load_key (char* buf, csObject* pParent);
  /// Parse a map node definition and return a new object
  static csMapNode* load_node (char* name, char* buf, csSector* sec);
  /// Parse the definition for a skydome and create the corresponding objects
  static void skydome_process (csThing& thing, char* name, char* buf,
    csMaterialWrapper* material);
  /// Parse the terrain engine's parameters
  static void terrain_process (csSector& sector, char* name, char* buf);
  /// Parse the definition for a thing and create a thing object
  static csThing* load_thing (char* name, char* buf, csSector*, bool is_sky,
      	bool is_template);
  static void load_thing_part (csThing* thing, csSector* sec, PSLoadInfo& info,
	csReversibleTransform& obj, char* name, char* buf, int vt_offset,
	bool isParent);
  /// Parse a 3D polygon definition and return a new object
  static csPolygon3D* load_poly3d (char* polyname, char* buf,
    csMaterialWrapper* default_material, float default_texlen,
    csThing* parent, int vt_offset);

  /// Load a image and return an iImage object
  static iImage* load_image(const char* name);
  /// Parse and load a single texture
  static void txt_process (char *name, char* buf);
  /// Parse and load a single material
  static void mat_process (char *name, char* buf, const char* prefix = NULL);
  /// Parse a Bezier surface definition and return a new object
  static csCurveTemplate* load_beziertemplate (char* ptname, char* buf,
    csMaterialWrapper* default_material, float default_texlen,
    csVector3* curve_vertices);

  /// Parse a sector definition and return a new object
  static csSector* load_sector (char* secname, char* buf);

  /// Load a Mesh Object Factory from the map file.
  static bool LoadMeshObjectFactory (csMeshFactoryWrapper* meshFact, char* buf);

  /**
   * Load the mesh object from the map file.
   */
  static bool LoadMeshObject (csMeshWrapper* mesh, char* buf, csSector* sector);

  /**
   * Load sounds from a SOUNDS(...) argument.
   * This function is normally called automatically by the parser.
   */
  static bool LoadSounds (char* buf);

  /**
   * Load all the texture descriptions from the map file
   * (no actual images). 
   */
  static bool LoadTextures (char* buf);

  /**
   * Load all the material descriptions from the map file
   * (no actual images).If a prefix is given, all material names will be
   * prefixed with the corresponding string.
   */
  static bool LoadMaterials (char* buf, const char* prefix = NULL);

  /**
   * Loads a skeletal motion from a file
   */
  static iMotion* LoadMotion (csEngine*, const char* fname);

  /**
   * Loads a skeletal motion from an existing stream
   */
  static bool LoadMotion (iMotion* mot, char* buf);

  /**
   * Load a library into given engine.<p>
   * A library is just a map file that contains just mesh factories,
   * thing templates, sounds and textures.
   */
  static bool LoadLibrary (char* buf);

  /// Load map from a memory buffer
  static bool LoadMap (char* buf, bool onlyRegion);

  /// Find a material (and create one from texture if possible)
  static csMaterialWrapper* FindMaterial (const char *iName, bool onlyRegion = false);

  /**
   * If the polygon is a portal and has no special functions,
   * the texturing mode is reset to POLYTXT_NONE.
   */
  static void OptimizePolygon (csPolygon3D *p);

public:
  /// Load map file into engine.
  static bool LoadMapFile (csEngine*, const char* filename);

  /**
   * Merge map file into engine (i.e. don't clear the current engine
   * contents first). If 'onlyRegion' is true then portals will only
   * connect to the sectors in the current region, things will only use
   * thing templates defined in the current region and meshes will
   * only use mesh factories defined in the current region.
   */
  static bool AppendMapFile (csEngine*, const char* filename,
  	bool onlyRegion = true);

  /// Load library from a VFS file
  static bool LoadLibraryFile (csEngine*, const char* filename);

  /**
   * Load a texture and add it to the engine.
   * The texture will be registered for 3d use only.
   * A corresponding material with the same name will be added too.
   */
  static csTextureWrapper* LoadTexture (csEngine*, const char* name,
    const char* fname);

  /// Load a Mesh Object Factory from the map file.
  static csMeshFactoryWrapper* LoadMeshObjectFactory (csEngine*, const char* fname);

  /**
   * Load a thing from a file.
   */
  static csThing* LoadThing (csEngine*, const char* fname);

  /**
   * Load a thing template from a file.
   */
  static csThing* LoadThingTemplate (csEngine*, const char* fname);

  /// Load a image and return an iImage object
  static iImage* LoadImage (const char* name)
  { return load_image (name); }

  /// Load a sound and return an iSoundData object
  static iSoundData *LoadSoundData (const char *filename);

  /// Load a sound and add it to the engine
  static csSoundDataObject *LoadSoundObject (csEngine*, char* name,
    const char* fname);

  /// Set loader mode (see CS_LOADER_XXX flags above)
  static void SetMode (int iFlags);
};

#endif // __CS_CSLOADER_H__
