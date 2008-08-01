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

#include "cssysdef.h"
#include "threadtest.h"
#include "cstool/initapp.h"

CS_IMPLEMENT_APPLICATION

csThreadTest::csThreadTest(iObjectRegistry* objReg) : scfImplementationType(this),
                                                      objReg(objReg)
{
}


THREADED_CALLABLE_IMPL(csThreadTest, Test1)
{
  printf("Test 1 passed!\n");
}

THREADED_CALLABLE_IMPL1(csThreadTest, Test2, bool b)
{
  if(!b)
  {
    printf("Test 2 passed!\n");
  }
  else
  {
    printf("Test 2 failed!\n");
  }
}

THREADED_CALLABLE_IMPL2(csThreadTest, Test3, int i, float f)
{
  if(i == 3 && f == 0.1415f)
  {
    printf("Test 3 passed!\n");
  }
  else
  {
    printf("Test 3 failed!\n");
  }
}

THREADED_CALLABLE_IMPL1(csThreadTest, Test4, csWeakRef<iThreadTest> myself)
{
  iThreadTest* me = myself;
  if(me == this)
  {
    printf("Test 4 passed!\n");
  }
  else
  {
    printf("Test 4 failed!\n");
  }
}

THREADED_CALLABLE_IMPL(csThreadTest, Test5Real)
{
  ret->MarkSuccessful();
}

void csThreadTest::Test5() const
{
  csRef<iThreadReturn> ret = Test5Real();
  if(ret->WasSuccessful())
  {
    printf("Test 5 passed!\n");
  }
  else
  {
    printf("Test 5 failed!\n");
  }
}

THREADED_CALLABLE_IMPL1(csThreadTest, Test6, csRef<Data> stuff)
{
  csSleep(1000);
  bool passed = true;

  bool b = stuff->b;
  int i = stuff->i;
  float f = stuff->f;
  unsigned long long reallyBig = stuff->reallyBig;
  csWeakRefArray<iThreadTest> t = stuff->t;

  if(b && i == 42 && f == 3.14159f && reallyBig == 123456789012345 && stuff->objreg == objReg)
  {
    int counter = 0;
    for(int x=0; x<10000000; x++)
    {
      passed &= (stuff->d[x] == x/10);
    }

    csWeakRef<iThreadTest> t1 = t.Pop();
    csWeakRef<iThreadTest> t2 = t.Pop();
    passed &= (t1 == t2 && t1 == this &&
               t1->GetRefCount() == 1 &&
               stuff->GetRefCount() == 5);
  }
  else
  {
    passed = false; 
  }

  if(passed)
  {
    printf("Test 6 passed!\n");
  }
  else
  {
    printf("Test 6 failed!\n");
  }
}

THREADED_CALLABLE_IMPL(csThreadTest, Test7)
{
  csSleep(1000);
  Test7Data(1);
  Test7Data(2);
  Test7Data(3);
  Test7Data(4);
  Test7Data(5);
  ret->MarkSuccessful();
}

THREADED_CALLABLE_IMPL1(csThreadTest, Test7Data, int counter)
{
  printf("Test 7 Data %i of 5\n", counter);
  csSleep(1000);
}

int main(int argc, char* argv[])
{
  iObjectRegistry* objReg = csInitializer::CreateObjectRegistry();
  iThreadManager* tm = csInitializer::CreateThreadManager(objReg);

  csRef<iThreadTest> threadTest;
  threadTest.AttachNew(new csThreadTest(objReg));

  threadTest->Test1();
  threadTest->Test2(false);
  threadTest->Test3(3, 0.1415f);
  csWeakRef<iThreadTest> test4 = threadTest;
  threadTest->Test4(test4);
  threadTest->Test5();

  csRef<Data> dat;
  dat.AttachNew(new Data);

  dat->i = 42;
  dat->f = 3.14159f;
  dat->b = true;
  dat->reallyBig = 123456789012345; 

  for(int i=0; i<10000000; i++)
  {
    dat->d[i] = i/10;
  }

  csWeakRef<iThreadTest> test51 = threadTest;
  csWeakRef<iThreadTest> test52 = threadTest;
  dat->t.Push(test52);
  dat->t.Push(test51);
  dat->objreg = threadTest->GetObjectRegistry();

  threadTest->Test6(dat);

  csRef<iThreadReturn> ret = threadTest->Test7();
  if(ret->WasSuccessful())
  {
    printf("Test 7 passed!\n");
  }

  getchar();

  return 0;
}