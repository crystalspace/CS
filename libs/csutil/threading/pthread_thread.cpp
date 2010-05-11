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
#include "csutil/threading/barrier.h"
#include "csutil/threading/condition.h"


namespace CS
{
namespace Threading
{
namespace Implementation
{

  namespace
  {

    class ThreadStartParams : public CS::Memory::CustomAllocated
    {
    public:
      ThreadStartParams (ThreadBase* thread, Runnable* runner, int32* isRunningPtr, 
        Barrier* startupBarrier)
        : thread (thread), runnable (runner), isRunningPtr (isRunningPtr), 
        startupBarrier (startupBarrier)
      {
      }

      ThreadBase* thread;
      Runnable* runnable;
      int32* isRunningPtr;
      Barrier* startupBarrier;
    };

    void* proxyFunc (void* param)
    {
      // Extract the parameters
      ThreadStartParams* tp = static_cast<ThreadStartParams*> (param);
      csRef<ThreadBase> thread (tp->thread);
      int32* isRunningPtr = tp->isRunningPtr;
      Runnable* runnable = tp->runnable;
      Barrier* startupBarrier = tp->startupBarrier;

      // Set as running and wait for main thread to catch up
      AtomicOperations::Set (isRunningPtr, 1);
      startupBarrier->Wait ();

    #ifdef CS_HAVE_PTHREAD_SETNAME_NP
      {
	// Set the name, for debugging
	const char* threadName = runnable->GetName ();
	if (threadName)
	  pthread_setname_np (pthread_self(), threadName);
      }
    #endif
      
      // Run      
      runnable->Run ();

      // Set as non-running
      AtomicOperations::Set (isRunningPtr, 0);
      
      return 0;
    }

  }


  ThreadBase::ThreadBase (Runnable* runnable)
    : runnable (runnable), isRunning (0), priority (THREAD_PRIO_NORMAL),
    startupBarrier (2)
  {
  }

  void ThreadBase::Start ()
  {
    if (!IsRunning ())
    {      
      ThreadStartParams param (this, runnable, &isRunning, &startupBarrier);

      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
      pthread_create(&threadHandle, &attr, proxyFunc, &param); 
            
      startupBarrier.Wait ();

      // Set priority to make sure its updated if we set it before starting
      SetPriority (priority);
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
    int res = 1;
    
    if (IsRunning ())
    {    
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
        res = pthread_setschedparam (threadHandle, SCHED_OTHER, &SchedulerProperties);
        break;

      case THREAD_PRIO_HIGH:
        SchedulerProperties.sched_priority = sched_get_priority_max (SCHED_RR) - 1;
        res = pthread_setschedparam (threadHandle, SCHED_RR, &SchedulerProperties);
        break;
      }
    }

    if (res != 0)
    {
      priority = prio;
    }

    return res != 0;
  }

  void ThreadBase::Wait () const
  {
    if (IsRunning ())
    {
      pthread_join (threadHandle,0);
    }
  }

  void ThreadBase::Yield () 
  {
    sched_yield ();
  }

  ThreadID ThreadBase::GetThreadID ()
  {
    return (ThreadID)pthread_self();
  }
}
}
}
