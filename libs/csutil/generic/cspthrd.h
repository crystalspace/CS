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

#ifndef __CS_POSIX_THREAD_H__
#define __CS_POSIX_THREAD_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>
#include <semaphore.h>
#include "csutil/thread.h"

class csPosixThread : public csThread
{
 public:
  /**
   * Construct a new thread.
   * The thread does not run yet, you have to call Start () upon it
   */
  csPosixThread (csRunnable*, uint32 options);
  virtual ~csPosixThread ();

  /**
   * This actually starts the thread.
   * If something gone wrong false is returned.
   */
  virtual bool Start ();

  /**
   * Unmercifully stop the thread as soon as possible.
   * This method performs a dirty shutdown of the thread.  The thread is not
   * given a chance to exit normally.  Do not invoke this method unless you
   * have a very good reason for doing so.  In general, it is best to implement
   * some sort of communication with threads so that you can ask them to
   * terminate in an orderly fashion.  Returns true if the thread was killed.
   */
  virtual bool Stop ();

  /**
   * Wait for the thread to die.  Only returns once the thread has terminated.
   */
  virtual bool Wait ();

  /**
   * Yield Thread frees CPU time if nothing to do.
   */
  virtual void Yield ();

  /**
   * Return the last eror description and 0 if there was none.
   */
  virtual char const* GetLastError ();

 protected:
  static void* ThreadRun (void* param);

 protected:
  pthread_t thread;
  csRef<csRunnable> runnable;
  const char *lasterr;
  bool running;
  bool created;
};

class csPosixMutex : public csMutex
{
public:
  virtual ~csPosixMutex ();

  virtual bool LockWait ();
  virtual bool LockTry  ();
  virtual bool Release  ();
  virtual char const* GetLastError ();

protected:
  friend class csMutex;
  csPosixMutex (pthread_mutexattr_t* attr);
  
  static void Cleanup (void* arg);
private:
  bool Destroy ();

protected:
  pthread_mutex_t mutex;
  
  int lasterr;
  friend class csPosixCondition;
};

#ifndef CS_PTHREAD_NUTEX_RECURSIVE
class csPosixMutexRecursive : public csPosixMutex
{
public:
  virtual ~csPosixMutexRecursive ();

  virtual bool LockWait ();
  virtual bool LockTry ();
  virtual bool Release ();
  
protected:
  friend class csMutex;
  csPosixMutexRecursive (pthread_mutexattr_t* attr);

private:
  int count;
  pthread_t owner;
};
#endif

class csPosixSemaphore : public csSemaphore
{
public:
  csPosixSemaphore (uint32 value);
  virtual ~csPosixSemaphore ();

  virtual bool LockWait ();
  virtual bool LockTry ();
  virtual bool Release ();
  virtual uint32 Value ();
  virtual char const* GetLastError ();

protected:
  char const *lasterr;
  sem_t sem;

private:
  bool Destroy ();
};

class csPosixCondition : public csCondition
{
public:
  csPosixCondition (uint32 conditionAttributes);
  virtual ~csPosixCondition ();

  virtual void Signal (bool WakeAll = false);
  virtual bool Wait (csMutex*, csTicks timeout = 0);
  virtual char const* GetLastError ();

private:
  bool Destroy ();

  pthread_cond_t cond;
  char const* lasterr;
};

#endif // __CS_POSIX_THREAD_H__

