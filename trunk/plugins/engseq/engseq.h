/*
    Copyright (C) 2002-2006 by Jorrit Tyberghein

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
#include "csutil/csstring.h"
#include "csutil/refcount.h"
#include "csutil/refarr.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/selfdestruct.h"
#include "ivaria/engseq.h"
#include "iengine/sector.h"
#include "iengine/engine.h"

struct iObjectRegistry;
struct iSharedVariable;

CS_PLUGIN_NAMESPACE_BEGIN(EngSeq)
{

class csEngineSequenceManager;
class OpStandard;

/**
 * Implementation of iEngineSequenceParameters.
 */
class csEngineSequenceParameters : 
  public scfImplementation1<csEngineSequenceParameters, 
                            iEngineSequenceParameters>
{
private:
  struct par : public csRefCount
  {
    csString name;
    csRef<iBase> value;
  };
  csRefArray<par> params;

public:
  csEngineSequenceParameters () : scfImplementationType (this)
  { }
  virtual ~csEngineSequenceParameters ()
  { }

  virtual size_t GetParameterCount () const
  {
    return params.GetSize ();
  }
  virtual iBase* GetParameter (size_t idx) const
  {
    CS_ASSERT (idx < params.GetSize ());
    return params[idx]->value;
  }
  virtual iBase* GetParameter (const char* name) const
  {
    size_t i;
    for (i = 0 ; i < params.GetSize () ; i++)
      if (!strcmp (name, params[i]->name)) return params[i]->value;
    return 0;
  }
  virtual size_t GetParameterIdx (const char* name) const
  {
    size_t i;
    for (i = 0 ; i < params.GetSize () ; i++)
      if (!strcmp (name, params[i]->name)) return i;
    return csArrayItemNotFound;
  }
  virtual const char* GetParameterName (size_t idx) const
  {
    CS_ASSERT (idx < params.GetSize ());
    return params[idx]->name;
  }
  virtual void AddParameter (const char* name, iBase* def_value = 0)
  {
    par* p = new par;
    p->name = name;
    p->value = def_value;
    params.Push (p);
    p->DecRef ();
  }
  virtual void SetParameter (size_t idx, iBase* value)
  {
    CS_ASSERT (idx < params.GetSize ());
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
class csSequenceWrapper : public scfImplementationExt2<csSequenceWrapper,
						       csObject,
						       iSequenceWrapper,
						       iSelfDestruct>
{
private:
  csRef<iSequence> sequence;
  csEngineSequenceManager* eseqmgr;
  csRef<csEngineSequenceParameters> params;
  uint sequence_id;

protected:
  void InternalRemove() { SelfDestruct(); }

public:
  csSequenceWrapper (csEngineSequenceManager* eseqmgr, iSequence* sequence,
      uint sequence_id);
  virtual ~csSequenceWrapper ();

  uint GetSequenceID () const { return sequence_id; }

  virtual iSequence* GetSequence () { return sequence; }

  virtual iEngineSequenceParameters* CreateBaseParameterBlock ();
  virtual iEngineSequenceParameters* GetBaseParameterBlock ();
  virtual csPtr<iEngineSequenceParameters> CreateParameterBlock ();

  virtual void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, float value, float dvalue = 0);
  virtual void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, iSharedVariable* value,
		iSharedVariable* dvalue = 0);
  virtual void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, const csVector3& v);
  virtual void AddOperationSetVariable (csTicks time,
  		iSharedVariable* var, const csColor& c);
  virtual void AddOperationSetMaterial (csTicks time, iParameterESM* mesh,
		iParameterESM* mat);
  virtual void AddOperationSetLight (csTicks time, iParameterESM* light,
		  const csColor& color);
  virtual void AddOperationFadeLight (csTicks time, iParameterESM* light,
		  const csColor& color, csTicks duration);
  virtual void AddOperationSetAmbient (csTicks time, iParameterESM* sector,
		  const csColor& color,iSharedVariable *var);
  virtual void AddOperationFadeAmbient (csTicks time, iParameterESM* sector,
		  const csColor& color, csTicks duration);
  virtual void AddOperationRandomDelay(csTicks time,int min, int max);
  virtual void AddOperationSetMeshColor (csTicks time, iParameterESM* mesh,
		  const csColor& color);
  virtual void AddOperationFadeMeshColor (csTicks time, iParameterESM* mesh,
		  const csColor& color, csTicks duration);
  virtual void AddOperationSetFog (csTicks time, iParameterESM* sector,
		  const csColor& color, float density);
  virtual void AddOperationFadeFog (csTicks time, iParameterESM* sector,
		  const csColor& color, float density, csTicks duration);
  //virtual void AddOperationAbsoluteMove (csTicks time, iParameterESM* mesh,
		  //iParameterESM* sector, const csReversibleTransform& trans);
  //virtual void AddOperationAbsoluteMove (csTicks time, iParameterESM* mesh,
		  //iParameterESM* sector, const csVector3& pos);
  //virtual void AddOperationRelativeMove (csTicks time, iParameterESM* mesh,
		  //const csReversibleTransform& trans);
  //virtual void AddOperationRelativeMove (csTicks time, iParameterESM* mesh,
		  //const csVector3& pos);
  virtual void AddOperationRotateDuration (csTicks time, iParameterESM* mesh,
  		int axis1, float tot_angle1,
		int axis2, float tot_angle2,
		int axis3, float tot_angle3,
		const csVector3& offset,
		csTicks duration, bool relative);
  virtual void AddOperationMoveDuration (csTicks time, iParameterESM* mesh,
		const csVector3& offset,
		csTicks duration);
  virtual void AddOperationTriggerState (csTicks time, iParameterESM* trigger,
		  bool en);
  virtual void AddOperationCheckTrigger (csTicks time,
  		  iParameterESM* trigger, csTicks delay);
  virtual void AddOperationTestTrigger (csTicks time,
  		  iParameterESM* trigger,
		  iSequence* trueSequence,
		  iSequence* falseSequence);

  void OverrideTimings(OpStandard *afterop,int ticks);

  virtual iObject* QueryObject () { return this; }

  //--------------------- iSelfDestruct implementation -------------------//

  virtual void SelfDestruct ();
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
class csSequenceTrigger : public scfImplementationExt2<csSequenceTrigger,
						       csObject,
						       iSequenceTrigger,
						       iSelfDestruct>
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

protected:
  void InternalRemove() { SelfDestruct(); }

public:
  csSequenceTrigger (csEngineSequenceManager* eseqmgr);
  virtual ~csSequenceTrigger ();

  void AddConditionInSector (iSector* sector, bool insideonly,
		  const csBox3* box, const csSphere* sphere);
  virtual void AddConditionMeshClick (iMeshWrapper* mesh);
  virtual void AddConditionLightChange (iLight *whichlight, 
				int oper, const csColor& col);
  virtual void AddConditionManual ();

  virtual void SetEnabled (bool en) { enabled = en; }
  virtual bool IsEnabled () const { return enabled; }
  virtual void ClearConditions ();
  virtual void Trigger ();

  virtual void SetParameters (iEngineSequenceParameters* params)
  {
    csSequenceTrigger::params = params;
  }
  virtual iEngineSequenceParameters* GetParameters () const
  {
    return params;
  }

  virtual void FireSequence (csTicks delay, iSequenceWrapper* seq);
  virtual iSequenceWrapper* GetFiredSequence () { return fire_sequence; }

  virtual void TestConditions (csTicks delay);
  virtual bool CheckState ();
  csTicks GetConditionTestDelay () const { return condtest_delay; }
  void EnableOneTest ();

  iMeshWrapper* GetClickMesh () const { return click_mesh; }

  csEngineSequenceManager* GetEngineSequenceManager () const
  {
    return eseqmgr;
  }

  void Fire ();
  virtual void ForceFire (bool now = false);

  virtual iObject* QueryObject () { return this; }
  virtual void AddConditionInSector (iSector* sector)
  {
    AddConditionInSector (sector, true, 0, 0);
  }
  virtual void AddConditionInSector (iSector* sector, const csBox3& box)
  {
    AddConditionInSector (sector, true, &box, 0);
  }
  virtual void AddConditionInSector (iSector* sector, const csSphere& sphere)
  {
    AddConditionInSector (sector, true, 0, &sphere);
  }
  virtual void AddConditionSectorVisible (iSector* sector)
  {
    AddConditionInSector (sector, false, 0, 0);
  }

  //--------------------- iSelfDestruct implementation -------------------//

  virtual void SelfDestruct ();
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
  uint sequence_id;

public:
  csRef<iSequenceTimedOperation> op;
  csTicks start, end;

public:
  csTimedOperation (iSequenceTimedOperation* iop, iBase* iparams,
      uint sequence_id) : ref (1), sequence_id (sequence_id)
  {
    op = iop;
    params = iparams;
  }
  virtual ~csTimedOperation () { }
  iBase* GetParams () { return params; }
  uint GetSequenceID () const { return sequence_id; }

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

class csCameraCatcher : public scfImplementation1<csCameraCatcher,
	iEngineFrameCallback>
{
public:
  iCamera* camera;

public:
  csCameraCatcher () : scfImplementationType (this) { camera = 0; }
  virtual ~csCameraCatcher () { }
  virtual void StartFrame (iEngine* /*engine*/, iRenderView* rview)
  {
    camera = rview->GetCamera ();
  }
};

/**
 * Implementation of iEngineSequenceManager.
 */
class csEngineSequenceManager : 
  public scfImplementation2<csEngineSequenceManager, 
                            iEngineSequenceManager,
                            iComponent>
{
private:
  iObjectRegistry *object_reg;
  csRef<iSequenceManager> seqmgr;

  /// All loaded triggers.
  csRefArray<iSequenceTrigger> triggers;

  /// All loaded sequences.
  csRefArray<iSequenceWrapper> sequences;

  /// All triggers interested in knowing when a mesh is clicked.
  csArray<csSequenceTrigger*> mesh_triggers;

  /// All timed operations.
  csRefArray<csTimedOperation> timed_operations;
  uint32 global_framenr;

  /// Camera catcher.
  csRef<csCameraCatcher> cameracatcher;

  /// Refernce to the engine.
  csWeakRef<iEngine> engine;

public:
  csEngineSequenceManager (iBase *iParent);
  virtual ~csEngineSequenceManager ();
  virtual bool Initialize (iObjectRegistry *object_reg);

  /// This is set to receive the once per frame nothing event.
  virtual bool HandleEvent (iEvent &event);

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
	iBase* params = 0, uint sequence_id = 0);
  virtual void DestroyTimedOperations (uint sequence_id);

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

  struct EventHandler : 
    public scfImplementation1<EventHandler, iEventHandler>
  {
  private:
    csWeakRef<csEngineSequenceManager> parent;
  public:
    EventHandler (csEngineSequenceManager* p) : scfImplementationType (this),
      parent (p)
    { }
    virtual ~EventHandler ()
    { }
    virtual bool HandleEvent (iEvent& e) 
    { return parent ? parent->HandleEvent(e) : false; }
    CS_EVENTHANDLER_PHASE_2D("crystalspace.utilities.sequence.engine")
  };
  csRef<EventHandler> eventHandler;

  csEventID Frame;
  csEventID MouseEvent;
};

}
CS_PLUGIN_NAMESPACE_END(EngSeq)

#endif // __CS_ENGSEQ_H__

