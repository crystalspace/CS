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

#include "csutil/refarr.h"
#include "csutil/refcount.h"
#include "csutil/scf_interface.h"
#include "csutil/threading/condition.h"

#include "igraphic/image.h"
#include "ivideo/txtmgr.h"
#include "imap/streamsource.h"
#include "iutil/job.h"
#include "iutil/threadmanager.h"

#include "csutil/deprecated_warn_off.h"

struct iCameraPosition;
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
struct iSharedVariable;
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
 * Return structure for the iLoader::Load() routines.
 */
struct csLoadResult
{
  /// True if the object was loaded successfully.
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
  csRef<iBase> result;
};

/**
 * Return structure for threaded loader functions.
 */
class csLoaderReturn : public scfImplementation1<csLoaderReturn, iThreadReturn>
{
public:
  csLoaderReturn(iThreadManager* tm) : scfImplementationType(this),
    finished(false), success(false), waitLock(0), wait(0), tm(tm)
  {
  }

  virtual ~csLoaderReturn()
  {
  }

  bool IsFinished()
  {
    CS::Threading::MutexScopedLock lock(updateLock);
    return finished;
  }

  bool WasSuccessful()
  {
    CS::Threading::MutexScopedLock lock(updateLock);
    return success;
  }

  void* GetResultPtr()
  { CS_ASSERT_MSG("csLoaderReturn does not implement a void* result", false); return NULL; }

  csRef<iBase> GetResultRefPtr() { return result; }

  void MarkFinished()
  {
    if(waitLock)
      waitLock->Lock();

    {
      CS::Threading::MutexScopedLock ulock(updateLock);

      finished = true;
      if(wait)
      {
        wait->NotifyAll();
      }
    }

    if(waitLock)
      waitLock->Unlock();
  }

  void MarkSuccessful()
  {
    CS::Threading::MutexScopedLock lock(updateLock);
    success = true;
  }

  void SetResult(void* result)
  { CS_ASSERT_MSG("csLoaderReturn does not implement a void* result", false); }

  void SetResult(csRef<iBase> result) { this->result = result; }

  void Copy(iThreadReturn* other)
  {
    result = other->GetResultRefPtr();
  }

  void Wait(bool process = true)
  {
    if(tm.IsValid())
    {
      csRefArray<iThreadReturn> rets;
      rets.Push(this);
      tm->Wait(rets, process);
    }
  }

  void SetWaitPtrs(CS::Threading::Condition* c, CS::Threading::Mutex* m)
  {
    CS::Threading::MutexScopedLock lock(updateLock);
    wait = c;
    waitLock = m;
  }

  void SetJob(iJob* j)
  {
      job = j;
  }

  iJob* GetJob() const
  {
      return job;
  }

private:
  /// True if the loading has finished (should be true at some point).
  bool finished;

  /// True if the object was loaded successfully.
  bool success;

  /// Wait condition.
  CS::Threading::Mutex* waitLock;
  CS::Threading::Condition* wait;
  CS::Threading::Mutex updateLock;

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
  csRef<iBase> result;

  // Reference to the thread manager.
  csRef<iThreadManager> tm;

  // Pointer to the thread job.
  csRef<iJob> job;
};

struct iSectorLoaderIterator : public virtual iBase
{
  SCF_INTERFACE (iSectorLoaderIterator, 1, 0, 0);

  virtual iSector* Next() = 0;

  virtual bool HasNext() const = 0;
};

struct iMeshFactLoaderIterator : public virtual iBase
{
  SCF_INTERFACE (iMeshFactLoaderIterator, 1, 0, 0);

  virtual iMeshFactoryWrapper* Next() = 0;

  virtual bool HasNext() const = 0;
};

struct iMeshLoaderIterator : public virtual iBase
{
  SCF_INTERFACE (iMeshLoaderIterator, 1, 0, 0);

  virtual iMeshWrapper* Next() = 0;

  virtual bool HasNext() const = 0;
};

struct iCamposLoaderIterator : public virtual iBase
{
  SCF_INTERFACE (iCamposLoaderIterator, 1, 0, 0);

  virtual iCameraPosition* Next() = 0;

  virtual bool HasNext() const = 0;
};

struct iTextureLoaderIterator : public virtual iBase
{
  SCF_INTERFACE (iTextureLoaderIterator, 1, 0, 0);

  virtual iTextureWrapper* Next() = 0;

  virtual bool HasNext() const = 0;
};

struct iMaterialLoaderIterator : public virtual iBase
{
  SCF_INTERFACE (iMaterialLoaderIterator, 1, 0, 0);

  virtual iMaterialWrapper* Next() = 0;

  virtual bool HasNext() const = 0;
};

struct iSharedVarLoaderIterator : public virtual iBase
{
  SCF_INTERFACE (iSharedVarLoaderIterator, 1, 0, 0);

  virtual iSharedVariable* Next() = 0;

  virtual bool HasNext() const = 0;
};

/**
 * Loader flags
 */
#define CS_LOADER_NONE 0
#define CS_LOADER_CREATE_DUMMY_MATS 1

/**
* This interface represents the threaded map loader methods.
*/
struct iThreadedLoader : public virtual iBase
{
  SCF_INTERFACE (iThreadedLoader, 2, 3, 0);

 /**
  * Get the loader sector list.
  */
  virtual csPtr<iSectorLoaderIterator> GetLoaderSectors() = 0;
  
 /**
  * Get the loader mesh factory list.
  */
  virtual csPtr<iMeshFactLoaderIterator> GetLoaderMeshFactories() = 0;
  
 /**
  * Get the loader mesh sector list.
  */
  virtual csPtr<iMeshLoaderIterator> GetLoaderMeshes() = 0;

 /**
  * Get the loader camera list.
  */
  virtual csPtr<iCamposLoaderIterator> GetLoaderCameraPositions() = 0;
  
 /**
  * Get the loader texture list.
  */
  virtual csPtr<iTextureLoaderIterator> GetLoaderTextures() = 0;
  
 /**
  * Get the loader material list.
  */
  virtual csPtr<iMaterialLoaderIterator> GetLoaderMaterials() = 0;
  
 /**
  * Get the loader shared variable list.
  */
  virtual csPtr<iSharedVarLoaderIterator> GetLoaderSharedVariables() = 0;

 /**
  * Load an image file. The image will be loaded in the format requested by
  * the engine. If no engine exists, the format is taken from the video
  * renderer. If no video renderer exists, this function fails. You may also
  * request an alternate format to override the above sequence.
  * \param result Result of the method call.
  * \param Filename VFS path to the image file to load.
  * \param Format The format of the image.
  */
  THREADED_INTERFACE4(LoadImage, const char* cwd, const char* Filename, int Format = CS_IMGFMT_INVALID,
    bool do_verbose = false);

 /**
  * Load an image file. The image will be loaded in the format requested by
  * the engine. If no engine exists, the format is taken from the video
  * renderer. If no video renderer exists, this function fails. You may also
  * request an alternate format to override the above sequence.
  * This version reads the image from a data buffer.
  */
  THREADED_INTERFACE4(LoadImage, const char* cwd, csRef<iDataBuffer> buf, int Format = CS_IMGFMT_INVALID,
  bool do_verbose = false);

  /**
  * Load an image as with LoadImage() and create a texture handle from it.
  * \param result Result of the method call.
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
  THREADED_INTERFACE6(LoadTexture, const char* cwd, const char* Filename, int Flags = CS_TEXTURE_3D, 
  csRef<iTextureManager> tm = 0, csRef<iImage>* image = 0, bool do_verbose = false);

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
  THREADED_INTERFACE6(LoadTexture, const char* cwd, csRef<iDataBuffer> buf, int Flags = CS_TEXTURE_3D,
  csRef<iTextureManager> texman = 0, csRef<iImage>* image = 0, bool do_verbose = false);

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
  * 'iEngine::Prepare()' later as that function will take care of registering
  * too.
  * \param create_material if true then this function also creates a
  * material for the texture.
  * \param free_image if true then after registration the loaded image
  * will be removed immediatelly. This saves some memory. Set to false
  * if you want to keep it. free_image is ignored if reg is false.
  */
  THREADED_INTERFACE9(LoadTexture, const char* cwd, const char *Name, csRef<iDataBuffer> buf,
    int Flags = CS_TEXTURE_3D, csRef<iTextureManager> texman = 0, bool reg = true,
    bool create_material = true, bool free_image = true, bool do_verbose = false);

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
  *   'iEngine::Prepare()' later as that function will take care of registering
  *   too.
  * \param create_material If true then this function also creates a
  *   material for the texture.
  * \param free_image If true then after registration the loaded image
  *   will be removed immediatelly. This saves some memory. Set to false
  *   if you want to keep it. free_image is ignored if reg is false.
  * \param collection [optional] Collection to register the texture
  *   and material to.
  */
  THREADED_INTERFACE11(LoadTexture, const char* cwd, const char *Name, const char *FileName,
  int Flags = CS_TEXTURE_3D, csRef<iTextureManager> texman = 0, bool reg = true,
  bool create_material = true, bool free_image = true, csRef<iCollection> Collection = 0,
  uint keepFlags = KEEP_ALL, bool do_verbose = false);
  //@}

  /// New Sound System: Load a sound file and return an iSndSysData object
  THREADED_INTERFACE3(LoadSoundSysData, const char* cwd, const char *fname,
  bool do_verbose = false);

  /**
  * New Sound System: Load a sound file and create a stream from it.
  * \param fname is the VFS filename.
  * \param mode3d is one of CS_SND3D_DISABLE, CS_SND3D_RELATIVE, or
  * CS_SND3D_ABSOLUTE.
  */
  THREADED_INTERFACE4(LoadSoundStream, const char* cwd, const char *fname, int mode3d,
  bool do_verbose = false);

  /**
  * New Sound System: Load a sound file, create sound data and create a
  * wrapper object for it.
  * \param name of the sound.
  * \param fname is the VFS filename.
  */
  THREADED_INTERFACE4(LoadSoundWrapper, const char* cwd, const char *name, const char *fname,
  bool do_verbose = false);


 /**
  * Load a Mesh Object Factory from a file.
  * \param fname is the VFS name of the file.
  * \param ssource is an optional stream source for faster loading.
  */
  THREADED_INTERFACE4(LoadMeshObjectFactory, const char* cwd, const char* fname, csRef<iStreamSource> ssource = 0,
  bool do_verbose = false);

 /**
  * Load a mesh object from a file.
  * The mesh object is not automatically added to any sector.
  * \param fname is the VFS name of the file.
  * \param ssource is an optional stream source for faster loading.
  */
  THREADED_INTERFACE4(LoadMeshObject, const char* cwd, const char* fname, csRef<iStreamSource> ssource = 0,
  bool do_verbose = false);


 /**
  * Load a shader from a file.
  */
  THREADED_INTERFACE4(LoadShader, const char* cwd, const char* filename, bool registerShader = true,
  bool do_verbose = false);

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
  * \param searchCollectionOnly is true by default which means that it will only
  * find materials/factories/... from current collection if that is given.
  * \param checkDupes if true then materials, textures,
  * and mesh factories will only be loaded if they don't already exist
  * in the entire engine (ignoring collections). By default this is false because
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
  THREADED_INTERFACE8(LoadMapFile, const char* cwd, const char* filename, bool clearEngine = true,
  csRef<iCollection> collection = 0, csRef<iStreamSource> ssource = 0, csRef<iMissingLoaderData> missingdata = 0,
  uint keepFlags = KEEP_ALL, bool do_verbose = false);
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
  * added to any collection. If you give a collection here then all loaded objects
  * will be added to that collection.
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
  * \param Set useProxyTextures to true if you're loading materials and
  * textures from a library shared with other maps.
  * \param keepFlags Use these to define whether or not you wish to guarantee
  * that all loaded resources are kept (default to KEEP_ALL). KEEP_USED is useful
  * when you are loading from a shared library containing more resources than you
  * actually need (a world file loading from a shared library of textures for example).
  * This argument is only used in conjunction with Collections, and overrides the value
  * of useProxyTextures in that case.
  */
  THREADED_INTERFACE8(LoadMap, const char* cwd, csRef<iDocumentNode> world_node, bool clearEngine = true,
  csRef<iCollection> collection = 0, csRef<iStreamSource> ssource = 0, csRef<iMissingLoaderData> missingdata = 0,
  uint keepFlags = KEEP_ALL, bool do_verbose = false);
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
  * in the entire engine (ignoring collections). By default this is false because
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
  THREADED_INTERFACE7(LoadLibraryFile, const char* cwd, const char* filename, csRef<iCollection> collection = 0,
  csRef<iStreamSource> ssource = 0, csRef<iMissingLoaderData> missingdata = 0, uint keepFlags = KEEP_ALL,
  bool do_verbose = false);
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
  * in the entire engine (ignoring collections). By default this is false because
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
  THREADED_INTERFACE7(LoadLibrary, const char* cwd, csRef<iDocumentNode> lib_node, csRef<iCollection> collection = 0,
  csRef<iStreamSource> ssource = 0, csRef<iMissingLoaderData> missingdata = 0, uint keepFlags = KEEP_ALL,
  bool do_verbose = false);
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
  * in the entire engine (ignoring collections). By default this is false because
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
  THREADED_INTERFACE7(LoadFile, const char* cwd, const char* fname, csRef<iCollection> collection = 0,
  csRef<iStreamSource> ssource = 0, csRef<iMissingLoaderData> missingdata = 0, uint keepFlags = KEEP_ALL,
  bool do_verbose = false);
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
  * in the entire engine (ignoring collections). By default this is false because
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
  THREADED_INTERFACE7(LoadBuffer, const char* cwd, csRef<iDataBuffer> buffer, csRef<iCollection> collection = 0,
  csRef<iStreamSource> ssource = 0, csRef<iMissingLoaderData> missingdata = 0, uint keepFlags = KEEP_ALL,
  bool do_verbose = false);
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
  * in the entire engine (ignoring collections). By default this is false because
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
  THREADED_INTERFACE8(LoadNode, const char* cwd, csRef<iDocumentNode> node, csRef<iCollection> collection = 0,
  csRef<iSector> sector = 0, csRef<iStreamSource> ssource = 0, csRef<iMissingLoaderData> missingdata = 0,
  uint keepFlags = KEEP_ALL, bool do_verbose = false);
  //@}

  /// Add object to the transfer list.
  virtual void AddSectorToList(iSector* obj) = 0;
  virtual void AddMeshFactToList(iMeshFactoryWrapper* obj) = 0;
  virtual void AddMeshToList(iMeshWrapper* obj) = 0;
  virtual void AddCamposToList(iCameraPosition* obj) = 0;
  virtual void AddTextureToList(iTextureWrapper* obj) = 0;
  virtual void AddMaterialToList(iMaterialWrapper* obj) = 0;
  virtual void AddSharedVarToList(iSharedVariable* obj) = 0;

  virtual void MarkSyncDone() = 0;

  // Get/Set loader flags.
  virtual const int& GetFlags () const = 0;
  virtual void SetFlags (int flags) = 0;
};

/**
* This interface represents the map loader.
*/
struct iLoader : public virtual iBase
{
  SCF_INTERFACE (iLoader, 5, 0, 0);

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
  * 'iEngine::Prepare()' later as that function will take care of registering
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
  *   'iEngine::Prepare()' later as that function will take care of registering
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
  //@}
};

/** @} */

#include "csutil/deprecated_warn_on.h"

#endif // __CS_IMAP_PARSER_H__

