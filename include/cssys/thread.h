/*
    Copyright (C) 2002 by Norman Krämer
  
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

#ifndef _CS_THREAD_H_
#define _CS_THREAD_H_

#include "csutil/scf.h"

// list of thread initialisation attributes

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

SCF_VERSION (iRunnable, 0, 0, 1);

struct iRunnable : public iBase
{

  /**
   * Implement this interface and your implementation can be run in a thread.
   */
  virtual void Run () = 0;

  /*
  // maybe we could add those for smoother exit/cancel/detroy operations
  virtual void PrepareExit () = 0;
  virtual void PrepareJoin () = 0;
  virtual voidPrepareKill () = 0;
  virtual voidPrepareCancel () = 0;
  virtual voidPrepareDestroy () = 0;
  */
};

struct csRefCounter
{
  int refcount;
  csRefCounter ():refcount(1) {}
  virtual void IncRef (){ refcount++; }
  virtual void DecRef (){ if (--refcount <= 0) delete this; }
  virtual int GetRefCount () {return refcount;}
};

struct csThread : public csRefCounter
{
  /**
   * This actually starts the thread.
   * If something gone wrong false is returned.
   */
  virtual bool Start () = 0;

  /**
   * Kindly asking the thread to stop. 
   * (This is eventually honred by the thread at cancellation points like MutexLocks)
   * You can then call Start () again (it does not resume operation where stopped but starts over agin)
   */
  virtual bool Stop () = 0;

  virtual bool Wait () = 0;

  /**
   * This brutally kills the thread. No kindly asking, no nothing.
   */
  virtual bool Kill () = 0;

  /**
   * Return the last eror description and NULL if there was none.
   */
  virtual const char *GetLastError () = 0;

  static csPtr<csThread> Create (iRunnable *runnable, uint32 options=0);
};

struct csMutex : public csRefCounter
{
  friend struct csCondition;
  static csPtr<csMutex> Create ();
  virtual bool LockWait() = 0;
  virtual bool LockTry () = 0;
  virtual bool Release () = 0;
  virtual const char* GetLastError () = 0;
};

struct csSemaphore : public csRefCounter
{
  static csPtr<csSemaphore> Create (uint32 value);
  virtual bool LockWait () = 0;
  virtual bool LockTry () = 0;
  virtual bool Release () = 0;
  virtual uint32 Value () = 0;
  virtual const char* GetLastError () = 0;
};

struct csCondition : public csRefCounter
{
  static csPtr<csCondition> Create (uint32 conditionAttributes=0);
  virtual void Signal (bool bAll = false) = 0;
  virtual bool Wait (csMutex *mutex, csTicks timeout=0) = 0;
  virtual const char* GetLastError () = 0;
};

#include CS_THREAD_INC

#endif
