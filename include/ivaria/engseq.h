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
   * Get the sequence.
   */
  virtual iSequence* GetSequence () = 0;

  /**
   * Operation: set a light color.
   */
  virtual void AddOperationSetLightColor (iLight* light,
		  const csColor& color) = 0;

  /**
   * Operation: fade a light to some color during some time.
   */
  virtual void AddOperationSetLightColor (iLight* light,
		  const csColor& color, csTicks duration) = 0;

  /**
   * Operation: absolute move of object.
   */
  virtual void AddOperationAbsoluteMove (iMeshWrapper* mesh,
		  iSector* sector, const csReversibleTransform& trans);

  /**
   * Operation: absolute move of object.
   */
  virtual void AddOperationAbsoluteMove (iMeshWrapper* mesh,
		  iSector* sector, const csVector3& pos);

  /**
   * Operation: relative move of object.
   */
  virtual void AddOperationRelativeMove (iMeshWrapper* mesh,
		  const csReversibleTransform& trans);

  /**
   * Operation: relative move of object.
   */
  virtual void AddOperationRelativeMove (iMeshWrapper* mesh,
		  const csVector3& pos);

  /**
   * Operation: enable/disable a given trigger.
   */
  virtual void AddOperationTriggerState (iSequenceTrigger* trigger,
		  bool en);
};

SCF_VERSION (iSequenceTrigger, 0, 0, 1);

/**
 * A sequence trigger. When all conditions in a trigger are
 * true it will run a sequence.
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
   * Enable/disable this trigger. Triggers start disabled by
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

  //-----------------------------------------------------------------------
};

#endif // __IVARIA_ENGSEQ_H__

