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

#ifndef __CS_CSUTIL_THREADING_WIN32_MUTEX_H__
#define __CS_CSUTIL_THREADING_WIN32_MUTEX_H__

#include "csutil/threading/atomicops.h"
#include "csutil/threading/win32_apifuncs.h"

#if !defined(CS_PLATFORM_WIN32)
#error "This file is only for Windows and requires you to include csysdefs.h before"
#else

#ifdef CS_THREAD_CHECKER
#include <libittnotify.h>
#endif

namespace CS
{
namespace Threading
{
namespace Implementation
{

  /**
   * Basic implementation of non-recursive mutex for win32
   */
  class MutexBase
  {
  public:
    void Initialize ()
    {
      activeFlag = 0;
      semaphore = 0;
    }

    void Destroy ()
    {
      void* oldSemaphore = AtomicOperations::Set (&semaphore, (void*)0);
      if (oldSemaphore)
      {
        Implementation::CloseHandle (semaphore);
      }
    }
      

    bool Lock ()
    {
#ifdef CS_THREAD_CHECKER
      __itt_notify_sync_prepare((void *)this);
#endif
      if (AtomicOperations::Increment (&activeFlag) != 1)
      {
        Implementation::WaitForSingleObject (GetSemaphore (), INFINITE);
      }
#ifdef CS_THREAD_CHECKER
      __itt_notify_sync_acquired((void *)this);
#endif
      return IsLocked ();
    }

    bool TryLock ()
    {
#ifdef CS_THREAD_CHECKER
      __itt_notify_sync_prepare((void *)this);
      bool locked = !AtomicOperations::CompareAndSet (&activeFlag, 1, 0);
      if(locked)
        __itt_notify_sync_acquired((void *)this);
      else
        __itt_notify_sync_cancel((void *)this);
      return locked;
#else
      return !AtomicOperations::CompareAndSet (&activeFlag, 1, 0);
#endif
    }

    void Unlock ()
    {
#ifdef CS_THREAD_CHECKER
      __itt_notify_sync_releasing((void *)this);
#endif
      if (AtomicOperations::Decrement (&activeFlag) > 0)
      {
        Implementation::ReleaseSemaphore (GetSemaphore (), 1, 0);
      }
    }

  protected:
    friend class RecursiveMutexBase;
    bool IsLocked ()
    {
      return AtomicOperations::Read (&activeFlag) > 0;
    }
  
    void* GetSemaphore ()
    {
      void* currentSem = AtomicOperations::Read (&semaphore);
      if (!currentSem)
      {
        //Create a new semaphore and try to set it
        void* const newSem = Implementation::CreateSemaphoreA (0,0,1,0);
        void* const oldSem = AtomicOperations::CompareAndSet (&semaphore, 
          newSem, 0);

        //We already have one, use it
        if (oldSem != 0)
        {
          Implementation::CloseHandle (newSem);
          return oldSem;
        }
        else
        {
          //We didn't have any before, so use our new semaphore
          return newSem;
        }
      }
      return currentSem;
    }

    int32 activeFlag; //Lock flag for mutex
    void* semaphore; //Semaphore for being able to wait for
  };


  /**
  * Basic implementation of recursive mutex for win32
  */
  class RecursiveMutexBase
  {
  public:
    void Initialize ()
    {
      recursionCount = 0;
      lockingThreadID = 0;
      mutex.Initialize ();
    }

    void Destroy ()
    {
      mutex.Destroy ();
    }

    bool IsLocked ()
    {
      return mutex.IsLocked ();
    }

    bool Lock ()
    {
      int32 currentThreadID = (int32)Implementation::GetCurrentThreadId ();
      if (!TryRecursiveLock (currentThreadID))
      {
        mutex.Lock ();
        AtomicOperations::Set (&lockingThreadID, currentThreadID);
        recursionCount = 1;
      }
      return IsLocked ();
    }

    bool TryLock ()
    {
      int32 currentThreadID = (int32)Implementation::GetCurrentThreadId ();
      return TryRecursiveLock (currentThreadID) || 
             TryNormalLock (currentThreadID);
    }

    void Unlock ()
    {
      if(!--recursionCount)
      {
        AtomicOperations::Set (&lockingThreadID, 0);
        mutex.Unlock ();
      }
    }

  private:
    bool TryRecursiveLock (int32 currentThreadID)
    {
      if (AtomicOperations::Read (&lockingThreadID) == currentThreadID)
      {
        ++recursionCount;
        return true;
      }
      return false;
    }

    bool TryNormalLock (int32 currentThreadID)
    {
      if (mutex.TryLock ())
      {
        AtomicOperations::Set (&lockingThreadID, currentThreadID);
        recursionCount = 1;
        return true;
      }
      return false;
    }

    MutexBase mutex; //Non-recursive base-mutex
    int32 recursionCount;
    int32 lockingThreadID;
  }; 

} // namespace Implementation
} // namespace Threading
} // namespace CS

#endif // !defined(CS_PLATFORM_WIN32)

#endif // __CS_CSUTIL_THREADING_WIN32_MUTEX_H__
