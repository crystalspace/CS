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

#ifndef __CS_IUTIL_JOB_H__
#define __CS_IUTIL_JOB_H__

/**\file
 * Simple interfaces to manage delayed or parallel running of units of
 * work ("jobs").
 */

#include "csutil/scf_interface.h"


/**
 * A unit of work passed to iJobQueue.
 * \remark A job must be self-contained and thread-safe!
 */
struct iJob : public virtual iBase
{
  SCF_INTERFACE(iJob, 2,0,0);
  /// Do stuff.
  virtual void Run() = 0;
};

/**
 * Interface to simple job management system. 
 * 
 * The queue will execute the jobs according to the policy of the implementation,
 * such as in parallel using multiple threads.
 *
 * \remark Implementations makes no gurantees as to the order or latency for job
 * execution.
 * \sa csThreadJobQueue
 */
struct iJobQueue : public virtual iBase
{
  SCF_INTERFACE(iJobQueue,4,1,0);
  
  /// Add a job to the queue.
  virtual void Enqueue (iJob* job) = 0;

  /**
   * Status values for Dequeue and PullAndRun
   */
  enum JobStatus
  {
    /// The job was not enqueued
    NotEnqueued,
    /// The job is currently running
    Pending,
    /**
     * The job was pulled from a queue and executed (PullAndRun()) resp.
     * dropped (PullAndDrop())
     */
    Dequeued
  };
  
  /**
   * Remove a job from the queue.
   * If the job is currently running, this will wait for it to finish
   * if \a waitForCompletion is \c true, otherwise nothing is done.
   * \remarks The job queue will continue to hold a reference to the job
   *   if it is currently running (return value \c Pending). Wait for
   *   job completion if this is not desired.
   */
  virtual JobStatus Dequeue (iJob* job, bool waitForCompletion = false) = 0;
  
  /**
   * Check if a job is still in the queue and, 
   * if so, remove it from the queue and run it immediately.
   * If a job is currently running, either wait for it 
   * (\a waitForCompletion is \c true) to finish or just let it be.
   * \remarks The job queue will continue to hold a reference to the job
   *   if it is currently running (return value \c Pending). Wait for
   *   job completion if this is not desired.
   */
  virtual JobStatus PullAndRun (iJob* job, bool waitForCompletion = true) = 0;
  
  /**
   * Wait for all jobs in queue to finish executing.
   *
   * \remark Might return prematurely if jobs are enqueued or dequeued from
   * other threads during this call.
   */
  virtual void WaitAll () = 0;

  /**
   * Return true if all enqueued jobs are finished.
   *
   * \remark Might return wrong result if jobs are enqueued or dequeued from
   * other threads during this call.
   */
  virtual bool IsFinished () = 0;

  /**
   * Return the number of jobs in the queue.
   */
  virtual int32 GetQueueCount() = 0;


};

#endif // __CS_IUTIL_JOB_H__
