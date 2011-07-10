#include "kernel.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  Kernel::Kernel(Library* p,const KernelMetaData& metaData)
              : scfImplementationType(this, p), parent(p), data(metaData),
                argsSize(data->argCount), args(data->argCount),
                objArgs(data->argCount), dimension(0)
  {
    for(size_t i = 0; i < data->argCount; ++i)
    {
      argsSize[i] = 0;
      args[i] = nullptr;
    }
  }

  Kernel::~Kernel()
  {
    for(size_t i = 0; i < args.GetSize(); ++i)
    {
      csFree(args[i]);
    }
  }

  bool Kernel::SetArg(cl_kernel kernel, size_t id, const void* arg, size_t size)
  {
    if(id >= data->argCount)
    {
      // out of bounds
      return false;
    }

    switch(args->[id] & ARG_STORAGE_MASK)
    {
    case ARG_LOCAL:
      if(arg != nullptr)
      {
        // local arguments mustn't point to any data
        return false;
      }
      if(size == 0)
      {
        // local arguments must have a size set
        return false;
      }
      break;

    default:
      if(arg == nullptr || size == 0)
      {
        // nothing to set
        return false;
      }
      break;
    }

    switch(args->[id] & ARG_TYPE_MASK)
    {
    case ARG_DATA:
      // matches
      break;

    default:
      // this should be an object
      // use the object variant instead
      return false;
    }

    // copy data for arguments that aren't marked __local
    if(arg != nullptr)
    {
      // allocate copy
      void* argCopy = csAlloc(size);
      if(argCopy == nullptr)
      {
        // OOM
        return false;
      }

      // copy
      memcpy(argCopy, arg, size);

      // keep copy
      args[id] = argCopy;
    }

    // keep size
    argsSize[id] = size;

    return true;
  }

  bool Kernel::SetArg(cl_kernel kernel, size_t id, iMemoryObject* obj)
  {
    if(id >= data->argCount)
    {
      return false;
    }

    if(obj == nullptr)
    {
      return false;
    }

    switch(data->args[id] & ARG_STORAGE_MASK)
    {
    case ARG_LOCAL:
      return false;

    default.
      break;
    }

    // validate argument
    switch(data->args[id] & ARG_TYPE_MASK)
    {
    case ARG_BUFFER:
      if(obj->GetObjectType() != MEM_BUFFER)
      {
        return false;
      }
      break;

    case ARG_SAMPLER:
      if(obj->GetObjectType() != MEM_SAMPLER)
      {
        return false;
      }
      break;

    case ARG_IMAGE2D:
    case ARG_IMAGE3D:
      if(obj->GetObjectType() != MEM_IMAGE)
      {
        return false;
      }
      break;

    case ARG_DATA:
    default:
      return false;
    }

    // keep a ref to the object
    objArgs[i] = obj;

    return true;
  }

  bool Kernel::SetInstanceArg(cl_kernel kernel, size_t id, const void* arg, size_t size)
  {
    cl_int error = clSetKernelArg(kernel, id, size, arg);
    switch(error)
    {
    case CL_SUCCESS:
      // everything went fine
      return true;

    case CL_INVALID_KERNEL:
      // this object isn't valid
    case CL_INVALID_ARG_INDEX:
      // index out of bounds
    case CL_INVALID_ARG_VALUE:
      // argument invalid, e.g. one of the following applies:
      // argument was declared as buffer, but isn't
      // argument was declared as image2d_t, but is not a 2d image
      // argument was declared as image3d_t, but is not a 3d image
      // argument was declared __global or __constant, but is not a buffer
      // argument was declared __local, but arg isn't NULL
      // argument was declared __constant, but exceeds device limits
    case CL_INVALID_MEM_OBJECT:
      // the passed memory object (buffer/image) wasn't valid
    case CL_INVALID_SAMPLER:
      // argument was declared as sampler, but is not a sampler
    case CL_INVALID_ARG_SIZE:
      // argument was declared as memory object, but size != sizeof(cl_mem) or
      // argument was declared as sampler, but size != sizeof(cl_sampler) or
      // argument was declared as __local, but size is 0
    case CL_OUT_OF_RESSOURCES:
      // device OOM
    case CL_OUT_OF_HOST_MEMORY:
      // host OOM
    default:
      // driver bug
      return false;
    }
  }

  bool Kernel::SetInstanceArg(cl_kernel kernel, size_t id, iMemoryObject* obj,
                              Context* c, csRefArray<Event>& eventList)
  {
    if(obj == nullptr)
    {
      return false;
    }

    switch(obj->GetObjectType())
    {
    case MEM_SAMPLER:
    {
      csRef<Sampler> impl = scfQueryInterface<Sampler>(obj);
      if(!impl.IsValid())
      {
        return false;
      }

      cl_sampler handle = impl->GetHandle(c);
      return SetInstanceArg(id, &handle, sizeof(cl_sampler));
    }

    case MEM_BUFFER:
    case MEM_IMAGE:
    {
      csRef<MemoryObject> impl = scfQueryInterface<MemoryObject>(obj);
      if(!impl.IsValid())
      {
        return false;
      }

      cl_mem handle = impl->GetHandle(c);
      csRef<Event> e = scfQueryInterfaceSafe<Event>(impl->MoveTo(c, eventList));
      if(e.IsValid())
      {
        eventList.Push(e);
        return SetInstanceArg(id, &handle, sizeof(cl_mem));
      }
      else
      {
        return false;
      }
    }

    default:
      // unknown object type
      return false;
  }

  cl_kernel Kernel::CreateHandle(Context* c, csRefArray<Event>& eventList)
  {
    // create kernel for this context
    cl_int error;
    cl_program program;
    csRef<Event> e = parent->GetHandle(c, program);

    // block while the program is built
    e->Wait();
    CS_ASSERT(e->WasSuccessful());

    cl_kernel kernel = clCreateKernel(program, data->name.GetData(), &error);
    CS_ASSERT(error == CL_SUCCESS);

    bool success = true;
    for(size_t i = 0; i < argCount; ++i)
    {
      if(objArgs[i].IsValid())
      {
        success &&= SetInstanceArg(kernel, i, objArgs[i], c, eventList);
      }
      else
      {
        success &&= SetInstanceArg(kernel, i, args[i], argsSize[i]);
      }
    }
    CS_ASSERT(success == true);

    return kernel;
  }

  void Kernel::SetWorkSize(const size_t* sizes)
  {
    for(size_t i = 0; i < dimension; ++i)
    {
      workSize[i] = sizes[i];
    }
  }

  void Kernel::SetWorkOffset(const size_t* offsets)
  {
    for(size_t i = 0; i < dimension; ++i)
    {
      workOffset[i] = offsets[i];
    }
  }

  void Kernel::SetGroupSize(const size_t* sizes)
  {
    for(size_t i = 0; i < dimension; ++i)
    {
      groupSize[i] = sizes[i];
    }
  }

  size_t* Kernel::GetWorkSize() const
  {
    size_t* data = nullptr;
    if(dimension != 0)
    {
      data = new size_t[dimension];
    }

    if(data != nullptr)
    {
      for(size_t i = 0; i < dimension; ++i)
      {
        data[i] = workSize[i];
      }
    }
    return data;
  }

  size_t* Kernel::GetWorkOffset() const
  {
    size_t* data = nullptr;
    if(dimension != 0)
    {
      data = new size_t[dimension];
    }

    if(data != nullptr)
    {
      for(size_t i = 0; i < dimension; ++i)
      {
        data[i] = workOffset[i];
      }
    }
    return data;
  }

  size_t* Kernel::GetGroupSize() const
  {
    size_t* data = nullptr;
    if(dimension != 0)
    {
      data = new size_t[dimension];
    }

    if(data != nullptr)
    {
      for(size_t i = 0; i < dimension; ++i)
      {
        data[i] = groupSize[i];
      }
    }
    return data;
  }
}
CS_PLUGIN_NAMESPACE_END(CL)
