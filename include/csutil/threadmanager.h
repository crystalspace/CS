/*
  Copyright (C) 2008 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSUTIL_THREADMANAGER_H__
#define __CS_CSUTIL_THREADMANAGER_H__

#include "csutil/listaccessqueue.h"
#include "csutil/objreg.h"
#include "csutil/threadevent.h"
#include "csutil/threadjobqueue.h"
#include "iutil/eventh.h"
#include "iutil/threadmanager.h"

struct iEvent;
struct iEventQueue;

using namespace CS::Threading;

namespace CS
{
  namespace Utility
  {
    class CS_CRYSTALSPACE_EXPORT csThreadManager : public scfImplementation2<csThreadManager,
      iThreadManager,
      iEventHandler>
    {
    public:
      csThreadManager(iObjectRegistry* objReg);

      bool HandleEvent(iEvent&);
      CS_EVENTHANDLER_NAMES("crystalspace.threadmanager")
      CS_EVENTHANDLER_NIL_CONSTRAINTS

      inline void Process(uint num = 1)
      {
        listQueue->ProcessQueue(num);
      }

    private:
      void PushToQueue(bool useThreadQueue, iJob* job)
      {
        if(useThreadQueue)
        {
          threadQueue->Enqueue(job);
        }
        else
        {
          listQueue->Enqueue(job);
        }
      }

      iObjectRegistry* objectReg;
      csRef<ThreadedJobQueue> threadQueue;
      csRef<ListAccessQueue> listQueue;
      csEventID ProcessQueue;
      csRef<iEventQueue> eventQueue;
    };

    struct ThreadReturn : public csRefCount
    {
      bool finished;
      void* result;

      ThreadReturn()
      {
        finished = false;
        result = NULL;
      }
    };

    template<class T>
    void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(void), bool useThreadQueue = true)
    {
      csRef<ThreadEvent<T> > threadEvent;
      threadEvent.AttachNew(new ThreadEvent<T>(object, method));
      tm->PushToQueue(useThreadQueue, threadEvent);
    }

    template<class T, typename A1>
    void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1), void const** &args, bool useThreadQueue = true)
    {
      csRef<ThreadEvent1<T, A1> > threadEvent;
      threadEvent.AttachNew(new ThreadEvent1<T, A1>(object, method, args));
      tm->PushToQueue(useThreadQueue, threadEvent);
    }

    template<class T, typename A1, typename A2>
    void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2), void const** &args, bool useThreadQueue = true)
    {
      csRef<ThreadEvent2<T, A1, A2> > threadEvent;
      threadEvent.AttachNew(new ThreadEvent2<T, A1, A2>(object, method, args));
      tm->PushToQueue(useThreadQueue, threadEvent);
    }

    template<class T, typename A1, typename A2, typename A3>
    void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3), void const** &args, bool useThreadQueue = true)
    {
      csRef<ThreadEvent3<T, A1, A2, A3> > threadEvent;
      threadEvent.AttachNew(new ThreadEvent3<T, A1, A2, A3>(object, method, args));
      tm->PushToQueue(useThreadQueue, threadEvent);
    }

    template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
    void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), void const** &args, bool useThreadQueue = true)
    {
      csRef<ThreadEvent10<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> > threadEvent;
      threadEvent.AttachNew(new ThreadEvent10<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>(object, method, args));
      tm->PushToQueue(useThreadQueue, threadEvent);
    }

#define TC_STORE(type, var) \
    args.Push(mempool->Store<type>((type)var));

#define THREADED_CALLABLE_DECL(type, function, useThreadQueue) \
  void function##TC(); \
  void function() \
    { \
      const type* objTC = const_cast<const type*>(this); \
      objTC->function(); \
    } \
  void function() const \
    { \
      csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
      CS_ASSERT(tm.IsValid()); \
      CS::Utility::QueueEvent<type>(tm, (ThreadedCallable*)this, &type::function##TC, useThreadQueue); \
    }

#define THREADED_CALLABLE_IMPL(type, function) \
  void type::function##TC()

#define THREADED_CALLABLE_DECL1(type, function, T1, A1, useThreadQueue) \
  void function##TC(T1 A1); \
  void function(T1 A1) \
    { \
      const type* objTC = const_cast<const type*>(this); \
      objTC->function(A1); \
    } \
  void function(T1 A1) const \
    { \
      void const** args = new void const*[2]; \
      TEventMemPool *mempool = new TEventMemPool; \
      args[0] = mempool; \
      args[1] = mempool->Store<T1>((T1)A1); \
      csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
      CS_ASSERT(tm.IsValid()); \
      CS::Utility::QueueEvent<type, T1>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
    }

#define THREADED_CALLABLE_IMPL1(type, function, A1) \
  void type::function##TC(A1)

#define THREADED_CALLABLE_DECL2(type, function, T1, A1, T2, A2, useThreadQueue) \
  void function##TC(T1 A1, T2 A2); \
  void function(T1 A1, T2 A2) \
    { \
      const type* objTC = const_cast<const type*>(this); \
      objTC->function(A1, A2); \
    } \
  void function(T1 A1, T2 A2) const \
    { \
      void const** args = new void const*[3]; \
      TEventMemPool *mempool = new TEventMemPool; \
      args[0] = mempool; \
      args[1] = mempool->Store<T1>((T1)A1); \
      args[2] = mempool->Store<T2>((T2)A2); \
      csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
      CS_ASSERT(tm.IsValid()); \
      CS::Utility::QueueEvent<type, T1, T2>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
    }

#define THREADED_CALLABLE_IMPL2(type, function, A1, A2) \
  void type::function##TC(A1, A2)

#define THREADED_CALLABLE_DECL3(type, function, T1, A1, T2, A2, T3, A3, useThreadQueue) \
  void function##TC(T1 A1, T2 A2, T3 A3); \
  void function(T1 A1, T2 A2, T3 A3) \
    { \
      const type* objTC = const_cast<const type*>(this); \
      objTC->function(A1, A2, A3); \
    } \
  void function(T1 A1, T2 A2, T3 A3) const \
    { \
      void const** args = new void const*[4]; \
      TEventMemPool *mempool = new TEventMemPool; \
      args[0] = mempool; \
      args[1] = mempool->Store<T1>((T1)A1); \
      args[2] = mempool->Store<T2>((T2)A2); \
      args[3] = mempool->Store<T3>((T3)A3); \
      csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
      CS_ASSERT(tm.IsValid()); \
      CS::Utility::QueueEvent<type, T1, T2, T3>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
    }

#define THREADED_CALLABLE_IMPL3(type, function, A1, A2, A3) \
  void type::function##TC(A1, A2, A3)

#define THREADED_CALLABLE_DECL10(type, function, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, T8, A8, T9, A9, T10, A10, useThreadQueue) \
  void function##TC(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10); \
  void function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10) \
    { \
      const type* objTC = const_cast<const type*>(this); \
      objTC->function(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10); \
    } \
  void function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10) const \
    { \
      void const** args = new void const*[11]; \
      TEventMemPool *mempool = new TEventMemPool; \
      args[0] = mempool; \
      args[1] = mempool->Store<T1>((T1)A1); \
      args[2] = mempool->Store<T2>((T2)A2); \
      args[3] = mempool->Store<T3>((T3)A3); \
      args[4] = mempool->Store<T4>((T4)A4); \
      args[5] = mempool->Store<T5>((T5)A5); \
      args[6] = mempool->Store<T6>((T6)A6); \
      args[7] = mempool->Store<T7>((T7)A7); \
      args[8] = mempool->Store<T8>((T8)A8); \
      args[9] = mempool->Store<T9>((T9)A9); \
      args[10] = mempool->Store<T10>((T10)A10); \
      csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
      CS_ASSERT(tm.IsValid()); \
      CS::Utility::QueueEvent<type, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
    }

#define THREADED_CALLABLE_IMPL10(type, function, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) \
    void type::function##TC(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)

  }
}

#endif // __CS_IUTIL_THREADMANAGER_H__
