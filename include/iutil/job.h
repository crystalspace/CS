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

#ifndef __CS_IUTIL_JOB_H__
#define __CS_IUTIL_JOB_H__

/**\file
 * Simple interfaces to manage delayed or parallel running of units of
 * work ("jobs").
 */

#include "csutil/scf.h"

SCF_VERSION (iJob, 0, 0, 1);

/**
 * A unit of work passed to iJobQueue.
 * \remark A job must be self-contained and thread-safe!
 */
struct iJob : public iBase
{
  /// Do stuff.
  virtual void Run() = 0;
};

SCF_VERSION (iJobQueue, 0, 0, 1);

/**
 * Interface to simple job management. Jobs are enqueued and run one after
 * another, e.g. in another thread.
 * \sa csThreadJobQueue
 */
struct iJobQueue : public iBase
{
  /// Add a job to the queue.
  virtual void Enqueue (iJob* job) = 0;
  /**
   * Check if a job is still in the queue. If yes, remove it from the queue
   * and run it immediately.
   */
  virtual void PullAndRun (iJob* job) = 0;
  /**
   * Remove a job from the queue.
   * If the job is currently running and \a waitIfCurrent is true, wait until
   * the job has finished. This guarantees that the queue won't hold any 
   * reference to the job object.
   */
  virtual void Unqueue (iJob* job, bool waitIfCurrent = true) = 0;
};

#endif // __CS_IUTIL_JOB_H__
