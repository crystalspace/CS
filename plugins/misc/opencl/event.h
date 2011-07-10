#ifndef __CS_OPENCL_EVENT_IMPL_H__
#define __CS_OPENCL_EVENT_IMPL_H__

#include <ivaria/clmanager.h>
#include <ivaria/clconsts.h>
#include <csutil/threading/mutex.h>
#include "context.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  class Event : public scfImplementation1<Event,iEvent>
  {
  public:
    Event(Context*, cl_event); // cl event
    Event(const csRefArray<Event>&); // meta event
    Event(const iEventList&); // meta event
    Event(); // user-created event
    ~Event();

    cl_event GetHandle(Context*);
    void SetResult(iBase* obj)
    {
      result = obj;
    }

    // iEvent
    bool IsFinished()
    {
      return finished;
    }

    bool WasSuccessful()
    {
      return success;
    }

    bool Wait()
    {
      statusLock.Lock();
      if(!IsFinished())
      {
        // wait for the event to finish
        statusCond.Wait(statusLock);
      }
      statusLock.Unlock();

      return success;
    }

    bool Fire(bool success);

    iBase* GetResult()
    {
      return result;
    }

    void AddCallback(Callback listener, void* data);
    bool RemoveCallback(Callback listener);

  private:
    // handles to actual event objects in the contexts
    csHash<csRef<Context>, cl_event> handles;

    // events this event depends on
    iEventList dependencies;
    CS::Threading::Mutex depLock;

    // callback data
    csArray<Callback> callbacks;
    csArray<void*> callbackData;

    // source event if this is created from a cl event
    cl_event origin;
    bool submitted;

    // execution status
    CS::Threading::Mutex statusLock;
    CS::Threading::Condition statusCond;
    bool finished;
    bool success;

    // result if there is any
    csRef<iBase> result;

    // helper function that validates cl errors
    bool CheckError(cl_int);

    // helper function for the meta-event constructor
    void AddCallbacks();

    // complete the user events
    void FireEvents(bool);

    // callback from cl event
    static void NotifyHandler(cl_event, cl_int, void*);

    // callback from dependencies
    static void DependencyHandler(iEvent*, void*);
  };

  // little helper functions
  csRefArray<Event> CreateEventList(const iEventList&);
  cl_event* CreateEventHandleList(const csRefArray<Event>&, Context*);
}
CS_PLUGIN_NAMESPACE_END(CL)

#endif // __CS_OPENCL_EVENT_IMPL_H__