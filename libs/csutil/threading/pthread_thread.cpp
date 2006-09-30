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
#include "csutil/threading/pthread_thread.h"
#include "csutil/threading/condition.h"


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
      ThreadStartParams (Runnable* runner, int32& isRunningFlag)
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
        AtomicOperations::Set (&isRunning, 1);
        startCondition.NotifyOne ();
      }

      void Stopped ()
      {
        AtomicOperations::Set (&isRunning, 0);
      }

    
      Mutex mutex;
      Condition startCondition;

      Runnable* runnable;
      int32& isRunning;
    };

    void* proxyFunc (void* param)
    {
      ThreadStartParams* tp = reinterpret_cast<ThreadStartParams*>
        (param);

      tp->Started ();

      tp->runnable->Run ();

      tp->Stopped ();
      
      pthread_exit (0);
      return 0;
    }

  }


  ThreadBase::ThreadBase (Runnable* runnable)
    : runnable (runnable), isRunning (false)
  {
  }

  void ThreadBase::Start ()
  {
    if (!IsRunning ())
    {
      ThreadStartParams param (runnable, isRunning);

      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
      int rc = pthread_create(&threadHandle, &attr, proxyFunc, &param); 
      
      param.Wait ();
    }
  }

  void ThreadBase::Stop ()
  {
    if (IsRunning ())
    {
      int res = pthread_cancel (threadHandle);
      if (res == 0)
      {
        AtomicOperations::Set (&isRunning, 0);
      }
    }
  }

  bool ThreadBase::IsRunning () const
  {
    return (AtomicOperations::Read ((int32*)&isRunning) != 0);
  }

  bool ThreadBase::SetPriority (ThreadPriority prio)
  {
    int res;
    struct sched_param SchedulerProperties;

    // Clear the properties initially
    memset(&SchedulerProperties, 0, sizeof (struct sched_param));

    // Map the CS thread priority identifier to an appropriate platform specific identifier
    //  or fail if this mapping is not possible.
    switch(prio)
    {
    case THREAD_PRIO_LOW:
      // Posix Pthreads does not guarantee support for any compatible priority,
      //  so we'll default to NORMAL
    case THREAD_PRIO_NORMAL:
      SchedulerProperties.sched_priority = sched_get_priority_max (SCHED_OTHER);
      res = pthread_setschedparam (thread, SCHED_OTHER, &SchedulerProperties);

      if (res != 0)
        return false;

      return true;
    case THREAD_PRIO_HIGH:
      SchedulerProperties.sched_priority = sched_get_priority_max (SCHED_RR) - 1;
      res = pthread_setschedparam (thread, SCHED_RR, &SchedulerProperties);

      if (res != 0)
        return false;

      return true;
    }

    return false;
  }

  void ThreadBase::Wait () const
  {
    if (IsRunning ())
    {
      int res = pthread_join (threadHandle,0);
    }
  }

  void ThreadBase::Yield () 
  {
    sched_yield ();
  }
}
}
}
