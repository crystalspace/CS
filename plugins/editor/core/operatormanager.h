/*
    Copyright (C) 2011 by Jelle Hellemans

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

#ifndef __OPERATORMANAGER_H__
#define __OPERATORMANAGER_H__

#include <csutil/refarr.h>
#include <csutil/hash.h>

#include <iutil/event.h>
#include <iutil/eventh.h>
#include <iutil/eventq.h>


#include "ieditor/operator.h"


namespace CS {
namespace EditorApp {

class OperatorManager : public scfImplementation2<OperatorManager,iOperatorManager, iEventHandler>
{
public:
  OperatorManager (iObjectRegistry* obj_reg);
  virtual ~OperatorManager ();
  
  virtual void Initialize ();
  virtual void Uninitialize ();
  
  //iOperatorManager
  virtual iOperator* Execute (const char*);
  virtual iOperator* Invoke (const char*, iEvent*);
  virtual void Register(const char*, iOperatorFactory*);
  virtual const char* GetLabel (const char*);
  virtual const char* GetDescription (const char*);

private:  
  //iEventHandler
  virtual bool HandleEvent(iEvent& ev);
  csRef<iOperator> modalOperator; //TODO: needs to become a stack?
  
  /// The queue of events waiting to be handled.
  csRef<iEventQueue> eventQueue;
  /// The event name registry, used to convert event names to IDs and back.
  csRef<iEventNameRegistry> nameRegistry;

  CS_EVENTHANDLER_NAMES ("crystalspace.editor.managers.operator")
  CS_EVENTHANDLER_NIL_CONSTRAINTS

private:
  iObjectRegistry* object_reg;
  csHash<csRef<iOperatorFactory>, csString> operatorFactories;
  csRef<iOperatorFactory> GetOperator (const char* name);
};


} // namespace EditorApp
} // namespace CS

#endif
