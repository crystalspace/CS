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
#include "csutil/thread.h"
#include "iutil/job.h"

/**
 * iJobQueue implementation that lets the jobs run in a thread.
 */
class CS_CRYSTALSPACE_EXPORT csThreadJobQueue : public iJobQueue
{
  typedef csFIFO<csRef<iJob> > JobFifo;
  struct QueueAndRunnableShared
  {
    JobFifo* jobFifo;
    csRef<csMutex> fifoXS;
    csRef<csCondition> queueWake;
    csRef<iJob>* currentJob;
    csRef<csMutex> jobXS;
    csRef<csCondition> jobFinish;
  };
  class CS_CRYSTALSPACE_EXPORT QueueRunnable : public csRunnable
  {
    int refCount;
    QueueAndRunnableShared sharedData;
  public:
    QueueRunnable (const QueueAndRunnableShared& sharedData);
    virtual ~QueueRunnable();

    virtual void Run ();
    virtual void IncRef();
    virtual void DecRef();
    virtual int GetRefCount();
  };
  csRef<csThread> queueThread;
  QueueAndRunnableShared sharedData;
  csRef<iJob> currentJob;
  csRef<csMutex> jobFinishMutex;
  // stats
  uint jobsAdded, jobsPulled, jobsWaited, jobsUnqueued;
public:
  SCF_DECLARE_IBASE;

  csThreadJobQueue();
  virtual ~csThreadJobQueue();

  virtual void Enqueue (iJob* job);
  virtual void PullAndRun (iJob* job);
  virtual void Unqueue (iJob* job, bool waitIfCurrent = true);
};

#endif // __CS_CSUTIL_THREADJOBQUEUE_H__
