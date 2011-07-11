#include "cssysdef.h"

#include "context.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  Context::~Context()
  {
    if(context != nullptr)
    {
      cl_int error = clReleaseContext(context);
      CS_ASSERT(error == CL_SUCCESS);
    }
  }

  void Context::NotifyHandler(const char* error, const void* data,
                              size_t dataSize, void* userData)
  {
    Context* c = static_cast<Context*>(userData);
    (void)c;
    // error is a printable string
    // data is dataSize in bytes and contains implementation specific data
    // which may be used for debugging purposes
    //@@@todo: find out how to determine whether the error invalidated the device or not
  }

  bool Context::Initialize()
  {
    if(devices.IsEmpty())
    {
      // no devices to use
      return false;
    }

    cl_device_id* deviceList = new cl_device_id[devices.GetSize()];
    if(deviceList == nullptr)
    {
      // OOM
      return false;
    }

    for(size_t i = 0; i < devices.GetSize(); ++i)
    {
      deviceList[i] = devices[i]->GetHandle();
    }

    cl_context_properties properties[] =
    {
      CL_CONTEXT_PLATFORM,
      (cl_context_properties)(platform->GetHandle()),
      0
    };

    cl_int error;
    context = clCreateContext(properties, devices.GetSize(), deviceList,
                              NotifyHandler, (void*)this, &error);
    switch(error)
    {
    case CL_SUCCESS:
      // context properly created
      break;

    case CL_INVALID_PLATFORM:
      // platform wasn't valid
    case CL_INVALID_PROPERTY:
      // invalid property passed or
      // valid property passed multiple times
    case CL_INVALID_VALUE:
      // devices not valid or
      // device list empty or
      // no data passed for notify or
      // invalid device in device list
    case CL_DEVICE_NOT_AVAILABLE:
      // one of the devices isn't available
    case CL_OUT_OF_RESOURCES:
      // OOM on one of the devices
    case CL_OUT_OF_HOST_MEMORY:
      // host OOM
    default:
      // driver bug
      delete [] deviceList;
      return false;
    }

    // try to find a device with out of order support
    // for the main queue
    for(size_t i = 0; i < devices.GetSize(); ++i)
    {
      if(devices[i]->OutOfOrderSupport())
      {
        contextQueue = CreateQueue(devices[i]);
        break;
      }
    }

    // just use some device if there's no suitable one
    if(!contextQueue.IsValid())
    {
      contextQueue = CreateQueue(devices[0]);
    }

    return contextQueue.IsValid();
  }

  csPtr<Queue> Context::CreateQueue(Device* d)
  {
    if(!devices.Contains(d))
    {
      return csPtr<Queue>(nullptr);
    }

    cl_command_queue_properties properties = 0;
    if(d->OutOfOrderSupport())
    {
      properties |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
    }

    if(d->ProfilingSupport() && false) //@@@todo: determine whether we want to profile or not
    {
      properties |= CL_QUEUE_PROFILING_ENABLE;
    }

    cl_int error;
    cl_command_queue queue = clCreateCommandQueue(context, d->GetHandle(), properties, &error);
    switch(error)
    {
    case CL_SUCCESS:
      // no error
      break;

    case CL_INVALID_VALUE:
      // properties aren't valid
    case CL_INVALID_QUEUE_PROPERTIES:
      // one of the properties isn't supported
      // try again without out-of-order/profiling?
    case CL_INVALID_CONTEXT:
      // this object isn't valid
      // either Notify or Initialize failed
    case CL_INVALID_DEVICE:
      // device is either invalid or doesn't belong to this context
    case CL_OUT_OF_RESOURCES:
      // device OOM
    case CL_OUT_OF_HOST_MEMORY:
      // host OOM
    default:
      // driver bug
      return csPtr<Queue>(nullptr);
    }

    csRef<Queue> q;
    q.AttachNew(new Queue(this, d, queue));
    queues.Push(q);
    return csPtr<Queue>(q);
  }
}
CS_PLUGIN_NAMESPACE_END(CL)
