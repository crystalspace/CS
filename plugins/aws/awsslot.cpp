#include "cssysdef.h"
#include "awsslot.h"
#include "awsadler.h"
#include <string.h>

#define callRefMemberFunction(object, ptrToMember)  ((object).*(ptrToMember))
#define callPtrMemberFunction(object, ptrToMember)  ((object)->*(ptrToMember))

static unsigned long NameToId (char *n)
{
  if (n)
  {
    unsigned long id = aws_adler32 (
        aws_adler32 (0, NULL, 0),
        (unsigned char *)n,
        strlen (n));

    return id;
  }
  else
    return 0;
}

///////////////////////////////////// Signal Sink Manager ////////////////////////////////////////////////////
awsSinkManager::awsSinkManager (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

awsSinkManager::~awsSinkManager ()
{
  int i;

  for (i = 0; i < sinks.Length (); ++i)
  {
    SinkMap *sm = (SinkMap *)sinks[i];

    sm->sink->DecRef ();
    delete sm;
  }

  return ;
}

bool awsSinkManager::Initialize (iObjectRegistry *)
{
  return true;
}

void awsSinkManager::RegisterSink (char *name, iAwsSink *sink)
{
  sink->IncRef ();
  sinks.Push (new SinkMap (NameToId (name), sink));
}

iAwsSink *awsSinkManager::FindSink (char *_name)
{
  int i;
  unsigned long name = NameToId (_name);

  for (i = 0; i < sinks.Length (); ++i)
  {
    SinkMap *sm = (SinkMap *)sinks[i];

    if (sm->name == name) return sm->sink;
  }

  return NULL;
}

iAwsSink *awsSinkManager::CreateSink (void *parm)
{
  return new awsSink (parm);
}

///////////////////////////////////// Signal Sinks //////////////////////////////////////////////////////////
awsSink::awsSink (void *p) :
  parm(p)
{
  SCF_CONSTRUCT_IBASE (NULL);
}

awsSink::~awsSink ()
{
}

unsigned long awsSink::GetTriggerID (char *_name)
{
  unsigned long name = NameToId (_name);
  int i;

  for (i = 0; i < triggers.Length (); ++i)
  {
    TriggerMap *tm = (TriggerMap *)triggers[i];

    if (tm->name == name) return i;
  }

  return 0;
}

void awsSink::HandleTrigger (int trigger, iAwsSource *source)
{
  if (triggers.Length () == 0) return ;

  void (*Trigger) (void *, iAwsSource *) =
    (((TriggerMap *) (triggers[trigger]))->trigger);
  (Trigger) (parm, source);
}

void awsSink::RegisterTrigger (
  char *name,
  void (*Trigger) (void *, iAwsSource *))
{
  triggers.Push (new TriggerMap (NameToId (name), Trigger));
}

///////////////////////////////////// Signal Sources ////////////////////////////////////////////////////////
awsSource::awsSource (iAwsComponent *_owner) :
  owner(_owner)
{
  SCF_CONSTRUCT_IBASE (NULL);
}

awsSource::~awsSource ()
{
  int i;
  for (i = 0; i < slots.Length (); ++i)
  {
    void *p = slots[i];
    delete (SlotSignalMap *)p;
  }

  slots.SetLength (0);
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

  slot->IncRef ();
  slots.Push (ssm);

  return true;
}

bool awsSource::UnregisterSlot (iAwsSlot *slot, unsigned long signal)
{
  void *entry;
  int i;

  for (i = 0; i < slots.Length (); ++i)
  {
    entry = slots[i];

    SlotSignalMap *ssm = STATIC_CAST (SlotSignalMap *, entry);

    if (ssm->signal == signal && ssm->slot == slot)
    {
      slot->DecRef ();
      slots.Delete (i);
      delete ssm;

      return true;
    }
  }

  return false;
}

void awsSource::Broadcast (unsigned long signal)
{
  int i;

  for (i = 0; i < slots.Length (); ++i)
  {
    SlotSignalMap *ssm = STATIC_CAST (SlotSignalMap *, slots[i]);

    if (ssm->signal == signal) ssm->slot->Emit (*this, signal);
  }
}

///////////////////////////////////// Slots ////////////////////////////////////////////////////////
awsSlot::awsSlot ()
{
  SCF_CONSTRUCT_IBASE (NULL);
}

awsSlot::~awsSlot ()
{
}

void awsSlot::Connect (
  iAwsSource *source,
  unsigned long signal,
  iAwsSink *sink,
  unsigned long trigger)
{
  source->RegisterSlot (this, signal);

  int i;

  for (i = 0; i < stmap.Length (); ++i)
  {
    SignalTriggerMap *stm = (SignalTriggerMap *)stmap[i];

    if (stm->signal == signal && stm->trigger == trigger && stm->sink == sink)
    {
      stm->refs++;
      stm->sink->IncRef ();
      return ;
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

  int i;

  for (i = 0; i < stmap.Length (); ++i)
  {
    SignalTriggerMap *stm = (SignalTriggerMap *)stmap[i];

    if (stm->signal == signal && stm->trigger == trigger && stm->sink == sink)
    {
      stm->refs--;
      stm->sink->DecRef ();

      if (stm->refs == 0) stmap.Delete (i);

      return ;
    }
  }
}

void awsSlot::Emit (iAwsSource &source, unsigned long signal)
{
  int i;

  for (i = 0; i < stmap.Length (); ++i)
  {
    SignalTriggerMap *stm = (SignalTriggerMap *)stmap[i];

    if (stm->signal == signal)
      stm->sink->HandleTrigger (stm->trigger, &source);
  }
}
