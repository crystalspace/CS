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
#include <string.h>

#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "engseq.h"
#include "csutil/scf.h"
#include "iutil/objreg.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivaria/sequence.h"
#include "ivaria/reporter.h"
#include "iengine/light.h"
#include "iengine/sector.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "csutil/cscolor.h"

CS_IMPLEMENT_PLUGIN

//---------------------------------------------------------------------------

/**
 * The superclass of all sequence operations.
 */
class OpStandard : public iSequenceOperation
{
protected:
  virtual ~OpStandard () { }

public:
  OpStandard () { SCF_CONSTRUCT_IBASE (NULL); }
  SCF_DECLARE_IBASE;
};

SCF_IMPLEMENT_IBASE (OpStandard)
  SCF_IMPLEMENTS_INTERFACE (iSequenceOperation)
SCF_IMPLEMENT_IBASE_END

//---------------------------------------------------------------------------

/**
 * Set fog operation.
 */
class OpSetFog : public OpStandard
{
private:
  csRef<iSector> sector;
  csColor color;
  float density;

public:
  OpSetFog (iSector* sector, const csColor& color, float density)
  {
    OpSetFog::sector = sector;
    OpSetFog::color = color;
    OpSetFog::density = density;
  }

  virtual void Do (csTicks dt)
  {
    if (density < 0.001)
      sector->DisableFog ();
    else
      sector->SetFog (density, color);
  }
};

/**
 * Fade fog timed operation.
 */
class TimedOpFadeFog : public iSequenceTimedOperation
{
private:
  csRef<iSector> sector;
  csColor start_col, end_col;
  float start_density, end_density;

public:
  TimedOpFadeFog (iSector* sector,
  	const csColor& start_col, float start_density,
  	const csColor& end_col, float end_density)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    TimedOpFadeFog::sector = sector;
    TimedOpFadeFog::start_col = start_col;
    TimedOpFadeFog::start_density = start_density;
    TimedOpFadeFog::end_col = end_col;
    TimedOpFadeFog::end_density = end_density;
  }
  virtual ~TimedOpFadeFog () { }

  SCF_DECLARE_IBASE;

  virtual void Do (float time)
  {
    float density = (1-time) * start_density + time * end_density;
    if (density < 0.001)
      sector->DisableFog ();
    else
    {
      csColor color;
      color.red = (1-time) * start_col.red + time * end_col.red;
      color.green = (1-time) * start_col.green + time * end_col.green;
      color.blue = (1-time) * start_col.blue + time * end_col.blue;
      sector->SetFog (density, color);
    }
  }
};

SCF_IMPLEMENT_IBASE (TimedOpFadeFog)
  SCF_IMPLEMENTS_INTERFACE (iSequenceTimedOperation)
SCF_IMPLEMENT_IBASE_END

/**
 * Fade fog operation.
 */
class OpFadeFog : public OpStandard
{
private:
  csRef<iSector> sector;
  csColor color;
  float density;
  csTicks duration;
  iEngineSequenceManager* eseqmgr;

public:
  OpFadeFog (iSector* sector, const csColor& color, float density,
  	csTicks duration, iEngineSequenceManager* eseqmgr)
  {
    OpFadeFog::sector = sector;
    OpFadeFog::color = color;
    OpFadeFog::density = density;
    OpFadeFog::duration = duration;
    OpFadeFog::eseqmgr = eseqmgr;
  }

  virtual void Do (csTicks dt)
  {
    csFog* fog = sector->GetFog ();
    csColor start_col (fog->red, fog->green, fog->blue);
    TimedOpFadeFog* timedop = new TimedOpFadeFog (
    	sector, start_col, fog->density, color, density);
    eseqmgr->FireTimedOperation (dt, duration, timedop);
    timedop->DecRef ();
  }
};

//---------------------------------------------------------------------------

/**
 * Set light operation.
 */
class OpSetLight : public OpStandard
{
private:
  csRef<iLight> light;
  csColor color;

public:
  OpSetLight (iLight* light, const csColor& color)
  {
    OpSetLight::light = light;
    OpSetLight::color = color;
  }

  virtual void Do (csTicks dt)
  {
    light->SetColor (color);
  }
};

/**
 * Fade light timed operation.
 */
class TimedOpFadeLight : public iSequenceTimedOperation
{
private:
  csRef<iLight> light;
  csColor start_col, end_col;

public:
  TimedOpFadeLight (iLight* light,
  	const csColor& start_col, const csColor& end_col)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    TimedOpFadeLight::light = light;
    TimedOpFadeLight::start_col = start_col;
    TimedOpFadeLight::end_col = end_col;
  }
  virtual ~TimedOpFadeLight () { }

  SCF_DECLARE_IBASE;

  virtual void Do (float time)
  {
    csColor color;
    color.red = (1-time) * start_col.red + time * end_col.red;
    color.green = (1-time) * start_col.green + time * end_col.green;
    color.blue = (1-time) * start_col.blue + time * end_col.blue;
    light->SetColor (color);
  }
};

SCF_IMPLEMENT_IBASE (TimedOpFadeLight)
  SCF_IMPLEMENTS_INTERFACE (iSequenceTimedOperation)
SCF_IMPLEMENT_IBASE_END

/**
 * Fade light operation.
 */
class OpFadeLight : public OpStandard
{
private:
  csRef<iLight> light;
  csColor color;
  csTicks duration;
  iEngineSequenceManager* eseqmgr;

public:
  OpFadeLight (iLight* light, const csColor& color,
  	csTicks duration, iEngineSequenceManager* eseqmgr)
  {
    OpFadeLight::light = light;
    OpFadeLight::color = color;
    OpFadeLight::duration = duration;
    OpFadeLight::eseqmgr = eseqmgr;
  }

  virtual void Do (csTicks dt)
  {
    const csColor& start_col = light->GetColor ();
    TimedOpFadeLight* timedop = new TimedOpFadeLight (
    	light, start_col, color);
    eseqmgr->FireTimedOperation (dt, duration, timedop);
    timedop->DecRef ();
  }
};

//---------------------------------------------------------------------------

/**
 * Set trigger state.
 */
class OpTriggerState : public OpStandard
{
private:
  csRef<iSequenceTrigger> trigger;
  bool en;

public:
  OpTriggerState (iSequenceTrigger* trigger, bool en)
  {
    OpTriggerState::trigger = trigger;
    OpTriggerState::en = en;
  }

  virtual void Do (csTicks dt)
  {
    trigger->SetEnabled (en);
  }
};

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT(csSequenceWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iSequenceWrapper)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSequenceWrapper::SequenceWrapper)
  SCF_IMPLEMENTS_INTERFACE(iSequenceWrapper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSequenceWrapper::csSequenceWrapper (csEngineSequenceManager* eseqmgr,
	iSequence* sequence)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSequenceWrapper);
  csSequenceWrapper::eseqmgr = eseqmgr;
  csSequenceWrapper::sequence = sequence;
}

csSequenceWrapper::~csSequenceWrapper ()
{
}

void csSequenceWrapper::AddOperationSetLight (csTicks time,
	iLight* light, const csColor& color)
{
  OpSetLight* op = new OpSetLight (light, color);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationFadeLight (csTicks time,
	iLight* light, const csColor& color, csTicks duration)
{
  OpFadeLight* op = new OpFadeLight (light, color, duration, eseqmgr);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationSetFog (csTicks time,
	iSector* sector, const csColor& color, float density)
{
  OpSetFog* op = new OpSetFog (sector, color, density);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationFadeFog (csTicks time,
	iSector* sector, const csColor& color, float density,
	csTicks duration)
{
  OpFadeFog* op = new OpFadeFog (sector, color, density, duration, eseqmgr);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

void csSequenceWrapper::AddOperationAbsoluteMove (csTicks time,
	iMeshWrapper* mesh, iSector* sector,
	const csReversibleTransform& trans)
{
}

void csSequenceWrapper::AddOperationAbsoluteMove (csTicks time,
	iMeshWrapper* mesh, iSector* sector, const csVector3& pos)
{
}

void csSequenceWrapper::AddOperationRelativeMove (csTicks time,
	iMeshWrapper* mesh, const csReversibleTransform& trans)
{
}

void csSequenceWrapper::AddOperationRelativeMove (csTicks time,
	iMeshWrapper* mesh, const csVector3& pos)
{
}

void csSequenceWrapper::AddOperationTriggerState (csTicks time,
	iSequenceTrigger* trigger, bool en)
{
  OpTriggerState* op = new OpTriggerState (trigger, en);
  sequence->AddOperation (time, op);
  op->DecRef ();
}

//---------------------------------------------------------------------------

/**
 * Callback that will activate trigger when sector is visible.
 */
class csTriggerSectorCallback : public iSectorCallback
{
private:
  csSequenceTrigger* trigger;
  uint32 framenr;

public:
  csTriggerSectorCallback (csSequenceTrigger* trigger)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    csTriggerSectorCallback::trigger = trigger;
    framenr = 0;
  }
  virtual ~csTriggerSectorCallback () { }

  SCF_DECLARE_IBASE;

  virtual void Traverse (iSector* sector, iBase* context)
  {
    csRef<iRenderView> rview (SCF_QUERY_INTERFACE (context, iRenderView));
    if (rview)
    {
      uint32 global_framenr = trigger->GetEngineSequenceManager ()
      	->GetGlobalFrameNr ();
      if (framenr != global_framenr)
      {
        framenr = global_framenr;
	trigger->Fire ();
      }
    }
  }
};

SCF_IMPLEMENT_IBASE (csTriggerSectorCallback)
  SCF_IMPLEMENTS_INTERFACE (iSectorCallback)
SCF_IMPLEMENT_IBASE_END

//---------------------------------------------------------------------------

/**
 * Cleanup a sector callback.
 */
class csConditionCleanupSectorCB : public csConditionCleanup
{
private:
  csRef<iSector> sector;
  csRef<iSectorCallback> cb;

public:
  csConditionCleanupSectorCB (iSector* sect, iSectorCallback* cb)
  {
    sector = sect;
    csConditionCleanupSectorCB::cb = cb;
  }
  virtual void Cleanup ()
  {
    if (sector && cb)
    {
      sector->RemoveSectorCallback (cb);
    }
  }
};

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT (csSequenceTrigger)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSequenceTrigger)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSequenceTrigger::SequenceTrigger)
  SCF_IMPLEMENTS_INTERFACE (iSequenceTrigger)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSequenceTrigger::csSequenceTrigger (csEngineSequenceManager* eseqmgr)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSequenceTrigger);
  enabled = true;
  fire_delay = 0;
  csSequenceTrigger::eseqmgr = eseqmgr;
  framenr = 0;
  total_conditions = 0;
}

csSequenceTrigger::~csSequenceTrigger ()
{
  ClearConditions ();
}

void csSequenceTrigger::AddConditionInSector (iSector* sector)
{
}

void csSequenceTrigger::AddConditionSectorVisible (iSector* sector)
{
  csTriggerSectorCallback* trig = new csTriggerSectorCallback (this);
  sector->SetSectorCallback (trig);

  csConditionCleanupSectorCB* cleanup = new csConditionCleanupSectorCB (
  	sector, trig);
  condition_cleanups.Push (cleanup);
 
  cleanup->DecRef ();
  trig->DecRef ();

  total_conditions++;
}

void csSequenceTrigger::AddConditionInMeshSphere (iMeshWrapper* mesh)
{
}

void csSequenceTrigger::AddConditionInMeshBox (iMeshWrapper* mesh)
{
}

void csSequenceTrigger::AddConditionMeshVisible (iMeshWrapper* mesh)
{
}

void csSequenceTrigger::AddConditionManual ()
{
}

void csSequenceTrigger::ClearConditions ()
{
  total_conditions = 0;
  fired_conditions = 0;
  framenr = 0;
  condition_cleanups.DeleteAll ();
}

void csSequenceTrigger::Trigger ()
{
}

void csSequenceTrigger::FireSequence (csTicks delay, iSequenceWrapper* seq)
{
  fire_sequence = seq;
  fire_delay = delay;
}

void csSequenceTrigger::Fire ()
{
  if (!enabled) return;

  uint32 global_framenr = eseqmgr->GetGlobalFrameNr ();
  if (framenr != global_framenr)
  {
    framenr = global_framenr;
    fired_conditions = 0;
  }
  fired_conditions++;
  if (fired_conditions >= total_conditions)
  {
    eseqmgr->GetSequenceManager ()->RunSequence (fire_delay,
    	fire_sequence->GetSequence ());
    fired_conditions = 0;
    enabled = false;
  }
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY (csEngineSequenceManager)

SCF_EXPORT_CLASS_TABLE (engseq)
  SCF_EXPORT_CLASS (csEngineSequenceManager,
  	"crystalspace.utilities.sequence.engine", "Engine Sequence Manager")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE (csEngineSequenceManager)
  SCF_IMPLEMENTS_INTERFACE (iEngineSequenceManager)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csEngineSequenceManager::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csEngineSequenceManager::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

csEngineSequenceManager::csEngineSequenceManager (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  scfiEventHandler = NULL;
  object_reg = NULL;
  global_framenr = 1;
}

csEngineSequenceManager::~csEngineSequenceManager ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
    if (q != 0)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }
}

bool csEngineSequenceManager::Initialize (iObjectRegistry *r)
{
  object_reg = r;
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Nothing);

  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  seqmgr = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.utilities.sequence",
  	iSequenceManager);
  if (!seqmgr)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.utilities.sequence.engine",
	"Couldn't load sequence manager plugin!");
    return false;
  }
  seqmgr->Resume ();

  return true;
}

bool csEngineSequenceManager::HandleEvent (iEvent &event)
{
  if (event.Type == csevBroadcast
	&& event.Command.Code == cscmdPreProcess)
  {
    global_framenr++;

    csTicks curtime = seqmgr->GetMainTime ();
    int i = timed_operations.Length ()-1;
    while (i >= 0)
    {
      csTimedOperation* op = timed_operations[i];
      if (curtime >= op->end)
      {
        timed_operations.Delete (i);
      }
      else
      {
	float time = float (curtime-op->start) / float (op->end-op->start);
        op->op->Do (time);
      }
      i--;
    }

    return true;
  }
  return false;
}

csPtr<iSequenceTrigger> csEngineSequenceManager::CreateTrigger (
	const char* name)
{
  csSequenceTrigger* trig = new csSequenceTrigger (this);
  trig->SetName (name);
  triggers.Push (&(trig->scfiSequenceTrigger));
  return &(trig->scfiSequenceTrigger);
}

void csEngineSequenceManager::RemoveTrigger (iSequenceTrigger* trigger)
{
  triggers.Delete (trigger);
}

void csEngineSequenceManager::RemoveTriggers ()
{
  triggers.DeleteAll ();
}

int csEngineSequenceManager::GetTriggerCount () const
{
  return triggers.Length ();
}

iSequenceTrigger* csEngineSequenceManager::GetTrigger (int idx) const
{
  return triggers[idx];
}

iSequenceTrigger* csEngineSequenceManager::FindTriggerByName (
	const char* name) const
{
  int i;
  for (i = 0 ; i < triggers.Length () ; i++)
  {
    if (!strcmp (name, triggers[i]->QueryObject ()->GetName ()))
      return triggers[i];
  }
  return NULL;
}

csPtr<iSequenceWrapper> csEngineSequenceManager::CreateSequence (
	const char* name)
{
  csRef<iSequence> seq (csPtr<iSequence> (seqmgr->NewSequence ()));
  csSequenceWrapper* seqwrap = new csSequenceWrapper (this, seq);
  seqwrap->SetName (name);
  sequences.Push (&(seqwrap->scfiSequenceWrapper));
  return &(seqwrap->scfiSequenceWrapper);
}

void csEngineSequenceManager::RemoveSequence (iSequenceWrapper* seq)
{
  sequences.Delete (seq);
}

void csEngineSequenceManager::RemoveSequences ()
{
  sequences.DeleteAll ();
}

int csEngineSequenceManager::GetSequenceCount () const
{
  return sequences.Length ();
}

iSequenceWrapper* csEngineSequenceManager::GetSequence (int idx) const
{
  return sequences[idx];
}

iSequenceWrapper* csEngineSequenceManager::FindSequenceByName (
	const char* name) const
{
  int i;
  for (i = 0 ; i < sequences.Length () ; i++)
  {
    if (!strcmp (name, sequences[i]->QueryObject ()->GetName ()))
      return sequences[i];
  }
  return NULL;
}

void csEngineSequenceManager::FireTimedOperation (csTicks delta,
	csTicks duration, iSequenceTimedOperation* op)
{
  csTicks curtime = seqmgr->GetMainTime ();
  if (delta >= duration) return;	// Already done.

  csTimedOperation* top = new csTimedOperation (op);
  top->start = curtime-delta;
  top->end = top->start + duration;

  timed_operations.Push (top);
  top->DecRef ();
}


