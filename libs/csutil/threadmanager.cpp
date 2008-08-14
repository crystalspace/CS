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
#include "csutil/platform.h"
#include "csutil/sysfunc.h"
#include "csutil/threadmanager.h"
#include "iengine/engine.h"

ThreadID csThreadManager::tid = Thread::GetThreadID();

csThreadManager::csThreadManager(iObjectRegistry* objReg) : scfImplementationType(this), 
  objectReg(objReg)
{
  waiting = 0;
  threadCount = CS::Platform::GetProcessorCount()+1;

  // If we can't detect, assume we have one.
  if(threadCount == 0)
  {
    threadCount = 1;
  }

  // Have 'processor count' extra processing threads.
  threadQueue.AttachNew(new ThreadedJobQueue(threadCount));
  listQueue.AttachNew(new ListAccessQueue());

  // Event handler.
  tMEventHandler.AttachNew(new TMEventHandler(this));

  eventQueue = csQueryRegistry<iEventQueue>(objectReg);
  if(eventQueue.IsValid())
  {
    ProcessPerFrame = csevFrame(objReg);
    ProcessWhileWait = csevThreadWait(objReg);
    eventQueue->RegisterListener(tMEventHandler, ProcessPerFrame);
    eventQueue->RegisterListener(tMEventHandler, ProcessWhileWait);
  }
}

csThreadManager::~csThreadManager()
{
  eventQueue->RemoveListener(tMEventHandler);
}

void csThreadManager::Process(uint num)
{
  listQueue->ProcessQueue(num);  
}

void csThreadManager::Wait(csRef<iThreadReturn> result)
{
  if(!result->IsFinished())
  {
    if(!IsMainThread())
    {
      AtomicOperations::Increment(&waiting);
    }

    while(!result->IsFinished())
    {
      if(IsMainThread())
      {
        csRef<iEvent> evt = eventQueue->CreateBroadcastEvent(ProcessWhileWait);
        eventQueue->Dispatch(*evt);
        csSleep(1);
      }
    }

    if(!IsMainThread())
    {
      AtomicOperations::Decrement(&waiting);
    }
  }
}
