/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __IVARIA_ENGSEQ_H__
#define __IVARIA_ENGSEQ_H__

#include "csutil/scf.h"

struct iSequence;
struct iSequenceManager;
struct iSequenceTrigger;
struct iSector;
struct iMeshWrapper;
struct iObject;
struct iLight;
class csColor;
class csReversibleTransform;
class csVector3;

SCF_VERSION (iSequenceWrapper, 0, 0, 1);

/**
 * A sequence wrapper. This objects holds the reference
 * to the original sequence and also implements iObject.
 */
struct iSequenceWrapper : public iBase
{
  /**
   * Query object.
   */
  virtual iObject* QueryObject () = 0;

  /**
   * Get the sequence. It is allowed to use the underlying sequence
   * for general sequence operations like adding conditions, operations,
   * and general sequence management. The AddOperationBla() functions
   * provided in this wrapper do nothing more than add custom operations
   * through the regular iSequence->AddOperation().
   */
  virtual iSequence* GetSequence () = 0;

  /**
   * Operation: set a light color.
   */
  virtual void AddOperationSetLight (csTicks time, iLight* light,
		  const csColor& color) = 0;

  /**
   * Operation: fade a light to some color during some time.
   */
  virtual void AddOperationFadeLight (csTicks time, iLight* light,
		  const csColor& color, csTicks duration) = 0;

  /**
   * Operation: set a fog color and density.
   */
  virtual void AddOperationSetFog (csTicks time, iSector* sector,
		  const csColor& color, float density) = 0;

  /**
   * Operation: fade fog to some color/density during some time.
   */
  virtual void AddOperationFadeFog (csTicks time, iSector* sector,
		  const csColor& color, float density, csTicks duration) = 0;

  /**
   * Operation: absolute move of object.
   */
  virtual void AddOperationAbsoluteMove (csTicks time, iMeshWrapper* mesh,
		  iSector* sector, const csReversibleTransform& trans) = 0;

  /**
   * Operation: absolute move of object.
   */
  virtual void AddOperationAbsoluteMove (csTicks time, iMeshWrapper* mesh,
		  iSector* sector, const csVector3& pos) = 0;

  /**
   * Operation: relative move of object.
   */
  virtual void AddOperationRelativeMove (csTicks time, iMeshWrapper* mesh,
		  const csReversibleTransform& trans) = 0;

  /**
   * Operation: relative move of object.
   */
  virtual void AddOperationRelativeMove (csTicks time, iMeshWrapper* mesh,
		  const csVector3& pos) = 0;

  /**
   * Operation: enable/disable a given trigger.
   */
  virtual void AddOperationTriggerState (csTicks time,
  		  iSequenceTrigger* trigger, bool en) = 0;
};

SCF_VERSION (iSequenceTrigger, 0, 0, 1);

/**
 * A sequence trigger. When all conditions in a trigger are
 * true it will run a sequence. Note that after the succesfull firing
 * of a trigger it will automatically be disabled.
 */
struct iSequenceTrigger : public iBase
{
  /**
   * Query object.
   */
  virtual iObject* QueryObject () = 0;

  /**
   * Condition: true if camera is in some sector.
   */
  virtual void AddConditionInSector (iSector* sector) = 0;

  /**
   * Condition: true if (part of) sector is visible.
   */
  virtual void AddConditionSectorVisible (iSector* sector) = 0;

  /**
   * Condition: true if camera is in bouding sphere of some mesh.
   */
  virtual void AddConditionInMeshSphere (iMeshWrapper* mesh) = 0;

  /**
   * Condition: true if camera is in bouding box of some mesh.
   */
  virtual void AddConditionInMeshBox (iMeshWrapper* mesh) = 0;

  /**
   * Condition: true if (part of) mesh object is visible.
   */
  virtual void AddConditionMeshVisible (iMeshWrapper* mesh) = 0;

  /**
   * Condition: manual trigger. Call this to set add a trigger
   * that requires manual confirmation. The 'Trigger()' function
   * can then be used later to actually do the trigger.
   */
  virtual void AddConditionManual () = 0;

  /**
   * Enable/disable this trigger. Triggers start enabled by
   * default.
   */
  virtual void SetEnabled (bool en) = 0;

  /**
   * Get enabled/disabled state.
   */
  virtual bool IsEnabled () const = 0;

  /**
   * Clear all conditions.
   */
  virtual void ClearConditions () = 0;

  /**
   * Trigger the manual condition.
   */
  virtual void Trigger () = 0;

  /**
   * Attach the sequence that will be fired when all trigger
   * conditions are valid.
   */
  virtual void FireSequence (csTicks delay, iSequenceWrapper* seq) = 0;

  /**
   * Get the attached sequence.
   */
  virtual iSequenceWrapper* GetFiredSequence () = 0;
};

SCF_VERSION (iSequenceTimedOperation, 0, 0, 1);

/**
 * A timed operation for the engine sequence manager.
 * This is basically something that needs to run over some period
 * of time. The 'elapsed' value that needs to be implemented by
 * subclasses will go from 0 to 1. When the time expires (goes beyond
 * 1) then the operation will be deleted automatically.
 * Timed operations are usually fired from within a sequence operation
 * (iSequenceOperation).
 */
struct iSequenceTimedOperation : public iBase
{
  /**
   * Do the operation. 'time' will be between 0 and 1.
   */
  virtual void Do (float time) = 0;
};

SCF_VERSION (iEngineSequenceManager, 0, 0, 1);

/**
 * Sequence manager specifically designed for working on
 * the engine.
 */
struct iEngineSequenceManager : public iBase
{
  /**
   * Get a pointer to the underlying sequence manager that
   * is being used.
   */
  virtual iSequenceManager* GetSequenceManager () = 0;

  //-----------------------------------------------------------------------

  /**
   * Create a new trigger with a given name.
   */
  virtual csPtr<iSequenceTrigger> CreateTrigger (const char* name) = 0;

  /**
   * Remove trigger from the manager.
   */
  virtual void RemoveTrigger (iSequenceTrigger* trigger) = 0;

  /**
   * Remove all triggers.
   */
  virtual void RemoveTriggers () = 0;

  /**
   * Get the number of triggers.
   */
  virtual int GetTriggerCount () const = 0;

  /**
   * Get a trigger.
   */
  virtual iSequenceTrigger* GetTrigger (int idx) const = 0;

  /**
   * Get a trigger by name.
   */
  virtual iSequenceTrigger* FindTriggerByName (const char* name) const = 0;

  //-----------------------------------------------------------------------

  /**
   * Create a new sequence with a given name.
   */
  virtual csPtr<iSequenceWrapper> CreateSequence (const char* name) = 0;

  /**
   * Remove sequence from the manager.
   */
  virtual void RemoveSequence (iSequenceWrapper* seq) = 0;

  /**
   * Remove all sequences.
   */
  virtual void RemoveSequences () = 0;

  /**
   * Get the number of sequences.
   */
  virtual int GetSequenceCount () const = 0;

  /**
   * Get a sequence.
   */
  virtual iSequenceWrapper* GetSequence (int idx) const = 0;

  /**
   * Get a sequence by name.
   */
  virtual iSequenceWrapper* FindSequenceByName (const char* name) const = 0;

  //-----------------------------------------------------------------------

  /**
   * Start a timed operation with a given delta (in ticks).
   * The delta has to be interpreted as the amount of time that has
   * already elapsed since the beginning of the timed operation.
   */
  virtual void FireTimedOperation (csTicks delta, csTicks duration,
  	iSequenceTimedOperation* op) = 0;

  //-----------------------------------------------------------------------
};

#endif // __IVARIA_ENGSEQ_H__

