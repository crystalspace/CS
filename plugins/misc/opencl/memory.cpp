#include "cssysdef.h"

#include "memory.h"
#include "event.h"
#include "context.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  void MappedMemory::HandleUnmap(iEvent*, void* data)
  {
    csRef<Event> e = static_cast<Event*>(data);
    e->DecRef(); // release the copy we retained earlier
    e->Fire(true);
  }

  MappedMemory::~MappedMemory()
  {
    csRef<iEvent> e = Release();
    if(e.IsValid())
    {
      // retain a copy to the unmap event so it'll
      // stay alive until the release is done
      done->IncRef();

      e->AddCallback(HandleUnmap, (void*)done);
    }
    else
    {
      done->Fire(true);
    }
  }

  csPtr<iEvent> MappedMemory::Release(const iEventList& events)
  {
    if(data == nullptr || !parent.IsValid())
    {
      // this object was released already
      return csPtr<iEvent>(nullptr);
    }
    else if(context == nullptr)
    {
      // direct map

      // invalidate this object
      data = nullptr;
      parent.Invalidate();

      // create event and fire it
      csRef<iEvent> e;
      e.AttachNew(new Event());
      e->Fire(true);

      // return event
      return csPtr<iEvent>(e);
    }
    else
    {
      // real map - unmap the memory

      // get a queue for the context
      csRef<Queue> q = context->GetQueue();

      // convert event list
      csRefArray<Event> eventList;
      if(!parent->BuildEventList(events, eventList, MEM_READ_WRITE))
      {
        return csPtr<iEvent>(nullptr);
      }

      // create event handle list
      cl_event* handleList = CreateEventHandleList(eventList, context);
      if(!eventList.IsEmpty() && handleList == nullptr)
      {
        // OOM
        return csPtr<iEvent>(nullptr);
      }

      // queue unmap memory
      cl_event eventHandle;
      cl_int error = clEnqueueUnmapMemObject(q->GetHandle(), parent->GetHandle(context),
                                             data, eventList.GetSize(), handleList,
                                             &eventHandle);
      delete [] handleList;

      // check for errors
      switch(error)
      {
      case CL_SUCCESS:
        break;

      default:
        return csPtr<iEvent>(nullptr);
      }

      // create event
      csRef<iEvent> e;
      e.AttachNew(new Event(context, eventHandle));

      // invalidate this object so there won't be further access to it
      parent.Invalidate();
      data = nullptr;

      // return event object to wait on
      return csPtr<iEvent>(e);
    }
  }

  cl_mem MemoryObject::GetHandle(Context* c)
  {
    CS::Threading::RecursiveMutexScopedLock lock(useLock);

    cl_mem obj;
    if(handles.Contains(c))
    {
      // we already have a handle for this context
      obj = *handles.GetElementPointer(c);
    }
    else
    {
      // create a new handle for this context
      obj = CreateHandle(c);
      handles.PutUnique(c, obj);
      status.PutUnique(c,false);
    }
    return obj;
  }

  bool MemoryObject::BuildEventList(const iEventList& events, csRefArray<Event>& eventList,
                                    int accessMode, MemoryObject* obj, int objAccessMode)
  {
    eventList = CreateEventList(events);
    if(eventList.GetSize() != events.GetSize())
    {
      // failed to creat event list
      return false;
    }

    if(accessMode & MEM_WRITE)
    {
      // wait for all reads to finish before performing a write
      for(size_t i = 0; i < reads.GetSize(); ++i)
      {
        csRef<Event> e = scfQueryInterface<Event>(reads[i]);
        eventList.Push(e);
      }
    }

    if(obj != nullptr && objAccessMode & MEM_WRITE)
    {
      // wait for all reads to finish before performing a write
      for(size_t i = 0; i < obj->reads.GetSize(); ++i)
      {
        csRef<Event> e = scfQueryInterface<Event>(obj->reads[i]);
        eventList.Push(e);
      }
    }

    if(accessMode & MEM_READ)
    {
      // wait for the last write to finish before performing a read
      if(lastWrite.IsValid())
      {
        csRef<Event> e = scfQueryInterface<Event>(lastWrite);
        eventList.Push(e);
      }
    }

    if(obj != nullptr && objAccessMode & MEM_READ)
    {
      // wait for the last write to finish before performing a read
      if(obj->lastWrite.IsValid())
      {
        csRef<Event> e = scfQueryInterface<Event>(obj->lastWrite);
        eventList.Push(e);
      }
    }

    return true;
  }

  void MemoryObject::Use(Context* c, Event* e, int accessType)
  {
    if(accessType & MEM_WRITE)
    {
      // writes to the buffer
      CS::Threading::RecursiveMutexScopedLock lock(useLock);

      lastWriteContext = c;
      lastWrite = e;

      // mark all other contexts outdated
      csHash<bool, csRef<Context> >::GlobalIterator it = status.GetIterator();
      while(it.HasNext())
      {
        csRef<Context> context;
        bool& status = it.Next(context);
        status = context == c;
      }
    }

    if(accessType & MEM_READ)
    {
      // reads from the buffer
      if(e != nullptr)
      {
        {
          CS::Threading::RecursiveMutexScopedLock lock(useLock);
          reads.Push(e);
        }

        // notify us when the read is done
        e->AddCallback(EventHandler, (void*)this);
      }
    }
  }

  void MemoryObject::EventHandler(iEvent* e, void* data)
  {
    MemoryObject* obj = static_cast<MemoryObject*>(data);

    CS::Threading::RecursiveMutexScopedLock lock(obj->useLock);
    obj->reads.Delete(e);
  }

  Context* MemoryObject::FindContext(MemoryObject* other, csRefArray<Event>& eventList, bool moveOther)
  {
    // try to find a context where both are up to date
    Context* c = nullptr;
    csHash<bool, csRef<Context> >::GlobalIterator it = status.GetIterator();
    while(it.HasNext())
    {
      csRef<Context> candidate;
      bool status = it.Next(candidate);
      if(status && other->status.Get(candidate, false))
      {
        c = candidate;
        break;
      }
    }

    if(c == nullptr)
    {
      // no context where both are valid found - use lastWriteContext of dst
      MemoryObject* toMove = this;
      if(moveOther)
      {
        c = lastWriteContext;
        toMove = other;
      }
      else
      {
        c = other->lastWriteContext;
      }

      iEventList list;
      for(size_t i = 0; i < eventList.GetSize(); ++i)
      {
        list.Push(eventList[i]);
      }

      csRef<Event> e = scfQueryInterfaceSafe<Event>(toMove->MoveTo(c, list));
      CS_ASSERT(e.IsValid());
      eventList.Push(e);
    }
    return c;
  }

  csRef<iEvent> MemoryObject::MoveTo(Context* c, const iEventList& origEvents)
  {
    csRef<iEvent> e;

    CS::Threading::RecursiveMutexScopedLock lock(useLock);
    if(status.Get(c,false))
    {
      // the handle is already up to date
      e.AttachNew(new Event(origEvents));
    }
    else
    {
      // the handle for this context isn't up-to-date
      iEventList events(origEvents);

      // find the most recent version
      void* src = data;
      csRef<iMappedMemory> map;
      if(status.Get(nullptr,false))
      {
        //@@@todo: use Read/Write instead of Request/Release
        // map the most recent buffer
        e = Request(events);
        CS_ASSERT(e.IsValid());

        // get the mapped memory
        map = scfQueryInterface<iMappedMemory>(e->GetResult());
        CS_ASSERT(map.IsValid());

        // set the src to the mapped data
        src = map->GetPointer();

        // let further operations wait for the map to finish
        events.Push(e);
      }

      e = Write(src, c, events);
      CS_ASSERT(e.IsValid());

      if(status.Get(nullptr,false))
      {
        // add the write to the dependency list
        events.Push(e);

        // unmap memory
        e = map->Release(events);
        CS_ASSERT(e.IsValid());
      }

      // mark this context up-to-date
      status.GetOrCreate(c,true) = true;
    }
    reads.Push(e);

    return e;
  }

  MemoryObject::~MemoryObject()
  {
    // free handles
    lastWriteContext = nullptr;
    Purge();

    if(data != nullptr)
    {
      // free host data
      cs_free(data);
      data = nullptr;
    }
  }

  void MemoryObject::Purge()
  {
    CS::Threading::RecursiveMutexScopedLock lock(useLock);

    // wait till all reads are done
    while(!reads.IsEmpty())
    {
      csRef<iEvent> e = reads.Pop(); // get a waiting read
      // the event callback may call our callback
      // from another thread, so unlock here
      useLock.Unlock();

      e->Wait(); // event done

      useLock.Lock(); // lock again
    }

    // wait till the last write is done
    if(lastWrite.IsValid())
    {
      lastWrite->Wait();
    }

    if(lastWriteContext != nullptr && data != nullptr)
    {
      // free host data
      cs_free(data);
      data = nullptr;
    }

    // free all references except the last written one
    csHash<cl_mem, csRef<Context> >::GlobalIterator it = handles.GetIterator();
    while(it.HasNext())
    {
      csRef<Context> c;
      cl_mem obj = it.Next(c);
      if(c != lastWriteContext)
      {
        cl_int error = clReleaseMemObject(obj);
        CS_ASSERT(error == CL_SUCCESS);
        handles.DeleteElement(it);
      }
    }
  }

  void Sampler::Purge()
  {
    // free all references as they're easy to re-create
    csHash<cl_sampler, csRef<Context> >::GlobalIterator it = handles.GetIterator();
    while(it.HasNext())
    {
      cl_int error = clReleaseSampler(it.Next());
      CS_ASSERT(error == CL_SUCCESS);
    }
    handles.DeleteAll();
  }

  cl_sampler Sampler::GetHandle(Context* c)
  {
    cl_sampler obj;
    if(handles.Contains(c))
    {
      // we already have a handle for this context
      obj = *handles.GetElementPointer(c);
    }
    else
    {
      // create a new handle for this context
      cl_int error;
      obj = clCreateSampler(c->GetHandle(), normalized, addressMode, filterMode, &error);
      handles.PutUnique(c, obj);
    }
    return obj;
  }
}
CS_PLUGIN_NAMESPACE_END(CL)

