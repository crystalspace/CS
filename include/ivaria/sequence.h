/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __IVARIA_SEQUENCE_H__
#define __IVARIA_SEQUENCE_H__

#include "csutil/scf.h"

struct iSequenceManager;

/**
 * A sequence operation. This is effectively a callback
 * to the application. DecRef() and IncRef() have to perform reference
 * counting. If the reference is zero the operation must be deleted.
 */
struct iSequenceOperation
{
  virtual void DecRef () = 0;
  virtual void IncRef () = 0;
  /**
   * Do the operation. The dt parameter is the difference between
   * the time the sequence manager is at now and the time this operation
   * was supposed to happen. If this is 0 then the operation is called
   * at the right time. If this is 1000 (for example) then the operation
   * was called one second late. This latency can happen because the
   * sequence manager only kicks in every frame and frame rate can be low.
   */
  virtual void Do (cs_time dt) = 0;
};

/**
 * A sequence condition. This is also a callback to the application.
 * This condition returns true on success. DecRef() and IncRef() have to
 * perform reference counting. If the reference is zero the condition
 * must be deleted.
 */
struct iSequenceCondition
{
  virtual void DecRef () = 0;
  virtual void IncRef () = 0;
  /**
   * Do the condition. The dt parameter is the difference between
   * the time the sequence manager is at now and the time this condition
   * was supposed to be called. If this is 0 then the condition is called
   * at the right time. If this is 1000 (for example) then the condition
   * was called one second late. This latency can happen because the
   * sequence manager only kicks in every frame and frame rate can be low.
   */
  virtual bool Condition (cs_time dt) = 0;
};

SCF_VERSION (iSequence, 0, 0, 1);

/**
 * A sequence of operations tagged with relative time information.
 * All operations added to this sequence will be executed relative to the
 * time the sequence itself is executed. The execute order of operations
 * added at the same relative time is undefined.
 */
struct iSequence : public iBase
{
  /**
   * Add an operation to this sequence. This function will call IncRef()
   * on the operation.
   */
  virtual void AddOperation (cs_time time, iSequenceOperation* operation) = 0;

  /**
   * Add a standard operation to execute another sequence. This function
   * will call IncRef() on the sequence.
   */
  virtual void AddRunSequence (cs_time time, iSequence* sequence) = 0;

  /**
   * Add a standard operation to perform a condition and execute the right
   * sequence depending on the result. This function will call
   * IncRef() on the condition and sequences.
   */
  virtual void AddCondition (cs_time time, iSequenceCondition* condition,
  	iSequence* trueSequence, iSequence* falseSequence) = 0;

  /**
   * Perform the sequence for as long as the condition is valid.
   * This function will call IncRef() on the condition and sequence.
   */
  virtual void AddLoop (cs_time time, iSequenceCondition* condition,
  	iSequence* sequence) = 0;

  /**
   * Clear all operations in this sequence (call DecRef()).
   */
  virtual void Clear () = 0;

  /**
   * Return true if this sequence is empty.
   */
  virtual bool IsEmpty () = 0;
};

SCF_VERSION (iSequenceManager, 0, 0, 1);

/**
 * The sequence manager. The sequence manager is a plugin that will perform
 * sequences of operations depending on elapsed time. It is mostly useful
 * for demo's or intros of games.
 */
struct iSequenceManager : public iBase
{
  /**
   * Clear all sequence operations currently in memory (this will
   * call DecRef() on them).
   */
  virtual void Clear () = 0;

  /**
   * Return true if the sequence manager has nothing to do (i.e. the
   * queue of sequence operations is empty).
   */
  virtual bool IsEmpty () = 0;

  /**
   * Suspend the sequence manager. This will totally stop all actions
   * that the sequence manager was doing. Use Resume() to resume.
   * Calling Suspend() on an already suspended sequence manager has no
   * effect. Note that a sequence manager is suspended by default.
   * This is so you can set it up and add the needed operations and then
   * call resume to start it all.
   */
  virtual void Suspend () = 0;

  /**
   * Resume the sequence manager at exactly the point it was previously
   * suspended. Calling Resume() on a running sequence manager has no
   * effect.
   */
  virtual void Resume () = 0;

  /**
   * Return true if the sequence manager is suspended.
   */
  virtual bool IsSuspended () = 0;

  /**
   * Perform a time warp. This will effectively let the sequence manager
   * think that the given time has passed. If the 'skip' flag is set then
   * all sequence parts that would have been executed in the skipped time
   * are not executed. Otherwise they will all be executed at the same time
   * (but the delta time parameter to 'Do' and 'Condition' will contain
   * the correct difference). 'time' is usually positive. When 'time'
   * is negative this will have the effect of adding extra time before
   * the first operation in the queue will be executed. i.e. we jump in the
   * past but operations that used to be there before are already deleted
   * and will not be executed again.
   */
  virtual void TimeWarp (cs_time time, bool skip) = 0;

  /**
   * Create a new empty sequence. This sequence is not attached to the
   * sequence manager in any way. After calling NewSequence() you can
   * add operations to it and then use RunSequence() to run it.
   */
  virtual iSequence* NewSequence () = 0;

  /**
   * Execute a sequence at the given time. This will effectively put the
   * sequence on the queue to be executed when the time has elapsed.
   * Modifications on a sequence after it has been added have no effect.
   * You can also remove the sequence (with DecRef()) immediatelly after
   * running it.
   */
  virtual void RunSequence (cs_time time, iSequence* sequence) = 0;
};

#endif // __IVARIA_SEQUENCE_H__

