#include "cssysdef.h"

#include "manager.h"
#include "memory.h"
#include "library.h"
#include "kernel.h"
#include "queue.h"
#include "context.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  bool Manager::Initialize(iObjectRegistry*)
  {
    cl_uint platformCount;
    cl_int result;

    result = clGetPlatformIDs(0,nullptr,&platformCount);
    switch(result)
    {
    case CL_SUCCESS:
      // no error
      break;
    case CL_INVALID_VALUE:
      // manager bug
      return false;
    case CL_OUT_OF_HOST_MEMORY:
      // out of memory
      return false;
    default:
      // driver bug
      return false;
    }

    if(platformCount == 0)
    {
      // no platforms available
      return false;
    }

    cl_platform_id* platforms = new cl_platform_id[platformCount];
    if(platforms == nullptr)
    {
      // out of memory
      return false;
    }

    result = clGetPlatformIDs(platformCount,platforms,nullptr);

    switch(result)
    {
    case CL_SUCCESS:
      // no error
      break;
    case CL_INVALID_VALUE:
      // manager bug
    case CL_OUT_OF_HOST_MEMORY:
      // out of memory
    default:
      // driver bug
      delete [] platforms;
      return false;
    }

    for(cl_uint i = 0; i < platformCount; ++i)
    {
      csRef<Platform> p;
      p.AttachNew(new Platform(platforms[i]));
      if(!p->Initialize())
      {
        continue;
      }
      //@@@todo: validate whether this platform shall be used
      // e.g. isn't blacklisted (intel), is sufficiently recent, ...

      csRefArray<Device> devices = p->GetDevices();
      for(cl_uint j = 0; j < devices.GetSize(); ++j)
      {
        //@@@todo: validate device and remove it if it's invalid
      }

      if(!devices.IsEmpty())
      {
        csRef<Context> c;
        c.AttachNew(new Context(p, devices));

        if(!c->Initialize())
        {
          continue;
        }
        // validate whether this context should be used
        // e.g. is sufficiently recent, etc.
        contexts.Push(c);

        // create queues for devices
        for(size_t j = 0; j < devices.GetSize(); ++j)
        {
          csRef<CS_PLUGIN_NAMESPACE_NAME(CL)::Queue> q = c->CreateQueue(devices[j]);
          queues.Push(q);
        }
      }
    }

    delete [] platforms;
    return true;
  }

  bool Manager::QueueWait(const iEventList& list)
  {
    csRefArray<Event> events = CreateEventList(list);

    bool success = true;
    for(size_t i = 0; i < queues.GetSize(); ++i)
    {
      success &= queues[i]->QueueWait(events);
    }
    return success;
  }

  bool Manager::Wait(const iEventList& list)
  {
    if(list.IsEmpty())
    {
      return true;
    }

    CS_ASSERT(!contexts.IsEmpty())

    csRefArray<Event> events = CreateEventList(list);
    cl_event* handleList = CreateEventHandleList(events, contexts[0]);
    if(handleList == nullptr)
    {
      return false;
    }

    // wait for the events
    cl_int error = clWaitForEvents(events.GetSize(), handleList);
    delete [] handleList;

    // validate success
    switch(error)
    {
    case CL_SUCCESS:
      // everything fine
    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
      // some event finished signaling an error
      return true;

    case CL_INVALID_VALUE:
      // no events to wait on - bug in this function
    case CL_INVALID_CONTEXT:
      // not all events share the same context - bug in Event::GetHandle()
    case CL_INVALID_EVENT:
      // some event object wasn't valid - bug in Event::GetHandle()
    case CL_OUT_OF_RESOURCES:
      // device OOM
    case CL_OUT_OF_HOST_MEMORY:
      // host OOM
    default:
      // driver bug
      return false;
    }
  }

  void Manager::Finish()
  {
    for(size_t i = 0; i < queues.GetSize(); ++i)
    {
      queues[i]->Flush(); // do we have to/want to flush here?
      queues[i]->Finish();
    }
  }

  bool Manager::QueueBarrier()
  {
    bool success = true;
    for(size_t i = 0; i < queues.GetSize(); ++i)
    {
      success &= queues[i]->QueueBarrier();
    }
    return success;
  }

  csPtr<iEvent> Manager::QueueMarker()
  {
    csRefArray<Event> eventList(queues.GetSize());
    for(size_t i = 0; i < queues.GetSize(); ++i)
    {
      csRef<Event> e = queues[i]->QueueMarker();
      if(!e.IsValid())
      {
        return csPtr<iEvent>(nullptr);
      }
    }
    csRef<Event> e;
    e.AttachNew(new Event(eventList));
    return csPtr<iEvent>(e);
  }

  void Manager::FreeCompiler()
  {
    cl_int result = clUnloadCompiler();
    CS_ASSERT(result == CL_SUCCESS);
  }

  csPtr<iBuffer> Manager::CreateBuffer(size_t size, int access, void* src)
  {
    csRef<iBuffer> b;
    b.AttachNew(new Buffer(size, access));
    if(src != nullptr && b.IsValid())
    {
      csRef<iEvent> e = b->Write(0, size, src);
      if(e.IsValid())
      {
        e->Wait();
      }
      else
      {
        return csPtr<iBuffer>(nullptr);
      }
    }
    return csPtr<iBuffer>(b);
  }

  csPtr<iImage> Manager::CreateImage(size_t width, size_t height, int format,
                                     int access, void* src, size_t row_pitch)
  {
    return CreateImage(width, height, 1, format, access, src, row_pitch, 0);
  }

  csPtr<iImage> Manager::CreateImage(size_t width, size_t height, size_t depth,
                                     int format, int access, void* src,
                                     size_t row_pitch, size_t slice_pitch)
  {
    csRef<iImage> image;
    const size_t size[3] = {width, height, depth};
    image.AttachNew(new Image(size, format, access));
    if(image.IsValid() && src != nullptr)
    {
      const size_t offset[3] = {0,0,0};
      const size_t pitch[2] = {row_pitch, slice_pitch};
      csRef<iEvent> e = image->Write(src, size, offset, pitch);
      if(e.IsValid())
      {
        e->Wait();
      }
      else
      {
        return csPtr<iImage>(nullptr);
      }
    }
    return csPtr<iImage>(image);
  }
  
  csPtr<iSampler> Manager::CreateSampler(int addressMode, int filterMode, bool normalized)
  {
    csRef<iSampler> s;
    s.AttachNew(new Sampler(addressMode, filterMode, normalized));
    return csPtr<iSampler>(s);
  }

  csPtr<iLibrary> Manager::CreateLibrary(iStringArray* source)
  {
    csRef<iLibrary> l;
    l.AttachNew(new Library(this, source));
    return csPtr<iLibrary>(l);
  }

  csRef<iEvent> Manager::Queue(iKernel* kernel, const iEventList& events)
  {
    if(kernel->GetDimension() == 0)
    {
      return csRef<iEvent>(nullptr);
    }

    // find a suitable device to queue this kernel
    // just pick some gpu device with image support for now
    csRef<CS_PLUGIN_NAMESPACE_NAME(CL)::Queue> q;
    csRefArray<CS_PLUGIN_NAMESPACE_NAME(CL)::Queue> candidates;
    for(size_t i = 0; i < queues.GetSize(); ++i)
    {
      Device* d = queues[i]->GetDevice();
      if(!d->ImageSupport())
      {
        continue;
      }
      else if(d->GetType() == CL_DEVICE_TYPE_GPU)
      {
        candidates.Push(queues[i]);
      }
    }

    if(candidates.IsEmpty())
    {
      // no suitable one found, just use some queue
      q = queues[0];
    }
    else
    {
      // try to find one with out of order support
      for(size_t i = 0; i < candidates.GetSize(); ++i)
      {
        if(candidates[i]->GetDevice()->OutOfOrderSupport())
        {
          q = candidates[i];
          break;
        }
      }

      // nothing found, just use one of the candidates
      q = candidates[0];
    }

    CS_ASSERT(q.IsValid()); // no queue was valid?

    csRefArray<Event> eventList = CreateEventList(events);
    if(eventList.GetSize() != events.GetSize())
    {
      return csRef<iEvent>(nullptr);
    }

    csRef<Kernel> kernelObj = scfQueryInterface<Kernel>(kernel);
    CS_ASSERT(kernelObj.IsValid());

    Context* c = q->GetContext();
    cl_kernel k = kernelObj->CreateHandle(c, eventList);

    cl_event* handleList = CreateEventHandleList(eventList, c);
    if(!eventList.IsEmpty() && handleList == nullptr)
    {
      clReleaseKernel(k);
      return csRef<iEvent>(nullptr);
    }

    cl_event handle;
    size_t dimension = kernel->GetDimension();
    size_t* offset = kernel->GetWorkOffset();
    size_t* size = kernel->GetWorkSize();
    size_t* localSize = kernel->GetGroupSize();
    bool autoLocalSize = false;
    for(size_t i = 0; i < dimension; ++i)
    {
      if(localSize[dimension] == 0)
      {
        autoLocalSize = true;
        break;
      }
    }

    cl_int error = clEnqueueNDRangeKernel(q->GetHandle(), k, dimension, offset, size,
                                          autoLocalSize ? nullptr : localSize,
                                          eventList.GetSize(), handleList, &handle);
    delete [] handleList;
    delete [] offset;
    delete [] size;
    delete [] localSize;
    clReleaseKernel(k);

    switch(error)
    {
    case CL_SUCCESS:
      break;

    default:
      return csPtr<iEvent>(nullptr);
    }

    csRef<iEvent> e = csPtr<iEvent>(new Event(c, handle));
    return e;
  }
}
CS_PLUGIN_NAMESPACE_END(CL)
