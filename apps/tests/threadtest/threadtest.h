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

struct Data : public csRefCount
{
  int i;
  float f;
  double d[10000000];
  csWeakRefArray<iThreadTest> t;
  bool b;
  unsigned long long reallyBig;
  iObjectRegistry* objreg;
};

struct iThreadTest : public virtual iBase
{
  THREADED_INTERFACE(Test1)
  THREADED_INTERFACE1(Test2, bool b)
  THREADED_INTERFACE2(Test3, int i, float f)
  THREADED_INTERFACE1(Test4, csWeakRef<iThreadTest> myself)
  virtual void Test5() const = 0;
  THREADED_INTERFACE1(Test6, csRef<Data> stuff)
  THREADED_INTERFACE(Test7)
  virtual iObjectRegistry* GetObjectRegistry() const = 0;
};

class csThreadTest : public ThreadedCallable<csThreadTest>,
                     public scfImplementation1<csThreadTest,
                                               iThreadTest>
{
public:

  csThreadTest(iObjectRegistry* objReg);

  iObjectRegistry* GetObjectRegistry() const { return objReg; }

  THREADED_CALLABLE_DECL(csThreadTest, Test1, csThreadReturn, true, false)
  THREADED_CALLABLE_DECL1(csThreadTest, Test2, csThreadReturn, bool, b, true, false)
  THREADED_CALLABLE_DECL2(csThreadTest, Test3, csThreadReturn, int, in, float, flo, true, false)
  THREADED_CALLABLE_DECL1(csThreadTest, Test4, csThreadReturn, csWeakRef<iThreadTest>, myself, true, false)
  THREADED_CALLABLE_DECL(csThreadTest, Test5Real, csThreadReturn, true, false)
  THREADED_CALLABLE_DECL1(csThreadTest, Test6, csThreadReturn, csRef<Data>, stuff, true, true)
  THREADED_CALLABLE_DECL(csThreadTest, Test7, csThreadReturn, true, true)
  THREADED_CALLABLE_DECL1(csThreadTest, Test7Data, csThreadReturn, int, counter, true, false)
  THREADED_CALLABLE_DECL(csThreadTest, Test7RealData, csThreadReturn, true, true)

  void Test5() const;

private:
  iObjectRegistry* objReg;
};

#endif // __THREAD_TEST_H__
