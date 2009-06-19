/*
    Copyright (C) 2008 by Frank Richter

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

#ifndef __LOADERCONTEXT_H__
#define __LOADERCONTEXT_H__

#include "imap/ldrctxt.h"

#include "csutil/scf_implementation.h"

struct iLoader;
struct iTextureManager;

CS_PLUGIN_NAMESPACE_BEGIN(ShaderManager)
{
  /**
   * Context class for the standard loader.
   */
  class LoaderContext :
    public scfImplementation1<LoaderContext,
                              iLoaderContext>
  {
  private:
    iLoader* loader;
    iTextureManager* tm;
    
    void RegisterTexture (iTextureWrapper* tex);
  public:
    LoaderContext (iLoader* loader, iTextureManager* tm);

    iSector* FindSector (const char* name) { return 0; }
    iMaterialWrapper* FindMaterial (const char* name) { return 0; }
    iMaterialWrapper* FindNamedMaterial (const char* name,
      const char *filename) { return 0; }
    iMeshFactoryWrapper* FindMeshFactory (const char* name) { return 0; }
    iMeshWrapper* FindMeshObject (const char* name) { return 0; }
    iTextureWrapper* FindTexture (const char* name);
    iTextureWrapper* FindNamedTexture (const char* name,
      const char *filename);
    iLight* FindLight (const char *name) { return 0; }
    iShader* FindShader (const char *name) { return 0; }
    bool CheckDupes () const { return false; }
    iCollection* GetCollection() const { return 0; }
    bool CurrentCollectionOnly() const { return false; }
    uint GetKeepFlags() const { return 0; }
    void AddToCollection(iObject* obj) { return; }
    bool GetVerbose() { return false; }
  };

}
CS_PLUGIN_NAMESPACE_END(ShaderManager)

#endif // __LOADERCONTEXT_H__
