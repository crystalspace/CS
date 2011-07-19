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

CS_PLUGIN_NAMESPACE_BEGIN(csloader)
{

SCF_IMPLEMENT_FACTORY (ResourceManager)

ResourceManager::ResourceManager (iBase* parent)
: scfImplementationType (this, parent)
{
}

bool DAMNResourceManager::Initialize (iObjectRegistry* obj_reg)
{
  object_reg = obj_reg;
  
  csRef<iPluginManager> plugmgr = csQueryRegistry<iPluginManager> (object_reg);

  loader = csLoadPlugin<iResourceLoader> (plugmgr, "crystalspace.engine.persist");
  if (!loader) return false;
  
  vfs = csQueryRegistry<iVFS> (object_reg);
  if (!vfs) return false;
  
  static const char queueTag[] = "crystalspace.jobqueue.ResourceManager";
  jobQueue = csQueryRegistryTagInterface<iJobQueue> (object_reg, queueTag);
  if (!jobQueue.IsValid())
  {
    jobQueue.AttachNew (new CS::Threading::ThreadedJobQueue (4, CS::Threading::THREAD_PRIO_LOW, queueTag));
    object_reg->Register (jobQueue, queueTag);
  }
  
  eventQueue = csQueryRegistry<iEventQueue> (object_reg);
  if (!eventQueue) return false;

  nameRegistry = csEventNameRegistry::GetRegistry(object_reg);
  if (!nameRegistry) return false;

  eventQueue->RegisterListener(this);

  // Register for the Frame event, for Handle().
  eventQueue->RegisterListener (this, nameRegistry->GetID("crystalspace.frame"));
  
  cache.AttachNew(new CS::Resource::MemoryCache());
 
  return true;
}

DAMNResourceManager::~DAMNResourceManager ()
{
}

bool DAMNResourceManager::HandleEvent(iEvent& ev)
{
  CS::Threading::MutexScopedLock lock(mutex_);
  for (size_t i = 0; i < toBeProccessed.GetSize(); i++)
  {
    csRef<iLoadingResource> l = toBeProccessed.Pop();
    csRef<iResource> res = l->Get();
    //if (res->GetDependencies().GetSize() == 0);
    iResourceTrigger(l);  //No dependencies, trigger the resource as ready.
  }
  return true;
} 

void DAMNResourceManager::ToBeProccessed (csRef<iLoadingResource> res)
{
  CS::Threading::MutexScopedLock lock(mutex_);
  toBeProccessed.Push(res);
}

csRef<iLoadingResource> DAMNResourceManager::Get (CS::Resource::TypeID type, const char* name)
{
  csRef<iLoadingResource> res = cache->Get(type, name);
  if (!res.IsValid())
  {
    using namespace CS::Threading;
    using namespace std::tr1;
    
    std::string id;
    std::string format;
    SplitName(name, id, format);
    format = GetFormat(format.c_str());
    Future<csRef<iResource> > job = Queue(bind(&DAMNResourceManager::_Get, this, type, id, format));
    
    res.AttachNew(new CS::Resource::LoadingResource(job));
    
    cache->Add(type, name, res);
    
    csRef<iLoadingResource> t = res;
    Callback(job, bind(&DAMNResourceManager::ToBeProccessed, this, t));
  }

  return res;
}

}
CS_PLUGIN_NAMESPACE_END(csloader)
