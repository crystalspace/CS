#ifndef _CS_POSIX_THREAD_H_
#define _CS_POSIX_THREAD_H_

#include <pthread.h>
#include <semaphore.h>
#include "csutil/ref.h"

struct iRunnable;

class csPosixThread : public csThread
{
 public:
  /**
   * Construct a new thread.
   * The thread does not run yet, you have to call Start () upon it
   */
  csPosixThread (iRunnable *runnable, uint32 options);
  virtual ~csPosixThread ();

  /**
   * This actually starts the thread.
   * If something gone wrong false is returned.
   */
  virtual bool Start ();

  /**
   * Kindly asking the thread to stop. 
   * (This is eventually honred by the thread at cancellation points like MutexLocks)
   */
  virtual bool Stop ();

  virtual bool Wait ();

  /**
   * This brutally kills the thread. No kindly asking, no nothing.
   */
  virtual bool Kill ();

  /**
   * Return the last eror description and NULL if there was none.
   */
  virtual const char* GetLastError ();

 protected:
  static void* ThreadRun (void *param);

 protected:
  pthread_t thread;
  csRef<iRunnable> runnable;
  const char *lasterr;
  bool running, created;
};

class csPosixMutex : public csMutex
{
 public:
  csPosixMutex ();
  virtual ~csPosixMutex ();

  virtual bool LockWait ();
  virtual bool LockTry  ();
  virtual bool Release  ();
  virtual const char* GetLastError ();

  static void Cleanup (void *arg);
 protected:
  
 private:
  bool Destroy ();
 protected:
  pthread_mutex_t mutex;
  const char *lasterr;
  friend class csPosixCondition;
};

class csPosixSemaphore : public csSemaphore
{
 public:
  csPosixSemaphore (uint32 value);
  virtual ~csPosixSemaphore ();

  virtual bool LockWait ();
  virtual bool LockTry ();
  virtual bool Release ();
  virtual uint32 Value ();
  virtual const char* GetLastError ();

 protected:
  const char *lasterr;
  sem_t sem;
 private:
  bool Destroy ();
};

class csPosixCondition : public csCondition
{
 public:
  csPosixCondition (uint32 conditionAttributes);
  virtual ~csPosixCondition ();

  virtual void Signal (bool bAll=false);
  virtual bool Wait (csMutex *mutex, csTicks timeout=0);
  virtual const char* GetLastError ();

 private:
  bool Destroy ();
 private:
  pthread_cond_t cond;
  const char* lasterr;
};

#define CS_SAFE_LOCKWAIT(m) m->LockWait(); pthread_cleanup_push(((csPosixMutex*)(csMutex*)m)->Cleanup,m)
#define CS_SAFE_RELEASE(m) pthread_cleanup_pop(0); m->Release()

#endif
