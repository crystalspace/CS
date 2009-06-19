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

#include "cssysdef.h"

#include "iengine/engine.h"
#include "iengine/material.h"
#include "imap/loader.h"
#include "iutil/object.h"
#include "ivaria/reporter.h"

#include "csloadercontext.h"

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  csLoaderContext::csLoaderContext (iObjectRegistry* object_reg, iEngine* Engine,
    csThreadedLoader* loader, iCollection* collection, iMissingLoaderData* missingdata,
    uint keepFlags, bool do_verbose)
    : scfImplementationType (this), object_reg(object_reg), Engine(Engine), loader(loader),
    collection(collection), missingdata(missingdata), keepFlags(keepFlags), do_verbose(do_verbose)
  {
  }

  csLoaderContext::~csLoaderContext ()
  {
  }

  iSector* csLoaderContext::FindSector(const char* name)
  {
    CS::Threading::MutexScopedLock lock(loader->sectorsLock);
    iSector* s = NULL;

    for(size_t i=0; i<loader->loaderSectors.GetSize(); i++)
    {
      if(!strcmp(loader->loaderSectors[i]->QueryObject()->GetName(), name))
      {
        s = loader->loaderSectors[i];
        break;
      }
    }
    
    if(!s && collection)
    {
      s = Engine->FindSector(name, collection);
    }

    if(!s)
    {
      s = Engine->FindSector(name);
    }

    if(!s && missingdata)
    {
      s = missingdata->MissingSector(name);
    }

    return s;
  }

  iMaterialWrapper* csLoaderContext::FindMaterial(const char* filename)
  {
    iMaterialWrapper* mat = NULL;

    while(loader->FindLoadingMaterial(filename))
    {
      csSleep(10);
    }

    loader->materialsLock.Lock();
    for(size_t i=0; i<loader->loaderMaterials.GetSize(); i++)
    {
      if(!strcmp(loader->loaderMaterials[i]->QueryObject()->GetName(), filename))
      {
        mat = loader->loaderMaterials[i];
        loader->materialsLock.Unlock();
        return mat;
      }
    }
    loader->materialsLock.Unlock();
    
    if(collection)
    {
      mat = Engine->FindMaterial(filename, collection);
    }

    if(!mat)
    {
      mat = Engine->FindMaterial(filename);
    }

    if(!mat && missingdata)
    {
      mat = missingdata->MissingMaterial(0, filename);
    }

    if(!mat && do_verbose)
    {
      ReportNotify("Could not find material '%s'.", filename);
    }

    return mat;
  }

  iMaterialWrapper* csLoaderContext::FindNamedMaterial(const char* name, const char *filename)
  {
    iMaterialWrapper* mat = NULL;

    while(loader->FindLoadingMaterial(filename))
    {
      csSleep(10);
    }

    loader->materialsLock.Lock();
    for(size_t i=0; i<loader->loaderMaterials.GetSize(); i++)
    {
      if(!strcmp(loader->loaderMaterials[i]->QueryObject()->GetName(), name))
      {
        mat = loader->loaderMaterials[i];
        loader->materialsLock.Unlock();
        return mat;
      }
    }
    loader->materialsLock.Unlock();
    
    if(!mat && collection)
    {
      mat = Engine->FindMaterial(filename, collection);
    }

    if(!mat)
    {
      mat = Engine->FindMaterial(filename);
    }

    if(!mat && missingdata)
    {
      mat = missingdata->MissingMaterial(name, filename);
    }

    if(!mat && do_verbose)
    {
      ReportNotify("Could not find material '%s' with filename '%s'.", name, filename);
    }

    return mat;
  }


  iMeshFactoryWrapper* csLoaderContext::FindMeshFactory(const char* name)
  {
    csRef<iMeshFactoryWrapper> fact;
    
    while(loader->FindLoadingMeshFact(name))
    {
      csSleep(10);
    }

    loader->meshfactsLock.Lock();
    for(size_t i=0; i<loader->loaderMeshFactories.GetSize(); i++)
    {
      if(!strcmp(loader->loaderMeshFactories[i]->QueryObject()->GetName(), name))
      {
        fact = loader->loaderMeshFactories[i];
        loader->meshfactsLock.Unlock();
        return fact;
      }
    }
    loader->meshfactsLock.Unlock();
    
    if(collection)
    {
      fact = Engine->FindMeshFactory(name, collection);
    }

    if(!fact)
    {
      fact = Engine->FindMeshFactory(name);
    }

    if(!fact && missingdata)
    {
      fact = missingdata->MissingFactory(name);
    }

    if(!fact && do_verbose)
    {
      ReportNotify("Could not find mesh factory '%s'.", name);
    }

    return fact;
  }

  iMeshWrapper* csLoaderContext::FindMeshObject(const char* name)
  {
    csRef<iMeshWrapper> mesh;

    csRef<iThreadReturn> itr = loader->loadingMeshObjects.Get(name, NULL);
    if(itr.IsValid())
    {
      itr->Wait();
      if(itr->WasSuccessful())
      {
        mesh = scfQueryInterface<iMeshWrapper>(itr->GetResultRefPtr());
        return mesh;
      }            
    }

    CS::Threading::MutexScopedLock lock(loader->meshesLock);
    for(size_t i=0; i<loader->loaderMeshes.GetSize(); i++)
    {
      if(!strcmp(loader->loaderMeshes[i]->QueryObject()->GetName(), name))
      {
        mesh = loader->loaderMeshes[i];
        return mesh;
      }
    }
    
    if(collection)
    {
      mesh = Engine->FindMeshObject(name, collection);
    }

    if(!mesh)
    {
      mesh = Engine->FindMeshObject(name);
    }

    if (!mesh && missingdata)
    {
      mesh = missingdata->MissingMesh(name);
    }

    if(!mesh && do_verbose)
    {
      ReportNotify("Could not find mesh object '%s'.", name);
    }

    return mesh;
  }

  iLight* csLoaderContext::FindLight(const char *name)
  {
    iLight *light = NULL;
    loader->sectorsLock.Lock();
    for (size_t i = 0; i < loader->loaderSectors.GetSize(); i++)
    {
      light = loader->loaderSectors[i]->GetLights ()->FindByName (name);
      if (light)
      {
        break;
      }
    }
    loader->sectorsLock.Unlock();

    if(!light && collection)
    {
      csRef<iLightIterator> li = Engine->GetLightIterator(collection);

      while(li->HasNext())
      {
        light = li->Next();
        if(!strcmp(light->QueryObject()->GetName(), name))
        {
          break;
        }
      }
    }

    if(!light)
    {
      csRef<iLightIterator> li = Engine->GetLightIterator();

      while(li->HasNext())
      {
        light = li->Next();
        if(!strcmp(light->QueryObject()->GetName(), name))
        {
          break;
        }
      }
    }

    if(!light && missingdata)
    {
      light = missingdata->MissingLight(name);
    }

    if(!light && do_verbose)
    {
      ReportNotify("Could not find light '%s'.", name);
    }

    return light;
  }

  iShader* csLoaderContext::FindShader(const char *name)
  {
    csRef<iShaderManager> shaderMgr = csQueryRegistry<iShaderManager>(object_reg);

    if(!shaderMgr)
    {
      return 0;
    }

    iShader* shader = 0;

    // Always look up builtin shaders globally
    if(!collection || (name && *name == '*'))
    {
      shader = shaderMgr->GetShader(name);
      if(!shader && missingdata)
      {
        shader = missingdata->MissingShader(name);
      }
    }
    else
    {
      csRefArray<iShader> shaders = shaderMgr->GetShaders();
      for(size_t i=0; i<shaders.GetSize(); i++)
      {
        shader = shaders[i];
        if(collection)
        {
          if((collection->IsParentOf(shader->QueryObject()) ||
            collection->FindShader(shader->QueryObject()->GetName())) &&
            !strcmp(name, shader->QueryObject()->GetName()))
          {
            break;
          }
        }
      }

      if(missingdata)
      {
        shader = missingdata->MissingShader(name);
      }
    }

    if(!shader && do_verbose)
    {
      ReportNotify("Could not find shader '%s'.", name);
    }

    return shader;
  }

  iTextureWrapper* csLoaderContext::FindTexture(const char* name)
  {
    iTextureWrapper* result = NULL;

    while(loader->FindLoadingTexture(name))
    {
      csSleep(10);
    }

    loader->texturesLock.Lock();
    for(size_t i=0; i<loader->loaderTextures.GetSize(); i++)
    {
      if(loader->loaderTextures[i]->QueryObject()->GetName() &&
        !strcmp(loader->loaderTextures[i]->QueryObject()->GetName(), name))
      {
        result = loader->loaderTextures[i];
        loader->texturesLock.Unlock();
        return result;
      }
    }
    loader->texturesLock.Unlock();

    if(!result && collection)
    {
      result = collection->FindTexture(name);
    }

    if(!result)
    {
      result = Engine->GetTextureList()->FindByName (name);
    }

    if(!result && missingdata)
    {
      result = missingdata->MissingTexture (name, 0);
    }

    if(!result && do_verbose)
    {
      ReportNotify ("Could not find texture '%s'. Attempting to load.", name);
    }

    return result;
  }

  iTextureWrapper* csLoaderContext::FindNamedTexture (const char* name,
    const char *filename)
  {
    iTextureWrapper* result = NULL;

    while(loader->FindLoadingTexture(name))
    {
      csSleep(10);
    }

    loader->texturesLock.Lock();
    for(size_t i=0; i<loader->loaderTextures.GetSize(); i++)
    {
      if(!strcmp(loader->loaderTextures[i]->QueryObject()->GetName(), name))
      {
        result = loader->loaderTextures[i];
        loader->texturesLock.Unlock();
        return result;
      }
    }
    loader->texturesLock.Unlock();

    if(!result && collection)
    {
      result = collection->FindTexture(name);
    }

    if(!result)
    {
      result = Engine->GetTextureList()->FindByName (name);
    }

    if (!result && missingdata)
    {
      result = missingdata->MissingTexture(name, filename);
    }

    if(!result && do_verbose)
    {
      ReportNotify ("Could not find texture '%s'. Attempting to load.", name);
    }

    return result;
  }

  void csLoaderContext::AddToCollection(iObject* obj)
  {
    if(collection)
    {
      CS::Threading::MutexScopedLock lock(collectionLock);
      collection->Add(obj);
    }
  }

  void csLoaderContext::ReportNotify (const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    csReportV (object_reg, CS_REPORTER_SEVERITY_NOTIFY, "crystalspace.maploader", description, arg);
    va_end (arg);
  }
}
CS_PLUGIN_NAMESPACE_END(csparser)
