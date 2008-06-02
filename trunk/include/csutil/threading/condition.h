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

#ifndef __CS_CSUTIL_THREADING_CONDITION_H__
#define __CS_CSUTIL_THREADING_CONDITION_H__

// Include implementation specific versions
#if defined(CS_PLATFORM_WIN32)
# include "csutil/threading/win32_condition.h"
#elif defined(CS_PLATFORM_UNIX) || \
  defined(CS_PLATFORM_MACOSX)
# include "csutil/threading/pthread_condition.h"
#else
#error "No threading implementation for your platform"
#endif


namespace CS
{
namespace Threading
{

  /**
   * Condition variable.
   * A condition variable is a synchronization primitive used to have threads
   * wait for a shared state variable to attain a given value.
   */
  class Condition : private Implementation::ConditionBase,
                    private CS::NonCopyable
  {
  public:
    Condition ()
    {
    }

    /**
     * Notify and wake up one thread waiting for the condition.
     */
    void NotifyOne ()
    {
      ConditionBase::NotifyOne ();
    }

    /**
     * Notify and wake up all threads currently waiting for the condition.
     * The order different threads get woke up is indeterminate.
     */
    void NotifyAll ()
    {
      ConditionBase::NotifyAll ();
    }

    /**
     * Wait for some change of condition.  Suspends the calling thread until some
     * other thread invokes Notify() or NotifyAll() to notify a change of condition.
     * \param lock The mutex to associate with this condition. The caller must
     *   already hold a lock on the mutex before calling Wait(), and all threads
     *   waiting on the condition must be using the same mutex. The mutex must
     *   <b>not</b> be locked recursively within the same thread. When called,
     *   Wait() releases the caller's lock on the mutex and suspends the caller's
     *   thread. Upon return from Wait(), the caller's lock on the mutex is
     *   restored.
     * \param timeout Timeout in milliseconds for the wait. A value of 0 means
     *   infinite wait.
     * \return \c true when the condition was changed, or \c false if the wait
     *   timed out.
     * \remarks The reason that the mutex must not be locked recursively is
     *   because the implicit unlock performed by Wait() <em>must</em> actually
     *   release the mutex in order for other threads to be able to satisfy the
     *   condition. With recursively locked mutexes, there is no guarantee that
     *   the one implicit unlock operation performed by Wait() will actually
     *   release the mutex since it might have been locked multiple times within
     *   the same thread.
     * \remarks Having to use a timed wait may indicate a code flaw. Try to
     *   avoid timed waits and instead fix your synchronization logic.
     */
    template<typename LockType>
    bool Wait (LockType& lock, csTicks timeout = 0)
    {      
      return ConditionBase::Wait (lock, timeout);
    }
  };

}
}


#endif
