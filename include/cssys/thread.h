/*
    Copyright (C) 2002 by Norman Kr�mer
  
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

// list of errorcodes
#define CS_THREAD_NO_ERROR                    0
#define CS_THREAD_UNKNOWN_ERROR               (CS_THREAD_NO_ERROR+1)
#define CS_THREAD_OUT_OF_RESOURCES            (CS_THREAD_UNKNOWN_ERROR+1)
#define CS_THREAD_ERR_ATTRIBUTE               (CS_THREAD_OUT_OF_RESOURCES+1)
#define CS_THREAD_NO_PERMISSION               (CS_THREAD_ERR_ATTRIBUTE+1)
#define CS_THREAD_UNKNOWN_THREAD              (CS_THREAD_NO_PERMISSION+1)
#define CS_THREAD_DEADLOCK                    (CS_THREAD_UNKNOWN_THREAD+1)
#define CS_THREAD_OPERATION_PENDING           (CS_THREAD_DEADLOCK+1)
#define CS_THREAD_MUTEX_NOT_INITIALIZED       (CS_THREAD_OPERATION_PENDING+1)
#define CS_THREAD_MUTEX_BUSY                  (CS_THREAD_MUTEX_NOT_INITIALIZED+1)
#define CS_THREAD_MUTEX_UNKNOWN               (CS_THREAD_MUTEX_BUSY+1)
#define CS_THREAD_CONDITION_TIMEOUT           (CS_THREAD_MUTEX_UNKNOWN+1)
#define CS_THREAD_CONDITION_BUSY              (CS_THREAD_CONDITION_TIMEOUT+1)
#define CS_THREAD_CONDITION_WAIT_INTERRUPTED  (CS_THREAD_CONDITION_BUSY+1)
#define CS_THREAD_SIGNAL_UNKNOWN              (CS_THREAD_CONDITION_WAIT_INTERRUPTED+1)
#define CS_THREAD_SEMA_VALUE_TOO_LARGE        (CS_THREAD_SIGNAL_UNKNOWN+1)
#define CS_THREAD_SEMA_BUSY                   (CS_THREAD_SEMA_VALUE_TOO_LARGE+1)


/**
 * Abstract interface for objects which can be run in a thread.  Objects which
 * want to be run in a thread must implement this interface.
 */
class csRunnable : public csRefCount
{
public:
  /**
   * Implement this interface and your implementation can be run in a thread.
   */
  virtual void Run () = 0;

  /* maybe we could add those for smoother exit/cancel/detroy operations
  virtual void PrepareExit () = 0;
  virtual void PrepareJoin () = 0;
  virtual void PrepareKill () = 0;
  virtual void PrepareCancel () = 0;
  virtual void PrepareDestroy () = 0;
  */
};


/**
 * A platform-independent thread object.
 */
class csThread : public csRefCount
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
   * Ask the thread to stop. 
   * This is eventually honred by the thread at cancellation points like
   * MutexLocks.  You can then call Start () again; it does not resume
   * operation where stopped but starts over again.
   */
  virtual bool Stop () = 0;

  /// Wait for the thread to die.  Only returns once the thread has terminated.
  virtual bool Wait () = 0;

  /**
   * Brutally kill the thread.
   */
  virtual bool Kill () = 0;

  /**
   * Return the last error description, else NULL if there was none.
   */
  virtual char const* GetLastError () = 0;
};


/**
 * A platform-independent mutual-exclusion object.
 */
class csMutex : public csRefCount
{
public:
  static csRef<csMutex> Create ();
  virtual bool LockWait() = 0;
  virtual bool LockTry () = 0;
  virtual bool Release () = 0;
  virtual char const* GetLastError () = 0;
};


/**
 * A platform-independent semaphore object.
 */
class csSemaphore : public csRefCount
{
public:
  static csRef<csSemaphore> Create (uint32 value);
  virtual bool LockWait () = 0;
  virtual bool LockTry () = 0;
  virtual bool Release () = 0;
  virtual uint32 Value () = 0;
  virtual char const* GetLastError () = 0;
};


/**
 * A platform-independent condition object.
 */
class csCondition : public csRefCount
{
public:
  static csRef<csCondition> Create (uint32 conditionAttributes = 0);
  virtual void Signal (bool WakeAll = false) = 0;
  virtual bool Wait (csMutex*, csTicks timeout = 0) = 0;
  virtual char const* GetLastError () = 0;
};

#endif // __CS_CSSYS_THREAD_H__
