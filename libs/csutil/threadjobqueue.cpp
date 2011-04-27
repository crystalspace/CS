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
  static CS::Threading::Mutex rgenLock;
  static csRandomGen rgen;
}

namespace CS
{
namespace Threading
{

  ThreadedJobQueue::ThreadedJobQueue (size_t numWorkers, ThreadPriority priority,
    const char* name)
    : scfImplementationType (this), 
    numWorkerThreads (numWorkers), 
    outstandingJobs (0), name (name)
  {
    if (this->name.IsEmpty())
      this->name.Format ("Queue [%p]", this);

    allThreadState = new csRef<ThreadState>[numWorkerThreads];

    // Start up the threads
    for (unsigned int i = 0; i < numWorkerThreads; ++i)
    {
      allThreadState[i].AttachNew (new ThreadState (this, i)); 
      allThreadState[i]->threadObject->SetPriority(priority);

      allThreads.Add (allThreadState[i]->threadObject);
    }
    
    allThreads.StartAll ();
  }

  ThreadedJobQueue::~ThreadedJobQueue ()
  {
    // Kill all threads, friendly
    for(size_t i = 0; i < numWorkerThreads; ++i)
    {
      CS::Threading::AtomicOperations::Set (
	&(allThreadState[i]->runnable->shutdownQueue), 0xff);    
      allThreadState[i]->tsNewJob.NotifyAll ();
    }

    allThreads.WaitAll ();

    // Deallocate
    for (size_t i = 0; i < numWorkerThreads; ++i)
    {
      allThreadState[i]->runnable.Invalidate();
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
      size_t targetThread = rgen.Get ((uint32)numWorkerThreads);

      // Lock, add and notify
      ThreadState* ts = allThreadState[targetThread];

      // Might be contended, so try next if locked
      if (ts->tsMutex.TryLock ())
      {
        ts->jobQueue.Push (job);
        CS::Threading::AtomicOperations::Increment (&outstandingJobs); 
        ts->tsMutex.Unlock ();

        ts->tsNewJob.NotifyAll ();

        return;
      }
    }
    
  }

  iJobQueue::JobStatus ThreadedJobQueue::Dequeue (iJob* job, bool waitForCompletion)
  {
    // Check all the thread queues
    bool removedJob = PullFromQueues (job);
    
    if (!removedJob)
    {
      return Dequeued;
    }
    else
    {
      return CheckCompletion (job, waitForCompletion);
    }
    // Nothing
    return NotEnqueued;
  }


  iJobQueue::JobStatus ThreadedJobQueue::PullAndRun (iJob* job, bool waitForCompletion)
  {
    bool removedJob = PullFromQueues (job);

    if (removedJob)
    {      
      job->Run ();
      return Dequeued;
    }
    else
    {
      return CheckCompletion (job, waitForCompletion);
    }
    // Nothing
    return NotEnqueued;
  }
  
  iJobQueue::JobStatus ThreadedJobQueue::CheckCompletion (iJob* job, bool waitForCompletion)
  {
    // Check if it is running, then wait      
    ThreadState *ownerTs = 0;

    for (size_t i = 0; i < numWorkerThreads; ++i)
    {
      ThreadState* ts = allThreadState[i];
      
      ts->tsMutex.Lock ();

      if (ts->currentJob == job)
      {
	ownerTs = ts;
	break;
      }
      ts->tsMutex.Unlock (); // Unlock if not right
    }

    if (ownerTs)
    {
      JobStatus result;
      if (waitForCompletion)
      {
	// Always enter here with ownerTs->tsMutex locked!
	while (ownerTs->currentJob == job)
	{          
	  ownerTs->tsJobFinished.Wait (ownerTs->tsMutex);
	}
	result = Dequeued;
      }
      else
	result = Pending;

      ownerTs->tsMutex.Unlock (); // Unlock if right
      return result;
    }
    else
      return NotEnqueued;

    // All mutex that have been locked are unlocked!
  }

  void ThreadedJobQueue::WaitAll ()
  {   
    while(!IsFinished ())
    {
      for (size_t i = 0; i < numWorkerThreads; ++i)
      {
        ThreadState* ts = allThreadState[i];

        MutexScopedLock l (ts->tsMutex);
        if(ts->currentJob || ts->jobQueue.GetSize() > 0)
        {
          // Have a job, wait for it
          ts->tsJobFinished.Wait (ts->tsMutex);
        }
      }
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
    ThreadState* ts, unsigned int id)
    : ownerQueue (queue), shutdownQueue (0), threadState (ts)
  {
    name.Format ("#%u %s", id, queue->GetName());
  }

  void ThreadedJobQueue::QueueRunnable::Run ()
  {    
    // Forcibly keep QueueRunnable object alive until we got a shutdown
    this->IncRef();
    while (CS::Threading::AtomicOperations::Read(&(/*ownerQueue->*/shutdownQueue)) == 0x0)
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
        size_t start;
        {
          MutexScopedLock l (rgenLock);
          start = rgen.Get ((uint32)ownerQueue->numWorkerThreads);
        }

        for (size_t i = 0, index = start; 
             i < ownerQueue->numWorkerThreads; 
             ++i, index = (index + 1) % ownerQueue->numWorkerThreads
             )
        {
          ThreadState* foreignTS = ownerQueue->allThreadState[index];
          if (foreignTS == threadState)
            continue;

          // Try to lock it, but never wait for a lock
          if (foreignTS->tsMutex.TryLock ()) // Lock foreign object A
          {
            // Get the job
            if (foreignTS->jobQueue.GetSize() > 0)
            {
              currentJob = foreignTS->jobQueue.PopBottom ();
              foreignTS->tsMutex.Unlock (); // Unlock foreign object A if success
              break;
            }

            foreignTS->tsMutex.Unlock (); // Unlock foreign object A if unsuccessful
          } 
        }
      }

      if (currentJob)
      {
        // Got one, execute
        threadState->currentJob = currentJob;        
        threadState->tsMutex.Unlock (); // Unlock our own TS after getting a job

        currentJob->Run ();

	/* Release reference to job only until after the last access of the
	   thread manager.
	   This is a cautionary measure: the _job_ may keep the last ref to the
	   TM; releasing the job will also release the TM, causing segfaults
	   upon accesses to it. Thus keep it until after having accessed the
	   TM for the last time. */
	csRef<iJob> justKeepCurrentJobRefALittleLonger (currentJob);
        {
          MutexScopedLock l (threadState->tsMutex);
          threadState->currentJob = 0;
          currentJob = 0;
        }
       
        CS::Threading::AtomicOperations::Decrement(&(ownerQueue->outstandingJobs));
	
        threadState->tsJobFinished.NotifyAll ();
      }
      else
      {
        // Couldn't get one, wait for a newly added job        
        threadState->tsNewJob.Wait (threadState->tsMutex);
        threadState->tsMutex.Unlock ();
      }
    }
    
    // There is a circular ref between ThreadState and QueueRunnable, break it up
    threadState.Invalidate();
    
    this->DecRef();
  }

  const char* ThreadedJobQueue::QueueRunnable::GetName () const
  {
    return name.GetDataSafe ();
  }



}
}
