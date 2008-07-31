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
#include "iengine/sector.h"
#include "iengine/sharevar.h"
#include "imap/loader.h"
#include "iutil/comp.h"

#include "ldrplug.h"
#include "proxyimage.h"

class csReversibleTransform;
struct iCollection;
struct iDocumentNode;
struct iEngine;
struct iImageIO;
struct iImposter;
struct iLODControl;
struct iObject;
struct iObjectModel;
struct iObjectRegistry;
struct iShaderVarStringSet;
struct iStringSet;
struct iSyntaxService;
struct iVFS;

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  template<class T, class Interface>
  class csLoaderIterator : public scfImplementation1<csLoaderIterator<T, Interface>,
                                                     Interface>
  {
  public:
    csLoaderIterator(csRefArray<T>* objects, CS::Threading::Mutex* lock) :
        scfImplementationType(this), objects(objects), lock(*lock), itr(objects->GetIterator())
        {
        }

        virtual ~csLoaderIterator()
        {
          objects->Empty();
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
    CS::Threading::MutexScopedLock lock;
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

    bool HandleEvent(iEvent&);
    CS_EVENTHANDLER_NAMES("crystalspace.level.loader.threaded")
    CS_EVENTHANDLER_NIL_CONSTRAINTS

    virtual bool Initialize(iObjectRegistry *object_reg);

    iObjectRegistry* GetObjectRegistry() const { return object_reg; }

    csPtr<iSectorLoaderIterator> GetLoaderSectors()
    {
      csRef<iSectorLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iSector, iSectorLoaderIterator>(&loaderSectors, &sectorsLock));
      return csPtr<iSectorLoaderIterator>(itr);
    }
    csPtr<iMeshFactLoaderIterator> GetLoaderMeshFactories()
    {
      csRef<iMeshFactLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iMeshFactoryWrapper, iMeshFactLoaderIterator>(&loaderMeshFactories, &meshfactsLock));
      return csPtr<iMeshFactLoaderIterator>(itr);
    }
    csPtr<iMeshLoaderIterator> GetLoaderMeshes()
    {
      csRef<iMeshLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iMeshWrapper, iMeshLoaderIterator>(&loaderMeshes, &meshesLock));
      return csPtr<iMeshLoaderIterator>(itr);
    }
    csPtr<iCamposLoaderIterator> GetLoaderCameraPositions()
    {
      csRef<iCamposLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iCameraPosition, iCamposLoaderIterator>(&loaderCameraPositions, &camposLock));
      return csPtr<iCamposLoaderIterator>(itr);
    }
    csPtr<iTextureLoaderIterator> GetLoaderTextures()
    {
      csRef<iTextureLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iTextureWrapper, iTextureLoaderIterator>(&loaderTextures, &texturesLock));
      return csPtr<iTextureLoaderIterator>(itr);
    }
    csPtr<iMaterialLoaderIterator> GetLoaderMaterials()
    {
      csRef<iMaterialLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iMaterialWrapper, iMaterialLoaderIterator>(&loaderMaterials, &materialsLock));
      return csPtr<iMaterialLoaderIterator>(itr);
    }
    csPtr<iSharedVarLoaderIterator> GetLoaderSharedVariables()
    {
      csRef<iSharedVarLoaderIterator> itr;
      itr.AttachNew(new csLoaderIterator<iSharedVariable, iSharedVarLoaderIterator>(&loaderSharedVariables, &sharedvarLock));
      return csPtr<iSharedVarLoaderIterator>(itr);
    }

    THREADED_CALLABLE_DECL2(csThreadedLoader, LoadImage, csLoaderReturn, const char*, fname,
      int, Format, true, false)

    THREADED_CALLABLE_DECL2(csThreadedLoader, LoadImage, csLoaderReturn, iDataBuffer*, buf,
      int, Format, true, false)

    THREADED_CALLABLE_DECL4(csThreadedLoader, LoadTexture, csLoaderReturn, const char*, Filename,
    int, Flags, csRef<iTextureManager>, texman, csRef<iImage>*, image, true, false)

    THREADED_CALLABLE_DECL4(csThreadedLoader, LoadTexture, csLoaderReturn, iDataBuffer*, buf,
    int, Flags, iTextureManager*, texman, csRef<iImage>*, image, true, false)

    THREADED_CALLABLE_DECL7(csThreadedLoader, LoadTexture, csLoaderReturn, const char*, Name,
    iDataBuffer*, buf, int, Flags, iTextureManager*, texman, bool, reg, bool, create_material, bool,
    free_image, true, false)

    THREADED_CALLABLE_DECL9(csThreadedLoader, LoadTexture, csLoaderReturn, const char*, Name,
    const char*, FileName, int, Flags, iTextureManager*, texman, bool, reg, bool, create_material,
    bool, free_image, iCollection*, Collection, uint, keepFlags, true, false)

    THREADED_CALLABLE_DECL1(csThreadedLoader, LoadSoundSysData, csLoaderReturn, const char*, fname, true, false)

    THREADED_CALLABLE_DECL2(csThreadedLoader, LoadSoundStream, csLoaderReturn, const char*, fname,
    int, mode3d, true, false)

    THREADED_CALLABLE_DECL2(csThreadedLoader, LoadSoundWrapper, csLoaderReturn, const char*, name,
    const char*, fname, true, false)

    THREADED_CALLABLE_DECL2(csThreadedLoader, LoadMeshObjectFactory, csLoaderReturn, const char*, fname,
    iStreamSource*, ssource, true, false)

    THREADED_CALLABLE_DECL2(csThreadedLoader, LoadMeshObject, csLoaderReturn, const char*, fname,
    iStreamSource*, ssource, true, false)

    THREADED_CALLABLE_DECL2(csThreadedLoader, LoadShader, csLoaderReturn, const char*, filename,
    bool, registerShader, true, false)

    THREADED_CALLABLE_DECL8(csThreadedLoader, LoadMapFile, csLoaderReturn, const char*, filename,
    bool, clearEngine, iCollection*, collection, bool, searchCollectionOnly, bool, checkDupes,
    iStreamSource*, ssource, iMissingLoaderData*, missingdata, uint, keepFlags, true, false)

    THREADED_CALLABLE_DECL8(csThreadedLoader, LoadMap, csLoaderReturn, iDocumentNode*, world_node,
    bool, clearEngine, iCollection*, collection, bool, searchCollectionOnly, bool, checkDupes,
    iStreamSource*, ssource, iMissingLoaderData*, missingdata, uint, keepFlags, true, false)

    THREADED_CALLABLE_DECL7(csThreadedLoader, LoadLibraryFile, csLoaderReturn, const char*, filename,
    iCollection*, collection, bool, searchCollectionOnly, bool, checkDupes, iStreamSource*, ssource,
    iMissingLoaderData*, missingdata, uint, keepFlags, true, false)

    THREADED_CALLABLE_DECL7(csThreadedLoader, LoadLibrary, csLoaderReturn, iDocumentNode*, lib_node,
    iCollection*, collection, bool, searchCollectionOnly, bool, checkDupes, iStreamSource*, ssource,
    iMissingLoaderData*, missingdata, uint, keepFlags, true, false)

    THREADED_CALLABLE_DECL8(csThreadedLoader, LoadFile, csLoaderReturn, const char*, fname,
    iCollection*, collection, bool, searchCollectionOnly, bool, checkDupes, iStreamSource*, ssource,
    const char*, override_name, iMissingLoaderData*, missingdata, uint, keepFlags, true, false)

    THREADED_CALLABLE_DECL8(csThreadedLoader, LoadBuffer, csLoaderReturn, iDataBuffer*, buffer,
    iCollection*, collection, bool, searchCollectionOnly, bool, checkDupes, iStreamSource*, ssource,
    const char*, override_name, iMissingLoaderData*, missingdata, uint, keepFlags, true, false)

    THREADED_CALLABLE_DECL9(csThreadedLoader, LoadNode, csLoaderReturn, csRef<iDocumentNode>,
    node, csRef<iCollection>, collection, bool, searchCollectionOnly, bool, checkDupes,
    csRef<iStreamSource>, ssource, const char*, override_name, csRef<iMissingLoaderData>,
    missingdata, uint, keepFlags, bool, do_verbose, true, false)

  private:
      csRef<iEngine> Engine;
      csRef<iVFS> vfs;
      csRef<iGraphics3D> g3d;
      // Parser for common stuff like MixModes, vectors, matrices, ...
      csRef<iSyntaxService> SyntaxService;
      iObjectRegistry *object_reg;
      /// Shared string set
      csRef<iStringSet> stringSet;
      csRef<iShaderVarStringSet> stringSetSvName;
      // image loader
      csRef<iImageIO> ImageLoader;
      csRef<iThreadManager> tm;
      csEventID ProcessPerFrame;

      // Shared lists and locks.
      Mutex sectorsLock;
      Mutex meshfactsLock;
      Mutex meshesLock;
      Mutex camposLock;
      Mutex texturesLock;
      Mutex materialsLock;
      Mutex sharedvarLock;

      csRefArray<iSector> loaderSectors;
      csRefArray<iMeshFactoryWrapper> loaderMeshFactories;
      csRefArray<iMeshWrapper> loaderMeshes;
      csRefArray<iCameraPosition> loaderCameraPositions;
      csRefArray<iTextureWrapper> loaderTextures;
      csRefArray<iMaterialWrapper> loaderMaterials;
      csRefArray<iSharedVariable> loaderSharedVariables;

      void AddSectorToList(csRef<iSector> obj)
      {
        MutexScopedLock lock(sectorsLock);
        loaderSectors.Push(obj);
      }

      void AddMeshFactToList(csRef<iMeshFactoryWrapper> obj)
      {
        MutexScopedLock lock(meshfactsLock);
        loaderMeshFactories.Push(obj);
      }

      void AddMeshToList(csRef<iMeshWrapper> obj)
      {
        MutexScopedLock lock(meshesLock);
        loaderMeshes.Push(obj);
      }

      void AddCamposToList(csRef<iCameraPosition> obj)
      {
        MutexScopedLock lock(camposLock);
        loaderCameraPositions.Push(obj);
      }

      void AddTextureToList(csRef<iTextureWrapper> obj)
      {
        MutexScopedLock lock(texturesLock);
        loaderTextures.Push(obj);
      }

      void AddMaterialToList(csRef<iMaterialWrapper> obj)
      {
        MutexScopedLock lock(materialsLock);
        loaderMaterials.Push(obj);
      }

      void AddShareVarToList(csRef<iSharedVariable> obj)
      {
        MutexScopedLock lock(sharedvarLock);
        loaderSharedVariables.Push(obj);
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

      // Load all proxy textures which are used.
      bool LoadProxyTextures(csSafeCopyArray<ProxyTexture> &proxyTextures,
        csWeakRefArray<iMaterialWrapper> &materialArray);

      THREADED_CALLABLE_DECL6(csThreadedLoader, LoadMeshFactory,
        csLoaderReturn, iLoaderContext*, ldr_context,
        iDocumentNode*, meshfactnode, const char*, override_name,
        iMeshFactoryWrapper*, parent, csReversibleTransform*, transf,
        iStreamSource*, ssource, true, false);

      bool LoadMapLibraryFile (const char* filename, iCollection* collection,
        bool searchCollectionOnly, bool checkDupes, iStreamSource* ssource,
        iMissingLoaderData* missingdata, uint keepFlags = KEEP_ALL,
        bool loadProxyTex = true, bool do_verbose = false);

      /**
      * Load a Mesh Object Factory from the map file.
      * If the transformation pointer is given then this is for a hierarchical
      * mesh object factory and the transformation will be filled in with
      * the relative transform (from MOVE keyword).
      * parent is not 0 if the factory is part of a hierarchical factory.
      */
      bool LoadMeshObjectFactory(iLoaderContext* ldr_context, iMeshFactoryWrapper* meshFact,
        iMeshFactoryWrapper* parent, iDocumentNode* node, csReversibleTransform* transf = 0,
        iStreamSource* ssource = 0);

      /**
      * Load a library into given engine.
      * A library is just a map file that contains just mesh factories,
      * sounds and textures.
      */
      bool LoadLibrary(iLoaderContext* ldr_context, iDocumentNode* node,
        iStreamSource* ssource, iMissingLoaderData* missingdata, bool loadProxyTex = true);

      bool LoadLibraryFromNode(iLoaderContext* ldr_context, iDocumentNode* child,
        iStreamSource* ssource, iMissingLoaderData* missingdata, bool loadProxyTex = true);

      csPtr<iImage> LoadImage (iDataBuffer* buf, const char* fname, int Format);

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
      * Parse a key/value pair.
      * Takes "editoronly" attribute into account: KVPs should only be parsed 
      * if they're not editor-only or when the engine is in "saveable" mode.
      */
      bool ParseKey (iDocumentNode* node, iObject* obj);

      /// Parse a list of textures and add them to the engine.
      bool ParseTextureList (iLoaderContext* ldr_context, iDocumentNode* node,
        csSafeCopyArray<ProxyTexture> &proxyTextures);

      /**
      * Parse a list of materials and add them to the engine. If a prefix is
      * given, all material names will be prefixed with the corresponding string.
      */
      bool ParseMaterialList (iLoaderContext* ldr_context, iDocumentNode* node,
        csWeakRefArray<iMaterialWrapper> &materialArray, const char* prefix = 0);

      /// Parse a texture definition and add the texture to the engine
      iTextureWrapper* ParseTexture (iLoaderContext* ldr_context,
        iDocumentNode* node, csSafeCopyArray<ProxyTexture> &proxyTextures);

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
        iDocumentNode* node, csWeakRefArray<iMaterialWrapper> &materialArray,
        const char* prefix = 0);

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

      // Parse a 'trimesh' block.
      bool ParseTriMesh (iDocumentNode* node, iObjectModel* objmodel);
      bool ParseTriMeshChildBox (iDocumentNode* child, csRef<iTriangleMesh>& trimesh);
      bool ParseTriMeshChildMesh (iDocumentNode* child, csRef<iTriangleMesh>& trimesh);

      // Process the attributes of an <imposter> tag in a mesh specification.
      bool ParseImposterSettings(iImposter* mesh, iDocumentNode *node);

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

}
CS_PLUGIN_NAMESPACE_END(csparser)

#endif // __CS_THREADED_LOADER_H__
