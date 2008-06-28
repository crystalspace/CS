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

#include "csutil/threadevent.h"
#include "csutil/threadjobqueue.h"

using namespace CS::Threading;

class CS_CRYSTALSPACE_EXPORT ThreadManager
{
public:
  ThreadManager();
  ~ThreadManager();

  template<class T>
  void QueueEvent(ThreadedCallable<T>* object, void (T::*method)(void))
  {
      csRef<ThreadEvent<T> > threadEvent;
      threadEvent.AttachNew(new ThreadEvent<T>(object, method));
      queue->Enqueue(static_cast<iJob*>(threadEvent));
  }

  template<class T, typename A1>
  void QueueEvent(ThreadedCallable<T>* object, void (T::*method)(A1), csArray<void const*> args)
  {
      csRef<ThreadEvent1<T, A1> > threadEvent;
      threadEvent.AttachNew(new ThreadEvent1<T, A1>(object, method, args));
      queue->Enqueue(static_cast<iJob*>(threadEvent));
  }

  template<class T, typename A1, typename A2>
  void QueueEvent(ThreadedCallable<T>* object, void (T::*method)(A1, A2), csArray<void const*> args)
  {
      csRef<ThreadEvent2<T, A1, A2> > threadEvent;
      threadEvent.AttachNew(new ThreadEvent2<T, A1, A2>(object, method, args));
      queue->Enqueue(static_cast<iJob*>(threadEvent));
  }

  static ThreadManager* GetThreadManager();

private:
  csRef<ThreadedJobQueue> queue;
  static ThreadManager* singletonPtr;
};

#define TC_STORE(type, var) \
  args.Push(mempool->Store<type>((type)var));

#define THREADED_CALLABLE_DEC(type, function) \
  void function##TC(); \
  void function() \
  { \
    ThreadManager::GetThreadManager()->QueueEvent<type>(GetThreadedCallable(), &type::function##TC); \
  }

#define THREADED_CALLABLE_IMP(type, function) \
  void type::function##TC()

#define THREADED_CALLABLE_DEC1(type, function, T1, A1) \
  void function##TC(T1 A1); \
  void function(T1 A1) \
  { \
    csArray<void const*> args; \
    TEventMemPool *mempool = new TEventMemPool; \
    args.Push(mempool); \
    TC_STORE(T1, A1); \
    ThreadManager::GetThreadManager()->QueueEvent<type, T1>(GetThreadedCallable(), &type::function##TC, args); \
  }

#define THREADED_CALLABLE_IMP1(type, function, A1) \
  void type::function##TC(A1)

#define THREADED_CALLABLE_DEC2(type, function, T1, A1, T2, A2) \
  void function##TC(T1 A1, T2 A2); \
  void function(T1 A1, T2 A2) \
  { \
    csArray<void const*> args; \
    TEventMemPool *mempool = new TEventMemPool; \
    args.Push(mempool); \
    TC_STORE(T1, A1); \
    TC_STORE(T2, A2); \
    ThreadManager::GetThreadManager()->QueueEvent<type, T1, T2>(GetThreadedCallable(), &type::function##TC, args); \
  }

#define THREADED_CALLABLE_IMP2(type, function, A1, A2) \
  void type::function##TC(A1, A2)

#define THREADED_CALLABLE_DEC9(type, function, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, T8, A8, T9, A9) \
  void function##TC(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9); \
  void function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9) \
  { \
    csArray<void const*> args; \
    EventMemPool *mempool = new EventMemPool; \
    TC_STORE(void const*, mempool); \
    TC_STORE(T1, A1); \
    TC_STORE(T2, A2); \
    TC_STORE(T3, A3); \
    TC_STORE(T4, A4); \
    TC_STORE(T5, A5); \
    TC_STORE(T6, A6); \
    TC_STORE(T7, A7); \
    TC_STORE(T8, A8); \
    TC_STORE(T9, A9); \
    ThreadManager::GetThreadManager()->QueueEvent<type>(GetThreadedCallable(), &type::function##TC, args); \
  }

#endif // __CS_IUTIL_THREADMANAGER_H__
