/*
  Copyright (C) 2008 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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
#include "csutil/eventnames.h"
#include "csutil/platform.h"
#include "csutil/threadmanager.h"
#include "iutil/event.h"
#include "iutil/eventq.h"

namespace CS
{
  namespace Utility
  {
    csThreadManager::csThreadManager(iObjectRegistry* objReg) : scfImplementationType(this),
      objectReg(objReg)
    {
      uint count = CS::Platform::GetProcessorCount();

      // If we can't detect, assume we have one.
      if(count == 0)
      {
        count = 1;
      }

      // Have 'processor count' extra processing threads.
      threadQueue.AttachNew(new ThreadedJobQueue(count));
      listQueue.AttachNew(new ListAccessQueue());

      eventQueue = csQueryRegistry<iEventQueue>(objectReg);
      if(eventQueue)
      {
        ProcessQueue = csevFrame(objReg);
        eventQueue->RegisterListener(this, ProcessQueue);
      }
    }

    bool csThreadManager::HandleEvent(iEvent& Event)
    {
      if(Event.Name == ProcessQueue)
      {
        Process();
      }
      return false;
    }
  }
}
