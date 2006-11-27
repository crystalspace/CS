/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#include "csgeom/math.h"
#include "csutil/scopedmutexlock.h"
#include "csutil/sysfunc.h"
#include "csutil/threadjobqueue.h"


namespace CS
{
namespace Threading
{

  ThreadedJobQueue::ThreadedJobQueue (size_t numWorkers)
    : scfImplementationType (this), 
    numWorkerThreads (csMin<size_t> (MAX_WORKER_THREADS, numWorkers)), 
    shutdownQueue (false)
  {
    // Start up the threads
    for (size_t i = 0; i < numWorkerThreads; ++i)
    {
      allThreadState[i] = new ThreadState (this);
      allThreads.Add (allThreadState[i]->threadObject);
    }
    allThreads.StartAll ();
  }

  ThreadedJobQueue::~ThreadedJobQueue ()
  {
    {
      // Empty the queue for new jobs
      MutexScopedLock lock (jobMutex);
      jobQueue.DeleteAll ();
    }

    // Wait for all threads to finish their current job
    shutdownQueue = true;
    newJob.NotifyAll ();
    allThreads.WaitAll ();

    // Deallocate
    for (size_t i = 0; i < numWorkerThreads; ++i)
    {
      delete allThreadState[i];
    }
  }


  void ThreadedJobQueue::Enqueue (iJob* job)
  {
    if (!job)
      return;

    MutexScopedLock lock (jobMutex);
    jobQueue.Push (job);
    newJob.NotifyOne ();
  }

  void ThreadedJobQueue::PullAndRun (iJob* job)
  {
    bool jobUnqued = false;

    {
      MutexScopedLock lock (jobMutex);
      // Check if in queue
      jobUnqued = jobQueue.Delete (job);
    }

    if (jobUnqued)
    {
      job->Run ();
      return;
    }

    // Now we have to check the active jobs, just wait until it is done
    {
      MutexScopedLock lock (threadStateMutex);

      bool isRunning = false;
      size_t index;

      for (size_t i = 0; i < numWorkerThreads; ++i)
      {
        if (allThreadState[i]->currentJob == job)
        {
          isRunning = true;
          index = i;
          break;
        }
      }

      if (isRunning)
      {
        while (allThreadState[index]->currentJob == job)
          allThreadState[index]->jobFinished.Wait (threadStateMutex);
      }

    }
  }

  void ThreadedJobQueue::Unqueue (iJob* job, bool waitIfCurrent)
  {
    {
      MutexScopedLock lock (jobMutex);
      // Check if in queue
      bool jobUnqued = jobQueue.Delete (job);

      if (jobUnqued)
        return;
    }

    {
      // Check the running threads
      MutexScopedLock lock (threadStateMutex);

      bool isRunning = false;
      size_t index;

      for (size_t i = 0; i < numWorkerThreads; ++i)
      {
        if (allThreadState[i]->currentJob == job)
        {
          isRunning = true;
          index = i;
          break;
        }
      }

      if (isRunning && waitIfCurrent)
      {
        while (allThreadState[index]->currentJob == job)
          allThreadState[index]->jobFinished.Wait (threadStateMutex);
      }

    }
  }

  ThreadedJobQueue::QueueRunnable::QueueRunnable (ThreadedJobQueue* queue, 
    ThreadState* ts)
    : ownerQueue (queue), threadState (ts)
  {
  }

  void ThreadedJobQueue::QueueRunnable::Run ()
  {
    while (true)
    {
      // Get a job
      {
        MutexScopedLock lock (ownerQueue->jobMutex);
        while (ownerQueue->jobQueue.Length () == 0)
        {
          if (ownerQueue->shutdownQueue)
            return;
          ownerQueue->newJob.Wait (ownerQueue->jobMutex);
        }

        {
          MutexScopedLock lock2 (ownerQueue->threadStateMutex);
          threadState->currentJob = ownerQueue->jobQueue.PopTop (); 
        }
      }

      // Execute it
      if (threadState->currentJob)
      {
        threadState->currentJob->Run ();
      }

      // Clean up
      {
        MutexScopedLock lock (ownerQueue->threadStateMutex);
        threadState->currentJob = 0;
        threadState->jobFinished.NotifyAll ();
      }

    }
  }

}
}
