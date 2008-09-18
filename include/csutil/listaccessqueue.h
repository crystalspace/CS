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

using namespace CS::Threading;

class CS_CRYSTALSPACE_EXPORT ListAccessQueue : public csRefCount
{
public:
  ListAccessQueue()
  {
  }

  void Enqueue(iJob* job, bool high)
  {    
    if(high)
    {
      RecursiveMutexScopedLock lock(highQueueLock);
      highqueue.Push(job);
    }
    else
    {
      RecursiveMutexScopedLock lock(lowQueueLock);
      lowqueue.Push(job);
    }
  }

  void ProcessQueue(uint num)
  {
    {
      // Run num jobs.
      RecursiveMutexScopedLock lock(highQueueLock);
      for(size_t i=0; i<num && highqueue.GetSize() != 0; i++)
      {
        highqueue.PopTop()->Run();
      }
    }

    // Run one job.
    if(lowqueue.GetSize() != 0)
    {
      RecursiveMutexScopedLock lock(lowQueueLock);
      lowqueue.PopTop()->Run();
    }
  }

private:
  RecursiveMutex lowQueueLock;
  RecursiveMutex highQueueLock;
  csFIFO<csRef<iJob> > highqueue;
  csFIFO<csRef<iJob> > lowqueue;
};

#endif // __CS_CSUTIL_LISTACCESSQUEUE_H__
