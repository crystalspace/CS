/*
    Copyright (C) 2011 by Jelle Hellemans

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
#include "csutil/scf.h"
#include "csutil/sysfunc.h"

#include "iengine/sector.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivideo/material.h"

#include "csutil/memcache.h"
#include "csutil/threadjobqueue.h"
#include "csutil/scfstringarray.h"

#include <csutil/databuf.h>
#include <igraphic/image.h>
#include <iutil/vfs.h>

#include <iutil/cfgmgr.h>

#include <csutil/csevent.h>
#include <csutil/resource.h>

#include "resourcemanager.h"

using namespace CS::Resource;

CS_PLUGIN_NAMESPACE_BEGIN(csloader)
{

ResourceManager::ResourceManager (iBase* parent)
: scfImplementationType (this, parent)
{
}

ResourceManager::~ResourceManager ()
{
}

bool ResourceManager::Initialize (iObjectRegistry* obj_reg)
{
  object_reg = obj_reg;

  vfs = csQueryRegistry<iVFS> (object_reg);
  if (!vfs)
  {
    ReportError ("No VFS loaded!\n");
    return false;
  }

  docsys = csQueryRegistry<iDocumentSystem> (object_reg);
  if (!docsys)
  {
    ReportError ("No document system loaded!\n");
    return false;
  }

  resourceCache.AttachNew (new MemoryCache());

  InitLoadableResources ();
 
  return true;
}

csPtr<iLoadingResource> ResourceManager::Get (TypeID type, const char* name)
{
  return csPtr<iLoadingResource> (resourceCache->Get(type, name));
}

void ResourceManager::ProcessResources ()
{
  CS::Threading::MutexScopedLock lock (mutex);

  for (size_t i = 0; i < resourceQueue.GetSize(); i++)
  {
    resourceQueue.Pop ()->TriggerCallback ();
  }
}

iLoadingResource* ResourceManager::Load (TypeID type, const char* name, csRef<iDocumentNode> node)
{
  csRef<TMLoadingResource> resource;
  resource.AttachNew (new TMLoadingResource (name));

  resource->SetFuture (LoadT (type, node, resource));

  CS::Threading::MutexScopedLock lock (mutex);
  resourceCache->Add (type, name, resource);

  return resource;
}

THREADED_CALLABLE_IMPL3(ResourceManager, LoadT, TypeID type,
                        csRef<iDocumentNode> node, iLoadingResource* lresource)
{
  iResourceLoader* loader = loaderMapper.GetLoadable (type);

  if (!loader)
  {
    ReportError ("No known loader for resource '%s'!", lresource->GetName ());
    return false;
  }

  csRef<iResource> resource (loader->Load (node));
  ret->SetResult (csRef<iBase> (resource));

  // Satisfy dependencies.
  bool satisfied = resource->DependenciesSatisfied ();
  while (!satisfied)
  {
    satisfied = true;
    const csArray<ResourceReference>& deps = resource->GetDependencies ();

    for (size_t i = 0; i < deps.GetSize (); ++i)
    {
      iLoadingResource* dep = resourceCache->Get (deps[i].typeID, deps[i].id);
      if (dep)
      {
        resource->SetProperty (deps[i].property, dep->Get ());
      }
      else if (deps[i].node)
      {
        satisfied = false;

        Load (deps[i].typeID, deps[i].id, deps[i].node);
      }
      else
      {
        ReportError ("Resource '%s' has missing dependency '%s'!",
          lresource->GetName (), deps[i].id.GetData ());
        return false;
      }
    }
  }

  CS::Threading::MutexScopedLock lock (mutex);
  resourceQueue.Push (lresource);

  return true;
}

csPtr<iDocument> ResourceManager::LoadDocument (const char* fileName) const
{
  // Load the file as a document.
  csRef<iDataBuffer> dataBuffer = vfs->ReadFile (fileName, false);
  if (!dataBuffer.IsValid())
  {
    ReportError ("Could not open file %s!\n", CS::Quote::Single (fileName));
    return 0;
  }

  csRef<iDocument> doc = docsys->CreateDocument ();
  const char* error = doc->Parse (dataBuffer);
  if (error != 0)
  {
    ReportError ("Error parsing file %s: %s!\n", CS::Quote::Single (fileName), error);
    return 0;
  }

  if (!doc->GetRoot ())
  {
    ReportError ("Invalid document file %s!\n", CS::Quote::Single (fileName));
    return 0;
  }

  return csPtr<iDocument> (doc);
}

bool ResourceManager::InitLoadableResources ()
{
  // Init the map of resource loaders.
  loaderMapper.Initialize (object_reg);

  // Load the standard set of name -> loadable mappings
  const char* standardResourceHandlers = "/config/standard-resourcehandlers.xml";
  csRef<iDocument> doc = LoadDocument (standardResourceHandlers);
  if (!doc.IsValid ()) return false;

  csRef<iDocumentNode> resourceHandlersNode = doc->GetRoot ()->GetNode ("resourcehandlers");
  if (!resourceHandlersNode.IsValid ())
  {
    ReportError ("Invalid standard resources file!\n");
    return false;
  }

  return loaderMapper.Register (resourceHandlersNode);
}

void ResourceManager::ReportError (const char* description, ...) const
{
  va_list arg;
  va_start (arg, description);
  csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR,
    "crystalspace.loader.resourcemanager", description, arg);
  va_end (arg);
}

}
CS_PLUGIN_NAMESPACE_END(csloader)
