#include "cssysdef.h"
#include "aws/awsslot.h"
#include "aws/awsadler.h"
#include <string.h>

static unsigned long 
NameToId(char *n)
{
 if (n) {
    unsigned long id = aws_adler32(aws_adler32(0, NULL, 0), (unsigned char *)n, strlen(n));
           
    return id;
 }
 else 
    return 0;
}


///////////////////////////////////// Signal Sink Manager ////////////////////////////////////////////////////
awsSinkManager::awsSinkManager(iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

}
 
awsSinkManager::~awsSinkManager()
{
  int i;
  
  for(i=0; i<sinks.Length(); ++i)
  {
    SinkMap *sm = (SinkMap *)sinks[i];

    sm->sink->DecRef();
    delete sm;
  }
  
  return;
}
    
bool 
awsSinkManager::Initialize(iObjectRegistry *sys)
{
  return true;
}

void 
awsSinkManager::RegisterSink(char *name, iAwsSink *sink)
{
  sink->IncRef();
  sinks.Push(new SinkMap(NameToId(name), sink));
}

iAwsSink* 
awsSinkManager::FindSink(char *_name)
{
  int i;
  unsigned long name = NameToId(_name);

  for(i=0; i<sinks.Length(); ++i)
  {
    SinkMap *sm = (SinkMap *)sinks[i];

    if (sm->name == name)
      return sm->sink;    
  }
  return NULL;
}


///////////////////////////////////// Signal Sinks //////////////////////////////////////////////////////////

awsSink::awsSink() 
{
  
}

awsSink::~awsSink() {}

unsigned long 
awsSink::GetTriggerID(char *_name)
{
  unsigned long name=NameToId(_name);
  int i;

  for(i=0; i<triggers.Length(); ++i)
  {
    TriggerMap *tm = (TriggerMap *)triggers[i];

    if (tm->name == name) return i;
  }

  return 0;
}

void
awsSink::HandleTrigger(int trigger, iAwsSource &source)
{

  if (triggers.Length() == 0) return;
  
  // Get trigger.
  void (iBase::*Trigger)(iAwsSource &source) = ((TriggerMap *)(triggers[trigger]))->trigger;

  // Activate trigger.
  (this->*Trigger)(source);
}

void 
awsSink::RegisterTrigger(char *name, void (iBase::*Trigger)(iAwsSource &source))
{
  triggers.Push(new TriggerMap(NameToId(name), Trigger));
}

///////////////////////////////////// Signal Sources ////////////////////////////////////////////////////////
awsSource::awsSource() 
{  
}

awsSource::~awsSource() 
{
  int i;
  for(i=0; i<slots.Length(); ++i)
  {
    void *p = slots[i];
    delete (SlotSignalMap*)p;
  }

  slots.SetLength(0);
}


bool 
awsSource::RegisterSlot(iAwsSlot *slot, unsigned long signal)
{
  SlotSignalMap *ssm = new SlotSignalMap;

  ssm->slot = slot;
  ssm->signal = signal;

  slot->IncRef();
  slots.Push(ssm);
    
  return true;
}

bool 
awsSource::UnregisterSlot(iAwsSlot *slot, unsigned long signal)
{
    void *entry;
    int i;

    for(i=0; i<slots.Length(); ++i)
    {
       entry=slots[i];

       SlotSignalMap *ssm = STATIC_CAST(SlotSignalMap*,entry);

       if (ssm->signal == signal && ssm->slot == slot)
       {
          slot->DecRef();
          slots.Delete(i);
          delete ssm;

          return true;
       }
      
    }


  return false;
}

void 
awsSource::Broadcast(unsigned long signal)
{
 
 int i;

 for(i=0; i<slots.Length(); ++i)
 {
    
   SlotSignalMap *ssm = STATIC_CAST(SlotSignalMap*,slots[i]);

    if (ssm->signal == signal)
        ssm->slot->Emit(*this, signal);
   
 }
}
                                                                                                  

///////////////////////////////////// Slots ////////////////////////////////////////////////////////

awsSlot::awsSlot():sink(NULL) 
{
}

awsSlot::~awsSlot() 
{
  if (sink)
    sink->DecRef();
}

void 
awsSlot::Initialize(iAwsSink *_sink)
{
 sink = _sink;
 sink->IncRef();
}

void 
awsSlot::Connect(iAwsSource &source, unsigned long signal, unsigned long trigger)
{
  source.RegisterSlot(this, signal);

  int i;
  
  for(i=0; i<stmap.Length(); ++i)
  {
    SignalTriggerMap *stm = (SignalTriggerMap *)stmap[i];

    if (stm->signal==signal && stm->trigger==trigger)
    {
      stm->refs++;
      return;
    }
  }

  stmap.Push(new SignalTriggerMap(signal, trigger, 1)); 
}

void 
awsSlot::Disconnect(iAwsSource &source, unsigned long signal, unsigned long trigger)
{
  source.UnregisterSlot(this, signal);

  int i;

  for(i=0; i<stmap.Length(); ++i)
  {
    SignalTriggerMap *stm = (SignalTriggerMap *)stmap[i];

    if (stm->signal==signal && stm->trigger==trigger)
    {
      stm->refs--;

      if (stm->refs==0)
        stmap.Delete(i);

      return;
    }
  }

}

void 
awsSlot::Emit(iAwsSource &source, unsigned long signal)
{
    if (sink != 0)
    {
      int i;

      for(i=0; i<stmap.Length(); ++i)
      {
        SignalTriggerMap *stm = (SignalTriggerMap *)stmap[i];

        if (stm->signal==signal)
          sink->HandleTrigger(stm->trigger, source);
      }
      
    }
}
