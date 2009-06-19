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

#include "csgeom/quaternion.h"
#include "csutil/array.h"
#include "csutil/csstring.h"
#include "csutil/cscolor.h"
#include "csutil/hash.h"
#include "csutil/refarr.h"
#include "csutil/weakrefarr.h"
#include "csutil/scf_implementation.h"
#include "csutil/threading/thread.h"
#include "csutil/strhash.h"
#include "csutil/util.h"
#include "imap/ldrctxt.h"
#include "imap/loader.h"
#include "imap/services.h"
#include "isndsys/ss_renderer.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/plugin.h"
#include "ivaria/engseq.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

#include "ldrplug.h"
#include "proxyimage.h"

class csReversibleTransform;
class csColor;
struct csRGBcolor;

struct iImageIO;
struct iCollection;
struct iObjectModel;
struct iSndSysLoader;
struct iSndSysManager;
struct iEngine;
struct iVFS;
struct iGraphics3D;
struct iLoaderPlugin;
struct iBinaryLoaderPlugin;
struct iObjectRegistry;
struct iPluginManager;
struct iCameraPosition;
struct iDocumentNode;
struct iDocument;
struct iFile;
struct iTriangleMesh;
struct iShaderManager;
struct iMeshGenerator;
struct iSceneNode;
struct iRenderLoop;
struct iImposter;

struct iObject;
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
struct iSceneNodeArray;

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{

class csLoader;
struct csLoaderPluginRec;

/*
 * Context class for the standard loader.
 */
class StdLoaderContext : public scfImplementation1<StdLoaderContext,
			 iLoaderContext>
{
private:
  iEngine* Engine;
  iCollection* collection;
  bool searchCollectionOnly;
  csLoader* loader;
  csRef<iMissingLoaderData> missingdata;
  bool checkDupes;
  uint keepFlags;

public:
  StdLoaderContext(iEngine* Engine, iCollection* collection, bool searchCollectionOnly,
    csLoader* loader, bool checkDupes, iMissingLoaderData* missingdata, uint keepFlags);
  virtual ~StdLoaderContext ();

  virtual iSector* FindSector (const char* name);
  virtual iMaterialWrapper* FindMaterial (const char* name);
  virtual iMaterialWrapper* FindNamedMaterial (const char* name,
      const char *filename);
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name);
  virtual iMeshWrapper* FindMeshObject (const char* name);
  virtual iTextureWrapper* FindTexture (const char* name);
  virtual iTextureWrapper* FindNamedTexture (const char* name,
      const char *filename);
  virtual iLight* FindLight (const char *name);
  virtual iShader* FindShader (const char *name);
  virtual bool CheckDupes () const { return checkDupes; }
  virtual iCollection* GetCollection() const { return collection; }
  virtual bool CurrentCollectionOnly() const { return searchCollectionOnly; }
  virtual uint GetKeepFlags() const { return keepFlags; }
  virtual void AddToCollection(iObject* obj);
  virtual bool GetVerbose() { return false; }
};



#include "csutil/deprecated_warn_off.h"

/**
 * The loader for Crystal Space maps.
 */
class csLoader : public scfImplementation2<csLoader,
                                           iLoader,
                                           iComponent>
{
  friend class StdLoaderContext;
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
  csRef<iShaderVarStringSet> stringSetSvName;

  /// Pointer to built-in image texture loader.
  csRef<iLoaderPlugin> BuiltinImageTexLoader;
  /// Pointer to built-in checkerboard texture loader.
  //csRef<iLoaderPlugin> BuiltinCheckerTexLoader;
  csRef<iLoaderPlugin> BuiltinErrorTexLoader;

  /// Auto regions flag
  bool autoRegions;

  struct ProxyKeyColour
  {
    bool do_transp;
    csColor colours;
  };

  struct ProxyTexture
  {
    csWeakRef<iTextureWrapper> textureWrapper;
    csRef<ProxyImage> img;
    csAlphaMode::AlphaType alphaType;
    bool always_animate;
    ProxyKeyColour keyColour;
  };

  /// Points to proxy textures ready for processing.
  csSafeCopyArray<ProxyTexture> proxyTextures;

  /// Points to materials created by the current map loading.
  csWeakRefArray<iMaterialWrapper> materialArray;
  
  //Returns in the 'meshesArray' array all the meshes encountered walking thru
  //the hierarchy of meshes starting from 'meshWrapper'.
  void CollectAllChildren (iMeshWrapper* meshWrapper, csRefArray<iMeshWrapper>&
    meshesArray);
  //Two useful private functions to set the CS_TRIMESH_CLOSED and
  //CS_TRIMESH_CONVEX flags on a single mesh wrapper.
  void ConvexFlags (iMeshWrapper* mesh);
  void ClosedFlags (iMeshWrapper* mesh);

  /// List of loaded plugins
  csLoadedPluginVector loaded_plugins;

  //------------------------------------------------------------------------

  /**
   * Parse a key/value pair.
   * Takes "editoronly" attribute into account: KVPs should only be parsed 
   * if they're not editor-only or when the engine is in "saveable" mode.
   */
  bool ParseKey (iDocumentNode* node, iObject* obj);
/*
          iKeyValuePair* kvp = 0;
          SyntaxService->ParseKey (child, kvp);
          if (kvp)
          {
            Engine->QueryObject()->ObjAdd (kvp->QueryObject ());
	    kvp->DecRef ();
          }
	  else
	    return false;
*/

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
  bool ParseImposterSettings(iImposter* mesh, iDocumentNode *node);

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
  /// Parse a renderloop.
  iRenderLoop* ParseRenderLoop (iDocumentNode* node, bool& set);
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
  /// Parse a 'trimesh' block.
  bool ParseTriMesh (iDocumentNode* node, iObjectModel* objmodel);
  bool ParseTriMeshChildBox (iDocumentNode* child,
	csRef<iTriangleMesh>& trimesh);
  bool ParseTriMeshChildMesh (iDocumentNode* child,
	csRef<iTriangleMesh>& trimesh);

  /// -----------------------------------------------------------------------
  /// Parse a shaderlist
  bool LoadShaderExpressions (iLoaderContext* ldr_context,
  	iDocumentNode* node);
  bool ParseShaderList (iLoaderContext* ldr_context, iDocumentNode* node);
  bool ParseShader (iLoaderContext* ldr_context, iDocumentNode* node,
    iShaderManager* shaderMgr);
  virtual csRef<iShader> LoadShader (const char* filename,
    bool registerShader = true);

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
	csStringID id, bool& handled, csString& priority,
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
   * Load the trimesh object from the map file.
   */
  bool LoadTriMeshInSector (iLoaderContext* ldr_context,
  	iMeshWrapper* mesh, iDocumentNode* node, iStreamSource* ssource);

  /**
   * Load the mesh object from the map file.
   * This version will parse FACTORY statement to directly create
   * a mesh from a factory.
   */
  csRef<iMeshWrapper> LoadMeshObjectFromFactory (iLoaderContext* ldr_context,
  	iDocumentNode* node, iStreamSource* ssource);

  /**
   * Load a mesh generator geometry.
   */
  bool LoadMeshGenGeometry (iLoaderContext* ldr_context,
  	iDocumentNode* node, iMeshGenerator* meshgen);

  /**
   * Load a mesh generator.
   */
  bool LoadMeshGen (iLoaderContext* ldr_context,
  	iDocumentNode* node, iSector* sector);

  /**
   * Load a plugin in general.
   */
  bool LoadAddOn (iLoaderContext* ldr_context,
  	iDocumentNode* node, iBase* context, bool is_meta,
	iStreamSource* ssource);

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
  	iStreamSource* ssource, iMissingLoaderData* missingdata,
	bool loadProxyTex = true);

  /// Load map from a memory buffer
  bool LoadMap (iLoaderContext* ldr_context, iDocumentNode* world_node,
  	iStreamSource* ssource, iMissingLoaderData* missingdata);

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
   * Try loading file as a structured document via iDocumentSystem.
   * \return False on failure.
   */
  bool LoadStructuredDoc (const char* file, iDataBuffer* buf,
  	csRef<iDocument>& doc);

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
   * Add the given object to the collection in the context (if there is
   * such a collection).
   */
  void AddToCollection (iLoaderContext* ldr_context, iObject* obj);

  /**
   * Add children to the collection.
   */
  void AddChildrenToCollection (iLoaderContext* ldr_context,
    const iSceneNodeArray* children);

public:
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

private:
  // Load all proxy textures which are used.
  bool LoadProxyTextures();

public:
  /********** iLoader implementation **********/
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
  csRef<iSndSysLoader> SndSysLoader;
  // sound manager
  csRef<iSndSysManager> SndSysManager;
  // sound renderer
  csRef<iSndSysRenderer> SndSysRender;
  // engine
  csRef<iEngine> Engine;
  // graphics renderer
  csRef<iGraphics3D> G3D;

  // constructor
  csLoader(iBase *p);
  // destructor
  virtual ~csLoader();
  // initialize the plug-in
  virtual bool Initialize(iObjectRegistry *object_reg);

  /////////////////////////// Generic ///////////////////////////

  virtual csPtr<iImage> LoadImage (iDataBuffer* buf, int Format);
  virtual csPtr<iImage> LoadImage (const char *fname, int Format);

  virtual csPtr<iTextureHandle> LoadTexture (iDataBuffer* buf,
      int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
      csRef<iImage>* img=0);

  virtual iTextureWrapper* LoadTexture (const char *name, iDataBuffer* buf,
      int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0, bool reg = true,
      bool create_material = true, bool free_image = true);

  virtual csPtr<iTextureHandle> LoadTexture (const char* fname,
    int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
    csRef<iImage>* img=0);

  virtual csPtr<iSndSysData> LoadSoundSysData (const char *fname);
  virtual csPtr<iSndSysStream> LoadSoundStream (const char *fname, int mode3d);
  virtual iSndSysWrapper* LoadSoundWrapper (const char *name, const char *fname);

  bool LoadLibraryFromNode (iLoaderContext* ldr_context, iDocumentNode* child,
    iStreamSource* ssource, iMissingLoaderData* missingdata, bool loadProxyTex = true);

  virtual csPtr<iMeshFactoryWrapper> LoadMeshObjectFactory (const char* fname,
    iStreamSource* ssource);

  virtual csPtr<iMeshWrapper> LoadMeshObject (const char* fname,
    iStreamSource* ssource);

  /////////////////////////// Collections ///////////////////////////

  virtual iTextureWrapper* LoadTexture (const char *Name,
    const char *FileName, int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
    bool reg = true, bool create_material = true, bool free_image = true,
    iCollection* collection = 0, uint keepFlags = KEEP_ALL);

  virtual bool LoadMapFile (const char* filename, bool clearEngine = true,
    iCollection* collection = 0, bool searchCollectionOnly = true, bool checkDupes = false,
    iStreamSource* ssource = 0, iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL);

  virtual bool LoadMap (iDocumentNode* world_node, bool clearEngine = true,
    iCollection* collection = 0, bool searchCollectionOnly = true, bool checkDupes = false,
    iStreamSource* ssource = 0, iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL);

  virtual bool LoadMapLibraryFile (const char* filename, iCollection* collection,
      bool searchCollectionOnly, bool checkDupes, iStreamSource* ssource,
      iMissingLoaderData* missingdata, uint keepFlags = KEEP_ALL,
      bool loadProxyTex = true);

  virtual bool LoadLibraryFile (const char* filename, iCollection* collection = 0,
    bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL);

  virtual bool LoadLibrary (iDocumentNode* lib_node, iCollection* collection = 0,
    bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL);

  csLoadResult Load (iDataBuffer* buffer, const char* fname,
      iCollection* collection, bool searchCollectionOnly, bool checkDupes,
      iStreamSource* ssource, const char* override_name,
      iMissingLoaderData* missingdata, uint keepFlags = KEEP_ALL);

  virtual csLoadResult Load (const char* fname, iCollection* collection = 0,
    bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0,
    uint keepFlags = KEEP_ALL);

  virtual csLoadResult Load (iDataBuffer* buffer, iCollection* collection = 0,
    bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0,
    uint keepFlags = KEEP_ALL);

  virtual csLoadResult Load (iDocumentNode* node, iCollection* collection = 0,
    bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL);

  ///

  virtual void SetAutoRegions (bool autoRegions)
  {
      csLoader::autoRegions = autoRegions;
  }

  virtual bool GetAutoRegions ()
  {
      return autoRegions;
  }
};

}
CS_PLUGIN_NAMESPACE_END(csparser)

#include "csutil/deprecated_warn_on.h"

#endif // __CS_CSLOADER_H__
