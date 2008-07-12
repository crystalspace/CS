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
};

struct iThreadTest : public virtual iBase
{
  virtual void Test1() = 0;
  virtual void Test2(bool b) = 0;
  virtual void Test3(int i, float f) = 0;
  virtual void Test4(csWeakRef<iThreadTest> myself) = 0;
  virtual void Test5(csRef<Data> stuff) = 0;
};

class csThreadTest : public ThreadedCallable<csThreadTest>,
                     public scfImplementation1<csThreadTest,
                                               iThreadTest>
{
public:

  csThreadTest(iObjectRegistry* objReg);

  iObjectRegistry* GetObjectRegistry() { return objReg; }

  THREADED_CALLABLE_DECL(csThreadTest, Test1)
  THREADED_CALLABLE_DECL1(csThreadTest, Test2, bool, b)
  THREADED_CALLABLE_DECL2(csThreadTest, Test3, int, in, float, flo)
  THREADED_CALLABLE_DECL1(csThreadTest, Test4, csWeakRef<iThreadTest>, myself)
  THREADED_CALLABLE_DECL1(csThreadTest, Test5, csRef<Data>, stuff)

private:
  iObjectRegistry* objReg;
};

#endif // __THREAD_TEST_H__
