/*
  Copyright (C) 2011 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __CS_ASSIMPLOADER_H__
#define __CS_ASSIMPLOADER_H__

#include "csutil/refarr.h"
#include "imap/reader.h"
#include "imap/modelload.h"

struct iEngine;
struct iImageIO;
struct iMaterialWrapper;
struct iObjectRegistry;
struct iPluginManager;
struct iReporter;
struct iTextureWrapper;

class aiMaterial;
class aiMesh;
class aiTexture;

CS_PLUGIN_NAMESPACE_BEGIN(AssimpLoader)
{

/**
 * Open Asset Import Library loader for Crystal Space
 */
class AssimpLoader : 
  public scfImplementation3<AssimpLoader,
			    iBinaryLoaderPlugin,
			    iModelLoader,
			    iComponent>
{
private:
  iObjectRegistry* object_reg;

  csRef<iEngine> engine;
  csRef<iGraphics3D> g3d;
  csRef<iTextureManager> textureManager;
  csRef<iLoaderContext> loaderContext;
  csRef<iImageIO> imageLoader;

  csRefArray<iTextureWrapper> textures;
  csRefArray<iMaterialWrapper> materials;

  bool Load (iLoaderContext* loaderContext,
	     iGeneralFactoryState* gmstate, uint8* buffer, size_t size);
  iMeshFactoryWrapper* Load (const char* factname, const char* filename,
			     iDataBuffer* buffer);

public:
  /// Constructor.
  AssimpLoader (iBase*);

  /// Destructor.
  virtual ~AssimpLoader ();

  //-- iComponent
  virtual bool Initialize (iObjectRegistry *object_reg);

  // TODO: what?
  virtual bool IsThreadSafe() { return true; }

  //-- iBinaryLoaderPlugin
  virtual csPtr<iBase> Parse (iDataBuffer* buf, iStreamSource*,
    iLoaderContext* ldr_context, iBase* context, iStringArray*);

  //-- iModelLoader
  virtual iMeshFactoryWrapper* Load (const char* factname, const char* filename);
  virtual iMeshFactoryWrapper* Load (const char* factname, iDataBuffer* buffer);
  virtual bool IsRecognized (const char* filename);
  virtual bool IsRecognized (iDataBuffer* buffer);

 private:
  iTextureWrapper* FindTexture (const char* filename);
  iTextureWrapper* LoadTexture (iDataBuffer* buffer, const char* filename);

  void ImportTexture (aiTexture* texture, size_t index);
  void ImportMaterial (aiMaterial* material, size_t index);
  void ImportGenmesh (aiMesh* mesh);
};

}
CS_PLUGIN_NAMESPACE_END(AssimpLoader)

#endif // __CS_ASSIMPLOADER_H__
