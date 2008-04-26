/*
    The Crystal Space geometry loader interface
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_IMAP_PARSER_H__
#define __CS_IMAP_PARSER_H__

/**\file 
 * Geometry loader interface
 */
/**\addtogroup loadsave
 * @{ */
#include "csutil/scf_interface.h"

#include "iengine/region.h"
#include "igraphic/image.h"
#include "ivideo/txtmgr.h"
#include "imap/streamsource.h"

#include "csutil/deprecated_warn_off.h"

struct iCollection;
struct iDocumentNode;
struct iImage;
struct iLight;
struct iMaterialWrapper;
struct iMeshWrapper;
struct iMeshFactoryWrapper;
struct iSector;
struct iShader;
struct iTextureHandle;
struct iTextureManager;
struct iTextureWrapper;
struct iSndSysData;
struct iSndSysWrapper;
struct iSndSysStream;

// The keep type flags for collections.
#define KEEP_ALL  0
#define KEEP_USED 1

/**
* This callback is called when the loader can't find some material,
* texture, factory, mesh, light, shader, or sector. This gives the
* application a chance to manually load it first.
*/
struct iMissingLoaderData : public virtual iBase
{
  SCF_INTERFACE (iMissingLoaderData, 1, 0, 0);

  /**
  * Called when a material is missing. This implementation should
  * either attempt to find or load the material or else return 0.
  * In the last case the loader will proceed as usual when a material
  * is not found.
  */
  virtual iMaterialWrapper* MissingMaterial (const char* name,
    const char* filename) = 0;

  /**
  * Called when a texture is missing. This implementation should
  * either attempt to find or load the texture or else return 0.
  * In the last case the loader will proceed as usual when a texture
  * is not found.
  */
  virtual iTextureWrapper* MissingTexture (const char* name,
    const char* filename) = 0;

  /**
  * Called when a shader is missing. This implementation should
  * either attempt to find or load the shader or else return 0.
  * In the last case the loader will proceed as usual when a shader
  * is not found.
  */
  virtual iShader* MissingShader (const char* name) = 0;

  /**
  * Called when a mesh factory is missing. This implementation should
  * either attempt to find or load the mesh factory or else return 0.
  * In the last case the loader will proceed as usual when a mesh factory
  * is not found.
  */
  virtual iMeshFactoryWrapper* MissingFactory (const char* name) = 0;

  /**
  * Called when a mesh is missing. This implementation should
  * either attempt to find or load the mesh or else return 0.
  * In the last case the loader will proceed as usual when a mesh
  * is not found.
  */
  virtual iMeshWrapper* MissingMesh (const char* name) = 0;

  /**
  * Called when a sector is missing. This implementation should
  * either attempt to find or load the sector or else return 0.
  * In the last case the loader will proceed as usual when a sector
  * is not found.
  */
  virtual iSector* MissingSector (const char* name) = 0;

  /**
  * Called when a light is missing. This implementation should
  * either attempt to find or load the light or else return 0.
  * In the last case the loader will proceed as usual when a light
  * is not found.
  */
  virtual iLight* MissingLight (const char* name) = 0;
};

/**
* Return structure for the iLoader->Load() routines.
*/
struct csLoadResult
{
  /// True if loading was succesful
  bool success;
  /**
  * The object that was loaded. Depending on the file you load this
  * can be anything like:
  * - 'world' file: in that case 'result' will be set to the engine.
  * - 'library' file: 'result' will be 0.
  * - 'meshfact' file: 'result' will be the mesh factory wrapper.
  * - 'meshobj' file: 'result' will be the mesh wrapper.
  * - 'meshref' file: 'result' will be the mesh wrapper.
  * - 'portals' file: 'result' will be the portal's mesh wrapper.
  * - 'light' file: 'result' will be the light. 
  * Note! In case of a light call DecRef() after you added it to a sector.
  * Note! Use scfQueryInterface on 'result' to detect what type was loaded.
  */
  iBase* result;
};

/**
* This interface represents the map loader.
*/
struct iLoader : public virtual iBase
{
  SCF_INTERFACE (iLoader, 4, 0, 0);

  /////////////////////////// Generic ///////////////////////////

  /**
  * Load an image file. The image will be loaded in the format requested by
  * the engine. If no engine exists, the format is taken from the video
  * renderer. If no video renderer exists, this function fails. You may also
  * request an alternate format to override the above sequence.
  */
  virtual csPtr<iImage> LoadImage (const char* Filename,
    int Format = CS_IMGFMT_INVALID) = 0;
  /**
  * Load an image as with LoadImage() and create a texture handle from it.
  * \param Filename VFS path to the image file to load.
  * \param Flags Accepts the flags described in ivideo/txtmgr.h.
  *   The texture manager determines the format, so choosing an alternate 
  *   format doesn't make sense here. Instead you may choose an alternate 
  *   texture manager.
  * \param tm Texture manager, used to determine the format the image is to
  *   be loaded in (defaults to CS_IMGFMT_TRUECOLOR if no texture manager is
  *   specified).
  * \param image Optionally returns a reference to the loaded image.
  */
  virtual csPtr<iTextureHandle> LoadTexture (const char* Filename,
    int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
    csRef<iImage>* image=0) = 0;

  /// New Sound System: Load a sound file and return an iSndSysData object
  virtual csPtr<iSndSysData> LoadSoundSysData (const char *fname) = 0;

  /**
  * New Sound System: Load a sound file and create a stream from it.
  * \param fname is the VFS filename.
  * \param mode3d is one of CS_SND3D_DISABLE, CS_SND3D_RELATIVE, or
  * CS_SND3D_ABSOLUTE.
  */
  virtual csPtr<iSndSysStream> LoadSoundStream (const char *fname,
    int mode3d) = 0;

  /**
  * New Sound System: Load a sound file, create sound data and create a
  * wrapper object for it.
  * \param name of the sound.
  * \param fname is the VFS filename.
  */
  virtual iSndSysWrapper* LoadSoundWrapper (const char *name,
    const char *fname) = 0;

  /**
  * Load an image file. The image will be loaded in the format requested by
  * the engine. If no engine exists, the format is taken from the video
  * renderer. If no video renderer exists, this function fails. You may also
  * request an alternate format to override the above sequence.
  * This version reads the image from a data buffer.
  */
  virtual csPtr<iImage> LoadImage (iDataBuffer* buf,
    int Format = CS_IMGFMT_INVALID) = 0;
  /**
  * Load an image as with LoadImage() and create a texture handle from it.
  * This version reads the image from a data buffer.
  * \param buf Buffer containing the image file data.
  * \param Flags Accepts the flags described in ivideo/txtmgr.h.
  *   The texture manager determines the format, so choosing an alternate 
  *   format doesn't make sense here. Instead you may choose an alternate 
  *   texture manager.
  * \param tm Texture manager, used to determine the format the image is to
  *   be loaded in (defaults to CS_IMGFMT_TRUECOLOR if no texture manager is
  *   specified).
  * \param image Optionally returns a reference to the loaded image.
  */
  virtual csPtr<iTextureHandle> LoadTexture (iDataBuffer* buf,
    int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
    csRef<iImage>* image=0) = 0;
  /**
  * Load a texture as with LoadTexture() above and register it with the
  * engine. This version reads the image from a data buffer.
  * 
  * \param Name The name that the engine will use for the wrapper.
  * \param buf Buffer containing the image file data.
  * \param Flags Accepts the flags described in ivideo/txtmgr.h.
  *   The texture manager determines the format, so choosing an alternate 
  *   format doesn't make sense here. Instead you may choose an alternate 
  *   texture manager.
  * \param tm Texture manager, used to determine the format the image is to
  *   be loaded in (defaults to CS_IMGFMT_TRUECOLOR if no texture manager is
  *   specified).
  * \param reg if true then the texture and material will be registered
  * to the texture manager. Set 'register' to false if you plan on calling
  * 'engine->Prepare()' later as that function will take care of registering
  * too.
  * \param create_material if true then this function also creates a
  * material for the texture.
  * \param free_image if true then after registration the loaded image
  * will be removed immediatelly. This saves some memory. Set to false
  * if you want to keep it. free_image is ignored if reg is false.
  */
  virtual iTextureWrapper* LoadTexture (const char *Name,
    iDataBuffer* buf,
    int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
    bool reg = true, bool create_material = true,
    bool free_image = true) = 0;

  /**
  * Load a Mesh Object Factory from a file.
  * \param fname is the VFS name of the file.
  * \param ssource is an optional stream source for faster loading.
  */
  virtual csPtr<iMeshFactoryWrapper> LoadMeshObjectFactory (
    const char* fname, iStreamSource* ssource = 0) = 0;
  /**
  * Load a mesh object from a file.
  * The mesh object is not automatically added to any sector.
  * \param fname is the VFS name of the file.
  * \param ssource is an optional stream source for faster loading.
  */
  virtual csPtr<iMeshWrapper> LoadMeshObject (const char* fname,
    iStreamSource* ssource = 0) = 0;

  /**
  * Load a shader from a file.
  */
  virtual csRef<iShader> LoadShader (const char* filename, bool registerShader = true) = 0;

  //@{
  /**
  * Load a texture as with LoadTexture() above and register it with the
  * engine. 
  *
  * \param Name The name that the engine will use for the wrapper.
  * \param FileName VFS path to the image file to load.
  * \param Flags Accepts the flags described in ivideo/txtmgr.h.
  *   The texture manager determines the format, so choosing an alternate 
  *   format doesn't make sense here. Instead you may choose an alternate 
  *   texture manager.
  * \param tm Texture manager, used to determine the format the image is to
  *   be loaded in (defaults to CS_IMGFMT_TRUECOLOR if no texture manager is
  *   specified).
  * \param reg If true then the texture and material will be registered
  *   to the texture manager. Set 'register' to false if you plan on calling
  *   'engine->Prepare()' later as that function will take care of registering
  *   too.
  * \param create_material If true then this function also creates a
  *   material for the texture.
  * \param free_image If true then after registration the loaded image
  *   will be removed immediatelly. This saves some memory. Set to false
  *   if you want to keep it. free_image is ignored if reg is false.
  * \param collection [optional] Collection to register the texture
  *   and material to.
  */
  virtual iTextureWrapper* LoadTexture (const char *Name,
    const char *FileName, int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
    bool reg = true, bool create_material = true, bool free_image = true,
    iCollection* Collection = 0, uint keepFlags = KEEP_ALL) = 0;
  CS_DEPRECATED_METHOD_MSG("Regions are deprecated. Use Collections instead.")
  virtual iTextureWrapper* LoadTexture (const char *Name,
    const char *FileName, int Flags, iTextureManager *tm,
    bool reg, bool create_material, bool free_image,
    iRegion* region) = 0;
  /* Hack to ensure source compatibility when a 0 collection/region is used.
   * Remove with region variant. */
  CS_FORCEINLINE iTextureWrapper* LoadTexture (const char *Name,
    const char *FileName, int Flags, iTextureManager *tm,
    bool reg, bool create_material, bool free_image,
    int dummy)
  { 
    return LoadTexture (Name, FileName, Flags, tm, reg, create_material,
      free_image, (iCollection*)0); 
  }
  //@}

  //@}
  /**
  * Load a map file. If 'clearEngine' is true then the current contents
  * of the engine will be deleted before loading.
  * If 'region' is not 0 then portals will only connect to the
  * sectors in that region, things will only use thing templates
  * defined in that region and meshes will only use mesh factories
  * defined in that region. If the region is not 0 and curRegOnly is true
  * then objects (materials, factories, ...) will only be found in the
  * given region.
  * <p>
  * \param filename is a VFS filename for the XML file.
  * \param clearEngine is true by default which means that the previous
  * contents of the engine is cleared (all objects/sectors/... are removed).
  * \param collection is 0 by default which means that all loaded
  * objects are added to the default collection. If you give a collection
  * here then all loaded objects will be added to that collection (subject
  * to keepFlags).
  * \param curRegOnly is true by default which means that it will only
  * find materials/factories/... from current region if that is given.
  * \param checkDupes if true then materials, textures,
  * and mesh factories will only be loaded if they don't already exist
  * in the entire engine (ignoring regions). By default this is false because
  * it is very legal for different world files to have different objects
  * with the same name. Only use checkDupes == true if you know that your
  * objects have unique names accross all world files.
  * \param ssource is an optional stream source for faster loading.
  * \param missingdata is an optional callback in case data is missing.
  * The application can then provide that missing data in some other way.
  * \param Set useProxyTextures to true if you're loading materials and
  * textures from a library shared with other maps.
  * \param keepFlags Use these to define whether or not you wish to guarantee
  * that all loaded resources are kept (default to KEEP_ALL). KEEP_USED is useful
  * when you are loading from a shared library containing more resources than you
  * actually need (a world file loading from a shared library of textures for example).
  * This argument is only used in conjunction with Collections, and overrides the value
  * of useProxyTextures in that case.
  */
  virtual bool LoadMapFile (const char* filename, bool clearEngine = true,
    iCollection* collection = 0, bool curRegOnly = true,
    bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL) = 0;
  CS_DEPRECATED_METHOD_MSG("Regions are deprecated. Use Collections instead.")
  virtual bool LoadMapFile (const char* filename, bool clearEngine,
    iRegion* region, bool curRegOnly = true,
    bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0) = 0;
  /* Hack to ensure source compatibility when a 0 collection/region is used.
   * Remove with region variant. */
  CS_FORCEINLINE bool LoadMapFile (const char* filename, bool clearEngine,
    int dummy, bool curRegOnly = true,
    bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL)
  { 
    return LoadMapFile (filename, clearEngine, (iCollection*)0, curRegOnly,
			checkDupes, ssource, missingdata, keepFlags); 
  }
  //@}
  
  //@{
  /**
  * Load a map from the given 'world' node. If 'clearEngine' is true then
  * the current contents of the engine will be deleted before loading.
  * If 'region' is not 0 then portals will only connect to the
  * sectors in that region, things will only use thing templates
  * defined in that region and meshes will only use mesh factories
  * defined in that region. If the region is not 0 and curRegOnly is true
  * then objects (materials, factories, ...) will only be found in the
  * given region.
  * <p>
  * \param world_node is the 'world' node from an XML document.
  * \param clearEngine is true by default which means that the previous
  * contents of the engine is cleared (all objects/sectors/... are removed).
  * \param collection is 0 by default which means that all loaded objects are not
  * added to any collection. If you give a region here then all loaded objects
  * will be added to that region.
  * \param curRegOnly is true by default which means that it will only
  * find materials/factories/... from current region if that is given.
  * \param checkDupes if true then materials, textures,
  * and mesh factories will only be loaded if they don't already exist
  * in the entire engine (ignoring regions). By default this is false because
  * it is very legal for different world files to have different objects
  * with the same name. Only use checkDupes == true if you know that your
  * objects have unique names accross all world files.
  * \param ssource is an optional stream source for faster loading.
  * \param missingdata is an optional callback in case data is missing.
  * The application can then provide that missing data in some other way.
  * \param Set useProxyTextures to true if you're loading materials and
  * textures from a library shared with other maps.
  * \param keepFlags Use these to define whether or not you wish to guarantee
  * that all loaded resources are kept (default to KEEP_ALL). KEEP_USED is useful
  * when you are loading from a shared library containing more resources than you
  * actually need (a world file loading from a shared library of textures for example).
  * This argument is only used in conjunction with Collections, and overrides the value
  * of useProxyTextures in that case.
  */
  virtual bool LoadMap (iDocumentNode* world_node, bool clearEngine = true,
    iCollection* collection = 0, bool curRegOnly = true,
    bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL) = 0;
  CS_DEPRECATED_METHOD_MSG("Regions are deprecated. Use Collections instead.")
  virtual bool LoadMap (iDocumentNode* world_node, bool clearEngine,
    iRegion* region, bool curRegOnly = true,
    bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0) = 0;
  /* Hack to ensure source compatibility when a 0 collection/region is used.
   * Remove with region variant. */
  CS_FORCEINLINE bool LoadMap (iDocumentNode* world_node, bool clearEngine,
    int dummy, bool curRegOnly = true,
    bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0)
  { 
    return LoadMap (world_node, clearEngine, (iCollection*)0, curRegOnly,
		    checkDupes, ssource, missingdata);
  }
  //@}

  //@{
  /**
  * Load library from a VFS file
  * \param filename is a VFS filename for the XML file.
  * \param collection is 0 by default which means that all loaded objects are
  * either added to the default collection or not added to any collection,
  * depending on the value of the keepFlags argument.
  * \param searchCollectionOnly is true by default which means that it will only
  * find materials/factories/... from current collection if that is given.
  * \param checkDupes if true then materials, textures,
  * and mesh factories will only be loaded if they don't already exist
  * in the entire engine (ignoring regions). By default this is false because
  * it is very legal for different world files to have different objects
  * with the same name. Only use checkDupes == true if you know that your
  * objects have unique names accross all world files.
  * \param ssource is an optional stream source for faster loading.
  * \param missingdata is an optional callback in case data is missing.
  * The application can then provide that missing data in some other way.
  * \param keepFlags Use these to define whether or not you wish to guarantee
  * that all loaded resources are kept (default to KEEP_ALL). KEEP_USED is useful
  * when you are loading from a shared library containing more resources than you
  * actually need (a world file loading from a shared library of textures for example).
  */
  virtual bool LoadLibraryFile (const char* filename, iCollection* collection = 0,
    bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL) = 0;
  CS_DEPRECATED_METHOD_MSG("Regions are deprecated. Use Collections instead.")
  virtual bool LoadLibraryFile (const char* filename, iRegion* region,
    bool searchregionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0) = 0;
  /* Hack to ensure source compatibility when a 0 collection/region is used.
   * Remove with region variant. */
  CS_FORCEINLINE bool LoadLibraryFile (const char* filename, int dummy,
    bool searchregionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0)
  { 
    return LoadLibraryFile (filename, (iCollection*)0, searchregionOnly, 
			    checkDupes, ssource, missingdata);
  }
  //@}

  //@{
  /**
  * Load library from a 'library' node.
  * \param lib_node is the 'library' node from an XML document.
  * \param collection is 0 by default which means that all loaded objects are
  * either added to the default collection or not added to any collection,
  * depending on the value of the keepFlags argument.
  * \param searchCollectionOnly is true by default which means that it will only
  * find materials/factories/... from current collection if that is given.
  * \param checkDupes if true then materials, textures,
  * and mesh factories will only be loaded if they don't already exist
  * in the entire engine (ignoring regions). By default this is false because
  * it is very legal for different world files to have different objects
  * with the same name. Only use checkDupes == true if you know that your
  * objects have unique names accross all world files.
  * \param ssource is an optional stream source for faster loading.
  * \param missingdata is an optional callback in case data is missing.
  * The application can then provide that missing data in some other way.
  * \param keepFlags Use these to define whether or not you wish to guarantee
  * that all loaded resources are kept (default to KEEP_ALL). KEEP_USED is useful
  * when you are loading from a shared library containing more resources than you
  * actually need (a world file loading from a shared library of textures for example).
  */
  virtual bool LoadLibrary (iDocumentNode* lib_node, iCollection* collection = 0,
    bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0, uint keepFlags = KEEP_ALL) = 0;
  CS_DEPRECATED_METHOD_MSG("Regions are deprecated. Use Collections instead.")
  virtual bool LoadLibrary (iDocumentNode* lib_node, iRegion* region,
    bool searchRegionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0) = 0;
  /* Hack to ensure source compatibility when a 0 collection/region is used.
   * Remove with region variant. */
  CS_FORCEINLINE bool LoadLibrary (iDocumentNode* lib_node, int dummy,
    bool searchRegionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    iMissingLoaderData* missingdata = 0)
  { 
    return LoadLibrary (lib_node, (iCollection*)0, searchRegionOnly, 
			checkDupes, ssource, missingdata); 
  }
  //@)

  //@{
  /**
  * Load a file. This is a smart function that will try to recognize
  * what kind of file it is. It recognizes the following types of
  * files:
  * - 'world' file: in that case 'result' will be set to the engine.
  * - 'library' file: 'result' will be 0.
  * - 'meshfact' file: 'result' will be the mesh factory wrapper.
  * - 'meshobj' file: 'result' will be the mesh wrapper.
  * - 'meshref' file: 'result' will be the mesh wrapper.
  * - 3ds/md2 models: 'result' will be the mesh factory wrapper.
  * - 'portals' file: 'result' will be the portal's mesh wrapper.
  * - 'light' file: 'result' will be the light.
  *
  * Returns csLoadResult.
  * <br>
  * Note! In case a world file is loaded this function will NOT
  * clear the engine!
  * <br>
  * Note! In case a mesh factory or mesh object is loaded this function
  * will not actually do anything if checkDupes is true and the mesh or
  * factory is already in memory (with that name). This function will
  * still return true in that case and set 'result' to the correct object.
  * <br>
  * \param fname is a VFS filename for the XML file.
  * \param collection is 0 by default which means that all loaded objects are
  * either added to the default collection or not added to any collection,
  * depending on the value of the keepFlags argument.
  * \param searchCollectionOnly is true by default which means that it will only
  * find materials/factories/... from current collection if that is given.
  * \param checkDupes if true then materials, textures,
  * and mesh factories will only be loaded if they don't already exist
  * in the entire engine (ignoring regions). By default this is false because
  * it is very legal for different world files to have different objects
  * with the same name. Only use checkDupes == true if you know that your
  * objects have unique names accross all world files.
  * \param ssource is an optional stream source for faster loading.
  * \param override_name if this is given the the name of the loaded object
  * will be set to that. This only works in case of meshfact, meshobj, and
  * 3ds or md2 model.
  * \param missingdata is an optional callback in case data is missing.
  * The application can then provide that missing data in some other way.
  * \param keepFlags Use these to define whether or not you wish to guarantee
  * that all loaded resources are kept (default to KEEP_ALL). KEEP_USED is useful
  * when you are loading from a shared library containing more resources than you
  * actually need (a world file loading from a shared library of textures for example).
  */
  virtual csLoadResult Load (const char* fname, iCollection* collection = 0,
    bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0,
    uint keepFlags = KEEP_ALL) = 0;
  CS_DEPRECATED_METHOD_MSG("Regions are deprecated. Use Collections instead.")
  virtual csLoadResult Load (const char* fname, iRegion* region,
    bool searchRegionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0) = 0;
  /* Hack to ensure source compatibility when a 0 collection/region is used.
   * Remove with region variant. */
  CS_FORCEINLINE csLoadResult Load (const char* fname, int dummy,
    bool searchRegionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0)
  { 
    return Load (fname, (iCollection*)0, searchRegionOnly, checkDupes, ssource,
		 override_name, missingdata);
  }
  //@}

  //@{
  /**
  * Load a file. This is a smart function that will try to recognize
  * what kind of file it is. It recognizes the following types of
  * files:
  * - 'world' file: in that case 'result' will be set to the engine.
  * - 'library' file: 'result' will be 0.
  * - 'meshfact' file: 'result' will be the mesh factory wrapper.
  * - 'meshobj' file: 'result' will be the mesh wrapper.
  * - 'meshref' file: 'result' will be the mesh wrapper.
  * - 3ds/md2 models: 'result' will be the mesh factory wrapper.
  * - 'portals' file: 'result' will be the portal's mesh wrapper.
  * - 'light' file: 'result' will be the light.
  *
  * Returns csLoadResult.
  * <br>
  * Note! In case a world file is loaded this function will NOT
  * clear the engine!
  * <br>
  * Note! In case a mesh factory or mesh object is loaded this function
  * will not actually do anything if checkDupes is true and the mesh or
  * factory is already in memory (with that name). This function will
  * still return true in that case and set 'result' to the correct object.
  * <br>
  * \param buffer is a buffer for the model contents.
  * \param collection is 0 by default which means that all loaded objects are
  * either added to the default collection or not added to any collection,
  * depending on the value of the keepFlags argument.
  * \param searchCollectionOnly is true by default which means that it will only
  * find materials/factories/... from current collection if that is given.
  * \param checkDupes if true then materials, textures,
  * and mesh factories will only be loaded if they don't already exist
  * in the entire engine (ignoring regions). By default this is false because
  * it is very legal for different world files to have different objects
  * with the same name. Only use checkDupes == true if you know that your
  * objects have unique names accross all world files.
  * \param ssource is an optional stream source for faster loading.
  * \param override_name if this is given the the name of the loaded object
  * will be set to that. This only works in case of meshfact, meshobj, and
  * 3ds or md2 model.
  * \param missingdata is an optional callback in case data is missing.
  * The application can then provide that missing data in some other way.
  * \param keepFlags Use these to define whether or not you wish to guarantee
  * that all loaded resources are kept (default to KEEP_ALL). KEEP_USED is useful
  * when you are loading from a shared library containing more resources than you
  * actually need (a world file loading from a shared library of textures for example).
  */
  virtual csLoadResult Load (iDataBuffer* buffer, iCollection* collection = 0,
    bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0,
    uint keepFlags = KEEP_ALL) = 0;
  CS_DEPRECATED_METHOD_MSG("Regions are deprecated. Use Collections instead.")
  virtual csLoadResult Load (iDataBuffer* buffer, iRegion* region,
    bool searchRegionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0) = 0;
  /* Hack to ensure source compatibility when a 0 collection/region is used.
   * Remove with region variant. */
  CS_FORCEINLINE csLoadResult Load (iDataBuffer* buffer, int dummy,
    bool searchRegionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0)
  { 
    return Load (buffer, (iCollection*)0, searchRegionOnly, checkDupes,
		 ssource, override_name, missingdata); 
  }
  //@}

  //@{
  /**
  * Load a node. This is a smart function that will try to recognize
  * what kind of node it is. It recognizes the following types of
  * nodes:
  * - 'world' node: in that case 'result' will be set to the engine.
  * - 'library' node: 'result' will be 0.
  * - 'meshfact' node: 'result' will be the mesh factory wrapper.
  * - 'meshobj' node: 'result' will be the mesh wrapper.
  * - 'meshref' file: 'result' will be the mesh wrapper.
  * - 'portals' file: 'result' will be the portal's mesh wrapper.
  * - 'light' file: 'result' will be the light.
  *
  * Returns csLoadResult.
  * <br>
  * Note! In case a world node is loaded this function will NOT
  * clear the engine!
  * <br>
  * Note! In case a mesh factory or mesh object is loaded this function
  * will not actually do anything if checkDupes is true and the mesh or
  * factory is already in memory (with that name). This function will
  * still return true in that case and set 'result' to the correct object.
  * <br>
  * \param node is the node from which to read.
  * \param collection is 0 by default which means that all loaded objects are
  * either added to the default collection or not added to any collection,
  * depending on the value of the keepFlags argument.
  * \param searchCollectionOnly is true by default which means that it will only
  * find materials/factories/... from current collection if that is given.
  * \param checkDupes if true then materials, textures,
  * and mesh factories will only be loaded if they don't already exist
  * in the entire engine (ignoring regions). By default this is false because
  * it is very legal for different world files to have different objects
  * with the same name. Only use checkDupes == true if you know that your
  * objects have unique names accross all world files.
  * \param ssource is an optional stream source for faster loading.
  * \param override_name if this is given the the name of the loaded object
  * will be set to that. This only works in case of meshfact, meshobj, and
  * 3ds or md2 model.
  * \param missingdata is an optional callback in case data is missing.
  * The application can then provide that missing data in some other way.
  * \param keepFlags Use these to define whether or not you wish to guarantee
  * that all loaded resources are kept (default to KEEP_ALL). KEEP_USED is useful
  * when you are loading from a shared library containing more resources than you
  * actually need (a world file loading from a shared library of textures for example).
  */
  virtual csLoadResult Load (iDocumentNode* node, iCollection* collection = 0,
    bool searchCollectionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0,
    uint keepFlags = KEEP_ALL) = 0;
  CS_DEPRECATED_METHOD_MSG("Regions are deprecated. Use Collections instead.")
  virtual csLoadResult Load (iDocumentNode* node, iRegion* region,
    bool searchRegionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0) = 0;
  /* Hack to ensure source compatibility when a 0 collection/region is used.
   * Remove with region variant. */
  CS_FORCEINLINE csLoadResult Load (iDocumentNode* node, int dummy,
    bool searchRegionOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0)
  { 
    return Load (node, (iCollection*)0, searchRegionOnly, checkDupes, ssource,
		 override_name, missingdata);
  }
  //@}

  /**
  * Load a file.
  * \deprecated Deprecated in 1.3. Use the iLoader::Load() that returns
  * a csLoadResult object instead.
  */
  CS_DEPRECATED_METHOD_MSG("Use iLoader::Load() returning csLoadResult instead")
    virtual bool Load (const char* fname, iBase*& result, iRegion* region = 0,
    bool curRegOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0) = 0;

  /**
  * Load a file.
  * \deprecated Deprecated in 1.3. Use the iLoader::Load() that returns
  * a csLoadResult object instead.
  */
  CS_DEPRECATED_METHOD_MSG("Use iLoader::Load() returning csLoadResult instead")
    virtual bool Load (iDataBuffer* buffer, iBase*& result, iRegion* region = 0,
    bool curRegOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0) = 0;

  /**
  * Load a node.
  * \deprecated Deprecated in 1.3. Use the iLoader::Load() that returns
  * a csLoadResult object instead.
  */
  CS_DEPRECATED_METHOD_MSG("Use iLoader::Load() returning csLoadResult instead")
    virtual bool Load (iDocumentNode* node, iBase*& result, iRegion* region = 0,
    bool curRegOnly = true, bool checkDupes = false, iStreamSource* ssource = 0,
    const char* override_name = 0, iMissingLoaderData* missingdata = 0) = 0;

  /**
  * Set whether to load each file into a separate region.
  * \deprecated Deprecated in 1.3. Use the iCollections instead.
  */
  CS_DEPRECATED_METHOD_MSG("Regions are deprecated. Use Collections instead.")
    virtual void SetAutoRegions (bool autoRegions) = 0;

  /**
  * Get whether to load each file into a separate region.
  * \deprecated Deprecated in 1.3. Use the iCollections instead.
  */
  CS_DEPRECATED_METHOD_MSG("Regions are deprecated. Use Collections instead.")
    virtual bool GetAutoRegions () = 0;
};

/** @} */

#include "csutil/deprecated_warn_on.h"

#endif // __CS_IMAP_PARSER_H__

