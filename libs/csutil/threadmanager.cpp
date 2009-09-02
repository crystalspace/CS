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
#include "iutil/cfgmgr.h"

using namespace CS::Threading;

ThreadID csThreadManager::tid = Thread::GetThreadID();

csThreadManager::csThreadManager(iObjectRegistry* objReg) : scfImplementationType(this), 
  waiting(0), alwaysRunNow(false), objectReg(objReg), exiting(false)
{
  threadCount = CS::Platform::GetProcessorCount();

  // If we can't detect, assume we have one.
  if(threadCount == 0)
  {
    csFPrintf(stderr, "Processor count couldn't be detected!\n");
    threadCount = 1;
  }

  // Have 'processor count' extra processing threads.
  size_t threadCountHalf = size_t (ceil (threadCount/2.0f));
  threadQueue.AttachNew(new ThreadedJobQueue(threadCount, THREAD_PRIO_LOW));
  listQueue.AttachNew(new ListAccessQueue());

  // Event handler.
  tMEventHandler.AttachNew(new TMEventHandler(this));

  eventQueue = csQueryRegistry<iEventQueue>(objectReg);
  if(eventQueue.IsValid())
  {
    ProcessPerFrame = csevFrame(objReg);
    eventQueue->RegisterListener(tMEventHandler, ProcessPerFrame);
  }
}

csThreadManager::~csThreadManager()
{
  exiting = true;
  eventQueue->RemoveListener(tMEventHandler);
}

void csThreadManager::Init(iConfigManager* config)
{
  int32 oldCount = threadCount;
  threadCount = config->GetInt("ThreadManager.Threads", threadCount);
  if(oldCount != threadCount)
  {
    threadQueue.AttachNew(new ThreadedJobQueue(threadCount));
  }

  alwaysRunNow = config->GetBool("ThreadManager.AlwaysRunNow");
}

void csThreadManager::Process(uint num)
{
  listQueue->ProcessQueue(num);  
}

bool csThreadManager::Wait(csRefArray<iThreadReturn>& threadReturns, bool process)
{
  bool success = true;

  if(!IsMainThread())
  {
    AtomicOperations::Increment(&waiting);
  }

  while(threadReturns.GetSize() != 0)
  {
    Condition* c;
    Mutex* m;
    if(IsMainThread())
    {
      c = &waitingMain;
      m = &waitingMainLock;
    }
    else
    {
      c = new Condition();
      m = new Mutex();
    }

    threadReturns[0]->SetWaitPtrs(c, m);

    m->Lock();
    while(!threadReturns[0]->IsFinished())
    {
      if(IsMainThread())
      {
        if(process && listQueue->GetQueueCount() > 0)
        {
          m->Unlock();
          listQueue->ProcessQueue(1);
          m->Lock();
        }
        else
        {
          c->Wait(*m);
        }
      }
      else
      {
        MutexScopedLock lock(waitingThreadsLock);
        if(process && threadQueue->GetQueueCount() > 0)
        {
          waitingThreadsLock.Unlock();
          m->Unlock();
          //@@TODO: Fix!
          //threadQueue->PopAndRun();
          m->Lock();
          waitingThreadsLock.Lock();
        }
        else
        {
          waitingThreads.Push(c);
          waitingThreadsLock.Unlock();

          {
            c->Wait(*m);
          }

          waitingThreadsLock.Lock();
          waitingThreads.Delete(c);
        }
      }
    }
    m->Unlock();

    threadReturns[0]->SetWaitPtrs(0, 0);

    if(!IsMainThread())
    {
      delete c;
      delete m;
    }

    success &= threadReturns[0]->WasSuccessful();
    threadReturns.DeleteIndexFast(0);
  }

  if(!IsMainThread())
  {
    AtomicOperations::Decrement(&waiting);
  }

  return success;
}
