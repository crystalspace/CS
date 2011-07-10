#ifndef __CS_OPENCL_QUEUE_IMPL_H__
#define __CS_OPENCL_QUEUE_IMPL_H__

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  class Queue
  {
  public:
    Queue(Context* c, Device* d, cl_command_queue q) : queue(q),
                                                       context(c),
                                                       device(d)
    {
    }

    ~Queue()
    {
      cl_int error = clReleaseCommandQueue(queue);
      CS_ASSERT(error == CL_SUCCESS);
    }

    Context* GetContext() const
    {
      return context;
    }

    Device* GetDevice() const
    {
      return device;
    }

    cl_command_queue GetHandle() const
    {
      return queue;
    }

    bool Flush();
    bool Finish();

    csPtr<Event> QueueMarker();
    bool QueueBarrier();
    bool QueueWait(const csRefArray<Event>&);

  private:
    cl_command_queue queue;
    csRef<Context> context;
    csRef<Device> device;

    bool CheckError(cl_int);
  };
}
CS_PLUGIN_NAMESPACE_END(CL)

#endif // __CS_OPENCL_QUEUE_IMPL_H__
