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

#ifndef __CS_CSUTIL_THREADJOBQUEUE_H__
#define __CS_CSUTIL_THREADJOBQUEUE_H__

/**\file
 * Implementation of iJobQueue that lets the jobs run in a thread.
 */

#include "csextern.h"
#include "csutil/fifo.h"
#include "csutil/scf_implementation.h"
#include "iutil/job.h"

#include "csutil/threading/condition.h"
#include "csutil/threading/mutex.h"
#include "csutil/threading/thread.h"

namespace CS
{
namespace Threading
{

class CS_CRYSTALSPACE_EXPORT ThreadedJobQueue :
  public scfImplementation1<ThreadedJobQueue, iJobQueue>
{
public:
  ThreadedJobQueue (size_t numWorkers = 1, ThreadPriority priority = THREAD_PRIO_NORMAL,
    size_t numNonLowWorkers = 0);
  virtual ~ThreadedJobQueue ();

  virtual void Enqueue (iJob* job, bool lowPriority = false);
  virtual void PullAndRun (iJob* job);
  virtual void Unqueue (iJob* job, bool waitIfCurrent = true);
  virtual bool IsFinished ();
  virtual void Wait (iJob* job);
  virtual int32 GetQueueCount();

private:
  
  // Runnable
  struct ThreadState;

  class QueueRunnable : public Runnable
  {
  public:
    QueueRunnable (ThreadedJobQueue* queue, ThreadState* ts, bool doLow = true);

    virtual void Run ();

  private:
    ThreadedJobQueue* ownerQueue;
    ThreadState* threadState;
    bool doLow;
  };

  // Per thread state
  struct ThreadState
  {
    ThreadState (ThreadedJobQueue* queue, bool doLow = true)
    {
      runnable.AttachNew (new QueueRunnable (queue, this, doLow));
      threadObject.AttachNew (new Thread (runnable, false));
    }

    csRef<QueueRunnable> runnable;
    csRef<Thread> threadObject;
    csRef<iJob> currentJob;
    Condition jobFinished;
  };

  // Shared queue state
  typedef csFIFO<csRef<iJob> > JobFifo;
  JobFifo jobQueue;
  JobFifo jobQueueL;
  Mutex jobMutex;
  Condition newJob;

  ThreadState** allThreadState;
  ThreadGroup allThreads;
  Mutex threadStateMutex;
  // Condition to detect a finished job in any of the running threads
  Mutex jobFinishedMutex;
  Condition jobFinished;

  size_t numWorkerThreads;
  bool shutdownQueue;
  int32 outstandingJobs;
};

}
}

typedef CS::Threading::ThreadedJobQueue csThreadJobQueue;


#endif // __CS_CSUTIL_THREADJOBQUEUE_H__
