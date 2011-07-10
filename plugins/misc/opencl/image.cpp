#include "memory.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  cl_mem Image::CreateHandle(Context* c)
  {
    CS_ASSERT(GetWidth() != 0);
    CS_ASSERT(GetHeight() != 0);
    CS_ASSERT(c);

    cl_image_format imageFormat;

    // pretty dirty - maybe use a hash for the mapping instead
#   define MAP(X,Y,Z)\
    case FMT_ ## X :\
      Z = CL_ ## Y;\
      break
#   define MAPTYPE(X,Y) MAP(X,Y,imageFormat.image_channel_data_type)
#   define MAPORDER(X,Y) MAP(X,Y,imageFormat.image_channel_order)

    switch(format & FMT_TYPE)
    {
      // unsigned types
      MAPTYPE(UINT8_N,UNORM_INT8);
      MAPTYPE(UINT16_N,UNORM_INT16);
      MAPTYPE(UINT8,UNSIGNED_INT8);
      MAPTYPE(UINT16,UNSIGNED_INT16);
      MAPTYPE(UINT32,UNSIGNED_INT32);

      // signed types
      MAPTYPE(SINT8_N,SNORM_INT8);
      MAPTYPE(SINT16_N,SNORM_INT16);
      MAPTYPE(SINT8,SIGNED_INT8);
      MAPTYPE(SINT16,SIGNED_INT16);
      MAPTYPE(SINT32,SIGNED_INT32);

      // floating point types
      MAPTYPE(HALF,HALF_FLOAT);
      MAPTYPE(FLOAT,FLOAT);

      // packed types
      MAPTYPE(UINT16_N_565,UNORM_SHORT_565);
      MAPTYPE(UINT16_N_555,UNORM_SHORT_555);
      MAPTYPE(UINT32_N_101010,UNORM_INT_101010);

    default:
      CS_ASSERT(false);
    }

    switch(format & FMT_ORDER)
    {
      MAPORDER(R,R);
      MAPORDER(Rx,Rx);
      MAPORDER(A,A);

      MAPORDER(RG,RG);
      MAPORDER(RGx,RGx);
      MAPORDER(RA,RA);

      MAPORDER(RGB,RGB);
      MAPORDER(RGBx,RGBx);

      MAPORDER(RGBA,RGBA);
      MAPORDER(ARGB,ARGB);
      MAPORDER(BGRA,BGRA);

      MAPORDER(INT,INTENSITY);
      MAPORDER(LUM,LUMINANCE);

    default:
      CS_ASSERT(false);
    }

#   undef MAPTYPE
#   undef MAPORDER
#   undef MAP

    cl_mem obj;
    cl_int error;
    if(GetDepth() <= 1)
    {
      // 2D Image (3D images require a depth > 1)
      obj = clCreateImage2D(c->GetHandle(), CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,
                            &imageFormat, GetWidth(), GetHeight(), 0, nullptr, &error);
    }
    else
    {
      // 3D Image
      obj = clCreateImage3D(c->GetHandle(), CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,
                            &imageFormat, GetWidth(), GetHeight(), GetDepth(),
                            0, 0, nullptr, &error);
    }
    CS_ASSERT(error == CL_SUCCESS);

    return obj;
  }

  csPtr<iEvent> Image::Write(void* src, Context* c, const iEventList& events)
  {
    // obtain target handle
    cl_mem handle = *handles.GetElementPointer(c);

    // convert event list
    csRefArray<Event> eventList;
    if(!BuildEventList(events, eventList, 0))
    {
      return csPtr<iEvent>(nullptr);
    }

    csRef<iEvent> e;
    if(c == nullptr)
    {
      if(data == nullptr)
      {
        data = csAlloc(GetSize());
        if(data == nullptr)
        {
          // OOM
          return csPtr<iEvent>(nullptr);
        }
      }

      e.AttachNew(new Event(eventList));

      e->Wait(); // block for now

      memcpy(data, src, GetSize());
    }
    else
    {
      // obtain target queue
      csRef<Queue> q = c->GetQueue();

      // create the event handle list
      cl_event* handleList = CreateEventHandleList(eventList, c);
      if(!eventList.IsEmpty() && handleList == nullptr)
      {
        // OOM
        return csPtr<iEvent>(nullptr);
      }

      // queue the write
      cl_event handle;
      size_t offset[3] = {0,0,0};
      cl_int error = clEnqueueWriteImage(q->GetHandle(), obj, CL_FALSE, offset, size, 0, 0,
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
    return csPtr<iEvent>(e);
  }

  csPtr<iEvent> Image::Request(const size_t offset[3], const size_t size[3],
                               const iEventList& events)
  {
    for(size_t i = 0; i < 3; ++i)
    {
      if(offset[i]+size[i] > this->size[i])
      {
        // out of bounds
        return csPtr<iEvent>(nullptr);
      }
    }

    CS::Threading::ScopedLock lock(useLock);

    // convert event list
    csRefArray<Event> eventList;
    if(!BuildEventList(events, eventList))
    {
      return csPtr<iEvent>(nullptr);
    }

    csRef<iEvent> e; // holds the event the user shall wait on
    csRef<MappedMemory> map; // holds the new map object
    csRef<iEvent> unmapEvent; // holds the event further usages should wait on
    unmapEvent.AttachNew(new Event());

    if(lastWriteContext == nullptr)
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
      size_t rowPitch = GetWidth()*GetElementSize();
      size_t slicePitch = GetHeight()*rowPitch;
      size_t pitch[2] = { rowPitch, slicePitch };
      void* map = data + offset[0]*GetElementSize()
                       + offset[1]*rowPitch
                       + offset[2]*slicePitch;
      map.AttachNew(new MappedMemory(nullptr, this, map, size, offset, pitch, unmapEvent));
    }
    else
    {
      // convert event list
      cl_event* handleList = CreateEventHandleList(eventList, lastWriteContext);
      if(!eventList.IsEmpty() && handleList == nullptr)
      {
        // OOM
        return csPtr<iEvent>(nullptr);
      }

      cl_mem obj = GetHandle(lastWriteContext);

      // obtain queue
      csRef<Queue> q = lastContext->GetQueue();

      // map buffer
      cl_int error;
      cl_event handle;
      size_t pitch[2];
      void* dst = clEnqueueMapImage(q->GetHandle(), obj, CL_FALSE, CL_MAP_READ|CL_MAP_WRITE,
                                    offset, size, &pitch[0], &pitch[1], eventList.GetSize(),
                                    handleList, &handle, &error);
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
      map.AttachNew(new MappedMemory(lastWriteContext, this, dst, size, offset, pitch, unmapEvent));
    }

    // set map as result for the event
    e->SetResult(map);

    Use(lastWriteContext, unmapEvent, MEM_READ_WRITE);

    // return event
    return csPtr<iEvent>(e);
  }

  csRef<iEvent> Image::Read(void* dst, const size_t size[3], const size_t offset[3],
                            const size_t pitch[2], const iEventList& events)
  {
    if(dst == nullptr)
    {
      return csRef<iEvent>(nullptr);
    }

    for(size_t i = 0; i < 3; ++i)
    {
      if(offset[i]+size[i] > this->size[i])
      {
        // out of bounds
        return csRef<iEvent>(nullptr);
      }
    }

    size_t rowPitch = pitch[0] ? pitch[0] : size[0]*GetElementSize();
    size_t slicePitch = pitch[1] ? pitch[1] : rowPitch*size[1];

    if(rowPitch < size[0]*GetElementSize())
    {
      // overlapping regions
      return csRef<iEvent>(nullptr);
    }

    if(slicePitch < rowPitch*size[1])
    {
      // overlapping regions
      return csRef<iEvent>(nullptr);
    }

    CS::Threading::ScopedLock lock(useLock);

    // convert event list
    csRefArray<Event> eventList;
    if(!BuildEventList(events, eventList, MEM_READ))
    {
      return csRef<iEvent>(nullptr);
    }

    csRef<iEvent> e;
    Context* c = nullptr;
    if(status.Get(nullptr,false))
    {
      // latest version is host one
      e.AttachNew(new Event(eventList));

      e->Wait(); // block for now

      // size in bytes of a scanline
      size_t lineSize = size[0]*GetElementSize();

      // pitch of the original memory
      size_t srcRowPitch = GetWidth()*GetElementSize();
      size_t srcSlicePitch = GetHeight()*srcRowPitch;

      size_t sliceSkip = slicePitch - size[1]*rowPitch;
      size_t srcSliceSkip = srcSlicePitch - size[1]*srcRowPitch;

      void* source = data + offset[0]*GetElementSize();
      source += offset[1]*srcRowPitch;
      source += offset[2]*srcSlicePitch;

      void* target = dst;

      // copy the according line parts
      for(size_t z = 0; z < size[2]; ++z)
      {
        for(size_t y = 0; y < size[1]; ++y)
        {
          memcpy(target, source, lineSize);

          target += rowPitch;
          source += srcRowPitch;
        }
        target += sliceSkip;
        source += srcSliceSkip;
      }
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
      cl_int error = clEnqueueReadImage(q->GetHandle(), obj, CL_FALSE, offset, size,
                                        rowPitch, slicePitch, dst, eventList.GetSize(),
                                        handleList, &handle);
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

  csRef<iEvent> Image::Write(void* src, const size_t size[3], const size_t offset[3],
                             const size_t pitch[2], const iEventList& events)
  {
    if(src == nullptr)
    {
      return csRef<iEvent>(nullptr);
    }

    for(size_t i = 0; i < 3; ++i)
    {
      if(offset[i]+size[i] > this->size[i])
      {
        // out of bounds
        return csRef<iEvent>(nullptr);
      }
    }

    size_t rowPitch = pitch[0] ? pitch[0] : size[0]*GetElementSize();
    size_t slicePitch = pitch[1] ? pitch[1] : rowPitch*size[1];

    if(rowPitch < size[0]*GetElementSize())
    {
      // overlapping regions
      return csRef<iEvent>(nullptr);
    }

    if(slicePitch < rowPitch*size[1])
    {
      // overlapping regions
      return csRef<iEvent>(nullptr);
    }

    CS::Threading::ScopedLock lock(useLock);

    // convert event list
    csRefArray<Event> eventList;
    if(!BuildEventList(events, eventList))
    {
      return csRef<iEvent>(nullptr);
    }

    csRef<iEvent> e;
    Context* c = nullptr;
    if(status.Get(nullptr,false))
    {
      // latest version is host one
      e.AttachNew(new Event(eventList));

      e->Wait(); // block for now

      // size in bytes of a scanline
      size_t lineSize = size[0]*GetElementSize();

      // pitch of the original memory
      size_t dstRowPitch = GetWidth()*GetElementSize();
      size_t dstSlicePitch = GetHeight()*dstRowPitch;

      size_t sliceSkip = slicePitch - size[1]*rowPitch;
      size_t dstSliceSkip = dstSlicePitch - size[1]*dstRowPitch;

      void* target = data + offset[0]*GetElementSize();
      target += offset[1]*dstRowPitch;
      target += offset[2]*dstSlicePitch;

      void* source = src;

      // copy the according line parts
      for(size_t z = 0; z < size[2]; ++z)
      {
        for(size_t y = 0; y < size[1]; ++y)
        {
          memcpy(target, source, lineSize);

          target += dstrowPitch;
          source += RowPitch;
        }
        target += dstSliceSkip;
        source += sliceSkip;
      }
    }
    else
    {
      c = lastWriteContext;
      cl_mem obj = GetHandle(c);

      // get queue for target device
      csRef<Queue> q = lastContext->GetQueue();

      cl_event* handleList = CreateEventHandleList(eventList, c);
      if(!eventList.IsEmpty() && handleList == nullptr)
      {
        // OOM
        return csRef<Event>(nullptr);
      }

      cl_event handle;
      cl_int error = clEnqueueWriteImage(q->GetHandle(), obj, CL_FALSE, offset, size,
                                         rowPitch, slicePitch, dst, eventList.GetSize(),
                                         handleList, &handle);
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

  csRef<iEvent> Image::Copy(iImage* dstObj, const size_t size[3], const size_t src_offset[3],
                            const size_t dst_offset[3], const iEventList& events)
  {
    csRef<Image> dst = scfQueryInterfaceSafe(dstObj);
    if(!dst.IsValid())
    {
      // invalid destination
      return csRef<iEvent>(nullptr);
    }

    for(size_t i = 0; i < 3; ++i)
    {
      size_t limit = src_offset[i]+size[i];
      if(limit > this->size[i] || limit > dst->size[i])
      {
        // out of bounds
        return csRef<iEvent>(nullptr);
      }
    }

    if(format != dst->format)
    {
      // incompatible formats
      return csRef<iEvent>(nullptr);
    }

    CS::Threading::ScopedLock lock(useLock);
    CS::Threading::ScopedLock dstLock(dst->useLock);

    if(status.Get(nullptr, false))
    {
      iEventList list(events);
      if(lastWriteEvent.IsValid())
      {
        list.Push(lastWriteEvent);
      }

      size_t rowPitch = GetElementSize()*GetWidth();
      size_t slicePitch = rowPitch*GetHeight();
      void* src = data + src_offset[0]*GetElementSize()
                       + src_offset[1]*rowPitch
                       + src_offset[2]*slicePitch;
      size_t pitch[2] = {rowPitch, slicePitch};
      csRef<iEvent> e = dst->Write(src, size, dst_offset, pitch, list);
      if(e.IsValid())
      {
        Use(nullptr, e, MEM_READ);
      }
      return e;
    }
    else if(dst->status.Get(nullptr, false))
    {
      iEventList list(events);
      for(size_t i = 0; i < dst->reads.GetSize(); ++i)
      {
        list.Push(dst->reads[i]);
      }

      size_t rowPitch = dst->GetElementSize()*dst->GetWidth();
      size_t slicePitch = rowPitch*dst->GetHeight();
      void* dst = data + dst_offset[0]*dst->GetElementSize()
                       + dst_offset[1]*rowPitch
                       + dst_offset[2]*slicePitch;
      size_t pitch[2] = {rowPitch, slicePitch};
      csRef<iEvent> e = Read(dst, size, src_offset, pitch, list);
      if(e.IsValid())
      {
        dst->Use(nullptr, e, MEM_WRITE);
      }
      return e;
    }

    csRefArray<Event> eventList;
    if(!BuildEventList(events, eventList, dst))
    {
      return csRef<iEvent>(nullptr);
    }

    Context* c = FindContext(dst, eventList, false);

    // get queue for the context
    csRef<Queue> q = c->GetQueue();

    cl_mem src = GetHandle(c);
    cl_mem dst = dst->GetHandle(c);

    cl_event* handleList = CreateEventHandleList(eventList, c);
    if(!eventList.IsEmpty() && handleList == nullptr)
    {
      return csRef<iEvent>(nullptr);
    }

    cl_event handle;
    cl_int error = clEnqueueCopyImage(q->GetHandle(), src, dst, src_offset, dst_offset,
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

    // update usages
    Use(c, e, MEM_READ);
    dst->Use(c, e, MEM_WRITE);

    return e;
  }

  csRef<iEvent> Image::Copy(iBuffer* dstObj, size_t dst_offset, const size_t src_offset[3],
                            const size_t src_size[3], const iEventList& events)
  {
    csRef<Buffer> dst = scfQueryInterfaceSafe<Buffer>(dstObj);
    if(!dst.IsValid())
    {
      // invalid destination
      return csRef<iEvent>(nullptr);
    }

    for(size_t i = 0; i < 3; ++i)
    {
      if(src_offset[i]+src_size[i] < size[i])
      {
        // out of bounds
        return csRef<iEvent>(nullptr);
      }
    }

    if(dst_offset+GetElementSize()*(src_offset[0]+
       GetWidth()*(src_offset[1]+src_offset[2]*GetHeight())) > dst->GetSize())
    {
      // out of bounds
      return csRef<iEvent>(nullptr);
    }

    CS::Threading::ScopedLock lock(useLock);
    CS::Threading::ScopedLock dstLock(dst->useLock);

    //@@@todo: add specialization in case lastContext == nullptr
    if(dst->status.Get(nullptr, false))
    {
      iEventList list(events);
      for(size_t i = 0; i < dst->reads.GetSize(); ++i)
      {
        list.Push(dst->reads[i]);
      }

      size_t pitch[2] = {0,0}; // automatically computed
      csRef<iEvent> e = Read(dst->data + dst_offset, src_size, src_offset, pitch, list);
      if(e.IsValid())
      {
        dst->Use(nullptr, e, MEM_WRITE);
      }
      return e;
    }

    csRefArray<Event> eventList;
    if(!BuildEventList(events, eventList, dst))
    {
      return csRef<iEvent>(nullptr);
    }

    Context* c = FindContext(dst, eventList, false);

    csRef<Queue> q = c->GetQueue();

    cl_event* handleList = CreateEventHandleList(eventList, c);
    if(!eventList.IsEmpty() && handleList == nullptr)
    {
      // OOM
      return csRef<iEvent>(nullptr);
    }

    cl_mem src = GetHandle(c);
    cl_mem dst = dst->GetHandle(c);

    cl_event handle;
    cl_int error = clEnqueueCopyImageToBuffer(q->GetHandle(), src, dst, src_offset,
                                              src_size, dst_offset, eventList.GetSize(),
                                              handleList, &handle);
    delete [] handleList;

    switch(error)
    {
    case CL_SUCCESS:
      break;

    default:
      return csRef<iEvent>(nullptr);
    }

    csRef<iEvent> e = csPtr<iEvent>(new Event(c, handle));

    // update usage events
    Use(c, e, MEM_READ);
    dst->Use(c, e, MEM_WRITE);

    return e;
  }
}
CS_PLUGIN_NAMESPACE_END(CL)
