#ifndef _CS_POSIX_THREAD_H_
#define _CS_POSIX_THREAD_H_

#include <pthread.h>
#include <semaphore.h>
#include "csutil/ref.h"

struct iRunnable;

class csPosixThread
{
 protected:
  /**
   * Construct a new thread.
   * The thread does not run yet, you have to call Start () upon it
   */
  csPosixThread (iRunnable *runnable, uint32 options);
  ~csPosixThread ();

  /**
   * This actually starts the thread.
   * If something gone wrong false is returned.
   */
  bool Start ();

  /**
   * Kindly asking the thread to stop. 
   * (This is eventually honred by the thread at cancellation points like MutexLocks)
   */
  bool Stop ();

  bool Wait ();

  /**
   * This brutally kills the thread. No kindly asking, no nothing.
   */
  bool Kill ();

  /**
   * Return the last eror description and NULL if there was none.
   */
  const char* GetLastError ();

  static void* ThreadRun (void *param);
 protected:
  pthread_t thread;
  csRef<iRunnable> runnable;
  const char *lasterr;
  bool running, created;
};

class csPosixMutex
{
 protected:
  csPosixMutex ();
  ~csPosixMutex ();
  
  bool LockWait ();
  bool LockTry  ();
  bool Release  ();
  const char* GetLastError ();
  static void Cleanup (void *arg);

 private:
  bool Destroy ();
 protected:
  pthread_mutex_t mutex;
  const char *lasterr;
  friend class csPosixCondition;
};

class csPosixSemaphore
{
 protected:
  csPosixSemaphore (uint32 value);
  ~csPosixSemaphore ();
  bool LockWait ();
  bool LockTry ();
  bool Release ();
  uint32 Value ();
  const char* GetLastError ();
 protected:
  const char *lasterr;
  sem_t sem;
 private:
  bool Destroy ();
};

class csPosixCondition
{
 protected:
  csPosixCondition (uint32 conditionAttributes);
  ~csPosixCondition ();
  void Signal (bool bAll);
  bool Wait (csPosixMutex *mutex, csTicks timeout);
  const char* GetLastError ();
 private:
  bool Destroy ();
 private:
  pthread_cond_t cond;
  const char* lasterr;
};

typedef csPosixThread csThreadImpl;
typedef csPosixMutex csMutexImpl;
typedef csPosixSemaphore csSemaphoreImpl;
typedef csPosixCondition csConditionImpl;

#define CS_SAFE_LOCKWAIT(m) m.LockWait(); pthread_cleanup_push(m.Cleanup,&m)
#define CS_SAFE_RELEASE(m) pthread_cleanup_pop(0); m.Release()

#endif
