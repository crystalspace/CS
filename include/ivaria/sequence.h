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

#ifndef __CS_IVARIA_SEQUENCE_H__
#define __CS_IVARIA_SEQUENCE_H__

/**\file
 * Sequences
 */

#include "csutil/scf.h"

struct iSequenceManager;

SCF_VERSION (iSequenceOperation, 0, 2, 0);

/**
 * A sequence operation. This is effectively a callback
 * to the application.
 * 
 * Main creators of instances implementing this interface:
 * - Application using the sequence manager.
 *   
 * Main users of this interface:
 * - iSequence
 *   
 */
struct iSequenceOperation : public iBase
{
  /**
   * Do the operation. The dt parameter is the difference between
   * the time the sequence manager is at now and the time this operation
   * was supposed to happen. If this is 0 then the operation is called
   * at the right time. If this is 1000 (for example) then the operation
   * was called one second late. This latency can happen because the
   * sequence manager only kicks in every frame and frame rate can be low.
   */
  virtual void Do (csTicks dt, iBase* params) = 0;

  /**
   * This routine is responsible for forcibly cleaning up
   * all references to sequences. It is called when the sequence manager
   * is destructed.
   */
  virtual void CleanupSequences () = 0;
};

SCF_VERSION (iSequenceCondition, 0, 1, 0);

/**
 * A sequence condition. This is also a callback to the application.
 * This condition returns true on success.
 * 
 * Main creators of instances implementing this interface:
 * - Application using the sequence manager.
 *   
 * Main users of this interface:
 * - iSequence
 *   
 */
struct iSequenceCondition : public iBase
{
  /**
   * Do the condition. The dt parameter is the difference between
   * the time the sequence manager is at now and the time this condition
   * was supposed to be called. If this is 0 then the condition is called
   * at the right time. If this is 1000 (for example) then the condition
   * was called one second late. This latency can happen because the
   * sequence manager only kicks in every frame and frame rate can be low.
   */
  virtual bool Condition (csTicks dt, iBase* params) = 0;
};

struct csSequenceOp
{
  csSequenceOp* next, * prev;
  csTicks time;
  csRef<iBase> params;
  csRef<iSequenceOperation> operation;

  csSequenceOp () { }
  ~csSequenceOp () { }
};


SCF_VERSION (iSequence, 0, 0, 2);

/**
 * A sequence of operations tagged with relative time information.
 * All operations added to this sequence will be executed relative to the
 * time the sequence itself is executed. The execute order of operations
 * added at the same relative time is undefined.
 * 
 * Main creators of instances implementing this interface:
 * - iSequenceManager::NewSequence()
 *   
 * Main users of this interface:
 * - iSequenceManager
 *   
 */
struct iSequence : public iBase
{
  /**
   * Ugly but necessary for sequence to self-modify
   */
  virtual csSequenceOp* GetFirstSequence () = 0;

  /**
   * Add an operation to this sequence. This function will call IncRef()
   * on the operation.
   */
  virtual void AddOperation (csTicks time, iSequenceOperation* operation,
  	iBase* params = 0) = 0;

  /**
   * Add a standard operation to execute another sequence. This function
   * will NOT call IncRef() on the sequence.
   */
  virtual void AddRunSequence (csTicks time, iSequence* sequence,
  	iBase* params = 0) = 0;

  /**
   * Add a standard operation to perform a condition and execute the right
   * sequence depending on the result. This function will call
   * IncRef() on the condition, but NOT on the sequences.
   */
  virtual void AddCondition (csTicks time, iSequenceCondition* condition,
  	iSequence* trueSequence, iSequence* falseSequence,
	iBase* params = 0) = 0;

  /**
   * Perform the sequence for as long as the condition is valid.
   * This function will call IncRef() on the condition, but NOT on the 
   * sequence.
   */
  virtual void AddLoop (csTicks time, iSequenceCondition* condition,
  	iSequence* sequence, iBase* params = 0) = 0;

  /**
   * Clear all operations in this sequence (call DecRef()).
   */
  virtual void Clear () = 0;

  /**
   * Return true if this sequence is empty.
   */
  virtual bool IsEmpty () = 0;
};

SCF_VERSION (iSequenceManager, 0, 1, 0);

/**
 * The sequence manager. The sequence manager is a plugin that will perform
 * sequences of operations depending on elapsed time. It is mostly useful
 * for demo's or intros of games.
 * 
 * Main creators of instances implementing this interface:
 * - Sequence Manager plugin (crystalspace.utilities.sequence)
 *   
 * Main ways to get pointers to this interface:
 * - CS_QUERY_REGISTRY()
 *   
 * Main users of this interface:
 * - iEngineSequenceManager
 *   
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
  virtual void TimeWarp (csTicks time, bool skip) = 0;

  /**
   * Get the current time for the sequence manager. This is not
   * directly related to the real current time.
   * Suspending the sequence manager will also freeze this time.
   * Note that the sequence manager updates the main time AFTER
   * rendering frames. So if you want to get the real main time
   * you should add the delta returned by GetDeltaTime() too. However
   * from within operation callbacks you should just use GetMainTime()
   * in combination with the supplied delta.
   */
  virtual csTicks GetMainTime () const = 0;

  /**
   * Get the delta time to add to the main time to get the real main time.
   * Do not use GetDeltaTime() from within the operation callback.
   */
  virtual csTicks GetDeltaTime () const = 0;

  /**
   * Create a new empty sequence. This sequence is not attached to the
   * sequence manager in any way. After calling NewSequence() you can
   * add operations to it and then use RunSequence() to run it.
   */
  virtual csPtr<iSequence> NewSequence () = 0;

  /**
   * Execute a sequence at the given time. This will effectively put the
   * sequence on the queue to be executed when the time has elapsed.
   * Modifications on a sequence after it has been added have no effect.
   * You can also remove the sequence (with DecRef()) immediatelly after
   * running it.
   * 
   * The optional params instance will be given to all operations that
   * are added on the main sequence. Ref counting is used to keep track
   * of this object. So you can safely DecRef() your own reference after
   * calling RunSequence.
   */
  virtual void RunSequence (csTicks time, iSequence* sequence,
  	iBase* params = 0) = 0;
};

#endif // __CS_IVARIA_SEQUENCE_H__

