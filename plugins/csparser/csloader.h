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

#include <stdarg.h>
#include "ivideo/graph3d.h"
#include "imap/loader.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/util.h"
#include "csutil/strhash.h"
#include "csutil/hash.h"
#include "csutil/array.h"
#include "csutil/refarr.h"
#include "csutil/scopedmutexlock.h"
#include "csgeom/quaterni.h"
#include "iutil/plugin.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"
#include "ivaria/engseq.h"

class csGenerateImageTexture;
class csGenerateImageValue;
class csReversibleTransform;
class csColor;
struct csRGBcolor;

struct iImageIO;
struct iRegion;
struct iObjectModel;
struct iSoundLoader;
struct iSndSysLoader;
struct iEngine;
struct iVFS;
struct iGraphics3D;
struct iSoundRender;
struct iLoaderPlugin;
struct iBinaryLoaderPlugin;
struct iObjectRegistry;
struct iPluginManager;
struct iModelConverter;
struct iCrossBuilder;
struct iCameraPosition;
struct iDocumentNode;
struct iDocument;
struct iFile;
struct iPolygonMesh;
struct iShaderManager;

struct iObject;
struct iThingState;
struct iCollection;
struct iMaterialWrapper;
struct iMeshFactoryWrapper;
struct iMeshWrapper;
struct iSector;
struct iKeyValuePair;
struct iMapNode;
struct iReporter;
struct iLODControl;
struct iLoaderContext;
struct iSequenceTrigger;
struct iSequenceWrapper;
struct iEngineSequenceParameters;
struct iSharedVariable;

class csLoader;
struct csLoaderPluginRec;

/*
 * Context class for the standard loader.
 */
class StdLoaderContext : public iLoaderContext
{
private:
  iEngine* Engine;
  iRegion* region;
  csLoader* loader;
  bool checkDupes;
  bool curRegOnly;
public:
  StdLoaderContext (iEngine* Engine, iRegion* region, bool curRegOnly,
    csLoader* loader, bool checkDupes);
  virtual ~StdLoaderContext ();

  SCF_DECLARE_IBASE;

  virtual iSector* FindSector (const char* name);
  virtual iMaterialWrapper* FindMaterial (const char* name);
  virtual iMaterialWrapper* FindNamedMaterial (const char* name, const char *filename);
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name);
  virtual iMeshWrapper* FindMeshObject (const char* name);
  virtual iTextureWrapper* FindTexture (const char* name);
  virtual iTextureWrapper* FindNamedTexture (const char* name, const char *filename);
  virtual iLight* FindLight (const char *name);
  virtual iShader* FindShader (const char *name);
  virtual bool CheckDupes () const { return checkDupes; }
  virtual iRegion* GetRegion () const { return region; }
  virtual bool CurrentRegionOnly () const { return curRegOnly; }
};

/*
 * Context class for the threaded loader.
 */
class ThreadedLoaderContext : public iLoaderContext
{
private:
  iEngine* Engine;
  iRegion* region;
  csLoader* loader;
  bool checkDupes;
  bool curRegOnly;
  csRefArray<iSector> sectors;
  csRefArray<iMaterialWrapper> materials;
  csRefArray<iTextureWrapper> textures;
  csRefArray<iMeshFactoryWrapper> factories;
  csRefArray<iMeshWrapper> meshes;
  csRefArray<iLight> lights;

public:
  ThreadedLoaderContext (iEngine* Engine, iRegion* region, bool curRegOnly,
    csLoader* loader, bool checkDupes);
  virtual ~ThreadedLoaderContext ();

  SCF_DECLARE_IBASE;

  virtual iSector* FindSector (const char* name);
  virtual iMaterialWrapper* FindMaterial (const char* name);
  virtual iMaterialWrapper* FindNamedMaterial (const char* name, const char *filename);
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name);
  virtual iMeshWrapper* FindMeshObject (const char* name);
  virtual iTextureWrapper* FindTexture (const char* name);
  virtual iTextureWrapper* FindNamedTexture (const char* name, const char *filename);
  virtual iLight* FindLight (const char *name);
  virtual iShader* FindShader (const char *name);
  virtual bool CheckDupes () const { return checkDupes; }
  virtual iRegion* GetRegion () const { return region; }
  virtual bool CurrentRegionOnly () const { return curRegOnly; }
};

/**
 * Status class for the threaded loader.
 */
class csLoaderStatus : public iLoaderStatus
{
private:
  //csRef<csMutex> mutex;
  bool ready;
  bool error;

public:
  csLoaderStatus ();
  virtual ~csLoaderStatus ();

  SCF_DECLARE_IBASE;
  virtual bool IsReady () { return ready; }
  virtual bool IsError () { return error; }
};

/**
 * The loader for Crystal Space maps.
 */
class csLoader : public iLoader
{
  friend class StdLoaderContext;
  friend class ThreadedLoaderContext;
private:
  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "plugins/csparser/csloader.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE 

  /// Parser for common stuff like MixModes, vectors, matrices, ...
  csRef<iSyntaxService> SyntaxService;
  /// Pointer to the engine sequencer (optional module).
  csRef<iEngineSequenceManager> eseqmgr;
  /// Shared string set
  csRef<iStringSet> stringSet;

  /// Pointer to built-in image texture loader.
  csRef<iLoaderPlugin> BuiltinImageTexLoader;
  /// Pointer to built-in checkerboard texture loader.
  csRef<iLoaderPlugin> BuiltinCheckerTexLoader;

  //Returns in the 'meshesArray' array all the meshes encountered walking thru
  //the hierarchy of meshes starting from 'meshWrapper'.
  void CollectAllChildren (iMeshWrapper* meshWrapper, csRefArray<iMeshWrapper>&
    meshesArray);
  //Two useful private functions to set the CS_POLYMESH_CLOSED and
  //CS_POLYMESH_CONVEX flags on a single mesh wrapper.
  void ConvexFlags (iMeshWrapper* mesh);
  void ClosedFlags (iMeshWrapper* mesh);

  class csLoadedPluginVector
  {
  private:
    /// Mutex to make the plugin vector thread-safe.
    csRef<csMutex> mutex;
    iObjectRegistry* object_reg;

    csArray<csLoaderPluginRec*> vector;

    // Find a loader plugin record
    struct csLoaderPluginRec* FindPluginRec (const char* name);
    // Return the loader plugin from a record, possibly loading the plugin now
    bool GetPluginFromRec (csLoaderPluginRec*,
    	iLoaderPlugin*& plug, iBinaryLoaderPlugin*& binplug);
  public:
    iPluginManager* plugin_mgr;

    // constructor
    csLoadedPluginVector ();
    // destructor
    ~csLoadedPluginVector ();
    /**
     * Find a plugin by its name or load it if it doesn't exist.
     * Supports both binary and normal plugins. Returns 'false' if the
     * plugin doesn't exist.
     */
    bool FindPlugin (const char* Name, iLoaderPlugin*& plug,
    	iBinaryLoaderPlugin*& binplug, iDocumentNode*& defaults);
    // add a new plugin record
    void NewPlugin (const char* ShortName, iDocumentNode* child);
    /**
     * Delete all loaded plugins.
     */
    void DeleteAll ();

    void SetObjectRegistry (iObjectRegistry* object_reg)
    {
      csLoadedPluginVector::object_reg = object_reg;
    }
  };

  /// List of loaded plugins
  csLoadedPluginVector loaded_plugins;

  //------------------------------------------------------------------------

  /// Parse a quaternion definition
  bool ParseQuaternion (iDocumentNode* node, csQuaternion &q);
  /// Parse a color definition
  bool ParseColor (iDocumentNode* node, csRGBcolor &c);
  /// Parse a color definition
  bool ParseColor (iDocumentNode* node, csColor &c);

  /// Load a trigger.
  iSequenceTrigger* LoadTrigger (iLoaderContext* ldr_context,
  	iDocumentNode* node);
  /// Load a list of triggers.
  bool LoadTriggers (iLoaderContext* ldr_context, iDocumentNode* node);
  /// Create a sequence and make parameter bindings.
  iSequenceWrapper* CreateSequence (iDocumentNode* node);
  /// Load a sequence.
  iSequenceWrapper* LoadSequence (iLoaderContext* ldr_context,
  	iDocumentNode* node);
  /// Load a list of sequences.
  bool LoadSequences (iLoaderContext* ldr_context, iDocumentNode* node);
  /// Parse a parameter block for firing a sequence.
  csPtr<iEngineSequenceParameters> CreateSequenceParameters (
  	iLoaderContext* ldr_context,
	iSequenceWrapper* sequence, iDocumentNode* node,
	const char* parenttype, const char* parentname, bool& error);
  /// Resolve a parameter for a sequence operation.
  csPtr<iParameterESM> ResolveOperationParameter (
  	iLoaderContext* ldr_context, iDocumentNode* opnode,
	int partypeidx, const char* partype, const char* seqname,
	iEngineSequenceParameters* base_params);

  /// Parse a list of textures and add them to the engine.
  bool ParseTextureList (iLoaderContext* ldr_context, iDocumentNode* node);
  /**
   * Parse a list of materials and add them to the engine. If a prefix is
   * given, all material names will be prefixed with the corresponding string.
   */
  bool ParseMaterialList (iLoaderContext* ldr_context,
  	iDocumentNode* node, const char* prefix = 0);
  /// Parse a list of shared variables and add them each to the engine
  bool ParseVariableList (iLoaderContext* ldr_context, iDocumentNode* node);
  /// Process the attributes of one shared variable
  bool ParseSharedVariable (iLoaderContext* ldr_context, iDocumentNode* node);
  /// Process the attributes of an <imposter> tag in a mesh specification.
  bool ParseImposterSettings(iMeshWrapper* mesh,iDocumentNode *node);

  /// Parse a texture definition and add the texture to the engine
  iTextureWrapper* ParseTexture (iLoaderContext* ldr_context,
  	iDocumentNode* node);

  /// Parse a Cubemap texture definition and add the texture to the engine
  iTextureWrapper* ParseCubemap (iLoaderContext* ldr_context,
    iDocumentNode* node);

  /// Parse a 3D Texture definition and add the texture to the engine
  iTextureWrapper* ParseTexture3D (iLoaderContext* ldr_context,
    iDocumentNode* node);

  /// Parse a proc texture definition and add the texture to the engine
  //iTextureWrapper* ParseProcTex (iDocumentNode* node);
  /// Parse a material definition and add the material to the engine
  iMaterialWrapper* ParseMaterial (iLoaderContext* ldr_context,
  	iDocumentNode* node, const char* prefix = 0);
  /// Parse a collection definition and add the collection to the engine
  iCollection* ParseCollection (iLoaderContext* ldr_context,
  	iDocumentNode* node);
  /// Parse a camera position.
  bool ParseStart (iDocumentNode* node, iCameraPosition* campos);
  /// Parse a static light definition and add the light to the engine
  iLight* ParseStatlight (iLoaderContext* ldr_context, iDocumentNode* node);
  /// Parse a map node definition and add the node to the given sector
  iMapNode* ParseNode (iDocumentNode* node, iSector* sec);
  /**
   * Parse a portal definition. 'container_name' is the name of the portal
   * container to use. If 0 then the name of the portal itself will be
   * used instead.
   */
  bool ParsePortal (iLoaderContext* ldr_context,
	iDocumentNode* node, iSector* sourceSector, const char* container_name,
	iMeshWrapper*& container_mesh, iMeshWrapper* parent);
  /// Parse a portals definition.
  bool ParsePortals (iLoaderContext* ldr_context,
	iDocumentNode* node, iSector* sourceSector,
	iMeshWrapper* parent, iStreamSource* ssource);
  /// Parse a sector definition and add the sector to the engine
  iSector* ParseSector (iLoaderContext* ldr_context, iDocumentNode* node,
  	iStreamSource* ssource);
  /// Find the named shared variable and verify its type if specified
  iSharedVariable *FindSharedVariable(const char *colvar,
				      int verify_type );
  /// Parse a 'polymesh' block.
  bool ParsePolyMesh (iDocumentNode* node, iObjectModel* objmodel);
  bool ParsePolyMeshChildBox (iDocumentNode* child,
	csRef<iPolygonMesh>& polymesh);
  bool ParsePolyMeshChildMesh (iDocumentNode* child,
	csRef<iPolygonMesh>& polymesh);

  /// -----------------------------------------------------------------------
  /// Parse a shaderlist
  bool ParseShaderList (iLoaderContext* ldr_context, iDocumentNode* node);
  bool ParseShader (iLoaderContext* ldr_context, iDocumentNode* node,
    iShaderManager* shaderMgr);

  /// For heightgen.
  csGenerateImageTexture* ParseHeightgenTexture (iDocumentNode* node);
  /// For heightgen.
  csGenerateImageValue* ParseHeightgenValue (iDocumentNode* node);
  /// Parse and load a height texture
  bool ParseHeightgen (iLoaderContext* ldr_context, iDocumentNode* node);

  /**
   * Load a LOD control object.
   */
  bool LoadLodControl (iLODControl* lodctrl, iDocumentNode* node);

  /**
   * Load a Mesh Object Factory from the map file.
   * If the transformation pointer is given then this is for a hierarchical
   * mesh object factory and the transformation will be filled in with
   * the relative transform (from MOVE keyword).
   * parent is not 0 if the factory is part of a hierarchical factory.
   */
  bool LoadMeshObjectFactory (
  	iLoaderContext* ldr_context, iMeshFactoryWrapper* meshFact,
	iMeshFactoryWrapper* parent,
  	iDocumentNode* node, csReversibleTransform* transf = 0,
	iStreamSource* ssource = 0);

  /**
   * Handle various common mesh object parameters.
   */
  bool HandleMeshParameter (iLoaderContext* ldr_context,
  	iMeshWrapper* mesh, iMeshWrapper* parent, iDocumentNode* child,
	csStringID id, bool& handled, char*& priority,
	bool do_portal_container, bool& staticpos, bool& staticshape,
	bool& zmodeChanged, bool& prioChanged,
	bool recursive, iStreamSource* ssource);
  /**
   * Load the mesh object from the map file.
   * The parent is not 0 if this mesh is going to be part of a hierarchical
   * mesh.
   */
  bool LoadMeshObject (iLoaderContext* ldr_context,
  	iMeshWrapper* mesh, iMeshWrapper* parent, iDocumentNode* node,
	iStreamSource* ssource);
  /**
   * Load the polymesh object from the map file.
   */
  bool LoadPolyMeshInSector (iLoaderContext* ldr_context,
  	iMeshWrapper* mesh, iDocumentNode* node, iStreamSource* ssource);

  /**
   * Load the mesh object from the map file.
   * This version will parse FACTORY statement to directly create
   * a mesh from a factory.
   */
  iMeshWrapper* LoadMeshObjectFromFactory (iLoaderContext* ldr_context,
  	iDocumentNode* node, iStreamSource* ssource);

  /**
   * Load a plugin in general.
   */
  bool LoadAddOn (iLoaderContext* ldr_context,
  	iDocumentNode* node, iBase* context, bool is_meta,
	iStreamSource* ssource);

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
  bool LoadLibrary (iLoaderContext* ldr_context, iDocumentNode* node,
  	iStreamSource* ssource);

  /// Load map from a memory buffer
  bool LoadMap (iLoaderContext* ldr_context, iDocumentNode* world_node,
  	iStreamSource* ssource);

  /// Get the engine sequence manager (load it if not already present).
  iEngineSequenceManager* GetEngineSequenceManager ();

  //========================================================================
  //========================================================================

  /**
   * Try loading file as a structured document via iDocumentSystem.
   * \return False on failure.
   */
  bool LoadStructuredDoc (const char* file, iFile* buf, csRef<iDocument>& doc);

  /**
   * Try loading the file as a structured document.
   * \return True if the documented loaded and appears to be a map file,
   *   otherwise false.
   */
  csPtr<iBase> LoadStructuredMap (iLoaderContext* ldr_context,
  	iLoaderPlugin* plug, iFile* buf,
  	iBase* context, const char* fname, iStreamSource* ssource);

  /**
   * Handle the result of a mesh object plugin loader.
   */
  bool HandleMeshObjectPluginResult (iBase* mo, iDocumentNode* child,
	iMeshWrapper* mesh, bool keepZbuf, bool keepPrio);

  /**
   * Add the given object to the region in the context (if there is
   * such a region).
   */
  void AddToRegion (iLoaderContext* ldr_context, iObject* obj);

  /// Report any error.
  void ReportError (const char* id, const char* description, ...)
	CS_GNUC_PRINTF(3,4);
  /// Report a notification.
  void ReportNotify (const char* description, ...)
	CS_GNUC_PRINTF(2,3);
  /// Report a warning.
  void ReportWarning (const char* id, const char* description, ...)
	CS_GNUC_PRINTF(3,4);
  /// Report a notification.
  void ReportNotifyV (const char* id, const char* description, va_list arg)
	CS_GNUC_PRINTF(3,0);
  /// Report a notification.
  void ReportNotify2 (const char* id, const char* description, ...)
	CS_GNUC_PRINTF(3,4);

  /// Report a warning.
  void ReportWarning (const char* id, iDocumentNode* node,
  	const char* description, ...)
	CS_GNUC_PRINTF(4,5);

  csPtr<iImage> LoadImage (iDataBuffer* buf, const char* fname, int Format);

public:
  /********** iLoader implementation **********/
  SCF_DECLARE_IBASE;

  static bool do_verbose;

  // system driver
  iObjectRegistry* object_reg;
  // virtual file system
  csRef<iVFS> VFS;
  // The error reporter
  csRef<iReporter> Reporter;
  // image loader
  csRef<iImageIO> ImageLoader;
  // sound loader
  csRef<iSoundLoader> SoundLoader;
  // sound loader
  csRef<iSndSysLoader> SndSysLoader;
  // engine
  csRef<iEngine> Engine;
  // graphics renderer
  csRef<iGraphics3D> G3D;
  // sound renderer
  csRef<iSoundRender> SoundRender;
  // model converter
  csRef<iModelConverter> ModelConverter;
  // crossbuilder
  csRef<iCrossBuilder> CrossBuilder;

  // constructor
  csLoader(iBase *p);
  // destructor
  virtual ~csLoader();
  // initialize the plug-in
  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual csPtr<iImage> LoadImage (iDataBuffer* buf, int Format);
  virtual csPtr<iTextureHandle> LoadTexture (iDataBuffer* buf,
	int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
	csRef<iImage>* img=0);
  virtual iTextureWrapper* LoadTexture (const char *name,
  	iDataBuffer* buf,
	int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
	bool reg = false, bool create_material = true);

  virtual csPtr<iImage> LoadImage (const char *fname, int Format);
  virtual csPtr<iTextureHandle> LoadTexture (const char* fname,
	int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
	csRef<iImage>* img=0);
  virtual iTextureWrapper* LoadTexture (const char *name,
  	const char *fname,
	int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
	bool reg = false, bool create_material = true);

  virtual csPtr<iSoundData> LoadSoundData (const char *fname);
  virtual csPtr<iSoundHandle> LoadSound (const char *fname);
  virtual csPtr<iSoundWrapper> LoadSound (const char *name, const char *fname);

  virtual csPtr<iLoaderStatus> ThreadedLoadMapFile (const char* filename,
	iRegion* region, bool curRegOnly, bool checkDupes);
  virtual bool LoadMapFile (const char* filename, bool clearEngine,
	iRegion* region, bool curRegOnly, bool checkDupes,
	iStreamSource* ssource);
  virtual bool LoadMap (iDocumentNode* world_node, bool clearEngine,
	iRegion* region, bool curRegOnly, bool checkDupes,
	iStreamSource* ssource);
  virtual bool LoadLibraryFile (const char* filename, iRegion* region,
  	bool curRegOnly, bool checkDupes,
	iStreamSource* ssource);
  virtual bool LoadLibrary (iDocumentNode* lib_node, iRegion* region,
  	bool curRegOnly, bool checkDupes,
	iStreamSource* ssource);
  bool LoadLibraryFromNode (iLoaderContext* ldr_context,
	iDocumentNode* child,
	iStreamSource* ssource);

  virtual bool Load (const char* fname, iBase*& result, iRegion* region,
  	bool curRegOnly, bool checkDupes, iStreamSource* ssource);
  virtual bool Load (iDocumentNode* node, iBase*& result, iRegion* region,
  	bool curRegOnly, bool checkDupes, iStreamSource* ssource);

  virtual csPtr<iMeshFactoryWrapper> LoadMeshObjectFactory (const char* fname,
  	iStreamSource* ssource);
  virtual csPtr<iMeshWrapper> LoadMeshObject (const char* fname,
  	iStreamSource* ssource);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csLoader);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

#endif // __CS_CSLOADER_H__
