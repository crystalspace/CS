/*
    Copyright (C) 2001 by Christopher Nelson

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
#include "awsslot.h"
#include "iaws/awsdefs.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include <string.h>

#define callRefMemberFunction(object, ptrToMember)  ((object).*(ptrToMember))
#define callPtrMemberFunction(object, ptrToMember)  ((object)->*(ptrToMember))

////////////////////////// Signal Sink Manager ////////////////////////////////

awsSinkManager::awsSinkManager (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
}

awsSinkManager::~awsSinkManager ()
{
  SCF_DESTRUCT_IBASE ();
}

bool awsSinkManager::Setup (iObjectRegistry* r)
{
  strset = CS_QUERY_REGISTRY_TAG_INTERFACE(r, "crystalspace.shared.stringset",
    iStringSet);
  if (!strset.IsValid())
  {
    csReport (r, CS_REPORTER_SEVERITY_ERROR, "crystalspace.aws",
      "AWS sink manager could not locate the global shared string set \""
      "crystalspace.shared.stringset\". This is a serious error.");
    return false;
  }
  return true;
}

unsigned long awsSinkManager::NameToId (const char *n) const
{
  if (n)
    return strset->Request(n);
  else
    return csInvalidStringID;
}

void awsSinkManager::RegisterSink (const char *name, iAwsSink *sink)
{
  sinks.Push (new SinkMap (NameToId (name), sink));
}

bool awsSinkManager::RemoveSink (iAwsSink* sink)
{
  for (size_t i = 0; i < sinks.Length (); ++i)
  {
    SinkMap *sm = sinks[i];
    if (sm->sink == sink)
    {
      sinks.DeleteIndex (i);
      return true;
    }
  }
  return false;
}

iAwsSink *awsSinkManager::FindSink (const char *_name)
{
  unsigned long name = NameToId (_name);
  for (size_t i = 0; i < sinks.Length (); ++i)
  {
    SinkMap *sm = sinks[i];
    if (sm->name == name)
      return sm->sink;
  }
  return 0;
}

iAwsSink *awsSinkManager::CreateSink (intptr_t parm)
{
  awsSink* sink = new awsSink (strset);
  sink->SetParm (parm);
  return sink;
}

iAwsSlot *awsSinkManager::CreateSlot ()
{
  return new awsSlot ();
}

///////////////////////// Signal Sinks ////////////////////////////////////////

awsSink::awsSink (iAws* a) : parm(0), sink_err(0), strset(a->GetStringTable())
{
  SCF_CONSTRUCT_IBASE (0);
}

awsSink::awsSink (iStringSet* s) : parm(0), sink_err(0), strset(s)
{
  SCF_CONSTRUCT_IBASE (0);
}

awsSink::~awsSink ()
{
  SCF_DESTRUCT_IBASE();
}

unsigned long awsSink::NameToId (const char *n) const
{
  if (n)
    return strset->Request(n);
  else
    return csInvalidStringID;
}

unsigned long awsSink::GetTriggerID (const char *_name)
{
  unsigned long name = NameToId (_name);
  sink_err=0;

  for (size_t i = 0; i < triggers.Length (); ++i)
  {
    TriggerMap *tm = triggers[i];
    if (tm->name == name)
      return (unsigned long)i;
  }

  sink_err = AWS_ERR_SINK_TRIGGER_NOT_FOUND;
  return 0;
}

void awsSink::HandleTrigger (int trigger, iAwsSource *source)
{
  sink_err = 0;

  if (triggers.Length () == 0) 
  {
    sink_err = AWS_ERR_SINK_NO_TRIGGERS;
    return ;
  }

  void (*Trigger) (intptr_t, iAwsSource *) = triggers[trigger]->trigger;
  (Trigger) (parm, source);
}

void awsSink::RegisterTrigger (const char *name,
  void (*Trigger) (intptr_t, iAwsSource *))
{
  sink_err = 0;
  triggers.Push (new TriggerMap (NameToId (name), Trigger));
}

////////////////////////// Signal Sources /////////////////////////////////////

awsSource::awsSource () : owner(0)
{
  SCF_CONSTRUCT_IBASE (0);
}

awsSource::~awsSource ()
{
  SCF_DESTRUCT_IBASE();
}

iAwsComponent *awsSource::GetComponent ()
{
  return owner;
}

bool awsSource::RegisterSlot (iAwsSlot *slot, unsigned long signal)
{
  SlotSignalMap *ssm = new SlotSignalMap;
  ssm->slot = slot;
  ssm->signal = signal;
  slots.Push (ssm);
  return true;
}

bool awsSource::UnregisterSlot (iAwsSlot *slot, unsigned long signal)
{
  for (size_t i = 0; i < slots.Length (); ++i)
  {
    SlotSignalMap *ssm = slots[i];
    if (ssm->signal == signal && ssm->slot == slot)
    {
      slots.DeleteIndex (i);
      return true;
    }
  }
  return false;
}

void awsSource::Broadcast (unsigned long signal)
{
  for (size_t i = 0; i < slots.Length (); ++i)
  {
    SlotSignalMap *ssm = slots[i];
    if (ssm->signal == signal)
	ssm->slot->Emit (*this, signal);
  }
}

/////////////////////////////// Slots /////////////////////////////////////////

awsSlot::awsSlot ()
{
  SCF_CONSTRUCT_IBASE (0);
}

awsSlot::~awsSlot ()
{
  SCF_DESTRUCT_IBASE();
}

void awsSlot::Connect (
  iAwsSource *source,
  unsigned long signal,
  iAwsSink *sink,
  unsigned long trigger)
{
  source->RegisterSlot (this, signal);

  for (size_t i = 0; i < stmap.Length (); ++i)
  {
    SignalTriggerMap *stm = stmap[i];
    if (stm->signal == signal && stm->trigger == trigger && stm->sink == sink)
    {
      stm->refs++;
      return;
    }
  }

  stmap.Push (new SignalTriggerMap (signal, sink, trigger, 1));
}

void awsSlot::Disconnect (
  iAwsSource *source,
  unsigned long signal,
  iAwsSink *sink,
  unsigned long trigger)
{
  source->UnregisterSlot (this, signal);

  for (size_t i = 0; i < stmap.Length (); ++i)
  {
    SignalTriggerMap *stm = stmap[i];
    if (stm->signal == signal && stm->trigger == trigger && stm->sink == sink)
    {
      stm->refs--;
      if (stm->refs == 0)
	stmap.DeleteIndex (i);
      return;
    }
  }
}

void awsSlot::Emit (iAwsSource &source, unsigned long signal)
{
  for (size_t i = 0; i < stmap.Length (); ++i)
  {
    SignalTriggerMap *stm = stmap[i];
    if (stm->signal == signal)
      stm->sink->HandleTrigger (stm->trigger, &source);
  }
}
