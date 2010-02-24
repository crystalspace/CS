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
#include "csutil/eventhandlers.h"
#include "sequence.h"
#include "iutil/objreg.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/virtclk.h"
#include "iutil/vfs.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"



csSequence::csSequence (iSequenceManager* seqmgr) :
  scfImplementationType(this), first (0), last (0)
{
  csSequence::seqmgr = seqmgr;
}

csSequence::~csSequence ()
{
  Clear ();
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
	iBase* params, uint sequence_id)
{
  csSequenceOp* op = new csSequenceOp ();
  op->time = time;
  op->operation = operation;
  op->params = params;
  op->sequence_id = sequence_id;
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
	iBase* params, uint sequence_id)
{
  RunSequenceOp* op = new RunSequenceOp (seqmgr, sequence);
  AddOperation (time, op, params, sequence_id);
  op->DecRef ();
}

void csSequence::RunSequenceOp::Do (csTicks dt, iBase* params)
{
  if (sequence)
    seqmgr->RunSequence (-(signed)dt, sequence, params);
}

void csSequence::AddCondition (csTicks time, iSequenceCondition* condition,
  	iSequence* trueSequence, iSequence* falseSequence,
	iBase* params, uint sequence_id)
{
  RunCondition* op = new RunCondition (seqmgr, condition, trueSequence,
  	falseSequence);
  AddOperation (time, op, params, sequence_id);
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
  	iSequence* sequence, iBase* params, uint sequence_id)
{
  RunLoop* op = new RunLoop (seqmgr, condition, sequence);
  AddOperation (time, op, params, sequence_id);
  op->DecRef ();
}

void csSequence::RunLoop::Do (csTicks dt, iBase* params)
{
  while (sequence && condition->Condition (dt, params))
    seqmgr->RunSequence (-(signed)dt, sequence, params);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY (csSequenceManager)

csSequenceManager::csSequenceManager (iBase *iParent) :
  scfImplementationType(this, iParent), weakref_alloc (100)
{
  object_reg = 0;
  main_sequence = new csSequence (this);
  previous_time_valid = false;
  main_time = 0;
  suspended = true;
  sequence_id = 0;
}

csSequenceManager::~csSequenceManager ()
{
  if (object_reg)
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
    if (q != 0)
      CS::RemoveWeakListener (q, weakEventHandler);
  }
  Clear ();
  main_sequence->DecRef ();
}

bool csSequenceManager::Initialize (iObjectRegistry *r)
{
  object_reg = r;
  vc = csQueryRegistry<iVirtualClock> (object_reg);
  CS_INITIALIZE_EVENT_SHORTCUTS (object_reg);
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
  if (q != 0)
    CS::RegisterWeakListener (q, this, Frame, weakEventHandler);
  return true;
}

bool csSequenceManager::HandleEvent (iEvent &event)
{
  // Sequence manager must be final because engine sequence manager
  // must come first. @@@ HACKY
  if (event.Name != Frame)
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
  for (i = 0 ; i < sequences.GetSize () ; i++)
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
    if (sequences.GetSize () > 100)
    {
      size_t i;
      csArray<csWeakRef<csSequence>* > copy;
      for (i = 0 ; i < sequences.GetSize () ; i++)
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
	iBase* params, uint sequence_id)
{
  csSequence* seq = (csSequence*)sequence;
  csSequenceOp* op = seq->GetFirstSequence ();
  while (op)
  {
    main_sequence->AddOperation (main_time + time + op->time, op->operation,
    	params ? params : (iBase*)op->params, sequence_id);
    op = op->next;
  }
}

void csSequenceManager::DestroySequenceOperations (uint sequence_id)
{
  csSequenceOp* op = main_sequence->GetFirstSequence ();
  while (op)
  {
    csSequenceOp* op_next = op->next;
    if (op->sequence_id == sequence_id)
    {
      if (op->next) op->next->prev = op->prev;
      if (op->prev) op->prev->next = op->next;
      else main_sequence->SetFirstSequence (op->next);
      delete op;
    }
    op = op_next;
  }
}
