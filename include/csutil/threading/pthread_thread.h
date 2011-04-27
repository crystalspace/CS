/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef __CS_CSUTIL_THREADING_PTHREAD_THREAD_H__
#define __CS_CSUTIL_THREADING_PTHREAD_THREAD_H__

#ifndef DOXYGEN_RUN

#include <pthread.h>
#include "csutil/threading/barrier.h"

namespace CS
{
namespace Threading
{

  enum ThreadPriority;
  class Runnable;

namespace Implementation
{

  // Thread base-class for pthreads
  class CS_CRYSTALSPACE_EXPORT ThreadBase : public CS::Utility::AtomicRefCount
  {
  public:
    ThreadBase (Runnable* runnable);

    void Start ();

    void Stop ();

    bool IsRunning () const;

    bool SetPriority (ThreadPriority prio);
    
    void Wait () const;

    static void Yield ();

    static ThreadID GetThreadID ();

    ThreadPriority GetPriority () const
    {
      return priority;
    }

  private:
    csRef<Runnable> runnable;

    pthread_t threadHandle;
    
    int32 isRunning;
    ThreadPriority priority;
    Barrier startupBarrier;
  };


}
}
}

#endif // DOXYGEN_RUN

#endif // __CS_CSUTIL_THREADING_PTHREAD_THREAD_H__
