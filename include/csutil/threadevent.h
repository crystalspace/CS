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

class TEventMemPool;

template<class T>
class ThreadedCallable
{
public:
  ThreadedCallable() {}
  virtual ~ThreadedCallable() {}
  
  virtual iObjectRegistry* GetObjectRegistry() const = 0;

  template<typename A1>
  void RunMethod1(bool (T::*method)(A1), void const** &args)
  {
      T* mySelf = (T*)this;
      A1* a1 = (A1*)args[1];
      if((mySelf->*method)(*a1))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2>
  void RunMethod2(bool (T::*method)(A1, A2), void const** &args)
  {
      T* mySelf = (T*)this;
      A2* a2 = (A2*)args[2];
      A1* a1 = (A1*)args[1];
      if((mySelf->*method)(*a1, *a2))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2, typename A3>
  void RunMethod3(bool (T::*method)(A1, A2, A3), void const** &args)
  {
      T* mySelf = (T*)this;
      A3* a3 = (A3*)args[3];
      A2* a2 = (A2*)args[2];
      A1* a1 = (A1*)args[1];
      if((mySelf->*method)(*a1, *a2, *a3))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2, typename A3, typename A4>
  void RunMethod4(bool (T::*method)(A1, A2, A3, A4), void const** &args)
  {
      T* mySelf = (T*)this;
      A4* a4 = (A4*)args[4];
      A3* a3 = (A3*)args[3];
      A2* a2 = (A2*)args[2];
      A1* a1 = (A1*)args[1];
      if((mySelf->*method)(*a1, *a2, *a3, *a4))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5>
  void RunMethod5(bool (T::*method)(A1, A2, A3, A4, A5), void const** &args)
  {
      T* mySelf = (T*)this;
      A5* a5 = (A5*)args[5];
      A4* a4 = (A4*)args[4];
      A3* a3 = (A3*)args[3];
      A2* a2 = (A2*)args[2];
      A1* a1 = (A1*)args[1];
      if((mySelf->*method)(*a1, *a2, *a3, *a4, *a5))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  void RunMethod6(bool (T::*method)(A1, A2, A3, A4, A5, A6), void const** &args)
  {
      T* mySelf = (T*)this;
      A6* a6 = (A6*)args[6];
      A5* a5 = (A5*)args[5];
      A4* a4 = (A4*)args[4];
      A3* a3 = (A3*)args[3];
      A2* a2 = (A2*)args[2];
      A1* a1 = (A1*)args[1];
      if((mySelf->*method)(*a1, *a2, *a3, *a4, *a5, *a6))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
  void RunMethod7(bool (T::*method)(A1, A2, A3, A4, A5, A6, A7), void const** &args)
  {
      T* mySelf = (T*)this;
      A7* a7 = (A7*)args[7];
      A6* a6 = (A6*)args[6];
      A5* a5 = (A5*)args[5];
      A4* a4 = (A4*)args[4];
      A3* a3 = (A3*)args[3];
      A2* a2 = (A2*)args[2];
      A1* a1 = (A1*)args[1];
      if((mySelf->*method)(*a1, *a2, *a3, *a4, *a5, *a6, *a7))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
  void RunMethod8(bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8), void const** &args)
  {
      T* mySelf = (T*)this;
      A8* a8 = (A8*)args[8];
      A7* a7 = (A7*)args[7];
      A6* a6 = (A6*)args[6];
      A5* a5 = (A5*)args[5];
      A4* a4 = (A4*)args[4];
      A3* a3 = (A3*)args[3];
      A2* a2 = (A2*)args[2];
      A1* a1 = (A1*)args[1];
      if((mySelf->*method)(*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
  void RunMethod9(bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9), void const** &args)
  {
      T* mySelf = (T*)this;
      A9* a9 = (A9*)args[9];
      A8* a8 = (A8*)args[8];
      A7* a7 = (A7*)args[7];
      A6* a6 = (A6*)args[6];
      A5* a5 = (A5*)args[5];
      A4* a4 = (A4*)args[4];
      A3* a3 = (A3*)args[3];
      A2* a2 = (A2*)args[2];
      A1* a1 = (A1*)args[1];
      if((mySelf->*method)(*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
  void RunMethod10(bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), void const** &args)
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
      if((mySelf->*method)(*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
  void RunMethod11(bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11), void const** &args)
  {
      T* mySelf = (T*)this;
      A11* a11 = (A11*)args[11];
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
      if((mySelf->*method)(*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10, *a11))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
  void RunMethod12(bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12), void const** &args)
  {
      T* mySelf = (T*)this;
      A12* a12 = (A12*)args[12];
      A11* a11 = (A11*)args[11];
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
      if((mySelf->*method)(*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10, *a11, *a12))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
  void RunMethod13(bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13), void const** &args)
  {
      T* mySelf = (T*)this;
      A13* a13 = (A13*)args[13];
      A12* a12 = (A12*)args[12];
      A11* a11 = (A11*)args[11];
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
      if((mySelf->*method)(*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10, *a11, *a12, *a13))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
  void RunMethod14(bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14), void const** &args)
  {
      T* mySelf = (T*)this;
      A14* a14 = (A14*)args[14];
      A13* a13 = (A13*)args[13];
      A12* a12 = (A12*)args[12];
      A11* a11 = (A11*)args[11];
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
      if((mySelf->*method)(*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10, *a11, *a12, *a13, *a14))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
  void RunMethod15(bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15), void const** &args)
  {
      T* mySelf = (T*)this;
      A15* a15 = (A15*)args[15];
      A14* a14 = (A14*)args[14];
      A13* a13 = (A13*)args[13];
      A12* a12 = (A12*)args[12];
      A11* a11 = (A11*)args[11];
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
      if((mySelf->*method)(*a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10, *a11, *a12, *a13, *a14, *a15))
      {
        (*a1)->MarkSuccessful();
      }
      (*a1)->MarkFinished();
      TEventMemPool* mempool = (TEventMemPool*)args[0];
      a1->Invalidate();
      delete mempool;
      delete args;
  }
};

template<class T, typename A1>
class ThreadEvent1 : public scfImplementation1<ThreadEvent1<T, A1>, iJob>
{
public:
  ThreadEvent1(ThreadedCallable<T>* &object, bool (T::*method)(A1), void const** &args)
    : scfImplementation1<ThreadEvent1<T, A1>, iJob> (this), object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod1 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1);
  void const** args;
};

template<class T, typename A1, typename A2>
class ThreadEvent2 : public scfImplementation1<ThreadEvent2<T, A1, A2>, iJob>
{
public:
  ThreadEvent2(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2), void const** &args)
    : scfImplementation1<ThreadEvent2<T, A1, A2>, iJob> (this),
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod2 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2);
  void const** args;
};

template<class T, typename A1, typename A2, typename A3>
class ThreadEvent3 : public scfImplementation1<ThreadEvent3<T, A1, A2, A3>, iJob>
{
public:
  ThreadEvent3(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2, A3), void const** &args)
    : scfImplementation1<ThreadEvent3<T, A1, A2, A3>, iJob> (this),
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod3 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3);
  void const** args;
};

template<class T, typename A1, typename A2, typename A3, typename A4>
class ThreadEvent4 : public scfImplementation1<ThreadEvent4<T, A1, A2, A3, A4>, iJob>
{
public:
  ThreadEvent4(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2, A3, A4), void const** &args)
    : scfImplementation1<ThreadEvent4<T, A1, A2, A3, A4>, iJob> (this),
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod4 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3, A4);
  void const** args;
};

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5>
class ThreadEvent5 : public scfImplementation1<ThreadEvent5<T, A1, A2, A3, A4, A5>, iJob>
{
public:
  ThreadEvent5(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2, A3, A4, A5), void const** &args)
    : scfImplementation1<ThreadEvent5<T, A1, A2, A3, A4, A5>, iJob> (this),
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod5 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3, A4, A5);
  void const** args;
};

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
class ThreadEvent6 : public scfImplementation1<ThreadEvent6<T, A1, A2, A3, A4, A5, A6>, iJob>
{
public:
  ThreadEvent6(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2, A3, A4, A5, A6), void const** &args)
    : scfImplementation1<ThreadEvent6<T, A1, A2, A3, A4, A5, A6>, iJob> (this),
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod6 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3, A4, A5, A6);
  void const** args;
};

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
class ThreadEvent7 : public scfImplementation1<ThreadEvent7<T, A1, A2, A3, A4, A5, A6, A7>, iJob>
{
public:
  ThreadEvent7(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2, A3, A4, A5, A6, A7), void const** &args)
    : scfImplementation1<ThreadEvent7<T, A1, A2, A3, A4, A5, A6, A7>, iJob> (this),
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod7 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3, A4, A5, A6, A7);
  void const** args;
};

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
class ThreadEvent8 : public scfImplementation1<ThreadEvent8<T, A1, A2, A3, A4, A5, A6, A7, A8>, iJob>
{
public:
  ThreadEvent8(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8), void const** &args)
    : scfImplementation1<ThreadEvent8<T, A1, A2, A3, A4, A5, A6, A7, A8>, iJob> (this),
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod8 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8);
  void const** args;
};

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
class ThreadEvent9 : public scfImplementation1<ThreadEvent9<T, A1, A2, A3, A4, A5, A6, A7, A8, A9>, iJob>
{
public:
  ThreadEvent9(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9), void const** &args)
    : scfImplementation1<ThreadEvent9<T, A1, A2, A3, A4, A5, A6, A7, A8, A9>, iJob>(this),
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod9 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9);
  void const** args;
};

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
class ThreadEvent10 : public scfImplementation1<ThreadEvent10<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>, iJob>
{
public:
  ThreadEvent10(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), void const** &args)
    : scfImplementation1<ThreadEvent10<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>, iJob>(this),
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod10 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
  void const** args;
};

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
class ThreadEvent11 : public scfImplementation1<ThreadEvent11<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>, iJob>
{
public:
  ThreadEvent11(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11), void const** &args)
    : scfImplementation1<ThreadEvent11<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>, iJob> (this),
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod11 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11);
  void const** args;
};

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
class ThreadEvent12 : public scfImplementation1<ThreadEvent12<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>, iJob>
{
public:
  ThreadEvent12(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12), void const** &args)
    : scfImplementation1<ThreadEvent12<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>, iJob> (this),
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod12 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12);
  void const** args;
};

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
class ThreadEvent13 : public scfImplementation1<ThreadEvent13<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13>, iJob>
{
public:
  ThreadEvent13(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13), void const** &args)
    : scfImplementation1<ThreadEvent13<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13>, iJob> (this),
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod13 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13);
  void const** args;
};

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
class ThreadEvent14 : public scfImplementation1<ThreadEvent14<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14>, iJob>
{
public:
  ThreadEvent14(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14), void const** &args)
    : scfImplementation1<ThreadEvent14<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14>, iJob> (this),
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod14 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14);
  void const** args;
};

template<class T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
class ThreadEvent15 : public scfImplementation1<ThreadEvent15<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15>, iJob>
{
public:
  ThreadEvent15(ThreadedCallable<T>* &object, bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15), void const** &args)
    : scfImplementation1<ThreadEvent15<T, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15>, iJob> (this), 
      object(object), method(method), args(args)
  {
  }

  void Run()
  {
    object->RunMethod15 (method, args);
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15);
  void const** args;
};

class TEventMemPool : public csMemoryPool
{
public:

  template<typename T>
  void const* Store(T* p)
  {
    T* newp = (T*)csMemoryPool::Alloc (sizeof(T));
    new (newp) T (*p);
    return (void const*)newp;
  }
};

template<>
inline void const* TEventMemPool::Store<const char*>(const char** p)
{
  if(!p)
  {
    return 0;
  }

  char* ptr = (char*)Alloc(strlen(*p) + 1);
  strcpy(ptr, *p);

  char** ptrPtr = new (this) char*;
  *ptrPtr = ptr;
  return (void const*)ptrPtr;
}

#endif // __CS_CSUTIL_THREADEVENT_H__
