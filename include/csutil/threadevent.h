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
  struct Args1 { A1* a1; };
  template<typename A1, typename A2>
  struct Args2 : public Args1<A1> { A2* a2; };
  template<typename A1, typename A2, typename A3>
  struct Args3 : public Args2<A1, A2> { A3* a3; };
  template<typename A1, typename A2, typename A3, typename A4>
  struct Args4 : public Args3<A1, A2, A3> { A4* a4; };
  template<typename A1, typename A2, typename A3, typename A4, typename A5>
  struct Args5 : public Args4<A1, A2, A3, A4> { A5* a5; };
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  struct Args6 : public Args5<A1, A2, A3, A4, A5> { A6* a6; };
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
  struct Args7 : public Args6<A1, A2, A3, A4, A5, A6> { A7* a7; };
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
  struct Args8 : public Args7<A1, A2, A3, A4, A5, A6, A7> { A8* a8; };
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
  struct Args9 : public Args8<A1, A2, A3, A4, A5, A6, A7, A8> { A9* a9; };
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
  struct Args10 : public Args9<A1, A2, A3, A4, A5, A6, A7, A8, A9> { A10* a10; };
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
  struct Args11 : public Args10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> { A11* a11; };
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
  struct Args12 : public Args11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> { A12* a12; };
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
  struct Args13 : public Args12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> { A13* a13; };
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
  struct Args14 : public Args13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13> { A14* a14; };
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
  struct Args15 : public Args14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14> { A15* a15; };
  
  template<typename A1>
  static void FetchArgs1 (void const** inArgs, Args1<A1>& outArgs)
  {
    outArgs.a1 = (A1*)(inArgs[1]);
  }
  template<typename A1, typename A2>
  static void FetchArgs2 (void const** inArgs, Args2<A1, A2>& outArgs)
  {
    outArgs.a2 = (A2*)(inArgs[2]);
    FetchArgs1<A1> (inArgs, outArgs);
  }
  template<typename A1, typename A2, typename A3>
  static void FetchArgs3 (void const** inArgs, Args3<A1, A2, A3>& outArgs)
  {
    outArgs.a3 = (A3*)(inArgs[3]);
    FetchArgs2<A1, A2> (inArgs, outArgs);
  }
  template<typename A1, typename A2, typename A3, typename A4>
  static void FetchArgs4 (void const** inArgs, Args4<A1, A2, A3, A4>& outArgs)
  {
    outArgs.a4 = (A4*)(inArgs[4]);
    FetchArgs3<A1, A2, A3> (inArgs, outArgs);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5>
  static void FetchArgs5 (void const** inArgs, Args5<A1, A2, A3, A4, A5>& outArgs)
  {
    outArgs.a5 = (A5*)(inArgs[5]);
    FetchArgs4<A1, A2, A3, A4> (inArgs, outArgs);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  static void FetchArgs6 (void const** inArgs, Args6<A1, A2, A3, A4, A5, A6>& outArgs)
  {
    outArgs.a6 = (A6*)(inArgs[6]);
    FetchArgs5<A1, A2, A3, A4, A5> (inArgs, outArgs);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
  static void FetchArgs7 (void const** inArgs, Args7<A1, A2, A3, A4, A5, A6, A7>& outArgs)
  {
    outArgs.a7 = (A7*)(inArgs[7]);
    FetchArgs6<A1, A2, A3, A4, A5, A6> (inArgs, outArgs);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
  static void FetchArgs8 (void const** inArgs, Args8<A1, A2, A3, A4, A5, A6, A7, A8>& outArgs)
  {
    outArgs.a8 = (A8*)(inArgs[8]);
    FetchArgs7<A1, A2, A3, A4, A5, A6, A7> (inArgs, outArgs);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
  static void FetchArgs9 (void const** inArgs, Args9<A1, A2, A3, A4, A5, A6, A7, A8, A9>& outArgs)
  {
    outArgs.a9 = (A9*)(inArgs[9]);
    FetchArgs8<A1, A2, A3, A4, A5, A6, A7, A8> (inArgs, outArgs);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
  static void FetchArgs10 (void const** inArgs, Args10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>& outArgs)
  {
    outArgs.a10 = (A10*)(inArgs[10]);
    FetchArgs9<A1, A2, A3, A4, A5, A6, A7, A8, A9> (inArgs, outArgs);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
  static void FetchArgs11 (void const** inArgs, Args11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>& outArgs)
  {
    outArgs.a11 = (A11*)(inArgs[11]);
    FetchArgs10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> (inArgs, outArgs);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
  static void FetchArgs12 (void const** inArgs, Args12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>& outArgs)
  {
    outArgs.a12 = (A12*)(inArgs[12]);
    FetchArgs11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> (inArgs, outArgs);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
  static void FetchArgs13 (void const** inArgs, Args13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13>& outArgs)
  {
    outArgs.a13 = (A13*)(inArgs[13]);
    FetchArgs12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> (inArgs, outArgs);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
  static void FetchArgs14 (void const** inArgs, Args14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14>& outArgs)
  {
    outArgs.a14 = (A14*)(inArgs[14]);
    FetchArgs13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13> (inArgs, outArgs);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
  static void FetchArgs15 (void const** inArgs, Args15<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15>& outArgs)
  {
    outArgs.a15 = (A15*)(inArgs[15]);
    FetchArgs14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14> (inArgs, outArgs);
  }
  
  template<typename A1>
  static void FreeArgs1 (void const** args, const Args1<A1>& A)
  {
    TEventMemPool* mempool = (TEventMemPool*)args[0];
    A.a1->Invalidate();
    delete mempool;
    delete[] args;
  }
  template<typename A1, typename A2>
  static void FreeArgs2 (void const** args, const Args2<A1, A2>& A)
  {
    A.a2->~A2();
    FreeArgs1<A1> (args, A);
  }
  template<typename A1, typename A2, typename A3>
  static void FreeArgs3 (void const** args, const Args3<A1, A2, A3>& A)
  {
    A.a3->~A3();
    FreeArgs2<A1, A2> (args, A);
  }
  template<typename A1, typename A2, typename A3, typename A4>
  static void FreeArgs4 (void const** args, const Args4<A1, A2, A3, A4>& A)
  {
    A.a4->~A4();
    FreeArgs3<A1, A2, A3> (args, A);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5>
  static void FreeArgs5 (void const** args, const Args5<A1, A2, A3, A4, A5>& A)
  {
    A.a5->~A5();
    FreeArgs4<A1, A2, A3, A4> (args, A);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  static void FreeArgs6 (void const** args, const Args6<A1, A2, A3, A4, A5, A6>& A)
  {
    A.a6->~A6();
    FreeArgs5<A1, A2, A3, A4, A5> (args, A);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
  static void FreeArgs7 (void const** args, const Args7<A1, A2, A3, A4, A5, A6, A7>& A)
  {
    A.a7->~A7();
    FreeArgs6<A1, A2, A3, A4, A5, A6> (args, A);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
  static void FreeArgs8 (void const** args, const Args8<A1, A2, A3, A4, A5, A6, A7, A8>& A)
  {
    A.a8->~A8();
    FreeArgs7<A1, A2, A3, A4, A5, A6, A7> (args, A);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
  static void FreeArgs9 (void const** args, const Args9<A1, A2, A3, A4, A5, A6, A7, A8, A9>& A)
  {
    A.a9->~A9();
    FreeArgs8<A1, A2, A3, A4, A5, A6, A7, A8> (args, A);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
  static void FreeArgs10 (void const** args, const Args10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>& A)
  {
    A.a10->~A10();
    FreeArgs9<A1, A2, A3, A4, A5, A6, A7, A8, A9> (args, A);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
  static void FreeArgs11 (void const** args, const Args11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>& A)
  {
    A.a11->~A11();
    FreeArgs10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> (args, A);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
  static void FreeArgs12 (void const** args, const Args12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>& A)
  {
    A.a12->~A12();
    FreeArgs11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> (args, A);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
  static void FreeArgs13 (void const** args, const Args13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13>& A)
  {
    A.a13->~A13();
    FreeArgs12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> (args, A);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
  static void FreeArgs14 (void const** args, const Args14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14>& A)
  {
    A.a14->~A14();
    FreeArgs13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13> (args, A);
  }
  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
  static void FreeArgs15 (void const** args, const Args15<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15>& A)
  {
    A.a15->~A15();
    FreeArgs14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14> (args, A);
  }

  template<typename A1, typename A2>
  void RunMethod (bool (T::*method)(A1, A2), const Args2<A1, A2>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
  }

  template<typename A1, typename A2, typename A3>
  void RunMethod (bool (T::*method)(A1, A2, A3), const Args3<A1, A2, A3>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2, *A.a3))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
  }

  template<typename A1, typename A2, typename A3, typename A4>
  void RunMethod (bool (T::*method)(A1, A2, A3, A4), const Args4<A1, A2, A3, A4>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2, *A.a3, *A.a4))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5>
  void RunMethod (bool (T::*method)(A1, A2, A3, A4, A5), const Args5<A1, A2, A3, A4, A5>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2, *A.a3, *A.a4, *A.a5))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  void RunMethod (bool (T::*method)(A1, A2, A3, A4, A5, A6), const Args6<A1, A2, A3, A4, A5, A6>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2, *A.a3, *A.a4, *A.a5, *A.a6))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
  void RunMethod (bool (T::*method)(A1, A2, A3, A4, A5, A6, A7), const Args7<A1, A2, A3, A4, A5, A6, A7>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2, *A.a3, *A.a4, *A.a5, *A.a6, *A.a7))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
  void RunMethod (bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8), const Args8<A1, A2, A3, A4, A5, A6, A7, A8>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2, *A.a3, *A.a4, *A.a5, *A.a6, *A.a7, *A.a8))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
  void RunMethod (bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9), const Args9<A1, A2, A3, A4, A5, A6, A7, A8, A9>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2, *A.a3, *A.a4, *A.a5, *A.a6, *A.a7, *A.a8, *A.a9))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
  void RunMethod (bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), const Args10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2, *A.a3, *A.a4, *A.a5, *A.a6, *A.a7, *A.a8, *A.a9, *A.a10))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
  void RunMethod (bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11), const Args11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2, *A.a3, *A.a4, *A.a5, *A.a6, *A.a7, *A.a8, *A.a9, *A.a10, *A.a11))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
  void RunMethod (bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12), const Args12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2, *A.a3, *A.a4, *A.a5, *A.a6, *A.a7, *A.a8, *A.a9, *A.a10, *A.a11, *A.a12))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
  void RunMethod (bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13), const Args13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2, *A.a3, *A.a4, *A.a5, *A.a6, *A.a7, *A.a8, *A.a9, *A.a10, *A.a11, *A.a12, *A.a13))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
  void RunMethod (bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14), const Args14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2, *A.a3, *A.a4, *A.a5, *A.a6, *A.a7, *A.a8, *A.a9, *A.a10, *A.a11, *A.a12, *A.a13, *A.a14))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
  }

  template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
  void RunMethod (bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15), const Args15<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15>& A)
  {
      T* mySelf = (T*)this;
      if((mySelf->*method)(*A.a1, *A.a2, *A.a3, *A.a4, *A.a5, *A.a6, *A.a7, *A.a8, *A.a9, *A.a10, *A.a11, *A.a12, *A.a13, *A.a14, *A.a15))
      {
        (*A.a1)->MarkSuccessful();
      }
      (*A.a1)->MarkFinished();
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
  ~ThreadEvent1()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args1<A1> A;
      ThreadedCallable<T>::FetchArgs1 (args, A);
      ThreadedCallable<T>::FreeArgs1 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args1<A1> A;
    ThreadedCallable<T>::FetchArgs1 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs1 (args, A);
    args = nullptr;
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
  ~ThreadEvent2()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args2<A1, A2> A;
      ThreadedCallable<T>::FetchArgs2 (args, A);
      ThreadedCallable<T>::FreeArgs2 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args2<A1, A2> A;
    ThreadedCallable<T>::FetchArgs2 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs2 (args, A);
    args = nullptr;
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
  ~ThreadEvent3()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args3<A1, A2, A3> A;
      ThreadedCallable<T>::FetchArgs3 (args, A);
      ThreadedCallable<T>::FreeArgs3 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args3<A1, A2, A3> A;
    ThreadedCallable<T>::FetchArgs3 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs3 (args, A);
    args = nullptr;
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
  ~ThreadEvent4()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args4<A1, A2, A3, A4> A;
      ThreadedCallable<T>::FetchArgs4 (args, A);
      ThreadedCallable<T>::FreeArgs4 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args4<A1, A2, A3, A4> A;
    ThreadedCallable<T>::FetchArgs4 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs4 (args, A);
    args = nullptr;
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
  ~ThreadEvent5()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args5<A1, A2, A3, A4, A5> A;
      ThreadedCallable<T>::FetchArgs5 (args, A);
      ThreadedCallable<T>::FreeArgs5 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args5<A1, A2, A3, A4, A5> A;
    ThreadedCallable<T>::FetchArgs5 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs5 (args, A);
    args = nullptr;
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
  ~ThreadEvent6()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args6<A1, A2, A3, A4, A5, A6> A;
      ThreadedCallable<T>::FetchArgs6 (args, A);
      ThreadedCallable<T>::FreeArgs6 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args6<A1, A2, A3, A4, A5, A6> A;
    ThreadedCallable<T>::FetchArgs6 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs6 (args, A);
    args = nullptr;
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
  ~ThreadEvent7()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args7<A1, A2, A3, A4, A5, A6, A7> A;
      ThreadedCallable<T>::FetchArgs7 (args, A);
      ThreadedCallable<T>::FreeArgs7 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args7<A1, A2, A3, A4, A5, A6, A7> A;
    ThreadedCallable<T>::FetchArgs7 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs7 (args, A);
    args = nullptr;
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
  ~ThreadEvent8()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args8<A1, A2, A3, A4, A5, A6, A7, A8> A;
      ThreadedCallable<T>::FetchArgs8 (args, A);
      ThreadedCallable<T>::FreeArgs8 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args8<A1, A2, A3, A4, A5, A6, A7, A8> A;
    ThreadedCallable<T>::FetchArgs8 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs8 (args, A);
    args = nullptr;
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
  ~ThreadEvent9()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args9<A1, A2, A3, A4, A5, A6, A7, A8, A9> A;
      ThreadedCallable<T>::FetchArgs9 (args, A);
      ThreadedCallable<T>::FreeArgs9 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args9<A1, A2, A3, A4, A5, A6, A7, A8, A9> A;
    ThreadedCallable<T>::FetchArgs9 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs9 (args, A);
    args = nullptr;
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
  ~ThreadEvent10()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> A;
      ThreadedCallable<T>::FetchArgs10 (args, A);
      ThreadedCallable<T>::FreeArgs10 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args10<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> A;
    ThreadedCallable<T>::FetchArgs10 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs10 (args, A);
    args = nullptr;
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
  ~ThreadEvent11()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> A;
      ThreadedCallable<T>::FetchArgs11 (args, A);
      ThreadedCallable<T>::FreeArgs11 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args11<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> A;
    ThreadedCallable<T>::FetchArgs11 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs11 (args, A);
    args = nullptr;
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
  ~ThreadEvent12()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> A;
      ThreadedCallable<T>::FetchArgs12 (args, A);
      ThreadedCallable<T>::FreeArgs12 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args12<A1, A2, A3, A4, A5, A6, A7, A8, A9, A11, A12> A;
    ThreadedCallable<T>::FetchArgs12 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs12 (args, A);
    args = nullptr;
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
  ~ThreadEvent13()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13> A;
      ThreadedCallable<T>::FetchArgs13 (args, A);
      ThreadedCallable<T>::FreeArgs13 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args13<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13> A;
    ThreadedCallable<T>::FetchArgs13 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs13 (args, A);
    args = nullptr;
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
  ~ThreadEvent14()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14> A;
      ThreadedCallable<T>::FetchArgs14 (args, A);
      ThreadedCallable<T>::FreeArgs14 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args14<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14> A;
    ThreadedCallable<T>::FetchArgs14 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs14 (args, A);
    args = nullptr;
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
  ~ThreadEvent15()
  {
    if (args)
    {
      typename ThreadedCallable<T>::template Args15<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15> A;
      ThreadedCallable<T>::FetchArgs15 (args, A);
      ThreadedCallable<T>::FreeArgs15 (args, A);
    }
  }

  void Run()
  {
    typename ThreadedCallable<T>::template Args15<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15> A;
    ThreadedCallable<T>::FetchArgs15 (args, A);
    object->RunMethod (method, A);
    ThreadedCallable<T>::FreeArgs15 (args, A);
    args = nullptr;
  }

private:
  ThreadedCallable<T>* object;
  bool (T::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15);
  void const** args;
};

#include "custom_new_disable.h"

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

  char* ptr = 0;
  if(*p)
  {
    size_t length = strlen(*p) + 1;
    ptr = (char*)Alloc(length);
    memcpy(ptr, *p, length);
  }

  char** ptrPtr = new (this) char*;
  *ptrPtr = ptr;
  return (void const*)ptrPtr;
}

#include "custom_new_enable.h"

#endif // __CS_CSUTIL_THREADEVENT_H__
