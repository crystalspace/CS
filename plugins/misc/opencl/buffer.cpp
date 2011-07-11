#include "cssysdef.h"

#include "memory.h"
#include "context.h"
#include "event.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  cl_mem Buffer::CreateHandle(Context* c)
  {
    cl_int error;
    cl_mem handle = clCreateBuffer(c->GetHandle(), CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,
                                   size, nullptr, &error);
    CS_ASSERT(error == CL_SUCCESS);
    return handle;
  }

  csPtr<iEvent> Buffer::Write(void* src, Context* c, const iEventList& events)
  {
    // CS::Threading::ScopedLock lock(useLock);
    // don't lock here, it's already locked and we don't
    // touch dependencies

    csRefArray<Event> eventList;
    // don't get dependencies here - we already have the right ones
    // as this only originates in MoveTo
    if(!BuildEventList(events, eventList, 0))
    {
      return csPtr<iEvent>(nullptr);
    }

    csRef<iEvent> e;
    if(c == nullptr)
    {
      if(data == nullptr)
      {
        data = cs_malloc(size);
        if(data == nullptr)
        {
          // OOM
          return csPtr<iEvent>(nullptr);
        }
      }

      e.AttachNew(new Event(eventList));

      e->Wait(); // block for now (probably not a good idea at all)

      memcpy(data, src, size);
    }
    else
    {
      // obtain target queue
      csRef<Queue> q = c->GetQueue();

      // create the event handle list
      cl_event* handleList = CreateEventHandleList(eventList, c);
      if(!events.IsEmpty() && handleList == nullptr)
      {
        // OOM
        return csPtr<iEvent>(nullptr);
      }

      // queue the write
      cl_mem obj = GetHandle(c);
      cl_event handle;
      cl_int error = clEnqueueWriteBuffer(q->GetHandle(), obj, CL_FALSE, 0, size,
                                          src, eventList.GetSize(), handleList, &handle);
      delete [] handleList;

      switch(error)
      {
      case CL_SUCCESS:
        // everything went fine
        break;

      default:
        return csPtr<iEvent>(nullptr);
      }

      e.AttachNew(new Event(c, handle));
    }

    // Use(c, e, MEM_WRITE);
    // don't mark usage here as this is only used for moving objects between contexts

    return csPtr<iEvent>(e);
  }

  csPtr<iEvent> Buffer::Request(size_t offset, size_t size, const iEventList& events)
  {
    if(offset+size > this->size)
    {
      return csPtr<iEvent>(nullptr);
    }

    CS::Threading::RecursiveMutexScopedLock lock(useLock);

    csRef<Event> e; // holds the event the user shall wait on
    csRef<MappedMemory> map; // holds the new map object
    csRef<Event> unmapEvent; // holds the event further usages should wait on
    unmapEvent.AttachNew(new Event());

    csRefArray<Event> eventList;
    if(!BuildEventList(events, eventList, MEM_READ_WRITE))
    {
      return csPtr<iEvent>(nullptr);
    }

    if(status.Get(nullptr,false))
    {
      // most recent version is located on host

      if(data == nullptr)
      {
        // most recent version isn't valid? fail...
        return csPtr<iEvent>(nullptr);
      }

      // create meta-event - fires once dependencies are finished
      e.AttachNew(new Event(eventList));

      // create new map
      size_t mapSize[3] = {size, 0, 0};
      size_t mapOffset[3] = {offset, 0, 0};
      size_t mapPitch[2] = {0,0};
      map.AttachNew(new MappedMemory(nullptr, this, (uint8*)data+offset, mapSize,
                                     mapOffset, mapPitch, unmapEvent));
    }
    else
    {
      cl_event* handleList = CreateEventHandleList(eventList, lastWriteContext);
      if(!eventList.IsEmpty() && handleList == nullptr)
      {
        // OOM
        return csPtr<iEvent>(nullptr);
      }

      cl_mem obj = GetHandle(lastWriteContext);

      // obtain queue
      csRef<Queue> q = lastWriteContext->GetQueue();

      // map buffer
      cl_int error;
      cl_event handle;
      void* dst = clEnqueueMapBuffer(q->GetHandle(), obj, CL_FALSE, CL_MAP_READ|CL_MAP_WRITE,
                                     offset, size, eventList.GetSize(), handleList, &handle,
                                     &error);
      delete [] handleList;

      switch(error)
      {
      case CL_SUCCESS:
        break;

      default:
        return csPtr<iEvent>(nullptr);
      }

      e.AttachNew(new Event(lastWriteContext, handle)); // create event

      // create new map
      size_t mapSize[3] = {size, 0, 0};
      size_t mapOffset[3] = {offset, 0, 0};
      size_t mapPitch[2] = {0,0};
      map.AttachNew(new MappedMemory(lastWriteContext, this, dst, mapSize,
                                     mapOffset, mapPitch, unmapEvent));
    }

    // set map as result for the event
    e->SetResult(map);

    // don't allow reads or writes until the map is released
    Use(lastWriteContext, unmapEvent, MEM_READ_WRITE);

    // return event
    return csPtr<iEvent>(e);
  }

  csRef<iEvent> Buffer::Read(size_t offset, size_t size, void* dst, const iEventList& events)
  {
    if(dst == nullptr)
    {
      return csRef<iEvent>(nullptr);
    }

    if(offset+size > this->size)
    {
      // out of bounds
      return csRef<iEvent>(nullptr);
    }

    CS::Threading::RecursiveMutexScopedLock lock(useLock);

    // convert event list
    csRefArray<Event> eventList;
    if(!BuildEventList(events, eventList, MEM_READ))
    {
      return csRef<iEvent>(nullptr);
    }

    csRef<Event> e;
    Context* c = nullptr;
    if(status.Get(nullptr,false))
    {
      // latest version is host one
      e.AttachNew(new Event(eventList));

      e->Wait(); // block for now

      memcpy(dst, (uint8*)data+offset, size);
    }
    else
    {
      c = lastWriteContext;
      cl_mem obj = GetHandle(c);

      // get queue for target device
      csRef<Queue> q = c->GetQueue();

      cl_event* handleList = CreateEventHandleList(eventList, c);
      if(!eventList.IsEmpty() && handleList == nullptr)
      {
        // OOM
        return csRef<Event>(nullptr);
      }

      cl_event handle;
      cl_int error = clEnqueueReadBuffer(q->GetHandle(), obj, CL_FALSE, offset, size,
                                         dst, eventList.GetSize(), handleList, &handle);
      delete [] handleList;

      switch(error)
      {
      case CL_SUCCESS:
        break;

      default:
        return csRef<Event>(nullptr);
      }

      e.AttachNew(new Event(c, handle));
    }
    Use(c, e, MEM_READ);

    return e;
  }

  csRef<iEvent> Buffer::Write(size_t offset, size_t size, void* src, const iEventList& events)
  {
    if(src == nullptr)
    {
      return csRef<iEvent>(nullptr);
    }

    if(offset+size > this->size)
    {
      // out of bounds
      return csRef<iEvent>(nullptr);
    }

    CS::Threading::RecursiveMutexScopedLock lock(useLock);

    // convert event list
    csRefArray<Event> eventList;
    if(!BuildEventList(events, eventList, MEM_WRITE))
    {
      return csRef<iEvent>(nullptr);
    }

    csRef<Event> e;
    Context* c = nullptr;
    if(status.Get(nullptr,false))
    {
      // latest version is host one
      e.AttachNew(new Event(eventList));

      e->Wait(); // block for now

      memcpy((uint8*)data+offset, src, size);
    }
    else
    {
      c = lastWriteContext;
      cl_mem obj = GetHandle(c);

      // get queue for target device
      csRef<Queue> q = c->GetQueue();

      cl_event* handleList = CreateEventHandleList(eventList, c);
      if(!eventList.IsEmpty() && handleList == nullptr)
      {
        // OOM
        return csRef<Event>(nullptr);
      }

      cl_event handle;
      cl_int error = clEnqueueWriteBuffer(q->GetHandle(), obj, CL_FALSE, offset, size,
                                          src, eventList.GetSize(), handleList, &handle);
      delete [] handleList;

      switch(error)
      {
      case CL_SUCCESS:
        break;

      default:
        return csRef<Event>(nullptr);
      }

      e.AttachNew(new Event(c, handle));
    }

    Use(c, e, MEM_WRITE);

    return e;
  }

  csRef<iEvent> Buffer::Copy(iBuffer* dstBuffer, size_t size, size_t src_offset,
                             size_t dst_offset, const iEventList& events)
  {
    csRef<Buffer> dst = scfQueryInterfaceSafe<Buffer>(dstBuffer);
    if(!dst.IsValid())
    {
      return csRef<iEvent>(nullptr);
    }

    if(src_offset+size > this->size || dst_offset+size > dst->size)
    {
      // out of bounds
      return csRef<iEvent>(nullptr);
    }

    CS::Threading::RecursiveMutexScopedLock lock(useLock);
    CS::Threading::RecursiveMutexScopedLock dstLock(dst->useLock);

    if(status.Get(nullptr,false))
    {
      iEventList list(events);
      if(lastWrite.IsValid())
      {
        list.Push(lastWrite);
      }

      csRef<Event> e = scfQueryInterfaceSafe<Event>(dst->Write(dst_offset, size, (uint8*)data+src_offset, list));
      if(e.IsValid())
      {
        Use(nullptr, e, MEM_READ);
      }
      return e;
    }
    else if(dst->status.Get(nullptr,false))
    {
      iEventList list(events);
      for(size_t i = 0; i < dst->reads.GetSize(); ++i)
      {
        list.Push(dst->reads[i]);
      }

      csRef<Event> e = scfQueryInterfaceSafe<Event>(Read(src_offset, size, (uint8*)(dst->data)+dst_offset, list));
      if(e.IsValid())
      {
        dst->Use(nullptr, e, MEM_WRITE);
      }
      return e;
    }

    csRefArray<Event> eventList;
    if(!BuildEventList(events, eventList, MEM_READ, dst, MEM_WRITE))
    {
      return csRef<iEvent>(nullptr);
    }

    Context* c = FindContext(dst, eventList, false);

    // get queue for the context
    csRef<Queue> q = c->GetQueue();

    cl_mem src_obj = GetHandle(c);
    cl_mem dst_obj = dst->GetHandle(c);

    cl_event* handleList = CreateEventHandleList(eventList, c);
    if(!eventList.IsEmpty() && handleList == nullptr)
    {
      return csRef<iEvent>(nullptr);
    }

    cl_event handle;
    cl_int error = clEnqueueCopyBuffer(q->GetHandle(), src_obj, dst_obj, src_offset, dst_offset,
                                       size, eventList.GetSize(), handleList, &handle);
    delete [] handleList;

    switch(error)
    {
    case CL_SUCCESS:
      break;

    default:
      return csRef<iEvent>(nullptr);
    }

    csRef<Event> e = csPtr<Event>(new Event(c, handle));

    // update usage events
    Use(c, e, MEM_READ);
    dst->Use(c, e, MEM_WRITE);

    return e;
  }

  csRef<iEvent> Buffer::Copy(iImage* dstImage, size_t src_offset, const size_t dst_offset[3],
                             const size_t dst_size[3], const iEventList& events)
  {
    csRef<Image> dst = scfQueryInterfaceSafe<Image>(dstImage);
    if(!dst.IsValid())
    {
      return csRef<iEvent>(nullptr);
    }

    if(dst_offset[0]+dst_size[0] > dst->GetWidth() ||
       dst_offset[1]+dst_size[1] > dst->GetHeight() ||
       dst_offset[2]+dst_size[2] > dst->GetDepth())
    {
      // out of bounds
      return csRef<iEvent>(nullptr);
    }

    size_t elementSize = dst->GetElementSize();
    if(src_offset + elementSize*(dst_size[0] + dst->GetWidth() *
       (dst_size[1]+dst->GetHeight()*dst_size[2])) > size)
    {
      // too big for this buffer
      return csRef<iEvent>(nullptr);
    }

    CS::Threading::RecursiveMutexScopedLock lock(useLock);
    CS::Threading::RecursiveMutexScopedLock dstLock(dst->useLock);

    if(status.Get(nullptr,false))
    {
      iEventList list(events);
      if(lastWrite.IsValid())
      {
        list.Push(lastWrite);
      }

      size_t pitch[2] = {0,0};
      csRef<Event> e = scfQueryInterfaceSafe<Event>(dst->Write((uint8*)data+src_offset, dst_size, dst_offset, pitch, list));
      if(e.IsValid())
      {
        Use(nullptr, e, MEM_READ);
      }
      return e;
    }
    //@@@todo: add specialization using Buffer::Read if image is valid on host

    csRefArray<Event> eventList;
    if(!BuildEventList(events, eventList, MEM_READ, dst, MEM_WRITE))
    {
      return csRef<iEvent>(nullptr);
    }

    Context* c = FindContext(dst, eventList, false);

    // get queue for the context
    csRef<Queue> q = c->GetQueue();

    cl_mem src_obj = GetHandle(c);
    cl_mem dst_obj = dst->GetHandle(c);

    cl_event* handleList = CreateEventHandleList(eventList, c);
    if(!eventList.IsEmpty() && handleList == nullptr)
    {
      return csRef<iEvent>(nullptr);
    }

    cl_event handle;
    cl_int error = clEnqueueCopyBufferToImage(q->GetHandle(), src_obj, dst_obj, src_offset,
                           dst_offset, dst_size, eventList.GetSize(), handleList, &handle);
    delete [] handleList;

    switch(error)
    {
    case CL_SUCCESS:
      break;

    default:
      return csRef<iEvent>(nullptr);
    }

    csRef<Event> e = csPtr<Event>(new Event(c, handle));

    // update usages
    Use(c, e, MEM_READ);
    dst->Use(c, e, MEM_WRITE);

    return e;
  }
}
CS_PLUGIN_NAMESPACE_END(CL)
