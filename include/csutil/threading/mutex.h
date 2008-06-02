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

#ifndef __CS_CSUTIL_THREADING_MUTEX_H__
#define __CS_CSUTIL_THREADING_MUTEX_H__

#include "csutil/noncopyable.h"

/**\file
* Mutual-exclusion object.
* A mutual exclusion object ensures that only one thread at a time access a 
* shared object.
*/

// Include implementation specific versions
#if defined(CS_PLATFORM_WIN32)
# include "csutil/threading/win32_mutex.h"
#elif defined(CS_PLATFORM_UNIX) || \
      defined(CS_PLATFORM_MACOSX)
# include "csutil/threading/pthread_mutex.h"
#else
#error "No threading implementation for your platform"
#endif

namespace CS
{
namespace Threading
{

  /**
   * General mutex class.
   * A mutex is a mutual exclusion object, it stops two threads from having
   * it locked at the same time.
   * A thread can get the lock by calling Lock or TryLock, and release it by
   * calling Unlock.
   *
   * Users are advised to use the ScopedLock helper class.
   */
  template<typename BaseMutex>
  class MutexImpl : public BaseMutex, 
                    private CS::NonCopyable
  {
  public:
    /**
     * Initialize an initially unlocked mutex.
     */
    MutexImpl ()
    {
      BaseMutex::Initialize ();
    }

    /**
     * Destroy mutex.
     */
    ~MutexImpl ()
    {
      BaseMutex::Destroy ();
    }

    /**
     * Acquire lock on mutex.
     * Will block until lock can be successfully acquired.
     * \return true if lock was successfully acquired.
     */
    bool Lock ()
    {
      return BaseMutex::Lock ();
    }

    /**
     * Try to acquire lock on mutex.
     * Will return without blocking.
     * \return true if lock was successfully acquired.
     */
    bool TryLock ()
    {
      return BaseMutex::TryLock ();
    }

    /**
     * Unlock the mutex.
     */
    void Unlock ()
    {
      BaseMutex::Unlock ();
    }

  protected:
    friend class ConditionBase;
  };

  /**
   * Basic non-recursive mutex
   */
  typedef MutexImpl<Implementation::MutexBase> Mutex;
  
  /**
   * Basic recursive mutex.
   * The difference between a normal mutex and recursive mutex is that a 
   * recursive mutex won't lock if same thread try to enter it several times.
   */
  typedef MutexImpl<Implementation::RecursiveMutexBase> RecursiveMutex;

  /**
   * Helper that is a (non-recursive) mutex if \a _Lock is \c true or does 
   * nothing if \a _Lock is \c false. Intended to provide compile-time 
   * switching of locking behaviour.
   */
  template<bool _Lock>
  class OptionalMutex
  {
    Mutex theMutex;
  public:
    /// \sa Mutex::Lock
    bool Lock () { return theMutex.Lock(); }
    /// \sa Mutex::TryLock
    bool TryLock() { return theMutex.TryLock(); }
    /// \sa Mutex::Unlock
    void Unlock() { theMutex.Unlock(); }
  };
  
  template<>
  class OptionalMutex<false>
  {
  public:
    bool Lock () { return true; }
    bool TryLock() { return true; }
    void Unlock() { }
  };
  

  /**
   * This is a utility class for locking a Mutex. If a ScopedLock class is
   * created it locks the mutex, when it is destroyed it unlocks the Mutex
   * again. So locking a mutex can happen by creating a ScopedLock object on the
   * stack. The compiler will then take care that the Unlock calls will be done
   * in each case.
   * \code
   *   void Myfunc() {
   *      ScopedLock lock(mymutex);
   *      do something special
   *
   *      return;
   *  }
   * \endcode
   */
  template<typename T>
  class ScopedLock
  {
  public:
    ScopedLock (T& lockObj)
      : lockObj (lockObj)
    {
      lockObj.Lock ();
    }

    ~ScopedLock ()
    {
      lockObj.Unlock ();
    }

  private:
    T& lockObj;    
  };

  // Standard lock
  typedef ScopedLock<Mutex> MutexScopedLock;
  typedef ScopedLock<RecursiveMutex> RecursiveMutexScopedLock;
}
}

#endif
