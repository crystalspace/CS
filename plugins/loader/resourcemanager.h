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

#ifndef __CS_LOADER_RESOURCE_MANAGER_H__
#define __CS_LOADER_RESOURCE_MANAGER_H__

#include "csutil/resourcemapper.h"



#include "csutil/resource.h"

#include <csutil/hash.h>
#include <csutil/scf_implementation.h>
#include <csutil/threadmanager.h>

#include <iengine/engine.h>
#include <iengine/mesh.h>

#include <imap/resource.h>

#include <iutil/comp.h>
#include <iutil/event.h>
#include <iutil/eventh.h>
#include <iutil/eventq.h>

struct iVFS;
struct iLoader;
struct iJobQueue;

CS_PLUGIN_NAMESPACE_BEGIN(csloader)
{

class ResourceManager
  : public scfImplementation1<ResourceManager, iResourceManager>,
    public ThreadedCallable<ResourceManager>
{
public:
  ResourceManager (iBase* parent);
  virtual ~ResourceManager ();

  bool Initialize (iObjectRegistry* obj_reg);

  // ThreadedCallable
  virtual iObjectRegistry* GetObjectRegistry() const
  {
    return object_reg;
  }

  // iResourceManager
  virtual csPtr<iLoadingResource> Get (CS::Resource::TypeID type, const char* name);
  
  virtual void Add (iResource* resource) {}

  virtual void Remove (iResource* resource) {}

protected:
  void ProcessResources ();

  iLoadingResource* Load (CS::Resource::TypeID type, const char* name, csRef<iDocumentNode> node);

  THREADED_CALLABLE_DECL3(ResourceManager, LoadT, csThreadReturn, CS::Resource::TypeID,
    type, csRef<iDocumentNode>, node, iLoadingResource*, lresource, THREADED, false, false);

  // Loads a document file.
  csPtr<iDocument> LoadDocument (const char* fileName) const;

  // Initializes the set of loadable resources.
  bool InitLoadableResources ();

  // Reports an error.
  void ReportError (const char* description, ...) const;

private:
  iObjectRegistry* object_reg;
  csRef<iVFS> vfs;
  csRef<iDocumentSystem> docsys;

  CS::Threading::Mutex mutex;
  csRefArray<iLoadingResource> resourceQueue;

  // A cache if all loading resources.
  csRef<iResourceCache> resourceCache;

  // Maps node types to iResourceLoaders.
  CS::Resource::Mapper<iResourceLoader> loaderMapper;

  class TMLoadingResource : public scfImplementation1<TMLoadingResource, iLoadingResource>
  {
  public:
    TMLoadingResource (const char* name)
      : scfImplementationType(this), ready (false), name (name)
    {
    }

    virtual iResource* Get ()
    {
      future->Wait ();
      csRef<iResource> resource;
      resource = scfQueryInterface<iResource> (future->GetResultRefPtr ());
      return resource;
    }

    virtual const char* GetName ()
    {
      return name;
    }

    virtual bool Ready () 
    {
      if (future->IsFinished ())
      {
        csRef<iResource> resource = Get ();
        return resource->DependenciesSatisfied ();
      }

      return false;
    }

    virtual void AddListener (iResourceListener* listener)
    {
      if (Ready ())
        listener->OnLoaded (this);
      else
        listeners.Push (listener);
    }

    virtual void RemoveListener (iResourceListener* listener) 
    {
      listeners.Delete (listener);
    }

    virtual void TriggerCallback() 
    {
      for (size_t i = 0; i < listeners.GetSize (); i++)
        listeners.Get (i)->OnLoaded (this);

      listeners.DeleteAll ();
    }

    void SetFuture (iThreadReturn* fut)
    {
      future = fut;
    }

  private:
    bool ready;
    csString name;
    csRef<iThreadReturn> future;
    csRefArray<iResourceListener> listeners;
  };
};
}
CS_PLUGIN_NAMESPACE_END(csloader)

#endif // __CS_LOADER_RESOURCE_MANAGER_H__
