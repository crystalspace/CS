#ifndef __AWS_SLOT_H__
#define __AWS_SLOT_H__

/**************************************************************************
    Copyright (C) 2000-2001 by Christopher Nelson 
    
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
*****************************************************************************/

#include "ivaria/aws.h"
#include "csutil/csdllist.h"

/*********************************************************************************************************************
*                                                                                                                    *
*      This implements the signal source architecture.  A signal source is an object which knows how to register and *
*    unregister slots.  A signal source always defines and emits signals.  A signal source does not care if someone  *
*    is listening or not.                                                                                            *
*                                                                                                                    *
*********************************************************************************************************************/
class awsSigSrc : public iAwsSigSrc
{
   /// contains a list of all slots that we have registered
   csDLinkList slots;

   struct SlotSignalMap
   {
      /// The slot that's registered
      iAwsSlot *slot;

      /// The signal it's registered for
      unsigned long signal;
   };

public:  
    awsSigSrc();

    virtual ~awsSigSrc();

    /// Registers a slot for any one of the signals defined by a source.  Each sources's signals exist in it's own namespace
    virtual bool RegisterSlot(iAwsSlot *slot, unsigned long signal);

    /// Unregisters a slot for a signal.
    virtual bool UnregisterSlot(iAwsSlot *slot, unsigned long signal);

    /// Broadcasts a signal to all slots that are interested.
    virtual void Broadcast(unsigned long signal);
};

/**********************************************************************************************************************
*                                                                                                                     *
*   This implements the slot architecture.  Slots are sinks for signals.  Signals always have a source.  Slots may be *
*  a sink for multiple signals from multiple sources.  It is up to the user to determine how to use his slots.  Slots *
*  do not care.                                                                                                       *
*                                                                                                                     *
**********************************************************************************************************************/
class awsSlot : public iAwsSlot
{
   iBase *sink;

   void (iBase::*Slot)(iBase &source, unsigned long signal);

public:
  /// Does nothing
  awsSlot();

  /// Also does nothing
  virtual ~awsSlot();

  /// Sets up the slot's sink
  virtual void Initialize(iBase *sink, void (iBase::*_Slot)(iBase &source, unsigned long signal));
  
  /// Connects the slot to a signal source
  virtual void Connect(iAwsSigSrc &source, unsigned long signal);
                                          
  /// Disconnects the slot from a signal source
  virtual void Disconnect(iAwsSigSrc &source, unsigned long signal);

  /// Emit a signal
  virtual void Emit(iBase &source, unsigned long signal);
};


#endif 

