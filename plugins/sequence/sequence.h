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

#ifndef __CS_SEQUENCE_H__
#define __CS_SEQUENCE_H__

#include "csutil/util.h"
#include "csutil/csvector.h"
#include "ivaria/sequence.h"
#include "isys/plugin.h"

struct iSystem;

struct csSequenceOp
{
  csSequenceOp* next, * prev;
  csTime time;
  iSequenceOperation* operation;

  csSequenceOp () : operation (NULL) { }
  ~csSequenceOp () { if (operation) operation->DecRef (); }
};

class csSequence : public iSequence
{
private:
  iSequenceManager* seqmgr;
  // A double linked list of all sequence operations.
  csSequenceOp* first;
  csSequenceOp* last;

public:
  //=====
  // Standard operation.
  //=====
  class StandardOperation : public iSequenceOperation
  {
  private:
    int ref_count;
  protected:
    iSequenceManager* seqmgr;
    virtual ~StandardOperation () { }
  public:
    StandardOperation (iSequenceManager* sm) : ref_count (1), seqmgr (sm) { }
    virtual void IncRef () { ref_count++; }
    virtual void DecRef () { if (--ref_count <= 0) delete this; }
  };

  //=====
  // An operation to run a sequence.
  //=====
  class RunSequenceOp : public StandardOperation
  {
  private:
    iSequence* sequence;
  protected:
    virtual ~RunSequenceOp () { sequence->DecRef (); }
  public:
    RunSequenceOp (iSequenceManager* sm, iSequence* seq) :
    	StandardOperation (sm), sequence (seq)
    {
      seq->IncRef ();
    }
    virtual void Do (csTime dt);
  };

  //=====
  // An operation for a condition.
  //=====
  class RunCondition : public StandardOperation
  {
  private:
    iSequenceCondition* condition;
    iSequence* trueSequence;
    iSequence* falseSequence;
  protected:
    virtual ~RunCondition ()
    {
      trueSequence->DecRef ();
      falseSequence->DecRef ();
      condition->DecRef ();
    }
  public:
    RunCondition (iSequenceManager* sm, iSequenceCondition* cond,
    	iSequence* trueSeq, iSequence* falseSeq) :
	StandardOperation (sm), condition (cond), trueSequence (trueSeq),
	falseSequence (falseSeq)
    {
      trueSeq->IncRef ();
      falseSeq->IncRef ();
      cond->IncRef ();
    }
    virtual void Do (csTime dt);
  };

  //=====
  // An operation for a loop.
  //=====
  class RunLoop : public StandardOperation
  {
  private:
    iSequenceCondition* condition;
    iSequence* sequence;
  protected:
    virtual ~RunLoop ()
    {
      sequence->DecRef ();
      condition->DecRef ();
    }
  public:
    RunLoop (iSequenceManager* sm, iSequenceCondition* cond, iSequence* seq) :
    	StandardOperation (sm), condition (cond), sequence (seq)
    {
      seq->IncRef ();
      cond->IncRef ();
    }
    virtual void Do (csTime dt);
  };

public:
  SCF_DECLARE_IBASE;

  csSequence (iSequenceManager* seqmgr);
  virtual ~csSequence ();

  csSequenceOp* GetFirstSequence () { return first; }
  void DeleteFirstSequence ();

  virtual void AddOperation (csTime time, iSequenceOperation* operation);
  virtual void AddRunSequence (csTime time, iSequence* sequence);
  virtual void AddCondition (csTime time, iSequenceCondition* condition,
  	iSequence* trueSequence, iSequence* falseSequence);
  virtual void AddLoop (csTime time, iSequenceCondition* condition,
  	iSequence* sequence);
  virtual void Clear ();
  virtual bool IsEmpty () { return first == NULL; }
};

class csSequenceManager : public iSequenceManager
{
private:
  iSystem *System;

  // The sequence manager uses one big sequence to keep all queued
  // sequence operations. New sequences will be merged with this one.
  csSequence* main_sequence;

  // The previous time.
  csTime previous_time;
  bool previous_time_valid;

  // This is the current time for the sequence manager.
  // This is the main ticker that keeps the entire system running.
  csTime main_time;

  // If true the sequence manager is suspended
  bool suspended;

public:
  SCF_DECLARE_IBASE;

  csSequenceManager (iBase *iParent);
  virtual ~csSequenceManager ();
  virtual bool Initialize (iSystem *system);

  /// This is set to receive the once per frame nothing event.
  virtual bool HandleEvent (iEvent &event);

  virtual void Clear ();
  virtual bool IsEmpty () { return main_sequence->IsEmpty (); }
  virtual void Suspend ();
  virtual bool IsSuspended () { return suspended; }
  virtual void Resume ();
  virtual void TimeWarp (csTime time, bool skip);
  virtual csTime GetMainTime () { return main_time; }
  virtual iSequence* NewSequence ();
  virtual void RunSequence (csTime time, iSequence* sequence);

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSequenceManager);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent& e) { return scfParent->HandleEvent(e); }
  } scfiPlugin;
};

#endif // __CS_SEQUENCE_H__

