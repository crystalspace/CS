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

#include "inetwork/http.h"

#include <csutil/databuf.h>
#include <igraphic/image.h>
#include <iutil/vfs.h>

#include <iutil/cfgmgr.h>

#include <csutil/csevent.h>
#include <csutil/resource.h>

#include "json/json.h"

#include <iostream>
#include <sstream>
#include <string>

#include "damnresourcemanager.h"



CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{
  
SCF_IMPLEMENT_FACTORY (DAMNResourceManager)

DAMNResourceManager::DAMNResourceManager (iBase* parent) : scfImplementationType (this, parent)
{
}

bool DAMNResourceManager::Initialize (iObjectRegistry* obj_reg)
{
  using namespace CS::Network::HTTP;
  
  object_reg = obj_reg;
  
  csRef<iPluginManager> plugmgr = csQueryRegistry<iPluginManager> (object_reg);
  http = csLoadPlugin<iHTTPConnectionFactory> (plugmgr, "crystalspace.network.factory.http");
  if (!http) return false;
  
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

  //Register for the Frame event, for Handle().
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


csRef<iResource> DAMNResourceManager::_Get (CS::Resource::TypeID type, std::string id, std::string format)
{
  csRef<iDataBuffer> data = _GetData(id, format);
  csRef<iResource> image;
  if (data)
    image = loader->Load (type, data);
  return image;
}

bool GetETag(const std::string& content, std::string& eTag)
{
  size_t begin = content.find("eTag: ");
  if (begin != std::string::npos) 
  {
    begin += 6;
    size_t end = content.find('\r', begin);
    if (end != std::string::npos) 
    {
      eTag = content.substr(begin, end-begin);
      return true;
    }
  }
  return false;
}

csRef<iDataBuffer> DAMNResourceManager::_GetData (std::string id, std::string format)
{
  using namespace CS::Network::HTTP;
  
  std::string fileName = "/tmp/assets/"+id+"::"+CleanFileName(format);
  csRef<iStringArray> headers;
  // Check if a meta file exists that contains our eTag.
  if (vfs->Exists((fileName+".meta").c_str()))
  {
    csRef<iDataBuffer> data = vfs->	ReadFile((fileName+".meta").c_str());
    std::string content(data->GetData (), data->GetSize());
    std::string eTag;
    if (GetETag(content, eTag))
    {
      headers.AttachNew(new scfStringArray());
      headers->Push(("If-None-Match: "+eTag).c_str());
    }
  }
  
  csRef<iHTTPConnection> client = http->Create("http://damn.peragro.org/");
  csRef<iResponse> response = client->Get(std::string("assets/"+id+"/transcode/").c_str(), format.c_str(), headers);
  if (response && response->GetState() == OK)
  {
    if (response->GetCode() == 200)
    {
      vfs->WriteFile (fileName.c_str(), response->GetData()->GetData (), response->GetData()->GetSize());
      vfs->WriteFile ((fileName+".meta").c_str(), response->GetHeader()->GetData (), response->GetHeader()->GetSize());

      return response->GetData();
    }
    else if (response->GetCode() == 304)
    {
      //printf("NOT MODIFIED %d\n", response->GetCode());
      csRef<iDataBuffer> data = vfs->	ReadFile(fileName.c_str());
      return data;
    }
  }
  else if (response)
  {
    printf("FAILED %d %d\n", response->GetState(), response->GetCode());
  }
  return csRef<iDataBuffer> ();
}

void FindAndReplace(std::string& source, const char* find, const char* replace)
{
   size_t findLen = strlen(find);
   size_t replaceLen = strlen(replace);
   size_t pos = 0;

   while ((pos = source.find(find, pos)) != std::string::npos)
   {
      source.replace(pos, findLen, replace);
      pos += replaceLen;
   }
}


std::string DAMNResourceManager::CleanFileName(const std::string& name)
{
  std::string clean(name);
  FindAndReplace(clean, "/", "__");
  FindAndReplace(clean, "\\", "__");
  return clean;
}

void DAMNResourceManager::SplitName(const std::string& name, std::string& id, std::string& format)
{
  size_t pos = name.find("::");
  if (pos != std::string::npos)
  {
    id = name.substr(0, pos);
    if (pos+2 < name.size())
      format = name.substr(pos+2, name.size());
  }
}

void DAMNResourceManager::AddAbstraction (const char* type, const char* format)
{
  formats[type] = format;
}

const char* DAMNResourceManager::GetFormat (const char* type) const
{
  Formats::const_iterator found = formats.find(type);
  if (found != formats.end()) return found->second.c_str();
  return 0;
}

}
CS_PLUGIN_NAMESPACE_END(CSE)
