#ifndef __CS_OPENCL_CONTEXT_IMPL_H__
#define __CS_OPENCL_CONTEXT_IMPL_H__

#include <ivaria/csconsts.h>
#include "platform.h"
#include "device.h"
#include "queue.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  class Context
  {
  public:
    Context(Platform* p,const csRefArray<Device>& devices) : context(nullptr),
                                                             platform(p),
                                                             devices(devices)
    {
    }

    ~Context()
    {
      if(context != nullptr)
      {
        cl_int error = clReleaseContext(context);
        CS_ASSERT(error == CL_SUCCESS);
      }
    }

    bool Initialize();

    cl_context GetHandle() const
    {
      return context;
    }

    Queue* GetQueue() const
    {
      return contextQueue;
    }

    csPtr<Queue> CreateQueue(Device*);

  private:
    cl_context context;
    csRef<Platform> platform;
    csRefArray<Device> devices;

    // extra queue for "maintaince" tasks
    // e.g. moving buffers
    csRef<Queue> contextQueue;

    // list of associated objects
    csWeakRefArray<Queue> queues;

    static void NotifyHandler(const char*, const void*, size_t, void*);
  };
}
CS_PLUGIN_NAMESPACE_END(CL)

#endif // __CS_OPENCL_CONTEXT_IMPL_H__
