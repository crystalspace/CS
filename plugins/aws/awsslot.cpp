#include "cssysdef.h"
#include "awsslot.h"
#include <string.h>

static unsigned long aws_adler32(unsigned long adler,  const unsigned char *buf,  unsigned int len);


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


///////////////////////////////////// Signal Sinks //////////////////////////////////////////////////////////

awsSink::awsSink() {}

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

awsSlot::awsSlot():sink(NULL) {}

awsSlot::~awsSlot() {}

void 
awsSlot::Initialize(iAwsSink *_sink)
{
 sink = _sink;
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

      //(sink->*Slot)(source, signal);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

#define BASE 65521L /* largest prime smaller than 65536 */
#define NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

/* ========================================================================= */
unsigned long
aws_adler32(unsigned long adler,  const unsigned char *buf,  unsigned int len)
{
    unsigned long s1 = adler & 0xffff;
    unsigned long s2 = (adler >> 16) & 0xffff;
    int k;

    if (buf == NULL) return 1L;

    while (len > 0) {
        k = len < NMAX ? len : NMAX;
        len -= k;
        while (k >= 16) {
            DO16(buf);
	    buf += 16;
            k -= 16;
        }
        if (k != 0) do {
            s1 += *buf++;
	    s2 += s1;
        } while (--k);
        s1 %= BASE;
        s2 %= BASE;
    }
    return (s2 << 16) | s1;
}

