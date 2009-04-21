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

#ifndef __CS_CSUTIL_SYSTEMOPENMANAGER_H__
#define __CS_CSUTIL_SYSTEMOPENMANAGER_H__

#include "iutil/eventq.h"
#include "iutil/systemopenmanager.h"
#include "csutil/scf_implementation.h"

struct iObjectRegistry;

/**\file
 * Manager for system open events.
 */

struct iObjectRegistry;

namespace CS
{
  namespace Base
  {
    class CS_CRYSTALSPACE_EXPORT SystemOpenManager :
      public scfImplementation2<SystemOpenManager,
                                iSystemOpenManager,
                                iEventHandler>
    {
      bool isOpen;
      csEventID subscribeIDs[3];
      csRef<iEventQueue> queue;
    public:
      SystemOpenManager (iObjectRegistry* q);
    
      csHandlerID Register (iEventHandler* eventh);
      csHandlerID RegisterWeak (iEventHandler* eventh,
	csRef<iEventHandler> &handler);
      void RemoveListener (iEventHandler* eventh);
      void RemoveWeakListener (csRef<iEventHandler> &handler);
      
      bool HandleEvent (iEvent& ev);
      CS_EVENTHANDLER_NAMES("crystalspace.systemopenmgr")
      CS_EVENTHANDLER_NIL_CONSTRAINTS
    };
  } // namespace Base
} // namespace CS

#endif // __CS_CSUTIL_SYSTEMOPENMANAGER_H__
