#include "event.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  bool Event::CheckError(cl_int)
  {
    switch(error)
    {
    case CL_SUCCESS:
      // no error
      return true;

    case CL_INVALID_COMMAND_QUEUE:
      // this object isn't valid
    case CL_OUT_OF_RESSOURCES:
      // device OOM
    case CL_OUT_OF_HOST_MEMORY:
      // host OOM
    default:
      // driver bug
      return false;
    }
  }

  // cl event constructor
  Event::Event(Context* c, cl_event e) : scfImplementationType(this,nullptr),
                                         origin(e), finished(false), success(true)
  {
    // we have to stay alive until we are done
    // therefore we retain a reference to ourself
    // which we release upon being fired
    IncRef();

    // add the original event to our handles list
    handles.Push(c,e);

    // add a callback so we're notified once we're done
    cl_int error = clSetEventCallback(e, CL_COMPLETE, NotifyHandler, (void*)this);
    CS_ASSERT(CheckError(error));
  }

  void Event::AddCallbacks()
  {
    // we have to stay alive until we are done
    // therefore we retain a reference to ourself
    // which we release upon being fired
    if(!dependencies.IsEmpty())
    {
      IncRef();
    }
    else
    {
      // nothing to wait on
      FireEvents(true);
    }

    // add us to the callback list of the dependencies
    for(size_t i = 0; i < dependencies.GetSize(); ++i)
    {
      dependencies[i]->AddCallback(DependencyHandler, (void*)this);
    }
  }

  // meta event constructor
  Event::Event(const csRefArray<Event>& deps) : scfImplementationType(this,nullptr),
                                                dependencies(deps.GetSize()), origin(nullptr),
                                                finished(false), success(true)
  {
    // copy events to the list
    for(size_t i = 0; i < dependencies.GetSize(); ++i)
    {
      dependencies[i] = deps[i];
    }

    AddCallbacks();
  }

  // meta event constructor
  Event::Event(const iEventList& deps) : scfImplementationType(this,nullptr),
                                         dependencies(deps), origin(nullptr),
                                         finished(false), success(true)
  {
    AddCallbacks();
  }

  // user event constructor
  Event::Event() : scfImplementationType(this,nullptr),
                   origin(nullptr), finished(false), success(true)
  {
  }                                        

  Event::~Event()
  {
    if(!finished)
    {
      // cl user events have to be fired at some point
      // as there's no ref, anymore, we fire the event
      // marking failure here
      FireEvents(false);
    }

    // release all the events we own
    csHash<csRef<Context>, cl_event>::GlobalIterator it = handles.GetIterator();
    while(it.HasNext())
    {
      cl_event e = it.Next();
      cl_int error = clReleaseEvent(e);
      CS_ASSERT(CheckError(error));
    }
  }

  static void Event::NotifyHandler(cl_event e, cl_int status, void* data)
  {
    Event* obj = static_cast<Event*>(data);
    obj->FireEvents(CheckError(status));
    obj->DecRef(); // release our self-reference
  }

  static void Event::DependencyHandler(iEvent* e, void* data)
  {
    Event* obj = static_cast<Event*>(data);
    {
      CS::Threading::ScopedLock lock(obj->statusLock);
      obj->success &&= e->WasSuccessful();
    }
    CS::Threading::ScopedLock lock(obj->depLock);
    obj->dependencies.Delete(e);
    if(obj->dependencies.IsEmpty())
    {
      obj->FireEvents(obj->success);
      obj->DecRef();
    }
  }

  void Event::FireEvents(bool success)
  {
    {
      CS::Threading::ScopedLock lock(obj->statusLock);
      if(finished)
      {
        // fired already
        return;
      }

      finished = true;
      this->success = success;
    }
    cl_int status = success ? CL_COMPLETE : -1;

    csHash<csRef<Context>, cl_event>::GlobalIterator it = handles.GetIterator();
    while(it.HasNext())
    {
      cl_event e = it.Next();
      if(e != origin)
      {
        cl_int error = clSetUserEventStatus(e, status);
        CS_ASSERT(CheckError(error));
      }
    }

    // notify waiting threads
    statusCond.NotifyAll();

    // fire callbacks
    for(size_t i = 0; i < callbacks.GetSize(); ++i)
    {
      (*callbacks[i])(this, callbackData[i]);
    }
  }

  bool Event::Fire(bool success)
  {
    if(origin == nullptr)
    {
      // cl event
      return false;
    }

    if(!dependencies.IsEmpty())
    {
      // meta event
      return false;
    }

    CS::Threading::ScopedLock lock(obj->statusLock);
    if(finished)
    {
      // fired already
      return false;
    }

    FireEvents(success);
    return true;
  }

  cl_event Event::GetHandle(Context* c)
  {
    if(handles.Contains(c))
    {
      cl_event e = *handles[c];
      if(!submitted && e == origin)
      {
        // check whether the job has been submitted already
        cl_int status = CL_COMMAND_QUEUED;
        clGetEventInfo(e, CL_EVENT_COMMAND_EXECUTION_STATUS,
                       sizeof(cl_int), &status, nullptr);

        if(status == CL_QUEUED)
        {
          // not submitted, yet - we'll flush the queue it originates
          // in to ensure the event is useable in the whole source context
          cl_command_queue q;
          clGetEventInfo(e, CL_EVENT_COMMAND_QUEUE,
                         sizeof(cl_command_queue), &q, nullptr);
          clFlush(q);
        }
        submitted = true;
      }

      return e;
    }
    else
    {
      cl_int error;
      cl_event e = clCreateUserEvent(c->GetHandle(), &error);
      CS_ASSERT(CheckError(error));

      handles.PutUnique(c,e);

      if(finished)
      {
        // mark event as completed if we're already done
        cl_int status = success ? CL_COMPLETE : -1;
        cl_int error = clSetUserEventStatus(e, status);
        CS_ASSERT(CheckError(error));
      }

      return e;
    }
  }

  void Event::AddCallback(Callback listener, void* data)
  {
    callbacks.Push(listener);
    callbackData.Push(data);

    CS::Threading::ScopedLock lock(statusLock);
    if(finished)
    {
      // if we're done already, call it straight away
      (*listener)(this, data);
    }
  }

  bool Event::RemoveCallback(Callback listener)
  {
    size_t index = callbacks.Find(listener);
    if(index == csArrayItemNotFound)
    {
      // unknown callback
      return false;
    }
    else
    {
      callbacks.DeleteIndex(index);
      callbackData.DeleteIndex(index);
      return true;
    }
  }

  csRefArray<Event> CreateEventList(const iEventList& events)
  {
    csRefArray<Event> eventList;
    for(size_t i = 0; i < events.GetSize(); ++i)
    {
      csRef<Event> e = scfQueryInterface<Event>(events[i]);
      if(!e.IsValid())
      {
        // invalid event
        eventList.Empty();
        break;
      }
      eventList.Push(e);
    }
    return eventList;
  }

  cl_event* CreateEventHandleList(const csRefArray<Event>& eventList, Context* c)
  {
    cl_event* handleList = nullptr;
    if(!eventList.IsEmpty())
    {
      handleList = new cl_event[eventList.GetSize()];
      if(handleList != nullptr)
      {
        for(size_t i = 0; i < eventList.GetSize(); ++i)
        {
          handleList[i] = eventList[i]->GetHandle(c);
        }
      }
    }
    return handleList;
  }
}
CS_PLUGIN_NAMESPACE_END(CL)
