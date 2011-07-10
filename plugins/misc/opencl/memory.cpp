#include "memory.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  static void HandleUnmap(iEvent*, void* data)
  {
    csRef<Event> e = static_cast<Event*>(data);
    e->DecRef(); // release the copy we retained earlier
    e->Fire(true);
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
      if(!BuildEventList(events, eventList))
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
    CS::Threading::ScopedLock lock(useLock);

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
        eventList.Push(scfQueryInterface<Event>(reads[i]));
      }
    }

    if(obj != nullptr && objAccessMode & MEM_WRITE)
    {
      // wait for all reads to finish before performing a write
      for(size_t i = 0; i < obj->reads.GetSize(); ++i)
      {
        eventList.Push(scfQueryInterface<Event>(obj->reads[i]));
      }
    }

    if(accessMode & MEM_READ)
    {
      // wait for the last write to finish before performing a read
      if(lastWrite.IsValid())
      {
        eventList.Push(scfQueryInterface<Event>(lastWrite));
      }
    }

    if(obj != nullptr && objAccessMode & MEM_READ)
    {
      // wait for the last write to finish before performing a read
      if(obj->lastWrite.IsValid())
      {
        eventList.Push(scfQueryInterface<Event>(obj->lastWrite));
      }
    }

    return true;
  }

  void MemoryObject::Use(Context* c, Event* e, int accessType)
  {
    if(accessType & MEM_WRITE)
    {
      // writes to the buffer
      CS::Threading::ScopedLock lock(useLock)

      lastWriteContext = c;
      lastWrite = e;

      // mark all other contexts outdated
      csHash<csRef<Context>, bool>::Iterator it = status.GetIterator();
      while(it.HasNext())
      {
        Context* context;
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
          CS::Threading::ScopedLock lock(useLock);
          reads.Push(e);
        }

        // notify us when the read is done
        e->AddCallback(EventHandler, (void*)this);
      }
    }
  }

  static void MemoryObject::EventHandler(iEvent* e, void* data)
  {
    MemoryObject* obj = static_cast<MemoryObject*>(data);

    CS::Threading::ScopedLock lock(readLock);
    obj->Reads.Delete(e);
  }

  Context* FindContext(MemoryObject* other, csRefArray<Event>& eventList, bool moveOther)
  {
    // try to find a context where both are up to date
    Context* c = nullptr;
    csRef<csRef<Context>,bool>::Iterator it = status.GetIterator();
    while(it.HasNext())
    {
      Context* candidate;
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

      csRef<Event> e = scfQueryInterfaceSafe(toMove->MoveTo(c, eventList));
      CS_ASSERT(e.IsValid());
      eventList.Push(e);
    }
    return c;
  }

  csRef<iEvent> MemoryObject::MoveTo(Context* c, const iEventList& events)
  {
    csRefArray<Event> eventList;
    if(!BuildEventList(events, eventList, MEM_READ))
    {
      // failed to build dependency list
      return csRef<iEvent>(nullptr);
    }

    csRef<iEvent> e;

    CS::Threading::ScopedLock lock(useLock);
    if(status.Get(c,false))
    {
      // the handle is already up to date
      e.AttachNew(new Event(eventList));
    }
    else
    {
      // the handle for this context isn't up-to-date

      // find the most recent version
      void* src = data;
      csRef<iMappedMemory> map;
      if(lastContext != nullptr)
      {
        // map the most recent buffer
        e = Request(eventList);
        CS_ASSERT(e.IsValid());

        // get the mapped memory
        map = scfQueryInterface<iMappedMemory>(e->GetResult());
        CS_ASSERT(map.IsValid());

        // set the src to the mapped data
        src = map->GetPointer();

        // let further operations wait for the map to finish
        eventList.Push(e);
      }

      // write the data to the target buffer
      e = Write(src, context, eventList);
      CS_ASSERT(e.IsValid());

      if(lastContext != nullptr)
      {
        // add the write to the dependency list
        eventList.Push(e);

        // unmap memory
        e = map->Release(eventList);
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
    lastContext = nullptr;
    Purge();

    if(data != nullptr)
    {
      // free host data
      csFree(data);
      data = nullptr;
    }
  }

  void MemoryObject::Purge()
  {
    CS::Threading::ScopedLock lock(useLock);

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
      csFree(data);
      data = nullptr;
    }

    // free all references except the last written one
    csHash<csRef<Context>, cl_mem>::Iterator it = handles.GetIterator();
    while(it.HasNext())
    {
      Context* c;
      cl_mem obj = it.Next(c);
      if(c != lastWriteContext)
      {
        cl_int error = clReleaseMemObject(obj);
        CS_ASSERT(error == CL_SUCCESS);
        handles.Delete(it);
      }
    }
  }

  void Sampler::Purge()
  {
    // free all references as they're easy to re-create
    csHash<csRef<Context>, cl_sampler>::Iterator it = handles.GetIterator();
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
