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

#include "csutil/csvector.h"
#include "csutil/util.h"
#include "csgeom/quaterni.h"

class csGenerateImageTexture;
class csGenerateImageValue;
struct csRGBcolor;

struct iImageIO;
struct iSoundLoader;
struct iEngine;
struct iVFS;
struct iGraphics3D;
struct iSoundRender;
struct iMotionManager;
struct iLoaderPlugIn;

struct iObject;
struct iThingState;
struct iCollection;
struct iSkeletonLimb;
struct iMaterialWrapper;
struct iMeshFactoryWrapper;
struct iMeshWrapper;
struct iTerrainFactoryWrapper;
struct iTerrainWrapper;
struct iSector;
struct iStatLight;
struct iKeyValuePair;
struct iMapNode;

/**
 * The loader for Crystal Space maps.
 */
class csLoader : public iLoader
{
  class csLoadedPluginVector : public csVector
  {
  private:
    // Find a loader plugin record
    struct csLoaderPluginRec* FindPlugInRec (const char* name);
    // Return the loader plugin from a record, possibly loading the plugin now
    iLoaderPlugIn* GetPluginFromRec (csLoaderPluginRec*, const char *FuncID);
  public:
    iSystem *System;

    // constructor
    csLoadedPluginVector (int iLimit = 8, int iThresh = 16);
    // destructor
    ~csLoadedPluginVector ();
    // delete a plugin record
    virtual bool FreeItem (csSome Item);
    // find a plugin by its name or load it if it doesn't exist
    iLoaderPlugIn* FindPlugIn (const char* Name, const char* FuncID);
    // add a new plugin record
    void NewPlugIn (const char* ShortName, const char* ClassID);
  };
  
  /// List of loaded plugins
  csLoadedPluginVector loaded_plugins;
  /// Loader flags
  int flags;
  /**
   * If true then references to other objects (e.g. textures, mesh factories
   * etc.) are only resolved if the referenced object exists in the current
   * region.
   */
  bool ResolveOnlyRegion;
  /// Statistics
  class csLoaderStats *Stats;

  /// -----------------------------------------------------------------------
  
  /// Parse a matrix definition
  bool ParseMatrix (char* buf, csMatrix3 &m);
  /// Parse a vector definition
  bool ParseVector (char* buf, csVector3 &v);
  /// Parse a quaternion definition
  bool ParseQuaternion (char* buf, csQuaternion &q);
  /// Parse a color definition
  bool ParseColor (char *buf, csRGBcolor &c);

  /// Parse a texture definition and add the texture to the engine
  iTextureWrapper* ParseTexture (char *name, char* buf);
  /// Parse a proc texture definition and add the texture to the engine
  iTextureWrapper* ParseProcTex (char *name, char* buf);
  /// Parse a material definition and add the material to the engine
  iMaterialWrapper* ParseMaterial (char *name, char* buf, const char* prefix = NULL);
  /// Parse a collection definition and add the collection to the engine
  iCollection* ParseCollection (char* name, char* buf);
  /// Parse a static light definition and add the light to the engine
  iStatLight* ParseStatlight (char* name, char* buf);
  /// Parse a key definition and add the key to the given object
  iKeyValuePair* ParseKey (char* buf, iObject* pParent);
  /// Parse a map node definition and add the node to the given sector
  iMapNode* ParseNode (char* name, char* buf, iSector* sec);
  /// Parse a sector definition and add the sector to the engine
  iSector* ParseSector (char* name, char* buf);

  /// -----------------------------------------------------------------------

  /// parse a texture mixing mode specification
  UInt ParseMixmode (char* buf);

  /// For heightgen.
  csGenerateImageTexture* heightgen_txt_process (char* buf);
  /// For heightgen.
  csGenerateImageValue* heightgen_value_process (char* buf);
  /// Parse and load a height texture
  void heightgen_process (char* buf);

  /// Resolve the portals of a thing
  void ResolvePortalSectors (iThingState *Mesh);

  /// Load a Mesh Object Factory from the map file.
  bool LoadMeshObjectFactory (iMeshFactoryWrapper* meshFact, char* buf);

  /**
   * Load the mesh object from the map file.
   */
  bool LoadMeshObject (iMeshWrapper* mesh, char* buf);

  /// Load a Terrain Object Factory from the map file.
  bool LoadTerrainObjectFactory (iTerrainFactoryWrapper* pTerrFact, char* buf);

  /**
   * Load the terrain object from the map file.
   */
  bool LoadTerrainObject( iTerrainWrapper *pTerrain, char* buf,
  	iSector* pSector );

  /**
   * Load a plugin in general.
   */
  bool LoadAddOn (char* buf, iBase* context);

  /**
   * Load the render priority section.
   */
  bool LoadRenderPriorities (char* buf);

  /**
   * Load sounds from a SOUNDS(...) argument.
   * This function is normally called automatically by the parser.
   */
  bool LoadSounds (char* buf);

  /**
   * Load all the texture descriptions from the map file
   * (no actual images). 
   */
  bool LoadTextures (char* buf);

  /**
   * Load all the plugin descriptions from the map file
   * (the plugins are not actually loaded yet).
   */
  bool LoadPlugins (char* buf);

  /**
   * Load all the material descriptions from the map file
   * (no actual images).If a prefix is given, all material names will be
   * prefixed with the corresponding string.
   */
  bool LoadMaterials (char* buf, const char* prefix = NULL);

  /**
   * Load a library into given engine.<p>
   * A library is just a map file that contains just mesh factories,
   * thing templates, sounds and textures.
   */
  bool LoadLibrary (char* buf);

  /// Load map from a memory buffer
  bool LoadMap (char* buf);

  /// Find a material (and create one from texture if possible)
  iMaterialWrapper* FindMaterial (const char *iName);

  /**
   * Print an error about an unknown token. 'object' is the type of object
   * that was just being parsed, e.g. "a sector". This function will get
   * the unknown token from csGetLastOffender ().
   */
  void TokenError (const char *Object);

public:
  /********** iLoader implementation **********/
  DECLARE_IBASE;
  
  // system driver
  iSystem *System;
  // virtual file system
  iVFS *VFS;
  // image loader
  iImageIO *ImageLoader;
  // sound loader
  iSoundLoader *SoundLoader;
  // engine
  iEngine *Engine;
  // graphics renderer
  iGraphics3D *G3D;
  // sound renderer
  iSoundRender *SoundRender;
  // motion manager
  iMotionManager *MotionManager;

  // constructor
  csLoader(iBase *p);
  // destructor
  virtual ~csLoader();
  // initialize the plug-in
  virtual bool Initialize(iSystem *System);

  virtual void SetMode (int iFlags);

  virtual iImage *LoadImage (const char *fname, int Format);
  virtual iTextureHandle *LoadTexture (const char* fname,
	int Flags = CS_TEXTURE_3D, iTextureManager *tm = NULL);
  virtual iTextureWrapper *LoadTexture (const char *name, const char *fname,
	int Flags = CS_TEXTURE_3D, iTextureManager *tm = NULL);

  virtual iSoundData *LoadSoundData (const char *fname);
  virtual iSoundHandle *LoadSound (const char *fname);
  virtual iSoundWrapper *LoadSound (const char *name, const char *fname);

  virtual bool LoadMapFile (const char* filename, bool clearEngine,
	bool onlyRegion);
  virtual bool LoadLibraryFile (const char* filename);

  virtual iMeshFactoryWrapper* LoadMeshObjectFactory (const char* fname);
  virtual iMeshWrapper* LoadMeshObject (const char* fname);
};

#endif // __CS_CSLOADER_H__
