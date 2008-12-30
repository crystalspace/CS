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

#ifndef __CS_CSUTIL_LISTACCESSQUEUE_H__
#define __CS_CSUTIL_LISTACCESSQUEUE_H__

#include "csutil/fifo.h"
#include "csutil/ref.h"
#include "csutil/refcount.h"
#include "csutil/threading/mutex.h"
#include "iutil/job.h"
#include "iutil/threadmanager.h"

class CS_CRYSTALSPACE_EXPORT ListAccessQueue : public csRefCount
{
public:
  ListAccessQueue() : total(0)
  {
  }

  ~ListAccessQueue()
  {
    ProcessQueue(total);
  }

  void Enqueue(iJob* job, QueueType type)
  {
    if(type == HIGH)
    {
      CS::Threading::RecursiveMutexScopedLock lock(highQueueLock);
      highqueue.Push(job);
    }
    else if(type == MED)
    {
      CS::Threading::RecursiveMutexScopedLock lock(medQueueLock);
      medqueue.Push(job);
    }
    else if(type == LOW)
    {
      CS::Threading::RecursiveMutexScopedLock lock(lowQueueLock);
      lowqueue.Push(job);
    }
    
    CS::Threading::AtomicOperations::Increment(&total);
  }

  void ProcessQueue(uint num)
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

private:
  inline void ProcessHighQueue(uint& i, uint& num)
  {
    CS::Threading::RecursiveMutexScopedLock lock(highQueueLock);
    for(; i<num && highqueue.GetSize() != 0; i++)
    {
      highqueue.PopTop()->Run();
      CS::Threading::AtomicOperations::Decrement(&total);
    }
  }

  inline void ProcessMedQueue(uint& i, uint& num)
  {
    ProcessHighQueue(i, num);
    CS::Threading::RecursiveMutexScopedLock lock(medQueueLock);
    for(; i<num && medqueue.GetSize() != 0; i++)
    {
      medqueue.PopTop()->Run();
      CS::Threading::AtomicOperations::Decrement(&total);
      ProcessHighQueue(i, num);
    }
  }

  inline void ProcessLowQueue(uint& i, uint& num)
  {
    ProcessHighQueue(i, num);
    CS::Threading::RecursiveMutexScopedLock lock(lowQueueLock);
    for(; i<num && lowqueue.GetSize() != 0; i++)
    {
      lowqueue.PopTop()->Run();
      CS::Threading::AtomicOperations::Decrement(&total);
      ProcessHighQueue(i, num);
      ProcessMedQueue(i, num);
    }
  }

  CS::Threading::RecursiveMutex highQueueLock;
  CS::Threading::RecursiveMutex medQueueLock;
  CS::Threading::RecursiveMutex lowQueueLock;
  csFIFO<csRef<iJob> > highqueue;
  csFIFO<csRef<iJob> > medqueue;
  csFIFO<csRef<iJob> > lowqueue;
  int32 total;
};

#endif // __CS_CSUTIL_LISTACCESSQUEUE_H__
