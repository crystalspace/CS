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
#include "imap/loader.h"
#include "iutil/eventh.h"
#include "iutil/threadmanager.h"

struct iEngine;
struct iEvent;
struct iEventQueue;

using namespace CS::Threading;

class CS_CRYSTALSPACE_EXPORT csThreadManager : public scfImplementation2<csThreadManager,
  iThreadManager,
  iEventHandler>
{
public:
  csThreadManager(iObjectRegistry* objReg);

  bool HandleEvent(iEvent&);
  CS_EVENTHANDLER_NAMES("crystalspace.threadmanager")
  CS_EVENTHANDLER_NIL_CONSTRAINTS

  void Process(uint num = 1);
  void Wait(csRef<iThreadReturn> ret);

  void PushToQueue(bool useThreadQueue, iJob* job)
  {
    if(useThreadQueue)
    {
      CS::Threading::MutexScopedLock lock(queuePushLock);
      if(waiting == threadCount-1)
      {
        job->Run();
      }
      else
      {
        threadQueue->Enqueue(job);
      }
    }
    else
    {
      listQueue->Enqueue(job);
    }
  }

  private:

  int32 waiting;
  int32 threadCount;

  Mutex queuePushLock;

  iObjectRegistry* objectReg;
  csRef<ThreadedJobQueue> threadQueue;
  csRef<ListAccessQueue> listQueue;
  csEventID ProcessPerFrame;
  csRef<iEventQueue> eventQueue;
  csRef<iThreadedLoader> loader;
  csRef<iEngine> engine;
};

class csThreadReturn : public iThreadReturn
{
public:
  csThreadReturn()
  {
    finished = false;
    result = NULL;
  }

  bool IsFinished() { return finished; }

  bool WasSuccessful() { return true; }

  void GetResult(void* result) { result = this->result; }

  void GetResult(csRef<iBase>*) { return; }

  void MarkFinished() { finished = true; }
  void MarkSuccessful() { return; }
  void SetResult(void* result) { this->result = result; }
  void SetResult(csRef<iBase> result) { return; }

  void Copy(iThreadReturn* other)
  {
    other->GetResult(result);
    finished = other->IsFinished();
  }

private:
  bool finished;
  void* result;
};

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

template<class T, typename A1, typename A2, typename A3, typename A4>
void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4), void const** &args, bool useThreadQueue = true)
{
  csRef<ThreadEvent4<T, A1, A2, A3, A4> > threadEvent;
  threadEvent.AttachNew(new ThreadEvent4<T, A1, A2, A3, A4>(object, method, args));
  tm->PushToQueue(useThreadQueue, threadEvent);
}

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5>
void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4, A5), void const** &args, bool useThreadQueue = true)
{
  csRef<ThreadEvent5<T, A1, A2, A3, A4, A5> > threadEvent;
  threadEvent.AttachNew(new ThreadEvent5<T, A1, A2, A3, A4, A5>(object, method, args));
  tm->PushToQueue(useThreadQueue, threadEvent);
}

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4, A5, A6), void const** &args, bool useThreadQueue = true)
{
  csRef<ThreadEvent6<T, A1, A2, A3, A4, A5, A6> > threadEvent;
  threadEvent.AttachNew(new ThreadEvent6<T, A1, A2, A3, A4, A5, A6>(object, method, args));
  tm->PushToQueue(useThreadQueue, threadEvent);
}

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4, A5, A6, A7), void const** &args, bool useThreadQueue = true)
{
  csRef<ThreadEvent7<T, A1, A2, A3, A4, A5, A6, A7> > threadEvent;
  threadEvent.AttachNew(new ThreadEvent7<T, A1, A2, A3, A4, A5, A6, A7>(object, method, args));
  tm->PushToQueue(useThreadQueue, threadEvent);
}

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8), void const** &args, bool useThreadQueue = true)
{
  csRef<ThreadEvent8<T, A1, A2, A3, A4, A5, A6, A7, A8> > threadEvent;
  threadEvent.AttachNew(new ThreadEvent8<T, A1, A2, A3, A4, A5, A6, A7, A8>(object, method, args));
  tm->PushToQueue(useThreadQueue, threadEvent);
}

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9), void const** &args, bool useThreadQueue = true)
{
  csRef<ThreadEvent9<T, A1, A2, A3, A4, A5, A6, A7, A8, A9> > threadEvent;
  threadEvent.AttachNew(new ThreadEvent9<T, A1, A2, A3, A4, A5, A6, A7, A8, A9>(object, method, args));
  tm->PushToQueue(useThreadQueue, threadEvent);
}

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), void const** &args, bool useThreadQueue = true)
{
  csRef<ThreadEvent10<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> > threadEvent;
  threadEvent.AttachNew(new ThreadEvent10<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>(object, method, args));
  tm->PushToQueue(useThreadQueue, threadEvent);
}

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11), void const** &args, bool useThreadQueue = true)
{
  csRef<ThreadEvent11<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> > threadEvent;
  threadEvent.AttachNew(new ThreadEvent11<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>(object, method, args));
  tm->PushToQueue(useThreadQueue, threadEvent);
}

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12), void const** &args, bool useThreadQueue = true)
{
  csRef<ThreadEvent12<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> > threadEvent;
  threadEvent.AttachNew(new ThreadEvent12<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>(object, method, args));
  tm->PushToQueue(useThreadQueue, threadEvent);
}

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13), void const** &args, bool useThreadQueue = true)
{
  csRef<ThreadEvent13<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13> > threadEvent;
  threadEvent.AttachNew(new ThreadEvent13<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13>(object, method, args));
  tm->PushToQueue(useThreadQueue, threadEvent);
}

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14), void const** &args, bool useThreadQueue = true)
{
  csRef<ThreadEvent14<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14> > threadEvent;
  threadEvent.AttachNew(new ThreadEvent14<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14>(object, method, args));
  tm->PushToQueue(useThreadQueue, threadEvent);
}

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
void QueueEvent(iThreadManager* tm, ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15), void const** &args, bool useThreadQueue = true)
{
  csRef<ThreadEvent15<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15> > threadEvent;
  threadEvent.AttachNew(new ThreadEvent15<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15>(object, method, args));
  tm->PushToQueue(useThreadQueue, threadEvent);
}

#define THREADED_CALLABLE_DECL(type, function, returnClass, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret); \
  iThreadReturn* function() \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(); \
} \
  iThreadReturn* function() const \
{ \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  void const** args = new void const*[2]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  QueueEvent<type, csRef<iThreadReturn> >(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL1(type, function, returnClass, T1, A1, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1); \
  iThreadReturn* function(T1 A1) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1); \
} \
  iThreadReturn* function(T1 A1) const \
{ \
  void const** args = new void const*[3]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn> , T1>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL2(type, function, returnClass, T1, A1, T2, A2, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2); \
  iThreadReturn* function(T1 A1, T2 A2) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2); \
} \
  iThreadReturn* function(T1 A1, T2 A2) const \
{ \
  void const** args = new void const*[4]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn> , T1, T2>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL3(type, function, returnClass, T1, A1, T2, A2, T3, A3, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2, T3 A3); \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2, A3); \
} \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3) const \
{ \
  void const** args = new void const*[5]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  args[4] = mempool->Store<T3>((T3)A3); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn>, T1, T2, T3>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL4(type, function, returnClass, T1, A1, T2, A2, T3, A3, T4, A4, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2, T3 A3, T4 A4); \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2, A3, A4); \
} \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4) const \
{ \
  void const** args = new void const*[6]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  args[4] = mempool->Store<T3>((T3)A3); \
  args[5] = mempool->Store<T4>((T4)A4); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn>, T1, T2, T3, T4>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL5(type, function, returnClass, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2, T3 A3, T4 A4, T5 A5); \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2, A3, A4, A5); \
} \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5) const \
{ \
  void const** args = new void const*[7]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  args[4] = mempool->Store<T3>((T3)A3); \
  args[5] = mempool->Store<T4>((T4)A4); \
  args[6] = mempool->Store<T5>((T5)A5); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn>, T1, T2, T3, T4, T5>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL6(type, function, returnClass, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6); \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2, A3, A4, A5, A6); \
} \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6) const \
{ \
  void const** args = new void const*[8]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  args[4] = mempool->Store<T3>((T3)A3); \
  args[5] = mempool->Store<T4>((T4)A4); \
  args[6] = mempool->Store<T5>((T5)A5); \
  args[7] = mempool->Store<T6>((T6)A6); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn>, T1, T2, T3, T4, T5, T6>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL7(type, function, returnClass, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7); \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2, A3, A4, A5, A6, A7); \
} \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7) const \
{ \
  void const** args = new void const*[9]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  args[4] = mempool->Store<T3>((T3)A3); \
  args[5] = mempool->Store<T4>((T4)A4); \
  args[6] = mempool->Store<T5>((T5)A5); \
  args[7] = mempool->Store<T6>((T6)A6); \
  args[8] = mempool->Store<T7>((T7)A7); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn>, T1, T2, T3, T4, T5, T6, T7>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL8(type, function, returnClass, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, T8, A8, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8); \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2, A3, A4, A5, A6, A7, A8); \
} \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8) const \
{ \
  void const** args = new void const*[10]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  args[4] = mempool->Store<T3>((T3)A3); \
  args[5] = mempool->Store<T4>((T4)A4); \
  args[6] = mempool->Store<T5>((T5)A5); \
  args[7] = mempool->Store<T6>((T6)A6); \
  args[8] = mempool->Store<T7>((T7)A7); \
  args[9] = mempool->Store<T8>((T8)A8); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn>, T1, T2, T3, T4, T5, T6, T7, T8>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL9(type, function, returnClass, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, T8, A8, T9, A9, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9); \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2, A3, A4, A5, A6, A7, A8, A9); \
} \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9) const \
{ \
  void const** args = new void const*[11]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  args[4] = mempool->Store<T3>((T3)A3); \
  args[5] = mempool->Store<T4>((T4)A4); \
  args[6] = mempool->Store<T5>((T5)A5); \
  args[7] = mempool->Store<T6>((T6)A6); \
  args[8] = mempool->Store<T7>((T7)A7); \
  args[9] = mempool->Store<T8>((T8)A8); \
  args[10] = mempool->Store<T9>((T9)A9); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn>, T1, T2, T3, T4, T5, T6, T7, T8, T9>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL10(type, function, returnClass, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, T8, A8, T9, A9, T10, A10, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10); \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10); \
} \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10) const \
{ \
  void const** args = new void const*[12]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  args[4] = mempool->Store<T3>((T3)A3); \
  args[5] = mempool->Store<T4>((T4)A4); \
  args[6] = mempool->Store<T5>((T5)A5); \
  args[7] = mempool->Store<T6>((T6)A6); \
  args[8] = mempool->Store<T7>((T7)A7); \
  args[9] = mempool->Store<T8>((T8)A8); \
  args[10] = mempool->Store<T9>((T9)A9); \
  args[11] = mempool->Store<T10>((T10)A10); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn>, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL11(type, function, returnClass, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, T8, A8, T9, A9, T10, A10, T11, A11, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11); \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11); \
} \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11) const \
{ \
  void const** args = new void const*[13]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  args[4] = mempool->Store<T3>((T3)A3); \
  args[5] = mempool->Store<T4>((T4)A4); \
  args[6] = mempool->Store<T5>((T5)A5); \
  args[7] = mempool->Store<T6>((T6)A6); \
  args[8] = mempool->Store<T7>((T7)A7); \
  args[9] = mempool->Store<T8>((T8)A8); \
  args[10] = mempool->Store<T9>((T9)A9); \
  args[11] = mempool->Store<T10>((T10)A10); \
  args[12] = mempool->Store<T11>((T11)A11); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn>, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL12(type, function, returnClass, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, T8, A8, T9, A9, T10, A10, T11, A11, T12, A12, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11, T12 A12); \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11, T12 A12) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12); \
} \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11, T12 A12) const \
{ \
  void const** args = new void const*[14]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  args[4] = mempool->Store<T3>((T3)A3); \
  args[5] = mempool->Store<T4>((T4)A4); \
  args[6] = mempool->Store<T5>((T5)A5); \
  args[7] = mempool->Store<T6>((T6)A6); \
  args[8] = mempool->Store<T7>((T7)A7); \
  args[9] = mempool->Store<T8>((T8)A8); \
  args[10] = mempool->Store<T9>((T9)A9); \
  args[11] = mempool->Store<T10>((T10)A10); \
  args[12] = mempool->Store<T11>((T11)A11); \
  args[13] = mempool->Store<T12>((T12)A12); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn>, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL13(type, function, returnClass, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, T8, A8, T9, A9, T10, A10, T11, A11, T12, A12, T13, A13, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11, T12 A12, T13 A13); \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11, T12 A12, T13 A13) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13); \
} \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11, T12 A12, T13 A13) const \
{ \
  void const** args = new void const*[15]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  args[4] = mempool->Store<T3>((T3)A3); \
  args[5] = mempool->Store<T4>((T4)A4); \
  args[6] = mempool->Store<T5>((T5)A5); \
  args[7] = mempool->Store<T6>((T6)A6); \
  args[8] = mempool->Store<T7>((T7)A7); \
  args[9] = mempool->Store<T8>((T8)A8); \
  args[10] = mempool->Store<T9>((T9)A9); \
  args[11] = mempool->Store<T10>((T10)A10); \
  args[12] = mempool->Store<T11>((T11)A11); \
  args[13] = mempool->Store<T12>((T12)A12); \
  args[14] = mempool->Store<T13>((T13)A13); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn>, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL14(type, function, returnClass, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, T8, A8, T9, A9, T10, A10, T11, A11, T12, A12, T13, A13, T14, A14, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11, T12 A12, T13 A13, T14 A14); \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11, T12 A12, T13 A13, T14 A14) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14); \
} \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11, T12 A12, T13 A13, T14 A14) const \
{ \
  void const** args = new void const*[16]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  args[4] = mempool->Store<T3>((T3)A3); \
  args[5] = mempool->Store<T4>((T4)A4); \
  args[6] = mempool->Store<T5>((T5)A5); \
  args[7] = mempool->Store<T6>((T6)A6); \
  args[8] = mempool->Store<T7>((T7)A7); \
  args[9] = mempool->Store<T8>((T8)A8); \
  args[10] = mempool->Store<T9>((T9)A9); \
  args[11] = mempool->Store<T10>((T10)A10); \
  args[12] = mempool->Store<T11>((T11)A11); \
  args[13] = mempool->Store<T12>((T12)A12); \
  args[14] = mempool->Store<T13>((T13)A13); \
  args[15] = mempool->Store<T14>((T14)A14); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn>, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_DECL15(type, function, returnClass, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, T8, A8, T9, A9, T10, A10, T11, A11, T12, A12, T13, A13, T14, A14, T15, A15, useThreadQueue, wait) \
  void function##TC(csRef<iThreadReturn> ret, T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11, T12 A12, T13 A13, T14 A14, T15 A15); \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11, T12 A12, T13 A13, T14 A14, T15 A15) \
{ \
  const type* objTC = const_cast<const type*>(this); \
  return objTC->function(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15); \
} \
  iThreadReturn* function(T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10, T11 A11, T12 A12, T13 A13, T14 A14, T15 A15) const \
{ \
  void const** args = new void const*[17]; \
  TEventMemPool *mempool = new TEventMemPool; \
  args[0] = mempool; \
  csRef<iThreadReturn> ret; \
  ret.AttachNew(new returnClass); \
  args[1] = mempool->Store<csRef<iThreadReturn> >(ret); \
  args[2] = mempool->Store<T1>((T1)A1); \
  args[3] = mempool->Store<T2>((T2)A2); \
  args[4] = mempool->Store<T3>((T3)A3); \
  args[5] = mempool->Store<T4>((T4)A4); \
  args[6] = mempool->Store<T5>((T5)A5); \
  args[7] = mempool->Store<T6>((T6)A6); \
  args[8] = mempool->Store<T7>((T7)A7); \
  args[9] = mempool->Store<T8>((T8)A8); \
  args[10] = mempool->Store<T9>((T9)A9); \
  args[11] = mempool->Store<T10>((T10)A10); \
  args[12] = mempool->Store<T11>((T11)A11); \
  args[13] = mempool->Store<T12>((T12)A12); \
  args[14] = mempool->Store<T13>((T13)A13); \
  args[15] = mempool->Store<T14>((T14)A14); \
  args[16] = mempool->Store<T15>((T15)A15); \
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager>(GetObjectRegistry()); \
  CS_ASSERT(tm.IsValid()); \
  QueueEvent<type, csRef<iThreadReturn>, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15>(tm, (ThreadedCallable*)this, &type::function##TC, args, useThreadQueue); \
  if(wait) \
  { \
    tm->Wait(ret); \
  } \
  return ret; \
}

#define THREADED_CALLABLE_IMPL(type, function) \
  void type::function##TC(csRef<iThreadReturn> ret)

#define THREADED_CALLABLE_IMPL1(type, function, A1) \
  void type::function##TC(csRef<iThreadReturn> ret, A1)

#define THREADED_CALLABLE_IMPL2(type, function, A1, A2) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2)

#define THREADED_CALLABLE_IMPL3(type, function, A1, A2, A3) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2, A3)

#define THREADED_CALLABLE_IMPL4(type, function, A1, A2, A3, A4) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2, A3, A4)

#define THREADED_CALLABLE_IMPL5(type, function, A1, A2, A3, A4, A5) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2, A3, A4, A5)

#define THREADED_CALLABLE_IMPL6(type, function, A1, A2, A3, A4, A5, A6) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2, A3, A4, A5, A6)

#define THREADED_CALLABLE_IMPL7(type, function, A1, A2, A3, A4, A5, A6, A7) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2, A3, A4, A5, A6, A7)

#define THREADED_CALLABLE_IMPL8(type, function, A1, A2, A3, A4, A5, A6, A7, A8) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2, A3, A4, A5, A6, A7, A8)

#define THREADED_CALLABLE_IMPL9(type, function, A1, A2, A3, A4, A5, A6, A7, A8, A9) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2, A3, A4, A5, A6, A7, A8, A9)

#define THREADED_CALLABLE_IMPL10(type, function, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)

#define THREADED_CALLABLE_IMPL11(type, function, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)

#define THREADED_CALLABLE_IMPL12(type, function, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)

#define THREADED_CALLABLE_IMPL13(type, function, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)

#define THREADED_CALLABLE_IMPL14(type, function, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)

#define THREADED_CALLABLE_IMPL15(type, function, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15) \
  void type::function##TC(csRef<iThreadReturn> ret, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15)

#endif // __CS_IUTIL_THREADMANAGER_H__
