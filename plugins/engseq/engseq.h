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

#ifndef __CS_ENGSEQ_H__
#define __CS_ENGSEQ_H__

#include "csutil/util.h"
#include "csutil/csvector.h"
#include "csutil/csobject.h"
#include "csutil/refarr.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivaria/engseq.h"
#include "iengine/sector.h"

struct iObjectRegistry;
class csEngineSequenceManager;

/**
 * Wrapper for a sequence. Implements iSequenceWrapper.
 */
class csSequenceWrapper : public csObject
{
private:
  csRef<iSequence> sequence;
  csEngineSequenceManager* eseqmgr;

public:
  csSequenceWrapper (csEngineSequenceManager* eseqmgr, iSequence* sequence);
  virtual ~csSequenceWrapper ();

  iSequence* GetSequence () { return sequence; }
  void AddOperationSetLight (csTicks time, iLight* light,
		  const csColor& color);
  void AddOperationFadeLight (csTicks time, iLight* light,
		  const csColor& color, csTicks duration);
  void AddOperationSetFog (csTicks time, iSector* sector,
		  const csColor& color, float density);
  void AddOperationFadeFog (csTicks time, iSector* sector,
		  const csColor& color, float density, csTicks duration);
  void AddOperationAbsoluteMove (csTicks time, iMeshWrapper* mesh,
		  iSector* sector, const csReversibleTransform& trans);
  void AddOperationAbsoluteMove (csTicks time, iMeshWrapper* mesh,
		  iSector* sector, const csVector3& pos);
  void AddOperationRelativeMove (csTicks time, iMeshWrapper* mesh,
		  const csReversibleTransform& trans);
  void AddOperationRelativeMove (csTicks time, iMeshWrapper* mesh,
		  const csVector3& pos);
  void AddOperationRotateDuration (csTicks time, iMeshWrapper* mesh,
  		int axis1, float tot_angle1,
		int axis2, float tot_angle2,
		int axis3, float tot_angle3,
		const csVector3& offset,
		csTicks duration);
  void AddOperationMoveDuration (csTicks time, iMeshWrapper* mesh,
		const csVector3& offset,
		csTicks duration);
  void AddOperationTriggerState (csTicks time, iSequenceTrigger* trigger,
		  bool en);

  SCF_DECLARE_IBASE_EXT (csObject);

  //------------------- iSequenceWrapper implementation ------------------//
  struct SequenceWrapper : public iSequenceWrapper
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSequenceWrapper);
    virtual iObject* QueryObject ()
    {
      return scfParent;
    }
    virtual iSequence* GetSequence ()
    {
      return scfParent->GetSequence ();
    }
    virtual void AddOperationSetLight (csTicks time, iLight* light,
		  const csColor& color)
    {
      scfParent->AddOperationSetLight (time, light, color);
    }
    virtual void AddOperationFadeLight (csTicks time, iLight* light,
		  const csColor& color, csTicks duration)
    {
      scfParent->AddOperationFadeLight (time, light, color, duration);
    }
    virtual void AddOperationSetFog (csTicks time, iSector* sector,
		  const csColor& color, float density)
    {
      scfParent->AddOperationSetFog (time, sector, color, density);
    }
    virtual void AddOperationFadeFog (csTicks time, iSector* sector,
		  const csColor& color, float density, csTicks duration)
    {
      scfParent->AddOperationFadeFog (time, sector, color, density,
      	duration);
    }
    virtual void AddOperationAbsoluteMove (csTicks time, iMeshWrapper* mesh,
		  iSector* sector, const csReversibleTransform& trans)
    {
      scfParent->AddOperationAbsoluteMove (time, mesh, sector, trans);
    }
    virtual void AddOperationAbsoluteMove (csTicks time, iMeshWrapper* mesh,
		  iSector* sector, const csVector3& pos)
    {
      scfParent->AddOperationAbsoluteMove (time, mesh, sector, pos);
    }
    virtual void AddOperationRelativeMove (csTicks time, iMeshWrapper* mesh,
		  const csReversibleTransform& trans)
    {
      scfParent->AddOperationRelativeMove (time, mesh, trans);
    }
    virtual void AddOperationRelativeMove (csTicks time, iMeshWrapper* mesh,
		  const csVector3& pos)
    {
      scfParent->AddOperationRelativeMove (time, mesh, pos);
    }
    virtual void AddOperationRotateDuration (csTicks time, iMeshWrapper* mesh,
  		int axis1, float tot_angle1,
		int axis2, float tot_angle2,
		int axis3, float tot_angle3,
		const csVector3& offset,
		csTicks duration)
    {
      scfParent->AddOperationRotateDuration (time, mesh,
      	axis1, tot_angle1, axis2, tot_angle2, axis3, tot_angle3,
	offset, duration);
    }
    virtual void AddOperationMoveDuration (csTicks time, iMeshWrapper* mesh,
		const csVector3& offset,
		csTicks duration)
    {
      scfParent->AddOperationMoveDuration (time, mesh, offset, duration);
    }
    virtual void AddOperationTriggerState (csTicks time,
    		  iSequenceTrigger* trigger, bool en)
    {
      scfParent->AddOperationTriggerState (time, trigger, en);
    }
  } scfiSequenceWrapper;
  friend struct SequenceWrapper;
};

/**
 * For cleanup of a condition.
 */
class csConditionCleanup
{
private:
  int ref;

public:
  csConditionCleanup () : ref (1) { }
  virtual ~csConditionCleanup () { }

  virtual void Cleanup () = 0;
  void IncRef () { ref++; }
  void DecRef ()
  {
    ref--;
    if (ref <= 0)
    {
      Cleanup ();
      delete this;
    }
  }
};

/**
 * Sequence trigger implementation. Implements iSequenceTrigger.
 */
class csSequenceTrigger : public csObject
{
private:
  bool enabled;
  csRef<iSequenceWrapper> fire_sequence;
  csEngineSequenceManager* eseqmgr;
  csTicks fire_delay;
  uint32 framenr;
  csRefArray<csConditionCleanup> condition_cleanups;

  int total_conditions;
  int fired_conditions;

public:
  csSequenceTrigger (csEngineSequenceManager* eseqmgr);
  virtual ~csSequenceTrigger ();

  void AddConditionInSector (iSector* sector);
  void AddConditionSectorVisible (iSector* sector);
  void AddConditionInMeshSphere (iMeshWrapper* mesh);
  void AddConditionInMeshBox (iMeshWrapper* mesh);
  void AddConditionMeshVisible (iMeshWrapper* mesh);
  void AddConditionManual ();
  void SetEnabled (bool en) { enabled = en; }
  bool IsEnabled () const { return enabled; }
  void ClearConditions ();
  void Trigger ();
  void FireSequence (csTicks delay, iSequenceWrapper* seq);
  iSequenceWrapper* GetFiredSequence () { return fire_sequence; }

  csEngineSequenceManager* GetEngineSequenceManager () const
  {
    return eseqmgr;
  }

  void Fire ();

  SCF_DECLARE_IBASE_EXT (csObject);

  //------------------- iSequenceTrigger implementation ------------------//
  struct SequenceTrigger : public iSequenceTrigger
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSequenceTrigger);
    virtual iObject* QueryObject ()
    {
      return scfParent;
    }
    virtual void AddConditionInSector (iSector* sector)
    {
      scfParent->AddConditionInSector (sector);
    }
    virtual void AddConditionSectorVisible (iSector* sector)
    {
      scfParent->AddConditionSectorVisible (sector);
    }
    virtual void AddConditionInMeshSphere (iMeshWrapper* mesh)
    {
      scfParent->AddConditionInMeshSphere (mesh);
    }
    virtual void AddConditionInMeshBox (iMeshWrapper* mesh)
    {
      scfParent->AddConditionInMeshBox (mesh);
    }
    virtual void AddConditionMeshVisible (iMeshWrapper* mesh)
    {
      scfParent->AddConditionMeshVisible (mesh);
    }
    virtual void AddConditionManual ()
    {
      scfParent->AddConditionManual ();
    }
    virtual void SetEnabled (bool en)
    {
      scfParent->SetEnabled (en);
    }
    virtual bool IsEnabled () const
    {
      return scfParent->IsEnabled ();
    }
    virtual void ClearConditions ()
    {
      scfParent->ClearConditions ();
    }
    virtual void Trigger ()
    {
      scfParent->Trigger ();
    }
    virtual void FireSequence (csTicks delay, iSequenceWrapper* seq)
    {
      scfParent->FireSequence (delay, seq);
    }
    virtual iSequenceWrapper* GetFiredSequence ()
    {
      return scfParent->GetFiredSequence ();
    }
  } scfiSequenceTrigger;
  friend struct SequenceTrigger;
};

/**
 * An timed operation for the engine sequence manager.
 * This is basically something that needs to run over some period
 * of time. The 'elapsed' value that needs to be implemented by
 * subclasses will go from 0 to 1. When the time expires (goes beyond
 * 1) then the operation will be deleted automatically.
 */
class csTimedOperation
{
private:
  int ref;

public:
  csRef<iSequenceTimedOperation> op;
  csTicks start, end;

public:
  csTimedOperation (iSequenceTimedOperation* op) : ref (1)
  {
    csTimedOperation::op = op;
  }
  virtual ~csTimedOperation () { }

  void IncRef () { ref++; }
  void DecRef ()
  {
    ref--;
    if (ref <= 0)
    {
      delete this;
    }
  }
};

/**
 * Implementation of iEngineSequenceManager.
 */
class csEngineSequenceManager : public iEngineSequenceManager
{
private:
  iObjectRegistry *object_reg;
  csRef<iSequenceManager> seqmgr;
  csRefArray<iSequenceTrigger> triggers;
  csRefArray<iSequenceWrapper> sequences;
  csRefArray<csTimedOperation> timed_operations;
  uint32 global_framenr;

public:
  SCF_DECLARE_IBASE;

  csEngineSequenceManager (iBase *iParent);
  virtual ~csEngineSequenceManager ();
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// This is set to receive the once per frame nothing event.
  virtual bool HandleEvent (iEvent &event);

  virtual iSequenceManager* GetSequenceManager () { return seqmgr; }
  virtual csPtr<iSequenceTrigger> CreateTrigger (const char* name);
  virtual void RemoveTrigger (iSequenceTrigger* trigger);
  virtual void RemoveTriggers ();
  virtual int GetTriggerCount () const;
  virtual iSequenceTrigger* GetTrigger (int idx) const;
  virtual iSequenceTrigger* FindTriggerByName (const char* name) const;
  virtual csPtr<iSequenceWrapper> CreateSequence (const char* name);
  virtual void RemoveSequence (iSequenceWrapper* seq);
  virtual void RemoveSequences ();
  virtual int GetSequenceCount () const;
  virtual iSequenceWrapper* GetSequence (int idx) const;
  virtual iSequenceWrapper* FindSequenceByName (const char* name) const;
  virtual void FireTimedOperation (csTicks delta,
  	csTicks duration, iSequenceTimedOperation* op);

  /// Get the global frame number.
  uint32 GetGlobalFrameNr () const { return global_framenr; }

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csEngineSequenceManager);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;

  struct EventHandler : public iEventHandler
  {
  private:
    csEngineSequenceManager* parent;
  public:
    EventHandler (csEngineSequenceManager* parent)
    {
      SCF_CONSTRUCT_IBASE (NULL);
      EventHandler::parent = parent;
    }
    SCF_DECLARE_IBASE;
    virtual bool HandleEvent (iEvent& e) { return parent->HandleEvent(e); }
  } * scfiEventHandler;
};

#endif // __CS_ENGSEQ_H__

