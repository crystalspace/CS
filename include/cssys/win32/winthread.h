#ifndef _CS_WINTHREAD_H_
#define _CS_WINTHREAD_H_

#include "csutil/ref.h"

struct iRunnable;

class csWinThread : public csThread
{
 public:
  /**
   * Construct a new thread.
   * The thread does not run yet, you have to call Start () upon it
   */
  csWinThread (iRunnable *runnable, uint32 options);
  virtual ~csWinThread ();

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
  static void ThreadRun (void *param);
 protected:
  HANDLE thread;
  csRef<iRunnable> runnable;
  LPTSTR lasterr;
  bool running, created;
};

class csWinMutex : public csMutex
{
public:
  csWinMutex ();
  virtual ~csWinMutex ();
  
  virtual bool LockWait ();
  virtual bool LockTry  ();
  virtual bool Release  ();
  virtual const char* GetLastError ();

 private:
  bool Destroy ();
 protected:
  HANDLE mutex;
  LPTSTR lasterr;
  friend class csWinCondition;
};

class csWinSemaphore : public csSemaphore
{
public:
  csWinSemaphore (uint32 value);
  virtual ~csWinSemaphore ();
  virtual bool LockWait ();
  virtual bool LockTry ();
  virtual bool Release ();
  virtual uint32 Value ();
  virtual const char* GetLastError ();
 protected:
  LPTSTR lasterr;
  LONG value;
  HANDLE sem;
 private:
  bool Destroy ();
};

class csWinCondition : public csCondition
{
 public:
  csWinCondition (uint32 conditionAttributes);
  virtual ~csWinCondition ();
  virtual void Signal (bool bAll);
  virtual bool Wait (csMutex *mutex, csTicks timeout);
  virtual const char* GetLastError ();

 private:
  bool LockWait (DWORD nMilliSec);
  bool Destroy ();
 private:
  HANDLE cond;
  LPTSTR lasterr;
};

#define CS_SAFE_LOCKWAIT(m) m->LockWait()
#define CS_SAFE_RELEASE(m) m->Release()

#endif
