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

/**********************************************************************************************************************
*                                                                                                                     *
*   This implements the slot architecture.  Slots are sinks for signals.  Signals always have a source.  Slots may be *
*  a sink for multiple signals from multiple sources.  It is up to the user to determine how to use his slots.  Slots *
*  do not care.                                                                                                       *
*                                                                                                                     *
**********************************************************************************************************************/
class awsSlot : public iAwsSlot
{
   awsComponent *sink;

   void (awsComponent::*Slot)(awsComponent &source, unsigned long signal);

public:
  /// Does nothing
  awsSlot();

  /// Also does nothing
  virtual ~awsSlot();

  /// Sets up the slot's sink
  virtual void Initialize(awsComponent *sink, void (awsComponent::*_Slot)(awsComponent &source, unsigned long signal));
  
  /// Connects the slot to a signal source
  virtual void Connect(awsComponent &source, unsigned long signal);
                                          
  /// Disconnects the slot from a signal source
  virtual void Disconnect(awsComponent &source, unsigned long signal);

  /// Emit a signal
  virtual void Emit(awsComponent &source, unsigned long signal);
};


#endif 
