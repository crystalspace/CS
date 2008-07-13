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

#include "csutil/threadmanager.h"
#include "csutil/scf_implementation.h"
#include "csutil/strhash.h"
#include "csutil/weakrefhash.h"
#include "iutil/comp.h"
#include "imap/loader.h"

#include "ldrplug.h"

class csReversibleTransform;
struct iCollection;
struct iDocumentNode;
struct iEngine;
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
  class csThreadedLoader : public ThreadedCallable<csThreadedLoader>,
                           public scfImplementation2<csThreadedLoader,
                                                     iThreadedLoader,
                                                     iComponent>
  {
  public:
    csThreadedLoader(iBase *p);
    virtual ~csThreadedLoader();

    virtual bool Initialize(iObjectRegistry *object_reg);

    iObjectRegistry* GetObjectRegistry() { return object_reg; }

    THREADED_CALLABLE_DECL10(csThreadedLoader, LoadNode, csRef<iDocumentNode>, node,
      csRef<csLoadResult>, loadResult, csRef<iCollection>, collection, bool, searchCollectionOnly,
      bool, checkDupes, csRef<iStreamSource>, ssource, const char*, override_name,
      csRef<iMissingLoaderData>, missingdata, uint, keepFlags, bool, do_verbose)

  private:
      csRef<iEngine> Engine;
      csRef<iVFS> vfs;
      // Parser for common stuff like MixModes, vectors, matrices, ...
      csRef<iSyntaxService> SyntaxService;
      iObjectRegistry *object_reg;
      /// Shared string set
      csRef<iStringSet> stringSet;
      csRef<iShaderVarStringSet> stringSetSvName;

      iMeshFactoryWrapper* LoadMeshFactory(iLoaderContext* ldr_context, csLoadResult* loadResult,
        iDocumentNode* meshfactnode, const char* override_name = 0,
        iMeshFactoryWrapper* parent = 0, csReversibleTransform* transf = 0,
        iStreamSource* ssource = 0);

      /**
      * Load a Mesh Object Factory from the map file.
      * If the transformation pointer is given then this is for a hierarchical
      * mesh object factory and the transformation will be filled in with
      * the relative transform (from MOVE keyword).
      * parent is not 0 if the factory is part of a hierarchical factory.
      */
      bool LoadMeshObjectFactory(iLoaderContext* ldr_context, iMeshFactoryWrapper* meshFact,
        csLoadResult* loadResult, iMeshFactoryWrapper* parent, iDocumentNode* node,
        csReversibleTransform* transf = 0, iStreamSource* ssource = 0);

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

      /**
      * Parse a key/value pair.
      * Takes "editoronly" attribute into account: KVPs should only be parsed 
      * if they're not editor-only or when the engine is in "saveable" mode.
      */
      bool ParseKey (iDocumentNode* node, iObject* obj);

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

      // Locks on object type.
      CS::Threading::RecursiveMutex textureLock;
      CS::Threading::RecursiveMutex materialLock;
      CS::Threading::RecursiveMutex meshWrapperLock;
      CS::Threading::RecursiveMutex meshFactoryWrapperLock;    

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
