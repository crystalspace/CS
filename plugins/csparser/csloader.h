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
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/csvector.h"
#include "csutil/util.h"
#include "csutil/strhash.h"
#include "csutil/parser.h"
#include "csgeom/quaterni.h"
#include "iutil/plugin.h"
#include "imap/services.h"

class csGenerateImageTexture;
class csGenerateImageValue;
class csReversibleTransform;
struct csRGBcolor;
class csColor;

struct iImageIO;
struct iSoundLoader;
struct iEngine;
struct iVFS;
struct iGraphics3D;
struct iSoundRender;
struct iLoaderPlugin;
struct iObjectRegistry;
struct iPluginManager;
struct iModelConverter;
struct iCrossBuilder;
struct iCameraPosition;
struct iDocumentNode;

struct iObject;
struct iThingState;
struct iCollection;
struct iSkeletonLimb;
struct iMaterialWrapper;
struct iMeshFactoryWrapper;
struct iMeshWrapper;
struct iSector;
struct iStatLight;
struct iKeyValuePair;
struct iMapNode;
struct iReporter;
struct iLODControl;
struct iLoaderContext;

enum
{
  XMLTOKEN_AMBIENT = 1,
  XMLTOKEN_ADDON,
  XMLTOKEN_ATTENUATION,
  XMLTOKEN_CAMERA,
  XMLTOKEN_CENTER,
  XMLTOKEN_CLEARZBUF,
  XMLTOKEN_CLEARSCREEN,
  XMLTOKEN_COLLECTION,
  XMLTOKEN_COLOR,
  XMLTOKEN_CONVEX,
  XMLTOKEN_CULLER,
  XMLTOKEN_CULLERP,
  XMLTOKEN_DETAIL,
  XMLTOKEN_DYNAMIC,
  XMLTOKEN_DITHER,
  XMLTOKEN_DIFFUSE,
  XMLTOKEN_FACTORY,
  XMLTOKEN_FARPLANE,
  XMLTOKEN_FILE,
  XMLTOKEN_FOG,
  XMLTOKEN_FORWARD,
  XMLTOKEN_FOR2D,
  XMLTOKEN_FOR3D,
  XMLTOKEN_HALO,
  XMLTOKEN_HARDMOVE,
  XMLTOKEN_HEIGHTGEN,
  XMLTOKEN_INVISIBLE,
  XMLTOKEN_KEY,
  XMLTOKEN_LAYER,
  XMLTOKEN_LEVEL,
  XMLTOKEN_LIBRARY,
  XMLTOKEN_LIGHT,
  XMLTOKEN_LIGHTMAPCELLSIZE,
  XMLTOKEN_LMCACHE,
  XMLTOKEN_LOD,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MATERIALS,
  XMLTOKEN_MATRIX,
  XMLTOKEN_MAXLIGHTMAPSIZE,
  XMLTOKEN_MESHFACT,
  XMLTOKEN_MESHLIB,
  XMLTOKEN_MESHOBJ,
  XMLTOKEN_MESHREF,
  XMLTOKEN_MOVE,
  XMLTOKEN_MIPMAP,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_NODE,
  XMLTOKEN_NOLIGHTING,
  XMLTOKEN_NOSHADOWS,
  XMLTOKEN_PARAMS,
  XMLTOKEN_PARAMSFILE,
  XMLTOKEN_PLUGIN,
  XMLTOKEN_PLUGINS,
  XMLTOKEN_POSITION,
  XMLTOKEN_PRIORITY,
  XMLTOKEN_PROCTEX,
  XMLTOKEN_PROCEDURAL,
  XMLTOKEN_PERSISTENT,
  XMLTOKEN_RADIUS,
  XMLTOKEN_REGION,
  XMLTOKEN_RENDERPRIORITIES,
  XMLTOKEN_REFLECTION,
  XMLTOKEN_SCALE,
  XMLTOKEN_SECTOR,
  XMLTOKEN_SETTINGS,
  XMLTOKEN_SHIFT,
  XMLTOKEN_SOUND,
  XMLTOKEN_SOUNDS,
  XMLTOKEN_START,
  XMLTOKEN_TEXTURE,
  XMLTOKEN_TEXTURES,
  XMLTOKEN_TRANSPARENT,
  XMLTOKEN_TYPE,
  XMLTOKEN_MATSET,
  XMLTOKEN_UP,
  XMLTOKEN_V,
  XMLTOKEN_WORLD,
  XMLTOKEN_ZFILL,
  XMLTOKEN_ZNONE,
  XMLTOKEN_ZUSE,
  XMLTOKEN_ZTEST,
  XMLTOKEN_BLEND,
  XMLTOKEN_CONSTANT,
  XMLTOKEN_GENERATE,
  XMLTOKEN_HEIGHT,
  XMLTOKEN_HEIGHTMAP,
  XMLTOKEN_MULTIPLY,
  XMLTOKEN_PARTSIZE,
  XMLTOKEN_SINGLE,
  XMLTOKEN_SIZE,
  XMLTOKEN_SLOPE,
  XMLTOKEN_SOLID,
  XMLTOKEN_VALUE
};

/**
 * The loader for Crystal Space maps.
 */
class csLoader : public iLoader
{
private:
  iLoaderContext* ldr_context;
  iLoaderContext* GetLoaderContext ();
  csStringHash xmltokens;

  /// the current parser
  csParser parser;
  /// Parser for common stuff like MixModes
  iSyntaxService* SyntaxService;

  class csLoaderStats
  {
  public:
    int polygons_loaded;
    int portals_loaded;
    int sectors_loaded;
    int things_loaded;
    int lights_loaded;
    int curves_loaded;
    int meshes_loaded;
    int sounds_loaded;

    void Init();
    csLoaderStats();
  };

  class csLoadedPluginVector : public csVector
  {
  private:
    // Find a loader plugin record
    struct csLoaderPluginRec* FindPluginRec (const char* name);
    // Return the loader plugin from a record, possibly loading the plugin now
    iLoaderPlugin* GetPluginFromRec (csLoaderPluginRec*);
  public:
    iPluginManager* plugin_mgr;

    // constructor
    csLoadedPluginVector (int iLimit = 8, int iThresh = 16);
    // destructor
    ~csLoadedPluginVector ();
    // delete a plugin record
    virtual bool FreeItem (csSome Item);
    // find a plugin by its name or load it if it doesn't exist
    iLoaderPlugin* FindPlugin (const char* Name);
    // add a new plugin record
    void NewPlugin (const char* ShortName, const char* ClassID);
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

  /// Parse a quaternion definition
  bool ParseQuaternion (char* buf, csQuaternion &q);
  /// Parse a color definition
  bool ParseColor (char *buf, csRGBcolor &c);
  /// Parse a color definition
  bool ParseColor (char *buf, csColor &c);

  /// Parse a list of textures and add them to the engine.
  bool ParseTextureList (char* buf);
  /**
   * Parse a list of materials and add them to the engine. If a prefix is
   * given, all material names will be prefixed with the corresponding string.
   */
  bool ParseMaterialList (char* buf, const char* prefix = NULL);
  /// Parse a texture definition and add the texture to the engine
  iTextureWrapper* ParseTexture (char *name, char* buf);
  /// Parse a proc texture definition and add the texture to the engine
  iTextureWrapper* ParseProcTex (char *name, char* buf);
  /// Parse a material definition and add the material to the engine
  iMaterialWrapper* ParseMaterial (char *name, char* buf, const char* prefix = NULL);
  /// Parse a collection definition and add the collection to the engine
  iCollection* ParseCollection (char* name, char* buf);
  /// Parse a camera position.
  bool ParseStart (char* buf, iCameraPosition* campos);
  /// Parse a static light definition and add the light to the engine
  iStatLight* ParseStatlight (char* name, char* buf);
  /// Parse a key definition and add the key to the given object
  iKeyValuePair* ParseKey (char* buf, iObject* pParent);
  /// Parse a map node definition and add the node to the given sector
  iMapNode* ParseNode (char* name, char* buf, iSector* sec);
  /// Parse a sector definition and add the sector to the engine
  iSector* ParseSector (char* name, char* buf);

  /// -----------------------------------------------------------------------

  /// For heightgen.
  csGenerateImageTexture* ParseHeightgenTexture (char* buf);
  /// For heightgen.
  csGenerateImageValue* ParseHeightgenValue (char* buf);
  /// Parse and load a height texture
  bool ParseHeightgen (char* buf);

  /**
   * Load a LOD control object.
   */
  bool LoadLodControl (iLODControl* lodctrl, char* buf);

  /**
   * Load a Mesh Object Factory from the map file.
   * If the transformation pointer is given then this is for a hierarchical
   * mesh object factory and the transformation will be filled in with
   * the relative transform (from MOVE keyword).
   */
  bool LoadMeshObjectFactory (iMeshFactoryWrapper* meshFact, char* buf,
  	csReversibleTransform* transf = NULL);

  /**
   * Load the mesh object from the map file.
   */
  bool LoadMeshObject (iMeshWrapper* mesh, char* buf);

  /**
   * Load the mesh object from the map file.
   * This version will parse FACTORY statement to directly create
   * a mesh from a factory.
   */
  iMeshWrapper* LoadMeshObjectFromFactory (char* buf);

  /**
   * Load a plugin in general.
   */
  bool LoadAddOn (char* buf, iBase* context);

  /**
   * Load the render priority section.
   */
  bool LoadRenderPriorities (char* buf);

  /**
   * Load the settings section.
   */
  bool LoadSettings (char* buf);

  /**
   * Load sounds from a SOUNDS(...) argument.
   * This function is normally called automatically by the parser.
   */
  bool LoadSounds (char* buf);


  /**
   * Load all the plugin descriptions from the map file
   * (the plugins are not actually loaded yet).
   */
  bool LoadPlugins (char* buf);

  /**
   * Load a library into given engine.<p>
   * A library is just a map file that contains just mesh factories,
   * thing templates, sounds and textures.
   */
  bool LoadLibrary (char* buf);

  /// Load map from a memory buffer
  bool LoadMap (char* buf);

  //========================================================================
  // New XML versions of all functions accepting char*. Soon these
  // will be the only ones remaining.
  //========================================================================

  /// Parse a quaternion definition
  bool ParseQuaternion (iDocumentNode* node, csQuaternion &q);
  /// Parse a color definition
  bool ParseColor (iDocumentNode* node, csRGBcolor &c);
  /// Parse a color definition
  bool ParseColor (iDocumentNode* node, csColor &c);

  /// Parse a list of textures and add them to the engine.
  bool ParseTextureList (iDocumentNode* node);
  /**
   * Parse a list of materials and add them to the engine. If a prefix is
   * given, all material names will be prefixed with the corresponding string.
   */
  bool ParseMaterialList (iDocumentNode* node, const char* prefix = NULL);
  /// Parse a texture definition and add the texture to the engine
  iTextureWrapper* ParseTexture (iDocumentNode* node);
  /// Parse a proc texture definition and add the texture to the engine
  iTextureWrapper* ParseProcTex (iDocumentNode* node);
  /// Parse a material definition and add the material to the engine
  iMaterialWrapper* ParseMaterial (iDocumentNode* node, const char* prefix = NULL);
  /// Parse a collection definition and add the collection to the engine
  iCollection* ParseCollection (iDocumentNode* node);
  /// Parse a camera position.
  bool ParseStart (iDocumentNode* node, iCameraPosition* campos);
  /// Parse a static light definition and add the light to the engine
  iStatLight* ParseStatlight (iDocumentNode* node);
  /// Parse a key definition and add the key to the given object
  iKeyValuePair* ParseKey (iDocumentNode* node, iObject* pParent);
  /// Parse a map node definition and add the node to the given sector
  iMapNode* ParseNode (iDocumentNode* node, iSector* sec);
  /// Parse a sector definition and add the sector to the engine
  iSector* ParseSector (iDocumentNode* node);

  /// -----------------------------------------------------------------------

  /// For heightgen.
  csGenerateImageTexture* ParseHeightgenTexture (iDocumentNode* node);
  /// For heightgen.
  csGenerateImageValue* ParseHeightgenValue (iDocumentNode* node);
  /// Parse and load a height texture
  bool ParseHeightgen (iDocumentNode* node);

  /**
   * Load a LOD control object.
   */
  bool LoadLodControl (iLODControl* lodctrl, iDocumentNode* node);

  /**
   * Load a Mesh Object Factory from the map file.
   * If the transformation pointer is given then this is for a hierarchical
   * mesh object factory and the transformation will be filled in with
   * the relative transform (from MOVE keyword).
   */
  bool LoadMeshObjectFactory (iMeshFactoryWrapper* meshFact, iDocumentNode* node,
  	csReversibleTransform* transf = NULL);

  /**
   * Load the mesh object from the map file.
   */
  bool LoadMeshObject (iMeshWrapper* mesh, iDocumentNode* node);

  /**
   * Load the mesh object from the map file.
   * This version will parse FACTORY statement to directly create
   * a mesh from a factory.
   */
  iMeshWrapper* LoadMeshObjectFromFactory (iDocumentNode* node);

  /**
   * Load a plugin in general.
   */
  bool LoadAddOn (iDocumentNode* node, iBase* context);

  /**
   * Load the render priority section.
   */
  bool LoadRenderPriorities (iDocumentNode* node);

  /**
   * Load the settings section.
   */
  bool LoadSettings (iDocumentNode* node);

  /**
   * Load sounds from a SOUNDS(...) argument.
   * This function is normally called automatically by the parser.
   */
  bool LoadSounds (iDocumentNode* node);


  /**
   * Load all the plugin descriptions from the map file
   * (the plugins are not actually loaded yet).
   */
  bool LoadPlugins (iDocumentNode* node);

  /**
   * Load a library into given engine.<p>
   * A library is just a map file that contains just mesh factories,
   * thing templates, sounds and textures.
   */
  bool LoadLibrary (iDocumentNode* node);

  /// Load map from a memory buffer
  bool LoadMap (iDocumentNode* node);

  //========================================================================
  //========================================================================

  /**
   * Print an error about an unknown token. 'object' is the type of object
   * that was just being parsed, e.g. "a sector". This function will get
   * the unknown token from csGetLastOffender ().
   */
  void TokenError (const char *Object);

  /**
   * Print an error about an unknown token. 'object' is the type of object
   * that was just being parsed, e.g. "a sector". This function will get
   * the unknown token from 'Token'.
   */
  void TokenError (const char *Object, const char* Token);

  /// Report any error.
  void ReportError (const char* id, const char* description, ...);
  /// Report a notification.
  void ReportNotify (const char* description, ...);

public:
  /********** iLoader implementation **********/
  SCF_DECLARE_IBASE;

  // system driver
  iObjectRegistry* object_reg;
  iPluginManager* plugin_mgr;
  // virtual file system
  iVFS *VFS;
  // The error reporter
  iReporter* Reporter;
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
  // model converter
  iModelConverter *ModelConverter;
  // crossbuilder
  iCrossBuilder *CrossBuilder;

  // constructor
  csLoader(iBase *p);
  // destructor
  virtual ~csLoader();
  // initialize the plug-in
  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual void SetMode (int iFlags);

  virtual iImage *LoadImage (const char *fname, int Format);
  virtual iTextureHandle *LoadTexture (const char* fname,
	int Flags = CS_TEXTURE_3D, iTextureManager *tm = NULL);
  virtual iTextureWrapper *LoadTexture (const char *name, const char *fname,
	int Flags = CS_TEXTURE_3D, iTextureManager *tm = NULL,
	bool reg = false);

  virtual iSoundData *LoadSoundData (const char *fname);
  virtual iSoundHandle *LoadSound (const char *fname);
  virtual iSoundWrapper *LoadSound (const char *name, const char *fname);

  virtual bool LoadMapFile (const char* filename, bool clearEngine,
	bool onlyRegion);
  virtual bool LoadLibraryFile (const char* filename);

  virtual iMeshFactoryWrapper* LoadMeshObjectFactory (const char* fname);
  virtual iMeshWrapper* LoadMeshObject (const char* fname);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // __CS_CSLOADER_H__
