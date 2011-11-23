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

#ifdef CS_PLATFORM_UNIX
#include <termios.h>
#endif
#ifdef CS_PLATFORM_WIN32
#include <conio.h>
#endif

static void WaitForKey()
{
#if defined(CS_PLATFORM_UNIX)
  // Set up terminal so it doesn't do line-wise input
  int stdin_fd (0);
  termios orig_tca;
  tcgetattr (stdin_fd, &orig_tca);
  termios new_tca (orig_tca);
  new_tca.c_lflag &= ~ICANON;
  new_tca.c_cc[VMIN] = 1;
  tcsetattr (stdin_fd, TCSANOW, &new_tca);

  getchar();

  // Restore old state
  tcsetattr (stdin_fd, TCSANOW, &orig_tca);
#elif defined(CS_PLATFORM_WIN32)
  _getch();
#else
  getchar();
#endif
}

using namespace CS::Threading;

csThreadTest::csThreadTest(iObjectRegistry* objReg) : scfImplementationType(this),
                                                      objReg(objReg)
{
  test7 = 0;
}


THREADED_CALLABLE_IMPL(csThreadTest, Test1)
{
  csRefArray<iThreadReturn> threadReturns;
  threadReturns.Push(Test1Data());
  csRef<iThreadManager> tm = csQueryRegistry<iThreadManager> (objReg);
  tm->Wait(threadReturns);
  printf("Test 1 passed!\n");
  return true;
}

THREADED_CALLABLE_IMPL(csThreadTest, Test1Data)
{
  csRef<iThreadReturn> itr = Test1Data2(Test1Data2(csRef<iThreadReturn>()));
  while(!itr->IsFinished());
  return true;
}


THREADED_CALLABLE_IMPL1(csThreadTest, Test1Data2, csRef<iThreadReturn> itr)
{
  if(itr.IsValid())
  {
    while(!itr->IsFinished());
  }
  return true;
}

THREADED_CALLABLE_IMPL1(csThreadTest, Test2, bool b)
{
  if(!b)
  {
    printf("Test 2 passed!\n");
    return true;
  }
  else
  {
    printf("Test 2 failed!\n");
    return false;
  }
}

THREADED_CALLABLE_IMPL2(csThreadTest, Test3, int i, float f)
{
  if(i == 3 && f == 0.1415f)
  {
    printf("Test 3 passed!\n");
    return true;
  }
  else
  {
    printf("Test 3 failed!\n");
    return false;
  }
}

THREADED_CALLABLE_IMPL1(csThreadTest, Test4, csWeakRef<iThreadTest> myself)
{
  iThreadTest* me = myself;
  if(me == this)
  {
    printf("Test 4 passed!\n");
    return true;
  }
  else
  {
    printf("Test 4 failed!\n");
    return false;
  }
}

THREADED_CALLABLE_IMPL(csThreadTest, Test5Real)
{
  return true;
}

void csThreadTest::Test5() const
{
  csRef<iThreadReturn> ret = Test5RealWait();
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

  if(b && i == 42 && f == 3.14159f && reallyBig == CONST_UINT64(123456789012345) && stuff->objreg == objReg)
  {
    for(int x=0; x<10000000; x++)
    {
      passed &= (stuff->d[x] == x/10);
    }

    csWeakRef<iThreadTest> t1 = t.Pop();
    csWeakRef<iThreadTest> t2 = t.Pop();
    passed &= (t1 == t2 && t1 == this &&
               t1->GetRefCount() == 1 &&
               stuff->GetRefCount() == 6);
  }
  else
  {
    passed = false; 
  }

  if(passed)
  {
    printf("Test 6 passed!\n");
    return true;
  }
  else
  {
    printf("Test 6 failed!\n");
    return false;
  }
}

THREADED_CALLABLE_IMPL(csThreadTest, Test7)
{
  csSleep(100);
  Test7Data();
  Test7Data();
  Test7Data();
  Test7Data();
  Test7Data();
  Test7Data();
  Test7Data();
  Test7Data();
  Test7Data();
  Test7Data();
  return true;
}

THREADED_CALLABLE_IMPL(csThreadTest, Test7Data)
{
  csSleep(100);
  Test7RealData();

  return true;
}

THREADED_CALLABLE_IMPL(csThreadTest, Test7RealData)
{
  csSleep(500);

  MutexScopedLock lock(test7lock);
  test7++;

  return true;
}

int main(int argc, char* argv[])
{
  csInitializer::InitializeSCF(argc, argv);
  iObjectRegistry* objReg = csInitializer::CreateObjectRegistry();
  csInitializer::CreateEventQueue(objReg);
  csInitializer::CreateThreadManager(objReg);

  csRef<iThreadTest> threadTest;
  threadTest.AttachNew(new csThreadTest(objReg));

  csRef<iThreadReturn> itr = threadTest->Test1();
  while(!itr->IsFinished());
  threadTest->Test2(false);
  threadTest->Test3(3, 0.1415f);
  csWeakRef<iThreadTest> test4 = threadTest;
  threadTest->Test4(test4);
  threadTest->Test5();

  csRef<Data> dat;
  dat.AttachNew(new Data());

  dat->i = 42;
  dat->f = 3.14159f;
  dat->b = true;
  dat->reallyBig = CONST_UINT64(123456789012345); 

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
  while(!threadTest->Test7Passed());
  printf("Test 7 passed!\n");

  printf("\nPress any key to exit.\n");
  WaitForKey ();
  printf("\n");

  objReg->Clear();

  return 0;
}
