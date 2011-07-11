#include "cssysdef.h"

#include "queue.h"
#include "context.h"
#include "event.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  Queue::~Queue()
  {
    cl_int error = clReleaseCommandQueue(queue);
    CS_ASSERT(error == CL_SUCCESS);
  }

  bool Queue::CheckError(cl_int error)
  {
    switch(error)
    {
    case CL_SUCCESS:
      // no error
      return true;

    case CL_INVALID_EVENT:
      // only valid for QueueWait
      // one of the passed events isn't valid
      // (e.g. comes from another queue that wasn't flushed)
    case CL_INVALID_COMMAND_QUEUE:
      // this object isn't valid
    case CL_OUT_OF_RESOURCES:
      // device OOM
    case CL_OUT_OF_HOST_MEMORY:
      // host OOM
    default:
      // driver bug
      return false;
    }
  }

  bool Queue::Flush()
  {
    cl_int error;
    error = clFlush(queue);
    return CheckError(error);
  }

  bool Queue::Finish()
  {
    cl_int error;
    error = clFinish(queue);
    return CheckError(error);
  }

  bool Queue::QueueBarrier()
  {
    cl_int error;
    error = clEnqueueBarrier(queue);
    return CheckError(error);
  }

  csPtr<Event> Queue::QueueMarker()
  {
    cl_int error;
    cl_event e;
    error = clEnqueueMarker(queue, &e);
    if(CheckError(error))
    {
      csRef<Event> ev;
      ev.AttachNew(new Event(context, e));
      return csPtr<Event>(ev);
    }

    return csPtr<Event>(nullptr);
  }

  bool Queue::QueueWait(const csRefArray<Event>& eventList)
  {
    if(eventList.IsEmpty())
    {
      return true;
    }

    cl_event* events = CreateEventHandleList(eventList, context);
    if(events == nullptr)
    {
      // OOM
      return false;
    }

    cl_int error;
    error = clEnqueueWaitForEvents(queue, eventList.GetSize(), events);

    delete [] events;
    return CheckError(error);
  }
}
CS_PLUGIN_NAMESPACE_END(CL)

