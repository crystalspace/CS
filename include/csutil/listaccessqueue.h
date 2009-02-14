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
    queueLock.Initialize();
  }

  void Enqueue(iJob* job)
  {
    MutexScopedLock lock(queueLock);
    queue.Push(job);    
  }

  void ProcessQueue(uint num)
  {
    MutexScopedLock lock(queueLock);
    for(uint i=0; i<num && queue.GetSize() != 0; i++)
    {
      queue.PopTop()->Run();
    }
  }

private:
  Mutex queueLock;
  csFIFO<csRef<iJob> > queue;
};

#endif // __CS_CSUTIL_LISTACCESSQUEUE_H__
