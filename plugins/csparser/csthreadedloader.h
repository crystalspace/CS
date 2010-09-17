/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Ivan Avramovic <ivan@avramovic.com>

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

#ifndef __CS_THREADED_LOADER_H__
#define __CS_THREADED_LOADER_H__

#include "csutil/cscolor.h"
#include "csutil/threadmanager.h"
#include "csutil/scf_implementation.h"
#include "csutil/strhash.h"
#include "csutil/weakrefarr.h"
#include "csutil/weakrefhash.h"

#include "iengine/campos.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/portal.h"
#include "iengine/sector.h"
#include "iengine/sharevar.h"
#include "imap/ldrctxt.h"
#include "imap/loader.h"
#include "isndsys/ss_loader.h"
#include "isndsys/ss_manager.h"
#include "isndsys/ss_renderer.h"
#include "iutil/comp.h"
#include "iutil/object.h"
#include "ivaria/engseq.h"

#include "ldrplug.h"
#include "proxyimage.h"

class csReversibleTransform;
struct iCollection;
struct iDocumentNode;
struct iEngine;
struct iImageIO;
struct iImposterFactory;
struct iLODControl;
struct iMapNode;
struct iObjectModel;
struct iObjectRegistry;
struct iSceneNodeArray;
struct iShaderVarStringSet;
struct iStringSet;
struct iSyntaxService;
struct iTriangleMesh;
struct iVFS;

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  class csLoaderContext;

  template<class T, class Interface>
  class csLoaderIterator : public scfImplementation1<csLoaderIterator<T, Interface>,
                                                     Interface>
  {
  public:
    csLoaderIterator(csRefArray<T>* objects, CS::Threading::ReadWriteMutex* lock) :
        scfImplementation1<csLoaderIterator<T, Interface>,
                           Interface> (this),
        rwl(lock), lk(*lock), objects(objects), itr(objects->GetIterator())
        {
        }

        virtual ~csLoaderIterator()
        {
          rwl->UpgradeUnlockAndWriteLock();
          objects->Empty();
          rwl->WriteUnlockAndUpgradeLock();
        }

        T* Next()
        {
          return itr.Next();
        }

        bool HasNext() const
        {
          return itr.HasNext();
        }

  private:
    CS::Threading::ReadWriteMutex* rwl;
    CS::Threading::ScopedUpgradeableLock lk;
    csRefArray<T>* objects;
    typename csRefArray<T>::Iterator itr;
  };

  class csThreadedLoader : public ThreadedCallable<csThreadedLoader>,
                           public scfImplementation3<csThreadedLoader,
                                                     iThreadedLoader,
                                                     iComponent,
                                                     iEventHandler>
  {
  public:
    csThreadedLoader(iBase *p);
    virtual ~csThreadedLoader();

    virtual bool Initialize(iObjectRegistry *object_reg);

    iObjectRegistry* GetObjectRegistry() const { return object_reg; }

    virtual csPtr<iSectorLoaderIterator> GetLoaderSectors()
    {
      csRef<iSectorLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iSector, iSectorLoaderIterator>(&loaderSectors, &sectorsLock));
      return csPtr<iSectorLoaderIterator>(itr);
    }
    virtual csPtr<iMeshFactLoaderIterator> GetLoaderMeshFactories()
    {
      csRef<iMeshFactLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iMeshFactoryWrapper, iMeshFactLoaderIterator>(&loaderMeshFactories, &meshfactsLock));
      return csPtr<iMeshFactLoaderIterator>(itr);
    }
    virtual csPtr<iMeshLoaderIterator> GetLoaderMeshes()
    {
      csRef<iMeshLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iMeshWrapper, iMeshLoaderIterator>(&loaderMeshes, &meshesLock));
      return csPtr<iMeshLoaderIterator>(itr);
    }
    virtual csPtr<iCamposLoaderIterator> GetLoaderCameraPositions()
    {
      csRef<iCamposLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iCameraPosition, iCamposLoaderIterator>(&loaderCameraPositions, &camposLock));
      return csPtr<iCamposLoaderIterator>(itr);
    }
    virtual csPtr<iTextureLoaderIterator> GetLoaderTextures()
    {
      csRef<iTextureLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iTextureWrapper, iTextureLoaderIterator>(&loaderTextures, &texturesLock));
      return csPtr<iTextureLoaderIterator>(itr);
    }
    virtual csPtr<iMaterialLoaderIterator> GetLoaderMaterials()
    {
      csRef<iMaterialLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iMaterialWrapper, iMaterialLoaderIterator>(&loaderMaterials, &materialsLock));
      return csPtr<iMaterialLoaderIterator>(itr);
    }
    virtual csPtr<iSharedVarLoaderIterator> GetLoaderSharedVariables()
    {
      csRef<iSharedVarLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iSharedVariable, iSharedVarLoaderIterator>(&loaderSharedVariables, &sharedvarLock));
      return csPtr<iSharedVarLoaderIterator>(itr);
    }

    THREADED_CALLABLE_DECL4(csThreadedLoader, LoadImage, csLoaderReturn, const char*, cwd,
      const char*, fname, int, Format, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL4(csThreadedLoader, LoadImage, csLoaderReturn, const char*, cwd,
    csRef<iDataBuffer>, buf, int, Format, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL6(csThreadedLoader, LoadTexture, csLoaderReturn, const char*, cwd, const char*, Filename,
    int, Flags, csRef<iTextureManager>, texman, csRef<iImage>*, image, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL6(csThreadedLoader, LoadTexture, csLoaderReturn, const char*, cwd, csRef<iDataBuffer>, buf,
    int, Flags, csRef<iTextureManager>, texman, csRef<iImage>*, image, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL9(csThreadedLoader, LoadTexture, csLoaderReturn, const char*, cwd, const char*, Name,
    csRef<iDataBuffer>, buf, int, Flags, csRef<iTextureManager>, texman, bool, reg, bool, create_material, bool,
    free_image, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL11(csThreadedLoader, LoadTexture, csLoaderReturn, const char*, cwd, const char*, Name,
    const char*, FileName, int, Flags, csRef<iTextureManager>, texman, bool, reg, bool, create_material,
    bool, free_image, csRef<iCollection>, collection, uint, keepFlags, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL3(csThreadedLoader, LoadSoundSysData, csLoaderReturn, const char*, cwd, const char*, fname,
    bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL4(csThreadedLoader, LoadSoundStream, csLoaderReturn, const char*, cwd, const char*, fname,
    int, mode3d, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL4(csThreadedLoader, LoadSoundWrapper, csLoaderReturn, const char*, cwd, const char*, name,
    const char*, fname, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL4(csThreadedLoader, LoadMeshObjectFactory, csLoaderReturn, const char*, cwd, const char*, fname,
    csRef<iStreamSource>, ssource, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL4(csThreadedLoader, LoadMeshObject, csLoaderReturn, const char*, cwd, const char*, fname,
    csRef<iStreamSource>, ssource, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL4(csThreadedLoader, LoadShader, csLoaderReturn, const char*, cwd, const char*, filename,
    bool, registerShader, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL8(csThreadedLoader, LoadMapFile, csLoaderReturn, const char*, cwd, const char*, filename,
    bool, clearEngine, csRef<iCollection>, collection, csRef<iStreamSource>, ssource, csRef<iMissingLoaderData>,
    missingdata, uint, keepFlags, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL8(csThreadedLoader, LoadMap, csLoaderReturn, const char*, cwd, csRef<iDocumentNode>, world_node,
    bool, clearEngine, csRef<iCollection>, collection, csRef<iStreamSource>, ssource, csRef<iMissingLoaderData>,
    missingdata, uint, keepFlags, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL7(csThreadedLoader, LoadLibraryFile, csLoaderReturn, const char*, cwd, const char*, filename,
    csRef<iCollection>, collection, csRef<iStreamSource>, ssource, csRef<iMissingLoaderData>, missingdata, uint,
    keepFlags, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL7(csThreadedLoader, LoadLibrary, csLoaderReturn, const char*, cwd, csRef<iDocumentNode>, lib_node,
    csRef<iCollection>, collection, csRef<iStreamSource>, ssource, csRef<iMissingLoaderData>, missingdata, uint,
    keepFlags, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL7(csThreadedLoader, LoadFile, csLoaderReturn, const char*, cwd, const char*, fname,
    csRef<iCollection>, collection, csRef<iStreamSource>, ssource, csRef<iMissingLoaderData>, missingdata, uint,
    keepFlags, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL7(csThreadedLoader, LoadBuffer, csLoaderReturn, const char*, cwd, csRef<iDataBuffer>, buffer,
    csRef<iCollection>, collection, csRef<iStreamSource>, ssource, csRef<iMissingLoaderData>, missingdata, uint,
    keepFlags, bool, do_verbose, THREADED, false, false)

    THREADED_CALLABLE_DECL8(csThreadedLoader, LoadNode, csLoaderReturn, const char*, cwd, csRef<iDocumentNode>,
    node, csRef<iCollection>, collection, csRef<iSector>, sector, csRef<iStreamSource>, ssource, csRef<iMissingLoaderData>,
    missingdata, uint, keepFlags, bool, do_verbose, THREADED, false, false)

    void AddSectorToList(iSector* obj)
    {
      {
        CS::Threading::ScopedWriteLock lock(sectorsLock);
        loaderSectors.Push(obj);
        obj->DecRef(); // Compensate for CreateSector IncRef().
      }
      MarkSyncNeeded();
    }

    void AddMeshFactToList(iMeshFactoryWrapper* obj)
    {
      {
        CS::Threading::ScopedWriteLock lock(meshfactsLock);
        loaderMeshFactories.Push(obj);
      }
      MarkSyncNeeded();
    }

    void AddMeshToList(iMeshWrapper* obj)
    {
      {
        CS::Threading::ScopedWriteLock lock(meshesLock);
        loaderMeshes.Push(obj);
      }
      MarkSyncNeeded();
    }

    void AddCamposToList(iCameraPosition* obj)
    {
      {
        CS::Threading::ScopedWriteLock lock(camposLock);
        loaderCameraPositions.Push(obj);
      }
      MarkSyncNeeded();
    }

    void AddTextureToList(iTextureWrapper* obj)
    {
      {
        CS::Threading::ScopedWriteLock lock(texturesLock);
        loaderTextures.Push(obj);
      }
      MarkSyncNeeded();
    }

    void AddMaterialToList(iMaterialWrapper* obj)
    {
      {
        CS::Threading::ScopedWriteLock lock(materialsLock);
        loaderMaterials.Push(obj);
      }
      MarkSyncNeeded();
    }

    void AddSharedVarToList(iSharedVariable* obj)
    {
      {
        CS::Threading::ScopedWriteLock lock(sharedvarLock);
        loaderSharedVariables.Push(obj);
      }
      MarkSyncNeeded();
    }

    void AddLightToList(iLight* obj, const char* name)
    {
      {
        CS::Threading::ScopedWriteLock lock(lightsLock);
        loadedLights.Put(csString(name), obj);
      }
      MarkSyncNeeded();
    }

    void MarkSyncDone()
    {
      CS::Threading::ScopedWriteLock lock(listSyncLock);
      listSync = false;
    }

    iVFS* GetVFS() const { return vfs; }

    const int& GetFlags () const
    {
      return loaderFlags;
    }

    void SetFlags (int flags)
    {
      loaderFlags = flags;
    }

    /**\name iEventHandler implementation
     * @{ */
    bool HandleEvent (iEvent& event);

    CS_EVENTHANDLER_NAMES("crystalspace.threadedloader")
    CS_EVENTHANDLER_NIL_CONSTRAINTS
    /** @} */
  protected:

    friend class csLoaderContext;

    // Shared lists and locks.
    CS::Threading::ReadWriteMutex sectorsLock;
    CS::Threading::ReadWriteMutex meshfactsLock;
    CS::Threading::ReadWriteMutex meshesLock;
    CS::Threading::ReadWriteMutex camposLock;
    CS::Threading::ReadWriteMutex texturesLock;
    CS::Threading::ReadWriteMutex materialsLock;
    CS::Threading::ReadWriteMutex sharedvarLock;
    CS::Threading::ReadWriteMutex lightsLock;

    // Final objects.
    csRefArray<iSector> loaderSectors;
    csRefArray<iMeshFactoryWrapper> loaderMeshFactories;
    csRefArray<iMeshWrapper> loaderMeshes;
    csRefArray<iCameraPosition> loaderCameraPositions;
    csRefArray<iTextureWrapper> loaderTextures;
    csRefArray<iMaterialWrapper> loaderMaterials;
    csRefArray<iSharedVariable> loaderSharedVariables;
    csWeakRefHash<iLight, csString> loadedLights;

    // Clear all of the loader lists
    void ClearLoaderLists ();

    // General loading objects.
    csHash<csRef<iThreadReturn>, csString> loadingMeshObjects;
    CS::Threading::Mutex loadingMeshObjectsLock;

    void AddLoadingMeshObject(const char* name, csRef<iThreadReturn> itr)
    {
      CS::Threading::MutexScopedLock lock(loadingMeshObjectsLock);
      if(!FindLoadedMeshObject(name))
      {
        loadingMeshObjects.Put(name, itr);
      }
    }

    bool FindLoadedMeshObject(const char* name)
    {
      CS::Threading::ScopedReadLock lock(meshesLock);
      for(size_t i=0; i<loaderMeshes.GetSize(); i++)
      {
        if(csString(name).Compare(loaderMeshes[i]->QueryObject()->GetName()))
        {
          return true;
        }
      }

      return false;
    }

    void RemoveLoadingMeshObject(const char* name, csRef<iThreadReturn> itr)
    {
      CS::Threading::MutexScopedLock lock(loadingMeshObjectsLock);
      loadingMeshObjects.Delete(name, itr);
    }

    // Loading texture objects.
    csArray<csString> loadingTextures;
    CS::Threading::RecursiveMutex loadingTexturesLock;

    bool AddLoadingTexture(const char* name)
    {
      CS::Threading::RecursiveMutexScopedLock lock(loadingTexturesLock);
      if(!FindLoadingTexture(name))
      {
        loadingTextures.Push(name);
        return true;
      }
      return false;
    }

    bool FindLoadingTexture(const char* name)
    {
      CS::Threading::RecursiveMutexScopedLock lock(loadingTexturesLock);
      return loadingTextures.Find(name) != csArrayItemNotFound;
    }

    void RemoveLoadingTexture(const char* name)
    {
      CS::Threading::RecursiveMutexScopedLock lock(loadingTexturesLock);
      loadingTextures.Delete(name);
    }

    // Loading material objects.
    csArray<csString> loadingMaterials;
    CS::Threading::RecursiveMutex loadingMaterialsLock;

    bool AddLoadingMaterial(const char* name)
    {
      CS::Threading::RecursiveMutexScopedLock lock(loadingMaterialsLock);
      if(!FindLoadingMaterial(name))
      {
        loadingMaterials.Push(name);
        return true;
      }
      return false;
    }

    bool FindLoadingMaterial(const char* name)
    {
      CS::Threading::RecursiveMutexScopedLock lock(loadingMaterialsLock);
      return loadingMaterials.Find(name) != csArrayItemNotFound;
    }

    void RemoveLoadingMaterial(const char* name)
    {
      CS::Threading::RecursiveMutexScopedLock lock(loadingMaterialsLock);
      loadingMaterials.Delete(name);
    }

    // Loading meshfact objects.
    csArray<csString> loadingMeshFacts;
    CS::Threading::RecursiveMutex loadingMeshFactsLock;

    bool AddLoadingMeshFact(const char* name)
    {
      CS::Threading::RecursiveMutexScopedLock lock(loadingMeshFactsLock);
      if(!FindLoadingMeshFact(name))
      {
        return true;
        loadingMeshFacts.Push(name);
      }
      return false;
    }

    bool FindLoadingMeshFact(const char* name)
    {
      CS::Threading::RecursiveMutexScopedLock lock(loadingMeshFactsLock);
      return loadingMeshFacts.Find(name) != csArrayItemNotFound;
    }

    void RemoveLoadingMeshFact(const char* name)
    {
      CS::Threading::RecursiveMutexScopedLock lock(loadingMeshFactsLock);
      loadingMeshFacts.Delete(name);
    }

  private:
    csRef<iEngine> Engine;
    csRef<iVFS> vfs;
    csRef<iGraphics3D> g3d;
    // Parser for common stuff like MixModes, vectors, matrices, ...
    csRef<iSyntaxService> SyntaxService;
    // Object registry.
    iObjectRegistry *object_reg;
    // Shared string set
    csRef<iStringSet> stringSet;
    // Shader string set
    csRef<iShaderVarStringSet> stringSetSvName;
    // Image loader
    csRef<iImageIO> ImageLoader;
    // Pointer to the engine sequencer (optional module).
    csRef<iEngineSequenceManager> eseqmgr;
    // Pointer to the global thread manager.
    csWeakRef<iThreadManager> threadman;
    // Sound loader
    csRef<iSndSysLoader> SndSysLoader;
    // Sound manager
    csRef<iSndSysManager> SndSysManager;
    // Sound renderer
    csRef<iSndSysRenderer> SndSysRenderer;
    // Flags
    int loaderFlags;
    // Weak event handler
    csRef<iEventHandler> eventHandler;

    // For checking whether to schedule a engine list sync.
    CS::Threading::ReadWriteMutex listSyncLock;
    bool listSync;

    void MarkSyncNeeded()
    {
      bool syncNow = false;
      {
        CS::Threading::ScopedUpgradeableLock sLock(listSyncLock);
        if(!listSync)
        {
          listSyncLock.UpgradeUnlockAndWriteLock();
          if(!listSync)
          {
            listSync = true;
            syncNow = true;
          }
          listSyncLock.WriteUnlockAndUpgradeLock();
        }
      }

      if(syncNow)
        Engine->SyncEngineLists(this);
    }

    // ----------------------------------------------- //
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

    // Parses the data to find all materials and meshfacts.
    void ParseAvailableObjects(csLoaderContext* ldr_context, iDocumentNode* doc,
      csRefArray<iDocumentNode>& libs, csArray<csString>& libIDs);

    // Returns in the 'meshesArray' array all the meshes encountered walking through
    // the hierarchy of meshes starting from 'meshWrapper'.
    void CollectAllChildren (iMeshWrapper* meshWrapper, csRefArray<iMeshWrapper>&
      meshesArray);

    // Two useful private functions to set the CS_TRIMESH_CLOSED and
    // CS_TRIMESH_CONVEX flags on a single mesh wrapper.
    void ConvexFlags (iMeshWrapper* mesh);
    void ClosedFlags (iMeshWrapper* mesh);

    // Load all proxy textures which are used.
    bool LoadProxyTextures(csSafeCopyArray<ProxyTexture> &proxyTextures,
      csWeakRefArray<iMaterialWrapper> &materialArray);

    THREADED_CALLABLE_DECL7(csThreadedLoader, FindOrLoadMeshFactory, csLoaderReturn,
      const char*, name, csRef<iLoaderContext>, ldr_context, csRef<iDocumentNode>,
      meshfactnode, csRef<iMeshFactoryWrapper>, parent, csReversibleTransform*,
      transf, csRef<iStreamSource>, ssource, const char*, path, THREADED, false, false);

    /**
    * Load a Mesh Object Factory from the map file.
    * If the transformation pointer is given then this is for a hierarchical
    * mesh object factory and the transformation will be filled in with
    * the relative transform (from MOVE keyword).
    * parent is not 0 if the factory is part of a hierarchical factory.
    */
    bool LoadMeshObjectFactory(iLoaderContext* ldr_context, iMeshFactoryWrapper* meshFact,
      iMeshFactoryWrapper* parent, iDocumentNode* node, csReversibleTransform* transf,
      iStreamSource* ssource);

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
    THREADED_CALLABLE_DECL8(csThreadedLoader, LoadMeshObject, csLoaderReturn,
      csRef<iLoaderContext>, ldr_context, csRef<iMeshWrapper>, mesh, csRef<iMeshWrapper>, parent,
      csRef<iDocumentNode>, node, csRef<iStreamSource>, ssource, csRef<iSector>, sector, csString,
      name, const char*, cwd, THREADED, false, false);

    THREADED_CALLABLE_DECL2(csThreadedLoader, AddObjectToSector, csLoaderReturn,
      csRef<iMovable>, movable, csRef<iSector>, sector, MED, false, false);

    THREADED_CALLABLE_DECL4(csThreadedLoader, LoadMeshRef, csLoaderReturn, csRef<iDocumentNode>,
      node, csRef<iSector>, sector, csRef<iLoaderContext>, ldr_context, csRef<iStreamSource>,
      ssource, THREADED, false, false);

    /**
    * Load the mesh object from the map file.
    * This version will parse FACTORY statement to directly create
    * a mesh from a factory.
    */
    csRef<iMeshWrapper> LoadMeshObjectFromFactory (iLoaderContext* ldr_context,
      iDocumentNode* node, iStreamSource* ssource);

    /// Load map from a memory buffer
    bool LoadMap (iLoaderContext* ldr_context, iDocumentNode* world_node,
      iStreamSource* ssource, iMissingLoaderData* missingdata, bool do_verbose);

    bool Load (iThreadReturn* ret, iDataBuffer* buffer, const char* fname, iCollection* collection,
      iStreamSource* ssource, iMissingLoaderData* missingdata, uint keepFlags = KEEP_ALL,
      bool do_verbose = false);

    /**
    * Load a library into given engine.
    * A library is just a map file that contains just mesh factories,
    * sounds and textures.
    */
    bool LoadLibrary(iLoaderContext* ldr_context, iDocumentNode* node,
      iStreamSource* ssource, iMissingLoaderData* missingdata, csRefArray<iThreadReturn>& threadReturns,
      csRefArray<iDocumentNode>& libs, csArray<csString>& libIDs, bool loadProxyTex = true, bool mapload = false,
      bool do_verbose = false);

    THREADED_CALLABLE_DECL5(csThreadedLoader, LoadLibraryFromNode, csLoaderReturn,
      csRef<iLoaderContext>, ldr_context, csRef<iDocumentNode>, child, 
      csRefArray<iDocumentNode>*, libs, csArray<csString>*, libIDs, const char*, cwd, THREADED, false, false);

    /// Lock on pushing lib nodes and id to lists.
    CS::Threading::Mutex preParseLibsLock;

    csPtr<iImage> LoadImage (iDataBuffer* buf, const char* fname, int Format, bool do_verbose);

    /**
    * Load a LOD control object.
    */
    bool LoadLodControl (iLODControl* lodctrl, iDocumentNode* node);

    /**
    * Load a plugin in general.
    */
    bool LoadAddOn (iLoaderContext* ldr_context,
      iDocumentNode* node, iBase* context, bool is_meta,
      iStreamSource* ssource);

    bool LoadShaderExpressions (iLoaderContext* ldr_context,
      iDocumentNode* node);

    /**
    * Load the trimesh object from the map file.
    */
    bool LoadTriMeshInSector (iLoaderContext* ldr_context,
      iMeshWrapper* mesh, iDocumentNode* node, iStreamSource* ssource);

    bool LoadDeferredLibs(csRefArray<iDocumentNode>& defLibs,
    iLoaderContext* ldr_context, iStreamSource* ssource, iMissingLoaderData* missingdata,
    csRefArray<iThreadReturn>& threadReturns, csRefArray<iDocumentNode>& libs,
    csArray<csString>& libIDs, bool do_verbose);

    /// Parse a shaderlist
    bool ParseShaderList (csLoaderContext* ldr_context);
    bool ParseShader (iLoaderContext* ldr_context, iDocumentNode* node,
      iShaderManager* shaderMgr);

    /// Parse a list of shared variables and add them each to the engine
    bool ParseVariableList (iLoaderContext* ldr_context, iDocumentNode* node);
    /// Process the attributes of one shared variable
    bool ParseSharedVariable (iLoaderContext* ldr_context, iDocumentNode* node);

    /// Parse a map node definition and add the node to the given sector
    iMapNode* ParseNode (iDocumentNode* node, iSector* sec);

    /**
    * Parse a key/value pair.
    * Takes "editoronly" attribute into account: KVPs should only be parsed 
    * if they're not editor-only or when the engine is in "saveable" mode.
    */
    bool ParseKey (iDocumentNode* node, iObject* obj);

    /// Parse a texture definition and add the texture to the engine
    THREADED_CALLABLE_DECL4(csThreadedLoader, ParseTexture, csLoaderReturn,
      csRef<iLoaderContext>, ldr_context, csRef<iDocumentNode>, node,
      csSafeCopyArray<ProxyTexture>*, proxyTextures, const char*, path,
      THREADED, false, false);

    /// Parse a material definition and add the material to the engine
    bool ParseMaterial (iLoaderContext* ldr_context,
      iDocumentNode* node, csWeakRefArray<iMaterialWrapper> &materialArray,
      const char* prefix = 0);

    /// Parse a renderloop.
    iRenderLoop* ParseRenderLoop (iDocumentNode* node, bool& set);

    /// Parse a addon.
    THREADED_CALLABLE_DECL6(csThreadedLoader, ParseAddOn, csLoaderReturn,
      csRef<iLoaderPlugin>, plugin, csRef<iDocumentNode>, node, csRef<iStreamSource>, ssource,
      csRef<iLoaderContext>, ldr_context, csRef<iBase>, context, const char*, dir, HIGH, true, false);

    /// Parse a addon (binary plugin).
    THREADED_CALLABLE_DECL5(csThreadedLoader, ParseAddOnBinary, csLoaderReturn,
      csRef<iBinaryLoaderPlugin>, plugin, csRef<iDataBuffer>, dbuf, csRef<iStreamSource>,
      ssource, csRef<iLoaderContext>, ldr_context, csRef<iBase>, context, HIGH, true, false);

    /**
    * Try loading the file as a structured document.
    * \return True if the documented loaded and appears to be a map file,
    *   otherwise false.
    */
    csPtr<iBase> LoadStructuredMap (iLoaderContext* ldr_context, iLoaderPlugin* plug,
      iFile* buf, iBase* context, const char* fname, iStreamSource* ssource);

    /**
    * Try loading file as a structured document via iDocumentSystem.
    * \return False on failure.
    */
    bool LoadStructuredDoc (const char* file, iFile* buf, csRef<iDocument>& doc);

    /**
    * Try loading file as a structured document via iDocumentSystem.
    * \return False on failure.
    */
    bool LoadStructuredDoc (const char* file, iDataBuffer* buf, csRef<iDocument>& doc);

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
    * Load a single plugin.
    */
    void LoadPlugin (iDocumentNode* node);

    /**
    * Load the settings section.
    */
    bool LoadSettings (iDocumentNode* node);

    bool LoadTextures (csLoaderContext* ldr_context,
      csSafeCopyArray<ProxyTexture>* proxyTextures);

    bool LoadMaterials (csLoaderContext* ldr_context,
      csSafeCopyArray<ProxyTexture>* proxyTextures,
      csWeakRefArray<iMaterialWrapper> &materialArray);

    bool LoadMeshfacts (csLoaderContext* ldr_context,
      iStreamSource* ssource, csSafeCopyArray<ProxyTexture>* proxyTextures,
    csWeakRefArray<iMaterialWrapper> &materialArray);

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
    * Handle the result of a mesh object plugin loader.
    */
    bool HandleMeshObjectPluginResult (iBase* mo, iDocumentNode* child,
      iMeshWrapper* mesh, bool keepZbuf, bool keepPrio);

    // Parse a 'trimesh' block.
    bool ParseTriMesh (iDocumentNode* node, iObjectModel* objmodel);
    bool ParseTriMeshChildBox (iDocumentNode* child, csRef<iTriangleMesh>& trimesh);
    bool ParseTriMeshChildMesh (iDocumentNode* child, csRef<iTriangleMesh>& trimesh);

    /// Parse a camera position.
    bool ParseStart (iDocumentNode* node, iCameraPosition* campos);

    /// Parse a sector definition and add the sector to the engine
    iSector* ParseSector (iLoaderContext* ldr_context, iDocumentNode* node,
      iStreamSource* ssource, csRefArray<iThreadReturn>& threadReturns);

    THREADED_CALLABLE_DECL3(csThreadedLoader, SetSectorVisibilityCuller, csLoaderReturn,
      csRef<iSector>, sector, const char*, culplugname, csRef<iDocumentNode>, culler_params,
      MED, false, false)

      // Process the attributes of an <imposter> tag in a mesh specification.
      bool ParseImposterSettings(iImposterFactory* mesh, iDocumentNode *node);

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

    /// Parse a static light definition and add the light to the engine
    iLight* ParseStatlight (iLoaderContext* ldr_context, iDocumentNode* node);

    /// Find the named shared variable and verify its type if specified
    iSharedVariable *FindSharedVariable(const char *colvar,
      int verify_type );

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

    /**
    * Add children to the collection.
    */
    void AddChildrenToCollection (iLoaderContext* ldr_context,
      const iSceneNodeArray* children);

    // List of loaded plugins
    csLoadedPluginVector loaded_plugins;

    // Tokens
    csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "plugins/csparser/csloader.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE

    // Reporting
    void ReportError (const char* id, const char* description, ...);
    void ReportNotify (const char* description, ...);
    void ReportNotifyV (const char* id, const char* description, va_list arg);
    void ReportNotify2 (const char* id, const char* description, ...);
    void ReportWarning (const char* id, const char* description, ...);
    void ReportWarning (const char* id, iDocumentNode* node, const char* description, ...);
  };

  class csMissingSectorCallback : 
    public scfImplementation1<csMissingSectorCallback, 
    iPortalCallback>
  {
  public:
    csRef<iLoaderContext> ldr_context;
    csString sectorname;
    bool autoresolve;

    csMissingSectorCallback (iLoaderContext* ldr_context, const char* sector,
      bool autoresolve) : scfImplementationType (this), ldr_context (ldr_context), 
      sectorname (sector), autoresolve (autoresolve)
    {
    }
    virtual ~csMissingSectorCallback ()
    {
    }

    virtual bool Traverse (iPortal* portal, iBase* /*context*/)
    {
      iSector* sector = ldr_context->FindSector (sectorname);
      if (!sector) return false;
      portal->SetSector (sector);
      // For efficiency reasons we deallocate the name here.
      if (!autoresolve)
      {
        sectorname.Empty ();
        portal->RemoveMissingSectorCallback (this);
      }
      return true;
    }
  };

}
CS_PLUGIN_NAMESPACE_END(csparser)

#endif // __CS_THREADED_LOADER_H__
