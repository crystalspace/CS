/*
    Copyright (C) 2002 by Norman Kraemer
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
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

#ifndef __CS_CSSYS_THREAD_H__
#define __CS_CSSYS_THREAD_H__

#include "csutil/ref.h"
#include "csutil/refcount.h"

/// List of errorcodes for threads.
enum
{
  CS_THREAD_NO_ERROR = 0,
  CS_THREAD_UNKNOWN_ERROR,
  CS_THREAD_OUT_OF_RESOURCES,
  CS_THREAD_ERR_ATTRIBUTE,
  CS_THREAD_NO_PERMISSION,
  CS_THREAD_UNKNOWN_THREAD,
  CS_THREAD_DEADLOCK,
  CS_THREAD_OPERATION_PENDING,
  CS_THREAD_MUTEX_NOT_INITIALIZED,
  CS_THREAD_MUTEX_BUSY,
  CS_THREAD_MUTEX_UNKNOWN,
  CS_THREAD_CONDITION_TIMEOUT,
  CS_THREAD_CONDITION_BUSY,
  CS_THREAD_CONDITION_WAIT_INTERRUPTED,
  CS_THREAD_SIGNAL_UNKNOWN,
  CS_THREAD_SEMA_VALUE_TOO_LARGE,
  CS_THREAD_SEMA_BUSY
};

/**
 * Abstract interface for objects which can be run in a thread.  Objects which
 * want to be run in a thread must implement this interface.
 */
class CS_CRYSTALSPACE_EXPORT csRunnable
{
// Jorrit: removed the code below as it causes 'pure virtual' method
// calls to happen upon destruction.
//protected:
  //// Needed for GCC4. Otherwise emits a flood of "virtual functions but
  //// non-virtual destructor" warnings.
  //virtual ~csRunnable() {}
public:
  /// Implement this method to allow your object to be run in a thread.
  virtual void Run () = 0;

  /* maybe we could add those for smoother exit/cancel/detroy operations
  virtual void PrepareExit () = 0;
  virtual void PrepareJoin () = 0;
  virtual void PrepareKill () = 0;
  virtual void PrepareCancel () = 0;
  virtual void PrepareDestroy () = 0;
  */

  /// Increment reference count.
  virtual void IncRef() = 0;
  /// Decrement reference count.
  virtual void DecRef() = 0;
  /// Get reference count.
  virtual int GetRefCount() = 0;
};


/**
 * Representation of a thread of executation.
 */
class CS_CRYSTALSPACE_EXPORT csThread : public csRefCount
{
public:
  /**
   * Create a new thread.  Returns the new thread object.  The thread begins
   * running when the Start() method is invoked.
   */
  static csRef<csThread> Create (csRunnable*, uint32 options = 0);

  /**
   * Start the thread.
   * If something goes awry false is returned.
   */
  virtual bool Start () = 0;

  /**
   * Unmercifully stop the thread as soon as possible.
   * This method performs a dirty shutdown of the thread.  The thread is not
   * given a chance to exit normally.  Do not invoke this method unless you
   * have a very good reason for doing so.  In general, it is best to implement
   * some sort of communication with threads so that you can ask them to
   * terminate in an orderly fashion.  Returns true if the thread was killed.
   */
  virtual bool Stop () = 0;

  /**
   * Wait for the thread to die.  Only returns once the thread has terminated.
   */
  virtual bool Wait () = 0;

  /**
   * Yield Thread frees CPU time if nothing to do.
   */
  virtual void Yield () = 0;

  /**
   * Return the last error description, else 0 if there was none.
   */
  virtual char const* GetLastError () const = 0;
};


/**
 * A mutual-exclusion object.  A thread-safe lock.  Mutexes are often used to
 * control access to one or more resources shared by multiple threads of
 * execution.  A thread should access the shared resource(s) only after it has
 * successfully locked the mutex; and it should unlock the mutex when it is
 * done accessing the shared resource so that other threads may access it.
 */
class CS_CRYSTALSPACE_EXPORT csMutex : public csRefCount
{
public:
  /**
   * Create a mutex.
   * \param recursive If true, the same thread can lock the mutex even when it
   *   already holds the lock without deadlocking. If false, then recursive
   *   locking within the same thread is not supported, and will probably
   *   deadlock.
   * \return The new mutex.
   * \remarks On Windows, mutexes always exhibit recursive semantics, however,
   *   for best portability, you should still choose an appropriate value for
   *   \a recursive.  On other platforms, such as @sc{gnu}/Linux, non-recursive
   *   threads may be the default since they are can be implemented more
   *   efficiently.
   */
  static csRef<csMutex> Create (bool recursive = false);

  /**
   * Lock the mutex.  Suspends execution of the thread until the mutex can be
   * locked.  Each LockWait() must be balanced by a call to Release().  Returns
   * true if locking succeeded.  Returns false if locking failed for some
   * catastrophic reason; check GetLastError().
   */
  virtual bool LockWait() = 0;

  /**
   * Lock the mutex if not already locked by some other entity.  Does not
   * suspend the thread waiting for the lock.  If lock succeeded, returns true.
   * If lock failed, immediately returns false.  Each successful call to
   * LockTry() must be balanced with a call to Release().
   */
  virtual bool LockTry () = 0;

  /**
   * Unlock the mutex.  Each successful call to LockWait() or LockTry() must be
   * balanced by a call to Release().  Returns true if unlocking succeeded.
   * Returns false if unlocking failed for some catastrophic reason; check
   * GetLastError().
   */
  virtual bool Release () = 0;

  /**
   * Return the last error description, else 0 if there was none.
   */
  virtual char const* GetLastError () const = 0;

  /**
   * Return true if the mutex is recursive.
   */
  virtual bool IsRecursive () const = 0;
};


/**
 * A semaphore object.
 */
class CS_CRYSTALSPACE_EXPORT csSemaphore : public csRefCount
{
public:
  /// Create a semaphore with some initial value.
  static csRef<csSemaphore> Create (uint32 value = 0);

  /**
   * Lock the semaphore.  Suspends execution of the thread until the mutex can
   * be locked.  Each LockWait() must be balanced by a call to Release().
   * Returns true if locking succeeded.  Returns false if locking failed for
   * some catastrophic reason; check GetLastError().
   */
  virtual bool LockWait () = 0;

  /**
   * Lock the semaphore if not already locked by some other entity.  Does not
   * suspend the thread waiting for the lock.  If lock succeeded, returns true.
   * If lock failed, immediately returns false.  Each successful call to
   * LockTry() must be balanced with a call to Release().
   */
  virtual bool LockTry () = 0;

  /**
   * Unlock the semaphore.  Each successful call to LockWait() or LockTry()
   * must be balanced by a call to Release().  Returns true if unlocking
   * succeeded.  Returns false if unlocking failed for some catastrophic
   * reason; check GetLastError().
   */
  virtual bool Release () = 0;

  /// Retrieve the present value of the semaphore.
  virtual uint32 Value () = 0;

  /**
   * Return the last error description, else 0 if there was none.
   */
  virtual char const* GetLastError () const = 0;
};


/**
 * A condition object.
 */
class CS_CRYSTALSPACE_EXPORT csCondition : public csRefCount
{
public:
  /// Create a condition  with specific attributes.
  static csRef<csCondition> Create (uint32 conditionAttributes = 0);

  /**
   * Wake up one or all threads waiting upon a change of condition.  If WakeAll
   * is false, only one waiting thread will be awakened and given access to the
   * associated mutex.  If WakeAll is true, all threads waiting on the
   * condition will be awakened and will vie for the associated mutex.  Only
   * one thread will win the mutex (thus gaining access to the condition); all
   * other waiting threads will be re-suspended.
   */
  virtual void Signal (bool WakeAll = false) = 0;

  /**
   * Wait for some change of condition.  Suspends the calling thread until some
   * other thread invokes Signal() to notify a change of condition.
   * \param mutex The mutex to associate with this condition. The caller must
   *   already hold a lock on the mutex before calling Wait(), and all threads
   *   waiting on the condition must be using the same mutex. The mutex must
   *   <b>not</b> be locked recursively within the same thread. When called,
   *   Wait() releases the caller's lock on the mutex and suspends the caller's
   *   thread. Upon return from Wait(), the caller's lock on the mutex is
   *   restored.
   * \param timeout The amount of time in milliseconds to wait for the the
   *   condition's state to change. If zero, the default, then it waits for a
   *   state change without timing-out.  If non-zero, and the indicated time
   *   elapses without a state change being signaled, then false is returned.
   * \return Returns true if the caller was wakened normally.  Returns false if
   *   the wait timed out, or if invoked with a recursive mutex, which is an
   *   invalid invocation style.
   * \remarks The reason that the mutex must not be locked recursively is
   *   because the implicit unlock performed by Wait() <em>must</em> actually
   *   release the mutex in order for other threads to be able to satisfy the
   *   condition. With recursively locked mutexes, there is no guarantee that
   *   the one implicit unlock operation performed by Wait() will actually
   *   release the mutex since it might have been locked multiple times within
   *   the same thread.
   */
  virtual bool Wait (csMutex* mutex, csTicks timeout = 0) = 0;

  /**
   * Return the last error description, else null if there was none.
   */
  virtual char const* GetLastError () const = 0;
};

#endif // __CS_CSSYS_THREAD_H__
