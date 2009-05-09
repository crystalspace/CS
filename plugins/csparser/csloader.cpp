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

#include "cssysdef.h"

#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "iengine/texture.h"
#include "isndsys/ss_manager.h"
#include "iutil/databuff.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "ivideo/shader/shader.h"

#include "csloader.h"

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  SCF_IMPLEMENT_FACTORY(csLoader)

  csLoader::csLoader(iBase *p) : scfImplementationType (this, p)
  {
  }

  csLoader::~csLoader()
  {
  }

  bool csLoader::Initialize(iObjectRegistry *object_reg)
  {
    loader = csQueryRegistryOrLoad<iThreadedLoader>(object_reg, "crystalspace.level.threadedloader");
    vfs = csQueryRegistry<iVFS>(object_reg);
    return loader.IsValid();
  }

  csPtr<iImage> csLoader::LoadImage (iDataBuffer* buf, int Format)
  {
    csRef<iThreadReturn> itr = loader->LoadImageWait(vfs->GetCwd(), buf, Format);
    return scfQueryInterfaceSafe<iImage>(itr->GetResultRefPtr());
  }

  csPtr<iImage> csLoader::LoadImage (const char *fname, int Format)
  {
    csRef<iThreadReturn> itr = loader->LoadImageWait(vfs->GetCwd(), fname, Format);
    return scfQueryInterfaceSafe<iImage>(itr->GetResultRefPtr());
  }

  csPtr<iTextureHandle> csLoader::LoadTexture (iDataBuffer* buf,
    int Flags, iTextureManager *tm , csRef<iImage>* img)
  {
    csRef<iThreadReturn> itr = loader->LoadTextureWait(vfs->GetCwd(), buf, Flags, tm, img);
    return scfQueryInterfaceSafe<iTextureHandle>(itr->GetResultRefPtr());
  }

  iTextureWrapper* csLoader::LoadTexture (const char *name, iDataBuffer* buf,
    int Flags, iTextureManager *tm, bool reg, bool create_material, bool free_image)
  {
    csRef<iThreadReturn> itr = loader->LoadTextureWait(vfs->GetCwd(), name, buf, Flags, tm, reg, create_material, free_image);
    csRef<iTextureWrapper> ret = scfQueryInterfaceSafe<iTextureWrapper>(itr->GetResultRefPtr());
    return ret;
  }

  csPtr<iTextureHandle> csLoader::LoadTexture (const char* fname,
    int Flags, iTextureManager *tm, csRef<iImage>* img)
  {
    csRef<iThreadReturn> itr = loader->LoadTextureWait(vfs->GetCwd(), fname, Flags, tm, img);
    return scfQueryInterfaceSafe<iTextureHandle>(itr->GetResultRefPtr());
  }

  csPtr<iSndSysData> csLoader::LoadSoundSysData (const char *fname)
  {
    csRef<iThreadReturn> itr = loader->LoadSoundSysDataWait(vfs->GetCwd(), fname);
    return scfQueryInterfaceSafe<iSndSysData>(itr->GetResultRefPtr());
  }

  csPtr<iSndSysStream> csLoader::LoadSoundStream (const char *fname, int mode3d)
  {
    csRef<iThreadReturn> itr = loader->LoadSoundStreamWait(vfs->GetCwd(), fname, mode3d);
    return scfQueryInterfaceSafe<iSndSysStream>(itr->GetResultRefPtr());
  }

  iSndSysWrapper* csLoader::LoadSoundWrapper (const char *name, const char *fname)
  {
    csRef<iThreadReturn> itr = loader->LoadSoundWrapperWait(vfs->GetCwd(), name, fname);
    csRef<iSndSysWrapper> ret = scfQueryInterfaceSafe<iSndSysWrapper>(itr->GetResultRefPtr());
    return ret;
  }

  csPtr<iMeshFactoryWrapper> csLoader::LoadMeshObjectFactory (const char* fname, iStreamSource* ssource)
  {
    csRef<iThreadReturn> itr = loader->LoadMeshObjectFactoryWait(vfs->GetCwd(), fname, ssource);
    return scfQueryInterfaceSafe<iMeshFactoryWrapper>(itr->GetResultRefPtr());
  }

  csPtr<iMeshWrapper> csLoader::LoadMeshObject (const char* fname, iStreamSource* ssource)
  {
    csRef<iThreadReturn> itr = loader->LoadMeshObjectWait(vfs->GetCwd(), fname, ssource);
    return scfQueryInterfaceSafe<iMeshWrapper>(itr->GetResultRefPtr());
  }

  csRef<iShader> csLoader::LoadShader (const char* filename, bool registerShader)
  {
    csRef<iThreadReturn> itr = loader->LoadShaderWait(vfs->GetCwd(), filename, registerShader);
    return scfQueryInterfaceSafe<iShader>(itr->GetResultRefPtr());
  }

  iTextureWrapper* csLoader::LoadTexture (const char *Name, const char *FileName, int Flags,
    iTextureManager *tm, bool reg, bool create_material, bool free_image,
    iCollection* collection, uint keepFlags)
  {
    csRef<iThreadReturn> itr = loader->LoadTextureWait(vfs->GetCwd(), Name, FileName, Flags, tm, reg,
      create_material, free_image, collection, keepFlags);
    csRef<iTextureWrapper> ret = scfQueryInterfaceSafe<iTextureWrapper>(itr->GetResultRefPtr());
    return ret;
  }

  bool csLoader::LoadMapFile (const char* filename, bool clearEngine, iCollection* collection,
    bool searchCollectionOnly, bool checkDupes, iStreamSource* ssource, iMissingLoaderData* missingdata,
    uint keepFlags)
  {
    csRef<iThreadReturn> itr = loader->LoadMapFileWait(vfs->GetCwd(), filename, clearEngine, collection,
      ssource, missingdata, keepFlags);
    return itr->WasSuccessful();
  }

  bool csLoader::LoadMap (iDocumentNode* world_node, bool clearEngine, iCollection* collection,
    bool searchCollectionOnly, bool checkDupes, iStreamSource* ssource, iMissingLoaderData* missingdata,
    uint keepFlags)
  {
    csRef<iThreadReturn> itr = loader->LoadMapWait(vfs->GetCwd(), world_node, clearEngine, collection,
      ssource, missingdata, keepFlags);
    return itr->WasSuccessful();
  }

  bool csLoader::LoadLibraryFile (const char* filename, iCollection* collection, bool searchCollectionOnly,
    bool checkDupes, iStreamSource* ssource, iMissingLoaderData* missingdata, uint keepFlags)
  {
    csRef<iThreadReturn> itr = loader->LoadLibraryFileWait(vfs->GetCwd(), filename, collection, ssource, missingdata, keepFlags);
    return itr->WasSuccessful();
  }

  bool csLoader::LoadLibrary (iDocumentNode* lib_node, iCollection* collection, bool searchCollectionOnly,
    bool checkDupes, iStreamSource* ssource, iMissingLoaderData* missingdata, uint keepFlags)
  {
    csRef<iThreadReturn> itr = loader->LoadLibraryWait(vfs->GetCwd(), lib_node, collection, ssource, missingdata, keepFlags);
    return itr->WasSuccessful();
  }

  csLoadResult csLoader::Load (const char* fname, iCollection* collection, bool searchCollectionOnly,
    bool checkDupes, iStreamSource* ssource, const char* override_name, iMissingLoaderData* missingdata,
    uint keepFlags)
  {
    csRef<iThreadReturn> itr = loader->LoadFileWait(vfs->GetCwd(), fname, collection, ssource, missingdata, keepFlags);
    csLoadResult ret;
    ret.success = itr->WasSuccessful();
    ret.result = itr->GetResultRefPtr();
    return ret;
  }

  csLoadResult csLoader::Load (iDataBuffer* buffer, iCollection* collection, bool searchCollectionOnly,
    bool checkDupes, iStreamSource* ssource, const char* override_name, iMissingLoaderData* missingdata,
    uint keepFlags)
  {
    csRef<iThreadReturn> itr = loader->LoadBufferWait(vfs->GetCwd(), buffer, collection, ssource, missingdata, keepFlags);
    csLoadResult ret;
    ret.success = itr->WasSuccessful();
    ret.result = itr->GetResultRefPtr();
    return ret;
  }

  csLoadResult csLoader::Load (iDocumentNode* node, iCollection* collection, bool searchCollectionOnly,
    bool checkDupes, iStreamSource* ssource, const char* override_name, iMissingLoaderData* missingdata,
    uint keepFlags)
  {
    csRef<iThreadReturn> itr = loader->LoadNodeWait(vfs->GetCwd(), node, collection, ssource, missingdata, keepFlags);
    csLoadResult ret;
    ret.success = itr->WasSuccessful();
    ret.result = itr->GetResultRefPtr();
    return ret;
  }
}
CS_PLUGIN_NAMESPACE_END(csparser)
