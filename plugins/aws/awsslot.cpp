#include "cssysdef.h"
#include "awsslot.h"
#include "awscomp.h"

awsSlot::awsSlot():sink(NULL), Slot(NULL) {}

awsSlot::~awsSlot() {}

void 
awsSlot::Initialize(awsComponent *_sink, void (awsComponent::*_Slot)(awsComponent &source, unsigned long signal))
{
 sink = _sink;
 Slot = _Slot;
}

void 
awsSlot::Connect(awsComponent &source, unsigned long signal)
{
  source.RegisterSlot(this, signal);
}

void 
awsSlot::Disconnect(awsComponent &source, unsigned long signal)
{
  source.UnregisterSlot(this, signal);
}

void 
awsSlot::Emit(awsComponent &source, unsigned long signal)
{
    if (sink && Slot) 
      (sink->*Slot)(source, signal);
}


