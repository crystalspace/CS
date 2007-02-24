/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#include "cssysdef.h"

#include "csutil/threading/thread.h"
#include "csutil/threading/win32_thread.h"
#include "csutil/threading/condition.h"

#include <windows.h>

#if defined (__CYGWIN__) 
//No _beginthreadex, emulate it
struct ThreadProxyData
{
  typedef unsigned (__stdcall* func)(void*);
  func startAddress;

  void* arglist;
  ThreadProxyData (func startAddress,void* arglist) : 
    startAddress (startAddress), arglist (arglist) 
  {}
};

DWORD WINAPI ThreadProxy (LPVOID args)
{
  ThreadProxyData* data = reinterpret_cast<ThreadProxyData*>(args);
  DWORD ret = data->startAddress (data->arglist);
  delete data;
  return ret;
}

inline unsigned _beginthreadex (void* security, unsigned stack_size, 
                                unsigned (__stdcall* start_address)(void*),
                                void* arglist, unsigned initflag,
                                unsigned* thrdaddr)
{
  DWORD threadID;
  HANDLE hthread  = CreateThread  (static_cast<LPSECURITY_ATTRIBUTES> (security),
    stack_size, ThreadProxy, new ThreadProxyData (start_address, arglist),
    initflag, &threadID);

  if (hthread!=0)
    *thrdaddr=threadID;

  return reinterpret_cast<unsigned> (hthread);
}

#endif

namespace CS
{
namespace Threading
{
namespace Implementation
{

  namespace
  {

    class ThreadStartParams
    {
    public:
      ThreadStartParams (Runnable* runner, bool& isRunningFlag)
        : runnable (runner), isRunning (isRunningFlag)
      {
      }

      // Wait for thread to start up
      void Wait ()
      {
        ScopedLock<Mutex> lock (mutex);
        while (!isRunning)
          startCondition.Wait (mutex);
      }

      void Started ()
      {
        ScopedLock<Mutex> lock (mutex);
        isRunning = true;
        startCondition.NotifyOne ();
      }

      void Stopped ()
      {
        ScopedLock<Mutex> lock (mutex);
        isRunning = false;
      }

    
      Mutex mutex;
      Condition startCondition;

      Runnable* runnable;
      bool& isRunning;
    };

    unsigned int __stdcall proxyFunc (void* param)
    {
      ThreadStartParams* tp = reinterpret_cast<ThreadStartParams*>
        (param);

      tp->Started ();

      tp->runnable->Run ();

      tp->Stopped ();

      return 0;
    }

  }


  ThreadBase::ThreadBase (Runnable* runnable)
    : runnable (runnable), threadHandle (0), threadId (0), isRunning (false)
  {
  }

  void ThreadBase::Start ()
  {
    if (!threadHandle)
    {
      ThreadStartParams param (runnable, isRunning);

      threadHandle = reinterpret_cast<void*> (_beginthreadex (0, 0, 
        &proxyFunc, &param, CREATE_SUSPENDED, &threadId));

      if (threadHandle == 0)
        return;

      param.Wait ();
    }
  }

  void ThreadBase::Stop ()
  {
    if (threadHandle)
    {
      int res = TerminateThread (threadHandle, ~0);
      if (res == 0)
      {
        threadHandle = 0;
        isRunning = false;
      }
    }
  }

  bool ThreadBase::IsRunning () const
  {
    return isRunning;
  }

  bool ThreadBase::SetPriority (ThreadPriority prio)
  {
    int PrioTable[] = {
      THREAD_PRIORITY_IDLE,
      THREAD_PRIORITY_NORMAL,
      THREAD_PRIORITY_HIGHEST
    };

    int res = SetThreadPriority (threadHandle, PrioTable[prio]);

    return res != 0;
  }

  void ThreadBase::Wait () const
  {
    if (threadHandle)
    {
      int res = 0;

      res = WaitForSingleObject (threadHandle, INFINITE);
      res = CloseHandle (threadHandle);
      threadHandle = 0;
    }
  }

}
}
}
