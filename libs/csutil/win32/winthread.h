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
#ifndef __CS_WINTHREAD_H__
#define __CS_WINTHREAD_H__

#include "csextern.h"
#include "csutil/thread.h"

class CS_CRYSTALSPACE_EXPORT csWinThread : public csThread
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
   * Yield thread frees CPU time if nothing to do.
   */
  virtual void Yield ();

  /**
   * Return the last eror description and 0 if there was none.
   */
  virtual char const* GetLastError () const;

 protected:
  static uint __stdcall ThreadRun (void* param);
 protected:
  HANDLE thread;
  csRef<csRunnable> runnable;
  char* lasterr;
  bool running;
  bool created;
};

class CS_CRYSTALSPACE_EXPORT csWinMutex : public csMutex
{
public:
  csWinMutex (bool recursive);
  virtual ~csWinMutex ();
  
  virtual bool LockWait ();
  virtual bool LockTry  ();
  virtual bool Release  ();
  virtual char const* GetLastError () const;
  virtual bool IsRecursive () const;

 private:
  bool Destroy ();
 protected:
  HANDLE mutex;
  char* lasterr;
  bool recursive;
  friend class csWinCondition;
};

class CS_CRYSTALSPACE_EXPORT csWinSemaphore : public csSemaphore
{
public:
  csWinSemaphore (uint32 value);
  virtual ~csWinSemaphore ();

  virtual bool LockWait ();
  virtual bool LockTry ();
  virtual bool Release ();
  virtual uint32 Value ();
  virtual char const* GetLastError () const;

 protected:
  char* lasterr;
  LONG value;
  HANDLE sem;
 private:
  bool Destroy ();
};

class CS_CRYSTALSPACE_EXPORT csWinCondition : public csCondition
{
 public:
  csWinCondition (uint32 conditionAttributes);
  virtual ~csWinCondition ();

  virtual void Signal (bool bAll);
  virtual bool Wait (csMutex*, csTicks timeout);
  virtual char const* GetLastError () const;

 private:
  bool LockWait (DWORD nMilliSec);
  bool Destroy ();
 private:
  HANDLE cond;
  char* lasterr;
};

#endif // __CS_WINTHREAD_H__
