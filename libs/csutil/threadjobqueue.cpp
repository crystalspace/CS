/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter
              (C) 2009 by Marten Svanfeldt

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
#include "csutil/randomgen.h"

namespace
{
  static csRandomGen rgen;
}

namespace CS
{
namespace Threading
{

  ThreadedJobQueue::ThreadedJobQueue (size_t numWorkers, ThreadPriority priority)
    : scfImplementationType (this), 
    numWorkerThreads (numWorkers), 
    shutdownQueue (0), outstandingJobs (0)
  {
    allThreadState = new ThreadState*[numWorkerThreads];

    // Start up the threads
    for (size_t i = 0; i < numWorkerThreads; ++i)
    {
      allThreadState[i] = new ThreadState (this);      
      allThreadState[i]->threadObject->SetPriority(priority);

      allThreads.Add (allThreadState[i]->threadObject);
    }
    
    allThreads.StartAll ();
  }

  ThreadedJobQueue::~ThreadedJobQueue ()
  {
    // Kill all threads, friendly
    CS::Threading::AtomicOperations::Set (&shutdownQueue, 0xff);
    for(size_t i = 0; i < numWorkerThreads; ++i)
    {
      allThreadState[i]->tsNewJob.NotifyAll ();
    }

    allThreads.WaitAll ();

    // Deallocate
    for (size_t i = 0; i < numWorkerThreads; ++i)
    {
      delete allThreadState[i];
    }
    delete[] allThreadState;
  }


  void ThreadedJobQueue::Enqueue (iJob* job)
  {
    if (!job)
      return;

    while (true)
    {
      // Find a thread (on random) to add it to
      size_t targetThread = rgen.Get (numWorkerThreads);

      // Lock, add and notify
      ThreadState* ts = allThreadState[targetThread];

      // Might be contended, so try next if locked
      if (ts->tsMutex.TryLock ())
      {
        ts->jobQueue.Push (job);
        CS::Threading::AtomicOperations::Increment (&outstandingJobs);      
        ts->tsMutex.Unlock ();

        ts->tsNewJob.NotifyAll ();        
      }
    }
    
  }

  void ThreadedJobQueue::Dequeue (iJob* job)
  {
    // Check all the thread queues
    for (size_t i = 0; i < numWorkerThreads; ++i)
    {
      ThreadState* ts = allThreadState[i];
      MutexScopedLock l (ts->tsMutex);
      
      bool removed = ts->jobQueue.Delete (job);
      
      if (removed)
      {
        break;
      }
    }
  }


  void ThreadedJobQueue::PullAndRun (iJob* job, bool waitForCompletion)
  {
    bool removedJob = PullFromQueues (job);

    if (removedJob)
    {      
      job->Run ();
    }
    else if (waitForCompletion)
    {
      // Check if it is running, then wait

      size_t i;
      for (i = 0; i < numWorkerThreads; ++i)
      {
        ThreadState* ts = allThreadState[i];
        MutexScopedLock l (ts->tsMutex);

        if (ts->currentJob == job)
        {
          break;
        }
      }

      if (i < numWorkerThreads)
      {
        while (allThreadState[i]->currentJob == job)
        {
          MutexScopedLock l (finishMutex);
          jobFinished.Wait (finishMutex);
        }
      }
    }
    // Nothing
  }

  void ThreadedJobQueue::WaitAll ()
  {   
    while(!IsFinished ())
    {      
      jobFinished.Wait (finishMutex);
    }
  }
  
  bool ThreadedJobQueue::IsFinished ()
  {
    int32 c = CS::Threading::AtomicOperations::Read (&outstandingJobs);
    return c == 0;
  }

  int32 ThreadedJobQueue::GetQueueCount()
  {
    return CS::Threading::AtomicOperations::Read(&outstandingJobs);
  }

  bool ThreadedJobQueue::PullFromQueues (iJob* job)
  {
    // Check all the thread queues
    for (size_t i = 0; i < numWorkerThreads; ++i)
    {
      ThreadState* ts = allThreadState[i];
      MutexScopedLock l (ts->tsMutex);

      bool removedJob = ts->jobQueue.Delete (job);

      if (removedJob)
      {
        CS::Threading::AtomicOperations::Decrement(&outstandingJobs);
        return true;
      }
    }

    return false;
  }


  ThreadedJobQueue::QueueRunnable::QueueRunnable (ThreadedJobQueue* queue, 
    ThreadState* ts)
    : ownerQueue (queue), threadState (ts)
  {
  }

  void ThreadedJobQueue::QueueRunnable::Run ()
  {    
    while (CS::Threading::AtomicOperations::Read(&(ownerQueue->shutdownQueue)) == 0x0)
    {
      // Get a job
      csRef<iJob> currentJob;

      // Try our own list first            
      // We need to hold this until currentJob is set, otherwise something might slip through in "wait"
      threadState->tsMutex.Lock ();

      if (threadState->jobQueue.GetSize () > 0)
      {
        currentJob = threadState->jobQueue.PopTop ();
      }
      
      if (!currentJob)
      {
      
        // If we couldn't get any job, try to steal. At most try to steal once
        // from each of the other threads
        for (size_t i = 0, index = rgen.Get (ownerQueue->numWorkerThreads); 
             i < ownerQueue->numWorkerThreads; 
             ++i, index = (index + 1) % ownerQueue->numWorkerThreads
             )
        {
          ThreadState* foreignTS = ownerQueue->allThreadState[index];
          if (foreignTS == threadState)
            continue;


          // Lock it
          MutexScopedLock l (foreignTS->tsMutex);

          // Get the job
          if (foreignTS->jobQueue.GetSize() > 0)
          {
            currentJob = foreignTS->jobQueue.PopTop ();
            break;
          }
        }
      }

      if (currentJob)
      {
        CS::Threading::AtomicOperations::Decrement(&(ownerQueue->outstandingJobs));

        // Got one, execute
        threadState->currentJob = currentJob;        
        threadState->tsMutex.Unlock ();

        currentJob->Run ();
        threadState->currentJob = 0;
        currentJob = 0;

        ownerQueue->jobFinished.NotifyAll ();
      }
      else
      {
        // Couldn't get one, wait for a newly added job        
        threadState->tsNewJob.Wait (threadState->tsMutex);
        threadState->tsMutex.Unlock ();
      }
    }
  }

}
}
