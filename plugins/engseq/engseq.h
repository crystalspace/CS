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
#include "csutil/csobject.h"
#include "csutil/refcount.h"
#include "csutil/refarr.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivaria/engseq.h"
#include "iengine/sector.h"

struct iObjectRegistry;
class csEngineSequenceManager;
class OpStandard;
struct iSharedVariable;

/**
 * Implementation of iEngineSequenceParameters.
 */
class csEngineSequenceParameters : public iEngineSequenceParameters
{
private:
  struct par : public csRefCount
  {
    char* name;
    csRef<iBase> value;
    virtual ~par () { delete[] name; }
  };
  csRefArray<par> params;

public:
  SCF_DECLARE_IBASE;

  csEngineSequenceParameters ()
  { SCF_CONSTRUCT_IBASE (0); }
  virtual ~csEngineSequenceParameters ()
  { SCF_DESTRUCT_IBASE(); }

  virtual size_t GetParameterCount () const
  {
    return params.Length ();
  }
  virtual iBase* GetParameter (size_t idx) const
  {
    CS_ASSERT (idx < params.Length ());
    return params[idx]->value;
  }
  virtual iBase* GetParameter (const char* name) const
  {
    size_t i;
    for (i = 0 ; i < params.Length () ; i++)
      if (!strcmp (name, params[i]->name)) return params[i]->value;
    return 0;
  }
  virtual size_t GetParameterIdx (const char* name) const
  {
    size_t i;
    for (i = 0 ; i < params.Length () ; i++)
      if (!strcmp (name, params[i]->name)) return i;
    return csArrayItemNotFound;
  }
  virtual const char* GetParameterName (size_t idx) const
  {
    CS_ASSERT (idx < params.Length ());
    return params[idx]->name;
  }
  virtual void AddParameter (const char* name, iBase* def_value = 0)
  {
    par* p = new par;
    p->name = csStrNew (name);
    p->value = def_value;
    params.Push (p);
    p->DecRef ();
  }
  virtual void SetParameter (size_t idx, iBase* value)
  {
    CS_ASSERT (idx < params.Length ());
    params[idx]->value = value;
  }
  virtual void SetParameter (const char* name, iBase* value)
  {
    size_t idx = GetParameterIdx (name);
    if (idx == csArrayItemNotFound) return;
    params[idx]->value = value;
  }
  virtual csPtr<iParameterESM> CreateParameterESM (const char* name);
};

/**
 * Wrapper for a sequence. Implements iSequenceWrapper.
 */
class csSequenceWrapper : public csObject
{
private:
  csRef<iSequence> sequence;
  csEngineSequenceManager* eseqmgr;
  csRef<csEngineSequenceParameters> params;

public:
  csSequenceWrapper (csEngineSequenceManager* eseqmgr, iSequence* sequence);
  virtual ~csSequenceWrapper ();

  iSequence* GetSequence () { return sequence; }

  iEngineSequenceParameters* CreateBaseParameterBlock ();
  iEngineSequenceParameters* GetBaseParameterBlock ();
  csPtr<iEngineSequenceParameters> CreateParameterBlock ();

  void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, float value, float dvalue = 0);
  void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, iSharedVariable* value,
		iSharedVariable* dvalue = 0);
  void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, const csVector3& v);
  void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, const csColor& c);
  void AddOperationSetMaterial (csTicks time, iParameterESM* mesh,
		  iParameterESM* mat);
  void AddOperationSetPolygonMaterial (csTicks time, iParameterESM* polygon,
		  iParameterESM* mat);
  void AddOperationSetLight (csTicks time, iParameterESM* light,
		  const csColor& color);
  void AddOperationFadeLight (csTicks time, iParameterESM* light,
		  const csColor& color, csTicks duration);
  void AddOperationSetAmbient (csTicks time, iParameterESM* sector,
		  const csColor& color,iSharedVariable *var);
  void AddOperationFadeAmbient (csTicks time, iParameterESM* sector,
		  const csColor& color, csTicks duration);
  void AddOperationRandomDelay(csTicks time,int min, int max);
  void AddOperationSetMeshColor (csTicks time, iParameterESM* mesh,
		  const csColor& color);
  void AddOperationFadeMeshColor (csTicks time, iParameterESM* mesh,
		  const csColor& color, csTicks duration);
  void AddOperationSetFog (csTicks time, iParameterESM* sector,
		  const csColor& color, float density);
  void AddOperationFadeFog (csTicks time, iParameterESM* sector,
		  const csColor& color, float density, csTicks duration);
  void AddOperationAbsoluteMove (csTicks time, iParameterESM* mesh,
		  iParameterESM* sector, const csReversibleTransform& trans);
  void AddOperationAbsoluteMove (csTicks time, iParameterESM* mesh,
		  iParameterESM* sector, const csVector3& pos);
  void AddOperationRelativeMove (csTicks time, iParameterESM* mesh,
		  const csReversibleTransform& trans);
  void AddOperationRelativeMove (csTicks time, iParameterESM* mesh,
		  const csVector3& pos);
  void AddOperationRotateDuration (csTicks time, iParameterESM* mesh,
  		int axis1, float tot_angle1,
		int axis2, float tot_angle2,
		int axis3, float tot_angle3,
		const csVector3& offset,
		csTicks duration);
  void AddOperationMoveDuration (csTicks time, iParameterESM* mesh,
		const csVector3& offset,
		csTicks duration);
  void AddOperationTriggerState (csTicks time, iParameterESM* trigger,
		  bool en);
  void AddOperationCheckTrigger (csTicks time,
  		  iParameterESM* trigger, csTicks delay);
  void AddOperationTestTrigger (csTicks time,
  		  iParameterESM* trigger,
		  iSequence* trueSequence,
		  iSequence* falseSequence);

  void OverrideTimings(OpStandard *afterop,int ticks);

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
    virtual iEngineSequenceParameters* CreateBaseParameterBlock ()
    {
      return scfParent->CreateBaseParameterBlock ();
    }
    virtual iEngineSequenceParameters* GetBaseParameterBlock ()
    {
      return scfParent->GetBaseParameterBlock ();
    }
    virtual csPtr<iEngineSequenceParameters> CreateParameterBlock ()
    {
      return scfParent->CreateParameterBlock ();
    }
    virtual void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, float value, float dvalue = 0)
    {
      scfParent->AddOperationSetVariable (time, var, value, dvalue);
    }
    virtual void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, iSharedVariable* value,
		iSharedVariable* dvalue = 0)
    {
      scfParent->AddOperationSetVariable (time, var, value, dvalue);
    }
    virtual void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, const csVector3& v)
    {
      scfParent->AddOperationSetVariable (time, var, v);
    }
    virtual void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, const csColor& c)
    {
      scfParent->AddOperationSetVariable (time, var, c);
    }
    virtual void AddOperationSetPolygonMaterial (csTicks time,
    		  iParameterESM* polygon, iParameterESM* mat)
    {
      scfParent->AddOperationSetPolygonMaterial (time, polygon, mat);
    }
    virtual void AddOperationSetMaterial (csTicks time, iParameterESM* mesh,
		  iParameterESM* mat)
    {
      scfParent->AddOperationSetMaterial (time, mesh, mat);
    }
    virtual void AddOperationSetLight (csTicks time, iParameterESM* light,
		  const csColor& color)
    {
      scfParent->AddOperationSetLight (time, light, color);
    }
    virtual void AddOperationFadeLight (csTicks time, iParameterESM* light,
		  const csColor& color, csTicks duration)
    {
      scfParent->AddOperationFadeLight (time, light, color, duration);
    }
    virtual void AddOperationSetAmbient (csTicks time, iParameterESM* sector,
		  const csColor& color, iSharedVariable *colorvar)
    {
      scfParent->AddOperationSetAmbient (time, sector, color, colorvar);
    }
    virtual void AddOperationFadeAmbient (csTicks time, iParameterESM* sector,
		  const csColor& color, csTicks duration)
    {
      scfParent->AddOperationFadeAmbient (time, sector, color, duration);
    }
    virtual void AddOperationRandomDelay (csTicks time,int min, int max)
    {
      scfParent->AddOperationRandomDelay (time,min,max);
    }
    virtual void AddOperationSetMeshColor (csTicks time, iParameterESM* mesh,
		  const csColor& color)
    {
      scfParent->AddOperationSetMeshColor (time, mesh, color);
    }
    virtual void AddOperationFadeMeshColor (csTicks time, iParameterESM* mesh,
		  const csColor& color, csTicks duration)
    {
      scfParent->AddOperationFadeMeshColor (time, mesh, color, duration);
    }
    virtual void AddOperationSetFog (csTicks time, iParameterESM* sector,
		  const csColor& color, float density)
    {
      scfParent->AddOperationSetFog (time, sector, color, density);
    }
    virtual void AddOperationFadeFog (csTicks time, iParameterESM* sector,
		  const csColor& color, float density, csTicks duration)
    {
      scfParent->AddOperationFadeFog (time, sector, color, density,
      	duration);
    }
    virtual void AddOperationRotateDuration (csTicks time, iParameterESM* mesh,
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
    virtual void AddOperationMoveDuration (csTicks time, iParameterESM* mesh,
		const csVector3& offset,
		csTicks duration)
    {
      scfParent->AddOperationMoveDuration (time, mesh, offset, duration);
    }
    virtual void AddOperationTriggerState (csTicks time,
    		  iParameterESM* trigger, bool en)
    {
      scfParent->AddOperationTriggerState (time, trigger, en);
    }
    virtual void AddOperationCheckTrigger (csTicks time,
  		  iParameterESM* trigger, csTicks delay)
    {
      scfParent->AddOperationCheckTrigger (time, trigger, delay);
    }
    virtual void AddOperationTestTrigger (csTicks time,
  		  iParameterESM* trigger,
		  iSequence* trueSequence,
		  iSequence* falseSequence)
    {
      scfParent->AddOperationTestTrigger (time, trigger,
		      trueSequence, falseSequence);
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
  int GetRefCount () { return ref; }
};

/**
 * Sequence trigger implementation. Implements iSequenceTrigger.
 */
class csSequenceTrigger : public csObject
{
private:
  bool enabled;
  bool enable_onetest;
  uint32 onetest_framenr;	// We test for this frame.
  iSequenceWrapper* fire_sequence;
  csRef<iEngineSequenceParameters> params;
  csEngineSequenceManager* eseqmgr;
  csTicks fire_delay;
  uint32 framenr;
  // Sequence created by TestConditions().
  csRef<iSequence> interval_seq;

  csRefArray<csConditionCleanup> condition_cleanups;

  // We are waiting for a click on this mesh.
  csRef<iMeshWrapper> click_mesh;

  bool last_trigger_state;
  csTicks condtest_delay;

  int total_conditions;
  int fired_conditions;

public:
  csSequenceTrigger (csEngineSequenceManager* eseqmgr);
  virtual ~csSequenceTrigger ();

  void AddConditionInSector (iSector* sector, bool insideonly,
		  const csBox3* box, const csSphere* sphere);
  void AddConditionMeshClick (iMeshWrapper* mesh);
  void AddConditionLightChange (iLight *whichlight, 
				int oper, const csColor& col);
  void AddConditionManual ();

  void SetEnabled (bool en) { enabled = en; }
  bool IsEnabled () const { return enabled; }
  void ClearConditions ();
  void Trigger ();

  void SetParameters (iEngineSequenceParameters* params)
  {
    csSequenceTrigger::params = params;
  }
  iEngineSequenceParameters* GetParameters () const
  {
    return params;
  }

  void FireSequence (csTicks delay, iSequenceWrapper* seq);
  iSequenceWrapper* GetFiredSequence () { return fire_sequence; }

  void TestConditions (csTicks delay);
  bool CheckState ();
  csTicks GetConditionTestDelay () const { return condtest_delay; }
  void EnableOneTest ();

  iMeshWrapper* GetClickMesh () const { return click_mesh; }

  csEngineSequenceManager* GetEngineSequenceManager () const
  {
    return eseqmgr;
  }

  void Fire ();
  void ForceFire (bool now);

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
      scfParent->AddConditionInSector (sector, true, 0, 0);
    }
    virtual void AddConditionInSector (iSector* sector,
	const csBox3& box)
    {
      scfParent->AddConditionInSector (sector, true, &box, 0);
    }
    virtual void AddConditionInSector (iSector* sector,
	const csSphere& sphere)
    {
      scfParent->AddConditionInSector (sector, true, 0, &sphere);
    }
    virtual void AddConditionSectorVisible (iSector* sector)
    {
      scfParent->AddConditionInSector (sector, false, 0, 0);
    }
    virtual void AddConditionMeshClick (iMeshWrapper* mesh)
    {
      scfParent->AddConditionMeshClick (mesh);
    }
    virtual void AddConditionLightChange (iLight *whichlight, 
					  int oper, const csColor& col)
    {
      scfParent->AddConditionLightChange(whichlight,oper,col);
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
    virtual void SetParameters (iEngineSequenceParameters* params)
    {
      scfParent->SetParameters (params);
    }
    virtual iEngineSequenceParameters* GetParameters () const
    {
      return scfParent->GetParameters ();
    }
    virtual void FireSequence (csTicks delay, iSequenceWrapper* seq)
    {
      scfParent->FireSequence (delay, seq);
    }
    virtual iSequenceWrapper* GetFiredSequence ()
    {
      return scfParent->GetFiredSequence ();
    }
    virtual void TestConditions (csTicks delay)
    {
      scfParent->TestConditions (delay);
    }
    virtual bool CheckState ()
    {
      return scfParent->CheckState ();
    }
    virtual void ForceFire (bool now = false)
    {
      scfParent->ForceFire (now);
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
  csRef<iBase> params;

public:
  csRef<iSequenceTimedOperation> op;
  csTicks start, end;

public:
  csTimedOperation (iSequenceTimedOperation* iop, iBase* iparams) : ref (1)
  {
    op = iop;
    params = iparams;
  }
  virtual ~csTimedOperation () { }
  iBase* GetParams () { return params; }

  void IncRef () { ref++; }
  void DecRef ()
  {
    ref--;
    if (ref <= 0)
    {
      delete this;
    }
  }
  int GetRefCount () { return ref; }
};

/**
 * Implementation of iEngineSequenceManager.
 */
class csEngineSequenceManager : public iEngineSequenceManager
{
private:
  iObjectRegistry *object_reg;
  csRef<iSequenceManager> seqmgr;

  /// All loaded triggers.
  csRefArray<iSequenceTrigger> triggers;

  /// All loaded sequences.
  csRefArray<iSequenceWrapper> sequences;

  /// All triggers interested in knowing when a mesh is clicked.
  csRefArray<csSequenceTrigger> mesh_triggers;

  /// All timed operations.
  csRefArray<csTimedOperation> timed_operations;
  uint32 global_framenr;

  /// Set the camera to use for some features.
  csWeakRef<iCamera> camera;

public:
  SCF_DECLARE_IBASE;

  csEngineSequenceManager (iBase *iParent);
  virtual ~csEngineSequenceManager ();
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// This is set to receive the once per frame nothing event.
  virtual bool HandleEvent (iEvent &event);

  virtual void SetCamera (iCamera* c)
  {
    camera = c;
  }
  virtual iCamera* GetCamera () { return camera; }
  virtual csPtr<iParameterESM> CreateParameterESM (iBase* value);
  virtual iSequenceManager* GetSequenceManager () { return seqmgr; }
  virtual csPtr<iSequenceTrigger> CreateTrigger (const char* name);
  virtual void RemoveTrigger (iSequenceTrigger* trigger);
  virtual void RemoveTriggers ();
  virtual size_t GetTriggerCount () const;
  virtual iSequenceTrigger* GetTrigger (size_t idx) const;
  virtual iSequenceTrigger* FindTriggerByName (const char* name) const;
  virtual bool FireTriggerByName (const char *name, bool now = false) const;
  virtual csPtr<iSequenceWrapper> CreateSequence (const char* name);
  virtual void RemoveSequence (iSequenceWrapper* seq);
  virtual void RemoveSequences ();
  virtual size_t GetSequenceCount () const;
  virtual iSequenceWrapper* GetSequence (size_t idx) const;
  virtual iSequenceWrapper* FindSequenceByName (const char* name) const;
  virtual bool RunSequenceByName (const char *name,int delay) const;
  virtual void FireTimedOperation (csTicks delta,
  	csTicks duration, iSequenceTimedOperation* op,
	iBase* params = 0);

  /**
   * Register a trigger that will be called whenever a mesh is clicked.
   */
  void RegisterMeshTrigger (csSequenceTrigger* trigger);
  /**
   * Unregister a mesh trigger.
   */
  void UnregisterMeshTrigger (csSequenceTrigger* trigger);

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
    SCF_DECLARE_IBASE;
    EventHandler (csEngineSequenceManager* p)
    {
      SCF_CONSTRUCT_IBASE (0);
      parent = p;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE ();
    }
    virtual bool HandleEvent (iEvent& e) { return parent->HandleEvent(e); }
  } * scfiEventHandler;
};

#endif // __CS_ENGSEQ_H__

