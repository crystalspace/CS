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

#include "csutil/scopedmutexlock.h"
#include "csutil/sysfunc.h"
#include "csutil/threadjobqueue.h"

//#define THREADJOBQUEUE_PRINT_STATS

csThreadJobQueue::csThreadJobQueue()
  : scfImplementationType (this), jobsAdded (0), jobsPulled (0),
  jobsWaited (0), jobsUnqueued (0)
{
  jobFinishMutex = csMutex::Create ();
  sharedData.jobFifo = new JobFifo ();
  sharedData.fifoXS = csMutex::Create ();
  sharedData.queueWake = csCondition::Create ();
  sharedData.jobXS = csMutex::Create ();
  sharedData.currentJob = &currentJob;
  sharedData.jobFinish = csCondition::Create ();

  csRef<csRunnable> queueRunnable;
  queueRunnable.AttachNew (new QueueRunnable (sharedData));
  queueThread = csThread::Create (queueRunnable);
  bool res = queueThread->Start();
  CS_ASSERT (res); (void)res;
}

csThreadJobQueue::~csThreadJobQueue()
{
  {
    // Flush remaining jobs
    csScopedMutexLock fifoLock (sharedData.fifoXS);
    sharedData.jobFifo->DeleteAll();
  }
  sharedData.queueWake->Signal();
  queueThread->Wait();

  delete sharedData.jobFifo;
#ifdef THREADJOBQUEUE_PRINT_STATS
  csPrintf ("csThreadJobQueue %p: %u added, %u pulled, %u waited, %u unqueued\n",
    this, jobsAdded, jobsPulled, jobsWaited, jobsUnqueued);
#endif
}

void csThreadJobQueue::Enqueue (iJob* job)
{
  CS_ASSERT (job != 0);
  jobsAdded++;
  {
    csScopedMutexLock fifoLock (sharedData.fifoXS);
    sharedData.jobFifo->Length();
    sharedData.jobFifo->Push (job);
    sharedData.queueWake->Signal();
  }
}

void csThreadJobQueue::PullAndRun (iJob* job)
{
  CS_ASSERT (job != 0);
  sharedData.jobXS->LockWait();
  bool jobEnqueued;
  {
    csScopedMutexLock fifoLock (sharedData.fifoXS);
    jobEnqueued = sharedData.jobFifo->Delete (job);
  }
  if (!jobEnqueued)
  {
    bool didWait = false;
    while (currentJob == job)
    {
      csScopedMutexLock jobLock (jobFinishMutex);
      sharedData.jobXS->Release();
      // wait for the job completion
      if (!sharedData.jobFinish->Wait (jobFinishMutex, 500))
      {
	// @@@ Timeout not v. nice
#ifdef CS_DEBUG
	csPrintf ("csThreadJobQueue::PullAndRun(): timeout\n");
#endif
      }
      didWait = true;
      sharedData.jobXS->LockWait();
    }
    if (didWait) jobsWaited++;
  }
  else
    jobsPulled++;
  sharedData.jobXS->Release();
  // If it was in the queue, run now.
  // if it was not, it either was waited for above or already finished.
  if (jobEnqueued) job->Run();
}

void csThreadJobQueue::Unqueue (iJob* job, bool waitIfCurrent)
{
  CS_ASSERT (job != 0);
  sharedData.jobXS->LockWait();
  bool jobEnqueued;
  {
    csScopedMutexLock fifoLock (sharedData.fifoXS);
    jobEnqueued = sharedData.jobFifo->Delete (job);
  }
  if (!jobEnqueued && waitIfCurrent)
  {
    bool didWait = false;
    while (currentJob == job)
    {
      sharedData.jobXS->Release();
      // wait for the job completion
      csScopedMutexLock jobLock (jobFinishMutex);
      sharedData.jobFinish->Wait (jobFinishMutex);
      didWait = true;
      sharedData.jobXS->LockTry();
    }
    if (didWait) jobsWaited++;
  }
  else
    jobsUnqueued++;
  sharedData.jobXS->Release();
}

//---------------------------------------------------------------------------

//#define RUNNABLE_CHATTY

csThreadJobQueue::QueueRunnable::QueueRunnable (
  const QueueAndRunnableShared& sharedData)
{
  refCount = 1;
  this->sharedData = sharedData;
}

csThreadJobQueue::QueueRunnable::~QueueRunnable()
{
}

void csThreadJobQueue::QueueRunnable::Run ()
{
  csRef<csMutex> awakenMutex = csMutex::Create ();
  csScopedMutexLock awakeLock (awakenMutex);
  bool jobbing = false;
  while (true)
  {
#ifdef RUNNABLE_CHATTY
    if (!jobbing) csPrintf ("csThreadJobQueue::QueueRunnable::Run(): zzzz\n");
#endif
    if (!jobbing) sharedData.queueWake->Wait (awakenMutex);
#ifdef RUNNABLE_CHATTY
    if (!jobbing) csPrintf ("csThreadJobQueue::QueueRunnable::Run(): *yawn*\n");
#endif

    csRef<iJob> newJob;
    {
      csScopedMutexLock jobLock (sharedData.jobXS);
      {
	csScopedMutexLock fifoLock (sharedData.fifoXS);
	if (sharedData.jobFifo->Length() > 0)
	{
	  newJob = sharedData.jobFifo->PopTop();
	  jobbing = true;
	}
	else
	{
	  if (jobbing)
	    jobbing = false;
	  else
	    // We were awoken, but fifo empty? End running.
	    break;
	}
      }
      *sharedData.currentJob = newJob;
    }
    if (newJob.IsValid())
    {
      newJob->Run();
#ifdef RUNNABLE_CHATTY
      csPrintf ("csThreadJobQueue::QueueRunnable::Run(): finished a job, yay\n");
#endif
      {
	csScopedMutexLock jobLock (sharedData.jobXS);
	*sharedData.currentJob = 0;
	sharedData.jobFinish->Signal();
      }
    }
  }
}

void csThreadJobQueue::QueueRunnable::IncRef()
{
  refCount++;
}

void csThreadJobQueue::QueueRunnable::DecRef()
{
  if (--refCount == 0)
    delete this;
}

int csThreadJobQueue::QueueRunnable::GetRefCount()
{
  return refCount;
}
