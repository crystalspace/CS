/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
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

#include "imap/parser.h"

#include "csparser/loadinfo.h"
#include "csutil/csvector.h"
#include "csutil/util.h"
#include "csgeom/quaterni.h"

struct iImageLoader;
struct iSoundLoader;
struct iEngine;
struct iVFS;
struct iGraphics3D;
struct iSoundRender;

struct csRGBcolor;
struct iMotion;
struct iMotionAnim;
struct iSkeletonLimb;
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
class csTerrainFactoryWrapper;
class csTerrainWrapper;

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
class csLoader : public iLoaderNew
{
  struct LoadedPlugin
  {
    LoadedPlugin (const char* shortName, const char *theName,
    	iPlugIn *thePlugin);
    ~LoadedPlugin ();
    char* short_name;
    char* name;
    iPlugIn* plugin;
  };
  
  class csLoadedPluginVector : public csVector
  {
    private:
      LoadedPlugin* FindPlugInPrivate (const char* name)
      {
        int i;
        for (i=0 ; i<Length () ; i++) 
	{
	  LoadedPlugin* pl = (LoadedPlugin*)Get (i);
	  if (pl->short_name && !strcmp (name, pl->short_name)) 
	    return pl;
	  if (!strcmp (name, pl->name)) 
	    return pl;
	}
	return NULL;
      }
    public:
      csLoadedPluginVector (int iLimit = 8, int iThresh = 16)
      	: csVector (iLimit, iThresh) {}
      ~csLoadedPluginVector () { DeleteAll (); }
      
      virtual bool FreeItem (csSome Item)
      { delete (LoadedPlugin*)Item; return true;}
  
      iPlugIn* FindPlugIn (const char* name, const char* classID);
      
      void NewPlugIn (char* short_name, char* name, iPlugIn* plugin)
      { Push (new LoadedPlugin (short_name, name, plugin)); }
  };
  
  static csLoadedPluginVector loaded_plugins;
  
  /// Parse a matrix definition
  static bool load_matrix (char* buf, csMatrix3 &m);
  /// Parse a vector definition
  static bool load_vector (char* buf, csVector3 &v);
  /// Parse a quaternion definition
  static bool load_quaternion (char* buf, csQuaternion &q);
  /// Parse a color definition
  static bool load_color (char *buf, csRGBcolor &c);
  /// Parse a collection definition and return a new object
  static csCollection* load_collection (char* name, char* buf);
  /// Parse a static light definition and return a new object
  static csStatLight* load_statlight (char* name, char* buf);
  /// Parse a key definition and return a new object
  static csKeyValuePair* load_key (char* buf, csObject* pParent);
  /// Parse a map node definition and return a new object
  static csMapNode* load_node (char* name, char* buf, csSector* sec);

  /// Parse and load a single texture
  static void txt_process (char *name, char* buf);
  /// Parse and load a single material
  static void mat_process (char *name, char* buf, const char* prefix = NULL);

  /// Parse a sector definition and return a new object
  static csSector* load_sector (char* secname, char* buf);

  /// Load a Mesh Object Factory from the map file.
  static bool LoadMeshObjectFactory (csMeshFactoryWrapper* meshFact, char* buf);

  /**
   * Load the mesh object from the map file.
   */
  static bool LoadMeshObject (csMeshWrapper* mesh, char* buf, csSector* sector);

  /// Load a Terrain Object Factory from the map file.
  static bool LoadTerrainObjectFactory (csTerrainFactoryWrapper* pTerrFact, char* buf);

  /**
   * Load the terrain object from the map file.
   */
  static bool LoadTerrainObject( csTerrainWrapper *pTerrain, char* buf,
  	csSector* pSector );

  /**
   * Load a plugin in general.
   */
  static bool LoadAddOn (char* buf, iBase* context);

  /**
   * Load the render priority section.
   */
  static bool LoadRenderPriorities (char* buf);

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
   * Load all the plugin descriptions from the map file
   * (the plugins are not actually loaded yet).
   */
  static bool LoadPlugins (char* buf);

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

  /// Load a Mesh Object Factory from the map file.
  static csMeshFactoryWrapper* LoadMeshObjectFactory (csEngine*, const char* fname);

  /**
   * Load a mesh object from a file.
   * The mesh object is not automatically added to the engine and sector.
   */
  static csMeshWrapper* LoadMeshObject (csEngine*, const char* fname);

  /// Load a sound and return an iSoundData object
  static iSoundHandle* LoadSoundHandle(const char* filename);

  /// Load a sound and add it to the engine
  static csSoundDataObject *LoadSoundObject (csEngine*, char* name,
    const char* fname);

  /// Set loader mode (see CS_LOADER_XXX flags above)
  static void SetMode (int iFlags);


  /********** iLoaderNew implementation **********/

  // this is temporarily used by the rest of csLoader
  static iLoaderNew *GlobalLoader;
  
  DECLARE_IBASE;
  
  // temporary wrapper to avoid name collision
  struct {
    // virtual file system
    iVFS *VFS;
    // image loader
    iImageLoader *ImageLoader;
    // sound loader
    iSoundLoader *SoundLoader;
    // engine
    iEngine *Engine;
    // graphics renderer
    iGraphics3D *G3D;
    // sound renderer
    iSoundRender *SoundRender;
  } tmpWrap;

  // constructor
  csLoader(iBase *p);
  // destructor
  virtual ~csLoader();
  // initialize the plug-in
  virtual bool Initialize(iSystem *System);

  virtual iImage *LoadImage (const char *fname);
  virtual iTextureHandle *LoadTexture (const char *fname, int flags);
  virtual iTextureWrapper *LoadTexture (const char *name, const char *fname, int flags);
};

#endif // __CS_CSLOADER_H__
