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
#ifndef __CS_WINTHREAD_H__
#define __CS_WINTHREAD_H__

#include "cssys/thread.h"

class csWinThread : public csThread
{
 public:
  /**
   * Construct a new thread.
   * The thread does not run yet, you have to call Start() upon it
   */
  csWinThread (csRunnable*, uint32 options);
  virtual ~csWinThread ();

  /**
   * This actually starts the thread.
   * If something gone wrong false is returned.
   */
  virtual bool Start ();

  /**
   * Ask the thread to stop. 
   * This is eventually honred by the thread at cancellation points like
   * MutexLocks.  You can then call Start () again; it does not resume
   * operation where stopped but starts over again.
   */
  virtual bool Stop ();

  /// Wait for the thread to die.  Only returns once the thread has terminated.
  virtual bool Wait ();

  /**
   * Brutally kill the thread.
   */
  virtual bool Kill ();

  /**
   * Return the last eror description and NULL if there was none.
   */
  virtual char const* GetLastError ();

 protected:
  static void ThreadRun (void* param);
 protected:
  HANDLE thread;
  csRef<csRunnable> runnable;
  LPTSTR lasterr;
  bool running;
  bool created;
};

class csWinMutex : public csMutex
{
public:
  csWinMutex ();
  virtual ~csWinMutex ();
  
  virtual bool LockWait ();
  virtual bool LockTry  ();
  virtual bool Release  ();
  virtual char const* GetLastError ();

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
  virtual char const* GetLastError ();

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
  virtual bool Wait (csMutex*, csTicks timeout);
  virtual char const* GetLastError ();

 private:
  bool LockWait (DWORD nMilliSec);
  bool Destroy ();
 private:
  HANDLE cond;
  LPTSTR lasterr;
};

#endif // __CS_WINTHREAD_H__
