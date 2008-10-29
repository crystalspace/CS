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
    engine = csQueryRegistry<iEngine>(object_reg);
    threadman = csQueryRegistry<iThreadManager>(object_reg);
    return (engine.IsValid() && loader.IsValid() && threadman.IsValid());
  }

  csPtr<iImage> csLoader::LoadImage (iDataBuffer* buf, int Format)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadImage(buf, Format);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    return scfQueryInterfaceSafe<iImage>(itr->GetResultRefPtr());
  }

  csPtr<iImage> csLoader::LoadImage (const char *fname, int Format)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadImage(fname, Format);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    return scfQueryInterfaceSafe<iImage>(itr->GetResultRefPtr());
  }

  csPtr<iTextureHandle> csLoader::LoadTexture (iDataBuffer* buf,
    int Flags, iTextureManager *tm , csRef<iImage>* img)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadTexture(buf, Flags, tm, img);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    return scfQueryInterfaceSafe<iTextureHandle>(itr->GetResultRefPtr());
  }

  iTextureWrapper* csLoader::LoadTexture (const char *name, iDataBuffer* buf,
    int Flags, iTextureManager *tm, bool reg, bool create_material, bool free_image)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadTexture(name, buf, Flags, tm, reg, create_material, free_image);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    csRef<iTextureWrapper> ret = scfQueryInterfaceSafe<iTextureWrapper>(itr->GetResultRefPtr());
    return ret;
  }

  csPtr<iTextureHandle> csLoader::LoadTexture (const char* fname,
    int Flags, iTextureManager *tm, csRef<iImage>* img)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadTexture(fname, Flags, tm, img);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    return scfQueryInterfaceSafe<iTextureHandle>(itr->GetResultRefPtr());
  }

  csPtr<iSndSysData> csLoader::LoadSoundSysData (const char *fname)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadSoundSysData(fname);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    return scfQueryInterfaceSafe<iSndSysData>(itr->GetResultRefPtr());
  }

  csPtr<iSndSysStream> csLoader::LoadSoundStream (const char *fname, int mode3d)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadSoundStream(fname, mode3d);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    return scfQueryInterfaceSafe<iSndSysStream>(itr->GetResultRefPtr());
  }

  iSndSysWrapper* csLoader::LoadSoundWrapper (const char *name, const char *fname)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadSoundWrapper(name, fname);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    csRef<iSndSysWrapper> ret = scfQueryInterfaceSafe<iSndSysWrapper>(itr->GetResultRefPtr());
    return ret;
  }

  csPtr<iMeshFactoryWrapper> csLoader::LoadMeshObjectFactory (const char* fname, iStreamSource* ssource)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadMeshObjectFactory(fname, ssource);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    return scfQueryInterfaceSafe<iMeshFactoryWrapper>(itr->GetResultRefPtr());
  }

  csPtr<iMeshWrapper> csLoader::LoadMeshObject (const char* fname, iStreamSource* ssource)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadMeshObject(fname, ssource);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    return scfQueryInterfaceSafe<iMeshWrapper>(itr->GetResultRefPtr());
  }

  csRef<iShader> csLoader::LoadShader (const char* filename, bool registerShader)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadShader(filename, registerShader);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    return scfQueryInterfaceSafe<iShader>(itr->GetResultRefPtr());
  }

  iTextureWrapper* csLoader::LoadTexture (const char *Name, const char *FileName, int Flags,
    iTextureManager *tm, bool reg, bool create_material, bool free_image,
    iCollection* collection, uint keepFlags)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadTexture(Name, FileName, Flags, tm, reg,
      create_material, free_image, collection, keepFlags);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    csRef<iTextureWrapper> ret = scfQueryInterfaceSafe<iTextureWrapper>(itr->GetResultRefPtr());
    return ret;
  }

  bool csLoader::LoadMapFile (const char* filename, bool clearEngine, iCollection* collection,
    bool searchCollectionOnly, bool checkDupes, iStreamSource* ssource, iMissingLoaderData* missingdata,
    uint keepFlags)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadMapFile(filename, clearEngine, collection,
      ssource, missingdata, keepFlags);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    return itr->WasSuccessful();
  }

  bool csLoader::LoadMap (iDocumentNode* world_node, bool clearEngine, iCollection* collection,
    bool searchCollectionOnly, bool checkDupes, iStreamSource* ssource, iMissingLoaderData* missingdata,
    uint keepFlags)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadMap(world_node, clearEngine, collection,
      ssource, missingdata, keepFlags);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    return itr->WasSuccessful();
  }

  bool csLoader::LoadLibraryFile (const char* filename, iCollection* collection, bool searchCollectionOnly,
    bool checkDupes, iStreamSource* ssource, iMissingLoaderData* missingdata, uint keepFlags)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadLibraryFile(filename, collection, ssource, missingdata, keepFlags);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    return itr->WasSuccessful();
  }

  bool csLoader::LoadLibrary (iDocumentNode* lib_node, iCollection* collection, bool searchCollectionOnly,
    bool checkDupes, iStreamSource* ssource, iMissingLoaderData* missingdata, uint keepFlags)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadLibrary(lib_node, collection, ssource, missingdata, keepFlags);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    return itr->WasSuccessful();
  }

  csLoadResult csLoader::Load (const char* fname, iCollection* collection, bool searchCollectionOnly,
    bool checkDupes, iStreamSource* ssource, const char* override_name, iMissingLoaderData* missingdata,
    uint keepFlags)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadFile(fname, collection, ssource, missingdata, keepFlags);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    csLoadResult ret;
    ret.success = itr->WasSuccessful();
    ret.result = itr->GetResultRefPtr();
    return ret;
  }

  csLoadResult csLoader::Load (iDataBuffer* buffer, iCollection* collection, bool searchCollectionOnly,
    bool checkDupes, iStreamSource* ssource, const char* override_name, iMissingLoaderData* missingdata,
    uint keepFlags)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadBuffer(buffer, collection, ssource, missingdata, keepFlags);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    csLoadResult ret;
    ret.success = itr->WasSuccessful();
    ret.result = itr->GetResultRefPtr();
    return ret;
  }

  csLoadResult csLoader::Load (iDocumentNode* node, iCollection* collection, bool searchCollectionOnly,
    bool checkDupes, iStreamSource* ssource, const char* override_name, iMissingLoaderData* missingdata,
    uint keepFlags)
  {
    bool arn = threadman->GetAlwaysRunNow();
    if(!arn)
    {
      threadman->SetAlwaysRunNow(true);
    }
    csRef<iThreadReturn> itr = loader->LoadNode(node, collection, ssource, missingdata, keepFlags);
    if(!arn)
    {
      threadman->SetAlwaysRunNow(false);
    }
    engine->SyncEngineListsNow(loader);
    csLoadResult ret;
    ret.success = itr->WasSuccessful();
    ret.result = itr->GetResultRefPtr();
    return ret;
  }
}
CS_PLUGIN_NAMESPACE_END(csparser)
