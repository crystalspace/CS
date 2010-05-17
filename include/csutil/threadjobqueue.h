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

#ifndef __CS_CSUTIL_THREADJOBQUEUE_H__
#define __CS_CSUTIL_THREADJOBQUEUE_H__

/**\file
 * Implementation of iJobQueue that lets the jobs run in a thread.
 */

#include "csextern.h"
#include "csutil/fifo.h"
#include "csutil/scf_implementation.h"
#include "csutil/csstring.h"
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
  /**
   * Construct job queue.
   * \param numWorkers Number of worker threads to use.
   * \param priority Priority of worker threads.
   * \param name Optional name of the queue.
   *   Used in worker thread naming and shows up in the debugger,
   *   if supported.
   */
  ThreadedJobQueue (size_t numWorkers = 1, ThreadPriority priority = THREAD_PRIO_NORMAL,
    const char* name = 0);
  virtual ~ThreadedJobQueue ();

  virtual void Enqueue (iJob* job);
  virtual JobStatus Dequeue (iJob* job, bool waitForCompletion);
  virtual JobStatus PullAndRun (iJob* job, bool waitForCompletion = true);
  virtual bool IsFinished ();  
  virtual int32 GetQueueCount();
  virtual void WaitAll ();

  /// Get name of this queue
  const char* GetName () const { return name; }
private:

  bool PullFromQueues (iJob* job);
  JobStatus CheckCompletion (iJob* job, bool waitForCompletion);

  // Runnable
  struct ThreadState;  

  class QueueRunnable : public Runnable
  {
  public:
    QueueRunnable (ThreadedJobQueue* queue, ThreadState* ts, unsigned int id);

    virtual void Run ();
    virtual const char* GetName () const;
  private:
    friend class ThreadedJobQueue;
    
    ThreadedJobQueue* ownerQueue;
    int32 shutdownQueue;
    csRef<ThreadState> threadState;
    csString name;
  };

  // Per thread state
  struct ThreadState : public CS::Utility::AtomicRefCount
  {
    ThreadState (ThreadedJobQueue* queue, unsigned int id)
    {
      runnable.AttachNew (new QueueRunnable (queue, this, id));
      threadObject.AttachNew (new Thread (runnable, false));
    }

    csRef<QueueRunnable> runnable;
    csRef<Thread> threadObject;
    csRef<iJob> currentJob;
    
    // 
    Mutex tsMutex;
    Condition tsNewJob;
    Condition tsJobFinished;

    csFIFO<csRef<iJob> > jobQueue;
  };

  csRef<ThreadState>* allThreadState;
  ThreadGroup allThreads;

  Mutex finishMutex;

  size_t numWorkerThreads;
  int32 outstandingJobs;
  csString name;
};

}
}

typedef CS::Threading::ThreadedJobQueue csThreadJobQueue;


#endif // __CS_CSUTIL_THREADJOBQUEUE_H__
