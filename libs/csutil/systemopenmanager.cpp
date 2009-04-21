/*
    Copyright (C) 2008 by Frank Richter
    
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

#include "csutil/systemopenmanager.h"

#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "csutil/event.h"
#include "csutil/eventnames.h"

/**\file
 * Manager for system open events.
 */
 
namespace CS
{
  namespace Base
  {
    SystemOpenManager::SystemOpenManager (iObjectRegistry* objReg)
     : scfImplementationType (this), isOpen (false)
    {
      queue = csQueryRegistry<iEventQueue> (objReg);
      CS_ASSERT(queue);
      subscribeIDs[0] = csevSystemOpen (objReg);
      subscribeIDs[1] = csevSystemClose (objReg);
      subscribeIDs[2] = CS_EVENTLIST_END;
      queue->RegisterListener (this, subscribeIDs);
    }
    
    csHandlerID SystemOpenManager::Register (iEventHandler* eventh)
    {
      csHandlerID id = queue->RegisterListener (eventh, subscribeIDs);
      if (isOpen)
      {
        csRef<iEvent> e (queue->CreateBroadcastEvent (subscribeIDs[0]));
        eventh->HandleEvent (*((iEvent*)e));
      }
      return id;
    }
    
    csHandlerID SystemOpenManager::RegisterWeak (iEventHandler* eventh,
      csRef<iEventHandler> &handler)
    {
      csHandlerID id = CS::RegisterWeakListener (queue, eventh, handler);
      queue->Subscribe (handler, subscribeIDs);
      if (isOpen)
      {
        csRef<iEvent> e (queue->CreateBroadcastEvent (subscribeIDs[0]));
        eventh->HandleEvent (*((iEvent*)e));
      }
      return id;
    }
    
    void SystemOpenManager::RemoveListener (iEventHandler* eventh)
    {
      queue->RemoveListener (eventh);
    }
    
    void SystemOpenManager::RemoveWeakListener (csRef<iEventHandler> &handler)
    {
      CS::RemoveWeakListener (queue, handler);
    }
  
    bool SystemOpenManager::HandleEvent (iEvent& event)
    {
      if (event.Name == subscribeIDs[0])
      {
	isOpen = true;
	return true;
      }
      else if (event.Name == subscribeIDs[1])
      {
	isOpen = false;
	return true;
      }
      return false;
    }
  
  } // namespace CS
} // namespace CS

