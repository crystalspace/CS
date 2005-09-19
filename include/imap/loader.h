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
#include "csutil/scf.h"

#include "igraphic/image.h"
#include "ivideo/txtmgr.h"
#include "imap/streamsource.h"

struct iDocumentNode;
struct iImage;
struct iMaterialWrapper;
struct iMeshWrapper;
struct iMeshFactoryWrapper;
struct iRegion;
struct iSector;
struct iSoundData;
struct iSoundHandle;
struct iSoundWrapper;
struct iTextureHandle;
struct iTextureManager;
struct iTextureWrapper;

SCF_VERSION (iLoaderStatus, 0, 1, 0);

/**
 * An object to query about the status of the threaded loader.
 */
struct iLoaderStatus : public iBase
{
  /// Check if the loader is ready.
  virtual bool IsReady () = 0;
  /// Check if there was an error during loading.
  virtual bool IsError () = 0;
};

SCF_VERSION (iLoader, 0, 0, 8);

/**
 * This interface represents the map loader.
 */
struct iLoader : public iBase
{
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
   * The 'Flags' parameter accepts the flags described in ivideo/txtmgr.h.
   * The texture manager determines the format, so choosing an alternate format
   * doesn't make sense here. Instead you may choose an alternate texture
   * manager.
   */
  virtual csPtr<iTextureHandle> LoadTexture (const char* Filename,
	int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
	csRef<iImage>* image=0) = 0;
  /**
   * Load a texture as with LoadTexture() above and register it with the
   * engine. 'Name' is the name that the engine will use for the wrapper.
   * If 'create_material' is true then this function also creates a
   * material for the texture.<br>
   * If 'register' is true then the texture and material will be registered
   * to the texture manager. Set 'register' to false if you plan on calling
   * 'engine->Prepare()' later as that function will take care of registering
   * too.
   */
  virtual iTextureWrapper* LoadTexture (const char *Name,
  	const char *FileName,
	int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
	bool reg = false, bool create_material = true) = 0;

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
   * The 'Flags' parameter accepts the flags described in ivideo/txtmgr.h.
   * The texture manager determines the format, so choosing an alternate format
   * doesn't make sense here. Instead you may choose an alternate texture
   * manager.
   * This version reads the image from a data buffer.
   */
  virtual csPtr<iTextureHandle> LoadTexture (iDataBuffer* buf,
	int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
	csRef<iImage>* image=0) = 0;
  /**
   * Load a texture as with LoadTexture() above and register it with the
   * engine. 'Name' is the name that the engine will use for the wrapper.
   * If 'create_material' is true then this function also creates a
   * material for the texture.<br>
   * If 'register' is true then the texture and material will be registered
   * to the texture manager. Set 'register' to false if you plan on calling
   * 'engine->Prepare()' later as that function will take care of registering
   * too.
   * This version reads the image from a data buffer.
   */
  virtual iTextureWrapper* LoadTexture (const char *Name,
  	iDataBuffer* buf,
	int Flags = CS_TEXTURE_3D, iTextureManager *tm = 0,
	bool reg = false, bool create_material = true) = 0;

  /// Load a sound file and return an iSoundData object
  virtual csPtr<iSoundData> LoadSoundData (const char *fname) = 0;
  /// Load a sound file and register the sound
  virtual csPtr<iSoundHandle> LoadSound (const char *fname) = 0;
  /// Load a sound file, register the sound and create a wrapper object for it
  virtual csPtr<iSoundWrapper> LoadSound (const char *name,
  	const char *fname) = 0;

  /**
   * Load a map file in a thread.
   * If 'region' is not 0 then portals will only connect to the
   * sectors in that region, things will only use thing templates
   * defined in that region and meshes will only use mesh factories
   * defined in that region. If the region is not 0 and curRegOnly is true
   * then objects (materials, factories, ...) will only be found in the
   * given region.
   * <p>
   * If you use 'checkDupes' == true then materials, textures,
   * and mesh factories will only be loaded if they don't already exist
   * in the entire engine (ignoring regions). By default this is false because
   * it is very legal for different world files to have different objects
   * with the same name. Only use checkDupes == true if you know that your
   * objects have unique names accross all world files.
   * <p>
   * This function returns immediatelly. You can check the iLoaderStatus
   * instance that is returned to see if the map has finished loading or
   * if there was an error.
   * @@@ NOT IMPLEMENTED YET @@@
   */
  virtual csPtr<iLoaderStatus> ThreadedLoadMapFile (const char* filename,
	iRegion* region = 0, bool curRegOnly = true,
	bool checkDupes = false) = 0;

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
   * \param region is 0 by default which means that all loaded objects are not
   * added to any region. If you give a region here then all loaded objects
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
   */
  virtual bool LoadMapFile (const char* filename, bool clearEngine = true,
	iRegion* region = 0, bool curRegOnly = true,
	bool checkDupes = false, iStreamSource* ssource = 0) = 0;

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
   * \param region is 0 by default which means that all loaded objects are not
   * added to any region. If you give a region here then all loaded objects
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
   */
  virtual bool LoadMap (iDocumentNode* world_node, bool clearEngine = true,
	iRegion* region = 0, bool curRegOnly = true,
	bool checkDupes = false, iStreamSource* ssource = 0) = 0;

  /**
   * Load library from a VFS file
   * \param filename is a VFS filename for the XML file.
   * \param region is 0 by default which means that all loaded objects are not
   * added to any region. If you give a region here then all loaded objects
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
   */
  virtual bool LoadLibraryFile (const char* filename, iRegion* region = 0,
  	bool curRegOnly = true, bool checkDupes = false,
	iStreamSource* ssource = 0) = 0;

  /**
   * Load library from a 'library' node.
   * \param lib_node is the 'library' node from an XML document.
   * \param region is 0 by default which means that all loaded objects are not
   * added to any region. If you give a region here then all loaded objects
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
   */
  virtual bool LoadLibrary (iDocumentNode* lib_node, iRegion* region = 0,
  	bool curRegOnly = true, bool checkDupes = false,
	iStreamSource* ssource) = 0;

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
   * Load a file. This is a smart function that will try to recognize
   * what kind of file it is. It recognizes the following types of
   * files:
   * <ul>
   * <li>'world' file: in that case 'result' will be set to the engine.
   * <li>'library' file: 'result' will be 0.
   * <li>'meshfact' file: 'result' will be the mesh factory wrapper.
   * <li>'meshobj' file: 'result' will be the mesh wrapper.
   * </ul>
   * Returns false on failure.
   * <br>
   * Note! In case a world file is loaded this function will NOT
   * clear the engine!
   * <br>
   * Note! In case a mesh factory or mesh object is loaded this function
   * will not actually do anything checkDupes is true and the mesh or
   * factory is already in memory (with that name). This function will
   * still return true in that case and set 'result' to the correct object.
   * <br>
   * Note! Use SCF_QUERY_INTERFACE on 'result' to detect what type was loaded.
   * \param fname is a VFS filename for the XML file.
   * \param result will be set to the loaded result (see above).
   * \param region is 0 by default which means that all loaded objects are not
   * added to any region. If you give a region here then all loaded objects
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
   */
  virtual bool Load (const char* fname, iBase*& result, iRegion* region = 0,
  	bool curRegOnly = true, bool checkDupes = false,
	iStreamSource* ssource = 0) = 0;

  /**
   * Load a node. This is a smart function that will try to recognize
   * what kind of node it is. It recognizes the following types of
   * nodes:
   * <ul>
   * <li>'world' node: in that case 'result' will be set to the engine.
   * <li>'library' node: 'result' will be 0.
   * <li>'meshfact' node: 'result' will be the mesh factory wrapper.
   * <li>'meshobj' node: 'result' will be the mesh wrapper.
   * </ul>
   * Returns false on failure.
   * <br>
   * Note! In case a world node is loaded this function will NOT
   * clear the engine!
   * <br>
   * Note! In case a mesh factory or mesh object is loaded this function
   * will not actually do anything checkDupes is true and the mesh or
   * factory is already in memory (with that name). This function will
   * still return true in that case and set 'result' to the correct object.
   * <br>
   * Note! Use SCF_QUERY_INTERFACE on 'result' to detect what type was loaded.
   * \param node is the node from which to read.
   * \param result will be set to the loaded result (see above).
   * \param region is 0 by default which means that all loaded objects are not
   * added to any region. If you give a region here then all loaded objects
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
   */
  virtual bool Load (iDocumentNode* node, iBase*& result, iRegion* region = 0,
  	bool curRegOnly = true, bool checkDupes = false,
	iStreamSource* ssource = 0) = 0;
};

/** @} */

#endif // __CS_IMAP_PARSER_H__

