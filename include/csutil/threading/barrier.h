/*
  Copyright (C) 2007 by Marten Svanfeldt

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

#ifndef __CS_CSUTIL_THREADING_BARRIER_H__
#define __CS_CSUTIL_THREADING_BARRIER_H__

#include "csutil/noncopyable.h"
#include "csutil/threading/condition.h"
#include "csutil/threading/mutex.h"

namespace CS
{
namespace Threading
{
  
  /**
   * Barrier synchronization class.
   * A barrier is used to synchronize the execution streams of any number of
   * threads. All threads calls Wait that will block until the set number
   * of threads have called it.
   */
  class Barrier : private CS::NonCopyable
  {
  public:
    /**
     * Initialize barrier with set max count
     */
    Barrier (size_t maxCount)
      : maxCount (maxCount), currentCount (0)
    {}


    /**
     * Wait for all threads to have called Wait.
     * \returns true if caller is last thread to call Wait
     */
    bool Wait ()
    {
      MutexScopedLock lock (mutex);
      if (++currentCount == maxCount)
      {
        currentCount = 0;
        condition.NotifyAll ();
        return true;
      }
      else
      {
        condition.Wait (mutex);
        return false;
      }
    }

  private:
    Mutex mutex;
    Condition condition;

    const size_t maxCount;
    size_t currentCount;
  };
  
}
}

#endif
