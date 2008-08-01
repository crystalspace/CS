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
    bool searchCollectionOnly;
    csRef<iMissingLoaderData> missingdata;
    bool checkDupes;
    uint keepFlags;
    bool do_verbose;

  public:
    csLoaderContext(iObjectRegistry* object_reg, iEngine* Engine, csThreadedLoader* loader,
      iCollection* collection, bool searchCollectionOnly, bool checkDupes,
      iMissingLoaderData* missingdata, uint keepFlags, bool do_verbose);
    virtual ~csLoaderContext ();

    virtual iSector* FindSector (const char* name);
    virtual iMaterialWrapper* FindMaterial(const char* name);
    virtual iMaterialWrapper* FindNamedMaterial(const char* name,
                                                const char *filename);
    virtual iMeshFactoryWrapper* FindMeshFactory(const char* name);
    virtual iMeshWrapper* FindMeshObject(const char* name);
    virtual iTextureWrapper* FindTexture(const char* name);
    virtual iTextureWrapper* FindNamedTexture(const char* name,
                                              const char *filename);
    virtual iLight* FindLight(const char *name);
    virtual iShader* FindShader(const char *name);
    virtual bool CheckDupes() const { return checkDupes; }
    virtual iCollection* GetCollection() const { return collection; }
    virtual bool CurrentCollectionOnly() const { return searchCollectionOnly; }
    virtual uint GetKeepFlags() const { return keepFlags; }
    virtual void AddToCollection(iObject* obj);

    void ReportNotify (const char* description, ...);
  };
}
CS_PLUGIN_NAMESPACE_END(csparser)

#endif // __CS_LOADER_CONTEXT_H__
