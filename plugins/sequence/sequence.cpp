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

#include "cssysdef.h"
#include <string.h>
#include "csutil/sysfunc.h"
#include "csutil/event.h"
#include "sequence.h"
#include "csutil/scf.h"
#include "iutil/objreg.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/virtclk.h"
#include "iutil/vfs.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csSequence)
  SCF_IMPLEMENTS_INTERFACE (iSequence)
SCF_IMPLEMENT_IBASE_END

csSequence::csSequence (iSequenceManager* seqmgr) : first (0), last (0)
{
  SCF_CONSTRUCT_IBASE (0);
  csSequence::seqmgr = seqmgr;
}

csSequence::~csSequence ()
{
  Clear ();
  SCF_DESTRUCT_IBASE();
}

void csSequence::Clear ()
{
  while (first)
  {
    csSequenceOp* n = first->next;
    delete first;
    first = n;
  }
  last = 0;
}

void csSequence::CleanupSequences ()
{
  csSequenceOp* n = first;
  while (n)
  {
    if (n->operation) n->operation->CleanupSequences ();
    n = n->next;
  }
}

void csSequence::DeleteFirstSequence ()
{
  if (first)
  {
    csSequenceOp* n = first->next;
    delete first;
    first = n;
    if (!first) last = 0;
    else first->prev = 0;
  }
}

void csSequence::AddOperation (csTicks time, iSequenceOperation* operation,
	iBase* params)
{
  csSequenceOp* op = new csSequenceOp ();
  op->time = time;
  op->operation = operation;
  op->params = params;
  // Insert this operation at the right time.
  if (first)
  {
    csSequenceOp* o = first;
    while (o)
    {
      if (time <= o->time)
      {
        op->next = o;
	op->prev = o->prev;
	if (o->prev) o->prev->next = op;
	else first = op;
	o->prev = op;
	break;
      }
      o = o->next;
    }
    if (!o)
    {
      // Put it last.
      op->next = 0;
      op->prev = last;
      last->next = op;
      last = op;
    }
  }
  else
  {
    // The very first operation.
    first = last = op;
    op->prev = op->next = 0;
  }
}

void csSequence::AddRunSequence (csTicks time, iSequence* sequence,
	iBase* params)
{
  RunSequenceOp* op = new RunSequenceOp (seqmgr, sequence);
  AddOperation (time, op, params);
  op->DecRef ();
}

void csSequence::RunSequenceOp::Do (csTicks dt, iBase* params)
{
  if (sequence)
    seqmgr->RunSequence (-(signed)dt, sequence, params);
}

void csSequence::AddCondition (csTicks time, iSequenceCondition* condition,
  	iSequence* trueSequence, iSequence* falseSequence,
	iBase* params)
{
  RunCondition* op = new RunCondition (seqmgr, condition, trueSequence,
  	falseSequence);
  AddOperation (time, op, params);
  op->DecRef ();
}

void csSequence::RunCondition::Do (csTicks dt, iBase* params)
{
  if (condition->Condition (dt, params))
  {
    if (trueSequence)
      seqmgr->RunSequence (-(signed)dt, trueSequence, params);
  }
  else
  {
    if (falseSequence)
      seqmgr->RunSequence (-(signed)dt, falseSequence, params);
  }
}

void csSequence::AddLoop (csTicks time, iSequenceCondition* condition,
  	iSequence* sequence, iBase* params)
{
  RunLoop* op = new RunLoop (seqmgr, condition, sequence);
  AddOperation (time, op, params);
  op->DecRef ();
}

void csSequence::RunLoop::Do (csTicks dt, iBase* params)
{
  while (condition->Condition (dt, params))
    seqmgr->RunSequence (-(signed)dt, sequence, params);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY (csSequenceManager)


SCF_IMPLEMENT_IBASE (csSequenceManager)
  SCF_IMPLEMENTS_INTERFACE (iSequenceManager)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSequenceManager::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSequenceManager::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

csSequenceManager::csSequenceManager (iBase *iParent) :
	weakref_alloc (100)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  scfiEventHandler = 0;
  object_reg = 0;
  main_sequence = new csSequence (this);
  previous_time_valid = false;
  main_time = 0;
  suspended = true;
}

csSequenceManager::~csSequenceManager ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
    if (q != 0)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }
  Clear ();
  main_sequence->DecRef ();
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csSequenceManager::Initialize (iObjectRegistry *r)
{
  object_reg = r;
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Nothing);
  return true;
}

bool csSequenceManager::HandleEvent (iEvent &event)
{
  // Sequence manager must be final because engine sequence manager
  // must come first. @@@ HACKY
  if (event.Type != csevBroadcast
      || csCommandEventHelper::GetCode(&event) != cscmdFinalProcess)
    return false;

  if (!suspended)
  {
    csTicks current_time = vc->GetCurrentTicks ();
    if (!previous_time_valid)
    {
      previous_time = current_time;
      previous_time_valid = true;
    }
    TimeWarp (current_time-previous_time, false);
    previous_time = current_time;
  }
  return true;
}

csTicks csSequenceManager::GetDeltaTime () const
{
  if (suspended) return 0;
  if (!previous_time_valid)
    return 0;
  else
    return vc->GetCurrentTicks () - previous_time;
}

void csSequenceManager::Clear ()
{
  main_sequence->Clear ();
  main_time = 0;
  previous_time_valid = false;
  size_t i;
  for (i = 0 ; i < sequences.Length () ; i++)
  {
    csWeakRef<csSequence>* seq = sequences[i];
    if (seq->IsValid())
    {
      // We keep a real ref to the sequence to prevent the
      // sequence deleting itself.
      csRef<csSequence> keepref = (csSequence*)*seq;
      ((csSequence*)*seq)->CleanupSequences ();
    }
    weakref_alloc.Free (seq);
  }
  sequences.DeleteAll ();
}

void csSequenceManager::Suspend ()
{
  suspended = true;
}

void csSequenceManager::Resume ()
{
  if (suspended)
  {
    suspended = false;
    previous_time_valid = false;
  }
}

void csSequenceManager::TimeWarp (csTicks time, bool skip)
{
  main_time += time;
  csSequenceOp* seqOp = main_sequence->GetFirstSequence ();
  while (seqOp && seqOp->time <= main_time)
  {
    // Because an operation can itself modify the main sequence
    // queue we take care to first unlink this sequence operation
    // before performing it.
    csRef<iSequenceOperation> op = seqOp->operation;
    csRef<iBase> params = seqOp->params;
    csTicks opt = seqOp->time;
    main_sequence->DeleteFirstSequence ();

    if (!skip)
    {
      op->Do (main_time - opt, params);
    }

    // Now really delete the operation.
    op = 0;
    params = 0;

    // And fetch the next one.
    seqOp = main_sequence->GetFirstSequence ();
  }
}

csPtr<iSequence> csSequenceManager::NewSequence ()
{
  static int cnt = 0;
  csSequence* n = new csSequence (this);
  csWeakRef<csSequence>* weakn = weakref_alloc.Alloc ();
  *weakn = n;
  sequences.Push (weakn);
  cnt++;
  if (cnt >= 100)
  {
    cnt = 0;
    if (sequences.Length () > 100)
    {
      size_t i;
      csArray<csWeakRef<csSequence>* > copy;
      for (i = 0 ; i < sequences.Length () ; i++)
      {
        csWeakRef<csSequence>* seq = sequences[i];
	if ((*seq) != 0) copy.Push (seq);
	else weakref_alloc.Free (seq);
      }
      sequences = copy;
    }
  }
  return csPtr<iSequence> (n);
}

void csSequenceManager::RunSequence (csTicks time, iSequence* sequence,
	iBase* params)
{
  csSequence* seq = (csSequence*)sequence;
  csSequenceOp* op = seq->GetFirstSequence ();
  while (op)
  {
    main_sequence->AddOperation (main_time + time + op->time, op->operation,
    	params ? params : (iBase*)op->params);
    op = op->next;
  }
}

SCF_IMPLEMENT_IBASE (csSequence::StandardOperation)
  SCF_IMPLEMENTS_INTERFACE (iSequenceOperation)
SCF_IMPLEMENT_IBASE_END

