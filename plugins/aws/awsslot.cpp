#include "cssysdef.h"
#include "awsslot.h"

///////////////////////////////////// Signal Sources ////////////////////////////////////////////////////////
awsSigSrc::awsSigSrc() 
{
}

awsSigSrc::~awsSigSrc() 
{
}


bool 
awsSigSrc::RegisterSlot(iAwsSlot *slot, unsigned long signal)
{
  SlotSignalMap *ssm = new SlotSignalMap;

  ssm->slot = slot;
  ssm->signal = signal;

  slots.AddItem(ssm);
    
  return true;
}

bool 
awsSigSrc::UnregisterSlot(iAwsSlot *slot, unsigned long signal)
{
    void *entry = slots.GetFirstItem();

    while(entry)
    {
       SlotSignalMap *ssm = static_cast<SlotSignalMap *>(entry);

       if (ssm->signal == signal && ssm->slot == slot)
       {
          slots.RemoveItem();
          delete ssm;

          return true;
       }

       entry = slots.GetNextItem();
    }


  return false;
}

void 
awsSigSrc::Broadcast(unsigned long signal)
{
 void *entry = slots.GetFirstItem();

 while(entry)
 {
    SlotSignalMap *ssm = static_cast<SlotSignalMap *>(entry);

    if (ssm->signal == signal)
        ssm->slot->Emit(*this, signal);
    

    entry = slots.GetNextItem();
 }
}
                                                                                                  

///////////////////////////////////// Slots ////////////////////////////////////////////////////////

awsSlot::awsSlot():sink(NULL), Slot(NULL) {}

awsSlot::~awsSlot() {}

void 
awsSlot::Initialize(iBase *_sink, void (iBase::*_Slot)(iBase &source, unsigned long signal))
{
 sink = _sink;
 Slot = _Slot;
}

void 
awsSlot::Connect(iAwsSigSrc &source, unsigned long signal)
{
  source.RegisterSlot(this, signal);
}

void 
awsSlot::Disconnect(iAwsSigSrc &source, unsigned long signal)
{
  source.UnregisterSlot(this, signal);
}

void 
awsSlot::Emit(iBase &source, unsigned long signal)
{
    if (sink && Slot) 
      (sink->*Slot)(source, signal);
}

