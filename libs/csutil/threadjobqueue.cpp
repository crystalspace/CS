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

SCF_IMPLEMENT_IBASE(csThreadJobQueue)
  SCF_IMPLEMENTS_INTERFACE(iJobQueue)
SCF_IMPLEMENT_IBASE_END

csThreadJobQueue::csThreadJobQueue()
{
  SCF_CONSTRUCT_IBASE(0);

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
  CS_ASSERT (res);
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
  SCF_DESTRUCT_IBASE();
}

void csThreadJobQueue::Enqueue (iJob* job)
{
  CS_ASSERT (job != 0);
  bool newJob;
  {
    csScopedMutexLock fifoLock (sharedData.fifoXS);
    newJob = sharedData.jobFifo->Length() == 0;
    sharedData.jobFifo->Push (job);
    if (newJob)
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
    while (currentJob == job)
    {
      sharedData.jobXS->Release();
      // wait for the job completion
      csScopedMutexLock jobLock (jobFinishMutex);
      sharedData.jobFinish->Wait (jobFinishMutex);
      sharedData.jobXS->LockTry();
    }
  }
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
    while (currentJob == job)
    {
      sharedData.jobXS->Release();
      // wait for the job completion
      csScopedMutexLock jobLock (jobFinishMutex);
      sharedData.jobFinish->Wait (jobFinishMutex);
      sharedData.jobXS->LockTry();
    }
  }
  sharedData.jobXS->Release();
}

//---------------------------------------------------------------------------

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
    if (!jobbing) sharedData.queueWake->Wait (awakenMutex);

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
