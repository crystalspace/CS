/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
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

#ifndef __CS_LOADER_CONTEXT_H__
#define __CS_LOADER_CONTEXT_H__

#include "csutil/scf_implementation.h"
#include "imap/ldrctxt.h"

#include "csthreadedloader.h"

struct iCollection;
struct iEngine;
struct iMissingLoaderData;

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  /*
  * Context class for the threaded loader.
  */
  class csLoaderContext : public scfImplementation1<csLoaderContext,
                                                    iLoaderContext>
  {
  private:
    iObjectRegistry* object_reg;
    iEngine* Engine;
    csThreadedLoader* loader;
    iCollection* collection;
    csRef<iMissingLoaderData> missingdata;
    uint keepFlags;
    bool do_verbose;
    csRef<iTextureManager> tm;
    csRef<iVFS> vfs;

public:
    struct NodeData
    {
      csRef<iDocumentNode> node;
      csString path;
    };

    // Pre-parse data.
    csArray<NodeData> availPlugins;
    csArray<NodeData> availShaders;
    csArray<NodeData> availTextures;
    csArray<NodeData> availMaterials;
    csArray<NodeData> availMeshfacts;
    CS::Threading::Mutex pluginObjects;
    CS::Threading::Mutex shaderObjects;
    CS::Threading::Mutex textureObjects;
    CS::Threading::Mutex materialObjects;
    CS::Threading::Mutex meshfactObjects;

    csLoaderContext(iObjectRegistry* object_reg, iEngine* Engine, csThreadedLoader* loader,
      iCollection* collection,iMissingLoaderData* missingdata, uint keepFlags, bool do_verbose);
    virtual ~csLoaderContext ();

    virtual iSector* FindSector (const char* name);
    virtual iMaterialWrapper* FindMaterial(const char* name, bool doLoad = true);
    virtual iMaterialWrapper* FindNamedMaterial(const char* name, const char *filename)
    {
      return FindMaterial(name);
    }
    virtual iMeshFactoryWrapper* FindMeshFactory(const char* name, bool notify = true);
    virtual iMeshWrapper* FindMeshObject(const char* name);
    virtual iTextureWrapper* FindTexture(const char* name, bool doLoad = true);
    virtual iTextureWrapper* FindNamedTexture(const char* name, const char *filename)
    {
      return FindTexture(name);
    }
    virtual iLight* FindLight(const char *name);
    virtual iShader* FindShader(const char *name);
    virtual bool CheckDupes() const { return true; }
    virtual iCollection* GetCollection() const { return collection; }
    virtual bool CurrentCollectionOnly() const { return false; }
    virtual uint GetKeepFlags() const { return keepFlags; }
    virtual void AddToCollection(iObject* obj);
    virtual bool GetVerbose() { return do_verbose; }

    void ReportNotify (const char* description, ...);
    void ReportWarning (const char* description, ...);

    // Pre-parse functions.
    void ParseAvailablePlugins(iDocumentNode* doc);
    void ParseAvailableTextures(iDocumentNode* doc);
    void ParseAvailableShaders(iDocumentNode* doc);
    void ParseAvailableMaterials(iDocumentNode* doc);
    void ParseAvailableAddons(iDocumentNode* doc);
    void ParseAvailableMeshfacts(iDocumentNode* doc);
  };
}
CS_PLUGIN_NAMESPACE_END(csparser)

#endif // __CS_LOADER_CONTEXT_H__
