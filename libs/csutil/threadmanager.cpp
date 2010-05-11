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

csThreadManager::ListAccessQueue::ListAccessQueue() : total(0)
{
}

csThreadManager::ListAccessQueue::~ListAccessQueue()
{
  ProcessAll ();
}

void csThreadManager::ListAccessQueue::Enqueue(iJob* job, QueueType type)
{
  if(type == HIGH)
  {
    RecursiveMutexScopedLock lock(highQueueLock);
    highqueue.Push(job);
  }
  else if(type == MED)
  {
    RecursiveMutexScopedLock lock(medQueueLock);
    medqueue.Push(job);
  }
  else if(type == LOW)
  {
    RecursiveMutexScopedLock lock(lowQueueLock);
    lowqueue.Push(job);
  }
  
  AtomicOperations::Increment(&total);
}

void csThreadManager::ListAccessQueue::ProcessQueue(uint num)
{
  uint i=0;

  ProcessHighQueue(i, num);

  if(i<num)
  {
    ProcessMedQueue(i, num);
  }

  if(i<num)
  {
    ProcessLowQueue(i, num);
  }
}

int32 csThreadManager::ListAccessQueue::GetQueueCount() const
{ 
  return AtomicOperations::Read (&total);
}

void csThreadManager::ListAccessQueue::ProcessAll ()
{
  while (GetQueueCount() > 0)
    ProcessQueue (total);
}

inline void csThreadManager::ListAccessQueue::ProcessHighQueue(uint& i, uint& num)
{
  RecursiveMutexScopedLock lock(highQueueLock);
  for(; i<num && highqueue.GetSize() != 0; i++)
  {
    AtomicOperations::Decrement(&total);
    highqueue.PopTop()->Run();
  }
}

inline void csThreadManager::ListAccessQueue::ProcessMedQueue(uint& i, uint& num)
{
  ProcessHighQueue(i, num);
  RecursiveMutexScopedLock lock(medQueueLock);
  for(; i<num && medqueue.GetSize() != 0; i++)
  {
    AtomicOperations::Decrement(&total);
    medqueue.PopTop()->Run();
    ProcessHighQueue(i, num);
  }
}

inline void csThreadManager::ListAccessQueue::ProcessLowQueue(uint& i, uint& num)
{
  ProcessHighQueue(i, num);
  RecursiveMutexScopedLock lock(lowQueueLock);
  for(; i<num && lowqueue.GetSize() != 0; i++)
  {
    AtomicOperations::Decrement(&total);
    lowqueue.PopTop()->Run();
    ProcessHighQueue(i, num);
    ProcessMedQueue(i, num);
  }
}
  
//---------------------------------------------------------------------------
  
ThreadID csThreadManager::tid;

csThreadManager::csThreadManager(iObjectRegistry* objReg) : scfImplementationType(this), 
  waiting(0), alwaysRunNow(false), objectReg(objReg), exiting(false)
{
  tid = Thread::GetThreadID();

  threadCount = CS::Platform::GetProcessorCount();

  // If we can't detect, assume we have one.
  if(threadCount == 0)
  {
    csFPrintf(stderr, "Processor count couldn't be detected!\n");
    threadCount = 1;
  }

  // Have 'processor count' extra processing threads.
  threadQueue.AttachNew(new ThreadedJobQueue(threadCount, THREAD_PRIO_LOW,
    "thread manager"));
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
  Condition* c;
  Mutex* m;
  bool success = true;
  csRef<iThreadReturn> threadReturn;

  if(!IsMainThread())
  {
    AtomicOperations::Increment(&waiting);
  }

  while(!threadReturns.IsEmpty())
  {
    threadReturn = threadReturns.Pop();

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

    threadReturn->SetWaitPtrs(c, m);

    m->Lock();
    while(!threadReturn->IsFinished())
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
        if(process)
        {
          waitingThreadsLock.Unlock();
          m->Unlock();
          threadQueue->PullAndRun(threadReturn->GetJob());
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

    threadReturn->SetWaitPtrs(0, 0);

    if(!IsMainThread())
    {
      delete c;
      delete m;
    }

    success &= threadReturn->WasSuccessful();
  }

  if(!IsMainThread())
  {
    AtomicOperations::Decrement(&waiting);
  }

  return success;
}

void csThreadManager::ProcessAll ()
{
  threadQueue->WaitAll ();
  listQueue->ProcessAll ();  
}

