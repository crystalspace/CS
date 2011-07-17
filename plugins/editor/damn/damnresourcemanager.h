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

#ifndef __DAMN_DAMNRESOURCEMANAGER_H__
#define __DAMN_DAMNRESOURCEMANAGER_H__

//#include "imap/resource.h"
#include "include/resource.h"

#include <csutil/scf_implementation.h>
#include <iutil/comp.h>
#include <csutil/hash.h>
#include <iengine/engine.h>
#include <iengine/mesh.h>

#include <iutil/event.h>
#include <iutil/eventh.h>
#include <iutil/eventq.h>

#include <imap/resource.h>

#include "include/wrapjob.h"
#include <functional>

#include "memcache.h"

namespace CS { namespace Network { namespace HTTP {
  struct iHTTPConnectionFactory;
}}}

struct iVFS;
struct iLoader;
struct iJobQueue;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{
class DAMNResourceManager : public scfImplementation4<DAMNResourceManager,iResourceManager,iFormatAbstractor, iEventHandler,iComponent>
{
public:
  DAMNResourceManager (iBase* parent);
  virtual ~DAMNResourceManager ();

  // iComponent
  virtual bool Initialize (iObjectRegistry* obj_reg);

  // iResourceManager
  virtual csRef<iLoading> Get (const char* name);
  
  // iFormatAbstractor
  virtual void AddAbstraction (const char* abstraction, const char* format);
  virtual const char* GetFormat (const char* abstraction) const;
  
  
  void ToBeProccessed (csRef<iLoading> res);
private:
  mutable CS::Threading::Mutex mutex_;
  bool HandleEvent(iEvent& ev);
  csRefArray<iLoading> toBeProccessed;

private:
  typedef std::map<std::string, std::string> Formats;
  Formats formats;
  
  
private:
  iObjectRegistry* object_reg;
  csRef<CS::Network::HTTP::iHTTPConnectionFactory> http;
  csRef<iVFS> vfs;
  //csRef<iLoader> loader;
  csRef<iResourceLoader> loader;
  csRef<iJobQueue> jobQueue;
  
  csRef<iResourceCache> cache;
  
  csRef<iResource> _Get (std::string id, std::string format);
  csRef<iDataBuffer> _GetData (std::string id, std::string format);

  /// The queue of events waiting to be handled.
  csRef<iEventQueue> eventQueue;
  /// The event name registry, used to convert event names to IDs and back.
  csRef<iEventNameRegistry> nameRegistry;

  CS_EVENTHANDLER_NAMES ("crystalspace.resources.managers.damn")
  CS_EVENTHANDLER_NIL_CONSTRAINTS

public: 
  void SplitName(const std::string& name, std::string& id, std::string& format); 
  std::string CleanFileName(const std::string& name);
  
  template<typename Callable>
  CS::Threading::Future<typename std::tr1::result_of<Callable()>::type> Queue(Callable function)
  {
    typedef CS::Threading::FunctorWrapperJob<typename std::tr1::result_of<Callable()>::type, Callable> Type;
    csRef<Type> job; job.AttachNew(new Type(function));
    jobQueue->Enqueue(job);
    return job->GetFuture();
  }
  
  template<typename Callable>
  struct FutureListener : public scfImplementation1<FutureListener<Callable> ,CS::Threading::iFutureListener>
  {
    FutureListener(Callable function) : scfImplementation1<FutureListener<Callable> ,CS::Threading::iFutureListener>(this), function(function) {}
    virtual void OnReady () { function(); }
    Callable function;
  };

  template<typename T, typename Callable>
  void Callback(CS::Threading::Future<T>& future, Callable function)
  {
    csRef<CS::Threading::iFutureListener> callback;
    callback.AttachNew(new FutureListener<Callable>(function));
    future.AddListener(callback);
  }
};
}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
