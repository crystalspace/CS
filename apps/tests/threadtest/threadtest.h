/*
Copyright (C) 2008 by Michael Gist

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
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

#ifndef __THREAD_TEST_H__
#define __THREAD_TEST_H__

#include "csutil/threadmanager.h"
#include "csutil/weakrefarr.h"

struct iThreadTest;

class Data : public scfImplementation0<Data>
{
public:
  Data() : scfImplementationType(this) {}

  int i;
  float f;
  double d[10000000];
  csWeakRefArray<iThreadTest> t;
  bool b;
  uint64 reallyBig;
  iObjectRegistry* objreg;
};

struct iThreadTest : public virtual iBase
{
  THREADED_INTERFACE(Test1);
  THREADED_INTERFACE1(Test2, bool b);
  THREADED_INTERFACE2(Test3, int i, float f);
  THREADED_INTERFACE1(Test4, csWeakRef<iThreadTest> myself);
  virtual void Test5() const = 0;
  THREADED_INTERFACE1(Test6, csRef<Data> stuff);
  THREADED_INTERFACE(Test7);
  virtual iObjectRegistry* GetObjectRegistry() const = 0;
  virtual bool Test7Passed() const = 0;
};

class csThreadTest : public ThreadedCallable<csThreadTest>,
                     public scfImplementation1<csThreadTest,
                                               iThreadTest>
{
public:

  csThreadTest(iObjectRegistry* objReg);

  iObjectRegistry* GetObjectRegistry() const { return objReg; }

  THREADED_CALLABLE_DECL(csThreadTest, Test1, csThreadReturn, THREADED, false, false)
  THREADED_CALLABLE_DECL(csThreadTest, Test1Data, csThreadReturn, THREADED, false, false)
  THREADED_CALLABLE_DECL1(csThreadTest, Test1Data2, csThreadReturn, csRef<iThreadReturn>, itr, THREADED, false, false)
  THREADED_CALLABLE_DECL1(csThreadTest, Test2, csThreadReturn, bool, b, THREADED, false, false)
  THREADED_CALLABLE_DECL2(csThreadTest, Test3, csThreadReturn, int, in, float, flo, THREADED, false, false)
  THREADED_CALLABLE_DECL1(csThreadTest, Test4, csThreadReturn, csWeakRef<iThreadTest>, myself, THREADED, false, false)
  THREADED_CALLABLE_DECL(csThreadTest, Test5Real, csThreadReturn, THREADED, false, false)
  THREADED_CALLABLE_DECL1(csThreadTest, Test6, csThreadReturn, csRef<Data>, stuff, THREADED, true, false)
  THREADED_CALLABLE_DECL(csThreadTest, Test7, csThreadReturn, THREADED, true, false)
  THREADED_CALLABLE_DECL(csThreadTest, Test7Data, csThreadReturn, THREADED, false, false)
  THREADED_CALLABLE_DECL(csThreadTest, Test7RealData, csThreadReturn, THREADED, true, false)

  void Test5() const;

  bool Test7Passed() const
  {
    CS::Threading::MutexScopedLock lock(test7lock);
    return test7 == 10;
  }

private:
  iObjectRegistry* objReg;

  mutable CS::Threading::Mutex test7lock;
  uint test7;
};

#endif // __THREAD_TEST_H__
