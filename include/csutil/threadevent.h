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

#ifndef __CS_CSUTIL_THREADEVENT_H__
#define __CS_CSUTIL_THREADEVENT_H__

#include "csutil/array.h"
#include "csutil/mempool.h"
#include "csutil/scf_implementation.h"
#include "csutil/scf_interface.h"
#include "csutil/weakref.h"
#include "iutil/job.h"

template<class T>
class ThreadedCallable
{
public:
  ThreadedCallable* GetThreadedCallable() { return this; }
  virtual iObjectRegistry* GetObjectRegistry() = 0;

  void RunMethod(void (T::*method)(void))
  {
      T* mySelf = (T*)this;
      (mySelf->*method)();
  }

  template<typename A1>
  void RunMethod1(void (T::*method)(A1), csArray<void const*> &args)
  {
      T* mySelf = (T*)this;
      A1* a1 = (A1*)args[1];
      (mySelf->*method)(*a1);
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      delete mempool;
  }

  template<typename A1, typename A2>
  void RunMethod2(void (T::*method)(A1, A2), csArray<void const*> &args)
  {
      T* mySelf = (T*)this;
      A2* a2 = (A2*)args[2];
      A1* a1 = (A1*)args[1];
      (mySelf->*method)(*a1, *a2);
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      delete mempool;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
  void RunMethod10(void (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), csArray<void const*> &args)
  {
      T* mySelf = (T*)this;
      A10* a10 = (A10*)args[10];
      A9* a9 = (A9*)args[9];
      A8* a8 = (A8*)args[8];
      A7* a7 = (A7*)args[7];
      A6* a6 = (A6*)args[6];
      A5* a5 = (A5*)args[5];
      A4* a4 = (A4*)args[4];
      A3* a3 = (A3*)args[3];
      A2* a2 = (A2*)args[2];
      A1* a1 = (A1*)args[1];
      (mySelf->*method)(*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10);
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      delete mempool;
  }
};

template<class T>
class ThreadEvent : public scfImplementation1<ThreadEvent<T>, iJob>
{
public:
  ThreadEvent(ThreadedCallable<T>* object, void (T::*method)(void))
    : scfImplementationType(this), object(object), method(method)
  {
  }

  void Run()
  {
    object->RunMethod(method);
  }

private:
  ThreadedCallable<T>* object;
  void (T::*method)(void);
};

template<class T, typename A1>
class ThreadEvent1 : public scfImplementation1<ThreadEvent1<T, A1>, iJob>
{
public:
  ThreadEvent1(ThreadedCallable<T>* object, void (T::*method)(A1), csArray<void const*> args)
    : scfImplementationType(this), object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod1<A1>(method, args);
  }

private:
  ThreadedCallable<T>* object;
  void (T::*method)(A1);
  csArray<void const*> args;
};

template<class T, typename A1, typename A2>
class ThreadEvent2 : public scfImplementation1<ThreadEvent2<T, A1, A2>, iJob>
{
public:
  ThreadEvent2(ThreadedCallable<T>* object, void (T::*method)(A1, A2), csArray<void const*> args)
    : scfImplementationType(this), object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod2<A1, A2>(method, args);
  }

private:
  ThreadedCallable<T>* object;
  void (T::*method)(A1, A2);
  csArray<void const*> args;
};

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
class ThreadEvent10 : public scfImplementation1<ThreadEvent10<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>, iJob>
{
public:
  ThreadEvent10(ThreadedCallable<T>* object, void (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), csArray<void const*> args)
    : scfImplementationType(this), object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>(method, args);
  }

private:
  ThreadedCallable<T>* object;
  void (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
  csArray<void const*> args;
};

class TEventMemPool : public csMemoryPool
{
public:
  template<typename T>
  void const* Store(T* p)
  {
    T* ptr = new (this) T(p);
    return (void const*)ptr;
  }

  template<typename T>
  void const* Store(T& p)
  {
    T* ptr = new (this) T;
    *ptr = p;
    return (void const*)ptr;
  }
};

#endif // __CS_CSUTIL_THREADEVENT_H__
