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
#include "csutil/sysfunc.h"
#include "csutil/threadjobqueue.h"


namespace CS
{
namespace Threading
{

  ThreadedJobQueue::ThreadedJobQueue (size_t numWorkers, ThreadPriority priority,
    size_t numNonLowWorkers)
    : scfImplementationType (this), 
    numWorkerThreads (numWorkers+numNonLowWorkers), 
    shutdownQueue (false), outstandingJobs (0)
  {
    allThreadState = new ThreadState*[numWorkerThreads];

    // Start up the threads
    for (size_t i = 0; i < numWorkerThreads; ++i)
    {
      if(i < numWorkers)
        allThreadState[i] = new ThreadState (this);
      else
        allThreadState[i] = new ThreadState (this, false);
      allThreadState[i]->threadObject->SetPriority(priority);
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
      jobQueueL.DeleteAll ();

      // Wait for all threads to finish their current job
      shutdownQueue = true;
    }

    newJob.NotifyAll ();
    allThreads.WaitAll ();

    // Deallocate
    for (size_t i = 0; i < numWorkerThreads; ++i)
    {
      delete allThreadState[i];
    }
    delete[] allThreadState;
  }


  void ThreadedJobQueue::Enqueue (iJob* job, bool lowPriority)
  {
    if (!job)
      return;

    {
      MutexScopedLock lock (jobMutex);
      if(lowPriority)
        jobQueueL.Push (job);
      else
        jobQueue.Push (job);
    }
    CS::Threading::AtomicOperations::Increment (&outstandingJobs);
    newJob.NotifyAll ();
  }

  void ThreadedJobQueue::PullAndRun (iJob* job)
  {
    bool jobUnqued = false;

    {
      MutexScopedLock lock (jobMutex);
      // Check if in queue
      jobUnqued = jobQueue.Delete (job) || jobQueueL.Delete (job);
    }

    if (jobUnqued)
    {
      CS::Threading::AtomicOperations::Decrement (&outstandingJobs);
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

  void ThreadedJobQueue::PopAndRun()
  {
    csRef<iJob> job;
    {
      MutexScopedLock lock (jobMutex);
      if(jobQueue.GetSize () > 0)
      {
        job = jobQueue.PopTop();
      }
      else if(jobQueueL.GetSize () > 0)
      {
        job = jobQueueL.PopTop();
      }
    }
    
    if(job.IsValid())
    {
      CS::Threading::AtomicOperations::Decrement (&outstandingJobs);
      job->Run ();
    }
  }

  void ThreadedJobQueue::Unqueue (iJob* job, bool waitIfCurrent)
  {
    {
      MutexScopedLock lock (jobMutex);
      // Check if in queue
      bool jobUnqued = jobQueue.Delete (job) || jobQueueL.Delete (job);

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
  
  void ThreadedJobQueue::Wait (iJob* job)
  {
    while (true)
    {
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

        if (isRunning)
        {
          /* The job is currently running, so wait until it finished */
          while (allThreadState[index]->currentJob == job)
            allThreadState[index]->jobFinished.Wait (threadStateMutex);
          return;
        }
      }

      {
        MutexScopedLock lock (jobMutex);
        // Check if in queue
        bool jobUnqued = jobQueue.Contains (job) || jobQueueL.Contains (job);

        if (!jobUnqued)
          // Not queued or running at all (any more)
          return;
      }

      /* The job is somewhere in a queue.
      * Just wait for any job to finish and check everything again...
      */
      if (!IsFinished())
      {
        MutexScopedLock lock(jobFinishedMutex);
        jobFinished.Wait (jobFinishedMutex);
      }
    }
  }
  
  bool ThreadedJobQueue::IsFinished ()
  {
    int32 c = CS::Threading::AtomicOperations::Read (&outstandingJobs);
    return c == 0;
  }

  ThreadedJobQueue::QueueRunnable::QueueRunnable (ThreadedJobQueue* queue, 
    ThreadState* ts, bool doLow)
    : ownerQueue (queue), threadState (ts), doLow(doLow)
  {
  }

  void ThreadedJobQueue::QueueRunnable::Run ()
  {
    while (true)
    {
      // Get a job
      {
        MutexScopedLock lock (ownerQueue->jobMutex);
        while (ownerQueue->jobQueue.GetSize () == 0 &&
          (!doLow || ownerQueue->jobQueueL.GetSize () == 0))
        {
          if (ownerQueue->shutdownQueue)
            return;
          ownerQueue->newJob.Wait (ownerQueue->jobMutex);
        }

        {
          MutexScopedLock lock2 (ownerQueue->threadStateMutex);
          if (ownerQueue->jobQueue.GetSize () > 0)
            threadState->currentJob = ownerQueue->jobQueue.PopTop ();
          else if (doLow)
            threadState->currentJob = ownerQueue->jobQueueL.PopTop ();
        }
      }

      // Execute it
      if (threadState->currentJob)
      {
        CS::Threading::AtomicOperations::Decrement (&(ownerQueue->outstandingJobs));
        threadState->currentJob->Run ();
      }

      // Clean up
      {
        MutexScopedLock lock (ownerQueue->threadStateMutex);
        threadState->currentJob.Invalidate ();
        threadState->jobFinished.NotifyAll ();
      }
      {
        ownerQueue->jobFinished.NotifyAll ();
      }
    }
  }

  int32 ThreadedJobQueue::GetQueueCount()
  {
    return CS::Threading::AtomicOperations::Read(&outstandingJobs);
  }

}
}
